# 将HLSLInterpreter的log配置改为JSON读取

**Session ID:** ses_1cee98c9fffef0qL6vNmQTAK36
**Created:** 2026/5/16 22:00:15
**Updated:** 2026/5/16 22:01:47

---

## User

hlsl_interpreter.py的HLSLInterpreter创建时使用参数决定log_file_mode和print_sequence，请把输入参数改成与hlsl源码文件，csv数据文件路径一样，加入到json文件中，从json文件中读取log_file_mode和print_sequence参数
    interpreter = HLSLInterpreter(log_to_file=True, log_file_path=log_file_path, log_file_mode='w', print_sequence=100) 

---

## Assistant (Build · MiniMax-M2.7 · 8.8s)

_Thinking:_

The user wants me to modify the `hlsl_interpreter.py` file so that `HLSLInterpreter` reads `log_file_mode` and `print_sequence` parameters from a JSON configuration file, similar to how `hlsl_source_file` and `csv_data_file_path` are already read.

Let me first find and read the `hlsl_interpreter.py` file to understand the current structure.

**Tool: glob**

**Input:**
```json
{
  "pattern": "**/hlsl_interpreter.py"
}
```

**Output:**
```
C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\specular_too_shining\hlsl_interpreter.py
C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\constant_buffer_attenuation_wrong\hlsl_interpreter.py
C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\hlsl_interpreter.py
```

---

## Assistant (Build · MiniMax-M2.7 · 8.6s)

_Thinking:_

There are multiple hlsl_interpreter.py files. Let me read the main one to understand its structure.


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
7: from dataclasses import dataclass, field
8: from typing import Any, Dict, List, Union, Optional
9: 
10: 
11: DATA_TYPE_LIST = [
12:     'float4x4', 'float3x3',  # 矩阵类型
13:     'float4', 'float3', 'float2', 'float',  # 浮点向量/标量
14:     'uint4', 'uint3', 'uint2', 'uint',  # 无符号整数
15:     'int4', 'int3', 'int2', 'int',  # 有符号整数
16:     'bool'  # 布尔类型
17: ]
18: 
19: 
20: class SyntaxTreeNode:
21:     """
22:     HLSL语法树节点基类
23:     用于表示HLSL表达式解析后的语法树结构
24:     node_type: 节点类型 - 'value'(值), 'function'(函数), 'binary_op'(二元操作),
25:                             'unary_op'(一元操作), 'cast'(类型转换), 'ternary'(三元条件)
26:     value: 节点值 - 变量名/函数名/操作符/类型名
27:     left: 左子节点 (用于二元/一元操作或三元条件)
28:     right: 右子节点 (用于二元操作或三元真的表达式)
29:     third_child: 第三子节点 (用于三元条件假的表达式)
30:     args: 函数参数列表 (用于函数调用)
31:     """
32:     def __init__(self, node_type: str, value: Any = None, left: Optional['SyntaxTreeNode'] = None, right: Optional['SyntaxTreeNode'] = None, third_child: Optional['SyntaxTreeNode'] = None, args: Optional[List['SyntaxTreeNode']] = None, line_number: int = 0):
33:         self.node_type = node_type
34:         self.value = value
35:         self.left = left
36:         self.right = right
37:         self.third_child = third_child
38:         self.args = args if args is not None else []
39:         self.line_number = line_number
40: 
41:     def __repr__(self):
42:         return self._pretty(0)
43: 
44:     def _pretty(self, indent: int) -> str:
45:         """
46:         格式化输出语法树，用于调试
47:         indent: 缩进层级
48:         """
49:         prefix = "  " * indent
50:         if self.node_type == 'function':
51:             lines = [f"Function({self.value})"]
52:             for i, arg in enumerate(self.args):
53:                 lines.append(f"{prefix}  arg[{i}]:")
54:                 lines.append(arg._pretty(indent + 2))
55:             return "\n".join(lines)
56:         elif self.node_type == 'binary_op':
57:             lines = [f"BinaryOp({self.value})"]
58:             lines.append(f"{prefix}  left:")
59:             lines.append(self.left._pretty(indent + 2))
60:             lines.append(f"{prefix}  right:")
61:             lines.append(self.right._pretty(indent + 2))
62:             return "\n".join(lines)
63:         elif self.node_type == 'unary_op':
64:             lines = [f"UnaryOp({self.value})"]
65:             lines.append(f"{prefix}  child:")
66:             lines.append(self.left._pretty(indent + 2))
67:             return "\n".join(lines)
68:         elif self.node_type == 'cast':
69:             lines = [f"Cast({self.value})"]
70:             lines.append(f"{prefix}  inner:")
71:             lines.append(self.left._pretty(indent + 2))
72:             return "\n".join(lines)
73:         elif self.node_type == 'ternary':
74:             lines = [f"Ternary({self.value})"]
75:             lines.append(f"{prefix}  condition:")
76:             lines.append(self.left._pretty(indent + 2))
77:             lines.append(f"{prefix}  true_expr:")
78:             lines.append(self.right._pretty(indent + 2))
79:             lines.append(f"{prefix}  false_expr:")
80:             lines.append(self.third_child._pretty(indent + 2))
81:             return "\n".join(lines)
82:         else:
83:             return f"{prefix}Value({self.value})"
84: 
85: 
86: class SyntaxTreeParser:
87:     """
88:     HLSL表达式语法树解析器
89:     负责将HLSL表达式字符串解析为SyntaxTreeNode组成的语法树
90:     支持: 类型转换、括号表达式、二元运算符、函数调用、变量引用
91:     """
92:     def __init__(self):
93:         self.operators = {
94:             '||': 1, '&&': 2,
95:             '==': 3, '!=': 3,
96:             '<': 4, '>': 4, '<=': 4, '>=': 4,
97:             '+': 5, '-': 5,
98:             '*': 6, '/': 6,
99:         }
100: 
101:     def parse(self, expr: str) -> SyntaxTreeNode:
102:         """
103:         解析HLSL表达式为语法树
104:         expr: HLSL表达式字符串
105:         返回: SyntaxTreeNode语法树根节点
106:         """
107:         expr = expr.strip()
108:         return self._parse_expression(expr)
109: 
110:     def _find_top_level_operator(self, expr: str) -> Optional[tuple]:
111:         """
112:         查找表达式中优先级最低的运算符(处于括号外的顶层运算符)
113:         用于实现运算符优先级解析
114:         expr: 表达式字符串
115:         返回: (位置, 运算符) 元组，或None
116: 
117:         运算符优先级(数字越小优先级越低):
118:         '||': 1, '&&': 2, '==': 3, '!=': 3,
119:         '<': 4, '>': 4, '<=': 4, '>=': 4,
120:         '+': 5, '-': 5, '*': 6, '/': 6
121: 
122:         规则: 找到优先级最低的运算符，如果有多个同优先级的运算符，返回最右边的那个
123:         """
124:         depth = 0
125:         candidates = []
126:         i = 0
127:         while i < len(expr):
128:             char = expr[i]
129:             if char == '(':
130:                 depth += 1
131:             elif char == ')':
132:                 depth -= 1
133:             elif depth == 0:
134:                 if i >= 1:
135:                     two_char = expr[i-1:i+1]
136:                     if two_char in self.operators:
137:                         candidates.append((i-1, two_char, self.operators[two_char]))
138:                         i += 1
139:                         continue
140:                 if char in self.operators:
141:                     candidates.append((i, char, self.operators[char]))
142:             i += 1
143: 
144:         if not candidates:
145:             return None
146: 
147:         min_prec = min(c[2] for c in candidates)
148:         rightmost = max(c[0] for c in candidates if c[2] == min_prec)
149:         for c in candidates:
150:             if c[0] == rightmost and c[2] == min_prec:
151:                 return (c[0], c[1])
152:         return None
153: 
154:     def _parse_expression(self, expr: str) -> SyntaxTreeNode:
155:         """
156:         将HLSL表达式字符串解析为语法树节点。
157: 
158:         解析顺序(从高优先级到低优先级):
159:         1. 类型转换: (float3x3)expr - 将表达式转换为指定类型
160:         2. 括号表达式: (expr) - 括号包围的表达式
161:         3. 三元运算符: a ? b : c - 条件表达式
162:         4. 二元运算符: + - * / == != < > <= >= && ||
163:         5. 函数调用: func(args) - 如normalize(), mul(), transpose()等
164:         6. 变量/常量值: 标识符或数字字面量
165:         """
166:         expr = expr.strip()
167:         if not expr:
168:             return SyntaxTreeNode('value', None)
169: 
170:         # =====================================================================
171:         # 第一步: 类型转换 (cast) - 匹配模式 (type)expression
172:         # 例如: (float3x3)World - 将4x4矩阵转换为3x3矩阵
173:         #       (float4)vec3 - 将vec3扩展为vec4
174:         # =====================================================================
175:         cast_match = re.match(r'\((\w+)\)\s*(.+)', expr, re.DOTALL)
176:         if cast_match:
177:             cast_type = cast_match.group(1)    # 转换目标类型，如float3x3
178:             rest = cast_match.group(2).strip()   # 类型声明之后的部分
179:             inner_node = self._parse_expression(rest)  # 递归解析内部表达式
180:             return SyntaxTreeNode('cast', cast_type, inner_node)
181: 
182:         # =====================================================================
183:         # 第二步: 括号表达式 - 检查是否被括号包围
184:         # 例如: (a + b) - 外层括号只是分组，不改变语义
185:         # 注意: 需要检查括号是否平衡，防止误匹配如 (a) + (b)
186:         # =====================================================================
187:         if expr.startswith('(') and expr.endswith(')'):
188:             inner = expr[1:-1].strip()
189:             # 遍历内部内容，检查括号是否平衡
190:             paren_depth = 0
191:             is_proper_paren = True
192:             for j, c in enumerate(inner):
193:                 if c == '(':
194:                     paren_depth += 1
195:                 elif c == ')':
196:                     paren_depth -= 1
197:                 # 如果在遍历过程中深度变为负数，说明括号不平衡
198:                 if paren_depth < 0:
199:                     is_proper_paren = False
200:                     break
201:             # 只有当内部括号都平衡时，才将外层括号视为分组
202:             if is_proper_paren:
203:                 return self._parse_expression(inner)
204: 
205:         # =====================================================================
206:         # 第三步: 三元运算符 - 匹配 a ? b : c 模式
207:         # 三元运算符优先级最低，在所有二元运算符之后处理
208:         # =====================================================================
209:         ternary_pos = -1
210:         depth = 0
211:         for i, char in enumerate(expr):
212:             if char == '(':
213:                 depth += 1
214:             elif char == ')':
215:                 depth -= 1
216:             elif char == '?' and depth == 0:
217:                 ternary_pos = i
218:                 break
219: 
220:         if ternary_pos >= 0:
221:             colon_pos = -1
222:             depth = 0
223:             for i in range(ternary_pos + 1, len(expr)):
224:                 char = expr[i]
225:                 if char == '(':
226:                     depth += 1
227:                 elif char == ')':
228:                     depth -= 1
229:                 elif char == ':' and depth == 0:
230:                     colon_pos = i
231:                     break
232: 
233:             if colon_pos >= 0:
234:                 cond_expr = expr[:ternary_pos].strip()
235:                 true_expr = expr[ternary_pos+1:colon_pos].strip()
236:                 false_expr = expr[colon_pos+1:].strip()
237:                 cond_node = self._parse_expression(cond_expr)
238:                 true_node = self._parse_expression(true_expr)
239:                 false_node = self._parse_expression(false_expr)
240:                 return SyntaxTreeNode('ternary', '?', cond_node, true_node, false_node)
241: 
242:         # =====================================================================
243:         # 第四步: 二元运算符 - 从右向左查找优先级最低的运算符
244:         # 支持: 逻辑或(||)、逻辑与(&&)、比较(== != < > <= >=)、
245:         #       算术(+ -)、乘除(* /)
246:         # =====================================================================
247:         op_info = self._find_top_level_operator(expr)
248:         if op_info:
249:             pos, op = op_info
250:             if op in ['||', '&&', '==', '!=', '<', '>', '<=', '>=', '+', '-', '*', '/']:
251:                 left_expr = expr[:pos].strip()
252:                 right_expr = expr[pos+len(op):].strip()
253:                 # 递归解析左右操作数
254:                 left_node = self._parse_expression(left_expr)
255:                 right_node = self._parse_expression(right_expr)
256:                 return SyntaxTreeNode('binary_op', op, left_node, right_node)
257: 
258:         # =====================================================================
259:         # 第五步: 函数调用 - 匹配函数名后跟括号
260:         # float[234]构造函数: float2(...), float3(...), float4(...)
261:         # 普通函数调用: normalize(...), mul(...), transpose(...)等
262:         # =====================================================================
263:         if re.match(r'float[234]\s*\(', expr):
264:             return self._parse_function_call(expr)
265: 
266:         if re.match(r'\w+\s*\(', expr):
267:             return self._parse_function_call(expr)
268: 
269:         # =====================================================================
270:         # 第六步: 变量/常量值 - 标识符、字符串或数字
271:         # 到达这里说明表达式不包含运算符和函数调用
272:         # =====================================================================
273:         return SyntaxTreeNode('value', expr)
274: 
275:     def _parse_function_call(self, expr: str) -> SyntaxTreeNode:
276:         """
277:         解析函数调用表达式
278:         处理类型转换和函数调用两种情况
279:         expr: 函数调用表达式，如 "(float3x3)World" 或 "mul(a, b)"
280:         """
281:         expr = expr.strip()
282:         if expr.startswith('('):
283:             match = re.match(r'\((\w+)\)\s*(.+)', expr, re.DOTALL)
284:             if match:
285:                 cast_type = match.group(1)
286:                 rest = match.group(2).strip()
287:                 inner_node = self._parse_expression(rest)
288:                 if inner_node.node_type == 'value':
289:                     return inner_node
290:                 return SyntaxTreeNode('cast', cast_type, inner_node)
291: 
292:         match = re.match(r'^(\w+)\s*\(', expr)
293:         if not match:
294:             return SyntaxTreeNode('value', expr)
295: 
296:         func_name = match.group(1)
297: 
298:         depth = 0
299:         paren_start = -1
300:         for i, char in enumerate(expr):
301:             if char == '(':
302:                 depth += 1
303:                 if depth == 1:
304:                     paren_start = i
305:             elif char == ')':
306:                 depth -= 1
307:                 if depth == 0:
308:                     args_str = expr[paren_start+1:i]
309:                     if func_name in ['transpose', 'normalize', 'length', 'abs', 'sin', 'cos', 'tan']:
310:                         inner_node = self._parse_expression(args_str.strip())
311:                         return SyntaxTreeNode('function', func_name, args=[inner_node])
312:                     elif func_name in ['mul', 'reflect', 'pow', 'max', 'min', 'dot', 'float2', 'float3', 'float4']:
313:                         args = self._split_args(args_str)
314:                         arg_nodes = [self._parse_expression(arg.strip()) for arg in args]
315:                         return SyntaxTreeNode('function', func_name, args=arg_nodes)
316:                     args = self._split_args(args_str)
317:                     arg_nodes = [self._parse_expression(arg.strip()) for arg in args]
318:                     return SyntaxTreeNode('function', func_name, args=arg_nodes)
319: 
320:         return SyntaxTreeNode('value', expr)
321: 
322:     def _split_args(self, args_str: str) -> List[str]:
323:         """
324:         分割函数参数字符串，处理嵌套括号
325:         args_str: 参数字符串，如 "a, b, float3(1,2,3)"
326:         返回: 参数列表
327:         """
328:         if not args_str.strip():
329:             return []
330:         args = []
331:         depth = 0
332:         current = ''
333:         for char in args_str:
334:             if char == '(':
335:                 depth += 1
336:                 current += char
337:             elif char == ')':
338:                 depth -= 1
339:                 current += char
340:             elif char == ',' and depth == 0:
341:                 args.append(current.strip())
342:                 current = ''
343:             else:
344:                 current += char
345:         if current.strip():
346:             args.append(current.strip())
347:         return args
348: 
349: 
350: @dataclass
351: class ShaderVariable:
352:     """着色器变量定义"""
353:     name: str       # 变量名
354:     type: str       # 变量类型
355:     value: Any      # 变量值
356: 
357: 
358: @dataclass
359: class FieldDefinition:
360:     """结构体或cbuffer的字段定义"""
361:     field_type: str      # 字段类型，如 float3, float4x4
362:     name: str           # 字段名
363:     semantic: str       # 语义名称，如 POSITION, NORMAL
364:     data: List[Any] = None  # 字段数据值
365: 
366: @dataclass
367: class StructDefinition:
368:     """HLSL结构体定义"""
369:     name: str                     # 结构体名称
370:     fields: List[FieldDefinition]  # 结构体字段列表
371: 
372: @dataclass
373: class CbufferDefinition:
374:     """HLSL常量缓冲区定义"""
375:     name: str                     # cbuffer名称
376:     fields: List[FieldDefinition]  # cbuffer字段列表
377: 
378: 
379: class HLSLInterpreter:
380:     """
381:     HLSL解释器 - 解析和执行HLSL着色器代码
382:     支持: 结构体定义、cbuffer定义、函数解析、表达式求值
383:     """
384: 
385:     def __init__(self, log_to_file: bool = True, log_file_path: str = "hlsl_interpreter.log", print_sequence: int = 1, log_file_mode: str = 'a'):
386:         self.structs: Dict[str, StructDefinition] = {}      # 解析的结构体定义
387:         self.cbuffers: Dict[str, CbufferDefinition] = {}    # 解析的cbuffer定义
388:         self.variables: Dict[str, Any] = {}                 # 全局变量
389:         self.debug = True                                   # 调试模式开关
390:         self.printSyntaxTree = True                         # 打印语法树开关
391:         self.syntax_parser = SyntaxTreeParser()             # 语法树解析器
392:         self.log_to_file = log_to_file                      # 是否输出到文件
393:         self.log_file_path = log_file_path                  # 日志文件路径
394:         self.log_file_mode = log_file_mode                  # 文件模式: 'a'=追加, 'w'=覆盖
395:         self.print_sequence = max(1, print_sequence)        # 打印间隔频率
396:         self._eval_counter = 0                              # evaluate_syntax_tree执行计数器
397:         self._should_print = True                           # 当前是否应该打印
398:         self._log_file = None                               # 日志文件句柄
399:         self.hlsl_code = None                               # 加载的HLSL代码
400:         if self.log_to_file and self.log_file_path:
401:             self._log_file = open(self.log_file_path, self.log_file_mode, encoding='utf-8')
402: 
403:     def __del__(self):
404:         """对象销毁时关闭日志文件"""
405:         if self._log_file:
406:             self._log_file.close()
407:             self._log_file = None
408: 
409:     def log_output(self, *args, **kwargs):
410:         """输出到stdout和日志文件"""
411:         msg = ' '.join(str(arg) for arg in args)
412:         print(*args, **kwargs)
413:         if self.log_to_file and self._log_file:
414:             self._log_file.write(msg + '\n')
415:             self._log_file.flush()
416: 
417:     def debug_print(self, msg: str):
418:         """调试打印"""
419:         if self.debug and self._should_print:
420:             self.log_output(msg)
421: 
422:     def _format_float(self, val):
423:         """
424:         格式化浮点数输出
425:         val: 值
426:         返回: 格式化后的字符串(保留4位小数)
427:         """
428:         if isinstance(val, float):
429:             return f"{val:.4f}"
430:         if isinstance(val, list):
431:             if val and isinstance(val[0], list):
432:                 return self._format_matrix(val)
433:             return [self._format_float(v) for v in val]
434:         return val
435: 
436:     def _format_matrix(self, val):
437:         """
438:         格式化矩阵输出
439:         val: 矩阵(二维列表)
440:         返回: 格式化后的矩阵字符串
441:         """
442:         if not val or not isinstance(val[0], list):
443:             return str(val)
444:         formatted = [[self._format_float(v) for v in row] for row in val]
445:         col_widths = [0] * len(formatted[0])
446:         for row in formatted:
447:             for j, cell in enumerate(row):
448:                 col_widths[j] = max(col_widths[j], len(cell))
449:         lines = []
450:         for row in formatted:
451:             cells = [cell.rjust(col_widths[j]) for j, cell in enumerate(row)]
452:             lines.append("[" + " ".join(cells) + "]")
453:         return "\n".join(lines)
454: 
455:     def _format_value(self, val):
456:         """格式化值输出(矩阵或标量/向量)"""
457:         if isinstance(val, list) and val and isinstance(val[0], list):
458:             return self._format_matrix(val)
459:         return self._format_float(val)
460: 
461:     def _format_msg(self, *args):
462:         """格式化多个值用于调试输出"""
463:         formatted = []
464:         for arg in args:
465:             formatted.append(self._format_float(arg))
466:         return formatted
467: 
468:     def load_json(self, filepath: str):
469:         """从JSON文件加载数据"""
470:         with open(filepath, 'r') as f:
471:             data = json.load(f)
472:         return data
473: 
474:     def load_csv(self, filepath: str) -> List[List[str]]:
475:         """从CSV文件加载数据，返回二维列表"""
476:         rows = []
477:         with open(filepath, 'r') as f:
478:             reader = csv.reader(f)
479:             for row in reader:
480:                 rows.append(row)
481:         return rows
482: 
483:     def get_type_size(self, field_type: str) -> int:
484:         """
485:         获取HLSL类型的大小(字节数)
486:         field_type: HLSL类型名，如 float4x4, float3, int
487:         返回: 类型占用的字节数
488:         """
489:         if 'float4x4' in field_type:
490:             return 64  # 4x4矩阵 = 16 floats * 4 bytes
491:         elif 'float3x3' in field_type:
492:             return 36  # 3x3矩阵 = 9 floats * 4 bytes
493:         elif 'float4' in field_type:
494:             return 16  # 4 floats * 4 bytes
495:         elif 'float3' in field_type:
496:             return 12  # 3 floats * 4 bytes
497:         elif 'float2' in field_type:
498:             return 8   # 2 floats * 4 bytes
499:         elif 'float' in field_type:
500:             return 4   # 1 float * 4 bytes
501:         elif 'uint4' in field_type:
502:             return 16
503:         elif 'uint3' in field_type:
504:             return 12
505:         elif 'uint2' in field_type:
506:             return 8
507:         elif 'uint' in field_type:
508:             return 4
509:         elif 'int4' in field_type:
510:             return 16
511:         elif 'int3' in field_type:
512:             return 12
513:         elif 'int2' in field_type:
514:             return 8
515:         elif 'int' in field_type:
516:             return 4
517:         elif 'bool' in field_type:
518:             return 4
519:         return 0
520: 
521:     def parse_value_by_type(self, value_str: str, field_type: str) -> Any:
522:         """
523:         根据类型解析字符串值为对应类型的Python对象
524:         value_str: 值的字符串表示
525:         field_type: HLSL类型名
526:         返回: 解析后的值
527:         """
528:         value_str = value_str.strip().strip('"')
529:         if 'float4x4' in field_type:
530:             parts = value_str.split(',')
531:             if len(parts) >= 16:
532:                 matrix = []
533:                 for i in range(4):
534:                     row = [float(parts[j]) for j in range(i*4, i*4+4)]
535:                     matrix.append(row)
536:                 return matrix
537:         elif 'float3x3' in field_type:
538:             parts = value_str.split(',')
539:             if len(parts) >= 9:
540:                 matrix = []
541:                 for i in range(3):
542:                     row = [float(parts[j]) for j in range(i*3, i*3+3)]
543:                     matrix.append(row)
544:                 return matrix
545:         elif 'float4' in field_type:
546:             parts = value_str.split(',')
547:             return [float(p) for p in parts[:4]]
548:         elif 'float3' in field_type:
549:             parts = value_str.split(',')
550:             return [float(p) for p in parts[:3]]
551:         elif 'float2' in field_type:
552:             parts = value_str.split(',')
553:             return [float(p) for p in parts[:2]]
554:         elif 'uint4' in field_type:
555:             parts = value_str.split(',')
556:             return [int(p) for p in parts[:4]]
557:         elif 'uint3' in field_type:
558:             parts = value_str.split(',')
559:             return [int(p) for p in parts[:3]]
560:         elif 'uint2' in field_type:
561:             parts = value_str.split(',')
562:             return [int(p) for p in parts[:2]]
563:         elif 'uint' in field_type:
564:             return int(value_str)
565:         elif 'int4' in field_type:
566:             parts = value_str.split(',')
567:             return [int(p) for p in parts[:4]]
568:         elif 'int3' in field_type:
569:             parts = value_str.split(',')
570:             return [int(p) for p in parts[:3]]
571:         elif 'int2' in field_type:
572:             parts = value_str.split(',')
573:             return [int(p) for p in parts[:2]]
574:         elif 'int' in field_type:
575:             return int(value_str)
576:         elif 'bool' in field_type:
577:             return value_str.lower() in ('true', '1', 'yes')
578:         try:
579:             return float(value_str)
580:         except:
581:             return value_str
582: 
583:     def parse_type(self, type_str: str) -> str:
584:         """
585:         解析HLSL类型字符串为标准类型名
586:         type_str: 类型字符串，如 "float4x4", "float3", "int2"
587:         返回: 标准类型名
588:         """
589:         type_str = type_str.strip()
590:         if type_str in DATA_TYPE_LIST:
591:             return type_str
592:         if type_str.startswith('float'):
593:             if 'x3' in type_str:
594:                 return 'float3x3'
595:             elif 'x4' in type_str:
596:                 return 'float4x4'
597:             elif type_str == 'float':
598:                 return 'float'
599:             return 'float'
600:         elif type_str.startswith('int'):
601:             if type_str == 'int':
602:                 return 'int'
603:             elif '2' in type_str:
604:                 return 'int2'
605:             elif '3' in type_str:
606:                 return 'int3'
607:             elif '4' in type_str:
608:                 return 'int4'
609:             return 'int'
610:         elif type_str.startswith('uint'):
611:             if type_str == 'uint':
612:                 return 'uint'
613:             elif '2' in type_str:
614:                 return 'uint2'
615:             elif '3' in type_str:
616:                 return 'uint3'
617:             elif '4' in type_str:
618:                 return 'uint4'
619:             return 'uint'
620:         elif type_str.startswith('bool'):
621:             return 'bool'
622:         return type_str
623: 
624:     def parse_struct(self, code: str) -> StructDefinition:
625:         """
626:         解析HLSL结构体定义
627:         code: 结构体代码，如 "struct VS_INPUT { float3 Pos : POSITION; }"
628:         返回: StructDefinition对象
629:         """
630:         match = re.search(r'struct\s+(\w+)\s*\{([^}]+)\}', code)
631:         if not match:
632:             return None
633:         name = match.group(1)
634:         fields_str = match.group(2)
635:         fields = []
636:         for line in fields_str.split(';'):
637:             line = line.strip()
638:             if not line:
639:                 continue
640:             parts = line.split(':')
641:             if len(parts) == 2:
642:                 type_and_name = parts[0].strip().split()
643:                 semantic = parts[1].strip()
644:                 if len(type_and_name) >= 2:
645:                     field_type = type_and_name[0]
646:                     field_name = type_and_name[-1]
647:                 else:
648:                     field_type = type_and_name[0]
649:                     field_name = ''
650:                 fields.append(FieldDefinition(field_type, field_name, semantic))
651:         return StructDefinition(name, fields)
652: 
653:     def parse_cbuffer(self, code: str) -> CbufferDefinition:
654:         """
655:         解析HLSL常量缓冲区定义
656:         code: cbuffer代码
657:         返回: CbufferDefinition对象
658:         """
659:         match = re.search(r'cbuffer\s+(\w+)\s*:.*?\{([^}]+)\}', code, re.DOTALL)
660:         if not match:
661:             return None
662:         name = match.group(1)
663:         fields = []
664:         lines = code[match.start():match.end()].split('\n')[1:]
665:         for line in lines:
666:             line = line.strip().rstrip(';')
667:             if not line or line.startswith('}'):
668:                 continue
669:             if any(t in line for t in DATA_TYPE_LIST):
670:                 parts = line.split()
671:                 if len(parts) >= 2:
672:                     field_type = parts[0]
673:                     field_name = parts[1]
674:                     fields.append(FieldDefinition(field_type, field_name, ''))
675:         return CbufferDefinition(name, fields)
676: 
677:     def parse_function(self, code: str) -> tuple:
678:         """
679:         解析HLSL函数定义
680:         code: 函数代码，如 "float4 main(VS_INPUT input) { ... }"
681:         返回: (返回类型, 函数名, 参数字典, 函数体) 元组
682:         """
683:         match = re.search(r'(\w+)\s+(\w+)\s*\(([^)]*)\)\s*\{([^}]+(?:\{[^}]*\}[^}]*)*)\}', code, re.DOTALL)
684:         if not match:
685:             return None, None, None, None
686:         ret_type = match.group(1)
687:         func_name = match.group(2)
688:         params_str = match.group(3)
689:         body = match.group(4)
690:         params = {}
691:         if params_str.strip():
692:             for param in params_str.split(','):
693:                 param = param.strip()
694:                 parts = param.split()
695:                 if len(parts) >= 2:
696:                     param_type = parts[0]
697:                     param_name = parts[1]
698:                     params[param_name] = param_type
699:         return ret_type, func_name, params, body
700: 
701:     def execute_unary_op(self, op: str, val: Any) -> Any:
702:         """
703:         执行一元运算符
704:         op: 运算符 '-' 或 '!'
705:         val: 操作数
706:         """
707:         result = val
708:         if op == '-':
709:             if isinstance(val, (int, float)):
710:                 result = -val
711:             elif isinstance(val, list):
712:                 result = [-v for v in val]
713:         elif op == '!':
714:             if isinstance(val, bool):
715:                 result = not val
716:             result = not bool(val)
717:         self.debug_print(f"[UNARY OP] operand={self._format_value(val)}, op={op}, result={self._format_value(result)}")
718:         return result
719: 
720:     def execute_binary_op(self, op: str, left: Any, right: Any) -> Any:
721:         """
722:         执行二元运算符
723:         op: 运算符 '+', '-', '*', '/', '.'
724:         left, right: 左右操作数
725:         """
726:         if left is None or right is None:
727:             result = None
728:             self.debug_print(f"[BINARY OP] left={self._format_value(left)}, right={self._format_value(right)}, op={op}, result={self._format_value(result)}")
729:             return None
730:         if op == '+':
731:             if isinstance(left, list) and isinstance(right, list):
732:                 result = [l + r for l, r in zip(left, right)]
733:             elif isinstance(left, list) and isinstance(right, (int, float)):
734:                 result = [v + right for v in left]
735:             elif isinstance(right, list) and isinstance(left, (int, float)):
736:                 result = [left + v for v in right]
737:             else:
738:                 result = left + right
739:         elif op == '-':
740:             if isinstance(left, list) and isinstance(right, list):
741:                 result = [l - r for l, r in zip(left, right)]
742:             elif isinstance(left, list) and isinstance(right, (int, float)):
743:                 result = [v - right for v in left]
744:             elif isinstance(right, list) and isinstance(left, (int, float)):
745:                 result = [left - v for v in right]
746:             else:
747:                 result = left - right
748:         elif op == '*':
749:             if isinstance(left, list) and isinstance(right, (int, float)):
750:                 result = [v * right for v in left]
751:             elif isinstance(right, list) and isinstance(left, (int, float)):
752:                 result = [v * left for v in right]
753:             elif isinstance(left, list) and isinstance(right, list):
754:                 result = [l * r for l, r in zip(left, right)]
755:             else:
756:                 result = left * right
757:         elif op == '/':
758:             if isinstance(left, list) and isinstance(right, (int, float)):
759:                 result = [v / right for v in left]
760:             elif isinstance(left, list) and isinstance(right, list):
761:                 result = [l / r for l, r in zip(left, right)]
762:             else:
763:                 result = left / right
764:         elif op == '.':
765:             result = (left, right)
766:         elif op == '==':
767:             result = left == right
768:         elif op == '!=':
769:             result = left != right
770:         elif op == '<':
771:             result = left < right
772:         elif op == '>':
773:             result = left > right
774:         elif op == '<=':
775:             result = left <= right
776:         elif op == '>=':
777:             result = left >= right
778:         elif op == '&&':
779:             result = bool(left and right)
780:         elif op == '||':
781:             result = bool(left or right)
782:         else:
783:             result = None
784:         self.debug_print(f"[BINARY OP] left={self._format_float(left)}, right={self._format_float(right)}, op={op}, result={self._format_float(result)}")
785:         return result
786: 
787:     def transpose_matrix(self, m: List[List[float]]) -> List[List[float]]:
788:         """
789:         矩阵转置
790:         m: 输入矩阵(4x4或3x3)
791:         返回: 转置后的矩阵
792:         """
793:         if len(m) == 4:
794:             return [[m[j][i] for j in range(4)] for i in range(4)]
795:         elif len(m) == 3:
796:             return [[m[j][i] for j in range(3)] for i in range(3)]
797:         return m
798: 
799:     def mul_matrix_vector(self, m: List[List[float]], v: List[float]) -> List[float]:
800:         """
801:         矩阵乘向量: result = m * v
802:         m: 4x4或3x3矩阵
803:         v: 向量(4维或3维)
804:         返回: 计算后的向量
805:         """
806:         if not v or any(x is None for x in v):
807:             return [0, 0, 0, 0]
808:         if not m:
809:             return [0, 0, 0, 0]
810:         num_cols = len(m[0]) if m else 0
811:         result = []
812:         for j in range(num_cols):
813:             s = sum(v[i] * m[i][j] for i in range(len(v)))
814:             result.append(s)
815:         return result
816: 
817:     def mul_matrix_matrix(self, a: List[List[float]], b: List[List[float]]) -> List[List[float]]:
818:         """
819:         矩阵乘法: result = a * b
820:         a, b: n x n 方阵
821:         返回: 结果矩阵
822:         """
823:         n = len(a)
824:         result = [[0.0] * n for _ in range(n)]
825:         for i in range(n):
826:             for j in range(n):
827:                 for k in range(n):
828:                     result[i][j] += a[i][k] * b[k][j]
829:         return result
830: 
831:     def length_vec(self, v: List[float]) -> float:
832:         """计算向量长度(模)"""
833:         return math.sqrt(sum(x * x for x in v))
834: 
835:     def normalize_vec(self, v: List[float]) -> List[float]:
836:         """
837:         向量归一化
838:         v: 输入向量
839:         返回: 归一化后的向量，长度为1
840:         """
841:         l = self.length_vec(v)
842:         if l < 1e-8:
843:             return v
844:         return [x / l for x in v]
845: 
846:     def dot_product(self, a: List[float], b: List[float]) -> float:
847:         """
848:         向量点积: a · b
849:         a, b: 同维度向量
850:         返回: 点积结果
851:         """
852:         if not isinstance(a, list) or not isinstance(b, list):
853:             return 0.0
854:         if len(a) != len(b):
855:             return 0.0
856:         return sum(x * y for x, y in zip(a, b))
857: 
858:     def reflect_vec(self, I: List[float], N: List[float]) -> List[float]:
859:         """
860:         计算反射向量 R = I - 2 * (N · I) * N
861:         I: 入射向量
862:         N: 法线向量(需要归一化)
863:         返回: 反射向量
864:         """
865:         if not isinstance(I, list) or not isinstance(N, list):
866:             return [0, 0, 0]
867:         dot = self.dot_product(N, I)
868:         result = []
869:         for i_val, n_val in zip(I, N):
870:             result.append(i_val - 2 * n_val * dot)
871:         return result
872: 
873:     def find_top_level_comma(self, expr: str) -> int:
874:         """
875:         查找表达式顶层逗号(不在括号内)
876:         用于分割函数多参数
877:         expr: 表达式字符串
878:         返回: 逗号位置索引，或-1表示未找到
879:         """
880:         depth = 0
881:         for i, char in enumerate(expr):
882:             if char == '(':
883:                 depth += 1
884:             elif char == ')':
885:                 depth -= 1
886:             elif char == ',' and depth == 0:
887:                 return i
888:         return -1
889: 
890:     def evaluate_expression(self, expr: str, local_vars: Dict[str, Any]) -> Any:
891:         """
892:         对HLSL表达式求值
893:         expr: 表达式字符串
894:         local_vars: 局部变量字典
895:         返回: 求值结果
896:         """
897:         expr = expr.strip()
898:         if not expr:
899:             return None
900: 
901:         if expr == 'return':
902:             return None
903: 
904:         if expr.startswith('return '):
905:             return self.evaluate_expression(expr[7:], local_vars)
906: 
907:         # 使用语法树解析器处理所有表达式（包括三元运算符）
908:         tree = self.syntax_parser.parse(expr)
909: 
910:         # Print syntax tree
911:         if self.printSyntaxTree == True:
912:             self.debug_print(f"[SYNTAX TREE]\n{tree}")
913: 
914:         result = self.evaluate_syntax_tree(tree, local_vars)
915:         return result
916: 
917:     def evaluate_syntax_tree(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
918:         """
919:         对语法树节点求值
920:         node: 语法树节点
921:         local_vars: 局部变量字典
922:         返回: 求值结果
923:         """
924: 
925:         if node is None:
926:             return None
927: 
928:         if node.node_type == 'value':
929:             if node.value is None:
930:                 return None
931:             return self.get_value(node.value, local_vars)
932: 
933:         elif node.node_type == 'binary_op':
934:             left = self.evaluate_syntax_tree(node.left, local_vars)
935:             right = self.evaluate_syntax_tree(node.right, local_vars)
936:             return self.execute_binary_op(node.value, left, right)
937: 
938:         elif node.node_type == 'unary_op':
939:             child = self.evaluate_syntax_tree(node.left, local_vars)
940:             return self.execute_unary_op(node.value, child)
941: 
942:         elif node.node_type == 'function':
943:             return self.execute_function_node(node, local_vars)
944: 
945:         elif node.node_type == 'ternary':
946:             cond = self.evaluate_syntax_tree(node.left, local_vars)
947:             if cond:
948:                 return self.evaluate_syntax_tree(node.right, local_vars)
949:             else:
950:                 return self.evaluate_syntax_tree(node.third_child, local_vars)
951: 
952:         elif node.node_type == 'cast':
953:             inner = self.evaluate_syntax_tree(node.left, local_vars)
954:             if inner is None:
955:                 return None
956:             cast_type = node.value
957:             # float3x3转换: 从4x4矩阵提取前3x3
958:             if cast_type == 'float3x3' and isinstance(inner, list) and len(inner) == 4:
959:                 return [row[:3] for row in inner[:3]]
960:             # float2x2转换: 从4x4矩阵提取前2x2
961:             if cast_type == 'float2x2' and isinstance(inner, list) and len(inner) == 4:
962:                 return [row[:2] for row in inner[:2]]
963:             # float2x2转换: 从3x3矩阵提取前2x2
964:             if cast_type == 'float2x2' and isinstance(inner, list) and len(inner) == 3:
965:                 return [row[:2] for row in inner[:2]]
966:             return inner
967: 
968:         return None
969: 
970:     def execute_function_node(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
971:         """
972:         执行函数调用语法树节点
973:         node: 函数调用节点
974:         local_vars: 局部变量字典
975:         返回: 函数执行结果
976:         """
977:         func_name = node.value
978:         args = node.args
979: 
980:         # transpose: 矩阵转置函数
981:         # 计算矩阵的转置，将行列互换
982:         if func_name == 'transpose':
983:             if len(args) != 1:
984:                 self.debug_print(f"[ERROR] transpose requires 1 arg, got {len(args)} at line {node.line_number}")
985:                 return None
986:             val = self.evaluate_syntax_tree(args[0], local_vars)
987:             if val is None:
988:                 return None
989:             result = self.transpose_matrix(val)
990:             self.debug_print(f"[FUNC] transpose(\n{self._format_value(val)}) =\n{self._format_value(result)}")
991:             return result
992: 
993:         # normalize: 向量归一化函数
994:         # 将输入向量缩放到单位长度，即长度为1
995:         elif func_name == 'normalize':
996:             if len(args) != 1:
997:                 self.debug_print(f"[ERROR] normalize requires 1 arg, got {len(args)} at line {node.line_number}")
998:                 return None
999:             val = self.evaluate_syntax_tree(args[0], local_vars)
1000:             if val is None:
1001:                 return None
1002:             if isinstance(val, list):
1003:                 result = self.normalize_vec(val)
1004:                 self.debug_print(f"[FUNC] normalize({self._format_float(val)}) = {self._format_float(result)}")
1005:                 return result
1006:             return val
1007: 
1008:         # length: 向量长度函数
1009:         # 计算向量的欧几里得长度(模)
1010:         elif func_name == 'length':
1011:             if len(args) != 1:
1012:                 self.debug_print(f"[ERROR] length requires 1 arg, got {len(args)} at line {node.line_number}")
1013:                 return None
1014:             val = self.evaluate_syntax_tree(args[0], local_vars)
1015:             if val is None:
1016:                 return None
1017:             result = self.length_vec(val)
1018:             self.debug_print(f"[FUNC] length({self._format_float(val)}) = {self._format_float(result)}")
1019:             return result
1020: 
1021:         # dot: 向量点积函数
1022:         # 计算两个向量的点积，结果为标量
1023:         elif func_name == 'dot':
1024:             if len(args) != 2:
1025:                 self.debug_print(f"[ERROR] dot requires 2 args, got {len(args)} at line {node.line_number}")
1026:                 return None
1027:             a = self.evaluate_syntax_tree(args[0], local_vars)
1028:             b = self.evaluate_syntax_tree(args[1], local_vars)
1029:             if a is None or b is None:
1030:                 return None
1031:             result = self.dot_product(a, b)
1032:             self.debug_print(f"[FUNC] dot({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
1033:             return result
1034: 
1035:         # reflect: 反射向量函数
1036:         # 计算光线关于法向量的反射向量，公式: R = I - 2 * N * dot(I, N)
1037:         elif func_name == 'reflect':
1038:             if len(args) != 2:
1039:                 self.debug_print(f"[ERROR] reflect requires 2 args, got {len(args)} at line {node.line_number}")
1040:                 return None
1041:             I = self.evaluate_syntax_tree(args[0], local_vars)
1042:             N = self.evaluate_syntax_tree(args[1], local_vars)
1043:             if I is None or N is None:
1044:                 return None
1045:             result = self.reflect_vec(I, N)
1046:             self.debug_print(f"[FUNC] reflect({self._format_float(I)}, {self._format_float(N)}) = {self._format_float(result)}")
1047:             return result
1048: 
1049:         # max: 最大值函数
1050:         # 返回两个值中的较大者
1051:         elif func_name == 'max':
1052:             if len(args) != 2:
1053:                 self.debug_print(f"[ERROR] max requires 2 args, got {len(args)} at line {node.line_number}")
1054:                 return None
1055:             a = self.evaluate_syntax_tree(args[0], local_vars)
1056:             b = self.evaluate_syntax_tree(args[1], local_vars)
1057:             if a is None or b is None:
1058:                 return None
1059:             result = max(a, b)
1060:             self.debug_print(f"[FUNC] max({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
1061:             return result
1062: 
1063:         # min: 最小值函数
1064:         # 返回两个值中的较小者
1065:         elif func_name == 'min':
1066:             if len(args) != 2:
1067:                 self.debug_print(f"[ERROR] min requires 2 args, got {len(args)} at line {node.line_number}")
1068:                 return None
1069:             a = self.evaluate_syntax_tree(args[0], local_vars)
1070:             b = self.evaluate_syntax_tree(args[1], local_vars)
1071:             if a is None or b is None:
1072:                 return None
1073:             result = min(a, b)
1074:             self.debug_print(f"[FUNC] min({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
1075:             return result
1076: 
1077:         # pow: 幂函数
1078:         # 计算base的exp次幂，即 base ^ exp
1079:         elif func_name == 'pow':
1080:             if len(args) != 2:
1081:                 self.debug_print(f"[ERROR] pow requires 2 args, got {len(args)} at line {node.line_number}")
1082:                 return None
1083:             base = self.evaluate_syntax_tree(args[0], local_vars)
1084:             exp = self.evaluate_syntax_tree(args[1], local_vars)
1085:             if base is None or exp is None:
1086:                 return None
1087:             result = math.pow(base, exp)
1088:             self.debug_print(f"[FUNC] pow({self._format_float(base)}, {self._format_float(exp)}) = {self._format_float(result)}")
1089:             return result
1090: 
1091:         # abs: 绝对值函数
1092:         # 返回数值的绝对值，对列表则对每个元素取绝对值
1093:         elif func_name == 'abs':
1094:             if len(args) != 1:
1095:                 self.debug_print(f"[ERROR] abs requires 1 arg, got {len(args)} at line {node.line_number}")
1096:                 return None
1097:             val = self.evaluate_syntax_tree(args[0], local_vars)
1098:             if val is None:
1099:                 return None
1100:             if isinstance(val, list):
1101:                 result = [abs(v) for v in val]
1102:             else:
1103:                 result = abs(val)
1104:             self.debug_print(f"[FUNC] abs({self._format_float(val)}) = {self._format_float(result)}")
1105:             return result
1106: 
1107:         # sin: 正弦函数
1108:         # 计算弧度的正弦值，对列表则对每个元素计算
1109:         elif func_name == 'sin':
1110:             if len(args) != 1:
1111:                 self.debug_print(f"[ERROR] sin requires 1 arg, got {len(args)} at line {node.line_number}")
1112:                 return None
1113:             val = self.evaluate_syntax_tree(args[0], local_vars)
1114:             if val is None:
1115:                 return None
1116:             if isinstance(val, list):
1117:                 result = [math.sin(v) for v in val]
1118:             else:
1119:                 result = math.sin(val)
1120:             self.debug_print(f"[FUNC] sin({self._format_float(val)}) = {self._format_float(result)}")
1121:             return result
1122: 
1123:         # cos: 余弦函数
1124:         # 计算弧度的余弦值，对列表则对每个元素计算
1125:         elif func_name == 'cos':
1126:             if len(args) != 1:
1127:                 self.debug_print(f"[ERROR] cos requires 1 arg, got {len(args)} at line {node.line_number}")
1128:                 return None
1129:             val = self.evaluate_syntax_tree(args[0], local_vars)
1130:             if val is None:
1131:                 return None
1132:             if isinstance(val, list):
1133:                 result = [math.cos(v) for v in val]
1134:             else:
1135:                 result = math.cos(val)
1136:             self.debug_print(f"[FUNC] cos({self._format_float(val)}) = {self._format_float(result)}")
1137:             return result
1138: 
1139:         # mul: 矩阵乘法函数
1140:         # 执行4x4或3x3矩阵乘法运算
1141:         elif func_name == 'mul':
1142:             if len(args) != 2:
1143:                 self.debug_print(f"[ERROR] mul requires 2 args, got {len(args)} at line {node.line_number}")
1144:                 return None
1145:             left = self.evaluate_syntax_tree(args[0], local_vars)
1146:             right = self.evaluate_syntax_tree(args[1], local_vars)
1147:             if left is None or right is None:
1148:                 return None
1149:             if isinstance(left, list) and isinstance(right, list):
1150:                 if len(left) == 4 and len(right) == 4:
1151:                     result = self.mul_matrix_vector(right, left)
1152:                     self.debug_print(f"[FUNC] mul(\nleft={self._format_value(left)},\nright={self._format_value(right)}) =\n{self._format_value(result)}")
1153:                     return result
1154:                 elif len(left) == 3 and len(right) == 3:
1155:                     result = self.mul_matrix_vector(right, left)
1156:                     self.debug_print(f"[FUNC] mul(\nleft={self._format_value(left)},\nright={self._format_value(right)}) =\n{self._format_value(result)}")
1157:                     return result
1158:             return None
1159: 
1160:         # float2/float3/float4: 向量构造函数
1161:         # 将参数展平合并为指定长度的向量
1162:         elif func_name in ['float2', 'float3', 'float4']:
1163:             # 向量构造函数: 将参数展平合并
1164:             result = []
1165:             for arg in args:
1166:                 val = self.evaluate_syntax_tree(arg, local_vars)
1167:                 if isinstance(val, list):
1168:                     result.extend(val)
1169:                 else:
1170:                     result.append(val)
1171:             self.debug_print(f"[FUNC] {func_name}(args={self._format_float(args)}) = {self._format_float(result)}")
1172:             return result
1173: 
1174:         return None
1175: 
1176:     def apply_swizzle(self, obj: Any, swizzle: str) -> Any:
1177:         """
1178:         对向量应用swizzle操作
1179:         obj: 向量对象(列表)
1180:         swizzle: swizzle模式字符串，如 'xyz', 'xxx', 'xxyy', 'xz' 等
1181:         返回: 应用swizzle后的结果
1182:         """
1183:         if obj is None:
1184:             return None
1185: 
1186:         if not isinstance(obj, list):
1187:             if swizzle == 'x':
1188:                 return obj
1189:             return None
1190: 
1191:         valid_chars = {'x': 0, 'y': 1, 'z': 2, 'w': 3}
1192:         result = []
1193:         for c in swizzle:
1194:             if c.lower() in valid_chars:
1195:                 idx = valid_chars[c.lower()]
1196:                 if idx < len(obj):
1197:                     result.append(obj[idx])
1198:                 else:
1199:                     result.append(0)
1200:             elif c in 'rgb':
1201:                 idx = {'r': 0, 'g': 1, 'b': 2}[c]
1202:                 if idx < len(obj):
1203:                     result.append(obj[idx])
1204:                 else:
1205:                     result.append(0)
1206: 
1207:         if len(result) == 1:
1208:             return result[0]
1209: 
1210:         numeric_types = (int, float)
1211:         if all(isinstance(v, numeric_types) for v in result):
1212:             if all(isinstance(v, int) for v in result):
1213:                 return [int(v) for v in result]
1214:             return result
1215: 
1216:         return result
1217: 
1218:     def get_value(self, name: str, local_vars: Dict[str, Any]) -> Any:
1219:         """
1220:         获取变量或常量的值
1221:         name: 变量名/常量名，支持结构体字段访问(如 input.Pos)
1222:         local_vars: 局部变量字典
1223:         返回: 变量值，如果未找到返回0.0
1224:         """
1225:         name = name.strip()
1226: 
1227:         # 处理布尔常量
1228:         if name == 'true':
1229:             return True
1230:         if name == 'false':
1231:             return False
1232: 
1233:         # 尝试解析为数字
1234:         try:
1235:             return float(name)
1236:         except ValueError:
1237:             pass
1238: 
1239:         # 检查是否包含swizzle操作 (如 LightPos.xyz, LightPos.xxx, input.Pos.xy)
1240:         if '.' in name:
1241:             parts = name.split('.')
1242:             if len(parts) >= 2:
1243:                 base_name = parts[0]
1244:                 swizzle_str = parts[1]
1245: 
1246:                 # 判断是否为swizzle模式（全是xyzwrgb组成的字符串）
1247:                 if swizzle_str and all(c in 'xyzwrgb' for c in swizzle_str.lower()):
1248:                     obj = local_vars.get(base_name)
1249:                     if obj is None:
1250:                         obj = self.variables.get(base_name)
1251:                     if obj is not None:
1252:                         return self.apply_swizzle(obj, swizzle_str)
1253: 
1254:                     # 尝试从cbuffer获取
1255:                     for cb_name, cb_def in self.cbuffers.items():
1256:                         if isinstance(cb_def, CbufferDefinition):
1257:                             for field in cb_def.fields:
1258:                                 if field.name == base_name:
1259:                                     if field.data is not None:
1260:                                         return self.apply_swizzle(field.data, swizzle_str)
1261:                                     return 0
1262: 
1263:                     # 检查是否在output对象中
1264:                     if base_name in local_vars:
1265:                         obj = local_vars[base_name]
1266:                         if isinstance(obj, dict):
1267:                             return self.apply_swizzle(obj.get(swizzle_str), swizzle_str) if isinstance(obj.get(swizzle_str), list) else self.apply_swizzle(obj, swizzle_str)
1268:                         return self.apply_swizzle(obj, swizzle_str)
1269: 
1270:                     return 0
1271: 
1272:         # 局部变量查找
1273:         if name in local_vars:
1274:             val = local_vars[name]
1275:             return val
1276: 
1277:         base_name = name.split('.')[0] if '.' in name else name
1278: 
1279:         # cbuffer字段查找
1280:         for cb_name, cb_def in self.cbuffers.items():
1281:             if isinstance(cb_def, CbufferDefinition):
1282:                 for field in cb_def.fields:
1283:                     if field.name == base_name:
1284:                         return field.data if field.data is not None else 0
1285: 
1286:         # 全局变量查找
1287:         if name in self.variables:
1288:             return self.variables[name]
1289: 
1290:         # 嵌套cbuffer查找
1291:         try:
1292:             if '.' in name:
1293:                 parts = name.split('.')

(Output capped at 50 KB. Showing lines 1-1293. Use offset=1294 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 13.2s)

_Thinking:_

用户希望将 `HLSLInterpreter` 的 `log_file_mode` 和 `print_sequence` 参数从构造函数参数改为从 JSON 文件中读取，就像 `hlsl_source_file` 和 `csv_data_file_path` 一样。

让我先看看 JSON 文件的格式以及如何被使用的。


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "offset": 1294
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\hlsl_interpreter.py</path>
<type>file</type>
<content>
1294:                 base = parts[0]
1295:                 for cb_name, cb_data in self.cbuffers.items():
1296:                     if base in cb_data:
1297:                         val = cb_data[base]
1298:                         for p in parts[1:]:
1299:                             if isinstance(val, list) and p in ['x', 'y', 'z', 'w']:
1300:                                 idx = ['x', 'y', 'z', 'w'].index(p)
1301:                                 val = val[idx] if idx < len(val) else 0
1302:                             else:
1303:                                 break
1304:                         return val
1305:         except:
1306:             pass
1307: 
1308:         return 0.0
1309: 
1310:     def execute_statement(self, stmt: str, local_vars: Dict[str, Any]):
1311:         """
1312:         执行单条HLSL语句
1313:         stmt: 语句字符串，如 "float3 pos = input.Pos;" 或 "output.Color = float4(1,0,0,1);"
1314:         local_vars: 局部变量字典
1315:         """
1316:         stmt = stmt.strip()
1317:         if not stmt:
1318:             return None
1319: 
1320:         self.debug_print(f"\n[STMT] Executing: {stmt}")
1321:         input_snapshot = {k: v for k, v in local_vars.items() if k.startswith('input.') or k == 'output'}
1322: 
1323:         # 变量声明语句: float4 pos = ...;
1324:         type_pattern = '|'.join(DATA_TYPE_LIST)
1325:         pattern = rf'^({type_pattern})\s+(\w+)\s*=\s*(.+?);?$'
1326:         match = re.match(pattern, stmt)
1327:         if match:
1328:             var_name = match.group(2)
1329:             value = self.evaluate_expression(match.group(3), local_vars)
1330:             local_vars[var_name] = value
1331:             self.debug_print(f"[STMT] {stmt} => {var_name} = {self._format_value(value)}")
1332:             return None
1333: 
1334:         # output字段赋值: output.Color = ...;
1335:         if 'output.' in stmt or 'output[' in stmt:
1336:             match = re.match(r'output\.(\w+)\s*=\s*(.+)', stmt)
1337:             if match:
1338:                 field_name = match.group(1)
1339:                 value_expr = match.group(2).rstrip(';').strip()
1340:                 value = self.evaluate_expression(value_expr, local_vars)
1341:                 if 'output' not in local_vars:
1342:                     local_vars['output'] = {}
1343:                 local_vars['output'][field_name] = value
1344:                 self.debug_print(f"[STMT] {stmt} => output.{field_name} = {self._format_float(value)}")
1345:                 return None
1346: 
1347:         # 一般赋值语句: var = ...;
1348:         if '=' in stmt and stmt.count('=') == 1:
1349:             match = re.match(r'(\w+)\s*=\s*(.+?);?$', stmt)
1350:             if match:
1351:                 var_name = match.group(1)
1352:                 value = self.evaluate_expression(match.group(2), local_vars)
1353:                 local_vars[var_name] = value
1354:                 self.debug_print(f"[STMT] {stmt} => {var_name} = {value}")
1355:                 return None
1356: 
1357:         self.debug_print(f"[STMT] {stmt} => (no assignment)")
1358:         return None
1359: 
1360:     def execute_main_function(self, code: str, main_func: str, input_struct_name: str, row_index: int, data: Dict[str, Any]):
1361:         """
1362:         执行HLSL main函数
1363:         code: HLSL代码
1364:         main_func: main函数名
1365:         input_struct_name: 输入结构体名
1366:         row_index: 数据行索引
1367:         data: 输入数据字典
1368:         返回: output结构体字典
1369:         """
1370:         input_struct = self.structs.get(input_struct_name)
1371:         if not input_struct:
1372:             self.log_output(f"Cannot find input_struct: {input_struct_name}\n")
1373:             return None
1374: 
1375:         input_fields = {}
1376:         for field in input_struct.fields:
1377:             input_fields[field.name] = field.field_type
1378: 
1379:         # 查找main函数签名
1380:         func_signature_pattern = r'(\w+)\s+' + re.escape(main_func) + r'\s*\(\s*(\w+)\s+input\s*\)'
1381:         func_signature_match = re.search(func_signature_pattern, code)
1382:         if not func_signature_match:
1383:             return None
1384: 
1385:         output_struct_name = func_signature_match.group(1)
1386:         input_struct_name_from_func = func_signature_match.group(2)
1387: 
1388:         if output_struct_name not in self.structs:
1389:             return None
1390: 
1391:         output_struct = self.structs[output_struct_name]
1392:         output_fields = {}
1393:         for field in output_struct.fields:
1394:             output_fields[field.name] = field.field_type
1395: 
1396:         # 定位main函数体
1397:         func_signature = rf'{output_struct_name}\s+{main_func}\s*\(\s*{input_struct_name_from_func}\s+input\s*\)'
1398:         func_start = re.search(func_signature, code)
1399:         if not func_start:
1400:             return None
1401: 
1402:         # 提取函数体(处理嵌套大括号)
1403:         open_brace_pos = func_start.end()
1404:         brace_depth = 1
1405:         pos = open_brace_pos
1406:         while pos < len(code) and brace_depth > 0:
1407:             if code[pos] == '{':
1408:                 brace_depth += 1
1409:             elif code[pos] == '}':
1410:                 brace_depth -= 1
1411:             pos += 1
1412: 
1413:         body = code[open_brace_pos+1:pos-1].strip()
1414:         if body.startswith('{') and body.endswith('}'):
1415:             body = body[1:-1].strip()
1416: 
1417:         # 初始化局部变量
1418:         local_vars = {'data': data}
1419: 
1420:         # 设置input字段变量
1421:         for field_name, field_value in data.items():
1422:             local_vars[f'input.{field_name}'] = field_value
1423: 
1424:         # 初始化output对象
1425:         output_obj = {}
1426:         for field in output_fields:
1427:             output_obj[field] = None
1428:         local_vars['output'] = output_obj
1429: 
1430:         # 分割语句
1431:         statements = []
1432:         current_stmt = []
1433:         brace_count = 0
1434:         in_string = False
1435: 
1436:         for char in body:
1437:             if char == '{':
1438:                 brace_count += 1
1439:                 current_stmt.append(char)
1440:             elif char == '}':
1441:                 brace_count -= 1
1442:                 current_stmt.append(char)
1443:             elif char == ';' and brace_count == 0 and not in_string:
1444:                 stmt = ''.join(current_stmt).strip()
1445:                 if stmt:
1446:                     statements.append(stmt)
1447:                 current_stmt = []
1448:             else:
1449:                 current_stmt.append(char)
1450: 
1451:         if current_stmt:
1452:             stmt = ''.join(current_stmt).strip()
1453:             if stmt:
1454:                 statements.append(stmt)
1455: 
1456:         ret_val = None
1457: 
1458:         self._eval_counter += 1
1459:         self._should_print = ((self._eval_counter - 1) % self.print_sequence == 0)
1460: 
1461:         self.debug_print(f"******************************************************")
1462:         self.debug_print(f"**************Begin {self._eval_counter}**************")
1463:         self.debug_print(f"******************************************************\n")
1464: 
1465:         self.debug_print(f"\n=== INPUT DATA ===")
1466:         for k, v in local_vars.items():
1467:             if k.startswith('input.') or k == 'output':
1468:                 self.debug_print(f"  {k} = {v}")
1469:         self.debug_print(f"==================")
1470: 
1471:         # 顺序执行语句
1472:         for stmt in statements:
1473:             if 'return' in stmt and 'output' in stmt:
1474:                 ret_val = local_vars.get('output')
1475:                 continue
1476:             self.execute_statement(stmt, local_vars)
1477: 
1478:         self.debug_print(f"******************************************************")
1479:         self.debug_print(f"**************End {self._eval_counter}**************")
1480:         self.debug_print(f"******************************************************\n")
1481: 
1482:         return ret_val
1483: 
1484:     def interpret(self, hlsl_file_path: str, csv_folder_path: str = None):
1485:         """
1486:         解释HLSL代码 - 解析结构体和cbuffer定义
1487:         hlsl_file_path: HLSL文件路径
1488:         csv_folder_path: CSV文件夹路径（如果为None则不加载CSV数据）
1489:         """
1490:         if not os.path.exists(hlsl_file_path):
1491:             self.log_output(f"Error: HLSL file not found: {hlsl_file_path}")
1492:             return
1493: 
1494:         with open(hlsl_file_path, 'r', encoding='utf-8') as f:
1495:             self.hlsl_code = f.read()
1496: 
1497:         code = self.hlsl_code
1498: 
1499:         if csv_folder_path is None:
1500:             csv_folder_path = os.path.dirname(hlsl_file_path)
1501: 
1502:         # 解析struct定义
1503:         struct_pattern = r'struct\s+\w+\s*\{[^}]+\}'
1504:         for struct_match in re.finditer(struct_pattern, code):
1505:             struct_def = self.parse_struct(struct_match.group())
1506:             if struct_def:
1507:                 self.structs[struct_def.name] = struct_def
1508: 
1509:         # 解析cbuffer定义
1510:         cbuffer_pattern = r'cbuffer\s+\w+[^}]+\}'
1511:         for cb_match in re.finditer(cbuffer_pattern, code, re.DOTALL):
1512:             cb_def = self.parse_cbuffer(cb_match.group())
1513:             if cb_def:
1514:                 self.cbuffers[cb_def.name] = cb_def
1515: 
1516:         # 从CSV加载struct数据
1517:         for struct_name in self.structs:
1518:             csv_path = os.path.join(csv_folder_path, f'{struct_name}.csv')
1519:             if os.path.exists(csv_path):
1520:                 self.load_struct_data_from_csv(struct_name, csv_path)
1521: 
1522:         # 从CSV加载cbuffer数据
1523:         for cb_name in self.cbuffers:
1524:             csv_path = os.path.join(csv_folder_path, f'{cb_name}.csv')
1525:             if os.path.exists(csv_path):
1526:                 self.load_cbuffer_data_from_csv(cb_name, csv_path)
1527: 
1528:     def executeVS(self, main_func: str, vs_input: str, code: str = None):
1529:         """
1530:         执行顶点着色器
1531:         main_func: 入口函数名
1532:         vs_input: 输入结构体名
1533:         code: HLSL代码（如果为None则使用self.hlsl_code）
1534:         返回: 输出结构体字典列表
1535:         """
1536:         if code is None:
1537:             code = self.hlsl_code
1538:         input_struct = self.structs.get(vs_input)
1539:         if not input_struct:
1540:             self.log_output(f"Cannot find vs input: {vs_input}\n")
1541:             return None
1542: 
1543:         # 统计行数
1544:         num_rows = 0
1545:         for field in input_struct.fields:
1546:             if field.data:
1547:                 num_rows = max(num_rows, len(field.data))
1548: 
1549:         results = []
1550:         for row_index in range(num_rows):
1551:             data = {}
1552:             for field in input_struct.fields:
1553:                 if field.data and row_index < len(field.data):
1554:                     data[field.name] = field.data[row_index]
1555:             result = self.execute_main_function(code, main_func, vs_input, row_index, data)
1556:             results.append(result)
1557:         return results
1558: 
1559:     def executePS(self, code: str, main_func: str, ps_input: str):
1560:         """
1561:         执行像素着色器(当前为占位函数)
1562:         """
1563:         pass
1564: 
1565:     def load_struct_data_from_csv(self, struct_name: str, csv_path: str):
1566:         """
1567:         从CSV文件加载struct数据
1568:         struct_name: 结构体名称
1569:         csv_path: CSV文件路径
1570:         """
1571:         if struct_name not in self.structs:
1572:             return
1573:         struct_def = self.structs[struct_name]
1574:         rows = self.load_csv(csv_path)
1575:         if not rows or len(rows) < 2:
1576:             return
1577: 
1578:         header = rows[0]
1579:         data_rows = rows[1:]
1580: 
1581:         # 建立字段列索引映射
1582:         field_col_indices = {}
1583:         for i, col in enumerate(header):
1584:             col_clean = col.strip()
1585:             if '.' in col_clean:
1586:                 parts = col_clean.split('.')
1587:                 base_name = parts[0]
1588:                 suffix = parts[1]
1589:                 if base_name not in field_col_indices:
1590:                     field_col_indices[base_name] = {}
1591:                 field_col_indices[base_name][suffix] = i
1592: 
1593:         # 填充字段数据
1594:         for field in struct_def.fields:
1595:             if field.semantic in field_col_indices:
1596:                 col_dict = field_col_indices[field.semantic]
1597:                 values = []
1598:                 for row in data_rows:
1599:                     if 'x' in col_dict and 'y' in col_dict and 'z' in col_dict and 'w' in col_dict:
1600:                         x = float(row[col_dict['x']].strip())
1601:                         y = float(row[col_dict['y']].strip())
1602:                         z = float(row[col_dict['z']].strip())
1603:                         w = float(row[col_dict['w']].strip())
1604:                         values.append([x, y, z, w])
1605:                     elif 'x' in col_dict and 'y' in col_dict and 'z' in col_dict:
1606:                         x = float(row[col_dict['x']].strip())
1607:                         y = float(row[col_dict['y']].strip())
1608:                         z = float(row[col_dict['z']].strip())
1609:                         values.append([x, y, z])
1610:                     elif 'x' in col_dict and 'y' in col_dict:
1611:                         x = float(row[col_dict['x']].strip())
1612:                         y = float(row[col_dict['y']].strip())
1613:                         values.append([x, y])
1614:                     else:
1615:                         val_str = row[col_dict['x']].strip().strip('"')
1616:                         values.append(self.parse_value_by_type(val_str, field.field_type))
1617:                 field.data = values
1618:                 self.log_output(f"Field '{field.semantic}' ({field.field_type}): {values[0] if values else 'N/A'}")
1619: 
1620:     def load_cbuffer_data_from_csv(self, cb_name: str, csv_path: str):
1621:         """
1622:         从CSV文件加载cbuffer数据
1623:         cb_name: cbuffer名称
1624:         csv_path: CSV文件路径
1625:         """
1626:         if cb_name not in self.cbuffers:
1627:             return
1628:         cb_def = self.cbuffers[cb_name]
1629:         rows = self.load_csv(csv_path)
1630:         if not rows or len(rows) < 2:
1631:             return
1632: 
1633:         header = rows[0]
1634:         name_idx = header.index('Name') if 'Name' in header else -1
1635:         value_idx = header.index('Value') if 'Value' in header else -1
1636:         type_idx = header.index('Type') if 'Type' in header else -1
1637: 
1638:         if name_idx == -1 or value_idx == -1:
1639:             return
1640: 
1641:         matrix_rows = {}
1642:         scalar_vars = {}
1643: 
1644:         for row in rows[1:]:
1645:             if len(row) <= max(name_idx, value_idx):
1646:                 continue
1647:             var_name = row[name_idx].strip().strip('"')
1648:             value_str = row[value_idx].strip().strip('"') if value_idx < len(row) else ''
1649:             type_str = row[type_idx].strip().strip('"') if type_idx != -1 and type_idx < len(row) else ''
1650: 
1651:             # 跳过空值
1652:             if value_str == '':
1653:                 continue
1654: 
1655:             # 矩阵行处理(如 World.row0, World.row1)
1656:             if '.' in var_name:
1657:                 parts = var_name.split('.')
1658:                 base_name = parts[0]
1659:                 suffix = parts[1]
1660:                 if suffix.startswith('row'):
1661:                     row_idx = int(suffix[3:])
1662:                     if base_name not in matrix_rows:
1663:                         matrix_rows[base_name] = {}
1664:                     matrix_rows[base_name][row_idx] = (value_str, type_str)
1665:             else:
1666:                 scalar_vars[var_name] = (value_str, type_str)
1667: 
1668:         # 填充字段数据
1669:         for field in cb_def.fields:
1670:             if field.name in matrix_rows:
1671:                 row_dict = matrix_rows[field.name]
1672:                 if all(i in row_dict for i in range(4)):
1673:                     matrix = []
1674:                     for i in range(4):
1675:                         value_str, type_str = row_dict[i]
1676:                         parts = value_str.split(',')
1677:                         matrix.append([float(p.strip()) for p in parts[:4]])
1678:                     field.data = matrix
1679:             elif field.name in scalar_vars:
1680:                 value_str, type_str = scalar_vars[field.name]
1681:                 field.data = self.parse_value_by_type(value_str, type_str)
1682: 
1683:         # 打印cbuffer内容
1684:         cb_n = cb_name
1685:         cb_d = cb_def
1686:         self.log_output(f"Cbuffer {cb_n}:")
1687:         for f in cb_d.fields:
1688:             data = f.data
1689:             ft = f.field_type
1690:             if 'float4x4' in ft:
1691:                 self.log_output(f"  {f.name} ({ft}):")
1692:                 for row in data:
1693:                     row_str = '  '.join(f"{v:12.5f}" for v in row)
1694:                     self.log_output(f"    [{row_str}]")
1695:             elif 'float3x3' in ft:
1696:                 self.log_output(f"  {f.name} ({ft}):")
1697:                 for row in data:
1698:                     row_str = '  '.join(f"{v:12.5f}" for v in row)
1699:                     self.log_output(f"    [{row_str}]")
1700:             elif 'float4' in ft:
1701:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
1702:             elif 'float3' in ft:
1703:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
1704:             elif 'float2' in ft:
1705:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
1706:             elif 'float' in ft:
1707:                 self.log_output(f"  {f.name} ({ft}): {data:.5f}")
1708:             elif 'uint4' in ft:
1709:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1710:             elif 'uint3' in ft:
1711:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1712:             elif 'uint2' in ft:
1713:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1714:             elif 'uint' in ft:
1715:                 self.log_output(f"  {f.name} ({ft}): {data}")
1716:             elif 'int4' in ft:
1717:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1718:             elif 'int3' in ft:
1719:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1720:             elif 'int2' in ft:
1721:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1722:             elif 'int' in ft:
1723:                 self.log_output(f"  {f.name} ({ft}): {data}")
1724:             elif 'bool' in ft:
1725:                 self.log_output(f"  {f.name} ({ft}): {data}")
1726:             else:
1727:                 self.log_output(f"  {f.name} ({ft}): {data}")
1728: 
1729:     def load_vs_output_golden_from_csv(self, csv_path: str):
1730:         """
1731:         从CSV文件加载VS_OUTPUT的golden数据
1732:         csv_path: CSV文件路径
1733:         """
1734:         if "VS_OUTPUT" not in self.structs:
1735:             self.log_output("Error: VS_OUTPUT struct not defined")
1736:             return False
1737: 
1738:         vs_output_def = self.structs["VS_OUTPUT"]
1739:         rows = self.load_csv(csv_path)
1740:         if not rows or len(rows) < 2:
1741:             self.log_output(f"Error: CSV file {csv_path} is empty or has no data rows")
1742:             return False
1743: 
1744:         header = rows[0]
1745:         data_rows = rows[1:]
1746: 
1747:         field_col_indices = {}
1748:         for i, col in enumerate(header):
1749:             col_clean = col.strip()
1750:             if '.' in col_clean:
1751:                 parts = col_clean.split('.')
1752:                 base_name = parts[0]
1753:                 suffix = parts[1]
1754:                 if base_name not in field_col_indices:
1755:                     field_col_indices[base_name] = {}
1756:                 field_col_indices[base_name][suffix] = i
1757: 
1758:         for field in vs_output_def.fields:
1759:             if field.semantic in field_col_indices:
1760:                 col_dict = field_col_indices[field.semantic]
1761:                 values = []
1762:                 for row in data_rows:
1763:                     try:
1764:                         if 'x' in col_dict and 'y' in col_dict and 'z' in col_dict and 'w' in col_dict:
1765:                             x = float(row[col_dict['x']].strip())
1766:                             y = float(row[col_dict['y']].strip())
1767:                             z = float(row[col_dict['z']].strip())
1768:                             w = float(row[col_dict['w']].strip())
1769:                             values.append([x, y, z, w])
1770:                         elif 'x' in col_dict and 'y' in col_dict and 'z' in col_dict:
1771:                             x = float(row[col_dict['x']].strip())
1772:                             y = float(row[col_dict['y']].strip())
1773:                             z = float(row[col_dict['z']].strip())
1774:                             values.append([x, y, z])
1775:                         elif 'x' in col_dict and 'y' in col_dict:
1776:                             x = float(row[col_dict['x']].strip())
1777:                             y = float(row[col_dict['y']].strip())
1778:                             values.append([x, y])
1779:                         else:
1780:                             val_str = row[col_dict['x']].strip().strip('"')
1781:                             values.append(self.parse_value_by_type(val_str, field.field_type))
1782:                     except (ValueError, IndexError) as e:
1783:                         self.log_output(f"Warning: Failed to parse {field.semantic} at row: {e}")
1784:                         values.append(None)
1785:                 field.data = values
1786: 
1787:         self.log_output(f"Loaded {len(data_rows)} golden data rows for VS_OUTPUT")
1788:         return True
1789: 
1790:     def compare_vs_output_with_golden(self, hlsl_output: List[Dict], output_struct_name: str = "VS_OUTPUT", float_tolerance: float = 0.0001) -> bool:
1791:         """
1792:         比较HLSL执行结果与golden数据
1793:         hlsl_output: executeVS返回的输出结构体字典列表
1794:         output_struct_name: 输出结构体名称，用于获取field name (默认"VS_OUTPUT")
1795:         float_tolerance: 浮点类型数据的比较误差容忍度
1796:         返回: True表示所有数据匹配, False表示存在不匹配
1797:         """
1798:         if output_struct_name not in self.structs:
1799:             self.log_output(f"Error: {output_struct_name} struct not found")
1800:             return False
1801: 
1802:         vs_output_def = self.structs[output_struct_name]
1803:         golden_data = {}
1804:         semantic_to_field = {}
1805: 
1806:         for field in vs_output_def.fields:
1807:             if field.data:
1808:                 golden_data[field.semantic] = field.data
1809:             semantic_to_field[field.semantic] = field.name
1810: 
1811:         num_golden_rows = 0
1812:         for field_data in golden_data.values():
1813:             if field_data:
1814:                 num_golden_rows = max(num_golden_rows, len(field_data))
1815: 
1816:         if not hlsl_output:
1817:             self.log_output("Error: No HLSL output to compare")
1818:             return False
1819: 
1820:         if len(hlsl_output) != num_golden_rows:
1821:             self.log_output(f"Error: Row count mismatch - HLSL output has {len(hlsl_output)} rows, golden has {num_golden_rows} rows")
1822:             return False
1823: 
1824:         all_match = True
1825:         passed_count = 0
1826:         field_type_map = {}
1827:         for field in vs_output_def.fields:
1828:             field_type_map[field.semantic] = field.field_type
1829: 
1830:         for row_idx in range(len(hlsl_output)):
1831:             output_row = hlsl_output[row_idx]
1832:             row_match = True
1833:             for semantic, golden_values in golden_data.items():
1834:                 if row_idx >= len(golden_values):
1835:                     continue
1836: 
1837:                 field_name = semantic_to_field.get(semantic, semantic)
1838:                 if field_name not in output_row:
1839:                     continue
1840: 
1841:                 output_value = output_row[field_name]
1842:                 golden_value = golden_values[row_idx]
1843: 
1844:                 if output_value is None or golden_value is None:
1845:                     continue
1846: 
1847:                 field_type = field_type_map.get(semantic, '')
1848: 
1849:                 if isinstance(output_value, list) and isinstance(golden_value, list):
1850:                     if len(output_value) != len(golden_value):
1851:                         self.log_output(f"Error: Row {row_idx}, {field_name}: length mismatch output={len(output_value)} golden={len(golden_value)}")
1852:                         row_match = False
1853:                         continue
1854: 
1855:                     is_float = 'float' in field_type
1856:                     for comp_idx in range(len(output_value)):
1857:                         out_comp = output_value[comp_idx]
1858:                         gold_comp = golden_value[comp_idx]
1859: 
1860:                         if is_float:
1861:                             if isinstance(out_comp, float) and isinstance(gold_comp, float):
1862:                                 if abs(out_comp - gold_comp) > float_tolerance:
1863:                                     self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp:.6f} golden={gold_comp:.6f} diff={abs(out_comp - gold_comp):.6f} > tolerance={float_tolerance}")
1864:                                     row_match = False
1865:                             elif out_comp != gold_comp:
1866:                                 self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp} golden={gold_comp} (float comparison failed)")
1867:                                 row_match = False
1868:                         else:
1869:                             if out_comp != gold_comp:
1870:                                 self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp} golden={gold_comp} (strict equality failed)")
1871:                                 row_match = False
1872: 
1873:             if row_match:
1874:                 passed_count += 1
1875:             else:
1876:                 all_match = False
1877: 
1878:         self.log_output(f"Total PASSED rows: {passed_count}/{num_golden_rows}")
1879:         if all_match:
1880:             self.log_output("Comparison PASSED: All output data matches golden data within tolerance")
1881:         else:
1882:             self.log_output("Comparison FAILED: Some output data does not match golden data")
1883: 
1884:         return all_match
1885: 
1886: 
1887: def main():
1888:     import sys
1889: 
1890:     if len(sys.argv) < 2:
1891:         print("Usage: python hlsl_interpreter.py <config.json>")
1892:         print("Config JSON should contain: hlsl_file_path, csv_folder_path, log_file_path")
1893:         config_path = './wrong_constant_attenuation.json'
1894:     else:
1895:         config_path = sys.argv[1]
1896: 
1897:     if not os.path.exists(config_path):
1898:         print(f"Error: Config file not found: {config_path}")
1899:         sys.exit(1)
1900: 
1901:     config = {}
1902:     with open(config_path, 'r', encoding='utf-8') as f:
1903:         config = json.load(f)
1904: 
1905:     hlsl_file_path = config.get('hlsl_file_path', '')
1906:     csv_folder_path = config.get('csv_folder_path', '')
1907:     log_file_path = config.get('log_file_path', 'hlsl_interpreter.log')
1908: 
1909:     if not hlsl_file_path:
1910:         print("Error: hlsl_file_path not specified in config")
1911:         sys.exit(1)
1912: 
1913:     if not os.path.exists(hlsl_file_path):
1914:         print(f"Error: HLSL file not found: {hlsl_file_path}")
1915:         sys.exit(1)
1916: 
1917:     if csv_folder_path and not os.path.exists(csv_folder_path):
1918:         print(f"Error: CSV folder not found: {csv_folder_path}")
1919:         sys.exit(1)
1920: 
1921:     interpreter = HLSLInterpreter(log_to_file=True, log_file_path=log_file_path, log_file_mode='w', print_sequence=100)
1922: 
1923:     total_start = time.time()
1924: 
1925:     interpret_start = time.time()
1926:     interpreter.interpret(hlsl_file_path, csv_folder_path)
1927:     interpret_time = time.time() - interpret_start
1928: 
1929:     golden_csv_path = os.path.join(csv_folder_path, 'VS_OUTPUT.csv') if csv_folder_path else None
1930:     load_golden_start = time.time()
1931:     if golden_csv_path and os.path.exists(golden_csv_path):
1932:         interpreter.load_vs_output_golden_from_csv(golden_csv_path)
1933:     load_golden_time = time.time() - load_golden_start
1934: 
1935:     execute_start = time.time()
1936:     results = interpreter.executeVS("main", "VS_INPUT")
1937:     execute_time = time.time() - execute_start
1938: 
1939:     interpreter.log_output("HLSL Interpreter Result:")
1940:     interpreter.log_output("=" * 40)
1941:     if results:
1942:         for idx, result in enumerate(results):
1943:             interpreter.log_output(f"\n--- Row {idx} ---")
1944:             if result:
1945:                 for key, value in result.items():
1946:                     if isinstance(value, list):
1947:                         if len(value) == 4:
1948:                             interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]")
1949:                         elif len(value) == 3:
1950:                             interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]")
1951:                         elif len(value) == 2:
1952:                             interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}]")
1953:                         else:
1954:                             interpreter.log_output(f"{key}: {value}")
1955:                     else:
1956:                         interpreter.log_output(f"{key}: {value}")
1957:     else:
1958:         interpreter.log_output("No result produced")
1959: 
1960:     if results and results[-1] and 'Color' in results[-1]:
1961:         color = results[-1]['Color']
1962:         if color and isinstance(color, list) and len(color) == 4:
1963:             interpreter.log_output("\nFinal Output Color (RGBA):")
1964:             interpreter.log_output(f"  R: {color[0]:.4f}")
1965:             interpreter.log_output(f"  G: {color[1]:.4f}")
1966:             interpreter.log_output(f"  B: {color[2]:.4f}")
1967:             interpreter.log_output(f"  A: {color[3]:.4f}")
1968:         else:
1969:             interpreter.log_output(f"\nColor result: {color}")
1970: 
1971:     interpreter.log_output("\n" + "=" * 40)
1972:     interpreter.log_output("Comparing with golden data...")
1973:     interpreter.log_output("=" * 40)
1974:     compare_start = time.time()
1975:     interpreter.compare_vs_output_with_golden(results)
1976:     compare_time = time.time() - compare_start
1977: 
1978:     total_time = time.time() - total_start
1979: 
1980:     interpreter.log_output("\n" + "=" * 40)
1981:     interpreter.log_output("Timing Summary:")
1982:     interpreter.log_output("=" * 40)
1983:     interpreter.log_output(f"interpreter.interpret():             {interpret_time:.4f}s")
1984:     interpreter.log_output(f"interpreter.load_vs_output_golden_from_csv(): {load_golden_time:.4f}s")
1985:     interpreter.log_output(f"interpreter.executeVS():           {execute_time:.4f}s")
1986:     interpreter.log_output(f"compare_vs_output_with_golden():    {compare_time:.4f}s")
1987:     interpreter.log_output(f"Total execution time:               {total_time:.4f}s")
1988: 
1989: 
1990: if __name__ == '__main__':
1991:     main()

(End of file - total 1991 lines)
</content>
```

**Tool: glob**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3",
  "pattern": "**/hlsl_interpreter/*.json"
}
```

**Output:**
```
C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\specular_too_shining.json
C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\wrong_constant_attenuation.json
```

---

## Assistant (Build · MiniMax-M2.7 · 10.5s)

_Thinking:_

Now I understand the code structure. The user wants to:
1. Move `log_file_mode` and `print_sequence` from being hardcoded in `main()` to being read from the JSON config file
2. These should be treated similarly to `hlsl_file_path` and `csv_folder_path` which are already read from the JSON

Let me check an example JSON file to see the structure.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\wrong_constant_attenuation.json"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\wrong_constant_attenuation.json</path>
<type>file</type>
<content>
1: {
2:     "hlsl_file_path": "./constant_buffer_attenuation_wrong/VERTEX_SHADER_STANDARD_POINT.hlsl",
3:     "csv_folder_path": "./constant_buffer_attenuation_wrong/",
4:     "log_file_path": "./constant_buffer_attenuation_wrong/output.log"
5: }

(End of file - total 5 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 14.6s)

_Thinking:_

Now I understand the structure. The user wants to:
1. Add `log_file_mode` and `print_sequence` to the JSON config file
2. Read them from the JSON and pass to `HLSLInterpreter`

Let me make these changes to the JSON files and the main function in hlsl_interpreter.py.


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\wrong_constant_attenuation.json",
  "newString": "{\n    \"hlsl_file_path\": \"./constant_buffer_attenuation_wrong/VERTEX_SHADER_STANDARD_POINT.hlsl\",\n    \"csv_folder_path\": \"./constant_buffer_attenuation_wrong/\",\n    \"log_file_path\": \"./constant_buffer_attenuation_wrong/output.log\",\n    \"log_file_mode\": \"w\",\n    \"print_sequence\": 100\n}",
  "oldString": "{\n    \"hlsl_file_path\": \"./constant_buffer_attenuation_wrong/VERTEX_SHADER_STANDARD_POINT.hlsl\",\n    \"csv_folder_path\": \"./constant_buffer_attenuation_wrong/\",\n    \"log_file_path\": \"./constant_buffer_attenuation_wrong/output.log\"\n}"
}
```

**Output:**
```
Edit applied successfully.
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\specular_too_shining.json"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\specular_too_shining.json</path>
<type>file</type>
<content>
1: {
2:     "hlsl_file_path": "./specular_too_shining/specular_too_shining.hlsl",
3:     "csv_folder_path": "./specular_too_shining/",
4:     "log_file_path": "./specular_too_shining/output.log"
5: }

(End of file - total 5 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 14.3s)

_Thinking:_

Now I need to:
1. Update specular_too_shining.json similarly
2. Update the main() function to read log_file_mode and print_sequence from the json config


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\specular_too_shining.json",
  "newString": "{\n    \"hlsl_file_path\": \"./specular_too_shining/specular_too_shining.hlsl\",\n    \"csv_folder_path\": \"./specular_too_shining/\",\n    \"log_file_path\": \"./specular_too_shining/output.log\",\n    \"log_file_mode\": \"w\",\n    \"print_sequence\": 100\n}",
  "oldString": "{\n    \"hlsl_file_path\": \"./specular_too_shining/specular_too_shining.hlsl\",\n    \"csv_folder_path\": \"./specular_too_shining/\",\n    \"log_file_path\": \"./specular_too_shining/output.log\"\n}"
}
```

**Output:**
```
Edit applied successfully.
```

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "newString": "hlsl_file_path = config.get('hlsl_file_path', '')\n    csv_folder_path = config.get('csv_folder_path', '')\n    log_file_path = config.get('log_file_path', 'hlsl_interpreter.log')\n    log_file_mode = config.get('log_file_mode', 'a')\n    print_sequence = config.get('print_sequence', 1)",
  "oldString": "hlsl_file_path = config.get('hlsl_file_path', '')\n    csv_folder_path = config.get('csv_folder_path', '')\n    log_file_path = config.get('log_file_path', 'hlsl_interpreter.log')"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 10.8s)

_Thinking:_

Now I need to update the HLSLInterpreter instantiation to use the config values instead of hardcoded ones.


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "newString": "interpreter = HLSLInterpreter(log_to_file=True, log_file_path=log_file_path, log_file_mode=log_file_mode, print_sequence=print_sequence)",
  "oldString": "interpreter = HLSLInterpreter(log_to_file=True, log_file_path=log_file_path, log_file_mode='w', print_sequence=100)"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 11.2s)

_Thinking:_

Done. I've made the following changes:

1. Modified `hlsl_interpreter.py` at line 1905-1908 to read `log_file_mode` and `print_sequence` from the JSON config file, with defaults `'a'` and `1` respectively.

2. Updated line 1921 to use the values from config instead of hardcoded values.

3. Updated both JSON config files (`wrong_constant_attenuation.json` and `specular_too_shining.json`) to include the new fields.




Done. Changes made:

1. **hlsl_interpreter.py** - Added reading `log_file_mode` and `print_sequence` from JSON config (lines 1908-1909), and updated `HLSLInterpreter` creation (line 1921) to use config values instead of hardcoded `log_file_mode='w', print_sequence=100`

2. **wrong_constant_attenuation.json** and **specular_too_shining.json** - Added `"log_file_mode": "w"` and `"print_sequence": 100` to both

---

