# HLSL解释器代码重构

**Session ID:** ses_1a76e1c36ffecK1kSapkhZr1tl
**Created:** 2026/5/24 14:00:16
**Updated:** 2026/5/24 14:02:17

---

## User

1. 创建render.py文件
2. 把hlsl_interpreter.py的main函数的代码迁移到render.py文件
3. hlsl_interpreter.py的代码只保留实现HLSL解释执行器的代码 

---

## Assistant (Build · MiniMax-M2.7 · 8.5s)

_Thinking:_

The user wants me to:
1. Create a render.py file
2. Move the main function code from hlsl_interpreter.py to render.py
3. Keep only the HLSL interpreter implementation code in hlsl_interpreter.py

Let me first look at the hlsl_interpreter.py file to understand its structure and find the main function.


**Tool: glob**

**Input:**
```json
{
  "pattern": "**/*hlsl_interpreter*"
}
```

**Output:**
```
C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\hlsl_interpreter.py
C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\__pycache__\hlsl_interpreter.cpython-314.pyc
```

---

## Assistant (Build · MiniMax-M2.7 · 7.4s)

_Thinking:_

Let me read the hlsl_interpreter.py file to understand its structure and find the main function.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\hlsl_interpreter.py</path>
<type>file</type>
<content>
1: import csv
2: import json
3: import math
4: import re
5: import os
6: import time
7: from concurrent.futures import ThreadPoolExecutor
8: from dataclasses import dataclass, field
9: from typing import Any, Dict, List, Union, Optional
10: 
11: from hlsl_syntax_tree import SyntaxTreeNode, SyntaxTreeParser, _COMPILED_PATTERNS
12: from rasterizer import Rasterizer
13: 
14: 
15: try:
16:     from mesh_view import MeshView, VertexData
17:     MESHVIEW_AVAILABLE = True
18: except ImportError:
19:     MESHVIEW_AVAILABLE = False
20: 
21: 
22: DATA_TYPE_LIST = [
23:     'float4x4', 'float3x3',  # 矩阵类型
24:     'float4', 'float3', 'float2', 'float',  # 浮点向量/标量
25:     'uint4', 'uint3', 'uint2', 'uint',  # 无符号整数
26:     'int4', 'int3', 'int2', 'int',  # 有符号整数
27:     'bool'  # 布尔类型
28: ]
29: 
30: from d3d import (
31:     D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
32:     D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
33:     D3D_PRIMITIVE_TOPOLOGY_LINELIST,
34:     D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
35:     D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
36:     D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
37:     D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN
38: )
39: 
40: 
41: @dataclass
42: class ShaderVariable:
43:     """着色器变量定义"""
44:     name: str       # 变量名
45:     type: str       # 变量类型
46:     value: Any      # 变量值
47: 
48: 
49: @dataclass
50: class FieldDefinition:
51:     """结构体或cbuffer的字段定义"""
52:     field_type: str      # 字段类型，如 float3, float4x4
53:     name: str           # 字段名
54:     semantic: str       # 语义名称，如 POSITION, NORMAL
55:     data: List[Any] = None  # 字段数据值
56: 
57: 
58: @dataclass
59: class Vertex:
60:     """顶点对象 - 保存输入和输出顶点数据"""
61:     index: int = 0                          # 顶点索引（按输入顺序）
62:     input_data: Dict[str, Any] = None      # 输入顶点数据（所有字段）
63:     output_data: Dict[str, Any] = None     # 输出顶点数据（所有字段）
64:     input_position: List[float] = None     # 输入坐标
65:     input_normal: List[float] = None       # 输入法向量
66:     input_color: List[float] = None        # 输入颜色
67:     input_texcoord: List[float] = None    # 输入纹理坐标
68:     input_texcoord2: List[float] = None   # 输入第二纹理坐标
69:     output_position: List[float] = None    # 输出坐标
70:     output_normal: List[float] = None      # 输出法向量
71:     output_color: List[float] = None       # 输出颜色
72:     output_texcoord: List[float] = None    # 输出纹理坐标
73:     output_texcoord2: List[float] = None   # 输出第二纹理坐标
74: 
75:     def __post_init__(self):
76:         if self.input_data is None:
77:             self.input_data = {}
78:         if self.output_data is None:
79:             self.output_data = {}
80: 
81: 
82: class VertexPool:
83:     """顶点池 - 根据输入顺序保存所有顶点对象"""
84: 
85:     def __init__(self):
86:         self.vertices: List[Vertex] = []
87:         self._input_struct: Optional[StructDefinition] = None
88:         self._output_struct: Optional[StructDefinition] = None
89: 
90:     def clear(self):
91:         """清空顶点池"""
92:         self.vertices.clear()
93: 
94:     def set_input_struct(self, struct: StructDefinition):
95:         """设置输入结构体定义"""
96:         self._input_struct = struct
97: 
98:     def set_output_struct(self, struct: StructDefinition):
99:         """设置输出结构体定义"""
100:         self._output_struct = struct
101: 
102:     def add_vertex(self, vertex: Vertex):
103:         """添加顶点到池中"""
104:         self.vertices.append(vertex)
105: 
106:     def get_vertex(self, index: int) -> Optional[Vertex]:
107:         """根据索引获取顶点"""
108:         if 0 <= index < len(self.vertices):
109:             return self.vertices[index]
110:         return None
111: 
112:     def get_input_positions(self) -> List[List[float]]:
113:         """获取所有输入坐标"""
114:         return [v.input_position for v in self.vertices if v.input_position]
115: 
116:     def get_input_normals(self) -> List[List[float]]:
117:         """获取所有输入法向量"""
118:         return [v.input_normal for v in self.vertices if v.input_normal]
119: 
120:     def get_input_colors(self) -> List[List[float]]:
121:         """获取所有输入颜色"""
122:         return [v.input_color for v in self.vertices if v.input_color]
123: 
124:     def get_input_texcoords(self) -> List[List[float]]:
125:         """获取所有输入纹理坐标"""
126:         return [v.input_texcoord for v in self.vertices if v.input_texcoord]
127: 
128:     def get_input_texcoords2(self) -> List[List[float]]:
129:         """获取所有输入第二纹理坐标"""
130:         return [v.input_texcoord2 for v in self.vertices if v.input_texcoord2]
131: 
132:     def get_output_positions(self) -> List[List[float]]:
133:         """获取所有输出坐标"""
134:         return [v.output_position for v in self.vertices if v.output_position]
135: 
136:     def get_output_normals(self) -> List[List[float]]:
137:         """获取所有输出法向量"""
138:         return [v.output_normal for v in self.vertices if v.output_normal]
139: 
140:     def get_output_colors(self) -> List[List[float]]:
141:         """获取所有输出颜色"""
142:         return [v.output_color for v in self.vertices if v.output_color]
143: 
144:     def get_output_texcoords(self) -> List[List[float]]:
145:         """获取所有输出纹理坐标"""
146:         return [v.output_texcoord for v in self.vertices if v.output_texcoord]
147: 
148:     def get_output_texcoords2(self) -> List[List[float]]:
149:         """获取所有输出第二纹理坐标"""
150:         return [v.output_texcoord2 for v in self.vertices if v.output_texcoord2]
151: 
152:     def build_from_input(self, vs_input: str, input_data: Dict[str, Any], row_index: int):
153:         """
154:         根据输入数据构建顶点
155:         vs_input: 输入结构体名
156:         input_data: 输入数据字典
157:         row_index: 行索引
158:         """
159:         input_struct = self._input_struct
160:         if not input_struct:
161:             return
162: 
163:         vertex = Vertex(index=row_index, input_data=dict(input_data))
164: 
165:         for field in input_struct.fields:
166:             field_name_lower = field.name.lower()
167:             field_semantic_upper = field.semantic.upper()
168:             value = input_data.get(field.name)
169: 
170:             if value is None:
171:                 continue
172: 
173:             if 'pos' in field_name_lower or 'position' in field_name_lower or field_semantic_upper == 'POSITION':
174:                 if isinstance(value, list) and len(value) >= 3:
175:                     vertex.input_position = value[:3]
176:             elif 'normal' in field_name_lower or field_semantic_upper == 'NORMAL':
177:                 if isinstance(value, list) and len(value) >= 3:
178:                     vertex.input_normal = value[:3]
179:             elif 'color' in field_name_lower or field_semantic_upper == 'COLOR':
180:                 if isinstance(value, list) and len(value) >= 4:
181:                     vertex.input_color = value[:4]
182:                 elif isinstance(value, list) and len(value) >= 3:
183:                     vertex.input_color = value[:3] + [1.0]
184:             elif 'texcoord' in field_name_lower or 'uv' in field_name_lower or field_semantic_upper.startswith('TEXCOORD'):
185:                 if isinstance(value, list):
186:                     if 'texcoord2' in field_name_lower or 'texcoord1' in field_name_lower or field_semantic_upper == 'TEXCOORD1':
187:                         vertex.input_texcoord2 = value[:2] if len(value) >= 2 else value
188:                     else:
189:                         vertex.input_texcoord = value[:2] if len(value) >= 2 else value
190: 
191:         self.add_vertex(vertex)
192: 
193:     def update_output(self, row_index: int, result: Dict[str, Any]):
194:         """
195:         更新顶点的输出数据
196:         row_index: 行索引
197:         result: 输出结果字典
198:         """
199:         if row_index >= len(self.vertices):
200:             return
201: 
202:         vertex = self.vertices[row_index]
203:         vertex.output_data = dict(result) if result else {}
204: 
205:         output_struct = self._output_struct
206:         if not output_struct:
207:             for key, value in result.items() if result else {}.items():
208:                 key_lower = key.lower()
209:                 if 'pos' in key_lower or 'position' in key_lower or key.upper() == 'SV_POSITION':
210:                     if isinstance(value, list) and len(value) >= 3:
211:                         vertex.output_position = value[:3]
212:                 elif 'normal' in key_lower:
213:                     if isinstance(value, list) and len(value) >= 3:
214:                         vertex.output_normal = value[:3]
215:                 elif 'color' in key_lower:
216:                     if isinstance(value, list) and len(value) >= 4:
217:                         vertex.output_color = value[:4]
218:                     elif isinstance(value, list) and len(value) >= 3:
219:                         vertex.output_color = value[:3] + [1.0]
220:                 elif 'texcoord' in key_lower or 'uv' in key_lower:
221:                     if isinstance(value, list):
222:                         if 'texcoord2' in key_lower or 'texcoord1' in key_lower:
223:                             vertex.output_texcoord2 = value[:2] if len(value) >= 2 else value
224:                         else:
225:                             vertex.output_texcoord = value[:2] if len(value) >= 2 else value
226:             return
227: 
228:         for field in output_struct.fields:
229:             field_name_lower = field.name.lower()
230:             field_semantic_upper = field.semantic.upper()
231:             value = result.get(field.name) if result else None
232: 
233:             if value is None:
234:                 continue
235: 
236:             if 'pos' in field_name_lower or 'position' in field_name_lower or field_semantic_upper == 'POSITION' or field_semantic_upper == 'SV_POSITION':
237:                 if isinstance(value, list) and len(value) >= 3:
238:                     vertex.output_position = value[:3]
239:             elif 'normal' in field_name_lower or field_semantic_upper == 'NORMAL':
240:                 if isinstance(value, list) and len(value) >= 3:
241:                     vertex.output_normal = value[:3]
242:             elif 'color' in field_name_lower or field_semantic_upper == 'COLOR':
243:                 if isinstance(value, list) and len(value) >= 4:
244:                     vertex.output_color = value[:4]
245:                 elif isinstance(value, list) and len(value) >= 3:
246:                     vertex.output_color = value[:3] + [1.0]
247:             elif 'texcoord' in field_name_lower or 'uv' in field_name_lower or field_semantic_upper.startswith('TEXCOORD'):
248:                 if isinstance(value, list):
249:                     if 'texcoord2' in field_name_lower or 'texcoord1' in field_name_lower or field_semantic_upper == 'TEXCOORD1':
250:                         vertex.output_texcoord2 = value[:2] if len(value) >= 2 else value
251:                     else:
252:                         vertex.output_texcoord = value[:2] if len(value) >= 2 else value
253: 
254:     def get_count(self) -> int:
255:         """获取顶点数量"""
256:         return len(self.vertices)
257: 
258: 
259: @dataclass
260: class StructDefinition:
261:     """HLSL结构体定义"""
262:     name: str                     # 结构体名称
263:     fields: List[FieldDefinition]  # 结构体字段列表
264: 
265: @dataclass
266: class CbufferDefinition:
267:     """HLSL常量缓冲区定义"""
268:     name: str                     # cbuffer名称
269:     fields: List[FieldDefinition]  # cbuffer字段列表
270: 
271: 
272: class HLSLInterpreter:
273:     """
274:     HLSL解释器 - 解析和执行HLSL着色器代码
275:     支持: 结构体定义、cbuffer定义、函数解析、表达式求值
276:     """
277: 
278:     def __init__(self,
279:                 log_to_file: bool = True,
280:                 log_file_path: str = "hlsl_interpreter.log",
281:                 print_sequence: int = 1,
282:                 log_file_mode: str = 'a',
283:                 printSyntaxTree: bool = True,
284:                 print_interpreter_result: bool = True,
285:                 max_workers: int = 1,
286:                 primitive_topology: int = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
287:                 log_cache_size: int = 10 * 1024 * 1024):
288:         self.structs: Dict[str, StructDefinition] = {}      # 解析的结构体定义
289:         self.cbuffers: Dict[str, CbufferDefinition] = {}    # 解析的cbuffer定义
290:         self.variables: Dict[str, Any] = {}                 # 全局变量
291:         self.debug = True                                   # 调试模式开关
292:         self.printSyntaxTree = printSyntaxTree              # 打印语法树开关
293:         self.syntax_parser = SyntaxTreeParser()             # 语法树解析器
294:         self.log_to_file = log_to_file                      # 是否输出到文件
295:         self.log_file_path = log_file_path                  # 日志文件路径
296:         self.log_file_mode = log_file_mode                  # 文件模式: 'a'=追加, 'w'=覆盖
297:         self.print_sequence = max(1, print_sequence)        # 打印间隔频率
298:         self.print_interpreter_result = print_interpreter_result  # 是否打印HLSL Interpreter Result
299:         self._eval_counter = 0                              # evaluate_syntax_tree执行计数器
300:         self._should_print = True                           # 当前是否应该打印
301:         self._log_file = None                               # 日志文件句柄
302:         self.hlsl_code = None                               # 加载的HLSL代码
303:         self.max_workers = max_workers                       # 线程池最大工作线程数
304:         self._parsed_func_cache = {}                         # 解析过的函数体缓存
305:         self.primitive_topology = primitive_topology         # 图元拓扑类型
306:         self._mesh_view = None                               # MeshView实例(用于显示输入和输出)
307:         self._mesh_view_enabled = False                      # 是否启用MeshView
308:         self.vertex_pool = VertexPool()                       # 顶点池
309:         self._log_cache = []                                 # 日志缓存
310:         self._log_cache_size = log_cache_size                # 日志缓存大小(字节)
311:         self._log_cache_bytes = 0                            # 当前缓存已用字节数
312: 
313:         # 预编译的正则表达式模式字典
314:         type_pattern = '|'.join(DATA_TYPE_LIST)
315:         self.patterns: Dict[str, re.Pattern] = {
316:             # execute_statement: 变量声明语句，如 "float4 pos = ...;"
317:             'variable_declaration': re.compile(rf'^({type_pattern})\s+(\w+)\s*=\s*(.+?);?$'),
318: 
319:             # execute_statement: output字段赋值语句，如 "output.Color = ...;" 或 "output.Color.r = ...;"
320:             'output_field_assignment': re.compile(r'output\.(\w+)(?:\.([xyzwrgba]+))?\s*=\s*(.+)'),
321: 
322:             # execute_statement: 一般赋值语句，如 "var = ...;"
323:             'simple_assignment': re.compile(r'(\w+)\s*=\s*(.+?);?$'),
324: 
325:             # execute_statement: if条件语句，如 "if(condition) { ... }"
326:             'if_statement': re.compile(r'if\s*\((.+?)\)\s*(.+)$', re.DOTALL),
327: 
328:             # parse_struct: 结构体定义，如 "struct VS_INPUT { ... }"
329:             'struct_definition': re.compile(r'struct\s+(\w+)\s*\{([^}]+)\}'),
330: 
331:             # parse_cbuffer: cbuffer定义，如 "cbuffer MyBuffer : register(b0) { ... }"
332:             'cbuffer_definition': re.compile(r'cbuffer\s+(\w+)\s*:.*?\{([^}]+)\}', re.DOTALL),
333: 
334:             # parse_function: 函数定义，如 "float4 main(VS_INPUT input) { ... }"
335:             'function_definition': re.compile(r'(\w+)\s+(\w+)\s*\(([^)]*)\)\s*\{([^}]+(?:\{[^}]*\}[^}]*)*)\}', re.DOTALL),
336: 
337:             # load_hlsl_code_from_file / executeVS: 查找struct定义（用于finditer）
338:             'struct_finditer': re.compile(r'struct\s+\w+\s*\{[^}]+\}'),
339: 
340:             # load_hlsl_code_from_file: 查找cbuffer定义（用于finditer）
341:             'cbuffer_finditer': re.compile(r'cbuffer\s+\w+[^}]+\}'),
342:         }
343: 
344:         if self.log_to_file and self.log_file_path:
345:             self._log_file = open(self.log_file_path, self.log_file_mode, encoding='utf-8')
346: 
347:     def __del__(self):
348:         """对象销毁时关闭日志文件"""
349:         if self._log_cache:
350:             self._flush_log_cache()
351:         if self._log_file:
352:             self._log_file.close()
353:             self._log_file = None
354: 
355:     def enable_mesh_view(self, enable: bool = True):
356:         """
357:         启用或禁用MeshView
358:         enable: 是否启用MeshView
359:         """
360:         if enable and not MESHVIEW_AVAILABLE:
361:             self.log_output("Warning: MeshView not available (tkinter may not be installed)")
362:             return
363:         self._mesh_view_enabled = enable
364:         if enable and self._mesh_view is None:
365:             self._mesh_view = MeshView(title="HLSL Interpreter - Input/Output Mesh")
366: 
367:         self.log_output(f"MeshView {'enabled' if enable else 'disabled'}")
368: 
369:     def show_input_mesh(self, vs_input: str, row_index: int = None):
370:         """
371:         显示当前输入的mesh数据
372:         vs_input: 输入结构体名
373:         row_index: 指定行索引，如果为None则显示所有行
374:         """
375:         if not self._mesh_view_enabled or not MESHVIEW_AVAILABLE:
376:             return
377: 
378:         input_struct = self.structs.get(vs_input)
379:         if not input_struct:
380:             self.log_output(f"Cannot find vs input struct: {vs_input}")
381:             return
382: 
383:         positions = self.vertex_pool.get_input_positions()
384:         normals = self.vertex_pool.get_input_normals()
385:         colors = self.vertex_pool.get_input_colors()
386:         texcoords = self.vertex_pool.get_input_texcoords()
387:         texcoords2 = self.vertex_pool.get_input_texcoords2()
388: 
389:         if not positions:
390:             self.log_output(f"No input vertices in vertex pool")
391:             return
392: 
393:         num_rows = len(positions)
394: 
395:         if row_index is not None:
396:             num_rows = min(row_index + 1, num_rows)
397:             row_start = row_index
398:             row_end = row_index + 1
399:         else:
400:             row_start = 0
401:             row_end = num_rows
402: 
403:         positions = positions[row_start:row_end]
404:         normals = normals[row_start:row_end] if normals and len(normals) >= row_end else None
405:         colors = colors[row_start:row_end] if colors and len(colors) >= row_end else None
406:         texcoords = texcoords[row_start:row_end] if texcoords and len(texcoords) >= row_end else None
407:         texcoords2 = texcoords2[row_start:row_end] if texcoords2 and len(texcoords2) >= row_end else None
408: 
409:         if positions:
410:             self._mesh_view.clear()
411:             self._mesh_view.set_primitive_topology(self.primitive_topology)
412:             self._mesh_view.set_input_data(positions, normals, colors, texcoords, texcoords2)
413:             self._mesh_view.show(blocking=False)
414:         else:
415:             self.log_output(f"No position data found in {vs_input}")
416: 
417:     def show_result_mesh(self, results: List[Dict[str, Any]], output_struct_name: str = None):
418:         """
419:         显示executeVS执行完毕后的results mesh数据
420:         results: executeVS返回的输出结构体字典列表
421:         output_struct_name: 输出结构体名(可选)
422:         """
423:         if not self._mesh_view_enabled or not MESHVIEW_AVAILABLE:
424:             return
425: 
426:         positions = self.vertex_pool.get_output_positions()
427:         normals = self.vertex_pool.get_output_normals()
428:         colors = self.vertex_pool.get_output_colors()
429:         texcoords = self.vertex_pool.get_output_texcoords()
430:         texcoords2 = self.vertex_pool.get_output_texcoords2()
431: 
432:         if not positions:
433:             self.log_output("No output vertices in vertex pool")
434:             return
435: 
436:         if positions:
437:             self._mesh_view.set_primitive_topology(self.primitive_topology)
438:             self._mesh_view.set_output_data(positions, normals, colors, texcoords, texcoords2)
439:             self._mesh_view.show(blocking=False)
440:             self.log_output(f"Result mesh displayed: {len(positions)} vertices")
441:         else:
442:             self.log_output("No position data found in results")
443: 
444:     def _flush_log_cache(self):
445:         """将缓存中的日志写入文件"""
446:         if self._log_cache and self._log_file:
447:             self._log_file.write(''.join(self._log_cache))
448:             self._log_file.flush()
449:             self._log_cache = []
450:             self._log_cache_bytes = 0
451: 
452:     def log_output(self, *args, **kwargs):
453:         """输出到stdout和日志文件"""
454:         msg = ' '.join(str(arg) for arg in args)
455:         print(*args, **kwargs)
456:         if self.log_to_file and self._log_file:
457:             msg_bytes = (msg + '\n').encode('utf-8')
458:             if self._log_cache_bytes + len(msg_bytes) >= self._log_cache_size:
459:                 self._flush_log_cache()
460:             self._log_cache.append(msg + '\n')
461:             self._log_cache_bytes += len(msg_bytes)
462: 
463:     def debug_print(self, msg: str):
464:         """调试打印"""
465:         if self.debug and self._should_print:
466:             self.log_output(msg)
467: 
468:     def _format_float(self, val):
469:         """
470:         格式化浮点数输出
471:         val: 值
472:         返回: 格式化后的字符串(保留4位小数)
473:         """
474:         if isinstance(val, float):
475:             return f"{val:.4f}"
476:         if isinstance(val, list):
477:             if val and isinstance(val[0], list):
478:                 return self._format_matrix(val)
479:             return [self._format_float(v) for v in val]
480:         return val
481: 
482:     def _format_matrix(self, val):
483:         """
484:         格式化矩阵输出
485:         val: 矩阵(二维列表)
486:         返回: 格式化后的矩阵字符串
487:         """
488:         if not val or not isinstance(val[0], list):
489:             return str(val)
490:         formatted = [[self._format_float(v) for v in row] for row in val]
491:         col_widths = [0] * len(formatted[0])
492:         for row in formatted:
493:             for j, cell in enumerate(row):
494:                 col_widths[j] = max(col_widths[j], len(cell))
495:         lines = []
496:         for row in formatted:
497:             cells = [cell.rjust(col_widths[j]) for j, cell in enumerate(row)]
498:             lines.append("[" + " ".join(cells) + "]")
499:         return "\n".join(lines)
500: 
501:     def _format_value(self, val):
502:         """格式化值输出(矩阵或标量/向量)"""
503:         if isinstance(val, list) and val and isinstance(val[0], list):
504:             return self._format_matrix(val)
505:         return self._format_float(val)
506: 
507:     def _format_msg(self, *args):
508:         """格式化多个值用于调试输出"""
509:         formatted = []
510:         for arg in args:
511:             formatted.append(self._format_float(arg))
512:         return formatted
513: 
514:     def load_json(self, filepath: str):
515:         """从JSON文件加载数据"""
516:         with open(filepath, 'r') as f:
517:             data = json.load(f)
518:         return data
519: 
520:     def load_csv(self, filepath: str) -> List[List[str]]:
521:         """从CSV文件加载数据，返回二维列表"""
522:         rows = []
523:         with open(filepath, 'r') as f:
524:             reader = csv.reader(f)
525:             for row in reader:
526:                 rows.append(row)
527:         return rows
528: 
529:     def get_type_size(self, field_type: str) -> int:
530:         """
531:         获取HLSL类型的大小(字节数)
532:         field_type: HLSL类型名，如 float4x4, float3, int
533:         返回: 类型占用的字节数
534:         """
535:         return self._TYPE_SIZE_MAP.get(field_type, 0)
536: 
537:     _TYPE_SIZE_MAP = {
538:         'float4x4': 64, 'float3x3': 36, 'float4': 16, 'float3': 12,
539:         'float2': 8, 'float': 4, 'uint4': 16, 'uint3': 12, 'uint2': 8,
540:         'uint': 4, 'int4': 16, 'int3': 12, 'int2': 8, 'int': 4, 'bool': 4
541:     }
542: 
543:     def parse_value_by_type(self, value_str: str, field_type: str) -> Any:
544:         """
545:         根据类型解析字符串值为对应类型的Python对象
546:         value_str: 值的字符串表示
547:         field_type: HLSL类型名
548:         返回: 解析后的值
549:         """
550:         value_str = value_str.strip().strip('"')
551:         handler = self._PARSE_TYPE_HANDLERS.get(field_type)
552:         if handler:
553:             return handler(self, value_str)
554:         try:
555:             return float(value_str)
556:         except:
557:             return value_str
558: 
559:     def _parse_float4x4(self, value_str):
560:         parts = value_str.split(',')
561:         if len(parts) >= 16:
562:             return [[float(parts[j]) for j in range(i*4, i*4+4)] for i in range(4)]
563:         return None
564: 
565:     def _parse_float3x3(self, value_str):
566:         parts = value_str.split(',')
567:         if len(parts) >= 9:
568:             return [[float(parts[j]) for j in range(i*3, i*3+3)] for i in range(3)]
569:         return None
570: 
571:     def _parse_float_vector(self, value_str, count):
572:         return [float(p) for p in value_str.split(',')[:count]]
573: 
574:     def _parse_int_vector(self, value_str, count):
575:         return [int(p) for p in value_str.split(',')[:count]]
576: 
577:     def _parse_bool(self, value_str):
578:         return value_str.lower() in ('true', '1', 'yes')
579: 
580:     _PARSE_TYPE_HANDLERS = {
581:         'float4x4': _parse_float4x4,
582:         'float3x3': _parse_float3x3,
583:         'float4': lambda s, v: s._parse_float_vector(v, 4),
584:         'float3': lambda s, v: s._parse_float_vector(v, 3),
585:         'float2': lambda s, v: s._parse_float_vector(v, 2),
586:         'uint4': lambda s, v: s._parse_int_vector(v, 4),
587:         'uint3': lambda s, v: s._parse_int_vector(v, 3),
588:         'uint2': lambda s, v: s._parse_int_vector(v, 2),
589:         'uint': lambda s, v: int(v),
590:         'int4': lambda s, v: s._parse_int_vector(v, 4),
591:         'int3': lambda s, v: s._parse_int_vector(v, 3),
592:         'int2': lambda s, v: s._parse_int_vector(v, 2),
593:         'int': lambda s, v: int(v),
594:         'bool': _parse_bool,
595:     }
596: 
597:     def parse_type(self, type_str: str) -> str:
598:         """
599:         解析HLSL类型字符串为标准类型名
600:         type_str: 类型字符串，如 "float4x4", "float3", "int2"
601:         返回: 标准类型名
602:         """
603:         type_str = type_str.strip()
604:         if type_str in DATA_TYPE_LIST:
605:             return type_str
606:         if type_str.startswith('float'):
607:             if 'x3' in type_str:
608:                 return 'float3x3'
609:             elif 'x4' in type_str:
610:                 return 'float4x4'
611:             elif type_str == 'float':
612:                 return 'float'
613:             return 'float'
614:         elif type_str.startswith('int'):
615:             if type_str == 'int':
616:                 return 'int'
617:             elif '2' in type_str:
618:                 return 'int2'
619:             elif '3' in type_str:
620:                 return 'int3'
621:             elif '4' in type_str:
622:                 return 'int4'
623:             return 'int'
624:         elif type_str.startswith('uint'):
625:             if type_str == 'uint':
626:                 return 'uint'
627:             elif '2' in type_str:
628:                 return 'uint2'
629:             elif '3' in type_str:
630:                 return 'uint3'
631:             elif '4' in type_str:
632:                 return 'uint4'
633:             return 'uint'
634:         elif type_str.startswith('bool'):
635:             return 'bool'
636:         return type_str
637: 
638:     def parse_struct(self, code: str) -> StructDefinition:
639:         """
640:         解析HLSL结构体定义
641:         code: 结构体代码，如 "struct VS_INPUT { float3 Pos : POSITION; }"
642:         返回: StructDefinition对象
643:         """
644:         match = self.patterns['struct_definition'].search(code)
645:         if not match:
646:             return None
647:         name = match.group(1)
648:         fields_str = match.group(2)
649:         fields = []
650:         for line in fields_str.split(';'):
651:             line = line.strip()
652:             if not line:
653:                 continue
654:             parts = line.split(':')
655:             if len(parts) == 2:
656:                 type_and_name = parts[0].strip().split()
657:                 semantic = parts[1].strip()
658:                 if len(type_and_name) >= 2:
659:                     field_type = type_and_name[0]
660:                     field_name = type_and_name[-1]
661:                 else:
662:                     field_type = type_and_name[0]
663:                     field_name = ''
664:                 fields.append(FieldDefinition(field_type, field_name, semantic))
665:         return StructDefinition(name, fields)
666: 
667:     def parse_cbuffer(self, code: str) -> CbufferDefinition:
668:         """
669:         解析HLSL常量缓冲区定义
670:         code: cbuffer代码
671:         返回: CbufferDefinition对象
672:         """
673:         match = self.patterns['cbuffer_definition'].search(code)
674:         if not match:
675:             return None
676:         name = match.group(1)
677:         fields = []
678:         lines = code[match.start():match.end()].split('\n')[1:]
679:         for line in lines:
680:             line = line.strip().rstrip(';')
681:             if not line or line.startswith('}'):
682:                 continue
683:             if any(t in line for t in DATA_TYPE_LIST):
684:                 parts = line.split()
685:                 if len(parts) >= 2:
686:                     field_type = parts[0]
687:                     field_name = parts[1]
688:                     fields.append(FieldDefinition(field_type, field_name, ''))
689:         return CbufferDefinition(name, fields)
690: 
691:     def parse_function(self, code: str) -> tuple:
692:         """
693:         解析HLSL函数定义
694:         code: 函数代码，如 "float4 main(VS_INPUT input) { ... }"
695:         返回: (返回类型, 函数名, 参数字典, 函数体) 元组
696:         """
697:         match = self.patterns['function_definition'].search(code)
698:         if not match:
699:             return None, None, None, None
700:         ret_type = match.group(1)
701:         func_name = match.group(2)
702:         params_str = match.group(3)
703:         body = match.group(4)
704:         params = {}
705:         if params_str.strip():
706:             for param in params_str.split(','):
707:                 param = param.strip()
708:                 parts = param.split()
709:                 if len(parts) >= 2:
710:                     param_type = parts[0]
711:                     param_name = parts[1]
712:                     params[param_name] = param_type
713:         return ret_type, func_name, params, body
714: 
715:     def execute_unary_op(self, op: str, val: Any) -> Any:
716:         """
717:         执行一元运算符
718:         op: 运算符 '-' 或 '!'
719:         val: 操作数
720:         """
721:         if op == '-':
722:             result = [-v for v in val] if isinstance(val, list) else (-val if isinstance(val, (int, float)) else val)
723:         else:
724:             result = not bool(val)
725:         if self.debug and self._should_print:
726:             self.debug_print(f"[UNARY OP] operand={self._format_value(val)}, op={op}, result={self._format_value(result)}")
727:         return result
728: 
729:     def execute_binary_op(self, op: str, left: Any, right: Any) -> Any:
730:         """
731:         执行二元运算符
732:         op: 运算符 '+', '-', '*', '/', '.'
733:         left, right: 左右操作数
734:         """
735:         if left is None or right is None:
736:             result = None
737:             self.debug_print(f"[BINARY OP] left={self._format_value(left)}, right={self._format_value(right)}, op={op}, result={self._format_value(result)}")
738:             return None
739:         if op == '+':
740:             if isinstance(left, list) and isinstance(right, list):
741:                 result = [l + r for l, r in zip(left, right)]
742:             elif isinstance(left, list) and isinstance(right, (int, float)):
743:                 result = [v + right for v in left]
744:             elif isinstance(right, list) and isinstance(left, (int, float)):
745:                 result = [left + v for v in right]
746:             else:
747:                 result = left + right
748:         elif op == '-':
749:             if isinstance(left, list) and isinstance(right, list):
750:                 result = [l - r for l, r in zip(left, right)]
751:             elif isinstance(left, list) and isinstance(right, (int, float)):
752:                 result = [v - right for v in left]
753:             elif isinstance(right, list) and isinstance(left, (int, float)):
754:                 result = [left - v for v in right]
755:             else:
756:                 result = left - right
757:         elif op == '*':
758:             if isinstance(left, list) and isinstance(right, (int, float)):
759:                 result = [v * right for v in left]
760:             elif isinstance(right, list) and isinstance(left, (int, float)):
761:                 result = [v * left for v in right]
762:             elif isinstance(left, list) and isinstance(right, list):
763:                 result = [l * r for l, r in zip(left, right)]
764:             else:
765:                 result = left * right
766:         elif op == '/':
767:             if isinstance(left, list) and isinstance(right, (int, float)):
768:                 result = [v / right for v in left]
769:             elif isinstance(left, list) and isinstance(right, list):
770:                 result = [l / r for l, r in zip(left, right)]
771:             else:
772:                 result = left / right
773:         elif op == '.':
774:             result = (left, right)
775:         elif op == '==':
776:             result = left == right
777:         elif op == '!=':
778:             result = left != right
779:         elif op == '<':
780:             result = left < right
781:         elif op == '>':
782:             result = left > right
783:         elif op == '<=':
784:             result = left <= right
785:         elif op == '>=':
786:             result = left >= right
787:         elif op == '&&':
788:             result = bool(left and right)
789:         elif op == '||':
790:             result = bool(left or right)
791:         else:
792:             result = None
793:         self.debug_print(f"[BINARY OP] left={self._format_float(left)}, right={self._format_float(right)}, op={op}, result={self._format_float(result)}")
794:         return result
795: 
796:     def transpose_matrix(self, m: List[List[float]]) -> List[List[float]]:
797:         """
798:         矩阵转置
799:         m: 输入矩阵(4x4或3x3)
800:         返回: 转置后的矩阵
801:         """
802:         n = len(m)
803:         return [[m[j][i] for j in range(n)] for i in range(n)]
804: 
805:     def mul_matrix_vector(self, m: List[List[float]], v: List[float]) -> List[float]:
806:         """
807:         矩阵乘向量: result = m * v
808:         m: 4x4或3x3矩阵
809:         v: 向量(4维或3维)
810:         返回: 计算后的向量
811:         """
812:         if not v or any(x is None for x in v):
813:             return [0, 0, 0, 0]
814:         if not m:
815:             return [0, 0, 0, 0]
816:         return [sum(v[i] * m[i][j] for i in range(len(v))) for j in range(len(m[0]))]
817: 
818:     def mul_matrix_matrix(self, a: List[List[float]], b: List[List[float]]) -> List[List[float]]:
819:         """
820:         矩阵乘法: result = a * b
821:         a, b: n x n 方阵
822:         返回: 结果矩阵
823:         """
824:         n = len(a)
825:         return [[sum(a[i][k] * b[k][j] for k in range(n)) for j in range(n)] for i in range(n)]
826: 
827:     def length_vec(self, v: List[float]) -> float:
828:         """计算向量长度(模)"""
829:         return math.sqrt(sum(x * x for x in v))
830: 
831:     def normalize_vec(self, v: List[float]) -> List[float]:
832:         """
833:         向量归一化
834:         v: 输入向量
835:         返回: 归一化后的向量，长度为1
836:         """
837:         l = self.length_vec(v)
838:         if l < 1e-8:
839:             return v
840:         return [x / l for x in v]
841: 
842:     def dot_product(self, a: List[float], b: List[float]) -> float:
843:         """
844:         向量点积: a · b
845:         a, b: 同维度向量
846:         返回: 点积结果
847:         """
848:         if not isinstance(a, list) or not isinstance(b, list):
849:             return 0.0
850:         if len(a) != len(b):
851:             return 0.0
852:         return sum(x * y for x, y in zip(a, b))
853: 
854:     def reflect_vec(self, I: List[float], N: List[float]) -> List[float]:
855:         """
856:         计算反射向量 R = I - 2 * (N · I) * N
857:         I: 入射向量
858:         N: 法线向量(需要归一化)
859:         返回: 反射向量
860:         """
861:         if not isinstance(I, list) or not isinstance(N, list):
862:             return [0, 0, 0]
863:         dot = self.dot_product(N, I)
864:         result = []
865:         for i_val, n_val in zip(I, N):
866:             result.append(i_val - 2 * n_val * dot)
867:         return result
868: 
869:     def find_top_level_comma(self, expr: str) -> int:
870:         """
871:         查找表达式顶层逗号(不在括号内)
872:         用于分割函数多参数
873:         expr: 表达式字符串
874:         返回: 逗号位置索引，或-1表示未找到
875:         """
876:         depth = 0
877:         for i, char in enumerate(expr):
878:             if char == '(':
879:                 depth += 1
880:             elif char == ')':
881:                 depth -= 1
882:             elif char == ',' and depth == 0:
883:                 return i
884:         return -1
885: 
886:     def evaluate_expression(self, expr: str, local_vars: Dict[str, Any]) -> Any:
887:         """
888:         对HLSL表达式求值
889:         expr: 表达式字符串
890:         local_vars: 局部变量字典
891:         返回: 求值结果
892:         """
893:         expr = expr.strip()
894:         if not expr:
895:             return None
896: 
897:         if expr == 'return':
898:             return None
899: 
900:         if expr.startswith('return '):
901:             return self.evaluate_expression(expr[7:], local_vars)
902: 
903:         # 使用语法树解析器处理所有表达式（包括三元运算符）
904:         tree = self.syntax_parser.parse(expr)
905: 
906:         # Print syntax tree
907:         if self.printSyntaxTree == True:
908:             self.debug_print(f"[SYNTAX TREE]\n{tree}")
909: 
910:         result = self.evaluate_syntax_tree(tree, local_vars)
911:         return result
912: 
913:     def evaluate_syntax_tree(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
914:         """
915:         对语法树节点求值
916:         node: 语法树节点
917:         local_vars: 局部变量字典
918:         返回: 求值结果
919:         """
920: 
921:         if node is None:
922:             return None
923: 
924:         if node.node_type == 'value':
925:             if node.value is None:
926:                 return None
927:             return self.get_value(node.value, local_vars)
928: 
929:         elif node.node_type == 'binary_op':
930:             left = self.evaluate_syntax_tree(node.left, local_vars)
931:             right = self.evaluate_syntax_tree(node.right, local_vars)
932:             return self.execute_binary_op(node.value, left, right)
933: 
934:         elif node.node_type == 'unary_op':
935:             child = self.evaluate_syntax_tree(node.left, local_vars)
936:             return self.execute_unary_op(node.value, child)
937: 
938:         elif node.node_type == 'function':
939:             return self.execute_function_node(node, local_vars)
940: 
941:         elif node.node_type == 'ternary':
942:             cond = self.evaluate_syntax_tree(node.left, local_vars)
943:             if cond:
944:                 return self.evaluate_syntax_tree(node.right, local_vars)
945:             else:
946:                 return self.evaluate_syntax_tree(node.third_child, local_vars)
947: 
948:         elif node.node_type == 'cast':
949:             inner = self.evaluate_syntax_tree(node.left, local_vars)
950:             if inner is None:
951:                 return None
952:             cast_type = node.value
953:             # float3x3转换: 从4x4矩阵提取前3x3
954:             if cast_type == 'float3x3' and isinstance(inner, list) and len(inner) == 4:
955:                 return [row[:3] for row in inner[:3]]
956:             # float2x2转换: 从4x4矩阵提取前2x2
957:             if cast_type == 'float2x2' and isinstance(inner, list) and len(inner) == 4:
958:                 return [row[:2] for row in inner[:2]]
959:             # float2x2转换: 从3x3矩阵提取前2x2
960:             if cast_type == 'float2x2' and isinstance(inner, list) and len(inner) == 3:
961:                 return [row[:2] for row in inner[:2]]
962:             return inner
963: 
964:         return None
965: 
966:     def execute_function_node(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
967:         """
968:         执行函数调用语法树节点
969:         node: 函数调用节点
970:         local_vars: 局部变量字典
971:         返回: 函数执行结果
972:         """
973:         func_name = node.value
974:         args = node.args
975: 
976:         # transpose: 矩阵转置函数
977:         # 计算矩阵的转置，将行列互换
978:         if func_name == 'transpose':
979:             if len(args) != 1:
980:                 self.debug_print(f"[ERROR] transpose requires 1 arg, got {len(args)} at line {node.line_number}")
981:                 return None
982:             val = self.evaluate_syntax_tree(args[0], local_vars)
983:             if val is None:
984:                 return None
985:             result = self.transpose_matrix(val)
986:             self.debug_print(f"[FUNC] transpose(\n{self._format_value(val)}) =\n{self._format_value(result)}")
987:             return result
988: 
989:         # normalize: 向量归一化函数
990:         # 将输入向量缩放到单位长度，即长度为1
991:         elif func_name == 'normalize':
992:             if len(args) != 1:
993:                 self.debug_print(f"[ERROR] normalize requires 1 arg, got {len(args)} at line {node.line_number}")
994:                 return None
995:             val = self.evaluate_syntax_tree(args[0], local_vars)
996:             if val is None:
997:                 return None
998:             if isinstance(val, list):
999:                 result = self.normalize_vec(val)
1000:                 self.debug_print(f"[FUNC] normalize({self._format_float(val)}) = {self._format_float(result)}")
1001:                 return result
1002:             return val
1003: 
1004:         # length: 向量长度函数
1005:         # 计算向量的欧几里得长度(模)
1006:         elif func_name == 'length':
1007:             if len(args) != 1:
1008:                 self.debug_print(f"[ERROR] length requires 1 arg, got {len(args)} at line {node.line_number}")
1009:                 return None
1010:             val = self.evaluate_syntax_tree(args[0], local_vars)
1011:             if val is None:
1012:                 return None
1013:             result = self.length_vec(val)
1014:             self.debug_print(f"[FUNC] length({self._format_float(val)}) = {self._format_float(result)}")
1015:             return result
1016: 
1017:         # dot: 向量点积函数
1018:         # 计算两个向量的点积，结果为标量
1019:         elif func_name == 'dot':
1020:             if len(args) != 2:
1021:                 self.debug_print(f"[ERROR] dot requires 2 args, got {len(args)} at line {node.line_number}")
1022:                 return None
1023:             a = self.evaluate_syntax_tree(args[0], local_vars)
1024:             b = self.evaluate_syntax_tree(args[1], local_vars)
1025:             if a is None or b is None:
1026:                 return None
1027:             result = self.dot_product(a, b)
1028:             self.debug_print(f"[FUNC] dot({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
1029:             return result
1030: 
1031:         # reflect: 反射向量函数
1032:         # 计算光线关于法向量的反射向量，公式: R = I - 2 * N * dot(I, N)
1033:         elif func_name == 'reflect':
1034:             if len(args) != 2:
1035:                 self.debug_print(f"[ERROR] reflect requires 2 args, got {len(args)} at line {node.line_number}")
1036:                 return None
1037:             I = self.evaluate_syntax_tree(args[0], local_vars)
1038:             N = self.evaluate_syntax_tree(args[1], local_vars)
1039:             if I is None or N is None:
1040:                 return None
1041:             result = self.reflect_vec(I, N)
1042:             self.debug_print(f"[FUNC] reflect({self._format_float(I)}, {self._format_float(N)}) = {self._format_float(result)}")
1043:             return result
1044: 
1045:         # max: 最大值函数
1046:         # 返回两个值中的较大者
1047:         elif func_name == 'max':
1048:             if len(args) != 2:
1049:                 self.debug_print(f"[ERROR] max requires 2 args, got {len(args)} at line {node.line_number}")
1050:                 return None
1051:             a = self.evaluate_syntax_tree(args[0], local_vars)
1052:             b = self.evaluate_syntax_tree(args[1], local_vars)
1053:             if a is None or b is None:
1054:                 return None
1055:             result = max(a, b)
1056:             self.debug_print(f"[FUNC] max({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
1057:             return result
1058: 
1059:         # min: 最小值函数
1060:         # 返回两个值中的较小者
1061:         elif func_name == 'min':
1062:             if len(args) != 2:
1063:                 self.debug_print(f"[ERROR] min requires 2 args, got {len(args)} at line {node.line_number}")
1064:                 return None
1065:             a = self.evaluate_syntax_tree(args[0], local_vars)
1066:             b = self.evaluate_syntax_tree(args[1], local_vars)
1067:             if a is None or b is None:
1068:                 return None
1069:             result = min(a, b)
1070:             self.debug_print(f"[FUNC] min({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
1071:             return result
1072: 
1073:         # pow: 幂函数
1074:         # 计算base的exp次幂，即 base ^ exp
1075:         elif func_name == 'pow':
1076:             if len(args) != 2:
1077:                 self.debug_print(f"[ERROR] pow requires 2 args, got {len(args)} at line {node.line_number}")
1078:                 return None
1079:             base = self.evaluate_syntax_tree(args[0], local_vars)
1080:             exp = self.evaluate_syntax_tree(args[1], local_vars)
1081:             if base is None or exp is None:
1082:                 return None
1083:             result = math.pow(base, exp)
1084:             self.debug_print(f"[FUNC] pow({self._format_float(base)}, {self._format_float(exp)}) = {self._format_float(result)}")
1085:             return result
1086: 
1087:         # abs: 绝对值函数
1088:         # 返回数值的绝对值，对列表则对每个元素取绝对值
1089:         elif func_name == 'abs':
1090:             if len(args) != 1:
1091:                 self.debug_print(f"[ERROR] abs requires 1 arg, got {len(args)} at line {node.line_number}")
1092:                 return None
1093:             val = self.evaluate_syntax_tree(args[0], local_vars)
1094:             if val is None:
1095:                 return None
1096:             if isinstance(val, list):
1097:                 result = [abs(v) for v in val]
1098:             else:
1099:                 result = abs(val)
1100:             self.debug_print(f"[FUNC] abs({self._format_float(val)}) = {self._format_float(result)}")
1101:             return result
1102: 
1103:         # sin: 正弦函数
1104:         # 计算弧度的正弦值，对列表则对每个元素计算
1105:         elif func_name == 'sin':
1106:             if len(args) != 1:
1107:                 self.debug_print(f"[ERROR] sin requires 1 arg, got {len(args)} at line {node.line_number}")
1108:                 return None
1109:             val = self.evaluate_syntax_tree(args[0], local_vars)
1110:             if val is None:
1111:                 return None
1112:             if isinstance(val, list):
1113:                 result = [math.sin(v) for v in val]
1114:             else:
1115:                 result = math.sin(val)
1116:             self.debug_print(f"[FUNC] sin({self._format_float(val)}) = {self._format_float(result)}")
1117:             return result
1118: 
1119:         # cos: 余弦函数
1120:         # 计算弧度的余弦值，对列表则对每个元素计算
1121:         elif func_name == 'cos':
1122:             if len(args) != 1:
1123:                 self.debug_print(f"[ERROR] cos requires 1 arg, got {len(args)} at line {node.line_number}")
1124:                 return None
1125:             val = self.evaluate_syntax_tree(args[0], local_vars)
1126:             if val is None:
1127:                 return None
1128:             if isinstance(val, list):
1129:                 result = [math.cos(v) for v in val]
1130:             else:
1131:                 result = math.cos(val)
1132:             self.debug_print(f"[FUNC] cos({self._format_float(val)}) = {self._format_float(result)}")
1133:             return result
1134: 
1135:         # mul: 矩阵乘法函数
1136:         # 执行4x4或3x3矩阵乘法运算
1137:         elif func_name == 'mul':
1138:             if len(args) != 2:
1139:                 self.debug_print(f"[ERROR] mul requires 2 args, got {len(args)} at line {node.line_number}")
1140:                 return None
1141:             left = self.evaluate_syntax_tree(args[0], local_vars)
1142:             right = self.evaluate_syntax_tree(args[1], local_vars)
1143:             if left is None or right is None:
1144:                 return None
1145:             if isinstance(left, list) and isinstance(right, list):
1146:                 if len(left) == 4 and len(right) == 4:
1147:                     result = self.mul_matrix_vector(right, left)
1148:                     self.debug_print(f"[FUNC] mul(\nleft={self._format_value(left)},\nright={self._format_value(right)}) =\n{self._format_value(result)}")
1149:                     return result
1150:                 elif len(left) == 3 and len(right) == 3:
1151:                     result = self.mul_matrix_vector(right, left)
1152:                     self.debug_print(f"[FUNC] mul(\nleft={self._format_value(left)},\nright={self._format_value(right)}) =\n{self._format_value(result)}")
1153:                     return result
1154:             return None
1155: 
1156:         # float2/float3/float4: 向量构造函数
1157:         # 将参数展平合并为指定长度的向量
1158:         elif func_name in ['float2', 'float3', 'float4']:
1159:             # 向量构造函数: 将参数展平合并
1160:             result = []
1161:             for arg in args:
1162:                 val = self.evaluate_syntax_tree(arg, local_vars)
1163:                 if isinstance(val, list):
1164:                     result.extend(val)
1165:                 else:
1166:                     result.append(val)
1167:             self.debug_print(f"[FUNC] {func_name}(args={self._format_float(args)}) = {self._format_float(result)}")
1168:             return result
1169: 
1170:         return None
1171: 
1172:     def apply_swizzle(self, obj: Any, swizzle: str) -> Any:
1173:         """
1174:         对向量应用swizzle操作
1175:         obj: 向量对象(列表)
1176:         swizzle: swizzle模式字符串，如 'xyz', 'xxx', 'xxyy', 'xz' 等
1177:         返回: 应用swizzle后的结果
1178:         """
1179:         if obj is None:
1180:             return None
1181: 
1182:         if not isinstance(obj, list):
1183:             return obj if swizzle == 'x' else None
1184: 
1185:         result = []
1186:         for c in swizzle:
1187:             if c.lower() in self._SWIZZLE_MAP:
1188:                 idx = self._SWIZZLE_MAP[c.lower()]
1189:                 result.append(obj[idx] if idx < len(obj) else 0)
1190:             elif c in 'rgb':
1191:                 idx = {'r': 0, 'g': 1, 'b': 2}[c]
1192:                 result.append(obj[idx] if idx < len(obj) else 0)
1193: 
1194:         if len(result) == 1:
1195:             return result[0]
1196: 
1197:         numeric_types = (int, float)
1198:         if all(isinstance(v, numeric_types) for v in result):
1199:             return [int(v) for v in result] if all(isinstance(v, int) for v in result) else result
1200: 
1201:         return result
1202: 
1203:     _SWIZZLE_MAP = {'x': 0, 'y': 1, 'z': 2, 'w': 3}
1204: 
1205:     def get_value(self, name: str, local_vars: Dict[str, Any]) -> Any:
1206:         """
1207:         获取变量或常量的值
1208:         name: 变量名/常量名，支持结构体字段访问(如 input.Pos)
1209:         local_vars: 局部变量字典
1210:         返回: 变量值，如果未找到返回0.0
1211:         """
1212:         name = name.strip()
1213: 
1214:         # 处理布尔常量
1215:         if name == 'true':
1216:             return True
1217:         if name == 'false':
1218:             return False
1219: 
1220:         # 尝试解析为数字
1221:         try:
1222:             return float(name)
1223:         except ValueError:
1224:             pass
1225: 
1226:         # 检查是否包含swizzle操作 (如 LightPos.xyz, LightPos.xxx, input.Pos.xy)
1227:         if '.' in name:
1228:             parts = name.split('.')
1229:             if len(parts) >= 2:
1230:                 base_name = parts[0]
1231: 
1232:                 # 判断是否为swizzle模式（全是xyzwrgb组成的字符串）
1233:                 # 对于 input.Color.g, parts = ['input', 'Color', 'g']
1234:                 # 只有当最后一部分是纯swizzle字符时，才认为是swizzle操作
1235:                 last_part = parts[-1]
1236:                 is_single_swizzle = len(parts) == 2 and last_part and all(c in 'xyzwrgb' for c in last_part.lower())
1237:                 is_multi_swizzle = len(parts) == 2 and last_part and all(c in 'xyzwrgb' for c in last_part.lower()) and len(last_part) > 1
1238: 
1239:                 if is_single_swizzle or is_multi_swizzle:
1240:                     # 两级访问: input.Pos 或 input.Color.rgb
1241:                     swizzle_str = last_part
1242:                     # 先检查 base_name + '.' + swizzle_str 是否直接存在
1243:                     full_swizzle_name = f'{base_name}.{swizzle_str}'
1244:                     if full_swizzle_name in local_vars:
1245:                         obj = local_vars[full_swizzle_name]
1246:                         if isinstance(obj, (int, float)):
1247:                             return obj

(Output capped at 50 KB. Showing lines 1-1247. Use offset=1248 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 7.7s)

_Thinking:_

The file is quite long, I need to read the rest to find the main function. Let me continue reading from line 1248.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "offset": 1248
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\hlsl_interpreter.py</path>
<type>file</type>
<content>
1248:                         if isinstance(obj, list):
1249:                             return obj
1250: 
1251:                     obj = local_vars.get(base_name)
1252:                     if obj is None:
1253:                         obj = self.variables.get(base_name)
1254:                     if obj is not None:
1255:                         return self.apply_swizzle(obj, swizzle_str)
1256: 
1257:                     # 尝试从cbuffer获取
1258:                     for cb_name, cb_def in self.cbuffers.items():
1259:                         if isinstance(cb_def, CbufferDefinition):
1260:                             for field in cb_def.fields:
1261:                                 if field.name == base_name:
1262:                                     if field.data is not None:
1263:                                         return self.apply_swizzle(field.data, swizzle_str)
1264:                                     return 0
1265: 
1266:                     # 检查是否在output对象中
1267:                     if base_name in local_vars:
1268:                         obj = local_vars[base_name]
1269:                         if isinstance(obj, dict):
1270:                             return self.apply_swizzle(obj.get(swizzle_str), swizzle_str) if isinstance(obj.get(swizzle_str), list) else self.apply_swizzle(obj, swizzle_str)
1271:                         return self.apply_swizzle(obj, swizzle_str)
1272: 
1273:                     return 0
1274:                 else:
1275:                     # 多级访问: input.Color.g (Color不是纯swizzle字符)
1276:                     if len(parts) == 2:
1277:                         # 两级访问但不是swizzle模式: input.Color
1278:                         # 直接查local_vars中是否存在 'input.Color'
1279:                         full_name = f'{base_name}.{parts[1]}'
1280:                         if full_name in local_vars:
1281:                             return local_vars[full_name]
1282:                         # 检查 base_name 是否在local_vars中作为dict
1283:                         if base_name in local_vars:
1284:                             obj = local_vars[base_name]
1285:                             if isinstance(obj, dict):
1286:                                 return obj.get(parts[1], 0)
1287:                             elif isinstance(obj, list):
1288:                                 # base_name是列表(比如input.Pos是float3),parts[1]是访问其元素
1289:                                 idx_map = {'x': 0, 'y': 1, 'z': 2, 'w': 3, 'r': 0, 'g': 1, 'b': 2, 'a': 3}
1290:                                 if parts[1].lower() in idx_map:
1291:                                     idx = idx_map[parts[1].lower()]
1292:                                     return obj[idx] if idx < len(obj) else 0
1293:                         # 检查cbuffer
1294:                         for cb_name, cb_def in self.cbuffers.items():
1295:                             if isinstance(cb_def, CbufferDefinition):
1296:                                 for field in cb_def.fields:
1297:                                     if field.name == base_name:
1298:                                         if field.data is not None:
1299:                                             return self.apply_swizzle(field.data, parts[1])
1300:                                         return 0
1301:                         return 0
1302:                     elif len(parts) == 3:
1303:                         # input.Color.g -> 获取 input.Color, 然后对结果应用 .g
1304:                         # 直接查找 input.Color 是否在local_vars中
1305:                         full_name = f'{base_name}.{parts[1]}'  # 'input.Color'
1306:                         if full_name in local_vars:
1307:                             base_val = local_vars[full_name]
1308:                         else:
1309:                             base_val = self.get_value(f'{base_name}.{parts[1]}', local_vars)
1310:                         if isinstance(base_val, list):
1311:                             idx_map = {'x': 0, 'y': 1, 'z': 2, 'w': 3, 'r': 0, 'g': 1, 'b': 2, 'a': 3}
1312:                             swizzle_ch = parts[2].lower()
1313:                             if swizzle_ch in idx_map:
1314:                                 return base_val[idx_map[swizzle_ch]] if idx_map[swizzle_ch] < len(base_val) else 0
1315:                         return 0
1316:                     else:
1317:                         # 超过3级,递归处理
1318:                         return self.get_value('.'.join(parts[1:]), local_vars)
1319: 
1320:         # 局部变量查找
1321:         if name in local_vars:
1322:             val = local_vars[name]
1323:             return val
1324: 
1325:         base_name = name.split('.')[0] if '.' in name else name
1326: 
1327:         # cbuffer字段查找
1328:         for cb_name, cb_def in self.cbuffers.items():
1329:             if isinstance(cb_def, CbufferDefinition):
1330:                 for field in cb_def.fields:
1331:                     if field.name == base_name:
1332:                         return field.data if field.data is not None else 0
1333: 
1334:         # 全局变量查找
1335:         if name in self.variables:
1336:             return self.variables[name]
1337: 
1338:         # 嵌套cbuffer查找
1339:         try:
1340:             if '.' in name:
1341:                 parts = name.split('.')
1342:                 base = parts[0]
1343:                 for cb_name, cb_data in self.cbuffers.items():
1344:                     if base in cb_data:
1345:                         val = cb_data[base]
1346:                         for p in parts[1:]:
1347:                             if isinstance(val, list) and p in ['x', 'y', 'z', 'w']:
1348:                                 idx = ['x', 'y', 'z', 'w'].index(p)
1349:                                 val = val[idx] if idx < len(val) else 0
1350:                             else:
1351:                                 break
1352:                         return val
1353:         except:
1354:             pass
1355: 
1356:         return 0.0
1357: 
1358:     def execute_statement(self, stmt: str, local_vars: Dict[str, Any]):
1359:         """
1360:         执行单条HLSL语句
1361:         stmt: 语句字符串，如 "float3 pos = input.Pos;" 或 "output.Color = float4(1,0,0,1);"
1362:         local_vars: 局部变量字典
1363:         """
1364:         stmt = stmt.strip()
1365:         if not stmt:
1366:             return None
1367: 
1368:         self.debug_print(f"\n[STMT] Executing: {stmt}")
1369:         input_snapshot = {k: v for k, v in local_vars.items() if k.startswith('input.') or k == 'output'}
1370: 
1371:         # if-else条件语句处理
1372:         if stmt.startswith('if'):
1373:             self.execute_if_statement(stmt, local_vars)
1374:             return None
1375: 
1376:         # 变量声明语句: float4 pos = ...;
1377:         match = self.patterns['variable_declaration'].match(stmt)
1378:         if match:
1379:             var_name = match.group(2)
1380:             value = self.evaluate_expression(match.group(3), local_vars)
1381:             local_vars[var_name] = value
1382:             self.debug_print(f"[STMT] {stmt} => {var_name} = {self._format_value(value)}")
1383:             return None
1384: 
1385:         # output字段赋值: output.Color = ...; 或 output.Color.r = ...;
1386:         if 'output.' in stmt:
1387:             match = self.patterns['output_field_assignment'].match(stmt)
1388:             if match:
1389:                 field_name = match.group(1)
1390:                 swizzle = match.group(2)
1391:                 value_expr = match.group(3).rstrip(';').strip()
1392:                 value = self.evaluate_expression(value_expr, local_vars)
1393: 
1394:                 if 'output' not in local_vars:
1395:                     local_vars['output'] = {}
1396: 
1397:                 if swizzle is None:
1398:                     local_vars['output'][field_name] = value
1399:                 else:
1400:                     if field_name not in local_vars['output']:
1401:                         local_vars['output'][field_name] = [0.0, 0.0, 0.0, 0.0]
1402:                     current = local_vars['output'][field_name]
1403:                     if not isinstance(current, list):
1404:                         current = [current, 0.0, 0.0, 0.0]
1405: 
1406:                     swizzle_map = {'x': 0, 'y': 1, 'z': 2, 'w': 3, 'r': 0, 'g': 1, 'b': 2, 'a': 3}
1407:                     if isinstance(value, list):
1408:                         for i, ch in enumerate(swizzle.lower()):
1409:                             if ch in swizzle_map and i < len(value):
1410:                                 current[swizzle_map[ch]] = value[i]
1411:                     else:
1412:                         ch = swizzle.lower()[0] if swizzle else 'x'
1413:                         if ch in swizzle_map:
1414:                             current[swizzle_map[ch]] = value
1415: 
1416:                     local_vars['output'][field_name] = current
1417:                 self.debug_print(f"[STMT] {stmt} => output.{field_name}" + (f".{swizzle}" if swizzle else "") + f" = {self._format_float(value)}")
1418:                 return None
1419: 
1420:         # 一般赋值语句: var = ...;
1421:         if '=' in stmt and stmt.count('=') == 1:
1422:             match = self.patterns['simple_assignment'].match(stmt)
1423:             if match:
1424:                 var_name = match.group(1)
1425:                 value = self.evaluate_expression(match.group(2), local_vars)
1426:                 local_vars[var_name] = value
1427:                 self.debug_print(f"[STMT] {stmt} => {var_name} = {value}")
1428:                 return None
1429: 
1430:         self.debug_print(f"[STMT] {stmt} => (no assignment)")
1431:         return None
1432: 
1433:     def execute_if_statement(self, stmt: str, local_vars: Dict[str, Any]):
1434:         """
1435:         执行if-else条件语句
1436:         stmt: if语句字符串
1437:         local_vars: 局部变量字典
1438:         """
1439:         stmt = stmt.strip()
1440: 
1441:         if_match = self.patterns['if_statement'].match(stmt)
1442:         if not if_match:
1443:             return
1444: 
1445:         condition_expr = if_match.group(1).strip()
1446:         then_branch = if_match.group(2).strip()
1447: 
1448:         cond_value = self.evaluate_expression(condition_expr, local_vars)
1449:         self.debug_print(f"[IF] condition: {condition_expr} => {cond_value}")
1450: 
1451:         if cond_value:
1452:             if then_branch.startswith('{'):
1453:                 self.execute_block(then_branch, local_vars)
1454:             elif not then_branch.startswith('else'):
1455:                 self.execute_statement(then_branch, local_vars)
1456:         else:
1457:             else_pos = self.find_else_branch(then_branch)
1458:             if else_pos >= 0:
1459:                 else_branch = then_branch[else_pos:].strip()
1460:                 if else_branch.startswith('else'):
1461:                     else_branch = else_branch[4:].strip()
1462:                     if else_branch.startswith('{'):
1463:                         self.execute_block(else_branch, local_vars)
1464:                     else:
1465:                         self.execute_statement(else_branch, local_vars)
1466: 
1467:     def find_else_branch(self, stmt: str) -> int:
1468:         """
1469:         查找else分支的起始位置(不在嵌套括号内)
1470:         stmt: 语句字符串
1471:         返回: else关键字位置，或-1表示未找到
1472:         """
1473:         depth = 0
1474:         pos = 0
1475:         while pos < len(stmt):
1476:             char = stmt[pos]
1477:             if char == '(':
1478:                 depth += 1
1479:             elif char == ')':
1480:                 depth -= 1
1481:             elif char == '{':
1482:                 depth += 1
1483:             elif char == '}':
1484:                 depth -= 1
1485:             elif depth == 0 and stmt[pos:pos+4] == 'else':
1486:                 return pos
1487:             pos += 1
1488:         return -1
1489: 
1490:     def execute_block(self, block: str, local_vars: Dict[str, Any]):
1491:         """
1492:         执行语句块(被大括号包围的语句列表)
1493:         block: 语句块字符串
1494:         local_vars: 局部变量字典
1495:         """
1496:         block = block.strip()
1497:         if not block.startswith('{') or not block.endswith('}'):
1498:             return
1499: 
1500:         inner = block[1:-1].strip()
1501:         if not inner:
1502:             return
1503: 
1504:         statements = self.GenerateStmts(inner)
1505:         for stmt in statements:
1506:             self.execute_statement(stmt, local_vars)
1507: 
1508:     def GenerateStmts(self, code: str):
1509:         statements = []
1510:         current_stmt = []
1511:         brace_count = 0
1512:         paren_count = 0
1513:         in_string = False
1514:         string_char = None
1515: 
1516:         for char in code:
1517:             if char == '{':
1518:                 brace_count += 1
1519:                 current_stmt.append(char)
1520:             elif char == '}':
1521:                 if brace_count > 0:
1522:                     current_stmt.append(char)
1523:                 brace_count -= 1
1524:                 if brace_count == 0 and current_stmt:
1525:                     stmt = ''.join(current_stmt).strip()
1526:                     if stmt:
1527:                         statements.append(stmt)
1528:                     current_stmt = []
1529:             elif char == '(':
1530:                 paren_count += 1
1531:                 current_stmt.append(char)
1532:             elif char == ')':
1533:                 paren_count -= 1
1534:                 current_stmt.append(char)
1535:             elif char in '"\'':
1536:                 if not in_string:
1537:                     in_string = True
1538:                     string_char = char
1539:                 elif char == string_char:
1540:                     in_string = False
1541:                     string_char = None
1542:                 current_stmt.append(char)
1543:             elif char == ';' and brace_count == 0 and paren_count == 0 and not in_string:
1544:                 stmt = ''.join(current_stmt).strip()
1545:                 if stmt:
1546:                     statements.append(stmt)
1547:                 current_stmt = []
1548:             else:
1549:                 current_stmt.append(char)
1550: 
1551:         if current_stmt:
1552:             stmt = ''.join(current_stmt).strip()
1553:             if stmt:
1554:                 statements.append(stmt)
1555: 
1556:         return statements
1557: 
1558: 
1559:     def execute_main_function(self, code: str, main_func: str, input_struct_name: str, row_index: int, data: Dict[str, Any]):
1560:         """
1561:         执行HLSL main函数
1562:         code: HLSL代码
1563:         main_func: main函数名
1564:         input_struct_name: 输入结构体名
1565:         row_index: 数据行索引
1566:         data: 输入数据字典
1567:         返回: output结构体字典
1568:         """
1569:         input_struct = self.structs.get(input_struct_name)
1570:         if not input_struct:
1571:             self.log_output(f"Cannot find input_struct: {input_struct_name}\n")
1572:             return None
1573: 
1574:         input_fields = {}
1575:         for field in input_struct.fields:
1576:             input_fields[field.name] = field.field_type
1577: 
1578:         # 查找main函数签名
1579:         func_signature_pattern = r'(\w+)\s+' + re.escape(main_func) + r'\s*\(\s*(\w+)\s+input\s*\)'
1580:         func_signature_match = re.search(func_signature_pattern, code)
1581:         if not func_signature_match:
1582:             return None
1583: 
1584:         output_struct_name = func_signature_match.group(1)
1585:         input_struct_name_from_func = func_signature_match.group(2)
1586: 
1587:         if output_struct_name not in self.structs:
1588:             return None
1589: 
1590:         output_struct = self.structs[output_struct_name]
1591:         output_fields = {}
1592:         for field in output_struct.fields:
1593:             output_fields[field.name] = field.field_type
1594: 
1595:         func_signature = rf'{output_struct_name}\s+{main_func}\s*\(\s*{input_struct_name_from_func}\s+input\s*\)'
1596: 
1597:         cache_key = f"{output_struct_name}_{main_func}_{input_struct_name_from_func}"
1598:         if cache_key in self._parsed_func_cache:
1599:             cached = self._parsed_func_cache[cache_key]
1600:             body = cached['body']
1601:             statements = cached['statements']
1602:         else:
1603:             func_start = re.search(func_signature, code)
1604:             if not func_start:
1605:                 return None
1606: 
1607:             open_brace_pos = func_start.end()
1608:             brace_depth = 1
1609:             pos = open_brace_pos
1610:             while pos < len(code) and brace_depth > 0:
1611:                 if code[pos] == '{':
1612:                     brace_depth += 1
1613:                 elif code[pos] == '}':
1614:                     brace_depth -= 1
1615:                 pos += 1
1616: 
1617:             body = code[open_brace_pos+1:pos-1].strip()
1618:             if body.startswith('{'):
1619:                 body = body[1:].strip()
1620:             if body.endswith('}'):
1621:                 body = body[:-1].strip()
1622: 
1623:             statements = self.GenerateStmts(body)
1624:             self._parsed_func_cache[cache_key] = {'body': body, 'statements': statements}
1625: 
1626:         # 初始化局部变量
1627:         local_vars = {'data': data}
1628: 
1629:         # 设置input字段变量
1630:         for field_name, field_value in data.items():
1631:             local_vars[f'input.{field_name}'] = field_value
1632: 
1633:         # 初始化output对象
1634:         output_obj = {}
1635:         for field in output_fields:
1636:             output_obj[field] = None
1637:         local_vars['output'] = output_obj
1638: 
1639:         ret_val = None
1640: 
1641:         self._eval_counter += 1
1642:         self._should_print = ((self._eval_counter - 1) % self.print_sequence == 0)
1643: 
1644:         self.debug_print(f"******************************************************")
1645:         self.debug_print(f"**************Begin {self._eval_counter}**************")
1646:         self.debug_print(f"******************************************************\n")
1647: 
1648:         self.debug_print(f"\n=== INPUT DATA ===")
1649:         for k, v in local_vars.items():
1650:             if k.startswith('input.') or k == 'output':
1651:                 self.debug_print(f"  {k} = {v}")
1652:         self.debug_print(f"==================")
1653: 
1654:         # 顺序执行语句
1655:         i = 0
1656:         while i < len(statements):
1657:             stmt = statements[i]
1658:             if stmt is None:
1659:                 i += 1
1660:                 continue
1661: 
1662:             if 'return' in stmt and 'output' in stmt:
1663:                 ret_val = local_vars.get('output')
1664:                 i += 1
1665:                 continue
1666: 
1667:             # 检查是否是if语句，且下一条是else
1668:             if stmt.startswith('if'):
1669:                 next_i = i + 1
1670:                 # 查找下一个非None的语句
1671:                 while next_i < len(statements) and statements[next_i] is None:
1672:                     next_i += 1
1673:                 
1674:                 if next_i < len(statements) and statements[next_i].startswith('else'):
1675:                     # 合并if和else为完整语句
1676:                     full_if_stmt = stmt + '\n' + statements[next_i]
1677:                     self.execute_if_statement(full_if_stmt, local_vars)
1678:                     statements[next_i] = None  # 标记else已处理
1679:                 else:
1680:                     self.execute_if_statement(stmt, local_vars)
1681:             else:
1682:                 self.execute_statement(stmt, local_vars)
1683: 
1684:             i += 1
1685: 
1686:         self.debug_print(f"******************************************************")
1687:         self.debug_print(f"**************End {self._eval_counter}**************")
1688:         self.debug_print(f"******************************************************\n")
1689: 
1690:         return ret_val
1691: 
1692:     def interpret(self, hlsl_file_path: str, csv_folder_path: str = None):
1693:         """
1694:         解释HLSL代码 - 解析结构体和cbuffer定义
1695:         hlsl_file_path: HLSL文件路径
1696:         csv_folder_path: CSV文件夹路径（如果为None则不加载CSV数据）
1697:         """
1698:         if not os.path.exists(hlsl_file_path):
1699:             self.log_output(f"Error: HLSL file not found: {hlsl_file_path}")
1700:             return
1701: 
1702:         with open(hlsl_file_path, 'r', encoding='utf-8') as f:
1703:             self.hlsl_code = f.read()
1704: 
1705:         code = self.hlsl_code
1706: 
1707:         if csv_folder_path is None:
1708:             csv_folder_path = os.path.dirname(hlsl_file_path)
1709: 
1710:         # 解析struct定义
1711:         for struct_match in self.patterns['struct_finditer'].finditer(code):
1712:             struct_def = self.parse_struct(struct_match.group())
1713:             if struct_def:
1714:                 self.structs[struct_def.name] = struct_def
1715: 
1716:         # 解析cbuffer定义
1717:         for cb_match in self.patterns['cbuffer_finditer'].finditer(code):
1718:             cb_def = self.parse_cbuffer(cb_match.group())
1719:             if cb_def:
1720:                 self.cbuffers[cb_def.name] = cb_def
1721: 
1722:         # 从CSV加载struct数据
1723:         for struct_name in self.structs:
1724:             csv_path = os.path.join(csv_folder_path, f'{struct_name}.csv')
1725:             if os.path.exists(csv_path):
1726:                 self.load_struct_data_from_csv(struct_name, csv_path)
1727: 
1728:         # 从CSV加载cbuffer数据
1729:         for cb_name in self.cbuffers:
1730:             csv_path = os.path.join(csv_folder_path, f'{cb_name}.csv')
1731:             if os.path.exists(csv_path):
1732:                 self.load_cbuffer_data_from_csv(cb_name, csv_path)
1733: 
1734:     def executeVS(self, main_func: str, vs_input: str, code: str = None, execute_count: int = None):
1735:         """
1736:         执行顶点着色器
1737:         main_func: 入口函数名
1738:         vs_input: 输入结构体名
1739:         code: HLSL代码（如果为None则使用self.hlsl_code）
1740:         execute_count: 执行次数（如果为None则使用input_struct.fields计算行数）
1741:         返回: 输出结构体字典列表
1742:         """
1743:         if code is None:
1744:             code = self.hlsl_code
1745:         self._last_executeVS_code = code
1746:         input_struct = self.structs.get(vs_input)
1747:         if not input_struct:
1748:             self.log_output(f"Cannot find vs input: {vs_input}\n")
1749:             return None
1750: 
1751:         output_struct_name = None
1752:         func_signature_pattern = r'(\w+)\s+' + re.escape(main_func) + r'\s*\(\s*(\w+)\s+input\s*\)'
1753:         func_signature_match = re.search(func_signature_pattern, code)
1754:         if func_signature_match:
1755:             output_struct_name = func_signature_match.group(1)
1756: 
1757:         output_struct = self.structs.get(output_struct_name) if output_struct_name else None
1758: 
1759:         self.vertex_pool.clear()
1760:         self.vertex_pool.set_input_struct(input_struct)
1761:         self.vertex_pool.set_output_struct(output_struct)
1762: 
1763:         # clear eval counter
1764:         self._eval_counter = 0
1765: 
1766:         if execute_count is None:
1767:             num_rows = 0
1768:             for field in input_struct.fields:
1769:                 if field.data:
1770:                     num_rows = max(num_rows, len(field.data))
1771:             execute_count = num_rows
1772: 
1773:         if self.max_workers > 1:
1774:             def execute_row(row_index: int):
1775:                 data = {}
1776:                 for field in input_struct.fields:
1777:                     if field.data and row_index < len(field.data):
1778:                         data[field.name] = field.data[row_index]
1779:                 self.vertex_pool.build_from_input(vs_input, data, row_index)
1780:                 result = self.execute_main_function(code, main_func, vs_input, row_index, data)
1781:                 self.vertex_pool.update_output(row_index, result)
1782:                 return row_index, result
1783: 
1784:             print(f"Run thread workers")
1785:             results = [None] * execute_count
1786:             with ThreadPoolExecutor(max_workers=self.max_workers) as executor:
1787:                 futures = [executor.submit(execute_row, i) for i in range(execute_count)]
1788:                 for future in futures:
1789:                     idx, result = future.result()
1790:                     results[idx] = result
1791:         else:
1792:             print(f"Run single thread")
1793:             results = []
1794:             for row_index in range(execute_count):
1795:                 data = {}
1796:                 for field in input_struct.fields:
1797:                     if field.data and row_index < len(field.data):
1798:                         data[field.name] = field.data[row_index]
1799:                 self.vertex_pool.build_from_input(vs_input, data, row_index)
1800:                 result = self.execute_main_function(code, main_func, vs_input, row_index, data)
1801:                 self.vertex_pool.update_output(row_index, result)
1802:                 results.append(result)
1803: 
1804:         return results
1805: 
1806:     def executePS(self, code: str, main_func: str, ps_input: str):
1807:         """
1808:         执行像素着色器(当前为占位函数)
1809:         """
1810:         pass
1811: 
1812:     def load_struct_data_from_csv(self, struct_name: str, csv_path: str):
1813:         """
1814:         从CSV文件加载struct数据
1815:         struct_name: 结构体名称
1816:         csv_path: CSV文件路径
1817:         """
1818:         if struct_name not in self.structs:
1819:             return
1820:         struct_def = self.structs[struct_name]
1821:         rows = self.load_csv(csv_path)
1822:         if not rows or len(rows) < 2:
1823:             return
1824: 
1825:         header = rows[0]
1826:         data_rows = rows[1:]
1827: 
1828:         # 建立字段列索引映射
1829:         field_col_indices = {}
1830:         for i, col in enumerate(header):
1831:             col_clean = col.strip()
1832:             if '.' in col_clean:
1833:                 parts = col_clean.split('.')
1834:                 base_name = parts[0]
1835:                 suffix = parts[1]
1836:                 if base_name not in field_col_indices:
1837:                     field_col_indices[base_name] = {}
1838:                 field_col_indices[base_name][suffix] = i
1839: 
1840:         # 填充字段数据
1841:         for field in struct_def.fields:
1842:             if field.semantic in field_col_indices:
1843:                 col_dict = field_col_indices[field.semantic]
1844:                 values = []
1845:                 for row in data_rows:
1846:                     if 'x' in col_dict and 'y' in col_dict and 'z' in col_dict and 'w' in col_dict:
1847:                         x = float(row[col_dict['x']].strip())
1848:                         y = float(row[col_dict['y']].strip())
1849:                         z = float(row[col_dict['z']].strip())
1850:                         w = float(row[col_dict['w']].strip())
1851:                         values.append([x, y, z, w])
1852:                     elif 'x' in col_dict and 'y' in col_dict and 'z' in col_dict:
1853:                         x = float(row[col_dict['x']].strip())
1854:                         y = float(row[col_dict['y']].strip())
1855:                         z = float(row[col_dict['z']].strip())
1856:                         values.append([x, y, z])
1857:                     elif 'x' in col_dict and 'y' in col_dict:
1858:                         x = float(row[col_dict['x']].strip())
1859:                         y = float(row[col_dict['y']].strip())
1860:                         values.append([x, y])
1861:                     else:
1862:                         val_str = row[col_dict['x']].strip().strip('"')
1863:                         values.append(self.parse_value_by_type(val_str, field.field_type))
1864:                 field.data = values
1865:                 self.log_output(f"Field '{field.semantic}' ({field.field_type}): {values[0] if values else 'N/A'}")
1866: 
1867:     def load_cbuffer_data_from_csv(self, cb_name: str, csv_path: str):
1868:         """
1869:         从CSV文件加载cbuffer数据
1870:         cb_name: cbuffer名称
1871:         csv_path: CSV文件路径
1872:         """
1873:         if cb_name not in self.cbuffers:
1874:             return
1875:         cb_def = self.cbuffers[cb_name]
1876:         rows = self.load_csv(csv_path)
1877:         if not rows or len(rows) < 2:
1878:             return
1879: 
1880:         header = rows[0]
1881:         name_idx = header.index('Name') if 'Name' in header else -1
1882:         value_idx = header.index('Value') if 'Value' in header else -1
1883:         type_idx = header.index('Type') if 'Type' in header else -1
1884: 
1885:         if name_idx == -1 or value_idx == -1:
1886:             return
1887: 
1888:         matrix_rows = {}
1889:         scalar_vars = {}
1890: 
1891:         for row in rows[1:]:
1892:             if len(row) <= max(name_idx, value_idx):
1893:                 continue
1894:             var_name = row[name_idx].strip().strip('"')
1895:             value_str = row[value_idx].strip().strip('"') if value_idx < len(row) else ''
1896:             type_str = row[type_idx].strip().strip('"') if type_idx != -1 and type_idx < len(row) else ''
1897: 
1898:             # 跳过空值
1899:             if value_str == '':
1900:                 continue
1901: 
1902:             # 矩阵行处理(如 World.row0, World.row1)
1903:             if '.' in var_name:
1904:                 parts = var_name.split('.')
1905:                 base_name = parts[0]
1906:                 suffix = parts[1]
1907:                 if suffix.startswith('row'):
1908:                     row_idx = int(suffix[3:])
1909:                     if base_name not in matrix_rows:
1910:                         matrix_rows[base_name] = {}
1911:                     matrix_rows[base_name][row_idx] = (value_str, type_str)
1912:             else:
1913:                 scalar_vars[var_name] = (value_str, type_str)
1914: 
1915:         # 填充字段数据
1916:         for field in cb_def.fields:
1917:             if field.name in matrix_rows:
1918:                 row_dict = matrix_rows[field.name]
1919:                 if all(i in row_dict for i in range(4)):
1920:                     matrix = []
1921:                     for i in range(4):
1922:                         value_str, type_str = row_dict[i]
1923:                         parts = value_str.split(',')
1924:                         matrix.append([float(p.strip()) for p in parts[:4]])
1925:                     field.data = matrix
1926:             elif field.name in scalar_vars:
1927:                 value_str, type_str = scalar_vars[field.name]
1928:                 field.data = self.parse_value_by_type(value_str, type_str)
1929: 
1930:         # 打印cbuffer内容
1931:         cb_n = cb_name
1932:         cb_d = cb_def
1933:         self.log_output(f"Cbuffer {cb_n}:")
1934:         for f in cb_d.fields:
1935:             data = f.data
1936:             ft = f.field_type
1937:             if 'float4x4' in ft:
1938:                 self.log_output(f"  {f.name} ({ft}):")
1939:                 for row in data:
1940:                     row_str = '  '.join(f"{v:12.5f}" for v in row)
1941:                     self.log_output(f"    [{row_str}]")
1942:             elif 'float3x3' in ft:
1943:                 self.log_output(f"  {f.name} ({ft}):")
1944:                 for row in data:
1945:                     row_str = '  '.join(f"{v:12.5f}" for v in row)
1946:                     self.log_output(f"    [{row_str}]")
1947:             elif 'float4' in ft:
1948:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
1949:             elif 'float3' in ft:
1950:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
1951:             elif 'float2' in ft:
1952:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
1953:             elif 'float' in ft:
1954:                 self.log_output(f"  {f.name} ({ft}): {data:.5f}")
1955:             elif 'uint4' in ft:
1956:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1957:             elif 'uint3' in ft:
1958:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1959:             elif 'uint2' in ft:
1960:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1961:             elif 'uint' in ft:
1962:                 self.log_output(f"  {f.name} ({ft}): {data}")
1963:             elif 'int4' in ft:
1964:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1965:             elif 'int3' in ft:
1966:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1967:             elif 'int2' in ft:
1968:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1969:             elif 'int' in ft:
1970:                 self.log_output(f"  {f.name} ({ft}): {data}")
1971:             elif 'bool' in ft:
1972:                 self.log_output(f"  {f.name} ({ft}): {data}")
1973:             else:
1974:                 self.log_output(f"  {f.name} ({ft}): {data}")
1975: 
1976:     def load_vs_output_golden_from_csv(self, csv_path: str):
1977:         """
1978:         从CSV文件加载VS_OUTPUT的golden数据
1979:         csv_path: CSV文件路径
1980:         """
1981:         if "VS_OUTPUT" not in self.structs:
1982:             self.log_output("Error: VS_OUTPUT struct not defined")
1983:             return False
1984: 
1985:         vs_output_def = self.structs["VS_OUTPUT"]
1986:         rows = self.load_csv(csv_path)
1987:         if not rows or len(rows) < 2:
1988:             self.log_output(f"Error: CSV file {csv_path} is empty or has no data rows")
1989:             return False
1990: 
1991:         header = rows[0]
1992:         data_rows = rows[1:]
1993: 
1994:         field_col_indices = {}
1995:         for i, col in enumerate(header):
1996:             col_clean = col.strip()
1997:             if '.' in col_clean:
1998:                 parts = col_clean.split('.')
1999:                 base_name = parts[0]
2000:                 suffix = parts[1]
2001:                 if base_name not in field_col_indices:
2002:                     field_col_indices[base_name] = {}
2003:                 field_col_indices[base_name][suffix] = i
2004: 
2005:         for field in vs_output_def.fields:
2006:             if field.semantic in field_col_indices:
2007:                 col_dict = field_col_indices[field.semantic]
2008:                 values = []
2009:                 for row in data_rows:
2010:                     try:
2011:                         if 'x' in col_dict and 'y' in col_dict and 'z' in col_dict and 'w' in col_dict:
2012:                             x = float(row[col_dict['x']].strip())
2013:                             y = float(row[col_dict['y']].strip())
2014:                             z = float(row[col_dict['z']].strip())
2015:                             w = float(row[col_dict['w']].strip())
2016:                             values.append([x, y, z, w])
2017:                         elif 'x' in col_dict and 'y' in col_dict and 'z' in col_dict:
2018:                             x = float(row[col_dict['x']].strip())
2019:                             y = float(row[col_dict['y']].strip())
2020:                             z = float(row[col_dict['z']].strip())
2021:                             values.append([x, y, z])
2022:                         elif 'x' in col_dict and 'y' in col_dict:
2023:                             x = float(row[col_dict['x']].strip())
2024:                             y = float(row[col_dict['y']].strip())
2025:                             values.append([x, y])
2026:                         else:
2027:                             val_str = row[col_dict['x']].strip().strip('"')
2028:                             values.append(self.parse_value_by_type(val_str, field.field_type))
2029:                     except (ValueError, IndexError) as e:
2030:                         self.log_output(f"Warning: Failed to parse {field.semantic} at row: {e}")
2031:                         values.append(None)
2032:                 field.data = values
2033: 
2034:         self.log_output(f"Loaded {len(data_rows)} golden data rows for VS_OUTPUT")
2035:         return True
2036: 
2037:     def compare_vs_output_with_golden(self, hlsl_output: List[Dict], output_struct_name: str = "VS_OUTPUT", float_tolerance: float = 0.0001, execute_count: int = None) -> bool:
2038:         """
2039:         比较HLSL执行结果与golden数据
2040:         hlsl_output: executeVS返回的输出结构体字典列表
2041:         output_struct_name: 输出结构体名称，用于获取field name (默认"VS_OUTPUT")
2042:         float_tolerance: 浮点类型数据的比较误差容忍度
2043:         execute_count: 执行次数（如果为None则使用golden数据计算行数）
2044:         返回: True表示所有数据匹配, False表示存在不匹配
2045:         """
2046:         if output_struct_name not in self.structs:
2047:             self.log_output(f"Error: {output_struct_name} struct not found")
2048:             return False
2049: 
2050:         vs_output_def = self.structs[output_struct_name]
2051:         golden_data = {}
2052:         semantic_to_field = {}
2053: 
2054:         for field in vs_output_def.fields:
2055:             if field.data:
2056:                 golden_data[field.semantic] = field.data
2057:             semantic_to_field[field.semantic] = field.name
2058: 
2059:         num_golden_rows = 0
2060:         for field_data in golden_data.values():
2061:             if field_data:
2062:                 num_golden_rows = max(num_golden_rows, len(field_data))
2063: 
2064:         if execute_count is not None:
2065:             num_golden_rows = execute_count
2066: 
2067:         if not hlsl_output:
2068:             self.log_output("Error: No HLSL output to compare")
2069:             return False
2070: 
2071:         if len(hlsl_output) != num_golden_rows:
2072:             self.log_output(f"Error: Row count mismatch - HLSL output has {len(hlsl_output)} rows, golden has {num_golden_rows} rows")
2073:             return False
2074: 
2075:         all_match = True
2076:         passed_count = 0
2077:         field_type_map = {}
2078:         for field in vs_output_def.fields:
2079:             field_type_map[field.semantic] = field.field_type
2080: 
2081:         for row_idx in range(len(hlsl_output)):
2082:             output_row = hlsl_output[row_idx]
2083:             row_match = True
2084:             for semantic, golden_values in golden_data.items():
2085:                 if row_idx >= len(golden_values):
2086:                     continue
2087: 
2088:                 field_name = semantic_to_field.get(semantic, semantic)
2089:                 if field_name not in output_row:
2090:                     continue
2091: 
2092:                 output_value = output_row[field_name]
2093:                 golden_value = golden_values[row_idx]
2094: 
2095:                 if output_value is None or golden_value is None:
2096:                     continue
2097: 
2098:                 field_type = field_type_map.get(semantic, '')
2099: 
2100:                 if isinstance(output_value, list) and isinstance(golden_value, list):
2101:                     if len(output_value) != len(golden_value):
2102:                         self.log_output(f"Error: Row {row_idx}, {field_name}: length mismatch output={len(output_value)} golden={len(golden_value)}")
2103:                         row_match = False
2104:                         continue
2105: 
2106:                     is_float = 'float' in field_type
2107:                     for comp_idx in range(len(output_value)):
2108:                         out_comp = output_value[comp_idx]
2109:                         gold_comp = golden_value[comp_idx]
2110: 
2111:                         if is_float:
2112:                             if isinstance(out_comp, float) and isinstance(gold_comp, float):
2113:                                 if abs(out_comp - gold_comp) > float_tolerance:
2114:                                     self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp:.6f} golden={gold_comp:.6f} diff={abs(out_comp - gold_comp):.6f} > tolerance={float_tolerance}")
2115:                                     row_match = False
2116:                             elif out_comp != gold_comp:
2117:                                 self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp} golden={gold_comp} (float comparison failed)")
2118:                                 row_match = False
2119:                         else:
2120:                             if out_comp != gold_comp:
2121:                                 self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp} golden={gold_comp} (strict equality failed)")
2122:                                 row_match = False
2123: 
2124:             if row_match:
2125:                 passed_count += 1
2126:             else:
2127:                 all_match = False
2128: 
2129:         self.log_output(f"Total PASSED rows: {passed_count}/{num_golden_rows}")
2130:         if all_match:
2131:             self.log_output("Comparison PASSED: All output data matches golden data within tolerance")
2132:         else:
2133:             self.log_output("Comparison FAILED: Some output data does not match golden data")
2134: 
2135:         return all_match
2136: 
2137:     def get_cbuffer_data(self):
2138:         """
2139:         Get all cbuffer definitions and their data
2140:         Returns: dict of cbuffer_name -> {fields: [{name, field_type, data}, ...]}
2141:         """
2142:         result = {}
2143:         for cb_name, cb_def in self.cbuffers.items():
2144:             if isinstance(cb_def, CbufferDefinition):
2145:                 fields_data = []
2146:                 for field in cb_def.fields:
2147:                     fields_data.append({
2148:                         'name': field.name,
2149:                         'field_type': field.field_type,
2150:                         'data': field.data
2151:                     })
2152:                 result[cb_name] = {'fields': fields_data}
2153:         return result
2154: 
2155:     def get_hlsl_code(self):
2156:         """
2157:         Get the current HLSL code
2158:         Returns: str - the HLSL code
2159:         """
2160:         return self.hlsl_code
2161: 
2162:     def get_last_executeVS_code(self):
2163:         """
2164:         Get the last code used in executeVS
2165:         Returns: str - the last code passed to executeVS, or None
2166:         """
2167:         return getattr(self, '_last_executeVS_code', None)
2168: 
2169: 
2170: def main():
2171:     import sys
2172: 
2173:     if len(sys.argv) < 2:
2174:         print("Usage: python hlsl_interpreter.py <config.json>")
2175:         print("Config JSON should contain: hlsl_file_path, csv_folder_path, log_file_path")
2176:         config_path = './wrong_constant_attenuation.json'
2177:     else:
2178:         config_path = sys.argv[1]
2179: 
2180:     if not os.path.exists(config_path):
2181:         print(f"Error: Config file not found: {config_path}")
2182:         sys.exit(1)
2183: 
2184:     config = {}
2185:     with open(config_path, 'r', encoding='utf-8') as f:
2186:         config = json.load(f)
2187: 
2188:     hlsl_file_path = config.get('hlsl_file_path', '')
2189:     csv_folder_path = config.get('csv_folder_path', '')
2190:     log_file_path = config.get('log_file_path', 'hlsl_interpreter.log')
2191:     log_file_mode = config.get('log_file_mode', 'a')
2192:     print_sequence = config.get('print_sequence', 1)
2193:     log_to_file = config.get('log_to_file', True)
2194:     printSyntaxTree = config.get('printSyntaxTree', True)
2195:     print_interpreter_result = config.get('print_interpreter_result', True)
2196:     float_tolerance = config.get('float_tolerance', 0.0001)
2197:     output_struct_name = config.get('output_struct_name', 'VS_OUTPUT')
2198:     execute_count = config.get('execute_count', None)
2199:     max_workers = config.get('max_workers', 1)
2200:     primitive_topology = config.get('primitive_topology', D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
2201:     mesh_view_enabled = config.get('mesh_view_enabled', False)
2202: 
2203:     if not hlsl_file_path:
2204:         print("Error: hlsl_file_path not specified in config")
2205:         sys.exit(1)
2206: 
2207:     if not os.path.exists(hlsl_file_path):
2208:         print(f"Error: HLSL file not found: {hlsl_file_path}")
2209:         sys.exit(1)
2210: 
2211:     if csv_folder_path and not os.path.exists(csv_folder_path):
2212:         print(f"Error: CSV folder not found: {csv_folder_path}")
2213:         sys.exit(1)
2214: 
2215:     interpreter = HLSLInterpreter(
2216:         log_to_file=log_to_file,
2217:         log_file_path=log_file_path,
2218:         log_file_mode=log_file_mode,
2219:         print_sequence=print_sequence,
2220:         printSyntaxTree=printSyntaxTree,
2221:         print_interpreter_result=print_interpreter_result,
2222:         max_workers=max_workers,
2223:         primitive_topology=primitive_topology)
2224: 
2225:     if mesh_view_enabled:
2226:         interpreter.enable_mesh_view(True)
2227: 
2228:     total_start = time.time()
2229: 
2230:     interpret_start = time.time()
2231:     interpreter.interpret(hlsl_file_path, csv_folder_path)
2232:     interpret_time = time.time() - interpret_start
2233: 
2234:     if mesh_view_enabled and interpreter._mesh_view:
2235:         interpreter._mesh_view.set_hlsl_interpreter(interpreter, "main", "VS_INPUT")
2236: 
2237:     golden_csv_path = os.path.join(csv_folder_path, 'VS_OUTPUT.csv') if csv_folder_path else None
2238:     load_golden_start = time.time()
2239:     if golden_csv_path and os.path.exists(golden_csv_path):
2240:         interpreter.load_vs_output_golden_from_csv(golden_csv_path)
2241:     load_golden_time = time.time() - load_golden_start
2242: 
2243:     execute_start = time.time()
2244:     results = interpreter.executeVS("main", "VS_INPUT", execute_count=execute_count)
2245:     execute_time = time.time() - execute_start
2246: 
2247:     r = Rasterizer("rasterizer_param.json")  # 或 Rasterizer() 使用默认配置
2248:     pixels = r.rasterize(results, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
2249: 
2250:     if mesh_view_enabled:
2251:         interpreter.log_output("Displaying input mesh before executeVS...")
2252:         interpreter.show_input_mesh("VS_INPUT")
2253: 
2254:     if mesh_view_enabled and results:
2255:         interpreter.log_output("Displaying result mesh after executeVS...")
2256:         interpreter.show_result_mesh(results)
2257: 
2258:     if interpreter.print_interpreter_result:
2259:         interpreter.log_output("HLSL Interpreter Result:")
2260:         interpreter.log_output("=" * 40)
2261:         if results:
2262:             for idx, result in enumerate(results):
2263:                 interpreter.log_output(f"\n--- Row {idx} ---")
2264:                 if result:
2265:                     for key, value in result.items():
2266:                         if isinstance(value, list):
2267:                             if len(value) == 4:
2268:                                 interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]")
2269:                             elif len(value) == 3:
2270:                                 interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]")
2271:                             elif len(value) == 2:
2272:                                 interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}]")
2273:                             else:
2274:                                 interpreter.log_output(f"{key}: {value}")
2275:                         else:
2276:                             interpreter.log_output(f"{key}: {value}")
2277:         else:
2278:             interpreter.log_output("No result produced")
2279: 
2280:         if results and results[-1] and 'Color' in results[-1]:
2281:             color = results[-1]['Color']
2282:             if color and isinstance(color, list) and len(color) == 4:
2283:                 interpreter.log_output("\nFinal Output Color (RGBA):")
2284:                 interpreter.log_output(f"  R: {color[0]:.4f}")
2285:                 interpreter.log_output(f"  G: {color[1]:.4f}")
2286:                 interpreter.log_output(f"  B: {color[2]:.4f}")
2287:                 interpreter.log_output(f"  A: {color[3]:.4f}")
2288:             else:
2289:                 interpreter.log_output(f"\nColor result: {color}")
2290: 
2291:         interpreter.log_output("\n" + "=" * 40)
2292:     interpreter.log_output("Comparing with golden data...")
2293:     interpreter.log_output("=" * 40)
2294:     compare_start = time.time()
2295:     interpreter.compare_vs_output_with_golden(results, output_struct_name=output_struct_name, float_tolerance=float_tolerance, execute_count=execute_count)
2296:     compare_time = time.time() - compare_start
2297: 
2298:     total_time = time.time() - total_start
2299: 
2300:     interpreter.log_output("\n" + "=" * 40)
2301:     interpreter.log_output("Timing Summary:")
2302:     interpreter.log_output("=" * 40)
2303:     interpreter.log_output(f"interpreter.interpret():             {interpret_time:.4f}s")
2304:     interpreter.log_output(f"interpreter.load_vs_output_golden_from_csv(): {load_golden_time:.4f}s")
2305:     interpreter.log_output(f"interpreter.executeVS():           {execute_time:.4f}s")
2306:     interpreter.log_output(f"compare_vs_output_with_golden():    {compare_time:.4f}s")
2307:     interpreter.log_output(f"Total execution time:               {total_time:.4f}s")
2308: 
2309:     while True:
2310:         user_input = input("\nEnter 'x' to exit, 'o' to open MeshView, 'r' to rerun executeVS: ")
2311:         user_input = user_input.strip().lower()
2312:         if user_input == 'x':
2313:             interpreter._mesh_view.close()
2314:             break
2315:         elif user_input == 'o':
2316:             if interpreter._mesh_view:
2317:                 interpreter._mesh_view.show(blocking=False)
2318:                 interpreter.log_output("MeshView reopened")
2319:         elif user_input == 'r':
2320:             results = []
2321:             execute_start = time.time()
2322:             results = interpreter.executeVS("main", "VS_INPUT", execute_count=execute_count)
2323:             execute_time = time.time() - execute_start
2324:             interpreter.log_output(f"Re-executed executeVS in {execute_time:.4f}s")
2325:             if mesh_view_enabled and results:
2326:                 interpreter.log_output("Displaying result mesh after re-execution...")
2327:                 interpreter.show_result_mesh(results)
2328:             if interpreter.print_interpreter_result:
2329:                 interpreter.log_output("HLSL Interpreter Result (re-run):")
2330:                 interpreter.log_output("=" * 40)
2331:                 if results:
2332:                     for idx, result in enumerate(results):
2333:                         interpreter.log_output(f"\n--- Row {idx} ---")
2334:                         if result:
2335:                             for key, value in result.items():
2336:                                 if isinstance(value, list):
2337:                                     if len(value) == 4:
2338:                                         interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]")
2339:                                     elif len(value) == 3:
2340:                                         interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]")
2341:                                     elif len(value) == 2:
2342:                                         interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}]")
2343:                                     else:
2344:                                         interpreter.log_output(f"{key}: {value}")
2345:                                 else:
2346:                                     interpreter.log_output(f"{key}: {value}")
2347:                 else:
2348:                     interpreter.log_output("No result produced")
2349:                 interpreter.log_output("=" * 40)
2350: 
2351: 
2352: if __name__ == '__main__':
2353:     main()

(End of file - total 2353 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 31.2s)

_Thinking:_

Now I understand the file structure. The `main()` function starts at line 2170. The main function does:
1. Parse command line arguments/config file
2. Create HLSLInterpreter instance
3. Call interpreter.interpret()
4. Call interpreter.executeVS()
5. Create Rasterizer and rasterize results
6. Display input/output meshes if enabled
7. Print results
8. Compare with golden data
9. Interactive loop for user input

I need to:
1. Create `render.py` with the main function code
2. Keep only the HLSL interpreter implementation in `hlsl_interpreter.py`

Let me create the render.py file first with the main function code.


**Tool: write**

**Input:**
```json
{
  "content": "import os\nimport sys\nimport time\nimport json\n\nfrom hlsl_interpreter import HLSLInterpreter\nfrom rasterizer import Rasterizer\nfrom d3d import D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST\n\n\ndef main():\n    if len(sys.argv) < 2:\n        print(\"Usage: python render.py <config.json>\")\n        print(\"Config JSON should contain: hlsl_file_path, csv_folder_path, log_file_path\")\n        config_path = './wrong_constant_attenuation.json'\n    else:\n        config_path = sys.argv[1]\n\n    if not os.path.exists(config_path):\n        print(f\"Error: Config file not found: {config_path}\")\n        sys.exit(1)\n\n    config = {}\n    with open(config_path, 'r', encoding='utf-8') as f:\n        config = json.load(f)\n\n    hlsl_file_path = config.get('hlsl_file_path', '')\n    csv_folder_path = config.get('csv_folder_path', '')\n    log_file_path = config.get('log_file_path', 'hlsl_interpreter.log')\n    log_file_mode = config.get('log_file_mode', 'a')\n    print_sequence = config.get('print_sequence', 1)\n    log_to_file = config.get('log_to_file', True)\n    printSyntaxTree = config.get('printSyntaxTree', True)\n    print_interpreter_result = config.get('print_interpreter_result', True)\n    float_tolerance = config.get('float_tolerance', 0.0001)\n    output_struct_name = config.get('output_struct_name', 'VS_OUTPUT')\n    execute_count = config.get('execute_count', None)\n    max_workers = config.get('max_workers', 1)\n    primitive_topology = config.get('primitive_topology', D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)\n    mesh_view_enabled = config.get('mesh_view_enabled', False)\n\n    if not hlsl_file_path:\n        print(\"Error: hlsl_file_path not specified in config\")\n        sys.exit(1)\n\n    if not os.path.exists(hlsl_file_path):\n        print(f\"Error: HLSL file not found: {hlsl_file_path}\")\n        sys.exit(1)\n\n    if csv_folder_path and not os.path.exists(csv_folder_path):\n        print(f\"Error: CSV folder not found: {csv_folder_path}\")\n        sys.exit(1)\n\n    interpreter = HLSLInterpreter(\n        log_to_file=log_to_file,\n        log_file_path=log_file_path,\n        log_file_mode=log_file_mode,\n        print_sequence=print_sequence,\n        printSyntaxTree=printSyntaxTree,\n        print_interpreter_result=print_interpreter_result,\n        max_workers=max_workers,\n        primitive_topology=primitive_topology)\n\n    if mesh_view_enabled:\n        interpreter.enable_mesh_view(True)\n\n    total_start = time.time()\n\n    interpret_start = time.time()\n    interpreter.interpret(hlsl_file_path, csv_folder_path)\n    interpret_time = time.time() - interpret_start\n\n    if mesh_view_enabled and interpreter._mesh_view:\n        interpreter._mesh_view.set_hlsl_interpreter(interpreter, \"main\", \"VS_INPUT\")\n\n    golden_csv_path = os.path.join(csv_folder_path, 'VS_OUTPUT.csv') if csv_folder_path else None\n    load_golden_start = time.time()\n    if golden_csv_path and os.path.exists(golden_csv_path):\n        interpreter.load_vs_output_golden_from_csv(golden_csv_path)\n    load_golden_time = time.time() - load_golden_start\n\n    execute_start = time.time()\n    results = interpreter.executeVS(\"main\", \"VS_INPUT\", execute_count=execute_count)\n    execute_time = time.time() - execute_start\n\n    r = Rasterizer(\"rasterizer_param.json\")\n    pixels = r.rasterize(results, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)\n\n    if mesh_view_enabled:\n        interpreter.log_output(\"Displaying input mesh before executeVS...\")\n        interpreter.show_input_mesh(\"VS_INPUT\")\n\n    if mesh_view_enabled and results:\n        interpreter.log_output(\"Displaying result mesh after executeVS...\")\n        interpreter.show_result_mesh(results)\n\n    if interpreter.print_interpreter_result:\n        interpreter.log_output(\"HLSL Interpreter Result:\")\n        interpreter.log_output(\"=\" * 40)\n        if results:\n            for idx, result in enumerate(results):\n                interpreter.log_output(f\"\\n--- Row {idx} ---\")\n                if result:\n                    for key, value in result.items():\n                        if isinstance(value, list):\n                            if len(value) == 4:\n                                interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]\")\n                            elif len(value) == 3:\n                                interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]\")\n                            elif len(value) == 2:\n                                interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}]\")\n                            else:\n                                interpreter.log_output(f\"{key}: {value}\")\n                        else:\n                            interpreter.log_output(f\"{key}: {value}\")\n        else:\n            interpreter.log_output(\"No result produced\")\n\n        if results and results[-1] and 'Color' in results[-1]:\n            color = results[-1]['Color']\n            if color and isinstance(color, list) and len(color) == 4:\n                interpreter.log_output(\"\\nFinal Output Color (RGBA):\")\n                interpreter.log_output(f\"  R: {color[0]:.4f}\")\n                interpreter.log_output(f\"  G: {color[1]:.4f}\")\n                interpreter.log_output(f\"  B: {color[2]:.4f}\")\n                interpreter.log_output(f\"  A: {color[3]:.4f}\")\n            else:\n                interpreter.log_output(f\"\\nColor result: {color}\")\n\n        interpreter.log_output(\"\\n\" + \"=\" * 40)\n    interpreter.log_output(\"Comparing with golden data...\")\n    interpreter.log_output(\"=\" * 40)\n    compare_start = time.time()\n    interpreter.compare_vs_output_with_golden(results, output_struct_name=output_struct_name, float_tolerance=float_tolerance, execute_count=execute_count)\n    compare_time = time.time() - compare_start\n\n    total_time = time.time() - total_start\n\n    interpreter.log_output(\"\\n\" + \"=\" * 40)\n    interpreter.log_output(\"Timing Summary:\")\n    interpreter.log_output(\"=\" * 40)\n    interpreter.log_output(f\"interpreter.interpret():             {interpret_time:.4f}s\")\n    interpreter.log_output(f\"interpreter.load_vs_output_golden_from_csv(): {load_golden_time:.4f}s\")\n    interpreter.log_output(f\"interpreter.executeVS():           {execute_time:.4f}s\")\n    interpreter.log_output(f\"compare_vs_output_with_golden():    {compare_time:.4f}s\")\n    interpreter.log_output(f\"Total execution time:               {total_time:.4f}s\")\n\n    while True:\n        user_input = input(\"\\nEnter 'x' to exit, 'o' to open MeshView, 'r' to rerun executeVS: \")\n        user_input = user_input.strip().lower()\n        if user_input == 'x':\n            interpreter._mesh_view.close()\n            break\n        elif user_input == 'o':\n            if interpreter._mesh_view:\n                interpreter._mesh_view.show(blocking=False)\n                interpreter.log_output(\"MeshView reopened\")\n        elif user_input == 'r':\n            results = []\n            execute_start = time.time()\n            results = interpreter.executeVS(\"main\", \"VS_INPUT\", execute_count=execute_count)\n            execute_time = time.time() - execute_start\n            interpreter.log_output(f\"Re-executed executeVS in {execute_time:.4f}s\")\n            if mesh_view_enabled and results:\n                interpreter.log_output(\"Displaying result mesh after re-execution...\")\n                interpreter.show_result_mesh(results)\n            if interpreter.print_interpreter_result:\n                interpreter.log_output(\"HLSL Interpreter Result (re-run):\")\n                interpreter.log_output(\"=\" * 40)\n                if results:\n                    for idx, result in enumerate(results):\n                        interpreter.log_output(f\"\\n--- Row {idx} ---\")\n                        if result:\n                            for key, value in result.items():\n                                if isinstance(value, list):\n                                    if len(value) == 4:\n                                        interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]\")\n                                    elif len(value) == 3:\n                                        interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]\")\n                                    elif len(value) == 2:\n                                        interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}]\")\n                                    else:\n                                        interpreter.log_output(f\"{key}: {value}\")\n                                else:\n                                    interpreter.log_output(f\"{key}: {value}\")\n                else:\n                    interpreter.log_output(\"No result produced\")\n                interpreter.log_output(\"=\" * 40)\n\n\nif __name__ == '__main__':\n    main()",
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\render.py"
}
```

**Output:**
```
Wrote file successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 33.7s)

_Thinking:_

Now I need to remove the main function from hlsl_interpreter.py. The main function spans from line 2170 to the end of the file (line 2353). I should remove the entire main function from hlsl_interpreter.py.

But wait, I need to keep the `if __name__ == '__main__':` guard and the main() call, and just remove the main function itself - or rather, remove the main function body and the `if __name__ == '__main__':` block entirely since the main function should be in render.py now.

Let me re-read the end of hlsl_interpreter.py to make sure what to remove.


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "newString": "",
  "oldString": "def main():\n    import sys\n\n    if len(sys.argv) < 2:\n        print(\"Usage: python hlsl_interpreter.py <config.json>\")\n        print(\"Config JSON should contain: hlsl_file_path, csv_folder_path, log_file_path\")\n        config_path = './wrong_constant_attenuation.json'\n    else:\n        config_path = sys.argv[1]\n\n    if not os.path.exists(config_path):\n        print(f\"Error: Config file not found: {config_path}\")\n        sys.exit(1)\n\n    config = {}\n    with open(config_path, 'r', encoding='utf-8') as f:\n        config = json.load(f)\n\n    hlsl_file_path = config.get('hlsl_file_path', '')\n    csv_folder_path = config.get('csv_folder_path', '')\n    log_file_path = config.get('log_file_path', 'hlsl_interpreter.log')\n    log_file_mode = config.get('log_file_mode', 'a')\n    print_sequence = config.get('print_sequence', 1)\n    log_to_file = config.get('log_to_file', True)\n    printSyntaxTree = config.get('printSyntaxTree', True)\n    print_interpreter_result = config.get('print_interpreter_result', True)\n    float_tolerance = config.get('float_tolerance', 0.0001)\n    output_struct_name = config.get('output_struct_name', 'VS_OUTPUT')\n    execute_count = config.get('execute_count', None)\n    max_workers = config.get('max_workers', 1)\n    primitive_topology = config.get('primitive_topology', D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)\n    mesh_view_enabled = config.get('mesh_view_enabled', False)\n\n    if not hlsl_file_path:\n        print(\"Error: hlsl_file_path not specified in config\")\n        sys.exit(1)\n\n    if not os.path.exists(hlsl_file_path):\n        print(f\"Error: HLSL file not found: {hlsl_file_path}\")\n        sys.exit(1)\n\n    if csv_folder_path and not os.path.exists(csv_folder_path):\n        print(f\"Error: CSV folder not found: {csv_folder_path}\")\n        sys.exit(1)\n\n    interpreter = HLSLInterpreter(\n        log_to_file=log_to_file,\n        log_file_path=log_file_path,\n        log_file_mode=log_file_mode,\n        print_sequence=print_sequence,\n        printSyntaxTree=printSyntaxTree,\n        print_interpreter_result=print_interpreter_result,\n        max_workers=max_workers,\n        primitive_topology=primitive_topology)\n\n    if mesh_view_enabled:\n        interpreter.enable_mesh_view(True)\n\n    total_start = time.time()\n\n    interpret_start = time.time()\n    interpreter.interpret(hlsl_file_path, csv_folder_path)\n    interpret_time = time.time() - interpret_start\n\n    if mesh_view_enabled and interpreter._mesh_view:\n        interpreter._mesh_view.set_hlsl_interpreter(interpreter, \"main\", \"VS_INPUT\")\n\n    golden_csv_path = os.path.join(csv_folder_path, 'VS_OUTPUT.csv') if csv_folder_path else None\n    load_golden_start = time.time()\n    if golden_csv_path and os.path.exists(golden_csv_path):\n        interpreter.load_vs_output_golden_from_csv(golden_csv_path)\n    load_golden_time = time.time() - load_golden_start\n\n    execute_start = time.time()\n    results = interpreter.executeVS(\"main\", \"VS_INPUT\", execute_count=execute_count)\n    execute_time = time.time() - execute_start\n\n    r = Rasterizer(\"rasterizer_param.json\")  # 或 Rasterizer() 使用默认配置\n    pixels = r.rasterize(results, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)\n\n    if mesh_view_enabled:\n        interpreter.log_output(\"Displaying input mesh before executeVS...\")\n        interpreter.show_input_mesh(\"VS_INPUT\")\n\n    if mesh_view_enabled and results:\n        interpreter.log_output(\"Displaying result mesh after executeVS...\")\n        interpreter.show_result_mesh(results)\n\n    if interpreter.print_interpreter_result:\n        interpreter.log_output(\"HLSL Interpreter Result:\")\n        interpreter.log_output(\"=\" * 40)\n        if results:\n            for idx, result in enumerate(results):\n                interpreter.log_output(f\"\\n--- Row {idx} ---\")\n                if result:\n                    for key, value in result.items():\n                        if isinstance(value, list):\n                            if len(value) == 4:\n                                interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]\")\n                            elif len(value) == 3:\n                                interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]\")\n                            elif len(value) == 2:\n                                interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}]\")\n                            else:\n                                interpreter.log_output(f\"{key}: {value}\")\n                        else:\n                            interpreter.log_output(f\"{key}: {value}\")\n        else:\n            interpreter.log_output(\"No result produced\")\n\n        if results and results[-1] and 'Color' in results[-1]:\n            color = results[-1]['Color']\n            if color and isinstance(color, list) and len(color) == 4:\n                interpreter.log_output(\"\\nFinal Output Color (RGBA):\")\n                interpreter.log_output(f\"  R: {color[0]:.4f}\")\n                interpreter.log_output(f\"  G: {color[1]:.4f}\")\n                interpreter.log_output(f\"  B: {color[2]:.4f}\")\n                interpreter.log_output(f\"  A: {color[3]:.4f}\")\n            else:\n                interpreter.log_output(f\"\\nColor result: {color}\")\n\n        interpreter.log_output(\"\\n\" + \"=\" * 40)\n    interpreter.log_output(\"Comparing with golden data...\")\n    interpreter.log_output(\"=\" * 40)\n    compare_start = time.time()\n    interpreter.compare_vs_output_with_golden(results, output_struct_name=output_struct_name, float_tolerance=float_tolerance, execute_count=execute_count)\n    compare_time = time.time() - compare_start\n\n    total_time = time.time() - total_start\n\n    interpreter.log_output(\"\\n\" + \"=\" * 40)\n    interpreter.log_output(\"Timing Summary:\")\n    interpreter.log_output(\"=\" * 40)\n    interpreter.log_output(f\"interpreter.interpret():             {interpret_time:.4f}s\")\n    interpreter.log_output(f\"interpreter.load_vs_output_golden_from_csv(): {load_golden_time:.4f}s\")\n    interpreter.log_output(f\"interpreter.executeVS():           {execute_time:.4f}s\")\n    interpreter.log_output(f\"compare_vs_output_with_golden():    {compare_time:.4f}s\")\n    interpreter.log_output(f\"Total execution time:               {total_time:.4f}s\")\n\n    while True:\n        user_input = input(\"\\nEnter 'x' to exit, 'o' to open MeshView, 'r' to rerun executeVS: \")\n        user_input = user_input.strip().lower()\n        if user_input == 'x':\n            interpreter._mesh_view.close()\n            break\n        elif user_input == 'o':\n            if interpreter._mesh_view:\n                interpreter._mesh_view.show(blocking=False)\n                interpreter.log_output(\"MeshView reopened\")\n        elif user_input == 'r':\n            results = []\n            execute_start = time.time()\n            results = interpreter.executeVS(\"main\", \"VS_INPUT\", execute_count=execute_count)\n            execute_time = time.time() - execute_start\n            interpreter.log_output(f\"Re-executed executeVS in {execute_time:.4f}s\")\n            if mesh_view_enabled and results:\n                interpreter.log_output(\"Displaying result mesh after re-execution...\")\n                interpreter.show_result_mesh(results)\n            if interpreter.print_interpreter_result:\n                interpreter.log_output(\"HLSL Interpreter Result (re-run):\")\n                interpreter.log_output(\"=\" * 40)\n                if results:\n                    for idx, result in enumerate(results):\n                        interpreter.log_output(f\"\\n--- Row {idx} ---\")\n                        if result:\n                            for key, value in result.items():\n                                if isinstance(value, list):\n                                    if len(value) == 4:\n                                        interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]\")\n                                    elif len(value) == 3:\n                                        interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]\")\n                                    elif len(value) == 2:\n                                        interpreter.log_output(f\"{key}: [{value[0]:.4f}, {value[1]:.4f}]\")\n                                    else:\n                                        interpreter.log_output(f\"{key}: {value}\")\n                                else:\n                                    interpreter.log_output(f\"{key}: {value}\")\n                else:\n                    interpreter.log_output(\"No result produced\")\n                interpreter.log_output(\"=\" * 40)\n\n\nif __name__ == '__main__':\n    main()"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 11.3s)

_Thinking:_

I also need to remove the import statements that are no longer needed in hlsl_interpreter.py after removing the main function. Let me check which imports were only used by main():
- os, sys, time, json - used only in main()
- Rasterizer - used only in main()
- D3D constants - used only in main()

But wait, some of these might be used elsewhere in the HLSLInterpreter class. Let me check:
- `os` is used in `interpret()` method for os.path.exists, os.path.dirname, os.path.join
- `time` is only used in main() for timing
- `json` is only used in main() for loading config
- `Rasterizer` is only used in main()
- D3D constants are used in HLSLInterpreter `__init__` for default primitive_topology

So I need to keep the D3D imports but can remove time, json, Rasterizer, and os is used elsewhere so need to keep it. Actually `sys` is only used in main().

Let me also check if `sys` is used anywhere else in the file. Looking at the code, `sys` was only used in main() for `sys.exit(1)`. So I should remove it from imports.

Let me update the imports in hlsl_interpreter.py.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "limit": 20
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\hlsl_interpreter.py</path>
<type>file</type>
<content>
1: import csv
2: import json
3: import math
4: import re
5: import os
6: import time
7: from concurrent.futures import ThreadPoolExecutor
8: from dataclasses import dataclass, field
9: from typing import Any, Dict, List, Union, Optional
10: 
11: from hlsl_syntax_tree import SyntaxTreeNode, SyntaxTreeParser, _COMPILED_PATTERNS
12: from rasterizer import Rasterizer
13: 
14: 
15: try:
16:     from mesh_view import MeshView, VertexData
17:     MESHVIEW_AVAILABLE = True
18: except ImportError:
19:     MESHVIEW_AVAILABLE = False
20: 

(Showing lines 1-20 of 2169. Use offset=21 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 10.2s)

_Thinking:_

hlsl_interpreter.py still imports `time` and `json` and `Rasterizer` but they're not needed for the interpreter itself. The render.py doesn't use Rasterizer directly - it imports it and uses it. Let me check what render.py imports to make sure the refactoring is correct.

render.py imports:
- os, sys, time, json
- from hlsl_interpreter import HLSLInterpreter
- from rasterizer import Rasterizer
- from d3d import D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST

So render.py handles rasterization. The hlsl_interpreter.py still has unused imports: `json`, `time`, and `Rasterizer`. I should remove them since the main function was removed.


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "newString": "import csv\nimport math\nimport re\nimport os\nfrom concurrent.futures import ThreadPoolExecutor\nfrom dataclasses import dataclass, field\nfrom typing import Any, Dict, List, Union, Optional\n\nfrom hlsl_syntax_tree import SyntaxTreeNode, SyntaxTreeParser, _COMPILED_PATTERNS",
  "oldString": "import csv\nimport json\nimport math\nimport re\nimport os\nimport time\nfrom concurrent.futures import ThreadPoolExecutor\nfrom dataclasses import dataclass, field\nfrom typing import Any, Dict, List, Union, Optional\n\nfrom hlsl_syntax_tree import SyntaxTreeNode, SyntaxTreeParser, _COMPILED_PATTERNS\nfrom rasterizer import Rasterizer"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 9.9s)

_Thinking:_

Now I need to remove the import statement for Rasterizer in render.py, since it will import HLSLInterpreter which no longer has Rasterizer. Actually, the render.py uses Rasterizer directly, so it should keep that import.

Wait, I removed `from rasterizer import Rasterizer` from hlsl_interpreter.py but render.py still needs it. That's correct because render.py directly imports Rasterizer.

Let me also remove the unused imports from hlsl_interpreter.py. Let me check if `json` and `time` are still needed in hlsl_interpreter.py.

Actually, looking at the original code, `json` was used in the `main()` function which I removed, and `time` was also used only in `main()`. So both can be removed from hlsl_interpreter.py.

Also, `Rasterizer` was imported but the `main()` function was the only place that used it. So removing both `json`, `time` and `Rasterizer` import is correct.

Let me verify the file is correct now.




完成：

1. **创建 `render.py`** - 包含原 `hlsl_interpreter.py` 的 `main()` 函数代码
2. **修改 `hlsl_interpreter.py`** - 移除了 `main()` 函数及相关代码（`json`、`time` 导入和 `Rasterizer` 导入也一并移除）

---

