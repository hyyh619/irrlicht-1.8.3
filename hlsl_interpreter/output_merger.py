import json
from typing import List, Optional, Dict, Any
from dataclasses import dataclass, field
from enum import Enum

from pixel import Pixel


class ComparisonFunc(Enum):
    NEVER = 0
    LESS = 1
    EQUAL = 2
    LESS_EQUAL = 3
    GREATER = 4
    NOT_EQUAL = 5
    GREATER_EQUAL = 6
    ALWAYS = 7


class StencilOp(Enum):
    KEEP = 0
    ZERO = 1
    REPLACE = 2
    INCR_SAT = 3
    DECR_SAT = 4
    INVERT = 5
    INCR = 6
    DECR = 7


class StencilFunc(Enum):
    NEVER = ComparisonFunc.NEVER
    LESS = ComparisonFunc.LESS
    EQUAL = ComparisonFunc.EQUAL
    LESS_EQUAL = ComparisonFunc.LESS_EQUAL
    GREATER = ComparisonFunc.GREATER
    NOT_EQUAL = ComparisonFunc.NOT_EQUAL
    GREATER_EQUAL = ComparisonFunc.GREATER_EQUAL
    ALWAYS = ComparisonFunc.ALWAYS


@dataclass
class StencilOpDesc:
    fail_op: StencilOp = StencilOp.KEEP
    pass_op: StencilOp = StencilOp.KEEP
    depth_fail_op: StencilOp = StencilOp.KEEP
    func: StencilFunc = StencilFunc.ALWAYS


@dataclass
class DepthStencilOpDesc:
    depth_enable: bool = False
    depth_write_mask: bool = False
    depth_func: ComparisonFunc = ComparisonFunc.LESS
    depth_init_value: float = 1.0
    stencil_enable: bool = False
    stencil_read_mask: int = 0xFF
    stencil_write_mask: int = 0xFF
    stencil_init_value: int = 0
    front_face: StencilOpDesc = None
    back_face: StencilOpDesc = None

    def __post_init__(self):
        if self.front_face is None:
            self.front_face = StencilOpDesc()
        if self.back_face is None:
            self.back_face = StencilOpDesc()


class Depth:
    """
    Depth/Stencil state for output merger stage in 3D rendering.
    Handles depth testing and stencil operations on pixels.
    """

    def __init__(self, config_path: str = None):
        self.config = DepthStencilOpDesc()
        self._depth_buffer: Dict[tuple, float] = {}
        self._stencil_buffer: Dict[tuple, int] = {}
        if config_path:
            self.load_config(config_path)

    def load_config(self, config_path: str):
        """Load depth/stencil configuration from JSON file"""
        try:
            with open(config_path, 'r', encoding='utf-8') as f:
                config_data = json.load(f)

            self.config.depth_enable = config_data.get('DepthEnable', False)
            self.config.depth_write_mask = config_data.get('DepthWriteMask', False)
            self.config.depth_init_value = config_data.get('DepthInitValue', 1.0)

            depth_func_map = {
                'never': ComparisonFunc.NEVER,
                'less': ComparisonFunc.LESS,
                'equal': ComparisonFunc.EQUAL,
                'less_equal': ComparisonFunc.LESS_EQUAL,
                'greater': ComparisonFunc.GREATER,
                'not_equal': ComparisonFunc.NOT_EQUAL,
                'greater_equal': ComparisonFunc.GREATER_EQUAL,
                'always': ComparisonFunc.ALWAYS
            }
            depth_func_str = config_data.get('DepthFunc', 'less').lower()
            self.config.depth_func = depth_func_map.get(depth_func_str, ComparisonFunc.LESS)

            self.config.stencil_enable = config_data.get('StencilEnable', False)
            self.config.stencil_read_mask = config_data.get('StencilReadMask', 0xFF)
            self.config.stencil_write_mask = config_data.get('StencilWriteMask', 0xFF)
            self.config.stencil_init_value = config_data.get('StencilInitValue', 0)

            stencil_op_map = {
                'keep': StencilOp.KEEP,
                'zero': StencilOp.ZERO,
                'replace': StencilOp.REPLACE,
                'incr_sat': StencilOp.INCR_SAT,
                'decr_sat': StencilOp.DECR_SAT,
                'invert': StencilOp.INVERT,
                'incr': StencilOp.INCR,
                'decr': StencilOp.DECR
            }

            stencil_func_map = {
                'never': StencilFunc.NEVER,
                'less': StencilFunc.LESS,
                'equal': StencilFunc.EQUAL,
                'less_equal': StencilFunc.LESS_EQUAL,
                'greater': StencilFunc.GREATER,
                'not_equal': StencilFunc.NOT_EQUAL,
                'greater_equal': StencilFunc.GREATER_EQUAL,
                'always': StencilFunc.ALWAYS
            }

            if 'FrontFace' in config_data:
                ff = config_data['FrontFace']
                self.config.front_face = StencilOpDesc(
                    fail_op=stencil_op_map.get(ff.get('StencilFailOp', 'keep'), StencilOp.KEEP),
                    pass_op=stencil_op_map.get(ff.get('StencilPassOp', 'keep'), StencilOp.KEEP),
                    depth_fail_op=stencil_op_map.get(ff.get('StencilDepthFailOp', 'keep'), StencilOp.KEEP),
                    func=stencil_func_map.get(ff.get('StencilFunc', 'always'), StencilFunc.ALWAYS)
                )
            else:
                self.config.front_face = StencilOpDesc()

            if 'BackFace' in config_data:
                bf = config_data['BackFace']
                self.config.back_face = StencilOpDesc(
                    fail_op=stencil_op_map.get(bf.get('StencilFailOp', 'keep'), StencilOp.KEEP),
                    pass_op=stencil_op_map.get(bf.get('StencilPassOp', 'keep'), StencilOp.KEEP),
                    depth_fail_op=stencil_op_map.get(bf.get('StencilDepthFailOp', 'keep'), StencilOp.KEEP),
                    func=stencil_func_map.get(bf.get('StencilFunc', 'always'), StencilFunc.ALWAYS)
                )
            else:
                self.config.back_face = StencilOpDesc()

        except Exception as e:
            print(f"Warning: Failed to load depth/stencil config from {config_path}: {e}")

    def clear_buffers(self):
        """Clear depth and stencil buffers"""
        self._depth_buffer.clear()
        self._stencil_buffer.clear()

    def execute(self, pixels: List[Pixel], early_z: bool = True) -> List[Pixel]:
        """
        Execute depth/stencil operations on pixels.

        Args:
            pixels: List of Pixel objects to process
            early_z: If True, pixels are from rasterizer (early-z mode).
                    If False, pixels are from executePS (late-z mode).

        Returns:
            List of Pixel objects that passed depth/stencil tests
        """
        if not pixels:
            return []

        depth_enabled = self.config.depth_enable
        stencil_enabled = self.config.stencil_enable

        if not depth_enabled and not stencil_enabled:
            return list(pixels)

        output_pixels = []

        for pixel in pixels:
            if self._pass_depth_stencil_test(pixel):
                if depth_enabled and self.config.depth_write_mask:
                    self._write_to_depth_buffer(pixel.x, pixel.y, pixel.depth)
                if stencil_enabled:
                    self._write_to_stencil_buffer(pixel.x, pixel.y, self._get_stencil_ref())
                output_pixels.append(pixel)
            else:
                if stencil_enabled:
                    self._handle_stencil_fail(pixel)

        return output_pixels

    def _pass_depth_stencil_test(self, pixel: Pixel) -> bool:
        """Check if pixel passes depth and stencil tests"""
        x, y = pixel.x, pixel.y
        current_depth = pixel.depth

        if self.config.stencil_enable:
            stencil_pass = self._test_stencil(pixel, x, y)
            if not stencil_pass:
                return False

        if self.config.depth_enable:
            if (x, y) in self._depth_buffer:
                stored_depth = self._depth_buffer[(x, y)]
            else:
                stored_depth = self.config.depth_init_value
            if not self._test_depth(stored_depth, current_depth):
                return False

        return True

    def _test_depth(self, stored_depth: float, current_depth: float) -> bool:
        """Test current depth against stored depth"""
        func = self.config.depth_func

        if func == ComparisonFunc.NEVER:
            return False
        elif func == ComparisonFunc.LESS:
            return current_depth < stored_depth
        elif func == ComparisonFunc.EQUAL:
            return abs(current_depth - stored_depth) < 1e-8
        elif func == ComparisonFunc.LESS_EQUAL:
            return current_depth <= stored_depth
        elif func == ComparisonFunc.GREATER:
            return current_depth > stored_depth
        elif func == ComparisonFunc.NOT_EQUAL:
            return abs(current_depth - stored_depth) >= 1e-8
        elif func == ComparisonFunc.GREATER_EQUAL:
            return current_depth >= stored_depth
        elif func == ComparisonFunc.ALWAYS:
            return True

        return True

    def _test_stencil(self, pixel: Pixel, x: int, y: int) -> bool:
        """Test stencil value"""
        stencil_ref = self._get_stencil_ref()
        if (x, y) in self._stencil_buffer:
            stored_stencil = self._stencil_buffer[(x, y)]
        else:
            stored_stencil = self.config.stencil_init_value
        mask = self.config.stencil_read_mask

        func = self.config.front_face.func

        return self._compare_stencil(stored_stencil & mask, stencil_ref & mask, func)

    def _compare_stencil(self, lhs: int, rhs: int, func: StencilFunc) -> bool:
        """Compare stencil values based on function"""
        if func == StencilFunc.NEVER:
            return False
        elif func == StencilFunc.LESS:
            return lhs < rhs
        elif func == StencilFunc.EQUAL:
            return lhs == rhs
        elif func == StencilFunc.LESS_EQUAL:
            return lhs <= rhs
        elif func == StencilFunc.GREATER:
            return lhs > rhs
        elif func == StencilFunc.NOT_EQUAL:
            return lhs != rhs
        elif func == StencilFunc.GREATER_EQUAL:
            return lhs >= rhs
        elif func == StencilFunc.ALWAYS:
            return True

        return True

    def _get_stencil_ref(self) -> int:
        """Get stencil reference value from config"""
        return self.config.stencil_init_value

    def _write_to_depth_buffer(self, x: int, y: int, depth: float):
        """Write depth value to depth buffer"""
        self._depth_buffer[(x, y)] = depth

    def _write_to_stencil_buffer(self, x: int, y: int, stencil_ref: int):
        """Write stencil value to stencil buffer"""
        self._stencil_buffer[(x, y)] = stencil_ref & self.config.stencil_write_mask

    def _handle_stencil_fail(self, pixel: Pixel):
        """Handle stencil test failure - apply fail operation"""
        pass


def create_default_depth_config() -> Dict[str, Any]:
    """Create default depth/stencil configuration"""
    return {
        'DepthEnable': True,
        'DepthWriteMask': 'D3D11_DEPTH_WRITE_MASK_ALL',
        'DepthFunc': 'D3D11_COMPARISON_LESS',
        'DepthInitValue': 1.0,
        'StencilEnable': False,
        'StencilReadMask': 255,
        'StencilWriteMask': 255,
        'StencilInitValue': 0,
        'FrontFace': {
            'StencilFailOp': 'D3D11_STENCIL_OP_KEEP',
            'StencilDepthFailOp': 'D3D11_STENCIL_OP_KEEP',
            'StencilPassOp': 'D3D11_STENCIL_OP_KEEP',
            'StencilFunc': 'D3D11_COMPARISON_ALWAYS'
        },
        'BackFace': {
            'StencilFailOp': 'D3D11_STENCIL_OP_KEEP',
            'StencilDepthFailOp': 'D3D11_STENCIL_OP_KEEP',
            'StencilPassOp': 'D3D11_STENCIL_OP_KEEP',
            'StencilFunc': 'D3D11_COMPARISON_ALWAYS'
        }
    }


def save_default_config(path: str):
    """Save default depth/stencil configuration to JSON file"""
    config = create_default_depth_config()
    with open(path, 'w', encoding='utf-8') as f:
        json.dump(config, f, indent=4)
    print(f"Default depth/stencil config saved to {path}")


if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        save_default_config(sys.argv[1])
    else:
        print("Usage: python output_merger.py <config_output_path.json>")
        print("Creating sample config...")
        save_default_config("depth_stencil_config.json")