# MeshView状态栏缩放倍数显示改进

**Session ID:** ses_1b788b564ffeb4nDHa3Msl6VfE
**Created:** 2026/5/21 10:57:17
**Updated:** 2026/5/21 10:58:01

---

## User

MeshView下面的状态栏显示了input顶点数量，output顶点数量，topology类型，zoom缩放的倍数，
1. zoom缩放倍数表示的是input窗口的倍数，请改名
2. 增加一个result窗口缩放倍数的状态显示 

---

## Assistant (Build · MiniMax-M2.7 · 8.7s)

_Thinking:_

The user wants me to modify the MeshView tool in the Irrlicht engine. They want to:
1. Rename "zoom缩放倍数" to something clearer since it represents the input window zoom level
2. Add a new status display showing the result window's zoom level

Let me first explore the codebase to find MeshView and understand its structure.


**Tool: grep**

**Input:**
```json
{
  "pattern": "MeshView"
}
```

**Output:**
```
Found 952 matches (showing first 100)
C:\Development\Graphics\irrlicht-1.8.3\Prompt\hlsl-interpreter-prompt.md:
  Line 1009: 1. 创建一个MeshView对象

  Line 1010: 2. 该MeshView对象接受输入的顶点数据，顶点数据包括了三维坐标，normal向量，颜色

  Line 1011: 3. 该MeshView对象接受primitive topology信息用于构造Mesh显示

  Line 1012: 4. 该MeshView需要创建一个图形界面用于显示，MeshView根据输入的顶点坐标和primitive topology信息构造对应的mesh

  Line 1013: 5. 这个MeshView对象在hlsl_interpreter.py里创建，在执行executeVS前，输入当前interpreter解释出来的input.Pos, input.Normal, input.Color等数据，然后显示在GUI界面中

  Line 1019: 1. MeshView的显示界面请提供一个缩放和旋转的按钮来控制当前显示的Mesh

  Line 1022: 4. 为MeshView增加一个平移当前模型的按钮

  Line 1028: 2. 为MeshView增加一个动画播放，暂停，下一步和上一步按钮来控制当前模型的渲染

  Line 1029: 3. 按下播放按钮，MeshView就从头开始渲染Mesh

  Line 1030: 4. 按下暂停按钮，MeshView就暂停当前的渲染，再按一次就继续渲染

  Line 1035: Git commit: hlsl-inter: add normal vector show of MeshView by MiniMax-M2.7.

  Line 1042: 1. 我们需要创建两个独立的MeshView显示UI，一个用于显示输入的vertex，已经创建，不需要修改

  Line 1043: 2. 另一个MeshView的UI用于显示用于显示executeVS执行完毕后的结果results

  Line 1044: 3. 请在HLSLInterpreter中创建一个新的MeshView对象，用于显示executeVS执行完毕后的results

  Line 1045: 4. 请把executeVS执行完毕后的results的三维坐标，normal向量，颜色信息送给MeshView显示

  Line 1050: 1. 给mesh_view.py的MeshView的UI界面创建两个显示窗口，一个用于输入的vertex显示，一个用于executeVS输出的results显示

  Line 1051: 2. HLSLInterpreter不需要创建单独的_result_mesh_view来显示executeVS的results，直接使用MeshView.set_output_vertices来传入results并显示

  Line 1062: Git commit: hlsl-inter: add user input 'o' and 'r'. 'o' means to open MeshView UI again. 'r' means to rerun executeVS again. by MiniMax-M2.7.

  Line 1064: 1. 如果MeshView UI已经关闭，用户输入o就重新打开UI

  Line 1067: MeshView.close只是隐藏UI界面，用户输入'o'后重新显示UI

  Line 1068: MeshView UI的真正销毁放在MeshView对象被销毁时执行

  Line 1156: 5. MeshView显示输入顶点和results的窗口的顶点数据从这个顶点pool来获得

  Line 1175: Git commit: hlsl-inter: move MeshView UI to a separate thread. by MiniMax-M2.7.

  Line 1176: 请创建一个单独的线程用于运行MeshView，使得MeshView的显示不阻挡主线程的执行

  Line 1182: Git commit: hlsl-inter: fix MeshView close crash if mainloop had have exited. by MiniMax-M2.7.

  Line 1202: MeshView下面的状态栏显示了input顶点数量，output顶点数量，topology类型，zoom缩放的倍数，


C:\Development\Graphics\irrlicht-1.8.3\Sessions\hlsl-interpreter\hlsl-step59-fix-MeshView-close-crash.md:
  Line 1: # MeshView独立线程实现

  Line 11: 请创建一个单独的线程用于运行MeshView，使得MeshView的显示不阻挡主线程的执行

  Line 21: The user is asking me to modify the MeshView class to run in a separate thread, where:

  Line 27: Let me first find the MeshView file to understand its current implementation.

  Line 34:   "pattern": "**/*MeshView*"

  Line 40: C:\Development\Graphics\irrlicht-1.8.3\Sessions\hlsl-interpreter\hlsl-step50-add-user-input-open-MeshView.md

  Line 41: C:\Development\Graphics\irrlicht-1.8.3\Sessions\hlsl-interpreter\hlsl-step41-Create-MeshView-GUI.md

  Line 42: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CGUIMeshViewer.cpp

  Line 43: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\MacOSX\MacOSX.xcodeproj\xcuserdata\yinghuang.xcuserdatad\xcschemes\09.MeshViewer.xcscheme

  Line 44: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CGUIMeshViewer.h

  Line 45: C:\Development\Graphics\irrlicht-1.8.3\include\IGUIMeshViewer.h

  Line 46: C:\Development\Graphics\irrlicht-1.8.3\examples\09.Meshviewer\MeshViewer.dev

  Line 55: Let me look at the MeshView-related files. The user seems to be working with a Python-based mesh viewer (based on the tk.Tk() reference). Let me check the Sessions folder for relevant files, as well as looking for Python files.

  Line 93:   "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\Sessions\\hlsl-interpreter\\hlsl-step50-add-user-input-open-MeshView.md"

  Line 99: <path>C:\Development\Graphics\irrlicht-1.8.3\Sessions\hlsl-interpreter\hlsl-step50-add-user-input-open-MeshView.md</path>

  Line 113: 12: 1. 如果MeshView UI已经关闭，用户输入o就重新打开UI

  Line 124: 23: 1. If the MeshView UI has been closed, pressing 'o' should reopen the UI

  Line 178: 77: 12:     from mesh_view import MeshView, VertexData

  Line 621: 520: 455:         self._mesh_view = None                               # MeshView实例(用于显示输入和输出)

  Line 622: 521: 456:         self._mesh_view_enabled = False                      # 是否启用MeshView

  Line 634: 533: 468:         启用或禁用MeshView

  Line 635: 534: 469:         enable: 是否启用MeshView

  Line 638: 537: 472:             self.log_output("Warning: MeshView not available (tkinter may not be installed)")

  Line 642: 541: 476:             self._mesh_view = MeshView(title="HLSL Interpreter - Input/Output Mesh")

  Line 644: 543: 478:         self.log_output(f"MeshView {'enabled' if enable else 'disabled'}")

  Line 1303: 2: MeshView - 3D Mesh Visualization Tool

  Line 1340: 39: class MeshView:

  Line 2059: 758:         显示MeshView窗口（双窗口：左侧输入，右侧输出）

  Line 2196: Now I understand the code. The user wants me to modify the `MeshView` class to run in a separate thread so that `tk.Tk()` and `self._root.mainloop()` both run in that thread, not blocking the main thread.

  Line 2206: Let me modify the `MeshView` class to run in a separate thread.

  Line 2208: 我将修改 `MeshView` 类，让 `tk.Tk()` 和 `mainloop()` 在单独线程中运行。

  Line 2217:   "oldString": "class MeshView:\n    \"\"\"\n    3D Mesh可视化工具\n    用于显示顶点着色器输入输出的mesh效果\n    支持双窗口显示：左侧为输入顶点，右侧为输出结果\n    \"\"\"\n    def __init__(self, vertices: List[VertexData] = None, primitive_topology: int = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, title: str = \"Mesh View\"):\n        self.input_vertices = vertices if vertices else []\n        self.output_vertices = []\n        self.primitive_topology = primitive_topology\n        self.title = title\n        self._root = None\n        self._input_canvas = None\n        self._output_canvas = None\n        self._running = False\n        self._input_rotation_x = MESH_VIEW_ROTATION_INIT_X\n        self._input_rotation_y = MESH_VIEW_ROTATION_INIT_Y\n        self._output_rotation_x = MESH_VIEW_ROTATION_INIT_X\n        self._output_rotation_y = MESH_VIEW_ROTATION_INIT_Y\n        self._input_scale = MESH_VIEW_SCALE_INIT\n        self._output_scale = MESH_VIEW_SCALE_INIT\n        self._input_offset_x = MESH_VIEW_OFFSET_X\n        self._input_offset_y = MESH_VIEW_OFFSET_Y\n        self._output_offset_x = MESH_VIEW_OFFSET_X\n        self._output_offset_y = MESH_VIEW_OFFSET_Y\n        self._last_mouse = None\n        self._info_label = None\n        self._input_bounds = None\n        self._output_bounds = None\n        self._current_index = 0\n        self._is_playing = False\n        self._is_paused = False\n        self._animation_job = None\n        self._animation_interval = 100\n        self._play_btn = None\n        self._pause_btn = None\n        self._next_btn = None\n        self._prev_btn = None\n        self._step_label = None\n        self._show_normals = False\n        self._normals_var = None\n        self._active_view_var = None\n        self._load_animation_config()",

  Line 2218:   "newString": "class MeshView:\n    \"\"\"\n    3D Mesh可视化工具\n    用于显示顶点着色器输入输出的mesh效果\n    支持双窗口显示：左侧为输入顶点，右侧为输出结果\n    \"\"\"\n    def __init__(self, vertices: List[VertexData] = None, primitive_topology: int = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, title: str = \"Mesh View\"):\n        self.input_vertices = vertices if vertices else []\n        self.output_vertices = []\n        self.primitive_topology = primitive_topology\n        self.title = title\n        self._root = None\n        self._input_canvas = None\n        self._output_canvas = None\n        self._running = False\n        self._input_rotation_x = MESH_VIEW_ROTATION_INIT_X\n        self._input_rotation_y = MESH_VIEW_ROTATION_INIT_Y\n        self._output_rotation_x = MESH_VIEW_ROTATION_INIT_X\n        self._output_rotation_y = MESH_VIEW_ROTATION_INIT_Y\n        self._input_scale = MESH_VIEW_SCALE_INIT\n        self._output_scale = MESH_VIEW_SCALE_INIT\n        self._input_offset_x = MESH_VIEW_OFFSET_X\n        self._input_offset_y = MESH_VIEW_OFFSET_Y\n        self._output_offset_x = MESH_VIEW_OFFSET_X\n        self._output_offset_y = MESH_VIEW_OFFSET_Y\n        self._last_mouse = None\n        self._info_label = None\n        self._input_bounds = None\n        self._output_bounds = None\n        self._current_index = 0\n        self._is_playing = False\n        self._is_paused = False\n        self._animation_job = None\n        self._animation_interval = 100\n        self._play_btn = None\n        self._pause_btn = None\n        self._next_btn = None\n        self._prev_btn = None\n        self._step_label = None\n        self._show_normals = False\n        self._normals_var = None\n        self._active_view_var = None\n        self._load_animation_config()\n        self._gui_thread = None\n        self._gui_ready_event = threading.Event()\n        self._start_gui_thread()"

  Line 2266:   "newString": "def show(self, blocking: bool = False):\n        \"\"\"\n        显示MeshView窗口（双窗口：左侧输入，右侧输出）\n        blocking: 如果为True，则阻塞直到窗口关闭（已废弃，仅为兼容）\n        \"\"\"\n        if self._root is None:\n            self._gui_ready_event.wait()\n        if self._root:\n            self._root.deiconify()\n            self._draw_mesh()\n\n    def _create_ui(self):\n        \"\"\"创建UI组件（在GUI线程中调用）\"\"\"",

  Line 2267:   "oldString": "def show(self, blocking: bool = False):\n        \"\"\"\n        显示MeshView窗口（双窗口：左侧输入，右侧输出）\n        blocking: 如果为True，则阻塞直到窗口关闭\n        \"\"\"\n        if self._root is not None:\n            self._root.deiconify()\n            return\n\n        self._root = tk.Tk()\n        self._root.title(self.title)\n        self._root.geometry(\"1400x700\")\n\n        self._active_view_var = tk.BooleanVar(value=True)"

  Line 2324: 779:         显示MeshView窗口（双窗口：左侧输入，右侧输出）

  Line 2920:   "newString": "def show(self, blocking: bool = False):\n        \"\"\"\n        显示MeshView窗口（双窗口：左侧输入，右侧输出）\n        blocking: 如果为True，则阻塞直到窗口关闭（已废弃，仅为兼容）\n        \"\"\"\n        if self._root is None:\n            self._gui_ready_event.wait()\n        if self._root:\n            self._root.deiconify()\n            self._schedule_draw()\n\n    def _schedule_draw(self):\n        \"\"\"在主线程中调度绘制\"\"\"\n        if self._root:\n            self._root.after(0, self._draw_mesh)\n\n    def _create_ui(self):",

  Line 2921:   "oldString": "def show(self, blocking: bool = False):\n        \"\"\"\n        显示MeshView窗口（双窗口：左侧输入，右侧输出）\n        blocking: 如果为True，则阻塞直到窗口关闭（已废弃，仅为兼容）\n        \"\"\"\n        if self._root is None:\n            self._gui_ready_event.wait()\n        if self._root:\n            self._root.deiconify()\n            self._draw_mesh()\n\n    def _schedule_draw(self):\n        \"\"\"在主线程中调度绘制\"\"\"\n        if self._root:\n            self._root.after(0, self._draw_mesh)\n\n    def _create_ui(self):"

  Line 3069: 781:         显示MeshView窗口（双窗口：左侧输入，右侧输出）

  Line 3544: 782:         显示MeshView窗口（双窗口：左侧输入，右侧输出）

  Line 3617: 782:         显示MeshView窗口（双窗口：左侧输入，右侧输出）

  Line 4060: The issue is that when MeshView is created, the GUI thread is started in `__init__`, which means tk.Tk() is created before the main thread's event loop is running. However, since the thread is daemon, it won't block the main thread.

  Line 4064: Actually, I just realized the real issue: when `MeshView` is created in `__init__`, it immediately starts the GUI thread and creates `tk.Tk()`. This should be fine since it's a separate thread.

  Line 4072: 1. `MeshView.__init__()` is called

  Line 4081: So by the time `MeshView()` constructor returns, the GUI thread is already running with `tk.Tk()` created and `mainloop()` started.

  Line 4093: The `enable_mesh_view` method checks if `_mesh_view` exists before using it, so it will create a new `MeshView` instance if needed. However, there's a potential race condition: the `MeshView.__init__` starts the GUI thread but the main thread could call `show_input_mesh` before the GUI is ready.

  Line 4099: However, there might be a timing issue if the code is calling `show()` without first checking that the MeshView instance has been properly created and the GUI thread is ready.

  Line 4109:   "command": "python -c \"import sys; sys.path.insert(0, r'C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter'); from mesh_view import MeshView; print('Import successful')\"",

  Line 4110:   "description": "Test MeshView import"

  Line 4151: MeshView now runs tk.Tk() and mainloop() in a separate thread. The key changes are:

  Line 4163: 完成。MeshView现在在单独的线程中运行tkinter：

  Line 4171: 主线程不再被MeshView的显示阻挡。


C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\mesh_view.py:
  Line 2: MeshView - 3D Mesh Visualization Tool

  Line 39: class MeshView:

  Line 864:         显示MeshView窗口（双窗口：左侧输入，右侧输出）


C:\Development\Graphics\irrlicht-1.8.3\Sessions\hlsl-interpreter\hlsl-step58-move-tk-UI-to-separate-thread.md:
  Line 1: # MeshView独立线程实现

  Line 11: 请创建一个单独的线程用于运行MeshView，使得MeshView的显示不阻挡主线程的执行

  Line 21: The user is asking me to modify the MeshView class to run in a separate thread, where:

  Line 27: Let me first find the MeshView file to understand its current implementation.

  Line 34:   "pattern": "**/*MeshView*"

  Line 40: C:\Development\Graphics\irrlicht-1.8.3\Sessions\hlsl-interpreter\hlsl-step50-add-user-input-open-MeshView.md

  Line 41: C:\Development\Graphics\irrlicht-1.8.3\Sessions\hlsl-interpreter\hlsl-step41-Create-MeshView-GUI.md

  Line 42: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CGUIMeshViewer.cpp

  Line 43: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\MacOSX\MacOSX.xcodeproj\xcuserdata\yinghuang.xcuserdatad\xcschemes\09.MeshViewer.xcscheme

  Line 44: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CGUIMeshViewer.h

  Line 45: C:\Development\Graphics\irrlicht-1.8.3\include\IGUIMeshViewer.h

  Line 46: C:\Development\Graphics\irrlicht-1.8.3\examples\09.Meshviewer\MeshViewer.dev

  Line 55: Let me look at the MeshView-related files. The user seems to be working with a Python-based mesh viewer (based on the tk.Tk() reference). Let me check the Sessions folder for relevant files, as well as looking for Python files.

  Line 93:   "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\Sessions\\hlsl-interpreter\\hlsl-step50-add-user-input-open-MeshView.md"

  Line 99: <path>C:\Development\Graphics\irrlicht-1.8.3\Sessions\hlsl-interpreter\hlsl-step50-add-user-input-open-MeshView.md</path>

  Line 113: 12: 1. 如果MeshView UI已经关闭，用户输入o就重新打开UI

  Line 124: 23: 1. If the MeshView UI has been closed, pressing 'o' should reopen the UI

  Line 178: 77: 12:     from mesh_view import MeshView, VertexData

  Line 621: 520: 455:         self._mesh_view = None                               # MeshView实例(用于显示输入和输出)


(Results truncated: showing 100 of 952 matches (852 hidden). Consider using a more specific path or pattern.)
```

**Tool: glob**

**Input:**
```json
{
  "pattern": "**/MeshView*"
}
```

**Output:**
```
C:\Development\Graphics\irrlicht-1.8.3\examples\09.Meshviewer\MeshViewer.dev
```

---

## Assistant (Build · MiniMax-M2.7 · 11.6s)

_Thinking:_

Based on the grep results, I can see the MeshView is a Python class in `C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\mesh_view.py`. Let me read this file to find the status bar implementation where it shows input vertices, output vertices, topology type, and zoom scale.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\mesh_view.py"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\mesh_view.py</path>
<type>file</type>
<content>
1: """
2: MeshView - 3D Mesh Visualization Tool
3: 类似于RenderDoc的mesh view功能，用于显示顶点组成的mesh
4: """
5: import tkinter as tk
6: from tkinter import ttk
7: import threading
8: import math
9: import json
10: import os
11: from typing import List, Tuple, Optional
12: 
13: 
14: D3D_PRIMITIVE_TOPOLOGY_UNDEFINED = 0
15: D3D_PRIMITIVE_TOPOLOGY_POINTLIST = 1
16: D3D_PRIMITIVE_TOPOLOGY_LINELIST = 2
17: D3D_PRIMITIVE_TOPOLOGY_LINESTRIP = 3
18: D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST = 4
19: D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP = 5
20: D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN = 6
21: 
22: MESH_VIEW_ROTATION_INIT_X = 0
23: MESH_VIEW_ROTATION_INIT_Y = 0
24: MESH_VIEW_SCALE_INIT = 0.2
25: MESH_VIEW_MIN_SCALE = 0.01
26: MESH_VIEW_MAX_SCALE = 50
27: MESH_VIEW_OFFSET_X = 0
28: MESH_VIEW_OFFSET_Y = 180
29: 
30: 
31: class VertexData:
32:     """顶点数据结构"""
33:     def __init__(self, position: List[float], normal: List[float] = None, color: List[float] = None):
34:         self.position = position
35:         self.normal = normal if normal else [0, 0, 1]
36:         self.color = color if color else [1, 1, 1, 1]
37: 
38: 
39: class MeshView:
40:     """
41:     3D Mesh可视化工具
42:     用于显示顶点着色器输入输出的mesh效果
43:     支持双窗口显示：左侧为输入顶点，右侧为输出结果
44:     """
45:     def __init__(self, vertices: List[VertexData] = None, primitive_topology: int = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, title: str = "Mesh View"):
46:         self.input_vertices = vertices if vertices else []
47:         self.output_vertices = []
48:         self.primitive_topology = primitive_topology
49:         self.title = title
50:         self._root = None
51:         self._input_canvas = None
52:         self._output_canvas = None
53:         self._running = False
54:         self._input_rotation_x = MESH_VIEW_ROTATION_INIT_X
55:         self._input_rotation_y = MESH_VIEW_ROTATION_INIT_Y
56:         self._output_rotation_x = MESH_VIEW_ROTATION_INIT_X
57:         self._output_rotation_y = MESH_VIEW_ROTATION_INIT_Y
58:         self._input_scale = MESH_VIEW_SCALE_INIT
59:         self._output_scale = MESH_VIEW_SCALE_INIT
60:         self._input_offset_x = MESH_VIEW_OFFSET_X
61:         self._input_offset_y = MESH_VIEW_OFFSET_Y
62:         self._output_offset_x = MESH_VIEW_OFFSET_X
63:         self._output_offset_y = MESH_VIEW_OFFSET_Y
64:         self._last_mouse = None
65:         self._info_label = None
66:         self._input_bounds = None
67:         self._output_bounds = None
68:         self._current_index = 0
69:         self._is_playing = False
70:         self._is_paused = False
71:         self._animation_job = None
72:         self._animation_interval = 100
73:         self._play_btn = None
74:         self._pause_btn = None
75:         self._next_btn = None
76:         self._prev_btn = None
77:         self._step_label = None
78:         self._show_normals = False
79:         self._normals_var = None
80:         self._active_view_var = None
81:         self._load_animation_config()
82:         self._gui_thread = None
83:         self._gui_thread_alive = True
84:         self._gui_ready_event = threading.Event()
85:         self._start_gui_thread()
86: 
87:     @property
88:     def vertices(self):
89:         return self.input_vertices
90: 
91:     @vertices.setter
92:     def vertices(self, value):
93:         self.input_vertices = value
94:         self._compute_input_bounds()
95: 
96:     def _load_animation_config(self):
97:         """从配置文件加载动画配置"""
98:         config_path = os.path.join(os.path.dirname(__file__), "animation_config.json")
99:         if os.path.exists(config_path):
100:             try:
101:                 with open(config_path, 'r') as f:
102:                     config = json.load(f)
103:                     self._animation_interval = config.get("interval_ms", 100)
104:             except:
105:                 self._animation_interval = 100
106:         else:
107:             self._animation_interval = 100
108: 
109:     def _start_gui_thread(self):
110:         """启动单独的GUI线程"""
111:         self._gui_thread = threading.Thread(target=self._gui_thread_run, daemon=True)
112:         self._gui_thread.start()
113: 
114:     def _gui_thread_run(self):
115:         """在单独线程中运行tkinter主循环"""
116:         self._root = tk.Tk()
117:         self._root.title(self.title)
118:         self._root.geometry("1400x700")
119:         self._setup_ui()
120:         self._gui_ready_event.set()
121:         self._root.mainloop()
122: 
123:     def _setup_ui(self):
124:         """设置UI组件（在GUI线程中调用）"""
125:         self._active_view_var = tk.BooleanVar(value=True)
126: 
127:         main_frame = ttk.Frame(self._root)
128:         main_frame.pack(fill=tk.BOTH, expand=True)
129: 
130:         controls_frame = ttk.Frame(main_frame)
131:         controls_frame.pack(side=tk.TOP, fill=tk.X, pady=2)
132: 
133:         ttk.Label(controls_frame, text="Active:").pack(side=tk.LEFT, padx=2)
134:         ttk.Radiobutton(controls_frame, text="Input", variable=self._active_view_var, value=True).pack(side=tk.LEFT, padx=2)
135:         ttk.Radiobutton(controls_frame, text="Output", variable=self._active_view_var, value=False).pack(side=tk.LEFT, padx=2)
136: 
137:         ttk.Separator(controls_frame, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=5)
138: 
139:         ttk.Label(controls_frame, text="Zoom:").pack(side=tk.LEFT, padx=2)
140:         ttk.Button(controls_frame, text="+", width=3, command=self._zoom_in).pack(side=tk.LEFT, padx=1)
141:         ttk.Button(controls_frame, text="-", width=3, command=self._zoom_out).pack(side=tk.LEFT, padx=1)
142: 
143:         ttk.Label(controls_frame, text="Rotate:").pack(side=tk.LEFT, padx=5)
144:         ttk.Button(controls_frame, text="↺", width=3, command=self._rotate_ccw).pack(side=tk.LEFT, padx=1)
145:         ttk.Button(controls_frame, text="↻", width=3, command=self._rotate_cw).pack(side=tk.LEFT, padx=1)
146:         ttk.Button(controls_frame, text="↑", width=3, command=self._rotate_up).pack(side=tk.LEFT, padx=1)
147:         ttk.Button(controls_frame, text="↓", width=3, command=self._rotate_down).pack(side=tk.LEFT, padx=1)
148: 
149:         ttk.Label(controls_frame, text="Pan:").pack(side=tk.LEFT, padx=5)
150:         ttk.Button(controls_frame, text="◀", width=3, command=self._pan_left).pack(side=tk.LEFT, padx=1)
151:         ttk.Button(controls_frame, text="▶", width=3, command=self._pan_right).pack(side=tk.LEFT, padx=1)
152:         ttk.Button(controls_frame, text="▲", width=3, command=self._pan_up).pack(side=tk.LEFT, padx=1)
153:         ttk.Button(controls_frame, text="▼", width=3, command=self._pan_down).pack(side=tk.LEFT, padx=1)
154: 
155:         ttk.Button(controls_frame, text="Reset", command=self._reset_view).pack(side=tk.LEFT, padx=5)
156: 
157:         self._normals_var = tk.BooleanVar(value=False)
158:         ttk.Checkbutton(controls_frame, text="Show Normals", variable=self._normals_var,
159:                         command=self._toggle_normals).pack(side=tk.LEFT, padx=5)
160: 
161:         anim_frame = ttk.Frame(controls_frame)
162:         anim_frame.pack(side=tk.LEFT, padx=10)
163:         ttk.Label(anim_frame, text="Animation:").pack(side=tk.LEFT, padx=2)
164:         self._play_btn = ttk.Button(anim_frame, text="Play", width=5, command=self._play_animation)
165:         self._play_btn.pack(side=tk.LEFT, padx=1)
166:         self._pause_btn = ttk.Button(anim_frame, text="Pause", width=5, command=self._pause_animation, state=tk.DISABLED)
167:         self._pause_btn.pack(side=tk.LEFT, padx=1)
168:         self._prev_btn = ttk.Button(anim_frame, text="Prev", width=5, command=self._prev_step, state=tk.DISABLED)
169:         self._prev_btn.pack(side=tk.LEFT, padx=1)
170:         self._next_btn = ttk.Button(anim_frame, text="Next", width=5, command=self._next_step, state=tk.DISABLED)
171:         self._next_btn.pack(side=tk.LEFT, padx=1)
172:         self._step_label = ttk.Label(anim_frame, text="Step: 0/0", width=12)
173:         self._step_label.pack(side=tk.LEFT, padx=5)
174: 
175:         ttk.Button(controls_frame, text="Close", command=self.hide).pack(side=tk.RIGHT, padx=5)
176: 
177:         canvas_frame = ttk.Frame(main_frame)
178:         canvas_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, pady=2)
179: 
180:         input_frame = ttk.LabelFrame(canvas_frame, text="Input Vertices", padding=5)
181:         input_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 5))
182:         self._input_canvas = tk.Canvas(input_frame, bg="#1a1a2e", width=600, height=520)
183:         self._input_canvas.pack(fill=tk.BOTH, expand=True)
184: 
185:         output_frame = ttk.LabelFrame(canvas_frame, text="Output (VS Result)", padding=5)
186:         output_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(5, 0))
187:         self._output_canvas = tk.Canvas(output_frame, bg="#1a1a2e", width=600, height=520)
188:         self._output_canvas.pack(fill=tk.BOTH, expand=True)
189: 
190:         self._input_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_input(e))
191:         self._input_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_input(e))
192:         self._input_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
193:         self._input_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_input(e))
194: 
195:         self._output_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_output(e))
196:         self._output_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_output(e))
197:         self._output_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
198:         self._output_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_output(e))
199: 
200:         self._root.bind("<Configure>", lambda e: self._on_resize(e))
201: 
202:         self._info_label = ttk.Label(main_frame, text="Input: 0 vertices | Output: 0 vertices | Topology: None", font=("Consolas", 10))
203:         self._info_label.pack(side=tk.BOTTOM, fill=tk.X, pady=2)
204: 
205:         self._draw_mesh()
206:         self._update_step_label()
207:         self._running = True
208: 
209:     def set_vertices(self, vertices: List[VertexData]):
210:         """设置顶点数据（输入顶点）"""
211:         self.input_vertices = vertices
212:         self._compute_input_bounds()
213: 
214:     def set_input_vertices(self, vertices: List[VertexData]):
215:         """设置输入顶点数据"""
216:         self.input_vertices = vertices
217:         self._compute_input_bounds()
218: 
219:     def set_output_vertices(self, vertices: List[VertexData]):
220:         """设置输出顶点数据（executeVS结果）"""
221:         self.output_vertices = vertices
222:         self._compute_output_bounds()
223: 
224:     def set_primitive_topology(self, primitive_topology: int):
225:         """设置图元拓扑类型"""
226:         self.primitive_topology = primitive_topology
227: 
228:     def clear(self):
229:         """清空顶点数据"""
230:         self.input_vertices = []
231:         self.output_vertices = []
232:         self._input_bounds = None
233:         self._output_bounds = None
234:         self._current_index = 0
235:         self._is_playing = False
236:         self._is_paused = False
237:         if self._animation_job and self._root:
238:             self._root.after_cancel(self._animation_job)
239:             self._animation_job = None
240:         self._update_button_states()
241: 
242:     def add_vertex(self, position: List[float], normal: List[float] = None, color: List[float] = None):
243:         """添加单个顶点到输入"""
244:         self.input_vertices.append(VertexData(position, normal, color))
245:         self._compute_input_bounds()
246: 
247:     def add_input_vertex(self, position: List[float], normal: List[float] = None, color: List[float] = None):
248:         """添加单个输入顶点"""
249:         self.input_vertices.append(VertexData(position, normal, color))
250:         self._compute_input_bounds()
251: 
252:     def add_output_vertex(self, position: List[float], normal: List[float] = None, color: List[float] = None):
253:         """添加单个输出顶点"""
254:         self.output_vertices.append(VertexData(position, normal, color))
255:         self._compute_output_bounds()
256: 
257:     def set_input_data(self, positions: List[List[float]], normals: List[List[float]] = None, colors: List[List[float]] = None):
258:         """
259:         设置输入数据
260:         positions: 顶点位置列表 [[x,y,z], [x,y,z], ...]
261:         normals: 法线列表 [[x,y,z], [x,y,z], ...]
262:         colors: 颜色列表 [[r,g,b,a], [r,g,b,a], ...]
263:         """
264:         self.input_vertices = []
265:         for i, pos in enumerate(positions):
266:             normal = normals[i] if normals and i < len(normals) else None
267:             color = colors[i] if colors and i < len(colors) else None
268:             self.input_vertices.append(VertexData(pos, normal, color))
269:         self._compute_input_bounds()
270: 
271:     def set_output_data(self, positions: List[List[float]], normals: List[List[float]] = None, colors: List[List[float]] = None):
272:         """
273:         设置输出数据（executeVS结果）
274:         positions: 顶点位置列表 [[x,y,z], [x,y,z], ...]
275:         normals: 法线列表 [[x,y,z], [x,y,z], ...]
276:         colors: 颜色列表 [[r,g,b,a], [r,g,b,a], ...]
277:         """
278:         self.output_vertices = []
279:         for i, pos in enumerate(positions):
280:             normal = normals[i] if normals and i < len(normals) else None
281:             color = colors[i] if colors and i < len(colors) else None
282:             self.output_vertices.append(VertexData(pos, normal, color))
283:         self._compute_output_bounds()
284: 
285:     def _compute_input_bounds(self):
286:         """计算输入顶点边界框"""
287:         if not self.input_vertices:
288:             self._input_bounds = None
289:             return
290: 
291:         min_x = min_y = min_z = float('inf')
292:         max_x = max_y = max_z = float('-inf')
293: 
294:         for v in self.input_vertices:
295:             x, y, z = v.position[0], v.position[1], v.position[2]
296:             min_x = min(min_x, x)
297:             max_x = max(max_x, x)
298:             min_y = min(min_y, y)
299:             max_y = max(max_y, y)
300:             min_z = min(min_z, z)
301:             max_z = max(max_z, z)
302: 
303:         center = [(min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2]
304:         size = max(max_x - min_x, max_y - min_y, max_z - min_z)
305:         if size < 0.001:
306:             size = 1
307: 
308:         self._input_bounds = (center, size)
309: 
310:     def _compute_output_bounds(self):
311:         """计算输出顶点边界框"""
312:         if not self.output_vertices:
313:             self._output_bounds = None
314:             return
315: 
316:         min_x = min_y = min_z = float('inf')
317:         max_x = max_y = max_z = float('-inf')
318: 
319:         for v in self.output_vertices:
320:             x, y, z = v.position[0], v.position[1], v.position[2]
321:             min_x = min(min_x, x)
322:             max_x = max(max_x, x)
323:             min_y = min(min_y, y)
324:             max_y = max(max_y, y)
325:             min_z = min(min_z, z)
326:             max_z = max(max_z, z)
327: 
328:         center = [(min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2]
329:         size = max(max_x - min_x, max_y - min_y, max_z - min_z)
330:         if size < 0.001:
331:             size = 1
332: 
333:         self._output_bounds = (center, size)
334: 
335:     def _transform_vertex_input(self, v: List[float]) -> Tuple[float, float, float]:
336:         """应用旋转变换到输入顶点"""
337:         x, y, z = v[0], v[1], v[2]
338: 
339:         ang_x = math.radians(self._input_rotation_x)
340:         ang_y = math.radians(self._input_rotation_y)
341: 
342:         cos_x, sin_x = math.cos(ang_x), math.sin(ang_x)
343:         cos_y, sin_y = math.cos(ang_y), math.sin(ang_y)
344: 
345:         y1 = y * cos_x - z * sin_x
346:         z1 = y * sin_x + z * cos_x
347:         x2 = x * cos_y + z1 * sin_y
348:         z2 = -x * sin_y + z1 * cos_y
349: 
350:         return x2, y1, z2
351: 
352:     def _transform_vertex_output(self, v: List[float]) -> Tuple[float, float, float]:
353:         """应用旋转变换到输出顶点"""
354:         x, y, z = v[0], v[1], v[2]
355: 
356:         ang_x = math.radians(self._output_rotation_x)
357:         ang_y = math.radians(self._output_rotation_y)
358: 
359:         cos_x, sin_x = math.cos(ang_x), math.sin(ang_x)
360:         cos_y, sin_y = math.cos(ang_y), math.sin(ang_y)
361: 
362:         y1 = y * cos_x - z * sin_x
363:         z1 = y * sin_x + z * cos_x
364:         x2 = x * cos_y + z1 * sin_y
365:         z2 = -x * sin_y + z1 * cos_y
366: 
367:         return x2, y1, z2
368: 
369:     def _project_input(self, v: Tuple[float, float, float], width: float, height: float) -> Tuple[float, float]:
370:         """将3D点投影到输入画布"""
371:         x, y, z = v
372: 
373:         margin = 40
374:         usable_width = width - 2 * margin
375:         usable_height = height - 2 * margin
376: 
377:         scale = self._input_scale * min(usable_width, usable_height) / 2.0
378: 
379:         proj_x = x * scale + width / 2 + self._input_offset_x
380:         proj_y = -y * scale + height / 2 + self._input_offset_y
381:         return proj_x, proj_y
382: 
383:     def _project_output(self, v: Tuple[float, float, float], width: float, height: float) -> Tuple[float, float]:
384:         """将3D点投影到输出画布"""
385:         x, y, z = v
386: 
387:         margin = 40
388:         usable_width = width - 2 * margin
389:         usable_height = height - 2 * margin
390: 
391:         scale = self._output_scale * min(usable_width, usable_height) / 2.0
392: 
393:         proj_x = x * scale + width / 2 + self._output_offset_x
394:         proj_y = -y * scale + height / 2 + self._output_offset_y
395:         return proj_x, proj_y
396: 
397:     def _color_to_hex(self, color: List[float]) -> str:
398:         """将颜色列表转换为十六进制颜色字符串"""
399:         if len(color) >= 4:
400:             r = int(min(255, max(0, color[0] * 255)))
401:             g = int(min(255, max(0, color[1] * 255)))
402:             b = int(min(255, max(0, color[2] * 255)))
403:         elif len(color) >= 3:
404:             r = int(min(255, max(0, color[0] * 255)))
405:             g = int(min(255, max(0, color[1] * 255)))
406:             b = int(min(255, max(0, color[2] * 255)))
407:         else:
408:             r = g = b = 200
409:         return f'#{r:02x}{g:02x}{b:02x}'
410: 
411:     def _draw_mesh_wireframe_input(self, transformed: list, width: float, height: float):
412:         """绘制输入wireframe线框"""
413:         if self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
414:             for i in range(0, len(transformed) - 2, 3):
415:                 pts = transformed[i:i+3]
416:                 color = self._color_to_hex(pts[0][1])
417:                 proj_pts = [self._project_input(p, width, height) for p, c in pts]
418:                 self._input_canvas.create_line(proj_pts[0][0], proj_pts[0][1], proj_pts[1][0], proj_pts[1][1], fill=color, width=1)
419:                 self._input_canvas.create_line(proj_pts[1][0], proj_pts[1][1], proj_pts[2][0], proj_pts[2][1], fill=color, width=1)
420:                 self._input_canvas.create_line(proj_pts[2][0], proj_pts[2][1], proj_pts[0][0], proj_pts[0][1], fill=color, width=1)
421: 
422:         elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
423:             for i in range(len(transformed) - 2):
424:                 pts = transformed[i:i+3]
425:                 color = self._color_to_hex(pts[0][1])
426:                 proj_pts = [self._project_input(p, width, height) for p, c in pts]
427:                 self._input_canvas.create_line(proj_pts[0][0], proj_pts[0][1], proj_pts[1][0], proj_pts[1][1], fill=color, width=1)
428:                 self._input_canvas.create_line(proj_pts[1][0], proj_pts[1][1], proj_pts[2][0], proj_pts[2][1], fill=color, width=1)
429:                 self._input_canvas.create_line(proj_pts[2][0], proj_pts[2][1], proj_pts[0][0], proj_pts[0][1], fill=color, width=1)
430: 
431:         elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINELIST:
432:             for i in range(0, len(transformed) - 1, 2):
433:                 p1, c1 = transformed[i]
434:                 p2, c2 = transformed[i+1]
435:                 proj1 = self._project_input(p1, width, height)
436:                 proj2 = self._project_input(p2, width, height)
437:                 color = self._color_to_hex(c1)
438:                 self._input_canvas.create_line(proj1[0], proj1[1], proj2[0], proj2[1], fill=color, width=2)
439: 
440:         elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
441:             for i in range(len(transformed) - 1):
442:                 p1, c1 = transformed[i]
443:                 p2, c2 = transformed[i+1]
444:                 proj1 = self._project_input(p1, width, height)
445:                 proj2 = self._project_input(p2, width, height)
446:                 color = self._color_to_hex(c1)
447:                 self._input_canvas.create_line(proj1[0], proj1[1], proj2[0], proj2[1], fill=color, width=2)
448: 
449:         elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
450:             for p, c in transformed:
451:                 proj = self._project_input(p, width, height)
452:                 self._input_canvas.create_oval(proj[0]-4, proj[1]-4, proj[0]+4, proj[1]+4, fill=self._color_to_hex(c), outline='white')
453: 
454:         if self._show_normals:
455:             self._draw_normals_input(transformed, width, height)
456: 
457:     def _draw_mesh_wireframe_output(self, transformed: list, width: float, height: float):
458:         """绘制输出wireframe线框"""
459:         if self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
460:             for i in range(0, len(transformed) - 2, 3):
461:                 pts = transformed[i:i+3]
462:                 color = self._color_to_hex(pts[0][1])
463:                 proj_pts = [self._project_output(p, width, height) for p, c in pts]
464:                 self._output_canvas.create_line(proj_pts[0][0], proj_pts[0][1], proj_pts[1][0], proj_pts[1][1], fill=color, width=1)
465:                 self._output_canvas.create_line(proj_pts[1][0], proj_pts[1][1], proj_pts[2][0], proj_pts[2][1], fill=color, width=1)
466:                 self._output_canvas.create_line(proj_pts[2][0], proj_pts[2][1], proj_pts[0][0], proj_pts[0][1], fill=color, width=1)
467: 
468:         elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
469:             for i in range(len(transformed) - 2):
470:                 pts = transformed[i:i+3]
471:                 color = self._color_to_hex(pts[0][1])
472:                 proj_pts = [self._project_output(p, width, height) for p, c in pts]
473:                 self._output_canvas.create_line(proj_pts[0][0], proj_pts[0][1], proj_pts[1][0], proj_pts[1][1], fill=color, width=1)
474:                 self._output_canvas.create_line(proj_pts[1][0], proj_pts[1][1], proj_pts[2][0], proj_pts[2][1], fill=color, width=1)
475:                 self._output_canvas.create_line(proj_pts[2][0], proj_pts[2][1], proj_pts[0][0], proj_pts[0][1], fill=color, width=1)
476: 
477:         elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINELIST:
478:             for i in range(0, len(transformed) - 1, 2):
479:                 p1, c1 = transformed[i]
480:                 p2, c2 = transformed[i+1]
481:                 proj1 = self._project_output(p1, width, height)
482:                 proj2 = self._project_output(p2, width, height)
483:                 color = self._color_to_hex(c1)
484:                 self._output_canvas.create_line(proj1[0], proj1[1], proj2[0], proj2[1], fill=color, width=2)
485: 
486:         elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:
487:             for i in range(len(transformed) - 1):
488:                 p1, c1 = transformed[i]
489:                 p2, c2 = transformed[i+1]
490:                 proj1 = self._project_output(p1, width, height)
491:                 proj2 = self._project_output(p2, width, height)
492:                 color = self._color_to_hex(c1)
493:                 self._output_canvas.create_line(proj1[0], proj1[1], proj2[0], proj2[1], fill=color, width=2)
494: 
495:         elif self.primitive_topology == D3D_PRIMITIVE_TOPOLOGY_POINTLIST:
496:             for p, c in transformed:
497:                 proj = self._project_output(p, width, height)
498:                 self._output_canvas.create_oval(proj[0]-4, proj[1]-4, proj[0]+4, proj[1]+4, fill=self._color_to_hex(c), outline='white')
499: 
500:         if self._show_normals:
501:             self._draw_normals_output(transformed, width, height)
502: 
503:     def _draw_normals_input(self, transformed: list, width: float, height: float):
504:         """绘制输入顶点法线向量"""
505:         if not self.input_vertices:
506:             return
507: 
508:         normal_scale = 0.1 * (self._input_bounds[1] if self._input_bounds else 1.0)
509: 
510:         for i, (pos, color) in enumerate(transformed):
511:             if i < len(self.input_vertices):
512:                 normal = self.input_vertices[i].normal
513:                 if normal:
514:                     nx, ny, nz = normal[0], normal[1], normal[2]
515:                     length = (nx*nx + ny*ny + nz*nz) ** 0.5
516:                     if length > 0.0001:
517:                         nx, ny, nz = nx/length, ny/length, nz/length
518:                     end_pos = (
519:                         pos[0] + nx * normal_scale,
520:                         pos[1] + ny * normal_scale,
521:                         pos[2] + nz * normal_scale
522:                     )
523:                     start_proj = self._project_input(pos, width, height)
524:                     end_proj = self._project_input(end_pos, width, height)
525:                     r = int(min(255, max(0, (nx * 0.5 + 0.5) * 255)))
526:                     g = int(min(255, max(0, (ny * 0.5 + 0.5) * 255)))
527:                     b = int(min(255, max(0, (nz * 0.5 + 0.5) * 255)))
528:                     color_hex = f'#{r:02x}{g:02x}{b:02x}'
529:                     self._input_canvas.create_line(start_proj[0], start_proj[1], end_proj[0], end_proj[1],
530:                                              fill=color_hex, width=1)
531:                     self._input_canvas.create_oval(end_proj[0]-2, end_proj[1]-2, end_proj[0]+2, end_proj[1]+2,
532:                                              fill=color_hex, outline='')
533: 
534:     def _draw_normals_output(self, transformed: list, width: float, height: float):
535:         """绘制输出顶点法线向量"""
536:         if not self.output_vertices:
537:             return
538: 
539:         normal_scale = 0.1 * (self._output_bounds[1] if self._output_bounds else 1.0)
540: 
541:         for i, (pos, color) in enumerate(transformed):
542:             if i < len(self.output_vertices):
543:                 normal = self.output_vertices[i].normal
544:                 if normal:
545:                     nx, ny, nz = normal[0], normal[1], normal[2]
546:                     length = (nx*nx + ny*ny + nz*nz) ** 0.5
547:                     if length > 0.0001:
548:                         nx, ny, nz = nx/length, ny/length, nz/length
549:                     end_pos = (
550:                         pos[0] + nx * normal_scale,
551:                         pos[1] + ny * normal_scale,
552:                         pos[2] + nz * normal_scale
553:                     )
554:                     start_proj = self._project_output(pos, width, height)
555:                     end_proj = self._project_output(end_pos, width, height)
556:                     r = int(min(255, max(0, (nx * 0.5 + 0.5) * 255)))
557:                     g = int(min(255, max(0, (ny * 0.5 + 0.5) * 255)))
558:                     b = int(min(255, max(0, (nz * 0.5 + 0.5) * 255)))
559:                     color_hex = f'#{r:02x}{g:02x}{b:02x}'
560:                     self._output_canvas.create_line(start_proj[0], start_proj[1], end_proj[0], end_proj[1],
561:                                              fill=color_hex, width=1)
562:                     self._output_canvas.create_oval(end_proj[0]-2, end_proj[1]-2, end_proj[0]+2, end_proj[1]+2,
563:                                              fill=color_hex, outline='')
564: 
565:     def _draw_mesh(self):
566:         """绘制mesh到画布"""
567:         self._draw_mesh_animated(max(len(self.input_vertices), len(self.output_vertices)))
568: 
569:     def _draw_mesh_animated(self, count: int = None):
570:         """绘制动画mesh到画布，只渲染前count个元素"""
571:         if not self._input_canvas or not self._output_canvas:
572:             return
573: 
574:         if not self.input_vertices and not self.output_vertices:
575:             self._input_canvas.delete("all")
576:             self._output_canvas.delete("all")
577:             return
578: 
579:         self._input_canvas.delete("all")
580:         self._output_canvas.delete("all")
581:         
582:         input_width = int(self._input_canvas.cget('width'))
583:         input_height = int(self._input_canvas.cget('height'))
584:         output_width = int(self._output_canvas.cget('width'))
585:         output_height = int(self._output_canvas.cget('height'))
586: 
587:         input_transformed = []
588:         for v in self.input_vertices:
589:             p = self._transform_vertex_input(v.position)
590:             input_transformed.append((p, v.color))
591: 
592:         output_transformed = []
593:         for v in self.output_vertices:
594:             p = self._transform_vertex_output(v.position)
595:             output_transformed.append((p, v.color))
596: 
597:         if count is None:
598:             count = self._current_index + 1
599: 
600:         self._draw_mesh_wireframe_input(input_transformed[:count], input_width, input_height)
601:         self._draw_mesh_wireframe_output(output_transformed[:count], output_width, output_height)
602:         self._update_info()
603: 
604:     def _update_info(self):
605:         """更新信息标签"""
606:         if self._info_label:
607:             topo_names = {
608:                 D3D_PRIMITIVE_TOPOLOGY_UNDEFINED: "Undefined",
609:                 D3D_PRIMITIVE_TOPOLOGY_POINTLIST: "Point List",
610:                 D3D_PRIMITIVE_TOPOLOGY_LINELIST: "Line List",
611:                 D3D_PRIMITIVE_TOPOLOGY_LINESTRIP: "Line Strip",
612:                 D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST: "Triangle List",
613:                 D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: "Triangle Strip",
614:                 D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN: "Triangle Fan",
615:             }
616:             info = f"Input: {len(self.input_vertices)} vertices | Output: {len(self.output_vertices)} vertices | Topology: {topo_names.get(self.primitive_topology, 'Unknown')} | Zoom: {self._input_scale:.2f}x"
617:             self._info_label.config(text=info)
618: 
619:     def _on_mouse_drag_input(self, event):
620:         """处理输入画布鼠标拖动旋转"""
621:         if self._last_mouse:
622:             dx = event.x - self._last_mouse[0]
623:             dy = event.y - self._last_mouse[1]
624:             if self._active_view_var.get():
625:                 self._input_rotation_y += dx * 0.5
626:                 self._input_rotation_x += dy * 0.5
627:             self._draw_mesh()
628:         self._last_mouse = (event.x, event.y)
629: 
630:     def _on_mouse_drag_output(self, event):
631:         """处理输出画布鼠标拖动旋转"""
632:         if self._last_mouse:
633:             dx = event.x - self._last_mouse[0]
634:             dy = event.y - self._last_mouse[1]
635:             if not self._active_view_var.get():
636:                 self._output_rotation_y += dx * 0.5
637:                 self._output_rotation_x += dy * 0.5
638:             self._draw_mesh()
639:         self._last_mouse = (event.x, event.y)
640: 
641:     def _on_mouse_release(self, event):
642:         """处理鼠标释放"""
643:         self._last_mouse = None
644: 
645:     def _on_mouse_wheel_input(self, event):
646:         """处理输入画布鼠标滚轮缩放"""
647:         if self._active_view_var.get():
648:             if event.delta > 0:
649:                 self._input_scale *= 1.1
650:             else:
651:                 self._input_scale *= 0.9
652:             self._input_scale = max(MESH_VIEW_MIN_SCALE, min(MESH_VIEW_MAX_SCALE, self._input_scale))
653:         self._draw_mesh()
654: 
655:     def _on_mouse_wheel_output(self, event):
656:         """处理输出画布鼠标滚轮缩放"""
657:         if not self._active_view_var.get():
658:             if event.delta > 0:
659:                 self._output_scale *= 1.1
660:             else:
661:                 self._output_scale *= 0.9
662:             self._output_scale = max(MESH_VIEW_MIN_SCALE, min(MESH_VIEW_MAX_SCALE, self._output_scale))
663:         self._draw_mesh()
664: 
665:     def _on_resize(self, event):
666:         """处理窗口大小改变"""
667:         self._draw_mesh()
668: 
669:     def _zoom_in(self):
670:         """放大当前活动视图"""
671:         if self._active_view_var.get():
672:             self._input_scale *= 1.2
673:             self._input_scale = min(MESH_VIEW_MAX_SCALE, self._input_scale)
674:         else:
675:             self._output_scale *= 1.2
676:             self._output_scale = min(MESH_VIEW_MAX_SCALE, self._output_scale)
677:         self._draw_mesh()
678: 
679:     def _zoom_out(self):
680:         """缩小当前活动视图"""
681:         if self._active_view_var.get():
682:             self._input_scale *= 0.8
683:             self._input_scale = max(MESH_VIEW_MIN_SCALE, self._input_scale)
684:         else:
685:             self._output_scale *= 0.8
686:             self._output_scale = max(MESH_VIEW_MIN_SCALE, self._output_scale)
687:         self._draw_mesh()
688: 
689:     def _rotate_cw(self):
690:         """顺时针旋转当前活动视图"""
691:         if self._active_view_var.get():
692:             self._input_rotation_y += 15
693:         else:
694:             self._output_rotation_y += 15
695:         self._draw_mesh()
696: 
697:     def _rotate_ccw(self):
698:         """逆时针旋转当前活动视图"""
699:         if self._active_view_var.get():
700:             self._input_rotation_y -= 15
701:         else:
702:             self._output_rotation_y -= 15
703:         self._draw_mesh()
704: 
705:     def _rotate_up(self):
706:         """向上旋转当前活动视图"""
707:         if self._active_view_var.get():
708:             self._input_rotation_x -= 15
709:         else:
710:             self._output_rotation_x -= 15
711:         self._draw_mesh()
712: 
713:     def _rotate_down(self):
714:         """向下旋转当前活动视图"""
715:         if self._active_view_var.get():
716:             self._input_rotation_x += 15
717:         else:
718:             self._output_rotation_x += 15
719:         self._draw_mesh()
720: 
721:     def _pan_left(self):
722:         """向左平移当前活动视图"""
723:         if self._active_view_var.get():
724:             self._input_offset_x -= 20
725:         else:
726:             self._output_offset_x -= 20
727:         self._draw_mesh()
728: 
729:     def _pan_right(self):
730:         """向右平移当前活动视图"""
731:         if self._active_view_var.get():
732:             self._input_offset_x += 20
733:         else:
734:             self._output_offset_x += 20
735:         self._draw_mesh()
736: 
737:     def _pan_up(self):
738:         """向上平移当前活动视图"""
739:         if self._active_view_var.get():
740:             self._input_offset_y -= 20
741:         else:
742:             self._output_offset_y -= 20
743:         self._draw_mesh()
744: 
745:     def _pan_down(self):
746:         """向下平移当前活动视图"""
747:         if self._active_view_var.get():
748:             self._input_offset_y += 20
749:         else:
750:             self._output_offset_y += 20
751:         self._draw_mesh()
752: 
753:     def _reset_view(self):
754:         """重置当前活动视图"""
755:         if self._active_view_var.get():
756:             self._input_rotation_x = MESH_VIEW_ROTATION_INIT_X
757:             self._input_rotation_y = MESH_VIEW_ROTATION_INIT_Y
758:             self._input_scale = MESH_VIEW_SCALE_INIT
759:             self._input_offset_x = MESH_VIEW_OFFSET_X
760:             self._input_offset_y = MESH_VIEW_OFFSET_Y
761:         else:
762:             self._output_rotation_x = MESH_VIEW_ROTATION_INIT_X
763:             self._output_rotation_y = MESH_VIEW_ROTATION_INIT_Y
764:             self._output_scale = MESH_VIEW_SCALE_INIT
765:             self._output_offset_x = MESH_VIEW_OFFSET_X
766:             self._output_offset_y = MESH_VIEW_OFFSET_Y
767:         self._draw_mesh()
768: 
769:     def _toggle_normals(self):
770:         """切换法线显示"""
771:         self._show_normals = self._normals_var.get()
772:         self._draw_mesh()
773: 
774:     def _get_active_view_props(self):
775:         """获取当前活动视图的属性引用"""
776:         if self._active_view_var.get():
777:             return self._input_rotation_x, self._input_rotation_y, self._input_scale, self._input_offset_x, self._input_offset_y
778:         else:
779:             return self._output_rotation_x, self._output_rotation_y, self._output_scale, self._output_offset_x, self._output_offset_y
780: 
781:     def _set_active_view_props(self, rot_x, rot_y, scale, offset_x, offset_y):
782:         """设置当前活动视图的属性"""
783:         if self._active_view_var.get():
784:             self._input_rotation_x, self._input_rotation_y, self._input_scale, self._input_offset_x, self._input_offset_y = rot_x, rot_y, scale, offset_x, offset_y
785:         else:
786:             self._output_rotation_x, self._output_rotation_y, self._output_scale, self._output_offset_x, self._output_offset_y = rot_x, rot_y, scale, offset_x, offset_y
787: 
788:     def _play_animation(self):
789:         """从开头开始播放动画"""
790:         if not self.input_vertices and not self.output_vertices:
791:             return
792:         self._current_index = 0
793:         self._is_playing = True
794:         self._is_paused = False
795:         self._update_button_states()
796:         self._run_animation_step()
797: 
798:     def _pause_animation(self):
799:         """暂停/继续动画"""
800:         if self._is_paused:
801:             self._is_paused = False
802:             self._run_animation_step()
803:         else:
804:             self._is_paused = True
805:         self._update_button_states()
806: 
807:     def _next_step(self):
808:         """渲染下一个顶点/线"""
809:         if not self.input_vertices and not self.output_vertices:
810:             return
811:         max_index = max(len(self.input_vertices), len(self.output_vertices)) - 1
812:         if self._current_index < max_index:
813:             self._current_index += 1
814:         self._draw_mesh_animated()
815:         self._update_step_label()
816:         self._update_button_states()
817: 
818:     def _prev_step(self):
819:         """回到上一个顶点/线"""
820:         if not self.input_vertices and not self.output_vertices:
821:             return
822:         if self._current_index > 0:
823:             self._current_index -= 1
824:         self._draw_mesh_animated()
825:         self._update_step_label()
826:         self._update_button_states()
827: 
828:     def _run_animation_step(self):
829:         """执行动画单步"""
830:         if not self._is_playing or self._is_paused:
831:             return
832:         if not self._root:
833:             return
834:         max_index = max(len(self.input_vertices), len(self.output_vertices)) - 1
835:         if self._current_index < max_index:
836:             self._current_index += 1
837:             self._draw_mesh_animated()
838:             self._update_step_label()
839:             self._animation_job = self._root.after(self._animation_interval, self._run_animation_step)
840:         else:
841:             self._is_playing = False
842:             self._update_button_states()
843: 
844:     def _update_button_states(self):
845:         """更新按钮状态"""
846:         if self._play_btn:
847:             self._play_btn.config(state=tk.NORMAL if not self._is_playing else tk.DISABLED)
848:         if self._pause_btn:
849:             self._pause_btn.config(state=tk.NORMAL if self._is_playing or self._current_index > 0 else tk.DISABLED)
850:         can_step = self._is_paused or self._current_index > 0
851:         if self._next_btn:
852:             self._next_btn.config(state=tk.NORMAL if can_step else tk.DISABLED)
853:         if self._prev_btn:
854:             self._prev_btn.config(state=tk.NORMAL if can_step and self._current_index > 0 else tk.DISABLED)
855: 
856:     def _update_step_label(self):
857:         """更新步骤显示"""
858:         if self._step_label:
859:             max_count = max(len(self.input_vertices), len(self.output_vertices))
860:             self._step_label.config(text=f"Step: {self._current_index + 1}/{max_count}")
861: 
862:     def show(self, blocking: bool = False):
863:         """
864:         显示MeshView窗口（双窗口：左侧输入，右侧输出）
865:         blocking: 如果为True，则阻塞直到窗口关闭（已废弃，仅为兼容）
866:         """
867:         if self._root is None:
868:             self._gui_ready_event.wait()
869:         if self._root:
870:             self._root.deiconify()
871:             self._schedule_draw()
872: 
873:     def _schedule_draw(self):
874:         """在主线程中调度绘制"""
875:         if self._root:
876:             self._root.after(0, self._draw_mesh)
877: 
878:     def _create_ui(self):
879:         """创建UI组件（在GUI线程中调用）"""
880: 
881:         main_frame = ttk.Frame(self._root)
882:         main_frame.pack(fill=tk.BOTH, expand=True)
883: 
884:         controls_frame = ttk.Frame(main_frame)
885:         controls_frame.pack(side=tk.TOP, fill=tk.X, pady=2)
886: 
887:         ttk.Label(controls_frame, text="Active:").pack(side=tk.LEFT, padx=2)
888:         ttk.Radiobutton(controls_frame, text="Input", variable=self._active_view_var, value=True).pack(side=tk.LEFT, padx=2)
889:         ttk.Radiobutton(controls_frame, text="Output", variable=self._active_view_var, value=False).pack(side=tk.LEFT, padx=2)
890: 
891:         ttk.Separator(controls_frame, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=5)
892: 
893:         ttk.Label(controls_frame, text="Zoom:").pack(side=tk.LEFT, padx=2)
894:         ttk.Button(controls_frame, text="+", width=3, command=self._zoom_in).pack(side=tk.LEFT, padx=1)
895:         ttk.Button(controls_frame, text="-", width=3, command=self._zoom_out).pack(side=tk.LEFT, padx=1)
896: 
897:         ttk.Label(controls_frame, text="Rotate:").pack(side=tk.LEFT, padx=5)
898:         ttk.Button(controls_frame, text="↺", width=3, command=self._rotate_ccw).pack(side=tk.LEFT, padx=1)
899:         ttk.Button(controls_frame, text="↻", width=3, command=self._rotate_cw).pack(side=tk.LEFT, padx=1)
900:         ttk.Button(controls_frame, text="↑", width=3, command=self._rotate_up).pack(side=tk.LEFT, padx=1)
901:         ttk.Button(controls_frame, text="↓", width=3, command=self._rotate_down).pack(side=tk.LEFT, padx=1)
902: 
903:         ttk.Label(controls_frame, text="Pan:").pack(side=tk.LEFT, padx=5)
904:         ttk.Button(controls_frame, text="◀", width=3, command=self._pan_left).pack(side=tk.LEFT, padx=1)
905:         ttk.Button(controls_frame, text="▶", width=3, command=self._pan_right).pack(side=tk.LEFT, padx=1)
906:         ttk.Button(controls_frame, text="▲", width=3, command=self._pan_up).pack(side=tk.LEFT, padx=1)
907:         ttk.Button(controls_frame, text="▼", width=3, command=self._pan_down).pack(side=tk.LEFT, padx=1)
908: 
909:         ttk.Button(controls_frame, text="Reset", command=self._reset_view).pack(side=tk.LEFT, padx=5)
910: 
911:         self._normals_var = tk.BooleanVar(value=False)
912:         ttk.Checkbutton(controls_frame, text="Show Normals", variable=self._normals_var,
913:                         command=self._toggle_normals).pack(side=tk.LEFT, padx=5)
914: 
915:         anim_frame = ttk.Frame(controls_frame)
916:         anim_frame.pack(side=tk.LEFT, padx=10)
917:         ttk.Label(anim_frame, text="Animation:").pack(side=tk.LEFT, padx=2)
918:         self._play_btn = ttk.Button(anim_frame, text="Play", width=5, command=self._play_animation)
919:         self._play_btn.pack(side=tk.LEFT, padx=1)
920:         self._pause_btn = ttk.Button(anim_frame, text="Pause", width=5, command=self._pause_animation, state=tk.DISABLED)
921:         self._pause_btn.pack(side=tk.LEFT, padx=1)
922:         self._prev_btn = ttk.Button(anim_frame, text="Prev", width=5, command=self._prev_step, state=tk.DISABLED)
923:         self._prev_btn.pack(side=tk.LEFT, padx=1)
924:         self._next_btn = ttk.Button(anim_frame, text="Next", width=5, command=self._next_step, state=tk.DISABLED)
925:         self._next_btn.pack(side=tk.LEFT, padx=1)
926:         self._step_label = ttk.Label(anim_frame, text="Step: 0/0", width=12)
927:         self._step_label.pack(side=tk.LEFT, padx=5)
928: 
929:         ttk.Button(controls_frame, text="Close", command=self.hide).pack(side=tk.RIGHT, padx=5)
930: 
931:         canvas_frame = ttk.Frame(main_frame)
932:         canvas_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True, pady=2)
933: 
934:         input_frame = ttk.LabelFrame(canvas_frame, text="Input Vertices", padding=5)
935:         input_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 5))
936:         self._input_canvas = tk.Canvas(input_frame, bg="#1a1a2e", width=600, height=520)
937:         self._input_canvas.pack(fill=tk.BOTH, expand=True)
938: 
939:         output_frame = ttk.LabelFrame(canvas_frame, text="Output (VS Result)", padding=5)
940:         output_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(5, 0))
941:         self._output_canvas = tk.Canvas(output_frame, bg="#1a1a2e", width=600, height=520)
942:         self._output_canvas.pack(fill=tk.BOTH, expand=True)
943: 
944:         self._input_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_input(e))
945:         self._input_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_input(e))
946:         self._input_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
947:         self._input_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_input(e))
948: 
949:         self._output_canvas.bind("<Button-1>", lambda e: self._on_mouse_drag_output(e))
950:         self._output_canvas.bind("<B1-Motion>", lambda e: self._on_mouse_drag_output(e))
951:         self._output_canvas.bind("<ButtonRelease-1>", lambda e: self._on_mouse_release(e))
952:         self._output_canvas.bind("<MouseWheel>", lambda e: self._on_mouse_wheel_output(e))
953: 
954:         self._root.bind("<Configure>", lambda e: self._on_resize(e))
955: 
956:         self._info_label = ttk.Label(main_frame, text="Input: 0 vertices | Output: 0 vertices | Topology: None", font=("Consolas", 10))
957:         self._info_label.pack(side=tk.BOTTOM, fill=tk.X, pady=2)
958: 
959:         self._draw_mesh()
960:         self._update_step_label()
961:         self._running = True
962: 
963:     def hide(self):
964:         """隐藏窗口"""
965:         if self._root:
966:             self._root.withdraw()
967: 
968:     def update(self):
969:         """更新显示（在GUI线程中执行）"""
970:         if self._root and self._running:
971:             self._root.after(0, self._draw_mesh)
972: 
973:     def _schedule_draw(self):
974:         """在主线程中调度绘制"""
975:         if self._root:
976:             self._root.after(0, self._draw_mesh)
977: 
978:     def is_visible(self) -> bool:
979:         """检查窗口是否可见"""
980:         return self._root is not None and self._running
981: 
982:     def close(self):
983:         """关闭窗口"""
984:         self._running = False
985:         if self._animation_job:
986:             if self._root:
987:                 try:
988:                     self._root.after_cancel(self._animation_job)
989:                 except:
990:                     pass
991:             self._animation_job = None
992:         if self._root:
993:             def _do_close():
994:                 try:
995:                     self._root.quit()
996:                     self._root.destroy()
997:                 except:
998:                     pass
999:                 self._root = None
1000:                 self._input_canvas = None
1001:                 self._output_canvas = None
1002:             try:
1003:                 self._root.after(0, _do_close)
1004:             except RuntimeError:
1005:                 self._gui_thread_alive = False
1006:                 self._root = None
1007:                 self._input_canvas = None
1008:                 self._output_canvas = None

(End of file - total 1008 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 13.7s)

_Thinking:_

The user wants me to:
1. Rename the zoom scale display from "zoom" to "input zoom" (since it represents the input window's zoom)
2. Add a "result zoom" (output zoom) display

Looking at line 616, I can see:
```python
info = f"Input: {len(self.input_vertices)} vertices | Output: {len(self.output_vertices)} vertices | Topology: {topo_names.get(self.primitive_topology, 'Unknown')} | Zoom: {self._input_scale:.2f}x"
```

I need to change this to show both input and output zoom values.


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\mesh_view.py",
  "newString": "info = f\"Input: {len(self.input_vertices)} vertices | Output: {len(self.output_vertices)} vertices | Topology: {topo_names.get(self.primitive_topology, 'Unknown')} | Input Zoom: {self._input_scale:.2f}x | Result Zoom: {self._output_scale:.2f}x\"",
  "oldString": "info = f\"Input: {len(self.input_vertices)} vertices | Output: {len(self.output_vertices)} vertices | Topology: {topo_names.get(self.primitive_topology, 'Unknown')} | Zoom: {self._input_scale:.2f}x\""
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 9.0s)

_Thinking:_

Now I need to check if there are any duplicate definitions of `_schedule_draw` and `_create_ui` methods. I noticed in the file that `_schedule_draw` appears twice (lines 873 and 973), and `_create_ui` at line 878 seems to be a duplicate of `_setup_ui` at line 123. 

Actually looking more closely:
- `_setup_ui` is called from `_gui_thread_run` (line 119)
- `_create_ui` at line 878 seems to be unused or a duplicate

Let me verify my change was made correctly and check for any other issues.




已完成：
1. 将"Zoom"改为"Input Zoom"，表示是input窗口的缩放倍数
2. 增加了"Result Zoom"显示result窗口的缩放倍数

修改后状态栏显示：`Input: X vertices | Output: Y vertices | Topology: Z | Input Zoom: X.XXx | Result Zoom: Y.YYx`

---

