import json
import math
from dataclasses import dataclass, field
from typing import List, Dict, Any, Optional, Tuple
from enum import Enum

from pixel import Pixel
from d3d import (
    D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
    D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
    D3D_PRIMITIVE_TOPOLOGY_LINELIST,
    D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN
)


class CullMode(Enum):
    NONE = 0
    FRONT = 1
    BACK = 2


class FillMode(Enum):
    POINT = 0
    LINE = 1
    SOLID = 2


class FrontFace(Enum):
    COUNTER_CLOCKWISE = 0
    CLOCKWISE = 1


@dataclass
class Viewport:
    x: float = 0.0
    y: float = 0.0
    width: float = 0.0
    height: float = 0.0
    min_depth: float = 0.0
    max_depth: float = 1.0

    def contains(self, x: float, y: float) -> bool:
        """Check if point is inside viewport"""
        return (self.x <= x < self.x + self.width and
                self.y <= y < self.y + self.height)

    def transform_to_screen(self, clip_x: float, clip_y: float, clip_w: float) -> Tuple[int, int]:
        """Transform clip coordinates to screen coordinates"""
        if abs(clip_w) < 1e-8:
            return (int(self.x + self.width / 2), int(self.y + self.height / 2))
        ndc_x = clip_x / clip_w
        ndc_y = clip_y / clip_w
        screen_x = int((ndc_x + 1.0) * 0.5 * self.width + self.x)
        screen_y = int((1.0 - (ndc_y + 1.0) * 0.5) * self.height + self.y)
        return (screen_x, screen_y)


@dataclass
class ScissorRect:
    left: int = 0
    top: int = 0
    right: int = 0
    bottom: int = 0

    def contains(self, x: int, y: int) -> bool:
        """Check if point is inside scissor rect (exclusive on right/bottom)"""
        return (self.left <= x < self.right and
                self.top <= y < self.bottom)


@dataclass
class RasterizerConfig:
    cull_mode: CullMode = CullMode.BACK
    fill_mode: FillMode = FillMode.SOLID
    front_face: FrontFace = FrontFace.COUNTER_CLOCKWISE
    scissor_enable: bool = False
    scissor_rect: ScissorRect = None
    multisample_enable: bool = False
    antialiasing_line_enable: bool = False
    depth_clip_enable: bool = True
    viewport: Viewport = None

    def __post_init__(self):
        if self.scissor_rect is None:
            self.scissor_rect = ScissorRect()
        if self.viewport is None:
            self.viewport = Viewport()


@dataclass
class Triangle:
    """Triangle primitive with vertices and interpolated attributes"""
    v0: Dict[str, Any]  # Vertex 0 output data
    v1: Dict[str, Any]  # Vertex 1 output data
    v2: Dict[str, Any]  # Vertex 2 output data
    primitive_id: int = 0

    def get_position(self, vertex: Dict[str, Any]) -> List[float]:
        """Extract position from vertex output data"""
        if not vertex:
            return [0.0, 0.0, 0.0, 1.0]
        for key in ['sv_position', 'position', 'pos', 'Pos', 'SV_Position']:
            if key in vertex and vertex[key]:
                pos = vertex[key]
                if isinstance(pos, list):
                    if len(pos) == 4:
                        return pos
                    elif len(pos) == 3:
                        return [pos[0], pos[1], pos[2], 1.0]
        return [0.0, 0.0, 0.0, 1.0]

    def get_attribute(self, vertex: Dict[str, Any], attr_name: str) -> Any:
        """Extract attribute from vertex output data"""
        if not vertex:
            return None
        attr_name_lower = attr_name.lower()
        for key, value in vertex.items():
            if key.lower() == attr_name_lower:
                return value
        return None


class Rasterizer:
    """
    D3D11-style rasterizer implementation

    Receives HLSLInterpreter's vertex shader output and performs:
    - Primitive assembly (points, lines, triangles)
    - Vertex post-processing (viewport transform, clipping)
    - Triangle culling
    - Rasterization with barycentric coordinate interpolation
    - Scissor testing
    - MSAA support
    """

    def __init__(self, config_path: str = None):
        self.config = RasterizerConfig()
        self._primitive_id_counter = 0
        self._pixels: List[Pixel] = []

        if config_path:
            self.load_config(config_path)

    def load_config(self, config_path: str):
        """Load rasterizer configuration from JSON file"""
        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                config_data = json.load(f)

            cull_mode_map = {
                'none': CullMode.NONE,
                'front': CullMode.FRONT,
                'back': CullMode.BACK
            }
            cull_mode_str = config_data.get('cull_mode', 'back').lower()
            self.config.cull_mode = cull_mode_map.get(cull_mode_str, CullMode.BACK)

            fill_mode_map = {
                'point': FillMode.POINT,
                'line': FillMode.LINE,
                'solid': FillMode.SOLID
            }
            fill_mode_str = config_data.get('fill_mode', 'solid').lower()
            self.config.fill_mode = fill_mode_map.get(fill_mode_str, FillMode.SOLID)

            front_face_str = config_data.get('front_face', 'counter_clockwise').lower()
            if front_face_str == 'clockwise':
                self.config.front_face = FrontFace.CLOCKWISE
            else:
                self.config.front_face = FrontFace.COUNTER_CLOCKWISE

            self.config.scissor_enable = config_data.get('scissor_enable', False)

            if 'scissor_rect' in config_data:
                sr = config_data['scissor_rect']
                self.config.scissor_rect = ScissorRect(
                    left=sr.get('left', 0),
                    top=sr.get('top', 0),
                    right=sr.get('right', 0),
                    bottom=sr.get('bottom', 0)
                )

            self.config.multisample_enable = config_data.get('multisample_enable', False)
            self.config.antialiasing_line_enable = config_data.get('antialiasing_line_enable', False)
            self.config.depth_clip_enable = config_data.get('depth_clip_enable', True)

            if 'viewport' in config_data:
                vp = config_data['viewport']
                self.config.viewport = Viewport(
                    x=vp.get('x', 0.0),
                    y=vp.get('y', 0.0),
                    width=vp.get('width', 800.0),
                    height=vp.get('height', 600.0),
                    min_depth=vp.get('min_depth', 0.0),
                    max_depth=vp.get('max_depth', 1.0)
                )

        except Exception as e:
            print(f"Warning: Failed to load rasterizer config from {config_path}: {e}")

    def clear_pixels(self):
        """Clear the pixel output buffer"""
        self._pixels = []

    def get_pixels(self) -> List[Pixel]:
        """Get the rasterized pixels"""
        return self._pixels

    def rasterize(self, results: List[Dict[str, Any]], primitive_topology: int = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) -> List[Pixel]:
        """
        Rasterize vertex shader output

        Args:
            results: List of vertex output dictionaries from HLSLInterpreter executorVS
            primitive_topology: D3D_PRIMITIVE_TOPOLOGY_* value

        Returns:
            List of Pixel objects representing rasterized fragments
        """
        self.clear_pixels()

        if primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            self._rasterize_triangle_list(results)
        elif primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            self._rasterize_triangle_strip(results)
        elif primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINELIST:
            self._rasterize_line_list(results)
        elif primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
            self._rasterize_line_strip(results)
        elif primitive_topology == D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
            self._rasterize_point_list(results)

        return self._pixels

    def _rasterize_triangle_list(self, vertices: List[Dict[str, Any]]):
        """Rasterize triangle list - every 3 vertices form a triangle"""
        num_primitives = len(vertices) // 3
        for i in range(num_primitives):
            tri = Triangle(
                v0=vertices[i * 3],
                v1=vertices[i * 3 + 1],
                v2=vertices[i * 3 + 2],
                primitive_id=i
            )
            self._rasterize_triangle(tri)

    def _rasterize_triangle_strip(self, vertices: List[Dict[str, Any]]):
        """Rasterize triangle strip"""
        if len(vertices) < 3:
            return
        for i in range(len(vertices) - 2):
            tri = Triangle(
                v0=vertices[i],
                v1=vertices[i + 1],
                v2=vertices[i + 2],
                primitive_id=i
            )
            self._rasterize_triangle(tri)

    def _rasterize_line_list(self, vertices: List[Dict[str, Any]]):
        """Rasterize line list - every 2 vertices form a line"""
        if len(vertices) < 2:
            return
        for i in range(0, len(vertices) - 1, 2):
            self._rasterize_line(vertices[i], vertices[i + 1], i // 2)

    def _rasterize_line_strip(self, vertices: List[Dict[str, Any]]):
        """Rasterize line strip"""
        if len(vertices) < 2:
            return
        for i in range(len(vertices) - 1):
            self._rasterize_line(vertices[i], vertices[i + 1], i)

    def _rasterize_point_list(self, vertices: List[Dict[str, Any]]):
        """Rasterize point list"""
        for i, vertex in enumerate(vertices):
            self._rasterize_point(vertex, i)

    def _rasterize_point(self, vertex: Dict[str, Any], primitive_id: int):
        """Rasterize a point primitive"""
        pos = self._get_vertex_position(vertex)
        if pos is None:
            return

        clip_w = pos[3] if len(pos) >= 4 else 1.0
        if abs(clip_w) < 1e-8:
            return

        screen_x, screen_y = self.config.viewport.transform_to_screen(pos[0], pos[1], clip_w)

        if not self._is_in_viewport(screen_x, screen_y):
            return

        if self.config.scissor_enable and not self._is_in_scissor(screen_x, screen_y):
            return

        pixel = Pixel(
            x=screen_x,
            y=screen_y,
            depth=pos[2] / clip_w if clip_w != 0 else pos[2],
            color=self._interpolate_vertex_attribute(vertex, 'color'),
            texcoord=self._interpolate_vertex_attribute(vertex, 'texcoord'),
            texcoord2=self._interpolate_vertex_attribute(vertex, 'texcoord2'),
            normal=self._interpolate_vertex_attribute(vertex, 'normal'),
            position=self._interpolate_vertex_attribute(vertex, 'position'),
            attributes={},
            primitive_id=primitive_id
        )
        self._pixels.append(pixel)

    def _rasterize_line(self, v0: Dict[str, Any], v1: Dict[str, Any], primitive_id: int):
        """Rasterize a line primitive using DDA"""
        pos0 = self._get_vertex_position(v0)
        pos1 = self._get_vertex_position(v1)

        if pos0 is None or pos1 is None:
            return

        clip_w0 = pos0[3] if len(pos0) >= 4 else 1.0
        clip_w1 = pos1[3] if len(pos1) >= 4 else 1.0

        if abs(clip_w0) < 1e-8 or abs(clip_w1) < 1e-8:
            return

        screen_x0, screen_y0 = self.config.viewport.transform_to_screen(pos0[0], pos0[1], clip_w0)
        screen_x1, screen_y1 = self.config.viewport.transform_to_screen(pos1[0], pos1[1], clip_w1)

        dx = abs(screen_x1 - screen_x0)
        dy = abs(screen_y1 - screen_y0)
        steps = max(dx, dy) + 1

        if steps < 1:
            steps = 1

        for i in range(int(steps)):
            t = i / max(steps - 1, 1) if steps > 1 else 0
            screen_x = int(screen_x0 + (screen_x1 - screen_x0) * t)
            screen_y = int(screen_y0 + (screen_y1 - screen_y0) * t)

            if not self._is_in_viewport(screen_x, screen_y):
                continue

            if self.config.scissor_enable and not self._is_in_scissor(screen_x, screen_y):
                continue

            depth = pos0[2] + (pos1[2] - pos0[2]) * t if len(pos0) >= 3 and len(pos1) >= 3 else 0.0

            interpolated_attrs = self._interpolate_attributes_line(v0, v1, t, clip_w0, clip_w1)

            pixel = Pixel(
                x=screen_x,
                y=screen_y,
                depth=depth,
                color=interpolated_attrs.get('color'),
                texcoord=interpolated_attrs.get('texcoord'),
                texcoord2=interpolated_attrs.get('texcoord2'),
                normal=interpolated_attrs.get('normal'),
                position=interpolated_attrs.get('position'),
                attributes=interpolated_attrs.get('attributes', {}),
                primitive_id=primitive_id
            )
            self._pixels.append(pixel)

    def _rasterize_triangle(self, triangle: Triangle):
        """Rasterize a triangle using barycentric coordinates"""
        v0_pos = self._get_vertex_position(triangle.v0)
        v1_pos = self._get_vertex_position(triangle.v1)
        v2_pos = self._get_vertex_position(triangle.v2)

        if v0_pos is None or v1_pos is None or v2_pos is None:
            return

        clip_w0 = v0_pos[3] if len(v0_pos) >= 4 else 1.0
        clip_w1 = v1_pos[3] if len(v1_pos) >= 4 else 1.0
        clip_w2 = v2_pos[3] if len(v2_pos) >= 4 else 1.0

        if abs(clip_w0) < 1e-8 or abs(clip_w1) < 1e-8 or abs(clip_w2) < 1e-8:
            return

        screen_v0 = self.config.viewport.transform_to_screen(v0_pos[0], v0_pos[1], clip_w0)
        screen_v1 = self.config.viewport.transform_to_screen(v1_pos[0], v1_pos[1], clip_w1)
        screen_v2 = self.config.viewport.transform_to_screen(v2_pos[0], v2_pos[1], clip_w2)

        min_x = min(screen_v0[0], screen_v1[0], screen_v2[0])
        max_x = max(screen_v0[0], screen_v1[0], screen_v2[0])
        min_y = min(screen_v0[1], screen_v1[1], screen_v2[1])
        max_y = max(screen_v0[1], screen_v1[1], screen_v2[1])

        min_x = max(min_x, int(self.config.viewport.x))
        max_x = min(max_x, int(self.config.viewport.x + self.config.viewport.width - 1))
        min_y = max(min_y, int(self.config.viewport.y))
        max_y = min(max_y, int(self.config.viewport.y + self.config.viewport.height - 1))

        if min_x > max_x or min_y > max_y:
            return

        v0_ndc = [v0_pos[0] / clip_w0, v0_pos[1] / clip_w0, v0_pos[2] / clip_w0]
        v1_ndc = [v1_pos[0] / clip_w1, v1_pos[1] / clip_w1, v1_pos[2] / clip_w1]
        v2_ndc = [v2_pos[0] / clip_w2, v2_pos[1] / clip_w2, v2_pos[2] / clip_w2]

        area = self._edge_function(screen_v0, screen_v1, screen_v2)
        if abs(area) < 1e-10:
            return

        if self._should_cull_triangle(screen_v0, screen_v1, screen_v2):
            return

        for y in range(int(min_y), int(max_y) + 1):
            for x in range(int(min_x), int(max_x) + 1):
                if self.config.scissor_enable and not self._is_in_scissor(x, y):
                    continue

                p = (x, y)
                w0 = self._edge_function(screen_v1, screen_v2, p)
                w1 = self._edge_function(screen_v2, screen_v0, p)
                w2 = self._edge_function(screen_v0, screen_v1, p)

                if (area > 0 and w0 >= 0 and w1 >= 0 and w2 >= 0) or \
                   (area < 0 and w0 <= 0 and w1 <= 0 and w2 <= 0):

                    bary_x = w0 / area
                    bary_y = w1 / area
                    bary_z = w2 / area

                    depth = bary_x * v0_ndc[2] + bary_y * v1_ndc[2] + bary_z * v2_ndc[2]

                    if depth < self.config.viewport.min_depth or depth > self.config.viewport.max_depth:
                        continue

                    interpolated = self._interpolate_with_barycentric(
                        triangle.v0, triangle.v1, triangle.v2,
                        bary_x, bary_y, bary_z,
                        clip_w0, clip_w1, clip_w2
                    )

                    pixel = Pixel(
                        x=x,
                        y=y,
                        depth=depth,
                        color=interpolated.get('Color'),
                        texcoord=interpolated.get('TexCoord'),
                        texcoord2=interpolated.get('TexCoord2'),
                        normal=interpolated.get('Normal'),
                        worldPos=interpolated.get('WorldPos'),
                        attributes=interpolated.get('attributes', {}),
                        primitive_id=triangle.primitive_id
                    )
                    self._pixels.append(pixel)

    def _should_cull_triangle(self, v0: Tuple[int, int], v1: Tuple[int, int], v2: Tuple[int, int]) -> bool:
        """Determine if triangle should be culled based on cull mode"""
        if self.config.cull_mode == CullMode.NONE:
            return False

        edge1_x = v1[0] - v0[0]
        edge1_y = v1[1] - v0[1]
        edge2_x = v2[0] - v0[0]
        edge2_y = v2[1] - v0[1]

        cross_z = edge1_x * edge2_y - edge1_y * edge2_x

        if self.config.cull_mode == CullMode.BACK:
            if self.config.front_face == FrontFace.COUNTER_CLOCKWISE:
                return cross_z < 0
            else:
                return cross_z > 0
        elif self.config.cull_mode == CullMode.FRONT:
            if self.config.front_face == FrontFace.COUNTER_CLOCKWISE:
                return cross_z > 0
            else:
                return cross_z < 0

        return False

    def _edge_function(self, a: Tuple[int, int], b: Tuple[int, int], c: Tuple[int, int]) -> float:
        """Calculate edge function for barycentric coordinates"""
        return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0])

    def _interpolate_with_barycentric(self, v0: Dict[str, Any], v1: Dict[str, Any], v2: Dict[str, Any],
                                      bary_x: float, bary_y: float, bary_z: float,
                                      clip_w0: float = 1.0, clip_w1: float = 1.0, clip_w2: float = 1.0) -> Dict[str, Any]:
        """
        Interpolate vertex attributes using barycentric coordinates with D3D11 perspective-correct interpolation.

        Uses perspective-correct (trilinear) interpolation: attributes are divided by w before interpolation,
        then the result is divided by the interpolated 1/w to get the correct perspective value.
        """
        result = {}

        attr_names = set()
        for v in [v0, v1, v2]:
            if v:
                attr_names.update(v.keys())

        inv_w0 = 1.0 / clip_w0 if abs(clip_w0) > 1e-8 else 0.0
        inv_w1 = 1.0 / clip_w1 if abs(clip_w1) > 1e-8 else 0.0
        inv_w2 = 1.0 / clip_w2 if abs(clip_w2) > 1e-8 else 0.0

        interpolated_inv_w = bary_x * inv_w0 + bary_y * inv_w1 + bary_z * inv_w2
        if abs(interpolated_inv_w) < 1e-8:
            interpolated_inv_w = 1.0

        for attr_name in attr_names:
            attr_lower = attr_name.lower()
            if attr_lower in ['sv_position', 'position', 'pos', 'sv_position']:
                continue

            vals = []
            for v in [v0, v1, v2]:
                if v and attr_name in v:
                    vals.append(v[attr_name])
                else:
                    vals.append(None)

            if all(isinstance(v, list) and v is not None for v in vals):
                min_len = min(len(v) for v in vals if isinstance(v, list))
                interpolated = []

                if attr_lower in ['color', 'normal']:
                    for i in range(min_len):
                        comp0 = vals[0][i] if len(vals[0]) > i else 0.0
                        comp1 = vals[1][i] if len(vals[1]) > i else 0.0
                        comp2 = vals[2][i] if len(vals[2]) > i else 0.0
                        val = bary_x * comp0 + bary_y * comp1 + bary_z * comp2
                        if attr_lower == 'color':
                            val = max(0.0, min(1.0, val))
                        interpolated.append(val)
                else:
                    for i in range(min_len):
                        comp0 = vals[0][i] if len(vals[0]) > i else 0.0
                        comp1 = vals[1][i] if len(vals[1]) > i else 0.0
                        comp2 = vals[2][i] if len(vals[2]) > i else 0.0

                        attr0_normalized = comp0 * inv_w0
                        attr1_normalized = comp1 * inv_w1
                        attr2_normalized = comp2 * inv_w2

                        attr_interpolated_normalized = bary_x * attr0_normalized + bary_y * attr1_normalized + bary_z * attr2_normalized

                        attr_interpolated = attr_interpolated_normalized / interpolated_inv_w
                        interpolated.append(attr_interpolated)

                result[attr_name] = interpolated
            elif all(isinstance(v, (int, float)) and v is not None for v in vals):
                attr0_normalized = vals[0] * inv_w0
                attr1_normalized = vals[1] * inv_w1
                attr2_normalized = vals[2] * inv_w2
                attr_interpolated_normalized = bary_x * attr0_normalized + bary_y * attr1_normalized + bary_z * attr2_normalized
                result[attr_name] = attr_interpolated_normalized / interpolated_inv_w

        return result

    def _interpolate_vertex_attribute(self, vertex: Dict[str, Any], attr_name: str) -> Any:
        """Interpolate a single attribute from vertex"""
        if vertex and attr_name in vertex:
            return vertex[attr_name]
        return None

    def _interpolate_attributes_line(self, v0: Dict[str, Any], v1: Dict[str, Any], t: float,
                                      clip_w0: float = 1.0, clip_w1: float = 1.0) -> Dict[str, Any]:
        """Interpolate attributes for line at parameter t with perspective-correct interpolation"""
        result = {}

        if not v0 or not v1:
            return result

        inv_w0 = 1.0 / clip_w0 if abs(clip_w0) > 1e-8 else 0.0
        inv_w1 = 1.0 / clip_w1 if abs(clip_w1) > 1e-8 else 0.0
        one_minus_t = 1.0 - t
        interpolated_inv_w = one_minus_t * inv_w0 + t * inv_w1
        if abs(interpolated_inv_w) < 1e-8:
            interpolated_inv_w = 1.0

        attr_names = set(v0.keys()) | set(v1.keys())

        for attr_name in attr_names:
            attr_lower = attr_name.lower()
            if attr_lower in ['sv_position', 'position', 'pos', 'sv_position']:
                continue

            val0 = v0.get(attr_name)
            val1 = v1.get(attr_name)

            if val0 is None and val1 is None:
                continue

            if val0 is None:
                val0 = val1
            if val1 is None:
                val1 = val0

            if isinstance(val0, list) and isinstance(val1, list):
                min_len = min(len(val0), len(val1))
                interpolated = []

                if attr_lower in ['color', 'normal']:
                    for i in range(min_len):
                        v0_comp = val0[i] if i < len(val0) else 0.0
                        v1_comp = val1[i] if i < len(val1) else 0.0
                        val = one_minus_t * v0_comp + t * v1_comp
                        if attr_lower == 'color':
                            val = max(0.0, min(1.0, val))
                        interpolated.append(val)
                else:
                    for i in range(min_len):
                        v0_comp = val0[i] if i < len(val0) else 0.0
                        v1_comp = val1[i] if i < len(val1) else 0.0
                        v0_normalized = v0_comp * inv_w0
                        v1_normalized = v1_comp * inv_w1
                        val_normalized = one_minus_t * v0_normalized + t * v1_normalized
                        val = val_normalized / interpolated_inv_w
                        interpolated.append(val)
                result[attr_name] = interpolated
            elif isinstance(val0, (int, float)) and isinstance(val1, (int, float)):
                v0_normalized = val0 * inv_w0
                v1_normalized = val1 * inv_w1
                val_normalized = one_minus_t * v0_normalized + t * v1_normalized
                result[attr_name] = val_normalized / interpolated_inv_w
            elif val0 is not None:
                result[attr_name] = val0

        return result

    def _get_vertex_position(self, vertex: Dict[str, Any]) -> Optional[List[float]]:
        """Extract position from vertex output"""
        if not vertex:
            return None

        pos_candidates = ['sv_position', 'position', 'pos', 'Pos', 'SV_Position']
        for key in pos_candidates:
            if key in vertex and vertex[key]:
                pos = vertex[key]
                if isinstance(pos, list):
                    if len(pos) >= 4:
                        return pos[:4]
                    elif len(pos) == 3:
                        return [pos[0], pos[1], pos[2], 1.0]
                    elif len(pos) == 2:
                        return [pos[0], pos[1], 0.0, 1.0]

        return None

    def _is_in_viewport(self, x: int, y: int) -> bool:
        """Check if pixel is inside viewport"""
        return self.config.viewport.contains(x, y)

    def _is_in_scissor(self, x: int, y: int) -> bool:
        """Check if pixel is inside scissor rect"""
        if not self.config.scissor_enable:
            return True
        return self.config.scissor_rect.contains(x, y)

    def get_primitive_count(self) -> int:
        """Get count of primitives processed"""
        return self._primitive_id_counter

    def get_pixel_count(self) -> int:
        """Get count of pixels generated"""
        return len(self._pixels)

    def load_config_from_pipeline_state_csv(self, csv_path: str) -> Optional[int]:
        """
        Load rasterizer/viewport/scissor config from pipeline_state.csv.
        Returns the primitive topology value if found, else None.

        CSV format:
          Section,Property,Value
          Viewport,X,0.0
          Viewport,Width,640.0
          Topology,Primitive,4
          ...
        """
        try:
            import csv as csv_mod
            with open(csv_path, 'r', encoding='utf-8') as f:
                reader = csv_mod.reader(f)
                rows = list(reader)
        except Exception as e:
            print(f"Warning: Failed to load pipeline_state from {csv_path}: {e}")
            return None

        if not rows or len(rows) < 2:
            return None

        vp = {'x': 0.0, 'y': 0.0, 'width': 800.0, 'height': 600.0,
              'min_depth': 0.0, 'max_depth': 1.0}
        sc = {'x': 0, 'y': 0, 'width': 0, 'height': 0}
        has_viewport = False
        has_scissor = False
        primitive_topology = None

        cull_mode_map = {
            'none': CullMode.NONE, 'front': CullMode.FRONT, 'back': CullMode.BACK,
            '0': CullMode.NONE, '1': CullMode.FRONT, '2': CullMode.BACK,
        }
        fill_mode_map = {
            'point': FillMode.POINT, 'line': FillMode.LINE, 'solid': FillMode.SOLID,
            '0': FillMode.POINT, '1': FillMode.LINE, '2': FillMode.SOLID,
        }

        for row in rows[1:]:
            if len(row) < 3:
                continue
            section = row[0].strip()
            prop = row[1].strip()
            val = row[2].strip().strip('"') if len(row) > 2 else ''

            if section == 'Viewport':
                has_viewport = True
                try:
                    if prop == 'X': vp['x'] = float(val)
                    elif prop == 'Y': vp['y'] = float(val)
                    elif prop == 'Width': vp['width'] = float(val)
                    elif prop == 'Height': vp['height'] = float(val)
                    elif prop == 'MinDepth': vp['min_depth'] = float(val)
                    elif prop == 'MaxDepth': vp['max_depth'] = float(val)
                except ValueError:
                    pass

            elif section == 'Scissor':
                try:
                    if prop == 'X': sc['x'] = int(float(val)); has_scissor = True
                    elif prop == 'Y': sc['y'] = int(float(val))
                    elif prop == 'Width': sc['width'] = int(float(val))
                    elif prop == 'Height': sc['height'] = int(float(val))
                except ValueError:
                    pass

            elif section == 'Rasterizer':
                if prop == 'CullMode':
                    mode = cull_mode_map.get(val.lower())
                    if mode is not None:
                        self.config.cull_mode = mode
                elif prop == 'FillMode':
                    mode = fill_mode_map.get(val.lower())
                    if mode is not None:
                        self.config.fill_mode = mode
                elif prop == 'FrontFace':
                    if 'clockwise' in val.lower() and 'counter' not in val.lower():
                        self.config.front_face = FrontFace.CLOCKWISE
                    else:
                        self.config.front_face = FrontFace.COUNTER_CLOCKWISE

            elif section == 'Topology':
                if prop == 'Primitive':
                    try:
                        primitive_topology = int(val)
                    except ValueError:
                        pass

        if has_viewport:
            self.config.viewport = Viewport(
                x=vp['x'], y=vp['y'],
                width=vp['width'], height=vp['height'],
                min_depth=vp['min_depth'], max_depth=vp['max_depth']
            )

        if has_scissor and (sc['width'] > 0 or sc['height'] > 0):
            self.config.scissor_rect = ScissorRect(
                left=sc['x'], top=sc['y'],
                right=sc['x'] + sc['width'],
                bottom=sc['y'] + sc['height']
            )

        return primitive_topology


def create_default_config() -> Dict[str, Any]:
    """Create default rasterizer configuration"""
    return {
        'cull_mode': 'back',
        'fill_mode': 'solid',
        'front_face': 'counter_clockwise',
        'scissor_enable': False,
        'scissor_rect': {
            'left': 0,
            'top': 0,
            'right': 0,
            'bottom': 0
        },
        'multisample_enable': False,
        'antialiasing_line_enable': False,
        'depth_clip_enable': True,
        'viewport': {
            'x': 0,
            'y': 0,
            'width': 800,
            'height': 600,
            'min_depth': 0.0,
            'max_depth': 1.0
        }
    }


def save_default_config(path: str):
    """Save default rasterizer configuration to JSON file"""
    config = create_default_config()
    with open(path, 'w', encoding='utf-8') as f:
        json.dump(config, f, indent=4)
    print(f"Default rasterizer config saved to {path}")


if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        save_default_config(sys.argv[1])
    else:
        print("Usage: python rasterizer.py <config_output_path.json>")
        print("Creating sample config...")
        save_default_config("rasterizer_config.json")