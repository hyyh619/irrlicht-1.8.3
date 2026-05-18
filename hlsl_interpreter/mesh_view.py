"""
MeshView - 3D Mesh Visualization Tool
类似于RenderDoc的mesh view功能，用于显示顶点组成的mesh
"""
import tkinter as tk
from tkinter import ttk
import threading
import math
import json
import os
from typing import List, Tuple, Optional


D3D_PRIMITIVE_TOPOLOGY_UNDEFINED = 0
D3D_PRIMITIVE_TOPOLOGY_POINTLIST = 1
D3D_PRIMITIVE_TOPOLOGY_LINELIST = 2
D3D_PRIMITIVE_TOPOLOGY_LINESTRIP = 3
D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST = 4
D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP = 5
D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN = 6


class VertexData:
    """顶点数据结构"""
    def __init__(self, position: List[float], normal: List[float] = None, color: List[float] = None):
        self.position = position
        self.normal = normal if normal else [0, 0, 1]
        self.color = color if color else [1, 1, 1, 1]


class MeshView:
    """
    3D Mesh可视化工具
    用于显示顶点着色器输入输出的mesh效果
    """
    def __init__(self, vertices: List[VertexData] = None, primitive_topology: int = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, title: str = "Mesh View"):
        self.vertices = vertices if vertices else []
        self.primitive_topology = primitive_topology
        self.title = title
        self._root = None
        self._canvas = None
        self._running = False
        self._rotation_x = 0 # 30
        self._rotation_y = 0 # 45
        self._scale = 0.2
        self._offset_x = 0
        self._offset_y = 140
        self._last_mouse = None
        self._info_label = None
        self._bounds = None
        self._current_index = 0
        self._is_playing = False
        self._is_paused = False
        self._animation_job = None
        self._animation_interval = 100
        self._play_btn = None
        self._pause_btn = None
        self._next_btn = None
        self._prev_btn = None
        self._step_label = None
        self._show_normals = False
        self._normals_var = None
        self._load_animation_config()

    def _load_animation_config(self):
        """从配置文件加载动画配置"""
        config_path = os.path.join(os.path.dirname(__file__), "animation_config.json")
        if os.path.exists(config_path):
            try:
                with open(config_path, 'r') as f:
                    config = json.load(f)
                    self._animation_interval = config.get("interval_ms", 100)
            except:
                self._animation_interval = 100
        else:
            self._animation_interval = 100

    def set_vertices(self, vertices: List[VertexData]):
        """设置顶点数据"""
        self.vertices = vertices
        self._compute_bounds()

    def set_primitive_topology(self, primitive_topology: int):
        """设置图元拓扑类型"""
        self.primitive_topology = primitive_topology

    def clear(self):
        """清空顶点数据"""
        self.vertices = []
        self._bounds = None
        self._current_index = 0
        self._is_playing = False
        self._is_paused = False
        if self._animation_job:
            self._canvas.after_cancel(self._animation_job)
            self._animation_job = None
        self._update_button_states()

    def add_vertex(self, position: List[float], normal: List[float] = None, color: List[float] = None):
        """添加单个顶点"""
        self.vertices.append(VertexData(position, normal, color))
        self._compute_bounds()

    def set_input_data(self, positions: List[List[float]], normals: List[List[float]] = None, colors: List[List[float]] = None):
        """
        设置输入数据
        positions: 顶点位置列表 [[x,y,z], [x,y,z], ...]
        normals: 法线列表 [[x,y,z], [x,y,z], ...]
        colors: 颜色列表 [[r,g,b,a], [r,g,b,a], ...]
        """
        self.vertices = []
        for i, pos in enumerate(positions):
            normal = normals[i] if normals and i < len(normals) else None
            color = colors[i] if colors and i < len(colors) else None
            self.vertices.append(VertexData(pos, normal, color))
        self._compute_bounds()

    def _compute_bounds(self):
        """计算顶点边界框"""
        if not self.vertices:
            self._bounds = None
            return

        min_x = min_y = min_z = float('inf')
        max_x = max_y = max_z = float('-inf')

        for v in self.vertices:
            x, y, z = v.position[0], v.position[1], v.position[2]
            min_x = min(min_x, x)
            max_x = max(max_x, x)
            min_y = min(min_y, y)
            max_y = max(max_y, y)
            min_z = min(min_z, z)
            max_z = max(max_z, z)

        center = [(min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2]
        size = max(max_x - min_x, max_y - min_y, max_z - min_z)
        if size < 0.001:
            size = 1

        self._bounds = (center, size)

    def _transform_vertex(self, v: List[float]) -> Tuple[float, float, float]:
        """应用旋转变换到顶点"""
        x, y, z = v[0], v[1], v[2]

        ang_x = math.radians(self._rotation_x)
        ang_y = math.radians(self._rotation_y)

        cos_x, sin_x = math.cos(ang_x), math.sin(ang_x)
        cos_y, sin_y = math.cos(ang_y), math.sin(ang_y)

        y1 = y * cos_x - z * sin_x
        z1 = y * sin_x + z * cos_x
        x2 = x * cos_y + z1 * sin_y
        z2 = -x * sin_y + z1 * cos_y

        return x2, y1, z2

    def _project(self, v: Tuple[float, float, float], width: float, height: float) -> Tuple[float, float]:
        """将3D点投影到2D画布"""
        x, y, z = v

        margin = 40
        usable_width = width - 2 * margin
        usable_height = height - 2 * margin

        scale = self._scale * min(usable_width, usable_height) / 2.0

        proj_x = x * scale + width / 2 + self._offset_x
        proj_y = -y * scale + height / 2 + self._offset_y
        return proj_x, proj_y

    def _color_to_hex(self, color: List[float]) -> str:
        """将颜色列表转换为十六进制颜色字符串"""
        if len(color) >= 4:
            r = int(min(255, max(0, color[0] * 255)))
            g = int(min(255, max(0, color[1] * 255)))
            b = int(min(255, max(0, color[2] * 255)))
        elif len(color) >= 3:
            r = int(min(255, max(0, color[0] * 255)))
            g = int(min(255, max(0, color[1] * 255)))
            b = int(min(255, max(0, color[2] * 255)))
        else:
            r = g = b = 200
        return f'#{r:02x}{g:02x}{b:02x}'

    def _draw_mesh_wireframe(self, transformed: list, width: float, height: float):
        """绘制wireframe线框"""
        if self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            for i in range(0, len(transformed) - 2, 3):
                pts = transformed[i:i+3]
                color = self._color_to_hex(pts[0][1])
                proj_pts = [self._project(p, width, height) for p, c in pts]
                self._canvas.create_line(proj_pts[0][0], proj_pts[0][1], proj_pts[1][0], proj_pts[1][1], fill=color, width=1)
                self._canvas.create_line(proj_pts[1][0], proj_pts[1][1], proj_pts[2][0], proj_pts[2][1], fill=color, width=1)
                self._canvas.create_line(proj_pts[2][0], proj_pts[2][1], proj_pts[0][0], proj_pts[0][1], fill=color, width=1)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            for i in range(len(transformed) - 2):
                pts = transformed[i:i+3]
                color = self._color_to_hex(pts[0][1])
                proj_pts = [self._project(p, width, height) for p, c in pts]
                self._canvas.create_line(proj_pts[0][0], proj_pts[0][1], proj_pts[1][0], proj_pts[1][1], fill=color, width=1)
                self._canvas.create_line(proj_pts[1][0], proj_pts[1][1], proj_pts[2][0], proj_pts[2][1], fill=color, width=1)
                self._canvas.create_line(proj_pts[2][0], proj_pts[2][1], proj_pts[0][0], proj_pts[0][1], fill=color, width=1)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINELIST:
            for i in range(0, len(transformed) - 1, 2):
                p1, c1 = transformed[i]
                p2, c2 = transformed[i+1]
                proj1 = self._project(p1, width, height)
                proj2 = self._project(p2, width, height)
                color = self._color_to_hex(c1)
                self._canvas.create_line(proj1[0], proj1[1], proj2[0], proj2[1], fill=color, width=2)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
            for i in range(len(transformed) - 1):
                p1, c1 = transformed[i]
                p2, c2 = transformed[i+1]
                proj1 = self._project(p1, width, height)
                proj2 = self._project(p2, width, height)
                color = self._color_to_hex(c1)
                self._canvas.create_line(proj1[0], proj1[1], proj2[0], proj2[1], fill=color, width=2)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
            for p, c in transformed:
                proj = self._project(p, width, height)
                self._canvas.create_oval(proj[0]-4, proj[1]-4, proj[0]+4, proj[1]+4, fill=self._color_to_hex(c), outline='white')

        if self._show_normals:
            self._draw_normals(transformed, width, height)

    def _draw_normals(self, transformed: list, width: float, height: float):
        """绘制顶点法线向量"""
        if not self.vertices:
            return

        normal_scale = 0.1 * (self._bounds[1] if self._bounds else 1.0)

        for i, (pos, color) in enumerate(transformed):
            if i < len(self.vertices):
                normal = self.vertices[i].normal
                if normal:
                    nx, ny, nz = normal[0], normal[1], normal[2]
                    length = (nx*nx + ny*ny + nz*nz) ** 0.5
                    if length > 0.0001:
                        nx, ny, nz = nx/length, ny/length, nz/length
                    end_pos = (
                        pos[0] + nx * normal_scale,
                        pos[1] + ny * normal_scale,
                        pos[2] + nz * normal_scale
                    )
                    start_proj = self._project(pos, width, height)
                    end_proj = self._project(end_pos, width, height)
                    r = int(min(255, max(0, (nx * 0.5 + 0.5) * 255)))
                    g = int(min(255, max(0, (ny * 0.5 + 0.5) * 255)))
                    b = int(min(255, max(0, (nz * 0.5 + 0.5) * 255)))
                    color_hex = f'#{r:02x}{g:02x}{b:02x}'
                    self._canvas.create_line(start_proj[0], start_proj[1], end_proj[0], end_proj[1],
                                             fill=color_hex, width=1)
                    self._canvas.create_oval(end_proj[0]-2, end_proj[1]-2, end_proj[0]+2, end_proj[1]+2,
                                             fill=color_hex, outline='')

    def _draw_mesh(self):
        """绘制mesh到画布"""
        self._draw_mesh_animated(len(self.vertices))

    def _draw_mesh_animated(self, count: int = None):
        """绘制动画mesh到画布，只渲染前count个元素"""
        if not self._canvas or not self.vertices:
            return

        self._canvas.delete("all")
        width = int(self._canvas.cget('width'))
        height = int(self._canvas.cget('height'))

        transformed = []
        for v in self.vertices:
            p = self._transform_vertex(v.position)
            transformed.append((p, v.color))

        if count is None:
            count = self._current_index + 1

        self._draw_mesh_wireframe(transformed[:count], width, height)
        self._update_info()

    def _update_info(self):
        """更新信息标签"""
        if self._info_label:
            topo_names = {
                D3D_PRIMITIVE_TOPOLOGY_UNDEFINED: "Undefined",
                D3D_PRIMITIVE_TOPOLOGY_POINTLIST: "Point List",
                D3D_PRIMITIVE_TOPOLOGY_LINELIST: "Line List",
                D3D_PRIMITIVE_TOPOLOGY_LINESTRIP: "Line Strip",
                D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST: "Triangle List",
                D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: "Triangle Strip",
                D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN: "Triangle Fan",
            }
            info = f"Vertices: {len(self.vertices)} | Topology: {topo_names.get(self.primitive_topology, 'Unknown')} | Zoom: {self._scale:.2f}x"
            self._info_label.config(text=info)

    def _on_mouse_drag(self, event):
        """处理鼠标拖动旋转"""
        if self._last_mouse:
            dx = event.x - self._last_mouse[0]
            dy = event.y - self._last_mouse[1]
            self._rotation_y += dx * 0.5
            self._rotation_x += dy * 0.5
            self._draw_mesh()
        self._last_mouse = (event.x, event.y)

    def _on_mouse_release(self, event):
        """处理鼠标释放"""
        self._last_mouse = None

    def _on_mouse_wheel(self, event):
        """处理鼠标滚轮缩放"""
        if event.delta > 0:
            self._scale *= 1.1
        else:
            self._scale *= 0.9
        self._scale = max(0.1, min(50, self._scale))
        self._draw_mesh()

    def _on_resize(self, event):
        """处理窗口大小改变"""
        self._draw_mesh()

    def _zoom_in(self):
        """放大"""
        self._scale *= 1.2
        self._scale = min(50, self._scale)
        self._draw_mesh()

    def _zoom_out(self):
        """缩小"""
        self._scale *= 0.8
        self._scale = max(0.1, self._scale)
        self._draw_mesh()

    def _rotate_cw(self):
        """顺时针旋转"""
        self._rotation_y += 15
        self._draw_mesh()

    def _rotate_ccw(self):
        """逆时针旋转"""
        self._rotation_y -= 15
        self._draw_mesh()

    def _rotate_up(self):
        """向上旋转"""
        self._rotation_x -= 15
        self._draw_mesh()

    def _rotate_down(self):
        """向下旋转"""
        self._rotation_x += 15
        self._draw_mesh()

    def _pan_left(self):
        """向左平移"""
        self._offset_x -= 20
        self._draw_mesh()

    def _pan_right(self):
        """向右平移"""
        self._offset_x += 20
        self._draw_mesh()

    def _pan_up(self):
        """向上平移"""
        self._offset_y -= 20
        self._draw_mesh()

    def _pan_down(self):
        """向下平移"""
        self._offset_y += 20
        self._draw_mesh()

    def _reset_view(self):
        """重置视图"""
        self._rotation_x = 30
        self._rotation_y = 45
        self._scale = 1.0
        self._offset_x = 0
        self._offset_y = 0
        self._draw_mesh()

    def _toggle_normals(self):
        """切换法线显示"""
        self._show_normals = self._normals_var.get()
        self._draw_mesh()

    def _play_animation(self):
        """从开头开始播放动画"""
        if not self.vertices:
            return
        self._current_index = 0
        self._is_playing = True
        self._is_paused = False
        self._update_button_states()
        self._run_animation_step()

    def _pause_animation(self):
        """暂停/继续动画"""
        if self._is_paused:
            self._is_paused = False
            self._run_animation_step()
        else:
            self._is_paused = True
        self._update_button_states()

    def _next_step(self):
        """渲染下一个顶点/线"""
        if not self.vertices:
            return
        max_index = len(self.vertices) - 1
        if self._current_index < max_index:
            self._current_index += 1
        self._draw_mesh_animated()
        self._update_step_label()

    def _prev_step(self):
        """回到上一个顶点/线"""
        if not self.vertices:
            return
        if self._current_index > 0:
            self._current_index -= 1
        self._draw_mesh_animated()
        self._update_step_label()

    def _run_animation_step(self):
        """执行动画单步"""
        if not self._is_playing or self._is_paused:
            return
        if self._current_index < len(self.vertices) - 1:
            self._current_index += 1
            self._draw_mesh_animated()
            self._update_step_label()
            self._animation_job = self._canvas.after(self._animation_interval, self._run_animation_step)
        else:
            self._is_playing = False
            self._update_button_states()

    def _update_button_states(self):
        """更新按钮状态"""
        if self._play_btn:
            self._play_btn.config(state=tk.NORMAL if not self._is_playing else tk.DISABLED)
        if self._pause_btn:
            self._pause_btn.config(state=tk.NORMAL if self._is_playing or self._current_index > 0 else tk.DISABLED)
        if self._next_btn:
            self._next_btn.config(state=tk.NORMAL if self._is_paused else tk.DISABLED)
        if self._prev_btn:
            self._prev_btn.config(state=tk.NORMAL if self._is_paused and self._current_index > 0 else tk.DISABLED)

    def _update_step_label(self):
        """更新步骤显示"""
        if self._step_label:
            self._step_label.config(text=f"Step: {self._current_index + 1}/{len(self.vertices)}")

    def show(self, blocking: bool = False):
        """
        显示MeshView窗口
        blocking: 如果为True，则阻塞直到窗口关闭
        """
        if self._root is not None:
            self._root.deiconify()
            return

        self._root = tk.Tk()
        self._root.title(self.title)
        self._root.geometry("800x600")

        main_frame = ttk.Frame(self._root)
        main_frame.pack(fill=tk.BOTH, expand=True)

        controls_frame = ttk.Frame(main_frame)
        controls_frame.pack(side=tk.TOP, fill=tk.X, pady=2)

        ttk.Label(controls_frame, text="Zoom:").pack(side=tk.LEFT, padx=2)
        ttk.Button(controls_frame, text="+", width=3, command=self._zoom_in).pack(side=tk.LEFT, padx=1)
        ttk.Button(controls_frame, text="-", width=3, command=self._zoom_out).pack(side=tk.LEFT, padx=1)

        ttk.Label(controls_frame, text="Rotate:").pack(side=tk.LEFT, padx=5)
        ttk.Button(controls_frame, text="↺", width=3, command=self._rotate_ccw).pack(side=tk.LEFT, padx=1)
        ttk.Button(controls_frame, text="↻", width=3, command=self._rotate_cw).pack(side=tk.LEFT, padx=1)
        ttk.Button(controls_frame, text="↑", width=3, command=self._rotate_up).pack(side=tk.LEFT, padx=1)
        ttk.Button(controls_frame, text="↓", width=3, command=self._rotate_down).pack(side=tk.LEFT, padx=1)

        ttk.Label(controls_frame, text="Pan:").pack(side=tk.LEFT, padx=5)
        ttk.Button(controls_frame, text="◀", width=3, command=self._pan_left).pack(side=tk.LEFT, padx=1)
        ttk.Button(controls_frame, text="▶", width=3, command=self._pan_right).pack(side=tk.LEFT, padx=1)
        ttk.Button(controls_frame, text="▲", width=3, command=self._pan_up).pack(side=tk.LEFT, padx=1)
        ttk.Button(controls_frame, text="▼", width=3, command=self._pan_down).pack(side=tk.LEFT, padx=1)

        ttk.Button(controls_frame, text="Reset", command=self._reset_view).pack(side=tk.LEFT, padx=5)

        self._normals_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(controls_frame, text="Show Normals", variable=self._normals_var,
                        command=self._toggle_normals).pack(side=tk.LEFT, padx=5)

        anim_frame = ttk.Frame(controls_frame)
        anim_frame.pack(side=tk.LEFT, padx=10)
        ttk.Label(anim_frame, text="Animation:").pack(side=tk.LEFT, padx=2)
        self._play_btn = ttk.Button(anim_frame, text="Play", width=5, command=self._play_animation)
        self._play_btn.pack(side=tk.LEFT, padx=1)
        self._pause_btn = ttk.Button(anim_frame, text="Pause", width=5, command=self._pause_animation, state=tk.DISABLED)
        self._pause_btn.pack(side=tk.LEFT, padx=1)
        self._prev_btn = ttk.Button(anim_frame, text="Prev", width=5, command=self._prev_step, state=tk.DISABLED)
        self._prev_btn.pack(side=tk.LEFT, padx=1)
        self._next_btn = ttk.Button(anim_frame, text="Next", width=5, command=self._next_step, state=tk.DISABLED)
        self._next_btn.pack(side=tk.LEFT, padx=1)
        self._step_label = ttk.Label(anim_frame, text="Step: 0/0", width=12)
        self._step_label.pack(side=tk.LEFT, padx=5)

        ttk.Button(controls_frame, text="Close", command=self._root.destroy).pack(side=tk.RIGHT, padx=5)

        self._canvas = tk.Canvas(main_frame, bg="black", width=780, height=520)
        self._canvas.pack(fill=tk.BOTH, expand=True)

        self._canvas.bind("<Button-1>", lambda e: self._on_mouse_drag(e))
        self._canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag(e))
        self._canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
        self._canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel(e))
        self._root.bind("<Configure>", lambda e: self._on_resize(e))

        self._info_label = ttk.Label(main_frame, text="Vertices: 0 | Topology: None", font=("Consolas", 10))
        self._info_label.pack(side=tk.BOTTOM, fill=tk.X, pady=2)

        self._draw_mesh()
        self._update_step_label()
        self._running = True

        if blocking:
            self._root.mainloop()
        else:
            threading.Thread(target=self._root.mainloop, daemon=True).start()

    def hide(self):
        """隐藏窗口"""
        if self._root:
            self._root.withdraw()

    def update(self):
        """更新显示"""
        if self._root and self._running:
            self._draw_mesh()

    def is_visible(self) -> bool:
        """检查窗口是否可见"""
        return self._root is not None and self._running

    def close(self):
        """关闭窗口"""
        self._running = False
        if self._animation_job:
            self._canvas.after_cancel(self._animation_job)
            self._animation_job = None
        if self._root:
            try:
                self._root.quit()
                self._root.destroy()
            except:
                pass
            self._root = None
            self._canvas = None