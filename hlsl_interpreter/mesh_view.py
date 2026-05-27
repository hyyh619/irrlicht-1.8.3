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

from d3d import (
    D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
    D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
    D3D_PRIMITIVE_TOPOLOGY_LINELIST,
    D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN
)

MESH_VIEW_ROTATION_INIT_X = 0
MESH_VIEW_ROTATION_INIT_Y = 0
MESH_VIEW_SCALE_INIT = 0.2
MESH_VIEW_MIN_SCALE = 0.01
MESH_VIEW_MAX_SCALE = 50
MESH_VIEW_OFFSET_X = 0
MESH_VIEW_OFFSET_Y = 180


class VertexData:
    """顶点数据结构"""
    def __init__(self, position: List[float], normal: List[float] = None, color: List[float] = None,
                 tex_coord: List[float] = None, tex_coord2: List[float] = None):
        self.position = position
        self.normal = normal if normal else [0, 0, 1]
        self.color = color if color else [1, 1, 1, 1]
        self.tex_coord = tex_coord if tex_coord else [0, 0]
        self.tex_coord2 = tex_coord2 if tex_coord2 else [0, 0]


class MeshView:
    """
    3D Mesh可视化工具
    用于显示顶点着色器输入输出的mesh效果
    支持双窗口显示：左侧为输入顶点，右侧为输出结果
    """
    def __init__(self, vertices: List[VertexData] = None, primitive_topology: int = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, title: str = "Mesh View"):
        self.input_vertices = vertices if vertices else []
        self.output_vertices = []
        self.primitive_topology = primitive_topology
        self.title = title
        self._root = None
        self._input_canvas = None
        self._output_canvas = None
        self._running = False
        self._input_rotation_x = MESH_VIEW_ROTATION_INIT_X
        self._input_rotation_y = MESH_VIEW_ROTATION_INIT_Y
        self._output_rotation_x = MESH_VIEW_ROTATION_INIT_X
        self._output_rotation_y = MESH_VIEW_ROTATION_INIT_Y
        self._input_scale = MESH_VIEW_SCALE_INIT
        self._output_scale = MESH_VIEW_SCALE_INIT
        self._input_offset_x = MESH_VIEW_OFFSET_X
        self._input_offset_y = MESH_VIEW_OFFSET_Y
        self._output_offset_x = MESH_VIEW_OFFSET_X
        self._output_offset_y = MESH_VIEW_OFFSET_Y
        self._last_mouse = None
        self._info_label = None
        self._input_bounds = None
        self._output_bounds = None
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
        self._active_view_var = None
        self._load_animation_config()
        self._gui_thread = None
        self._gui_thread_alive = True
        self._gui_ready_event = threading.Event()
        self._selected_input_vertex_index = None
        self._selected_output_vertex_index = None
        self._vertex_info_panel = None
        self._input_vertex_projections = []
        self._output_vertex_projections = []
        self._hlsl_interpreter = None
        self._hlsl_main_func = "main"
        self._hlsl_input_struct = "VS_INPUT"
        self._re_execute_btn = None
        self._vertex_shader_log = []
        self._vertex_shader_log_text = None
        self._vertex_info_font_size = 12
        self._shader_log_font_size = 12
        self._vertex_info_scroll_y = None
        self._vertex_info_inner_frame = None
        self._rasterizer_pixels = []  # pixels from rasterizer output
        self._rasterizer_canvas = None
        self._pixel_shader_canvas = None
        self._output_merger_canvas = None
        self._output_merger_pixels = []  # pixels after depth/stencil processing
        self._rasterizer_scale = 1.0
        self._rasterizer_offset_x = 0
        self._rasterizer_offset_y = 0
        self._start_gui_thread()

    @property
    def vertices(self):
        return self.input_vertices

    @vertices.setter
    def vertices(self, value):
        self.input_vertices = value
        self._compute_input_bounds()

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

    def _start_gui_thread(self):
        """启动单独的GUI线程"""
        self._gui_thread = threading.Thread(target=self._gui_thread_run, daemon=True)
        self._gui_thread.start()

    def _gui_thread_run(self):
        """在单独线程中运行tkinter主循环"""
        self._root = tk.Tk()
        self._root.title(self.title)
        self._root.geometry("1700x700")
        self._setup_ui()
        self._gui_ready_event.set()
        self._root.mainloop()

    def _setup_ui(self):
        """设置UI组件（在GUI线程中调用）"""
        self._active_view_var = tk.BooleanVar(value=True)

        main_frame = ttk.Frame(self._root)
        main_frame.pack(fill=tk.BOTH, expand=True)

        controls_frame = ttk.Frame(main_frame)
        controls_frame.pack(side=tk.TOP, fill=tk.X, pady=2)

        ttk.Label(controls_frame, text="Active:").pack(side=tk.LEFT, padx=2)
        ttk.Radiobutton(controls_frame, text="Input", variable=self._active_view_var, value=True).pack(side=tk.LEFT, padx=2)
        ttk.Radiobutton(controls_frame, text="Output", variable=self._active_view_var, value=False).pack(side=tk.LEFT, padx=2)

        ttk.Separator(controls_frame, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=5)

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

        ttk.Button(controls_frame, text="Close", command=self.hide).pack(side=tk.RIGHT, padx=5)

        canvas_frame = ttk.Frame(main_frame)
        canvas_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, pady=2)

        self._paned_window = ttk.PanedWindow(canvas_frame, orient=tk.HORIZONTAL)
        self._paned_window.pack(fill=tk.BOTH, expand=True)

        left_paned = ttk.PanedWindow(self._paned_window, orient=tk.VERTICAL)
        self._paned_window.add(left_paned)

        input_frame = ttk.LabelFrame(left_paned, text="Input Vertices", padding=5)
        left_paned.add(input_frame)
        self._input_canvas = tk.Canvas(input_frame, bg="#1a1a2e", width=1000, height=320)
        self._input_canvas.pack(fill=tk.BOTH, expand=True)

        output_frame = ttk.LabelFrame(left_paned, text="Output", padding=5)
        left_paned.add(output_frame)

        self._output_notebook = ttk.Notebook(output_frame)
        self._output_notebook.pack(fill=tk.BOTH, expand=True)

        vs_result_frame = ttk.Frame(self._output_notebook)
        self._output_notebook.add(vs_result_frame, text="VS Result")
        self._output_canvas = tk.Canvas(vs_result_frame, bg="#1a1a2e", width=1000, height=320)
        self._output_canvas.pack(fill=tk.BOTH, expand=True)

        rasterizer_frame = ttk.Frame(self._output_notebook)
        self._output_notebook.add(rasterizer_frame, text="Rasterizer")
        self._rasterizer_canvas = tk.Canvas(rasterizer_frame, bg="#1a1a2e", width=1000, height=320)
        self._rasterizer_canvas.pack(fill=tk.BOTH, expand=True)

        pixel_shader_frame = ttk.Frame(self._output_notebook)
        self._output_notebook.add(pixel_shader_frame, text="Pixel Shader")
        self._pixel_shader_canvas = tk.Canvas(pixel_shader_frame, bg="#1a1a2e", width=1000, height=320)
        self._pixel_shader_canvas.pack(fill=tk.BOTH, expand=True)

        output_merger_frame = ttk.Frame(self._output_notebook)
        self._output_notebook.add(output_merger_frame, text="Output Merger")
        self._output_merger_canvas = tk.Canvas(output_merger_frame, bg="#1a1a2e", width=1000, height=320)
        self._output_merger_canvas.pack(fill=tk.BOTH, expand=True)

        right_paned = ttk.PanedWindow(self._paned_window, orient=tk.VERTICAL)
        self._paned_window.add(right_paned)

        info_frame = ttk.LabelFrame(right_paned, text="Selected Vertex Info", padding=5)
        right_paned.add(info_frame)

        info_inner = ttk.Frame(info_frame)
        info_inner.pack(fill=tk.BOTH, expand=True)

        btn_frame = ttk.Frame(info_inner)
        btn_frame.pack(side=tk.TOP, fill=tk.X, pady=2)
        self._re_execute_btn = ttk.Button(btn_frame, text="Re-execute Vertex Shader", command=self._on_re_execute_vertex, state=tk.DISABLED)
        self._re_execute_btn.pack(side=tk.LEFT, padx=2)
        ttk.Button(btn_frame, text="Clear Log", command=self._on_clear_shader_log).pack(side=tk.LEFT, padx=2)
        ttk.Label(btn_frame, text="Info Font:").pack(side=tk.LEFT, padx=(10, 2))
        self._info_font_size_var = tk.IntVar(value=self._vertex_info_font_size)
        info_font_spin = ttk.Spinbox(btn_frame, from_=6, to=24, width=3, textvariable=self._info_font_size_var, command=self._on_info_font_size_changed)
        info_font_spin.pack(side=tk.LEFT, padx=2)
        ttk.Label(btn_frame, text="Log Font:").pack(side=tk.LEFT, padx=(10, 2))
        self._log_font_size_var = tk.IntVar(value=self._shader_log_font_size)
        log_font_spin = ttk.Spinbox(btn_frame, from_=6, to=24, width=3, textvariable=self._log_font_size_var, command=self._on_log_font_size_changed)
        log_font_spin.pack(side=tk.LEFT, padx=2)

        info_canvas_frame = ttk.Frame(info_inner)
        info_canvas_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        self._vertex_info_scroll_y = ttk.Scrollbar(info_canvas_frame)
        self._vertex_info_scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
        self._vertex_info_inner_frame = tk.Canvas(info_canvas_frame, bg="#1a1a2e", highlightthickness=0, yscrollcommand=self._vertex_info_scroll_y.set)
        self._vertex_info_inner_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self._vertex_info_scroll_y.config(command=self._vertex_info_inner_frame.yview)
        self._vertex_info_panel = self._vertex_info_inner_frame

        log_label_frame = ttk.LabelFrame(right_paned, text="Vertex Shader Execution Log", padding=2)
        right_paned.add(log_label_frame)
        log_scroll = ttk.Scrollbar(log_label_frame)
        log_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        self._vertex_shader_log_text = tk.Text(log_label_frame, bg="#0d0d1a", fg="#00ff00", font=("Consolas", self._shader_log_font_size), height=8, wrap=tk.WORD, yscrollcommand=log_scroll.set)
        self._vertex_shader_log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        log_scroll.config(command=self._vertex_shader_log_text.yview)

        self._input_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_input(e))
        self._input_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_input(e))
        self._input_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
        self._input_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_input(e))
        self._input_canvas.bind("<Button-3>", lambda e: self._on_right_click_input(e))

        self._output_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_output(e))
        self._output_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_output(e))
        self._output_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
        self._output_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_output(e))
        self._output_canvas.bind("<Button-3>", lambda e: self._on_right_click_output(e))

        if self._rasterizer_canvas:
            self._rasterizer_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_rasterizer(e))
            self._rasterizer_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_rasterizer(e))
            self._rasterizer_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
            self._rasterizer_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_rasterizer(e))

        if self._pixel_shader_canvas:
            self._pixel_shader_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_pixel_shader(e))
            self._pixel_shader_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_pixel_shader(e))
            self._pixel_shader_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
            self._pixel_shader_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_pixel_shader(e))

        if self._output_merger_canvas:
            self._output_merger_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_output_merger(e))
            self._output_merger_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_output_merger(e))
            self._output_merger_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
            self._output_merger_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_output_merger(e))

        self._root.bind("<Configure>", lambda e: self._on_resize(e))

        self._info_label = ttk.Label(main_frame, text="Input: 0 vertices | Output: 0 vertices | Topology: None", font=("Consolas", 10))
        self._info_label.pack(side=tk.BOTTOM, fill=tk.X, pady=2)

        self._draw_mesh()
        self._update_step_label()
        self._running = True

    def set_vertices(self, vertices: List[VertexData]):
        """设置顶点数据（输入顶点）"""
        self.input_vertices = vertices
        self._compute_input_bounds()

    def set_input_vertices(self, vertices: List[VertexData]):
        """设置输入顶点数据"""
        self.input_vertices = vertices
        self._compute_input_bounds()

    def set_output_vertices(self, vertices: List[VertexData]):
        """设置输出顶点数据（executeVS结果）"""
        self.output_vertices = vertices
        self._compute_output_bounds()

    def set_primitive_topology(self, primitive_topology: int):
        """设置图元拓扑类型"""
        self.primitive_topology = primitive_topology

    def clear(self):
        """清空顶点数据"""
        self.input_vertices = []
        self.output_vertices = []
        self._input_bounds = None
        self._output_bounds = None
        self._current_index = 0
        self._is_playing = False
        self._is_paused = False
        if self._animation_job and self._root:
            self._root.after_cancel(self._animation_job)
            self._animation_job = None
        self._update_button_states()

    def add_vertex(self, position: List[float], normal: List[float] = None, color: List[float] = None,
                   tex_coord: List[float] = None, tex_coord2: List[float] = None):
        """添加单个顶点到输入"""
        self.input_vertices.append(VertexData(position, normal, color, tex_coord, tex_coord2))
        self._compute_input_bounds()

    def add_input_vertex(self, position: List[float], normal: List[float] = None, color: List[float] = None,
                         tex_coord: List[float] = None, tex_coord2: List[float] = None):
        """添加单个输入顶点"""
        self.input_vertices.append(VertexData(position, normal, color, tex_coord, tex_coord2))
        self._compute_input_bounds()

    def add_output_vertex(self, position: List[float], normal: List[float] = None, color: List[float] = None,
                          tex_coord: List[float] = None, tex_coord2: List[float] = None):
        """添加单个输出顶点"""
        self.output_vertices.append(VertexData(position, normal, color, tex_coord, tex_coord2))
        self._compute_output_bounds()

    def set_input_data(self, positions: List[List[float]], normals: List[List[float]] = None,
                       colors: List[List[float]] = None, tex_coords: List[List[float]] = None,
                       tex_coords2: List[List[float]] = None):
        """
        设置输入数据
        positions: 顶点位置列表 [[x,y,z], [x,y,z], ...]
        normals: 法线列表 [[x,y,z], [x,y,z], ...]
        colors: 颜色列表 [[r,g,b,a], [r,g,b,a], ...]
        tex_coords: 纹理坐标列表 [[u,v], [u,v], ...]
        tex_coords2: 第二纹理坐标列表 [[u,v], [u,v], ...]
        """
        self.input_vertices = []
        for i, pos in enumerate(positions):
            normal = normals[i] if normals and i < len(normals) else None
            color = colors[i] if colors and i < len(colors) else None
            tex_coord = tex_coords[i] if tex_coords and i < len(tex_coords) else None
            tex_coord2 = tex_coords2[i] if tex_coords2 and i < len(tex_coords2) else None
            self.input_vertices.append(VertexData(pos, normal, color, tex_coord, tex_coord2))
        self._compute_input_bounds()

    def set_output_data(self, positions: List[List[float]], normals: List[List[float]] = None,
                        colors: List[List[float]] = None, tex_coords: List[List[float]] = None,
                        tex_coords2: List[List[float]] = None):
        """
        设置输出数据（executeVS结果）
        positions: 顶点位置列表 [[x,y,z], [x,y,z], ...]
        normals: 法线列表 [[x,y,z], [x,y,z], ...]
        colors: 颜色列表 [[r,g,b,a], [r,g,b,a], ...]
        tex_coords: 纹理坐标列表 [[u,v], [u,v], ...]
        tex_coords2: 第二纹理坐标列表 [[u,v], [u,v], ...]
        """
        self.output_vertices = []
        for i, pos in enumerate(positions):
            normal = normals[i] if normals and i < len(normals) else None
            color = colors[i] if colors and i < len(colors) else None
            tex_coord = tex_coords[i] if tex_coords and i < len(tex_coords) else None
            tex_coord2 = tex_coords2[i] if tex_coords2 and i < len(tex_coords2) else None
            self.output_vertices.append(VertexData(pos, normal, color, tex_coord, tex_coord2))
        self._compute_output_bounds()

    def set_rasterizer_pixels(self, pixels: List):
        """
        设置光栅化后的像素数据
        pixels: Pixel对象列表 from Rasterizer.rasterize()
        """
        self._rasterizer_pixels = pixels

    def set_pixel_shader_output(self, pixels: List):
        """
        设置Pixel Shader输出后的像素数据（更新ps_output_color）
        pixels: Pixel对象列表，executePS执行后的像素列表
        """
        self._rasterizer_pixels = pixels

    def set_output_merger_pixels(self, pixels: List):
        """
        设置Output Merger阶段的像素数据（经过depth/stencil处理后的像素）
        pixels: Pixel对象列表，Depth处理后的像素列表
        """
        self._output_merger_pixels = pixels

    def _compute_input_bounds(self):
        """计算输入顶点边界框"""
        if not self.input_vertices:
            self._input_bounds = None
            return

        min_x = min_y = min_z = float('inf')
        max_x = max_y = max_z = float('-inf')

        for v in self.input_vertices:
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

        self._input_bounds = (center, size)

    def _compute_output_bounds(self):
        """计算输出顶点边界框"""
        if not self.output_vertices:
            self._output_bounds = None
            return

        min_x = min_y = min_z = float('inf')
        max_x = max_y = max_z = float('-inf')

        for v in self.output_vertices:
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

        self._output_bounds = (center, size)

    def _transform_vertex_input(self, v: List[float]) -> Tuple[float, float, float]:
        """应用旋转变换到输入顶点"""
        x, y, z = v[0], v[1], v[2]

        ang_x = math.radians(self._input_rotation_x)
        ang_y = math.radians(self._input_rotation_y)

        cos_x, sin_x = math.cos(ang_x), math.sin(ang_x)
        cos_y, sin_y = math.cos(ang_y), math.sin(ang_y)

        y1 = y * cos_x - z * sin_x
        z1 = y * sin_x + z * cos_x
        x2 = x * cos_y + z1 * sin_y
        z2 = -x * sin_y + z1 * cos_y

        return x2, y1, z2

    def _transform_vertex_output(self, v: List[float]) -> Tuple[float, float, float]:
        """应用旋转变换到输出顶点"""
        x, y, z = v[0], v[1], v[2]

        ang_x = math.radians(self._output_rotation_x)
        ang_y = math.radians(self._output_rotation_y)

        cos_x, sin_x = math.cos(ang_x), math.sin(ang_x)
        cos_y, sin_y = math.cos(ang_y), math.sin(ang_y)

        y1 = y * cos_x - z * sin_x
        z1 = y * sin_x + z * cos_x
        x2 = x * cos_y + z1 * sin_y
        z2 = -x * sin_y + z1 * cos_y

        return x2, y1, z2

    def _project_input(self, v: Tuple[float, float, float], width: float, height: float) -> Tuple[float, float]:
        """将3D点投影到输入画布"""
        x, y, z = v

        margin = 40
        usable_width = width - 2 * margin
        usable_height = height - 2 * margin

        scale = self._input_scale * min(usable_width, usable_height) / 2.0

        proj_x = x * scale + width / 2 + self._input_offset_x
        proj_y = -y * scale + height / 2 + self._input_offset_y
        return proj_x, proj_y

    def _project_output(self, v: Tuple[float, float, float], width: float, height: float) -> Tuple[float, float]:
        """将3D点投影到输出画布"""
        x, y, z = v

        margin = 40
        usable_width = width - 2 * margin
        usable_height = height - 2 * margin

        scale = self._output_scale * min(usable_width, usable_height) / 2.0

        proj_x = x * scale + width / 2 + self._output_offset_x
        proj_y = -y * scale + height / 2 + self._output_offset_y
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

    def _draw_mesh_wireframe_input(self, transformed: list, width: float, height: float):
        """绘制输入wireframe线框"""
        if self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            for i in range(0, len(transformed) - 2, 3):
                pts = transformed[i:i+3]
                color = self._color_to_hex(pts[0][1])
                proj_pts = [self._project_input(p, width, height) for p, c in pts]
                self._input_canvas.create_line(proj_pts[0][0], proj_pts[0][1], proj_pts[1][0], proj_pts[1][1], fill=color, width=1)
                self._input_canvas.create_line(proj_pts[1][0], proj_pts[1][1], proj_pts[2][0], proj_pts[2][1], fill=color, width=1)
                self._input_canvas.create_line(proj_pts[2][0], proj_pts[2][1], proj_pts[0][0], proj_pts[0][1], fill=color, width=1)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            for i in range(len(transformed) - 2):
                pts = transformed[i:i+3]
                color = self._color_to_hex(pts[0][1])
                proj_pts = [self._project_input(p, width, height) for p, c in pts]
                self._input_canvas.create_line(proj_pts[0][0], proj_pts[0][1], proj_pts[1][0], proj_pts[1][1], fill=color, width=1)
                self._input_canvas.create_line(proj_pts[1][0], proj_pts[1][1], proj_pts[2][0], proj_pts[2][1], fill=color, width=1)
                self._input_canvas.create_line(proj_pts[2][0], proj_pts[2][1], proj_pts[0][0], proj_pts[0][1], fill=color, width=1)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINELIST:
            for i in range(0, len(transformed) - 1, 2):
                p1, c1 = transformed[i]
                p2, c2 = transformed[i+1]
                proj1 = self._project_input(p1, width, height)
                proj2 = self._project_input(p2, width, height)
                color = self._color_to_hex(c1)
                self._input_canvas.create_line(proj1[0], proj1[1], proj2[0], proj2[1], fill=color, width=2)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
            for i in range(len(transformed) - 1):
                p1, c1 = transformed[i]
                p2, c2 = transformed[i+1]
                proj1 = self._project_input(p1, width, height)
                proj2 = self._project_input(p2, width, height)
                color = self._color_to_hex(c1)
                self._input_canvas.create_line(proj1[0], proj1[1], proj2[0], proj2[1], fill=color, width=2)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
            for p, c in transformed:
                proj = self._project_input(p, width, height)
                self._input_canvas.create_oval(proj[0]-4, proj[1]-4, proj[0]+4, proj[1]+4, fill=self._color_to_hex(c), outline='white')

        if self._show_normals:
            self._draw_normals_input(transformed, width, height)

    def _draw_mesh_wireframe_output(self, transformed: list, width: float, height: float):
        """绘制输出wireframe线框"""
        if self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
            for i in range(0, len(transformed) - 2, 3):
                pts = transformed[i:i+3]
                color = self._color_to_hex(pts[0][1])
                proj_pts = [self._project_output(p, width, height) for p, c in pts]
                self._output_canvas.create_line(proj_pts[0][0], proj_pts[0][1], proj_pts[1][0], proj_pts[1][1], fill=color, width=1)
                self._output_canvas.create_line(proj_pts[1][0], proj_pts[1][1], proj_pts[2][0], proj_pts[2][1], fill=color, width=1)
                self._output_canvas.create_line(proj_pts[2][0], proj_pts[2][1], proj_pts[0][0], proj_pts[0][1], fill=color, width=1)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
            for i in range(len(transformed) - 2):
                pts = transformed[i:i+3]
                color = self._color_to_hex(pts[0][1])
                proj_pts = [self._project_output(p, width, height) for p, c in pts]
                self._output_canvas.create_line(proj_pts[0][0], proj_pts[0][1], proj_pts[1][0], proj_pts[1][1], fill=color, width=1)
                self._output_canvas.create_line(proj_pts[1][0], proj_pts[1][1], proj_pts[2][0], proj_pts[2][1], fill=color, width=1)
                self._output_canvas.create_line(proj_pts[2][0], proj_pts[2][1], proj_pts[0][0], proj_pts[0][1], fill=color, width=1)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINELIST:
            for i in range(0, len(transformed) - 1, 2):
                p1, c1 = transformed[i]
                p2, c2 = transformed[i+1]
                proj1 = self._project_output(p1, width, height)
                proj2 = self._project_output(p2, width, height)
                color = self._color_to_hex(c1)
                self._output_canvas.create_line(proj1[0], proj1[1], proj2[0], proj2[1], fill=color, width=2)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
            for i in range(len(transformed) - 1):
                p1, c1 = transformed[i]
                p2, c2 = transformed[i+1]
                proj1 = self._project_output(p1, width, height)
                proj2 = self._project_output(p2, width, height)
                color = self._color_to_hex(c1)
                self._output_canvas.create_line(proj1[0], proj1[1], proj2[0], proj2[1], fill=color, width=2)

        elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
            for p, c in transformed:
                proj = self._project_output(p, width, height)
                self._output_canvas.create_oval(proj[0]-4, proj[1]-4, proj[0]+4, proj[1]+4, fill=self._color_to_hex(c), outline='white')

        if self._show_normals:
            self._draw_normals_output(transformed, width, height)

    def _draw_normals_input(self, transformed: list, width: float, height: float):
        """绘制输入顶点法线向量"""
        if not self.input_vertices:
            return

        normal_scale = 0.1 * (self._input_bounds[1] if self._input_bounds else 1.0)

        for i, (pos, color) in enumerate(transformed):
            if i < len(self.input_vertices):
                normal = self.input_vertices[i].normal
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
                    start_proj = self._project_input(pos, width, height)
                    end_proj = self._project_input(end_pos, width, height)
                    r = int(min(255, max(0, (nx * 0.5 + 0.5) * 255)))
                    g = int(min(255, max(0, (ny * 0.5 + 0.5) * 255)))
                    b = int(min(255, max(0, (nz * 0.5 + 0.5) * 255)))
                    color_hex = f'#{r:02x}{g:02x}{b:02x}'
                    self._input_canvas.create_line(start_proj[0], start_proj[1], end_proj[0], end_proj[1],
                                            fill=color_hex, width=1)
                    self._input_canvas.create_oval(end_proj[0]-2, end_proj[1]-2, end_proj[0]+2, end_proj[1]+2,
                                            fill=color_hex, outline='')

    def _draw_normals_output(self, transformed: list, width: float, height: float):
        """绘制输出顶点法线向量"""
        if not self.output_vertices:
            return

        normal_scale = 0.1 * (self._output_bounds[1] if self._output_bounds else 1.0)

        for i, (pos, color) in enumerate(transformed):
            if i < len(self.output_vertices):
                normal = self.output_vertices[i].normal
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
                    start_proj = self._project_output(pos, width, height)
                    end_proj = self._project_output(end_pos, width, height)
                    r = int(min(255, max(0, (nx * 0.5 + 0.5) * 255)))
                    g = int(min(255, max(0, (ny * 0.5 + 0.5) * 255)))
                    b = int(min(255, max(0, (nz * 0.5 + 0.5) * 255)))
                    color_hex = f'#{r:02x}{g:02x}{b:02x}'
                    self._output_canvas.create_line(start_proj[0], start_proj[1], end_proj[0], end_proj[1],
                                            fill=color_hex, width=1)
                    self._output_canvas.create_oval(end_proj[0]-2, end_proj[1]-2, end_proj[0]+2, end_proj[1]+2,
                                            fill=color_hex, outline='')

    def _draw_mesh(self):
        """绘制mesh到画布"""
        self._draw_mesh_animated(max(len(self.input_vertices), len(self.output_vertices)))

    def _draw_rasterizer_pixels(self):
        """绘制光栅化后的像素到Rasterizer画布"""
        if not self._rasterizer_canvas or not self._rasterizer_pixels:
            return

        self._rasterizer_canvas.delete("all")

        canvas_width = int(self._rasterizer_canvas.cget('width'))
        canvas_height = int(self._rasterizer_canvas.cget('height'))

        if not self._rasterizer_pixels:
            return

        min_x = min(p.x for p in self._rasterizer_pixels)
        max_x = max(p.x for p in self._rasterizer_pixels)
        min_y = min(p.y for p in self._rasterizer_pixels)
        max_y = max(p.y for p in self._rasterizer_pixels)

        mesh_width = max(max_x - min_x, 1)
        mesh_height = max(max_y - min_y, 1)

        margin = 40
        usable_width = canvas_width - 2 * margin
        usable_height = canvas_height - 2 * margin
        scale = self._rasterizer_scale * min(usable_width, usable_height) / max(mesh_width, mesh_height)
        if scale < 0.01:
            scale = 0.01

        offset_x = canvas_width / 2 + self._rasterizer_offset_x - (min_x + max_x) / 2 * scale
        offset_y = canvas_height / 2 + self._rasterizer_offset_y - (min_y + max_y) / 2 * scale

        drawn_primitives = set()
        for pixel in self._rasterizer_pixels:
            screen_x = pixel.x * scale + offset_x
            screen_y = pixel.y * scale + offset_y

            prim_id = pixel.primitive_id
            if prim_id not in drawn_primitives:
                hue = (prim_id * 37) % 360
                drawn_primitives.add(prim_id)
            else:
                hue = (prim_id * 37) % 360

            r = int(127 + 127 * math.sin(hue * math.pi / 180))
            g = int(127 + 127 * math.sin((hue + 120) * math.pi / 180))
            b = int(127 + 127 * math.sin((hue + 240) * math.pi / 180))
            color_hex = f'#{r:02x}{g:02x}{b:02x}'

            self._rasterizer_canvas.create_rectangle(
                screen_x - 1, screen_y - 1, screen_x + 1, screen_y + 1,
                fill=color_hex, outline=color_hex
            )

    def _draw_pixel_shader_pixels(self):
        """绘制Pixel Shader输出后的像素到Pixel Shader画布"""
        if not self._pixel_shader_canvas or not self._rasterizer_pixels:
            return

        self._pixel_shader_canvas.delete("all")

        canvas_width = int(self._pixel_shader_canvas.cget('width'))
        canvas_height = int(self._pixel_shader_canvas.cget('height'))

        if not self._rasterizer_pixels:
            return

        min_x = min(p.x for p in self._rasterizer_pixels)
        max_x = max(p.x for p in self._rasterizer_pixels)
        min_y = min(p.y for p in self._rasterizer_pixels)
        max_y = max(p.y for p in self._rasterizer_pixels)

        mesh_width = max(max_x - min_x, 1)
        mesh_height = max(max_y - min_y, 1)

        margin = 40
        usable_width = canvas_width - 2 * margin
        usable_height = canvas_height - 2 * margin
        scale = self._rasterizer_scale * min(usable_width, usable_height) / max(mesh_width, mesh_height)
        if scale < 0.01:
            scale = 0.01

        offset_x = canvas_width / 2 + self._rasterizer_offset_x - (min_x + max_x) / 2 * scale
        offset_y = canvas_height / 2 + self._rasterizer_offset_y - (min_y + max_y) / 2 * scale

        for pixel in self._rasterizer_pixels:
            screen_x = pixel.x * scale + offset_x
            screen_y = pixel.y * scale + offset_y

            if pixel.ps_output_color:
                color = pixel.ps_output_color
                r = int(min(255, max(0, color[0] * 255)))
                g = int(min(255, max(0, color[1] * 255)))
                b = int(min(255, max(0, color[2] * 255)))
                color_hex = f'#{r:02x}{g:02x}{b:02x}'
            else:
                prim_id = pixel.primitive_id
                hue = (prim_id * 37) % 360
                r = int(127 + 127 * math.sin(hue * math.pi / 180))
                g = int(127 + 127 * math.sin((hue + 120) * math.pi / 180))
                b = int(127 + 127 * math.sin((hue + 240) * math.pi / 180))
                color_hex = f'#{r:02x}{g:02x}{b:02x}'

            self._pixel_shader_canvas.create_rectangle(
                screen_x - 1, screen_y - 1, screen_x + 1, screen_y + 1,
                fill=color_hex, outline=color_hex
            )

    def _draw_output_merger_pixels(self):
        """绘制Output Merger处理后的像素到Output Merger画布"""
        if not self._output_merger_canvas:
            return

        if self._output_merger_pixels:
            pixels = self._output_merger_pixels
        else:
            pixels = self._rasterizer_pixels

        if not pixels:
            return

        self._output_merger_canvas.delete("all")

        canvas_width = int(self._output_merger_canvas.cget('width'))
        canvas_height = int(self._output_merger_canvas.cget('height'))

        min_x = min(p.x for p in pixels)
        max_x = max(p.x for p in pixels)
        min_y = min(p.y for p in pixels)
        max_y = max(p.y for p in pixels)

        mesh_width = max(max_x - min_x, 1)
        mesh_height = max(max_y - min_y, 1)

        margin = 40
        usable_width = canvas_width - 2 * margin
        usable_height = canvas_height - 2 * margin
        scale = self._rasterizer_scale * min(usable_width, usable_height) / max(mesh_width, mesh_height)
        if scale < 0.01:
            scale = 0.01

        offset_x = canvas_width / 2 + self._rasterizer_offset_x - (min_x + max_x) / 2 * scale
        offset_y = canvas_height / 2 + self._rasterizer_offset_y - (min_y + max_y) / 2 * scale

        for pixel in pixels:
            screen_x = pixel.x * scale + offset_x
            screen_y = pixel.y * scale + offset_y

            if pixel.ps_output_color:
                color = pixel.ps_output_color
                r = int(min(255, max(0, color[0] * 255)))
                g = int(min(255, max(0, color[1] * 255)))
                b = int(min(255, max(0, color[2] * 255)))
                color_hex = f'#{r:02x}{g:02x}{b:02x}'
            else:
                prim_id = pixel.primitive_id
                hue = (prim_id * 37) % 360
                r = int(127 + 127 * math.sin(hue * math.pi / 180))
                g = int(127 + 127 * math.sin((hue + 120) * math.pi / 180))
                b = int(127 + 127 * math.sin((hue + 240) * math.pi / 180))
                color_hex = f'#{r:02x}{g:02x}{b:02x}'

            self._output_merger_canvas.create_rectangle(
                screen_x - 1, screen_y - 1, screen_x + 1, screen_y + 1,
                fill=color_hex, outline=color_hex
            )

    def _draw_mesh_animated(self, count: int = None):
        """绘制动画mesh到画布，只渲染前count个元素"""
        if not self._input_canvas or not self._output_canvas:
            return

        if not self.input_vertices and not self.output_vertices:
            self._input_canvas.delete("all")
            self._output_canvas.delete("all")
            return

        self._input_canvas.delete("all")
        self._output_canvas.delete("all")

        input_width = int(self._input_canvas.cget('width'))
        input_height = int(self._input_canvas.cget('height'))
        output_width = int(self._output_canvas.cget('width'))
        output_height = int(self._output_canvas.cget('height'))

        input_transformed = []
        for v in self.input_vertices:
            p = self._transform_vertex_input(v.position)
            input_transformed.append((p, v.color))

        output_transformed = []
        for v in self.output_vertices:
            p = self._transform_vertex_output(v.position)
            output_transformed.append((p, v.color))

        if count is None:
            count = self._current_index + 1

        self._draw_mesh_wireframe_input(input_transformed[:count], input_width, input_height)
        self._draw_mesh_wireframe_output(output_transformed[:count], output_width, output_height)

        if self._selected_input_vertex_index is not None and self._selected_input_vertex_index < len(input_transformed):
            p, c = input_transformed[self._selected_input_vertex_index]
            proj = self._project_input(p, input_width, input_height)
            self._input_canvas.create_oval(proj[0]-8, proj[1]-8, proj[0]+8, proj[1]+8, outline="#00ff00", width=2)

        if self._selected_output_vertex_index is not None and self._selected_output_vertex_index < len(output_transformed):
            p, c = output_transformed[self._selected_output_vertex_index]
            proj = self._project_output(p, output_width, output_height)
            self._output_canvas.create_oval(proj[0]-8, proj[1]-8, proj[0]+8, proj[1]+8, outline="#ff8800", width=2)

        self._draw_rasterizer_pixels()
        self._draw_pixel_shader_pixels()
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
            info = f"Input: {len(self.input_vertices)} vertices | Output: {len(self.output_vertices)} vertices | Topology: {topo_names.get(self.primitive_topology, 'Unknown')} | Input Zoom: {self._input_scale:.2f}x | Result Zoom: {self._output_scale:.2f}x"
            self._info_label.config(text=info)

    def _on_mouse_drag_input(self, event):
        """处理输入画布鼠标拖动旋转"""
        if self._last_mouse:
            dx = event.x - self._last_mouse[0]
            dy = event.y - self._last_mouse[1]
            if self._active_view_var.get():
                self._input_rotation_y += dx * 0.5
                self._input_rotation_x += dy * 0.5
            self._draw_mesh()
        self._last_mouse = (event.x, event.y)

    def _on_mouse_drag_output(self, event):
        """处理输出画布鼠标拖动旋转"""
        if self._last_mouse:
            dx = event.x - self._last_mouse[0]
            dy = event.y - self._last_mouse[1]
            if not self._active_view_var.get():
                self._output_rotation_y += dx * 0.5
                self._output_rotation_x += dy * 0.5
            self._draw_mesh()
        self._last_mouse = (event.x, event.y)

    def _on_mouse_release(self, event):
        """处理鼠标释放"""
        self._last_mouse = None

    def _on_right_click_input(self, event):
        """处理输入画布右键点击选择顶点"""
        if not self.input_vertices:
            return

        input_width = int(self._input_canvas.cget('width'))
        input_height = int(self._input_canvas.cget('height'))

        min_dist = float('inf')
        nearest_idx = None

        for i, v in enumerate(self.input_vertices):
            p = self._transform_vertex_input(v.position)
            proj = self._project_input(p, input_width, input_height)
            dist = ((event.x - proj[0]) ** 2 + (event.y - proj[1]) ** 2) ** 0.5
            if dist < min_dist and dist < 20:
                min_dist = dist
                nearest_idx = i

        if nearest_idx is not None:
            self._selected_input_vertex_index = nearest_idx
            self._selected_output_vertex_index = nearest_idx
            self._draw_mesh()
            self._update_vertex_info_panel()

    def _on_right_click_output(self, event):
        """处理输出画布右键点击选择顶点"""
        if not self.output_vertices:
            return

        output_width = int(self._output_canvas.cget('width'))
        output_height = int(self._output_canvas.cget('height'))

        min_dist = float('inf')
        nearest_idx = None

        for i, v in enumerate(self.output_vertices):
            p = self._transform_vertex_output(v.position)
            proj = self._project_output(p, output_width, output_height)
            dist = ((event.x - proj[0]) ** 2 + (event.y - proj[1]) ** 2) ** 0.5
            if dist < min_dist and dist < 20:
                min_dist = dist
                nearest_idx = i

        if nearest_idx is not None:
            self._selected_output_vertex_index = nearest_idx
            self._selected_input_vertex_index = nearest_idx
            self._draw_mesh()
            self._update_vertex_info_panel()

    def _update_vertex_info_panel(self):
        """更新顶点信息面板"""
        if not self._vertex_info_panel:
            return

        self._vertex_info_panel.delete("all")

        font_size = self._vertex_info_font_size
        line_height = font_size + 10

        self._vertex_info_panel.create_text(10, 10, anchor=tk.NW, fill="white", font=("Consolas", font_size), text="Selected Vertex Info")
        y_pos = 10 + line_height * 2

        input_idx = self._selected_input_vertex_index
        output_idx = self._selected_output_vertex_index

        if input_idx is not None and input_idx < len(self.input_vertices):
            v = self.input_vertices[input_idx]
            self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="#00ff00", font=("Consolas", font_size), text=f"--- Input Vertex [{input_idx}] ---")
            y_pos += line_height * 1.5

            pos = v.position
            self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 1), text=f"Position: ({pos[0]:.4f}, {pos[1]:.4f}, {pos[2]:.4f})")
            y_pos += line_height

            if v.normal:
                n = v.normal
                self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 1), text=f"Normal: ({n[0]:.4f}, {n[1]:.4f}, {n[2]:.4f})")
                y_pos += line_height

            if v.color:
                c = v.color
                self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 1), text=f"Color: ({c[0]:.4f}, {c[1]:.4f}, {c[2]:.4f}, {c[3]:.4f})")
                y_pos += line_height

            y_pos += line_height

            if self._re_execute_btn:
                self._re_execute_btn.config(state=tk.NORMAL)
        else:
            self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="gray", font=("Consolas", font_size - 1), text="No Input Vertex Selected")
            y_pos += line_height * 2

            if self._re_execute_btn:
                self._re_execute_btn.config(state=tk.DISABLED)

        if output_idx is not None and output_idx < len(self.output_vertices):
            v = self.output_vertices[output_idx]
            self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="#ff8800", font=("Consolas", font_size), text=f"--- Output Vertex [{output_idx}] ---")
            y_pos += line_height * 1.5

            pos = v.position
            self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 1), text=f"Position: ({pos[0]:.4f}, {pos[1]:.4f}, {pos[2]:.4f})")
            y_pos += line_height

            if v.normal:
                n = v.normal
                self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 1), text=f"Normal: ({n[0]:.4f}, {n[1]:.4f}, {n[2]:.4f})")
                y_pos += line_height

            if v.color:
                c = v.color
                self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 1), text=f"Color: ({c[0]:.4f}, {c[1]:.4f}, {c[2]:.4f}, {c[3]:.4f})")
                y_pos += line_height
        else:
            self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="gray", font=("Consolas", font_size - 1), text="No Output Vertex Selected")

        cb_data = self._get_cbuffer_display_data()
        if cb_data:
            y_pos += line_height
            self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="#00ffff", font=("Consolas", font_size), text="--- Constant Buffer Data ---")
            y_pos += line_height * 1.5

            for cb_name, cb_info in cb_data.items():
                self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="#00ffff", font=("Consolas", font_size - 1), text=f"[{cb_name}]")
                y_pos += line_height

                for field in cb_info.get('fields', []):
                    field_name = field['name']
                    field_type = field['field_type']
                    data = field['data']

                    if data is None:
                        continue

                    if 'float4x4' in field_type:
                        self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 2), text=f"  {field_name} (float4x4):")
                        y_pos += line_height
                        for row_idx, row in enumerate(data):
                            row_str = '  '.join(f"{v:8.4f}" for v in row)
                            self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 2), text=f"    [{row_str}]")
                            y_pos += line_height
                    elif 'float4' in field_type:
                        val_str = ', '.join(f"{v:.4f}" for v in data)
                        self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 2), text=f"  {field_name} (float4): [{val_str}]")
                        y_pos += line_height
                    elif 'float3' in field_type:
                        val_str = ', '.join(f"{v:.4f}" for v in data)
                        self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 2), text=f"  {field_name} (float3): [{val_str}]")
                        y_pos += line_height
                    elif 'float2' in field_type:
                        val_str = ', '.join(f"{v:.4f}" for v in data)
                        self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 2), text=f"  {field_name} (float2): [{val_str}]")
                        y_pos += line_height
                    elif 'float' in field_type:
                        self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 2), text=f"  {field_name} (float): {data:.4f}")
                        y_pos += line_height
                    else:
                        self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="white", font=("Consolas", font_size - 2), text=f"  {field_name} ({field_type}): {data}")
                        y_pos += line_height

        hlsl_code = self._get_hlsl_code_display()
        if hlsl_code:
            y_pos += line_height
            self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="#ffff00", font=("Consolas", font_size), text="--- HLSL Source Code ---")
            y_pos += line_height * 1.5

            code_lines = hlsl_code.split('\n')
            code_font_size = max(font_size - 3, 5)
            code_line_height = font_size - 2
            for line in code_lines[:30]:
                self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="#aaaaaa", font=("Consolas", code_font_size), text=line[:80])
                y_pos += code_line_height
            if len(code_lines) > 30:
                self._vertex_info_panel.create_text(10, y_pos, anchor=tk.NW, fill="gray", font=("Consolas", code_font_size), text=f"... ({len(code_lines) - 30} more lines)")
                y_pos += code_line_height

        bbox = self._vertex_info_panel.bbox("all")
        if bbox:
            self._vertex_info_panel.configure(scrollregion=bbox)

    def set_hlsl_interpreter(self, interpreter, main_func: str = "main", input_struct: str = "VS_INPUT"):
        """设置HLSL解释器以支持重新执行顶点着色器"""
        self._hlsl_interpreter = interpreter
        self._hlsl_main_func = main_func
        self._hlsl_input_struct = input_struct
        if self._re_execute_btn:
            self._re_execute_btn.config(state=tk.NORMAL)

    def _get_cbuffer_display_data(self):
        """Get cbuffer data formatted for display"""
        if not self._hlsl_interpreter:
            return {}
        return self._hlsl_interpreter.get_cbuffer_data()

    def _get_hlsl_code_display(self):
        """Get HLSL code for display"""
        if not self._hlsl_interpreter:
            return None
        return self._hlsl_interpreter.get_last_executeVS_code()

    def _on_re_execute_vertex(self):
        """重新执行选中顶点的顶点着色器"""
        if not self._hlsl_interpreter:
            self._append_shader_log("Error: No HLSL Interpreter set. Call set_hlsl_interpreter() first.")
            return

        input_idx = self._selected_input_vertex_index
        if input_idx is None or input_idx >= len(self.input_vertices):
            self._append_shader_log("Error: No input vertex selected")
            return

        self._append_shader_log("=" * 50)
        self._append_shader_log(f"Re-executing Vertex Shader for Input Vertex [{input_idx}]")
        self._append_shader_log("=" * 50)

        v = self.input_vertices[input_idx]
        input_struct = self._hlsl_interpreter.structs.get(self._hlsl_input_struct)
        if not input_struct:
            self._append_shader_log(f"Error: Cannot find input struct '{self._hlsl_input_struct}'")
            return

        input_data = {}
        for field in input_struct.fields:
            semantic_lower = field.semantic.lower() if field.semantic else ''
            if 'pos' in semantic_lower or 'position' in semantic_lower or semantic_lower == 'sv_position':
                input_data[field.name] = v.position
            elif 'normal' in semantic_lower:
                input_data[field.name] = v.normal if v.normal else [0, 0, 1]
            elif 'color' in semantic_lower:
                input_data[field.name] = v.color if v.color else [1, 1, 1, 1]
            elif 'texcoord' in semantic_lower:
                if 'texcoord2' in semantic_lower or 'texcoord1' in semantic_lower:
                    input_data[field.name] = v.tex_coord2 if v.tex_coord2 else [0, 0]
                else:
                    input_data[field.name] = v.tex_coord if v.tex_coord else [0, 0]

        old_print_syntax_tree = self._hlsl_interpreter.printSyntaxTree
        old_print_sequence = self._hlsl_interpreter.print_sequence

        self._hlsl_interpreter.printSyntaxTree = True
        self._hlsl_interpreter.print_sequence = 1

        captured_log = []
        original_log_output = self._hlsl_interpreter.log_output

        def capture_log(*args, **kwargs):
            msg = ' '.join(str(arg) for arg in args)
            captured_log.append(msg)
            original_log_output(*args, **kwargs)

        self._hlsl_interpreter.log_output = capture_log

        try:
            result = self._hlsl_interpreter.execute_main_function(
                self._hlsl_interpreter.hlsl_code,
                self._hlsl_main_func,
                self._hlsl_input_struct,
                input_idx,
                input_data
            )

            for line in captured_log:
                self._append_shader_log(line)

            self._append_shader_log("")
            self._append_shader_log("=== OUTPUT RESULT ===")
            if result:
                for key, value in result.items():
                    if isinstance(value, list):
                        if len(value) == 4:
                            self._append_shader_log(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]")
                        elif len(value) == 3:
                            self._append_shader_log(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]")
                        else:
                            self._append_shader_log(f"{key}: {value}")
                    else:
                        self._append_shader_log(f"{key}: {value}")
            else:
                self._append_shader_log("Result: None")

        except Exception as e:
            self._append_shader_log(f"Error during execution: {e}")

        finally:
            self._hlsl_interpreter.log_output = original_log_output
            self._hlsl_interpreter.printSyntaxTree = old_print_syntax_tree
            self._hlsl_interpreter.print_sequence = old_print_sequence

        self._append_shader_log("=" * 50)
        self._append_shader_log("Execution completed")
        self._append_shader_log("=" * 50)

    def _on_clear_shader_log(self):
        """清除顶点着色器执行日志"""
        self._vertex_shader_log = []
        if self._vertex_shader_log_text:
            self._vertex_shader_log_text.delete("1.0", tk.END)

    def _append_shader_log(self, text: str):
        """追加文本到顶点着色器日志"""
        self._vertex_shader_log.append(text)
        if self._vertex_shader_log_text:
            self._vertex_shader_log_text.insert(tk.END, text + "\n")
            self._vertex_shader_log_text.see(tk.END)

    def _on_info_font_size_changed(self):
        """处理顶点信息面板字体大小变化"""
        size = self._info_font_size_var.get()
        self._vertex_info_font_size = size
        self._update_vertex_info_panel()

    def _on_log_font_size_changed(self):
        """处理着色器日志字体大小变化"""
        size = self._log_font_size_var.get()
        self._shader_log_font_size = size
        if self._vertex_shader_log_text:
            self._vertex_shader_log_text.config(font=("Consolas", size))

    def _on_mouse_wheel_input(self, event):
        """处理输入画布鼠标滚轮缩放"""
        if self._active_view_var.get():
            if event.delta > 0:
                self._input_scale *= 1.1
            else:
                self._input_scale *= 0.9
            self._input_scale = max(MESH_VIEW_MIN_SCALE, min(MESH_VIEW_MAX_SCALE, self._input_scale))
        self._draw_mesh()

    def _on_mouse_wheel_output(self, event):
        """处理输出画布鼠标滚轮缩放"""
        if not self._active_view_var.get():
            if event.delta > 0:
                self._output_scale *= 1.1
            else:
                self._output_scale *= 0.9
            self._output_scale = max(MESH_VIEW_MIN_SCALE, min(MESH_VIEW_MAX_SCALE, self._output_scale))
        self._draw_mesh()

    def _on_mouse_drag_rasterizer(self, event):
        """处理Rasterizer画布鼠标拖动"""
        if self._last_mouse:
            dx = event.x - self._last_mouse[0]
            dy = event.y - self._last_mouse[1]
            self._rasterizer_offset_x += dx
            self._rasterizer_offset_y += dy
            self._draw_mesh()
        self._last_mouse = (event.x, event.y)

    def _on_mouse_wheel_rasterizer(self, event):
        """处理Rasterizer画布鼠标滚轮缩放"""
        if event.delta > 0:
            self._rasterizer_scale *= 1.1
        else:
            self._rasterizer_scale *= 0.9
        self._rasterizer_scale = max(0.01, min(100, self._rasterizer_scale))
        self._draw_mesh()

    def _on_mouse_drag_pixel_shader(self, event):
        """处理Pixel Shader画布鼠标拖动"""
        pass

    def _on_mouse_wheel_pixel_shader(self, event):
        """处理Pixel Shader画布鼠标滚轮缩放"""
        pass

    def _on_mouse_drag_output_merger(self, event):
        """处理Output Merger画布鼠标拖动"""
        pass

    def _on_mouse_wheel_output_merger(self, event):
        """处理Output Merger画布鼠标滚轮缩放"""
        pass

    def _on_resize(self, event):
        """处理窗口大小改变"""
        self._draw_mesh()

    def _on_layout_changed(self):
        """处理布局变更"""
        if not hasattr(self, '_layout_var') or self._layout_var is None:
            return
        layout = self._layout_var.get()
        if layout == "default":
            self._paned_window.delete(0, tk.END)
            left_paned = ttk.PanedWindow(self._paned_window, orient=tk.VERTICAL)
            self._paned_window.add(left_paned)
            input_frame = ttk.LabelFrame(left_paned, text="Input Vertices", padding=5)
            left_paned.add(input_frame)
            self._input_canvas = tk.Canvas(input_frame, bg="#1a1a2e", width=500, height=520)
            self._input_canvas.pack(fill=tk.BOTH, expand=True)
            output_frame = ttk.LabelFrame(left_paned, text="Output (VS Result)", padding=5)
            left_paned.add(output_frame)
            self._output_canvas = tk.Canvas(output_frame, bg="#1a1a2e", width=500, height=520)
            self._output_canvas.pack(fill=tk.BOTH, expand=True)
            right_paned = ttk.PanedWindow(self._paned_window, orient=tk.VERTICAL)
            self._paned_window.add(right_paned)
            info_frame = ttk.LabelFrame(right_paned, text="Selected Vertex Info", padding=5)
            right_paned.add(info_frame)
            info_inner = ttk.Frame(info_frame)
            info_inner.pack(fill=tk.BOTH, expand=True)
            btn_frame = ttk.Frame(info_inner)
            btn_frame.pack(side=tk.TOP, fill=tk.X, pady=2)
            self._re_execute_btn = ttk.Button(btn_frame, text="Re-execute Vertex Shader", command=self._on_re_execute_vertex, state=tk.DISABLED)
            self._re_execute_btn.pack(side=tk.LEFT, padx=2)
            ttk.Button(btn_frame, text="Clear Log", command=self._on_clear_shader_log).pack(side=tk.LEFT, padx=2)
            ttk.Label(btn_frame, text="Info Font:").pack(side=tk.LEFT, padx=(10, 2))
            self._info_font_size_var = tk.IntVar(value=self._vertex_info_font_size)
            info_font_spin = ttk.Spinbox(btn_frame, from_=6, to=24, width=3, textvariable=self._info_font_size_var, command=self._on_info_font_size_changed)
            info_font_spin.pack(side=tk.LEFT, padx=2)
            ttk.Label(btn_frame, text="Log Font:").pack(side=tk.LEFT, padx=(10, 2))
            self._log_font_size_var = tk.IntVar(value=self._shader_log_font_size)
            log_font_spin = ttk.Spinbox(btn_frame, from_=6, to=24, width=3, textvariable=self._log_font_size_var, command=self._on_log_font_size_changed)
            log_font_spin.pack(side=tk.LEFT, padx=2)
            info_canvas_frame = ttk.Frame(info_inner)
            info_canvas_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
            self._vertex_info_scroll_y = ttk.Scrollbar(info_canvas_frame)
            self._vertex_info_scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
            self._vertex_info_inner_frame = tk.Canvas(info_canvas_frame, bg="#1a1a2e", highlightthickness=0, yscrollcommand=self._vertex_info_scroll_y.set)
            self._vertex_info_inner_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            self._vertex_info_scroll_y.config(command=self._vertex_info_inner_frame.yview)
            self._vertex_info_panel = self._vertex_info_inner_frame
            log_label_frame = ttk.LabelFrame(right_paned, text="Vertex Shader Execution Log", padding=2)
            right_paned.add(log_label_frame)
            log_scroll = ttk.Scrollbar(log_label_frame)
            log_scroll.pack(side=tk.RIGHT, fill=tk.Y)
            self._vertex_shader_log_text = tk.Text(log_label_frame, bg="#0d0d1a", fg="#00ff00", font=("Consolas", self._shader_log_font_size), height=8, wrap=tk.WORD, yscrollcommand=log_scroll.set)
            self._vertex_shader_log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            log_scroll.config(command=self._vertex_shader_log_text.yview)
            self._bind_canvas_events()
            self._info_label = ttk.Label(self._root, text="Input: 0 vertices | Output: 0 vertices | Topology: None", font=("Consolas", 10))
            self._info_label.place(relx=0, rely=1.0, anchor=tk.SW, relwidth=1.0)

        elif layout == "side-by-side":
            self._paned_window.delete(0, tk.END)
            left_paned = ttk.PanedWindow(self._paned_window, orient=tk.VERTICAL)
            self._paned_window.add(left_paned)
            input_frame = ttk.LabelFrame(left_paned, text="Input Vertices", padding=5)
            left_paned.add(input_frame)
            self._input_canvas = tk.Canvas(input_frame, bg="#1a1a2e", width=500, height=520)
            self._input_canvas.pack(fill=tk.BOTH, expand=True)
            output_frame = ttk.LabelFrame(left_paned, text="Output (VS Result)", padding=5)
            left_paned.add(output_frame)
            self._output_canvas = tk.Canvas(output_frame, bg="#1a1a2e", width=500, height=520)
            self._output_canvas.pack(fill=tk.BOTH, expand=True)
            info_frame = ttk.LabelFrame(self._paned_window, text="Selected Vertex Info", padding=5)
            self._paned_window.add(info_frame)
            info_inner = ttk.Frame(info_frame)
            info_inner.pack(fill=tk.BOTH, expand=True)
            btn_frame = ttk.Frame(info_inner)
            btn_frame.pack(side=tk.TOP, fill=tk.X, pady=2)
            self._re_execute_btn = ttk.Button(btn_frame, text="Re-execute Vertex Shader", command=self._on_re_execute_vertex, state=tk.DISABLED)
            self._re_execute_btn.pack(side=tk.LEFT, padx=2)
            ttk.Button(btn_frame, text="Clear Log", command=self._on_clear_shader_log).pack(side=tk.LEFT, padx=2)
            self._vertex_info_panel = tk.Canvas(info_inner, bg="#1a1a2e", width=300, height=350, highlightthickness=0)
            self._vertex_info_panel.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
            log_label_frame = ttk.LabelFrame(info_inner, text="Vertex Shader Execution Log", padding=2)
            log_label_frame.pack(side=tk.BOTTOM, fill=tk.BOTH, expand=True, pady=(2, 0))
            log_scroll = ttk.Scrollbar(log_label_frame)
            log_scroll.pack(side=tk.RIGHT, fill=tk.Y)
            self._vertex_shader_log_text = tk.Text(log_label_frame, bg="#0d0d1a", fg="#00ff00", font=("Consolas", 8), height=8, wrap=tk.WORD, yscrollcommand=log_scroll.set)
            self._vertex_shader_log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            log_scroll.config(command=self._vertex_shader_log_text.yview)
            self._bind_canvas_events()

        elif layout == "stacked":
            self._paned_window.delete(0, tk.END)
            left_paned = ttk.PanedWindow(self._paned_window, orient=tk.VERTICAL)
            self._paned_window.add(left_paned)
            input_frame = ttk.LabelFrame(left_paned, text="Input Vertices", padding=5)
            left_paned.add(input_frame)
            self._input_canvas = tk.Canvas(input_frame, bg="#1a1a2e", width=500, height=520)
            self._input_canvas.pack(fill=tk.BOTH, expand=True)
            output_frame = ttk.LabelFrame(left_paned, text="Output (VS Result)", padding=5)
            left_paned.add(output_frame)
            self._output_canvas = tk.Canvas(output_frame, bg="#1a1a2e", width=500, height=520)
            self._output_canvas.pack(fill=tk.BOTH, expand=True)
            right_paned = ttk.PanedWindow(self._paned_window, orient=tk.VERTICAL)
            self._paned_window.add(right_paned)
            info_frame = ttk.LabelFrame(right_paned, text="Selected Vertex Info", padding=5)
            right_paned.add(info_frame)
            info_inner = ttk.Frame(info_frame)
            info_inner.pack(fill=tk.BOTH, expand=True)
            btn_frame = ttk.Frame(info_inner)
            btn_frame.pack(side=tk.TOP, fill=tk.X, pady=2)
            self._re_execute_btn = ttk.Button(btn_frame, text="Re-execute Vertex Shader", command=self._on_re_execute_vertex, state=tk.DISABLED)
            self._re_execute_btn.pack(side=tk.LEFT, padx=2)
            ttk.Button(btn_frame, text="Clear Log", command=self._on_clear_shader_log).pack(side=tk.LEFT, padx=2)
            self._vertex_info_panel = tk.Canvas(info_inner, bg="#1a1a2e", width=300, height=350, highlightthickness=0)
            self._vertex_info_panel.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
            log_label_frame = ttk.LabelFrame(right_paned, text="Vertex Shader Execution Log", padding=2)
            right_paned.add(log_label_frame)
            log_scroll = ttk.Scrollbar(log_label_frame)
            log_scroll.pack(side=tk.RIGHT, fill=tk.Y)
            self._vertex_shader_log_text = tk.Text(log_label_frame, bg="#0d0d1a", fg="#00ff00", font=("Consolas", 8), height=8, wrap=tk.WORD, yscrollcommand=log_scroll.set)
            self._vertex_shader_log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            log_scroll.config(command=self._vertex_shader_log_text.yview)
            self._bind_canvas_events()

        elif layout == "info-left":
            self._paned_window.delete(0, tk.END)
            left_paned = ttk.PanedWindow(self._paned_window, orient=tk.VERTICAL)
            self._paned_window.add(left_paned)
            info_frame = ttk.LabelFrame(left_paned, text="Selected Vertex Info", padding=5)
            left_paned.add(info_frame)
            info_inner = ttk.Frame(info_frame)
            info_inner.pack(fill=tk.BOTH, expand=True)
            btn_frame = ttk.Frame(info_inner)
            btn_frame.pack(side=tk.TOP, fill=tk.X, pady=2)
            self._re_execute_btn = ttk.Button(btn_frame, text="Re-execute Vertex Shader", command=self._on_re_execute_vertex, state=tk.DISABLED)
            self._re_execute_btn.pack(side=tk.LEFT, padx=2)
            ttk.Button(btn_frame, text="Clear Log", command=self._on_clear_shader_log).pack(side=tk.LEFT, padx=2)
            self._vertex_info_panel = tk.Canvas(info_inner, bg="#1a1a2e", width=300, height=350, highlightthickness=0)
            self._vertex_info_panel.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
            log_label_frame = ttk.LabelFrame(left_paned, text="Vertex Shader Execution Log", padding=2)
            left_paned.add(log_label_frame)
            log_scroll = ttk.Scrollbar(log_label_frame)
            log_scroll.pack(side=tk.RIGHT, fill=tk.Y)
            self._vertex_shader_log_text = tk.Text(log_label_frame, bg="#0d0d1a", fg="#00ff00", font=("Consolas", 8), height=8, wrap=tk.WORD, yscrollcommand=log_scroll.set)
            self._vertex_shader_log_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
            log_scroll.config(command=self._vertex_shader_log_text.yview)
            right_paned = ttk.PanedWindow(self._paned_window, orient=tk.VERTICAL)
            self._paned_window.add(right_paned)
            input_frame = ttk.LabelFrame(right_paned, text="Input Vertices", padding=5)
            right_paned.add(input_frame)
            self._input_canvas = tk.Canvas(input_frame, bg="#1a1a2e", width=500, height=520)
            self._input_canvas.pack(fill=tk.BOTH, expand=True)
            output_frame = ttk.LabelFrame(right_paned, text="Output (VS Result)", padding=5)
            right_paned.add(output_frame)
            self._output_canvas = tk.Canvas(output_frame, bg="#1a1a2e", width=500, height=520)
            self._output_canvas.pack(fill=tk.BOTH, expand=True)
            self._bind_canvas_events()

    def _bind_canvas_events(self):
        """绑定画布事件"""
        self._input_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_input(e))
        self._input_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_input(e))
        self._input_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
        self._input_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_input(e))
        self._input_canvas.bind("<Button-3>", lambda e: self._on_right_click_input(e))
        self._output_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_output(e))
        self._output_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_output(e))
        self._output_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
        self._output_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_output(e))
        self._output_canvas.bind("<Button-3>", lambda e: self._on_right_click_output(e))

    def _zoom_in(self):
        """放大当前活动视图"""
        if self._active_view_var.get():
            self._input_scale *= 1.2
            self._input_scale = min(MESH_VIEW_MAX_SCALE, self._input_scale)
        else:
            self._output_scale *= 1.2
            self._output_scale = min(MESH_VIEW_MAX_SCALE, self._output_scale)
        self._draw_mesh()

    def _zoom_out(self):
        """缩小当前活动视图"""
        if self._active_view_var.get():
            self._input_scale *= 0.8
            self._input_scale = max(MESH_VIEW_MIN_SCALE, self._input_scale)
        else:
            self._output_scale *= 0.8
            self._output_scale = max(MESH_VIEW_MIN_SCALE, self._output_scale)
        self._draw_mesh()

    def _rotate_cw(self):
        """顺时针旋转当前活动视图"""
        if self._active_view_var.get():
            self._input_rotation_y += 15
        else:
            self._output_rotation_y += 15
        self._draw_mesh()

    def _rotate_ccw(self):
        """逆时针旋转当前活动视图"""
        if self._active_view_var.get():
            self._input_rotation_y -= 15
        else:
            self._output_rotation_y -= 15
        self._draw_mesh()

    def _rotate_up(self):
        """向上旋转当前活动视图"""
        if self._active_view_var.get():
            self._input_rotation_x -= 15
        else:
            self._output_rotation_x -= 15
        self._draw_mesh()

    def _rotate_down(self):
        """向下旋转当前活动视图"""
        if self._active_view_var.get():
            self._input_rotation_x += 15
        else:
            self._output_rotation_x += 15
        self._draw_mesh()

    def _pan_left(self):
        """向左平移当前活动视图"""
        if self._active_view_var.get():
            self._input_offset_x -= 20
        else:
            self._output_offset_x -= 20
        self._draw_mesh()

    def _pan_right(self):
        """向右平移当前活动视图"""
        if self._active_view_var.get():
            self._input_offset_x += 20
        else:
            self._output_offset_x += 20
        self._draw_mesh()

    def _pan_up(self):
        """向上平移当前活动视图"""
        if self._active_view_var.get():
            self._input_offset_y -= 20
        else:
            self._output_offset_y -= 20
        self._draw_mesh()

    def _pan_down(self):
        """向下平移当前活动视图"""
        if self._active_view_var.get():
            self._input_offset_y += 20
        else:
            self._output_offset_y += 20
        self._draw_mesh()

    def _reset_view(self):
        """重置当前活动视图"""
        if self._active_view_var.get():
            self._input_rotation_x = MESH_VIEW_ROTATION_INIT_X
            self._input_rotation_y = MESH_VIEW_ROTATION_INIT_Y
            self._input_scale = MESH_VIEW_SCALE_INIT
            self._input_offset_x = MESH_VIEW_OFFSET_X
            self._input_offset_y = MESH_VIEW_OFFSET_Y
        else:
            self._output_rotation_x = MESH_VIEW_ROTATION_INIT_X
            self._output_rotation_y = MESH_VIEW_ROTATION_INIT_Y
            self._output_scale = MESH_VIEW_SCALE_INIT
            self._output_offset_x = MESH_VIEW_OFFSET_X
            self._output_offset_y = MESH_VIEW_OFFSET_Y
        self._draw_mesh()

    def _toggle_normals(self):
        """切换法线显示"""
        self._show_normals = self._normals_var.get()
        self._draw_mesh()

    def _get_active_view_props(self):
        """获取当前活动视图的属性引用"""
        if self._active_view_var.get():
            return self._input_rotation_x, self._input_rotation_y, self._input_scale, self._input_offset_x, self._input_offset_y
        else:
            return self._output_rotation_x, self._output_rotation_y, self._output_scale, self._output_offset_x, self._output_offset_y

    def _set_active_view_props(self, rot_x, rot_y, scale, offset_x, offset_y):
        """设置当前活动视图的属性"""
        if self._active_view_var.get():
            self._input_rotation_x, self._input_rotation_y, self._input_scale, self._input_offset_x, self._input_offset_y = rot_x, rot_y, scale, offset_x, offset_y
        else:
            self._output_rotation_x, self._output_rotation_y, self._output_scale, self._output_offset_x, self._output_offset_y = rot_x, rot_y, scale, offset_x, offset_y

    def _play_animation(self):
        """从开头开始播放动画"""
        if not self.input_vertices and not self.output_vertices:
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
        if not self.input_vertices and not self.output_vertices:
            return
        max_index = max(len(self.input_vertices), len(self.output_vertices)) - 1
        if self._current_index < max_index:
            self._current_index += 1
        self._draw_mesh_animated()
        self._update_step_label()
        self._update_button_states()

    def _prev_step(self):
        """回到上一个顶点/线"""
        if not self.input_vertices and not self.output_vertices:
            return
        if self._current_index > 0:
            self._current_index -= 1
        self._draw_mesh_animated()
        self._update_step_label()
        self._update_button_states()

    def _run_animation_step(self):
        """执行动画单步"""
        if not self._is_playing or self._is_paused:
            return
        if not self._root:
            return
        max_index = max(len(self.input_vertices), len(self.output_vertices)) - 1
        if self._current_index < max_index:
            self._current_index += 1
            self._draw_mesh_animated()
            self._update_step_label()
            self._animation_job = self._root.after(self._animation_interval, self._run_animation_step)
        else:
            self._is_playing = False
            self._update_button_states()

    def _update_button_states(self):
        """更新按钮状态"""
        if self._play_btn:
            self._play_btn.config(state=tk.NORMAL if not self._is_playing else tk.DISABLED)
        if self._pause_btn:
            self._pause_btn.config(state=tk.NORMAL if self._is_playing or self._current_index > 0 else tk.DISABLED)
        can_step = self._is_paused or self._current_index > 0
        if self._next_btn:
            self._next_btn.config(state=tk.NORMAL if can_step else tk.DISABLED)
        if self._prev_btn:
            self._prev_btn.config(state=tk.NORMAL if can_step and self._current_index > 0 else tk.DISABLED)

    def _update_step_label(self):
        """更新步骤显示"""
        if self._step_label:
            max_count = max(len(self.input_vertices), len(self.output_vertices))
            self._step_label.config(text=f"Step: {self._current_index + 1}/{max_count}")

    def show(self, blocking: bool = False):
        """
        显示MeshView窗口（双窗口：左侧输入，右侧输出）
        blocking: 如果为True，则阻塞直到窗口关闭（已废弃，仅为兼容）
        """
        if self._root is None:
            self._gui_ready_event.wait()
        if self._root:
            self._root.deiconify()
            self._schedule_draw()

    def _schedule_draw(self):
        """在主线程中调度绘制"""
        if self._root:
            self._root.after(0, self._draw_mesh)

    def _create_ui(self):
        """创建UI组件（在GUI线程中调用）"""

        main_frame = ttk.Frame(self._root)
        main_frame.pack(fill=tk.BOTH, expand=True)

        controls_frame = ttk.Frame(main_frame)
        controls_frame.pack(side=tk.TOP, fill=tk.X, pady=2)

        ttk.Label(controls_frame, text="Active:").pack(side=tk.LEFT, padx=2)
        ttk.Radiobutton(controls_frame, text="Input", variable=self._active_view_var, value=True).pack(side=tk.LEFT, padx=2)
        ttk.Radiobutton(controls_frame, text="Output", variable=self._active_view_var, value=False).pack(side=tk.LEFT, padx=2)

        ttk.Separator(controls_frame, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=5)

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

        ttk.Button(controls_frame, text="Close", command=self.hide).pack(side=tk.RIGHT, padx=5)

        canvas_frame = ttk.Frame(main_frame)
        canvas_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, pady=2)

        input_frame = ttk.LabelFrame(canvas_frame, text="Input Vertices", padding=5)
        input_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 5))
        self._input_canvas = tk.Canvas(input_frame, bg="#1a1a2e", width=600, height=520)
        self._input_canvas.pack(fill=tk.BOTH, expand=True)

        output_frame = ttk.LabelFrame(canvas_frame, text="Output (VS Result)", padding=5)
        output_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(5, 0))
        self._output_canvas = tk.Canvas(output_frame, bg="#1a1a2e", width=600, height=520)
        self._output_canvas.pack(fill=tk.BOTH, expand=True)

        self._input_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_input(e))
        self._input_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_input(e))
        self._input_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
        self._input_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_input(e))

        self._output_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_output(e))
        self._output_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_output(e))
        self._output_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
        self._output_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_output(e))

        self._root.bind("<Configure>", lambda e: self._on_resize(e))

        self._info_label = ttk.Label(main_frame, text="Input: 0 vertices | Output: 0 vertices | Topology: None", font=("Consolas", 10))
        self._info_label.pack(side=tk.BOTTOM, fill=tk.X, pady=2)

        self._draw_mesh()
        self._update_step_label()
        self._running = True

    def hide(self):
        """隐藏窗口"""
        if self._root:
            self._root.withdraw()

    def update(self):
        """更新显示（在GUI线程中执行）"""
        if self._root and self._running:
            self._root.after(0, self._draw_mesh)

    def _schedule_draw(self):
        """在主线程中调度绘制"""
        if self._root:
            self._root.after(0, self._draw_mesh)

    def is_visible(self) -> bool:
        """检查窗口是否可见"""
        return self._root is not None and self._running

    def close(self):
        """关闭窗口"""
        self._running = False
        if self._animation_job:
            if self._root:
                try:
                    self._root.after_cancel(self._animation_job)
                except:
                    pass
            self._animation_job = None
        if self._root:
            def _do_close():
                try:
                    self._root.quit()
                    self._root.destroy()
                except:
                    pass
                self._root = None
                self._input_canvas = None
                self._rasterizer_canvas = None
                self._pixel_shader_canvas = None
                self._output_merger_canvas = None
                self._output_notebook = None
            try:
                self._root.after(0, _do_close)
            except RuntimeError:
                self._gui_thread_alive = False
                self._root = None
                self._input_canvas = None
                self._output_canvas = None