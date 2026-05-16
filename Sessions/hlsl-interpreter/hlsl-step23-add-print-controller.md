# Add print_sequency control to HLSL interpreter

**Session ID:** ses_1cf9899e5ffeP9ox5gGGQrvwd1
**Created:** 2026/5/16 18:49:03
**Updated:** 2026/5/16 18:51:05

---

## User

hlsl_interpreter.py的self.evaluate_syntax_tree每次执行时，会打印详细的HLSL指令的计算过程，打印的例子如下
[STMT] Executing: float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist)
[SYNTAX TREE]
BinaryOp(/)
  left:
    Value(1.0)
  right:
BinaryOp(+)
      left:
BinaryOp(+)
          left:
            Value(Attenuation.x)
          right:
BinaryOp(*)
              left:
                Value(Attenuation.y)
              right:
                Value(dist)
      right:
BinaryOp(*)
          left:
BinaryOp(*)
              left:
                Value(Attenuation.z)
              right:
                Value(dist)
          right:
            Value(dist)
[BINARY OP] left=0.0000, right=498.6749, op=*, result=0.0000
[BINARY OP] left=0.0017, right=0.0000, op=+, result=0.0017
[BINARY OP] left=45.0000, right=498.6749, op=*, result=22440.3694
[BINARY OP] left=22440.3694, right=498.6749, op=*, result=11190448.3832
[BINARY OP] left=0.0017, right=11190448.3832, op=+, result=11190448.3849
[BINARY OP] left=1.0000, right=11190448.3849, op=/, result=0.0000
[STMT] float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist) => att = 0.0000
请增加一个控制变量，使得self.evaluate_syntax_tree不需要每次都打印，可以根据用户配置的print_sequency值，来间隔打印。
例如print_sequency=2，意味着每执行self.evaluate_syntax_tree两次，打印其中的log一次
print_sequency=200，意味着每执行self.evaluate_syntax_tree 200次，打印其中的log一次 

---

## Assistant (Build · MiniMax-M2.7 · 7.8s)

_Thinking:_

用户想要在 `hlsl_interpreter.py` 中的 `self.evaluate_syntax_tree` 方法添加一个控制变量 `print_sequency`，用于控制日志打印的频率。

让我先找到这个文件并查看其内容。


**Tool: glob**

**Input:**
```json
{
  "pattern": "**/*hlsl_interpreter*.py"
}
```

**Output:**
```
C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\hlsl_interpreter.py
C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\specular_too_shining\hlsl_interpreter.py
```

---

## Assistant (Build · MiniMax-M2.7 · 7.6s)

_Thinking:_

Let me read the main hlsl_interpreter.py file to understand the class structure.


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
6: from dataclasses import dataclass, field
7: from typing import Any, Dict, List, Union, Optional
8: 
9: 
10: DATA_TYPE_LIST = [
11:     'float4x4', 'float3x3',  # 矩阵类型
12:     'float4', 'float3', 'float2', 'float',  # 浮点向量/标量
13:     'uint4', 'uint3', 'uint2', 'uint',  # 无符号整数
14:     'int4', 'int3', 'int2', 'int',  # 有符号整数
15:     'bool'  # 布尔类型
16: ]
17: 
18: 
19: class SyntaxTreeNode:
20:     """
21:     HLSL语法树节点基类
22:     用于表示HLSL表达式解析后的语法树结构
23:     node_type: 节点类型 - 'value'(值), 'function'(函数), 'binary_op'(二元操作),
24:                             'unary_op'(一元操作), 'cast'(类型转换), 'ternary'(三元条件)
25:     value: 节点值 - 变量名/函数名/操作符/类型名
26:     left: 左子节点 (用于二元/一元操作或三元条件)
27:     right: 右子节点 (用于二元操作或三元真的表达式)
28:     third_child: 第三子节点 (用于三元条件假的表达式)
29:     args: 函数参数列表 (用于函数调用)
30:     """
31:     def __init__(self, node_type: str, value: Any = None, left: Optional['SyntaxTreeNode'] = None, right: Optional['SyntaxTreeNode'] = None, third_child: Optional['SyntaxTreeNode'] = None, args: Optional[List['SyntaxTreeNode']] = None, line_number: int = 0):
32:         self.node_type = node_type
33:         self.value = value
34:         self.left = left
35:         self.right = right
36:         self.third_child = third_child
37:         self.args = args if args is not None else []
38:         self.line_number = line_number
39: 
40:     def __repr__(self):
41:         return self._pretty(0)
42: 
43:     def _pretty(self, indent: int) -> str:
44:         """
45:         格式化输出语法树，用于调试
46:         indent: 缩进层级
47:         """
48:         prefix = "  " * indent
49:         if self.node_type == 'function':
50:             lines = [f"Function({self.value})"]
51:             for i, arg in enumerate(self.args):
52:                 lines.append(f"{prefix}  arg[{i}]:")
53:                 lines.append(arg._pretty(indent + 2))
54:             return "\n".join(lines)
55:         elif self.node_type == 'binary_op':
56:             lines = [f"BinaryOp({self.value})"]
57:             lines.append(f"{prefix}  left:")
58:             lines.append(self.left._pretty(indent + 2))
59:             lines.append(f"{prefix}  right:")
60:             lines.append(self.right._pretty(indent + 2))
61:             return "\n".join(lines)
62:         elif self.node_type == 'unary_op':
63:             lines = [f"UnaryOp({self.value})"]
64:             lines.append(f"{prefix}  child:")
65:             lines.append(self.left._pretty(indent + 2))
66:             return "\n".join(lines)
67:         elif self.node_type == 'cast':
68:             lines = [f"Cast({self.value})"]
69:             lines.append(f"{prefix}  inner:")
70:             lines.append(self.left._pretty(indent + 2))
71:             return "\n".join(lines)
72:         elif self.node_type == 'ternary':
73:             lines = [f"Ternary({self.value})"]
74:             lines.append(f"{prefix}  condition:")
75:             lines.append(self.left._pretty(indent + 2))
76:             lines.append(f"{prefix}  true_expr:")
77:             lines.append(self.right._pretty(indent + 2))
78:             lines.append(f"{prefix}  false_expr:")
79:             lines.append(self.third_child._pretty(indent + 2))
80:             return "\n".join(lines)
81:         else:
82:             return f"{prefix}Value({self.value})"
83: 
84: 
85: class SyntaxTreeParser:
86:     """
87:     HLSL表达式语法树解析器
88:     负责将HLSL表达式字符串解析为SyntaxTreeNode组成的语法树
89:     支持: 类型转换、括号表达式、二元运算符、函数调用、变量引用
90:     """
91:     def __init__(self):
92:         self.operators = {
93:             '||': 1, '&&': 2,
94:             '==': 3, '!=': 3,
95:             '<': 4, '>': 4, '<=': 4, '>=': 4,
96:             '+': 5, '-': 5,
97:             '*': 6, '/': 6,
98:         }
99: 
100:     def parse(self, expr: str) -> SyntaxTreeNode:
101:         """
102:         解析HLSL表达式为语法树
103:         expr: HLSL表达式字符串
104:         返回: SyntaxTreeNode语法树根节点
105:         """
106:         expr = expr.strip()
107:         return self._parse_expression(expr)
108: 
109:     def _find_top_level_operator(self, expr: str) -> Optional[tuple]:
110:         """
111:         查找表达式中优先级最低的运算符(处于括号外的顶层运算符)
112:         用于实现运算符优先级解析
113:         expr: 表达式字符串
114:         返回: (位置, 运算符) 元组，或None
115: 
116:         运算符优先级(数字越小优先级越低):
117:         '||': 1, '&&': 2, '==': 3, '!=': 3,
118:         '<': 4, '>': 4, '<=': 4, '>=': 4,
119:         '+': 5, '-': 5, '*': 6, '/': 6
120: 
121:         规则: 找到优先级最低的运算符，如果有多个同优先级的运算符，返回最右边的那个
122:         """
123:         depth = 0
124:         candidates = []
125:         i = 0
126:         while i < len(expr):
127:             char = expr[i]
128:             if char == '(':
129:                 depth += 1
130:             elif char == ')':
131:                 depth -= 1
132:             elif depth == 0:
133:                 if i >= 1:
134:                     two_char = expr[i-1:i+1]
135:                     if two_char in self.operators:
136:                         candidates.append((i-1, two_char, self.operators[two_char]))
137:                         i += 1
138:                         continue
139:                 if char in self.operators:
140:                     candidates.append((i, char, self.operators[char]))
141:             i += 1
142: 
143:         if not candidates:
144:             return None
145: 
146:         min_prec = min(c[2] for c in candidates)
147:         rightmost = max(c[0] for c in candidates if c[2] == min_prec)
148:         for c in candidates:
149:             if c[0] == rightmost and c[2] == min_prec:
150:                 return (c[0], c[1])
151:         return None
152: 
153:     def _parse_expression(self, expr: str) -> SyntaxTreeNode:
154:         """
155:         将HLSL表达式字符串解析为语法树节点。
156: 
157:         解析顺序(从高优先级到低优先级):
158:         1. 类型转换: (float3x3)expr - 将表达式转换为指定类型
159:         2. 括号表达式: (expr) - 括号包围的表达式
160:         3. 三元运算符: a ? b : c - 条件表达式
161:         4. 二元运算符: + - * / == != < > <= >= && ||
162:         5. 函数调用: func(args) - 如normalize(), mul(), transpose()等
163:         6. 变量/常量值: 标识符或数字字面量
164:         """
165:         expr = expr.strip()
166:         if not expr:
167:             return SyntaxTreeNode('value', None)
168: 
169:         # =====================================================================
170:         # 第一步: 类型转换 (cast) - 匹配模式 (type)expression
171:         # 例如: (float3x3)World - 将4x4矩阵转换为3x3矩阵
172:         #       (float4)vec3 - 将vec3扩展为vec4
173:         # =====================================================================
174:         cast_match = re.match(r'\((\w+)\)\s*(.+)', expr, re.DOTALL)
175:         if cast_match:
176:             cast_type = cast_match.group(1)    # 转换目标类型，如float3x3
177:             rest = cast_match.group(2).strip()   # 类型声明之后的部分
178:             inner_node = self._parse_expression(rest)  # 递归解析内部表达式
179:             return SyntaxTreeNode('cast', cast_type, inner_node)
180: 
181:         # =====================================================================
182:         # 第二步: 括号表达式 - 检查是否被括号包围
183:         # 例如: (a + b) - 外层括号只是分组，不改变语义
184:         # 注意: 需要检查括号是否平衡，防止误匹配如 (a) + (b)
185:         # =====================================================================
186:         if expr.startswith('(') and expr.endswith(')'):
187:             inner = expr[1:-1].strip()
188:             # 遍历内部内容，检查括号是否平衡
189:             paren_depth = 0
190:             is_proper_paren = True
191:             for j, c in enumerate(inner):
192:                 if c == '(':
193:                     paren_depth += 1
194:                 elif c == ')':
195:                     paren_depth -= 1
196:                 # 如果在遍历过程中深度变为负数，说明括号不平衡
197:                 if paren_depth < 0:
198:                     is_proper_paren = False
199:                     break
200:             # 只有当内部括号都平衡时，才将外层括号视为分组
201:             if is_proper_paren:
202:                 return self._parse_expression(inner)
203: 
204:         # =====================================================================
205:         # 第三步: 三元运算符 - 匹配 a ? b : c 模式
206:         # 三元运算符优先级最低，在所有二元运算符之后处理
207:         # =====================================================================
208:         ternary_pos = -1
209:         depth = 0
210:         for i, char in enumerate(expr):
211:             if char == '(':
212:                 depth += 1
213:             elif char == ')':
214:                 depth -= 1
215:             elif char == '?' and depth == 0:
216:                 ternary_pos = i
217:                 break
218: 
219:         if ternary_pos >= 0:
220:             colon_pos = -1
221:             depth = 0
222:             for i in range(ternary_pos + 1, len(expr)):
223:                 char = expr[i]
224:                 if char == '(':
225:                     depth += 1
226:                 elif char == ')':
227:                     depth -= 1
228:                 elif char == ':' and depth == 0:
229:                     colon_pos = i
230:                     break
231: 
232:             if colon_pos >= 0:
233:                 cond_expr = expr[:ternary_pos].strip()
234:                 true_expr = expr[ternary_pos+1:colon_pos].strip()
235:                 false_expr = expr[colon_pos+1:].strip()
236:                 cond_node = self._parse_expression(cond_expr)
237:                 true_node = self._parse_expression(true_expr)
238:                 false_node = self._parse_expression(false_expr)
239:                 return SyntaxTreeNode('ternary', '?', cond_node, true_node, false_node)
240: 
241:         # =====================================================================
242:         # 第四步: 二元运算符 - 从右向左查找优先级最低的运算符
243:         # 支持: 逻辑或(||)、逻辑与(&&)、比较(== != < > <= >=)、
244:         #       算术(+ -)、乘除(* /)
245:         # =====================================================================
246:         op_info = self._find_top_level_operator(expr)
247:         if op_info:
248:             pos, op = op_info
249:             if op in ['||', '&&', '==', '!=', '<', '>', '<=', '>=', '+', '-', '*', '/']:
250:                 left_expr = expr[:pos].strip()
251:                 right_expr = expr[pos+len(op):].strip()
252:                 # 递归解析左右操作数
253:                 left_node = self._parse_expression(left_expr)
254:                 right_node = self._parse_expression(right_expr)
255:                 return SyntaxTreeNode('binary_op', op, left_node, right_node)
256: 
257:         # =====================================================================
258:         # 第五步: 函数调用 - 匹配函数名后跟括号
259:         # float[234]构造函数: float2(...), float3(...), float4(...)
260:         # 普通函数调用: normalize(...), mul(...), transpose(...)等
261:         # =====================================================================
262:         if re.match(r'float[234]\s*\(', expr):
263:             return self._parse_function_call(expr)
264: 
265:         if re.match(r'\w+\s*\(', expr):
266:             return self._parse_function_call(expr)
267: 
268:         # =====================================================================
269:         # 第六步: 变量/常量值 - 标识符、字符串或数字
270:         # 到达这里说明表达式不包含运算符和函数调用
271:         # =====================================================================
272:         return SyntaxTreeNode('value', expr)
273: 
274:     def _parse_function_call(self, expr: str) -> SyntaxTreeNode:
275:         """
276:         解析函数调用表达式
277:         处理类型转换和函数调用两种情况
278:         expr: 函数调用表达式，如 "(float3x3)World" 或 "mul(a, b)"
279:         """
280:         expr = expr.strip()
281:         if expr.startswith('('):
282:             match = re.match(r'\((\w+)\)\s*(.+)', expr, re.DOTALL)
283:             if match:
284:                 cast_type = match.group(1)
285:                 rest = match.group(2).strip()
286:                 inner_node = self._parse_expression(rest)
287:                 if inner_node.node_type == 'value':
288:                     return inner_node
289:                 return SyntaxTreeNode('cast', cast_type, inner_node)
290: 
291:         match = re.match(r'^(\w+)\s*\(', expr)
292:         if not match:
293:             return SyntaxTreeNode('value', expr)
294: 
295:         func_name = match.group(1)
296: 
297:         depth = 0
298:         paren_start = -1
299:         for i, char in enumerate(expr):
300:             if char == '(':
301:                 depth += 1
302:                 if depth == 1:
303:                     paren_start = i
304:             elif char == ')':
305:                 depth -= 1
306:                 if depth == 0:
307:                     args_str = expr[paren_start+1:i]
308:                     if func_name in ['transpose', 'normalize', 'length', 'abs', 'sin', 'cos', 'tan']:
309:                         inner_node = self._parse_expression(args_str.strip())
310:                         return SyntaxTreeNode('function', func_name, args=[inner_node])
311:                     elif func_name in ['mul', 'reflect', 'pow', 'max', 'min', 'dot', 'float2', 'float3', 'float4']:
312:                         args = self._split_args(args_str)
313:                         arg_nodes = [self._parse_expression(arg.strip()) for arg in args]
314:                         return SyntaxTreeNode('function', func_name, args=arg_nodes)
315:                     args = self._split_args(args_str)
316:                     arg_nodes = [self._parse_expression(arg.strip()) for arg in args]
317:                     return SyntaxTreeNode('function', func_name, args=arg_nodes)
318: 
319:         return SyntaxTreeNode('value', expr)
320: 
321:     def _split_args(self, args_str: str) -> List[str]:
322:         """
323:         分割函数参数字符串，处理嵌套括号
324:         args_str: 参数字符串，如 "a, b, float3(1,2,3)"
325:         返回: 参数列表
326:         """
327:         if not args_str.strip():
328:             return []
329:         args = []
330:         depth = 0
331:         current = ''
332:         for char in args_str:
333:             if char == '(':
334:                 depth += 1
335:                 current += char
336:             elif char == ')':
337:                 depth -= 1
338:                 current += char
339:             elif char == ',' and depth == 0:
340:                 args.append(current.strip())
341:                 current = ''
342:             else:
343:                 current += char
344:         if current.strip():
345:             args.append(current.strip())
346:         return args
347: 
348: 
349: @dataclass
350: class ShaderVariable:
351:     """着色器变量定义"""
352:     name: str       # 变量名
353:     type: str       # 变量类型
354:     value: Any      # 变量值
355: 
356: 
357: @dataclass
358: class FieldDefinition:
359:     """结构体或cbuffer的字段定义"""
360:     field_type: str      # 字段类型，如 float3, float4x4
361:     name: str           # 字段名
362:     semantic: str       # 语义名称，如 POSITION, NORMAL
363:     data: List[Any] = None  # 字段数据值
364: 
365: @dataclass
366: class StructDefinition:
367:     """HLSL结构体定义"""
368:     name: str                     # 结构体名称
369:     fields: List[FieldDefinition]  # 结构体字段列表
370: 
371: @dataclass
372: class CbufferDefinition:
373:     """HLSL常量缓冲区定义"""
374:     name: str                     # cbuffer名称
375:     fields: List[FieldDefinition]  # cbuffer字段列表
376: 
377: 
378: class HLSLInterpreter:
379:     """
380:     HLSL解释器 - 解析和执行HLSL着色器代码
381:     支持: 结构体定义、cbuffer定义、函数解析、表达式求值
382:     """
383: 
384:     def __init__(self, log_to_file: bool = True, log_file_path: str = "hlsl_interpreter.log"):
385:         self.structs: Dict[str, StructDefinition] = {}      # 解析的结构体定义
386:         self.cbuffers: Dict[str, CbufferDefinition] = {}    # 解析的cbuffer定义
387:         self.variables: Dict[str, Any] = {}                 # 全局变量
388:         self.debug = True                                   # 调试模式开关
389:         self.printSyntaxTree = False                        # 打印语法树开关
390:         self.syntax_parser = SyntaxTreeParser()             # 语法树解析器
391:         self.log_to_file = log_to_file                      # 是否输出到文件
392:         self.log_file_path = log_file_path                  # 日志文件路径
393: 
394:     def log_output(self, *args, **kwargs):
395:         """输出到stdout和日志文件"""
396:         msg = ' '.join(str(arg) for arg in args)
397:         print(*args, **kwargs)
398:         if self.log_to_file and self.log_file_path:
399:             with open(self.log_file_path, 'a', encoding='utf-8') as f:
400:                 f.write(msg + '\n')
401: 
402:     def debug_print(self, msg: str):
403:         """调试打印"""
404:         if self.debug:
405:             self.log_output(msg)
406: 
407:     def _format_float(self, val):
408:         """
409:         格式化浮点数输出
410:         val: 值
411:         返回: 格式化后的字符串(保留4位小数)
412:         """
413:         if isinstance(val, float):
414:             return f"{val:.4f}"
415:         if isinstance(val, list):
416:             if val and isinstance(val[0], list):
417:                 return self._format_matrix(val)
418:             return [self._format_float(v) for v in val]
419:         return val
420: 
421:     def _format_matrix(self, val):
422:         """
423:         格式化矩阵输出
424:         val: 矩阵(二维列表)
425:         返回: 格式化后的矩阵字符串
426:         """
427:         if not val or not isinstance(val[0], list):
428:             return str(val)
429:         formatted = [[self._format_float(v) for v in row] for row in val]
430:         col_widths = [0] * len(formatted[0])
431:         for row in formatted:
432:             for j, cell in enumerate(row):
433:                 col_widths[j] = max(col_widths[j], len(cell))
434:         lines = []
435:         for row in formatted:
436:             cells = [cell.rjust(col_widths[j]) for j, cell in enumerate(row)]
437:             lines.append("[" + " ".join(cells) + "]")
438:         return "\n".join(lines)
439: 
440:     def _format_value(self, val):
441:         """格式化值输出(矩阵或标量/向量)"""
442:         if isinstance(val, list) and val and isinstance(val[0], list):
443:             return self._format_matrix(val)
444:         return self._format_float(val)
445: 
446:     def _format_msg(self, *args):
447:         """格式化多个值用于调试输出"""
448:         formatted = []
449:         for arg in args:
450:             formatted.append(self._format_float(arg))
451:         return formatted
452: 
453:     def load_json(self, filepath: str):
454:         """从JSON文件加载数据"""
455:         with open(filepath, 'r') as f:
456:             data = json.load(f)
457:         return data
458: 
459:     def load_csv(self, filepath: str) -> List[List[str]]:
460:         """从CSV文件加载数据，返回二维列表"""
461:         rows = []
462:         with open(filepath, 'r') as f:
463:             reader = csv.reader(f)
464:             for row in reader:
465:                 rows.append(row)
466:         return rows
467: 
468:     def get_type_size(self, field_type: str) -> int:
469:         """
470:         获取HLSL类型的大小(字节数)
471:         field_type: HLSL类型名，如 float4x4, float3, int
472:         返回: 类型占用的字节数
473:         """
474:         if 'float4x4' in field_type:
475:             return 64  # 4x4矩阵 = 16 floats * 4 bytes
476:         elif 'float3x3' in field_type:
477:             return 36  # 3x3矩阵 = 9 floats * 4 bytes
478:         elif 'float4' in field_type:
479:             return 16  # 4 floats * 4 bytes
480:         elif 'float3' in field_type:
481:             return 12  # 3 floats * 4 bytes
482:         elif 'float2' in field_type:
483:             return 8   # 2 floats * 4 bytes
484:         elif 'float' in field_type:
485:             return 4   # 1 float * 4 bytes
486:         elif 'uint4' in field_type:
487:             return 16
488:         elif 'uint3' in field_type:
489:             return 12
490:         elif 'uint2' in field_type:
491:             return 8
492:         elif 'uint' in field_type:
493:             return 4
494:         elif 'int4' in field_type:
495:             return 16
496:         elif 'int3' in field_type:
497:             return 12
498:         elif 'int2' in field_type:
499:             return 8
500:         elif 'int' in field_type:
501:             return 4
502:         elif 'bool' in field_type:
503:             return 4
504:         return 0
505: 
506:     def parse_value_by_type(self, value_str: str, field_type: str) -> Any:
507:         """
508:         根据类型解析字符串值为对应类型的Python对象
509:         value_str: 值的字符串表示
510:         field_type: HLSL类型名
511:         返回: 解析后的值
512:         """
513:         value_str = value_str.strip().strip('"')
514:         if 'float4x4' in field_type:
515:             parts = value_str.split(',')
516:             if len(parts) >= 16:
517:                 matrix = []
518:                 for i in range(4):
519:                     row = [float(parts[j]) for j in range(i*4, i*4+4)]
520:                     matrix.append(row)
521:                 return matrix
522:         elif 'float3x3' in field_type:
523:             parts = value_str.split(',')
524:             if len(parts) >= 9:
525:                 matrix = []
526:                 for i in range(3):
527:                     row = [float(parts[j]) for j in range(i*3, i*3+3)]
528:                     matrix.append(row)
529:                 return matrix
530:         elif 'float4' in field_type:
531:             parts = value_str.split(',')
532:             return [float(p) for p in parts[:4]]
533:         elif 'float3' in field_type:
534:             parts = value_str.split(',')
535:             return [float(p) for p in parts[:3]]
536:         elif 'float2' in field_type:
537:             parts = value_str.split(',')
538:             return [float(p) for p in parts[:2]]
539:         elif 'uint4' in field_type:
540:             parts = value_str.split(',')
541:             return [int(p) for p in parts[:4]]
542:         elif 'uint3' in field_type:
543:             parts = value_str.split(',')
544:             return [int(p) for p in parts[:3]]
545:         elif 'uint2' in field_type:
546:             parts = value_str.split(',')
547:             return [int(p) for p in parts[:2]]
548:         elif 'uint' in field_type:
549:             return int(value_str)
550:         elif 'int4' in field_type:
551:             parts = value_str.split(',')
552:             return [int(p) for p in parts[:4]]
553:         elif 'int3' in field_type:
554:             parts = value_str.split(',')
555:             return [int(p) for p in parts[:3]]
556:         elif 'int2' in field_type:
557:             parts = value_str.split(',')
558:             return [int(p) for p in parts[:2]]
559:         elif 'int' in field_type:
560:             return int(value_str)
561:         elif 'bool' in field_type:
562:             return value_str.lower() in ('true', '1', 'yes')
563:         try:
564:             return float(value_str)
565:         except:
566:             return value_str
567: 
568:     def parse_type(self, type_str: str) -> str:
569:         """
570:         解析HLSL类型字符串为标准类型名
571:         type_str: 类型字符串，如 "float4x4", "float3", "int2"
572:         返回: 标准类型名
573:         """
574:         type_str = type_str.strip()
575:         if type_str in DATA_TYPE_LIST:
576:             return type_str
577:         if type_str.startswith('float'):
578:             if 'x3' in type_str:
579:                 return 'float3x3'
580:             elif 'x4' in type_str:
581:                 return 'float4x4'
582:             elif type_str == 'float':
583:                 return 'float'
584:             return 'float'
585:         elif type_str.startswith('int'):
586:             if type_str == 'int':
587:                 return 'int'
588:             elif '2' in type_str:
589:                 return 'int2'
590:             elif '3' in type_str:
591:                 return 'int3'
592:             elif '4' in type_str:
593:                 return 'int4'
594:             return 'int'
595:         elif type_str.startswith('uint'):
596:             if type_str == 'uint':
597:                 return 'uint'
598:             elif '2' in type_str:
599:                 return 'uint2'
600:             elif '3' in type_str:
601:                 return 'uint3'
602:             elif '4' in type_str:
603:                 return 'uint4'
604:             return 'uint'
605:         elif type_str.startswith('bool'):
606:             return 'bool'
607:         return type_str
608: 
609:     def parse_struct(self, code: str) -> StructDefinition:
610:         """
611:         解析HLSL结构体定义
612:         code: 结构体代码，如 "struct VS_INPUT { float3 Pos : POSITION; }"
613:         返回: StructDefinition对象
614:         """
615:         match = re.search(r'struct\s+(\w+)\s*\{([^}]+)\}', code)
616:         if not match:
617:             return None
618:         name = match.group(1)
619:         fields_str = match.group(2)
620:         fields = []
621:         for line in fields_str.split(';'):
622:             line = line.strip()
623:             if not line:
624:                 continue
625:             parts = line.split(':')
626:             if len(parts) == 2:
627:                 type_and_name = parts[0].strip().split()
628:                 semantic = parts[1].strip()
629:                 if len(type_and_name) >= 2:
630:                     field_type = type_and_name[0]
631:                     field_name = type_and_name[-1]
632:                 else:
633:                     field_type = type_and_name[0]
634:                     field_name = ''
635:                 fields.append(FieldDefinition(field_type, field_name, semantic))
636:         return StructDefinition(name, fields)
637: 
638:     def parse_cbuffer(self, code: str) -> CbufferDefinition:
639:         """
640:         解析HLSL常量缓冲区定义
641:         code: cbuffer代码
642:         返回: CbufferDefinition对象
643:         """
644:         match = re.search(r'cbuffer\s+(\w+)\s*:.*?\{([^}]+)\}', code, re.DOTALL)
645:         if not match:
646:             return None
647:         name = match.group(1)
648:         fields = []
649:         lines = code[match.start():match.end()].split('\n')[1:]
650:         for line in lines:
651:             line = line.strip().rstrip(';')
652:             if not line or line.startswith('}'):
653:                 continue
654:             if any(t in line for t in DATA_TYPE_LIST):
655:                 parts = line.split()
656:                 if len(parts) >= 2:
657:                     field_type = parts[0]
658:                     field_name = parts[1]
659:                     fields.append(FieldDefinition(field_type, field_name, ''))
660:         return CbufferDefinition(name, fields)
661: 
662:     def parse_function(self, code: str) -> tuple:
663:         """
664:         解析HLSL函数定义
665:         code: 函数代码，如 "float4 main(VS_INPUT input) { ... }"
666:         返回: (返回类型, 函数名, 参数字典, 函数体) 元组
667:         """
668:         match = re.search(r'(\w+)\s+(\w+)\s*\(([^)]*)\)\s*\{([^}]+(?:\{[^}]*\}[^}]*)*)\}', code, re.DOTALL)
669:         if not match:
670:             return None, None, None, None
671:         ret_type = match.group(1)
672:         func_name = match.group(2)
673:         params_str = match.group(3)
674:         body = match.group(4)
675:         params = {}
676:         if params_str.strip():
677:             for param in params_str.split(','):
678:                 param = param.strip()
679:                 parts = param.split()
680:                 if len(parts) >= 2:
681:                     param_type = parts[0]
682:                     param_name = parts[1]
683:                     params[param_name] = param_type
684:         return ret_type, func_name, params, body
685: 
686:     def execute_unary_op(self, op: str, val: Any) -> Any:
687:         """
688:         执行一元运算符
689:         op: 运算符 '-' 或 '!'
690:         val: 操作数
691:         """
692:         result = val
693:         if op == '-':
694:             if isinstance(val, (int, float)):
695:                 result = -val
696:             elif isinstance(val, list):
697:                 result = [-v for v in val]
698:         elif op == '!':
699:             if isinstance(val, bool):
700:                 result = not val
701:             result = not bool(val)
702:         self.debug_print(f"[UNARY OP] operand={self._format_value(val)}, op={op}, result={self._format_value(result)}")
703:         return result
704: 
705:     def execute_binary_op(self, op: str, left: Any, right: Any) -> Any:
706:         """
707:         执行二元运算符
708:         op: 运算符 '+', '-', '*', '/', '.'
709:         left, right: 左右操作数
710:         """
711:         if left is None or right is None:
712:             result = None
713:             self.debug_print(f"[BINARY OP] left={self._format_value(left)}, right={self._format_value(right)}, op={op}, result={self._format_value(result)}")
714:             return None
715:         if op == '+':
716:             if isinstance(left, list) and isinstance(right, list):
717:                 result = [l + r for l, r in zip(left, right)]
718:             elif isinstance(left, list) and isinstance(right, (int, float)):
719:                 result = [v + right for v in left]
720:             elif isinstance(right, list) and isinstance(left, (int, float)):
721:                 result = [left + v for v in right]
722:             else:
723:                 result = left + right
724:         elif op == '-':
725:             if isinstance(left, list) and isinstance(right, list):
726:                 result = [l - r for l, r in zip(left, right)]
727:             elif isinstance(left, list) and isinstance(right, (int, float)):
728:                 result = [v - right for v in left]
729:             elif isinstance(right, list) and isinstance(left, (int, float)):
730:                 result = [left - v for v in right]
731:             else:
732:                 result = left - right
733:         elif op == '*':
734:             if isinstance(left, list) and isinstance(right, (int, float)):
735:                 result = [v * right for v in left]
736:             elif isinstance(right, list) and isinstance(left, (int, float)):
737:                 result = [v * left for v in right]
738:             elif isinstance(left, list) and isinstance(right, list):
739:                 result = [l * r for l, r in zip(left, right)]
740:             else:
741:                 result = left * right
742:         elif op == '/':
743:             if isinstance(left, list) and isinstance(right, (int, float)):
744:                 result = [v / right for v in left]
745:             elif isinstance(left, list) and isinstance(right, list):
746:                 result = [l / r for l, r in zip(left, right)]
747:             else:
748:                 result = left / right
749:         elif op == '.':
750:             result = (left, right)
751:         elif op == '==':
752:             result = left == right
753:         elif op == '!=':
754:             result = left != right
755:         elif op == '<':
756:             result = left < right
757:         elif op == '>':
758:             result = left > right
759:         elif op == '<=':
760:             result = left <= right
761:         elif op == '>=':
762:             result = left >= right
763:         elif op == '&&':
764:             result = bool(left and right)
765:         elif op == '||':
766:             result = bool(left or right)
767:         else:
768:             result = None
769:         self.debug_print(f"[BINARY OP] left={self._format_float(left)}, right={self._format_float(right)}, op={op}, result={self._format_float(result)}")
770:         return result
771: 
772:     def transpose_matrix(self, m: List[List[float]]) -> List[List[float]]:
773:         """
774:         矩阵转置
775:         m: 输入矩阵(4x4或3x3)
776:         返回: 转置后的矩阵
777:         """
778:         if len(m) == 4:
779:             return [[m[j][i] for j in range(4)] for i in range(4)]
780:         elif len(m) == 3:
781:             return [[m[j][i] for j in range(3)] for i in range(3)]
782:         return m
783: 
784:     def mul_matrix_vector(self, m: List[List[float]], v: List[float]) -> List[float]:
785:         """
786:         矩阵乘向量: result = m * v
787:         m: 4x4或3x3矩阵
788:         v: 向量(4维或3维)
789:         返回: 计算后的向量
790:         """
791:         if not v or any(x is None for x in v):
792:             return [0, 0, 0, 0]
793:         if not m:
794:             return [0, 0, 0, 0]
795:         num_cols = len(m[0]) if m else 0
796:         result = []
797:         for j in range(num_cols):
798:             s = sum(v[i] * m[i][j] for i in range(len(v)))
799:             result.append(s)
800:         return result
801: 
802:     def mul_matrix_matrix(self, a: List[List[float]], b: List[List[float]]) -> List[List[float]]:
803:         """
804:         矩阵乘法: result = a * b
805:         a, b: n x n 方阵
806:         返回: 结果矩阵
807:         """
808:         n = len(a)
809:         result = [[0.0] * n for _ in range(n)]
810:         for i in range(n):
811:             for j in range(n):
812:                 for k in range(n):
813:                     result[i][j] += a[i][k] * b[k][j]
814:         return result
815: 
816:     def length_vec(self, v: List[float]) -> float:
817:         """计算向量长度(模)"""
818:         return math.sqrt(sum(x * x for x in v))
819: 
820:     def normalize_vec(self, v: List[float]) -> List[float]:
821:         """
822:         向量归一化
823:         v: 输入向量
824:         返回: 归一化后的向量，长度为1
825:         """
826:         l = self.length_vec(v)
827:         if l < 1e-8:
828:             return v
829:         return [x / l for x in v]
830: 
831:     def dot_product(self, a: List[float], b: List[float]) -> float:
832:         """
833:         向量点积: a · b
834:         a, b: 同维度向量
835:         返回: 点积结果
836:         """
837:         if not isinstance(a, list) or not isinstance(b, list):
838:             return 0.0
839:         if len(a) != len(b):
840:             return 0.0
841:         return sum(x * y for x, y in zip(a, b))
842: 
843:     def reflect_vec(self, I: List[float], N: List[float]) -> List[float]:
844:         """
845:         计算反射向量 R = I - 2 * (N · I) * N
846:         I: 入射向量
847:         N: 法线向量(需要归一化)
848:         返回: 反射向量
849:         """
850:         if not isinstance(I, list) or not isinstance(N, list):
851:             return [0, 0, 0]
852:         dot = self.dot_product(N, I)
853:         result = []
854:         for i_val, n_val in zip(I, N):
855:             result.append(i_val - 2 * n_val * dot)
856:         return result
857: 
858:     def find_top_level_comma(self, expr: str) -> int:
859:         """
860:         查找表达式顶层逗号(不在括号内)
861:         用于分割函数多参数
862:         expr: 表达式字符串
863:         返回: 逗号位置索引，或-1表示未找到
864:         """
865:         depth = 0
866:         for i, char in enumerate(expr):
867:             if char == '(':
868:                 depth += 1
869:             elif char == ')':
870:                 depth -= 1
871:             elif char == ',' and depth == 0:
872:                 return i
873:         return -1
874: 
875:     def evaluate_expression(self, expr: str, local_vars: Dict[str, Any]) -> Any:
876:         """
877:         对HLSL表达式求值
878:         expr: 表达式字符串
879:         local_vars: 局部变量字典
880:         返回: 求值结果
881:         """
882:         expr = expr.strip()
883:         if not expr:
884:             return None
885: 
886:         if expr == 'return':
887:             return None
888: 
889:         if expr.startswith('return '):
890:             return self.evaluate_expression(expr[7:], local_vars)
891: 
892:         # 使用语法树解析器处理所有表达式（包括三元运算符）
893:         tree = self.syntax_parser.parse(expr)
894:         if self.printSyntaxTree == True:
895:             self.debug_print(f"[SYNTAX TREE]\n{tree}")
896: 
897:         return self.evaluate_syntax_tree(tree, local_vars)
898: 
899:     def evaluate_syntax_tree(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
900:         """
901:         对语法树节点求值
902:         node: 语法树节点
903:         local_vars: 局部变量字典
904:         返回: 求值结果
905:         """
906:         if node is None:
907:             return None
908: 
909:         if node.node_type == 'value':
910:             if node.value is None:
911:                 return None
912:             return self.get_value(node.value, local_vars)
913: 
914:         elif node.node_type == 'binary_op':
915:             left = self.evaluate_syntax_tree(node.left, local_vars)
916:             right = self.evaluate_syntax_tree(node.right, local_vars)
917:             return self.execute_binary_op(node.value, left, right)
918: 
919:         elif node.node_type == 'unary_op':
920:             child = self.evaluate_syntax_tree(node.left, local_vars)
921:             return self.execute_unary_op(node.value, child)
922: 
923:         elif node.node_type == 'function':
924:             return self.execute_function_node(node, local_vars)
925: 
926:         elif node.node_type == 'ternary':
927:             cond = self.evaluate_syntax_tree(node.left, local_vars)
928:             if cond:
929:                 return self.evaluate_syntax_tree(node.right, local_vars)
930:             else:
931:                 return self.evaluate_syntax_tree(node.third_child, local_vars)
932: 
933:         elif node.node_type == 'cast':
934:             inner = self.evaluate_syntax_tree(node.left, local_vars)
935:             if inner is None:
936:                 return None
937:             cast_type = node.value
938:             # float3x3转换: 从4x4矩阵提取前3x3
939:             if cast_type == 'float3x3' and isinstance(inner, list) and len(inner) == 4:
940:                 return [row[:3] for row in inner[:3]]
941:             # float2x2转换: 从4x4矩阵提取前2x2
942:             if cast_type == 'float2x2' and isinstance(inner, list) and len(inner) == 4:
943:                 return [row[:2] for row in inner[:2]]
944:             # float2x2转换: 从3x3矩阵提取前2x2
945:             if cast_type == 'float2x2' and isinstance(inner, list) and len(inner) == 3:
946:                 return [row[:2] for row in inner[:2]]
947:             return inner
948: 
949:         return None
950: 
951:     def execute_function_node(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
952:         """
953:         执行函数调用语法树节点
954:         node: 函数调用节点
955:         local_vars: 局部变量字典
956:         返回: 函数执行结果
957:         """
958:         func_name = node.value
959:         args = node.args
960: 
961:         # transpose: 矩阵转置函数
962:         # 计算矩阵的转置，将行列互换
963:         if func_name == 'transpose':
964:             if len(args) != 1:
965:                 self.debug_print(f"[ERROR] transpose requires 1 arg, got {len(args)} at line {node.line_number}")
966:                 return None
967:             val = self.evaluate_syntax_tree(args[0], local_vars)
968:             if val is None:
969:                 return None
970:             result = self.transpose_matrix(val)
971:             self.debug_print(f"[FUNC] transpose(\n{self._format_value(val)}) =\n{self._format_value(result)}")
972:             return result
973: 
974:         # normalize: 向量归一化函数
975:         # 将输入向量缩放到单位长度，即长度为1
976:         elif func_name == 'normalize':
977:             if len(args) != 1:
978:                 self.debug_print(f"[ERROR] normalize requires 1 arg, got {len(args)} at line {node.line_number}")
979:                 return None
980:             val = self.evaluate_syntax_tree(args[0], local_vars)
981:             if val is None:
982:                 return None
983:             if isinstance(val, list):
984:                 result = self.normalize_vec(val)
985:                 self.debug_print(f"[FUNC] normalize({self._format_float(val)}) = {self._format_float(result)}")
986:                 return result
987:             return val
988: 
989:         # length: 向量长度函数
990:         # 计算向量的欧几里得长度(模)
991:         elif func_name == 'length':
992:             if len(args) != 1:
993:                 self.debug_print(f"[ERROR] length requires 1 arg, got {len(args)} at line {node.line_number}")
994:                 return None
995:             val = self.evaluate_syntax_tree(args[0], local_vars)
996:             if val is None:
997:                 return None
998:             result = self.length_vec(val)
999:             self.debug_print(f"[FUNC] length({self._format_float(val)}) = {self._format_float(result)}")
1000:             return result
1001: 
1002:         # dot: 向量点积函数
1003:         # 计算两个向量的点积，结果为标量
1004:         elif func_name == 'dot':
1005:             if len(args) != 2:
1006:                 self.debug_print(f"[ERROR] dot requires 2 args, got {len(args)} at line {node.line_number}")
1007:                 return None
1008:             a = self.evaluate_syntax_tree(args[0], local_vars)
1009:             b = self.evaluate_syntax_tree(args[1], local_vars)
1010:             if a is None or b is None:
1011:                 return None
1012:             result = self.dot_product(a, b)
1013:             self.debug_print(f"[FUNC] dot({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
1014:             return result
1015: 
1016:         # reflect: 反射向量函数
1017:         # 计算光线关于法向量的反射向量，公式: R = I - 2 * N * dot(I, N)
1018:         elif func_name == 'reflect':
1019:             if len(args) != 2:
1020:                 self.debug_print(f"[ERROR] reflect requires 2 args, got {len(args)} at line {node.line_number}")
1021:                 return None
1022:             I = self.evaluate_syntax_tree(args[0], local_vars)
1023:             N = self.evaluate_syntax_tree(args[1], local_vars)
1024:             if I is None or N is None:
1025:                 return None
1026:             result = self.reflect_vec(I, N)
1027:             self.debug_print(f"[FUNC] reflect({self._format_float(I)}, {self._format_float(N)}) = {self._format_float(result)}")
1028:             return result
1029: 
1030:         # max: 最大值函数
1031:         # 返回两个值中的较大者
1032:         elif func_name == 'max':
1033:             if len(args) != 2:
1034:                 self.debug_print(f"[ERROR] max requires 2 args, got {len(args)} at line {node.line_number}")
1035:                 return None
1036:             a = self.evaluate_syntax_tree(args[0], local_vars)
1037:             b = self.evaluate_syntax_tree(args[1], local_vars)
1038:             if a is None or b is None:
1039:                 return None
1040:             result = max(a, b)
1041:             self.debug_print(f"[FUNC] max({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
1042:             return result
1043: 
1044:         # min: 最小值函数
1045:         # 返回两个值中的较小者
1046:         elif func_name == 'min':
1047:             if len(args) != 2:
1048:                 self.debug_print(f"[ERROR] min requires 2 args, got {len(args)} at line {node.line_number}")
1049:                 return None
1050:             a = self.evaluate_syntax_tree(args[0], local_vars)
1051:             b = self.evaluate_syntax_tree(args[1], local_vars)
1052:             if a is None or b is None:
1053:                 return None
1054:             result = min(a, b)
1055:             self.debug_print(f"[FUNC] min({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
1056:             return result
1057: 
1058:         # pow: 幂函数
1059:         # 计算base的exp次幂，即 base ^ exp
1060:         elif func_name == 'pow':
1061:             if len(args) != 2:
1062:                 self.debug_print(f"[ERROR] pow requires 2 args, got {len(args)} at line {node.line_number}")
1063:                 return None
1064:             base = self.evaluate_syntax_tree(args[0], local_vars)
1065:             exp = self.evaluate_syntax_tree(args[1], local_vars)
1066:             if base is None or exp is None:
1067:                 return None
1068:             result = math.pow(base, exp)
1069:             self.debug_print(f"[FUNC] pow({self._format_float(base)}, {self._format_float(exp)}) = {self._format_float(result)}")
1070:             return result
1071: 
1072:         # abs: 绝对值函数
1073:         # 返回数值的绝对值，对列表则对每个元素取绝对值
1074:         elif func_name == 'abs':
1075:             if len(args) != 1:
1076:                 self.debug_print(f"[ERROR] abs requires 1 arg, got {len(args)} at line {node.line_number}")
1077:                 return None
1078:             val = self.evaluate_syntax_tree(args[0], local_vars)
1079:             if val is None:
1080:                 return None
1081:             if isinstance(val, list):
1082:                 result = [abs(v) for v in val]
1083:             else:
1084:                 result = abs(val)
1085:             self.debug_print(f"[FUNC] abs({self._format_float(val)}) = {self._format_float(result)}")
1086:             return result
1087: 
1088:         # sin: 正弦函数
1089:         # 计算弧度的正弦值，对列表则对每个元素计算
1090:         elif func_name == 'sin':
1091:             if len(args) != 1:
1092:                 self.debug_print(f"[ERROR] sin requires 1 arg, got {len(args)} at line {node.line_number}")
1093:                 return None
1094:             val = self.evaluate_syntax_tree(args[0], local_vars)
1095:             if val is None:
1096:                 return None
1097:             if isinstance(val, list):
1098:                 result = [math.sin(v) for v in val]
1099:             else:
1100:                 result = math.sin(val)
1101:             self.debug_print(f"[FUNC] sin({self._format_float(val)}) = {self._format_float(result)}")
1102:             return result
1103: 
1104:         # cos: 余弦函数
1105:         # 计算弧度的余弦值，对列表则对每个元素计算
1106:         elif func_name == 'cos':
1107:             if len(args) != 1:
1108:                 self.debug_print(f"[ERROR] cos requires 1 arg, got {len(args)} at line {node.line_number}")
1109:                 return None
1110:             val = self.evaluate_syntax_tree(args[0], local_vars)
1111:             if val is None:
1112:                 return None
1113:             if isinstance(val, list):
1114:                 result = [math.cos(v) for v in val]
1115:             else:
1116:                 result = math.cos(val)
1117:             self.debug_print(f"[FUNC] cos({self._format_float(val)}) = {self._format_float(result)}")
1118:             return result
1119: 
1120:         # mul: 矩阵乘法函数
1121:         # 执行4x4或3x3矩阵乘法运算
1122:         elif func_name == 'mul':
1123:             if len(args) != 2:
1124:                 self.debug_print(f"[ERROR] mul requires 2 args, got {len(args)} at line {node.line_number}")
1125:                 return None
1126:             left = self.evaluate_syntax_tree(args[0], local_vars)
1127:             right = self.evaluate_syntax_tree(args[1], local_vars)
1128:             if left is None or right is None:
1129:                 return None
1130:             if isinstance(left, list) and isinstance(right, list):
1131:                 if len(left) == 4 and len(right) == 4:
1132:                     result = self.mul_matrix_vector(right, left)
1133:                     self.debug_print(f"[FUNC] mul(\nleft={self._format_value(left)},\nright={self._format_value(right)}) =\n{self._format_value(result)}")
1134:                     return result
1135:                 elif len(left) == 3 and len(right) == 3:
1136:                     result = self.mul_matrix_vector(right, left)
1137:                     self.debug_print(f"[FUNC] mul(\nleft={self._format_value(left)},\nright={self._format_value(right)}) =\n{self._format_value(result)}")
1138:                     return result
1139:             return None
1140: 
1141:         # float2/float3/float4: 向量构造函数
1142:         # 将参数展平合并为指定长度的向量
1143:         elif func_name in ['float2', 'float3', 'float4']:
1144:             # 向量构造函数: 将参数展平合并
1145:             result = []
1146:             for arg in args:
1147:                 val = self.evaluate_syntax_tree(arg, local_vars)
1148:                 if isinstance(val, list):
1149:                     result.extend(val)
1150:                 else:
1151:                     result.append(val)
1152:             self.debug_print(f"[FUNC] {func_name}(args={self._format_float(args)}) = {self._format_float(result)}")
1153:             return result
1154: 
1155:         return None
1156: 
1157:     def apply_swizzle(self, obj: Any, swizzle: str) -> Any:
1158:         """
1159:         对向量应用swizzle操作
1160:         obj: 向量对象(列表)
1161:         swizzle: swizzle模式字符串，如 'xyz', 'xxx', 'xxyy', 'xz' 等
1162:         返回: 应用swizzle后的结果
1163:         """
1164:         if obj is None:
1165:             return None
1166: 
1167:         if not isinstance(obj, list):
1168:             if swizzle == 'x':
1169:                 return obj
1170:             return None
1171: 
1172:         valid_chars = {'x': 0, 'y': 1, 'z': 2, 'w': 3}
1173:         result = []
1174:         for c in swizzle:
1175:             if c.lower() in valid_chars:
1176:                 idx = valid_chars[c.lower()]
1177:                 if idx < len(obj):
1178:                     result.append(obj[idx])
1179:                 else:
1180:                     result.append(0)
1181:             elif c in 'rgb':
1182:                 idx = {'r': 0, 'g': 1, 'b': 2}[c]
1183:                 if idx < len(obj):
1184:                     result.append(obj[idx])
1185:                 else:
1186:                     result.append(0)
1187: 
1188:         if len(result) == 1:
1189:             return result[0]
1190: 
1191:         numeric_types = (int, float)
1192:         if all(isinstance(v, numeric_types) for v in result):
1193:             if all(isinstance(v, int) for v in result):
1194:                 return [int(v) for v in result]
1195:             return result
1196: 
1197:         return result
1198: 
1199:     def get_value(self, name: str, local_vars: Dict[str, Any]) -> Any:
1200:         """
1201:         获取变量或常量的值
1202:         name: 变量名/常量名，支持结构体字段访问(如 input.Pos)
1203:         local_vars: 局部变量字典
1204:         返回: 变量值，如果未找到返回0.0
1205:         """
1206:         name = name.strip()
1207: 
1208:         # 处理布尔常量
1209:         if name == 'true':
1210:             return True
1211:         if name == 'false':
1212:             return False
1213: 
1214:         # 尝试解析为数字
1215:         try:
1216:             return float(name)
1217:         except ValueError:
1218:             pass
1219: 
1220:         # 检查是否包含swizzle操作 (如 LightPos.xyz, LightPos.xxx, input.Pos.xy)
1221:         if '.' in name:
1222:             parts = name.split('.')
1223:             if len(parts) >= 2:
1224:                 base_name = parts[0]
1225:                 swizzle_str = parts[1]
1226: 
1227:                 # 判断是否为swizzle模式（全是xyzwrgb组成的字符串）
1228:                 if swizzle_str and all(c in 'xyzwrgb' for c in swizzle_str.lower()):
1229:                     obj = local_vars.get(base_name)
1230:                     if obj is None:
1231:                         obj = self.variables.get(base_name)
1232:                     if obj is not None:
1233:                         return self.apply_swizzle(obj, swizzle_str)
1234: 
1235:                     # 尝试从cbuffer获取
1236:                     for cb_name, cb_def in self.cbuffers.items():
1237:                         if isinstance(cb_def, CbufferDefinition):
1238:                             for field in cb_def.fields:
1239:                                 if field.name == base_name:
1240:                                     if field.data is not None:
1241:                                         return self.apply_swizzle(field.data, swizzle_str)
1242:                                     return 0
1243: 
1244:                     # 检查是否在output对象中
1245:                     if base_name in local_vars:
1246:                         obj = local_vars[base_name]
1247:                         if isinstance(obj, dict):
1248:                             return self.apply_swizzle(obj.get(swizzle_str), swizzle_str) if isinstance(obj.get(swizzle_str), list) else self.apply_swizzle(obj, swizzle_str)
1249:                         return self.apply_swizzle(obj, swizzle_str)
1250: 
1251:                     return 0
1252: 
1253:         # 局部变量查找
1254:         if name in local_vars:
1255:             val = local_vars[name]
1256:             return val
1257: 
1258:         base_name = name.split('.')[0] if '.' in name else name
1259: 
1260:         # cbuffer字段查找
1261:         for cb_name, cb_def in self.cbuffers.items():
1262:             if isinstance(cb_def, CbufferDefinition):
1263:                 for field in cb_def.fields:
1264:                     if field.name == base_name:
1265:                         return field.data if field.data is not None else 0
1266: 
1267:         # 全局变量查找
1268:         if name in self.variables:
1269:             return self.variables[name]
1270: 
1271:         # 嵌套cbuffer查找
1272:         try:
1273:             if '.' in name:
1274:                 parts = name.split('.')
1275:                 base = parts[0]
1276:                 for cb_name, cb_data in self.cbuffers.items():
1277:                     if base in cb_data:
1278:                         val = cb_data[base]
1279:                         for p in parts[1:]:
1280:                             if isinstance(val, list) and p in ['x', 'y', 'z', 'w']:
1281:                                 idx = ['x', 'y', 'z', 'w'].index(p)
1282:                                 val = val[idx] if idx < len(val) else 0
1283:                             else:
1284:                                 break
1285:                         return val
1286:         except:
1287:             pass
1288: 
1289:         return 0.0
1290: 
1291:     def execute_statement(self, stmt: str, local_vars: Dict[str, Any]):
1292:         """
1293:         执行单条HLSL语句
1294:         stmt: 语句字符串，如 "float3 pos = input.Pos;" 或 "output.Color = float4(1,0,0,1);"
1295:         local_vars: 局部变量字典
1296:         """
1297:         stmt = stmt.strip()
1298:         if not stmt:
1299:             return None
1300: 

(Output capped at 50 KB. Showing lines 1-1300. Use offset=1301 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 8.4s)

_Thinking:_

Let me read the rest of the file to see the full `execute_statement` method and understand where the printing happens.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "offset": 1300
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\hlsl_interpreter.py</path>
<type>file</type>
<content>
1300: 
1301:         self.debug_print(f"\n[STMT] Executing: {stmt}")
1302:         input_snapshot = {k: v for k, v in local_vars.items() if k.startswith('input.') or k == 'output'}
1303: 
1304:         # 变量声明语句: float4 pos = ...;
1305:         type_pattern = '|'.join(DATA_TYPE_LIST)
1306:         pattern = rf'^({type_pattern})\s+(\w+)\s*=\s*(.+?);?$'
1307:         match = re.match(pattern, stmt)
1308:         if match:
1309:             var_name = match.group(2)
1310:             value = self.evaluate_expression(match.group(3), local_vars)
1311:             local_vars[var_name] = value
1312:             self.debug_print(f"[STMT] {stmt} => {var_name} = {self._format_value(value)}")
1313:             return None
1314: 
1315:         # output字段赋值: output.Color = ...;
1316:         if 'output.' in stmt or 'output[' in stmt:
1317:             match = re.match(r'output\.(\w+)\s*=\s*(.+)', stmt)
1318:             if match:
1319:                 field_name = match.group(1)
1320:                 value_expr = match.group(2).rstrip(';').strip()
1321:                 value = self.evaluate_expression(value_expr, local_vars)
1322:                 if 'output' not in local_vars:
1323:                     local_vars['output'] = {}
1324:                 local_vars['output'][field_name] = value
1325:                 self.debug_print(f"[STMT] {stmt} => output.{field_name} = {self._format_float(value)}")
1326:                 return None
1327: 
1328:         # 一般赋值语句: var = ...;
1329:         if '=' in stmt and stmt.count('=') == 1:
1330:             match = re.match(r'(\w+)\s*=\s*(.+?);?$', stmt)
1331:             if match:
1332:                 var_name = match.group(1)
1333:                 value = self.evaluate_expression(match.group(2), local_vars)
1334:                 local_vars[var_name] = value
1335:                 self.debug_print(f"[STMT] {stmt} => {var_name} = {value}")
1336:                 return None
1337: 
1338:         self.debug_print(f"[STMT] {stmt} => (no assignment)")
1339:         return None
1340: 
1341:     def execute_main_function(self, code: str, main_func: str, input_struct_name: str, row_index: int, data: Dict[str, Any]):
1342:         """
1343:         执行HLSL main函数
1344:         code: HLSL代码
1345:         main_func: main函数名
1346:         input_struct_name: 输入结构体名
1347:         row_index: 数据行索引
1348:         data: 输入数据字典
1349:         返回: output结构体字典
1350:         """
1351:         input_struct = self.structs.get(input_struct_name)
1352:         if not input_struct:
1353:             self.log_output(f"Cannot find input_struct: {input_struct_name}\n")
1354:             return None
1355: 
1356:         input_fields = {}
1357:         for field in input_struct.fields:
1358:             input_fields[field.name] = field.field_type
1359: 
1360:         # 查找main函数签名
1361:         func_signature_pattern = r'(\w+)\s+' + re.escape(main_func) + r'\s*\(\s*(\w+)\s+input\s*\)'
1362:         func_signature_match = re.search(func_signature_pattern, code)
1363:         if not func_signature_match:
1364:             return None
1365: 
1366:         output_struct_name = func_signature_match.group(1)
1367:         input_struct_name_from_func = func_signature_match.group(2)
1368: 
1369:         if output_struct_name not in self.structs:
1370:             return None
1371: 
1372:         output_struct = self.structs[output_struct_name]
1373:         output_fields = {}
1374:         for field in output_struct.fields:
1375:             output_fields[field.name] = field.field_type
1376: 
1377:         # 定位main函数体
1378:         func_signature = rf'{output_struct_name}\s+{main_func}\s*\(\s*{input_struct_name_from_func}\s+input\s*\)'
1379:         func_start = re.search(func_signature, code)
1380:         if not func_start:
1381:             return None
1382: 
1383:         # 提取函数体(处理嵌套大括号)
1384:         open_brace_pos = func_start.end()
1385:         brace_depth = 1
1386:         pos = open_brace_pos
1387:         while pos < len(code) and brace_depth > 0:
1388:             if code[pos] == '{':
1389:                 brace_depth += 1
1390:             elif code[pos] == '}':
1391:                 brace_depth -= 1
1392:             pos += 1
1393: 
1394:         body = code[open_brace_pos+1:pos-1].strip()
1395:         if body.startswith('{') and body.endswith('}'):
1396:             body = body[1:-1].strip()
1397: 
1398:         # 初始化局部变量
1399:         local_vars = {'data': data}
1400: 
1401:         # 设置input字段变量
1402:         for field_name, field_value in data.items():
1403:             local_vars[f'input.{field_name}'] = field_value
1404: 
1405:         # 初始化output对象
1406:         output_obj = {}
1407:         for field in output_fields:
1408:             output_obj[field] = None
1409:         local_vars['output'] = output_obj
1410: 
1411:         # 分割语句
1412:         statements = []
1413:         current_stmt = []
1414:         brace_count = 0
1415:         in_string = False
1416: 
1417:         for char in body:
1418:             if char == '{':
1419:                 brace_count += 1
1420:                 current_stmt.append(char)
1421:             elif char == '}':
1422:                 brace_count -= 1
1423:                 current_stmt.append(char)
1424:             elif char == ';' and brace_count == 0 and not in_string:
1425:                 stmt = ''.join(current_stmt).strip()
1426:                 if stmt:
1427:                     statements.append(stmt)
1428:                 current_stmt = []
1429:             else:
1430:                 current_stmt.append(char)
1431: 
1432:         if current_stmt:
1433:             stmt = ''.join(current_stmt).strip()
1434:             if stmt:
1435:                 statements.append(stmt)
1436: 
1437:         ret_val = None
1438: 
1439:         self.debug_print(f"\n=== INPUT DATA ===")
1440:         for k, v in local_vars.items():
1441:             if k.startswith('input.') or k == 'output':
1442:                 self.debug_print(f"  {k} = {v}")
1443:         self.debug_print(f"==================")
1444: 
1445:         # 顺序执行语句
1446:         for stmt in statements:
1447:             if 'return' in stmt and 'output' in stmt:
1448:                 ret_val = local_vars.get('output')
1449:                 continue
1450:             self.execute_statement(stmt, local_vars)
1451: 
1452:         return ret_val
1453: 
1454:     def interpret(self, code: str):
1455:         """
1456:         解释HLSL代码 - 解析结构体和cbuffer定义
1457:         code: HLSL源代码
1458:         """
1459:         script_dir = os.path.dirname(os.path.abspath(__file__))
1460: 
1461:         # 解析struct定义
1462:         struct_pattern = r'struct\s+\w+\s*\{[^}]+\}'
1463:         for struct_match in re.finditer(struct_pattern, code):
1464:             struct_def = self.parse_struct(struct_match.group())
1465:             if struct_def:
1466:                 self.structs[struct_def.name] = struct_def
1467: 
1468:         # 解析cbuffer定义
1469:         cbuffer_pattern = r'cbuffer\s+\w+[^}]+\}'
1470:         for cb_match in re.finditer(cbuffer_pattern, code, re.DOTALL):
1471:             cb_def = self.parse_cbuffer(cb_match.group())
1472:             if cb_def:
1473:                 self.cbuffers[cb_def.name] = cb_def
1474: 
1475:         # 从CSV加载struct数据
1476:         for struct_name in self.structs:
1477:             csv_path = os.path.join(script_dir, f'{struct_name}.csv')
1478:             if os.path.exists(csv_path):
1479:                 self.load_struct_data_from_csv(struct_name, csv_path)
1480: 
1481:         # 从CSV加载cbuffer数据
1482:         for cb_name in self.cbuffers:
1483:             csv_path = os.path.join(script_dir, f'{cb_name}.csv')
1484:             if os.path.exists(csv_path):
1485:                 self.load_cbuffer_data_from_csv(cb_name, csv_path)
1486: 
1487:     def executeVS(self, code: str, main_func: str, vs_input: str):
1488:         """
1489:         执行顶点着色器
1490:         code: HLSL代码
1491:         main_func: 入口函数名
1492:         vs_input: 输入结构体名
1493:         返回: 输出结构体字典列表
1494:         """
1495:         input_struct = self.structs.get(vs_input)
1496:         if not input_struct:
1497:             self.log_output(f"Cannot find vs input: {vs_input}\n")
1498:             return None
1499: 
1500:         # 统计行数
1501:         num_rows = 0
1502:         for field in input_struct.fields:
1503:             if field.data:
1504:                 num_rows = max(num_rows, len(field.data))
1505: 
1506:         results = []
1507:         for row_index in range(num_rows):
1508:             data = {}
1509:             for field in input_struct.fields:
1510:                 if field.data and row_index < len(field.data):
1511:                     data[field.name] = field.data[row_index]
1512:             result = self.execute_main_function(code, main_func, vs_input, row_index, data)
1513:             results.append(result)
1514:         return results
1515: 
1516:     def executePS(self, code: str, main_func: str, ps_input: str):
1517:         """
1518:         执行像素着色器(当前为占位函数)
1519:         """
1520:         pass
1521: 
1522:     def load_struct_data_from_csv(self, struct_name: str, csv_path: str):
1523:         """
1524:         从CSV文件加载struct数据
1525:         struct_name: 结构体名称
1526:         csv_path: CSV文件路径
1527:         """
1528:         if struct_name not in self.structs:
1529:             return
1530:         struct_def = self.structs[struct_name]
1531:         rows = self.load_csv(csv_path)
1532:         if not rows or len(rows) < 2:
1533:             return
1534: 
1535:         header = rows[0]
1536:         data_rows = rows[1:]
1537: 
1538:         # 建立字段列索引映射
1539:         field_col_indices = {}
1540:         for i, col in enumerate(header):
1541:             col_clean = col.strip()
1542:             if '.' in col_clean:
1543:                 parts = col_clean.split('.')
1544:                 base_name = parts[0]
1545:                 suffix = parts[1]
1546:                 if base_name not in field_col_indices:
1547:                     field_col_indices[base_name] = {}
1548:                 field_col_indices[base_name][suffix] = i
1549: 
1550:         # 填充字段数据
1551:         for field in struct_def.fields:
1552:             if field.semantic in field_col_indices:
1553:                 col_dict = field_col_indices[field.semantic]
1554:                 values = []
1555:                 for row in data_rows:
1556:                     if 'x' in col_dict and 'y' in col_dict and 'z' in col_dict and 'w' in col_dict:
1557:                         x = float(row[col_dict['x']].strip())
1558:                         y = float(row[col_dict['y']].strip())
1559:                         z = float(row[col_dict['z']].strip())
1560:                         w = float(row[col_dict['w']].strip())
1561:                         values.append([x, y, z, w])
1562:                     elif 'x' in col_dict and 'y' in col_dict and 'z' in col_dict:
1563:                         x = float(row[col_dict['x']].strip())
1564:                         y = float(row[col_dict['y']].strip())
1565:                         z = float(row[col_dict['z']].strip())
1566:                         values.append([x, y, z])
1567:                     elif 'x' in col_dict and 'y' in col_dict:
1568:                         x = float(row[col_dict['x']].strip())
1569:                         y = float(row[col_dict['y']].strip())
1570:                         values.append([x, y])
1571:                     else:
1572:                         val_str = row[col_dict['x']].strip().strip('"')
1573:                         values.append(self.parse_value_by_type(val_str, field.field_type))
1574:                 field.data = values
1575:                 self.log_output(f"Field '{field.semantic}' ({field.field_type}): {values[0] if values else 'N/A'}")
1576: 
1577:     def load_cbuffer_data_from_csv(self, cb_name: str, csv_path: str):
1578:         """
1579:         从CSV文件加载cbuffer数据
1580:         cb_name: cbuffer名称
1581:         csv_path: CSV文件路径
1582:         """
1583:         if cb_name not in self.cbuffers:
1584:             return
1585:         cb_def = self.cbuffers[cb_name]
1586:         rows = self.load_csv(csv_path)
1587:         if not rows or len(rows) < 2:
1588:             return
1589: 
1590:         header = rows[0]
1591:         name_idx = header.index('Name') if 'Name' in header else -1
1592:         value_idx = header.index('Value') if 'Value' in header else -1
1593:         type_idx = header.index('Type') if 'Type' in header else -1
1594: 
1595:         if name_idx == -1 or value_idx == -1:
1596:             return
1597: 
1598:         matrix_rows = {}
1599:         scalar_vars = {}
1600: 
1601:         for row in rows[1:]:
1602:             if len(row) <= max(name_idx, value_idx):
1603:                 continue
1604:             var_name = row[name_idx].strip().strip('"')
1605:             value_str = row[value_idx].strip().strip('"') if value_idx < len(row) else ''
1606:             type_str = row[type_idx].strip().strip('"') if type_idx != -1 and type_idx < len(row) else ''
1607: 
1608:             # 跳过空值
1609:             if value_str == '':
1610:                 continue
1611: 
1612:             # 矩阵行处理(如 World.row0, World.row1)
1613:             if '.' in var_name:
1614:                 parts = var_name.split('.')
1615:                 base_name = parts[0]
1616:                 suffix = parts[1]
1617:                 if suffix.startswith('row'):
1618:                     row_idx = int(suffix[3:])
1619:                     if base_name not in matrix_rows:
1620:                         matrix_rows[base_name] = {}
1621:                     matrix_rows[base_name][row_idx] = (value_str, type_str)
1622:             else:
1623:                 scalar_vars[var_name] = (value_str, type_str)
1624: 
1625:         # 填充字段数据
1626:         for field in cb_def.fields:
1627:             if field.name in matrix_rows:
1628:                 row_dict = matrix_rows[field.name]
1629:                 if all(i in row_dict for i in range(4)):
1630:                     matrix = []
1631:                     for i in range(4):
1632:                         value_str, type_str = row_dict[i]
1633:                         parts = value_str.split(',')
1634:                         matrix.append([float(p.strip()) for p in parts[:4]])
1635:                     field.data = matrix
1636:             elif field.name in scalar_vars:
1637:                 value_str, type_str = scalar_vars[field.name]
1638:                 field.data = self.parse_value_by_type(value_str, type_str)
1639: 
1640:         # 打印cbuffer内容
1641:         cb_n = cb_name
1642:         cb_d = cb_def
1643:         self.log_output(f"Cbuffer {cb_n}:")
1644:         for f in cb_d.fields:
1645:             data = f.data
1646:             ft = f.field_type
1647:             if 'float4x4' in ft:
1648:                 self.log_output(f"  {f.name} ({ft}):")
1649:                 for row in data:
1650:                     row_str = '  '.join(f"{v:12.5f}" for v in row)
1651:                     self.log_output(f"    [{row_str}]")
1652:             elif 'float3x3' in ft:
1653:                 self.log_output(f"  {f.name} ({ft}):")
1654:                 for row in data:
1655:                     row_str = '  '.join(f"{v:12.5f}" for v in row)
1656:                     self.log_output(f"    [{row_str}]")
1657:             elif 'float4' in ft:
1658:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
1659:             elif 'float3' in ft:
1660:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
1661:             elif 'float2' in ft:
1662:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
1663:             elif 'float' in ft:
1664:                 self.log_output(f"  {f.name} ({ft}): {data:.5f}")
1665:             elif 'uint4' in ft:
1666:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1667:             elif 'uint3' in ft:
1668:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1669:             elif 'uint2' in ft:
1670:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1671:             elif 'uint' in ft:
1672:                 self.log_output(f"  {f.name} ({ft}): {data}")
1673:             elif 'int4' in ft:
1674:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1675:             elif 'int3' in ft:
1676:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1677:             elif 'int2' in ft:
1678:                 self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
1679:             elif 'int' in ft:
1680:                 self.log_output(f"  {f.name} ({ft}): {data}")
1681:             elif 'bool' in ft:
1682:                 self.log_output(f"  {f.name} ({ft}): {data}")
1683:             else:
1684:                 self.log_output(f"  {f.name} ({ft}): {data}")
1685: 
1686:     def load_vs_output_golden_from_csv(self, csv_path: str):
1687:         """
1688:         从CSV文件加载VS_OUTPUT的golden数据
1689:         csv_path: CSV文件路径
1690:         """
1691:         if "VS_OUTPUT" not in self.structs:
1692:             self.log_output("Error: VS_OUTPUT struct not defined")
1693:             return False
1694: 
1695:         vs_output_def = self.structs["VS_OUTPUT"]
1696:         rows = self.load_csv(csv_path)
1697:         if not rows or len(rows) < 2:
1698:             self.log_output(f"Error: CSV file {csv_path} is empty or has no data rows")
1699:             return False
1700: 
1701:         header = rows[0]
1702:         data_rows = rows[1:]
1703: 
1704:         field_col_indices = {}
1705:         for i, col in enumerate(header):
1706:             col_clean = col.strip()
1707:             if '.' in col_clean:
1708:                 parts = col_clean.split('.')
1709:                 base_name = parts[0]
1710:                 suffix = parts[1]
1711:                 if base_name not in field_col_indices:
1712:                     field_col_indices[base_name] = {}
1713:                 field_col_indices[base_name][suffix] = i
1714: 
1715:         for field in vs_output_def.fields:
1716:             if field.semantic in field_col_indices:
1717:                 col_dict = field_col_indices[field.semantic]
1718:                 values = []
1719:                 for row in data_rows:
1720:                     try:
1721:                         if 'x' in col_dict and 'y' in col_dict and 'z' in col_dict and 'w' in col_dict:
1722:                             x = float(row[col_dict['x']].strip())
1723:                             y = float(row[col_dict['y']].strip())
1724:                             z = float(row[col_dict['z']].strip())
1725:                             w = float(row[col_dict['w']].strip())
1726:                             values.append([x, y, z, w])
1727:                         elif 'x' in col_dict and 'y' in col_dict and 'z' in col_dict:
1728:                             x = float(row[col_dict['x']].strip())
1729:                             y = float(row[col_dict['y']].strip())
1730:                             z = float(row[col_dict['z']].strip())
1731:                             values.append([x, y, z])
1732:                         elif 'x' in col_dict and 'y' in col_dict:
1733:                             x = float(row[col_dict['x']].strip())
1734:                             y = float(row[col_dict['y']].strip())
1735:                             values.append([x, y])
1736:                         else:
1737:                             val_str = row[col_dict['x']].strip().strip('"')
1738:                             values.append(self.parse_value_by_type(val_str, field.field_type))
1739:                     except (ValueError, IndexError) as e:
1740:                         self.log_output(f"Warning: Failed to parse {field.semantic} at row: {e}")
1741:                         values.append(None)
1742:                 field.data = values
1743: 
1744:         self.log_output(f"Loaded {len(data_rows)} golden data rows for VS_OUTPUT")
1745:         return True
1746: 
1747:     def compare_vs_output_with_golden(self, hlsl_output: List[Dict], output_struct_name: str = "VS_OUTPUT", float_tolerance: float = 0.0001) -> bool:
1748:         """
1749:         比较HLSL执行结果与golden数据
1750:         hlsl_output: executeVS返回的输出结构体字典列表
1751:         output_struct_name: 输出结构体名称，用于获取field name (默认"VS_OUTPUT")
1752:         float_tolerance: 浮点类型数据的比较误差容忍度
1753:         返回: True表示所有数据匹配, False表示存在不匹配
1754:         """
1755:         if output_struct_name not in self.structs:
1756:             self.log_output(f"Error: {output_struct_name} struct not found")
1757:             return False
1758: 
1759:         vs_output_def = self.structs[output_struct_name]
1760:         golden_data = {}
1761:         semantic_to_field = {}
1762: 
1763:         for field in vs_output_def.fields:
1764:             if field.data:
1765:                 golden_data[field.semantic] = field.data
1766:             semantic_to_field[field.semantic] = field.name
1767: 
1768:         num_golden_rows = 0
1769:         for field_data in golden_data.values():
1770:             if field_data:
1771:                 num_golden_rows = max(num_golden_rows, len(field_data))
1772: 
1773:         if not hlsl_output:
1774:             self.log_output("Error: No HLSL output to compare")
1775:             return False
1776: 
1777:         if len(hlsl_output) != num_golden_rows:
1778:             self.log_output(f"Error: Row count mismatch - HLSL output has {len(hlsl_output)} rows, golden has {num_golden_rows} rows")
1779:             return False
1780: 
1781:         all_match = True
1782:         field_type_map = {}
1783:         for field in vs_output_def.fields:
1784:             field_type_map[field.semantic] = field.field_type
1785: 
1786:         for row_idx in range(len(hlsl_output)):
1787:             output_row = hlsl_output[row_idx]
1788:             for semantic, golden_values in golden_data.items():
1789:                 if row_idx >= len(golden_values):
1790:                     continue
1791: 
1792:                 field_name = semantic_to_field.get(semantic, semantic)
1793:                 if field_name not in output_row:
1794:                     continue
1795: 
1796:                 output_value = output_row[field_name]
1797:                 golden_value = golden_values[row_idx]
1798: 
1799:                 if output_value is None or golden_value is None:
1800:                     continue
1801: 
1802:                 field_type = field_type_map.get(semantic, '')
1803: 
1804:                 if isinstance(output_value, list) and isinstance(golden_value, list):
1805:                     if len(output_value) != len(golden_value):
1806:                         self.log_output(f"Error: Row {row_idx}, {field_name}: length mismatch output={len(output_value)} golden={len(golden_value)}")
1807:                         all_match = False
1808:                         continue
1809: 
1810:                     is_float = 'float' in field_type
1811:                     for comp_idx in range(len(output_value)):
1812:                         out_comp = output_value[comp_idx]
1813:                         gold_comp = golden_value[comp_idx]
1814: 
1815:                         if is_float:
1816:                             if isinstance(out_comp, float) and isinstance(gold_comp, float):
1817:                                 if abs(out_comp - gold_comp) > float_tolerance:
1818:                                     self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp:.6f} golden={gold_comp:.6f} diff={abs(out_comp - gold_comp):.6f} > tolerance={float_tolerance}")
1819:                                     all_match = False
1820:                             elif out_comp != gold_comp:
1821:                                 self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp} golden={gold_comp} (float comparison failed)")
1822:                                 all_match = False
1823:                         else:
1824:                             if out_comp != gold_comp:
1825:                                 self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp} golden={gold_comp} (strict equality failed)")
1826:                                 all_match = False
1827: 
1828:         if all_match:
1829:             self.log_output("Comparison PASSED: All output data matches golden data within tolerance")
1830:         else:
1831:             self.log_output("Comparison FAILED: Some output data does not match golden data")
1832: 
1833:         return all_match
1834: 
1835: 
1836: def main():
1837:     interpreter = HLSLInterpreter()
1838: 
1839:     code = '''
1840:     struct VS_INPUT {
1841:         float3 Pos : POSITION;
1842:         float3 Normal : NORMAL;
1843:         float4 Color : COLOR;
1844:         float2 TexCoord : TEXCOORD;
1845:     };
1846:     struct VS_OUTPUT {
1847:         float4 Pos : SV_POSITION;
1848:         float4 Color : COLOR;
1849:         float2 TexCoord : TEXCOORD0;
1850:         float2 TexCoord2 : TEXCOORD1;
1851:         float3 Normal : NORMAL;
1852:         float3 WorldPos : WORLDPOS;
1853:     };
1854:     cbuffer MatrixBuffer : register(b0) {
1855:         float4x4 WorldViewProj;
1856:         float4x4 World;
1857:     };
1858:     cbuffer LightBuffer : register(b1) {
1859:         float4 AmbientColor;
1860:         float4 DiffuseColor;
1861:         float4 SpecularColor;
1862:         float3 LightPos;
1863:         float LightRadius;
1864:         float3 LightDir;
1865:         float3 Attenuation;
1866:         float OuterCone;
1867:         float InnerCone;
1868:     };
1869:     cbuffer MaterialBuffer : register(b2) {
1870:         float4 MaterialDiffuseColor;
1871:         float4 MaterialAmbientColor;
1872:         float4 MaterialSpecularColor;
1873:         float4 MaterialEmissiveColor;
1874:         float Shininess;
1875:         uint ColorMaterialMode;
1876:         float2 Padding;
1877:     };
1878:     cbuffer CameraBuffer : register(b3) {
1879:         float3 cameraPos;
1880:     };
1881:     VS_OUTPUT main(VS_INPUT input) {
1882:         VS_OUTPUT output;
1883:         output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));
1884:         float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));
1885:         float3 nor = normalize(input.Normal);
1886:         float3 normal = normalize(mul(nor, (float3x3)World));
1887:         output.WorldPos = worldPos.xyz;
1888:         output.Normal = normal;
1889:         output.TexCoord = input.TexCoord;
1890:         output.TexCoord2 = input.TexCoord;
1891:         float3 lightDistant = LightPos.xyz - worldPos.xyz;
1892:         float dist = length(lightDistant);
1893:         float3 lightDir = normalize(lightDistant);
1894:         float3 viewDir = cameraPos;
1895:         float NdotL = max(dot(normal, lightDir), 0.0);
1896:         float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;
1897:         float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;
1898:         float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;
1899:         float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;
1900:         float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;
1901:         float3 R = reflect(lightDir, normal);
1902:         float RdotV = max(dot(R, viewDir), 0.0);
1903:         float3 specular = RdotV > 0.0 ? matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess) : float3(0.0, 0.0, 0.0);
1904:         float3 ambient = matAmbient.rgb * AmbientColor.rgb;
1905:         float3 emissive = matEmissive.rgb;
1906:         float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);
1907:         float cond = dist <= LightRadius ? 1.0 : 0.0;
1908:         output.Color = float4((ambient + diffuse * att + specular * att + emissive) * cond, 1.0);
1909:         return output;
1910:     }
1911:     '''
1912: 
1913:     script_dir = os.path.dirname(os.path.abspath(__file__))
1914:     interpreter.interpret(code)
1915: 
1916:     golden_csv_path = os.path.join(script_dir, 'VS_OUTPUT.csv')
1917:     if os.path.exists(golden_csv_path):
1918:         interpreter.load_vs_output_golden_from_csv(golden_csv_path)
1919: 
1920:     results = interpreter.executeVS(code, "main", "VS_INPUT")
1921: 
1922:     interpreter.log_output("HLSL Interpreter Result:")
1923:     interpreter.log_output("=" * 40)
1924:     if results:
1925:         for idx, result in enumerate(results):
1926:             interpreter.log_output(f"\n--- Row {idx} ---")
1927:             if result:
1928:                 for key, value in result.items():
1929:                     if isinstance(value, list):
1930:                         if len(value) == 4:
1931:                             interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]")
1932:                         elif len(value) == 3:
1933:                             interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]")
1934:                         elif len(value) == 2:
1935:                             interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}]")
1936:                         else:
1937:                             interpreter.log_output(f"{key}: {value}")
1938:                     else:
1939:                         interpreter.log_output(f"{key}: {value}")
1940:     else:
1941:         interpreter.log_output("No result produced")
1942: 
1943:     if results and results[-1] and 'Color' in results[-1]:
1944:         color = results[-1]['Color']
1945:         if color and isinstance(color, list) and len(color) == 4:
1946:             interpreter.log_output("\nFinal Output Color (RGBA):")
1947:             interpreter.log_output(f"  R: {color[0]:.4f}")
1948:             interpreter.log_output(f"  G: {color[1]:.4f}")
1949:             interpreter.log_output(f"  B: {color[2]:.4f}")
1950:             interpreter.log_output(f"  A: {color[3]:.4f}")
1951:         else:
1952:             interpreter.log_output(f"\nColor result: {color}")
1953: 
1954:     interpreter.log_output("\n" + "=" * 40)
1955:     interpreter.log_output("Comparing with golden data...")
1956:     interpreter.log_output("=" * 40)
1957:     interpreter.compare_vs_output_with_golden(results)
1958: 
1959: 
1960: if __name__ == '__main__':
1961:     main()

(End of file - total 1961 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 23.0s)

_Thinking:_

Now I understand the code. The `evaluate_syntax_tree` method is at line 899. Looking at the code, I can see that:

1. The debug prints are controlled by `self.debug` (line 388) and `self.printSyntaxTree` (line 389)
2. The `debug_print` method (line 402-405) checks `self.debug` before printing
3. `evaluate_syntax_tree` doesn't directly print, it calls `execute_binary_op` and `execute_unary_op` which call `debug_print`

Looking at the user's example, the prints include:
- `[STMT] Executing: ...`
- `[SYNTAX TREE]` 
- `[BINARY OP] ...`
- `[STMT] ... => ...`

These come from:
- `execute_statement` at line 1301: `self.debug_print(f"\n[STMT] Executing: {stmt}")`
- `evaluate_expression` at line 894-895: `self.debug_print(f"[SYNTAX TREE]\n{tree}")`
- `execute_binary_op` at line 769: `self.debug_print(f"[BINARY OP] left=..., right=..., op=..., result=...")`
- `execute_statement` at line 1312: `self.debug_print(f"[STMT] {stmt} => {var_name} = ...")`

To implement the `print_sequency` feature, I need to:
1. Add a `print_sequence` parameter to `__init__`
2. Add a counter to track how many times `evaluate_syntax_tree` has been called
3. Modify the debug printing to only print when the counter meets the sequence requirement

Let me implement this properly:




我来添加 `print_sequence` 控制变量。首先在 `__init__` 中添加 `print_sequence` 参数和执行计数器，然后修改相关调试打印逻辑。


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "newString": "def __init__(self, log_to_file: bool = True, log_file_path: str = \"hlsl_interpreter.log\", print_sequence: int = 1):\n        self.structs: Dict[str, StructDefinition] = {}      # 解析的结构体定义\n        self.cbuffers: Dict[str, CbufferDefinition] = {}    # 解析的cbuffer定义\n        self.variables: Dict[str, Any] = {}                 # 全局变量\n        self.debug = True                                   # 调试模式开关\n        self.printSyntaxTree = False                        # 打印语法树开关\n        self.syntax_parser = SyntaxTreeParser()             # 语法树解析器\n        self.log_to_file = log_to_file                      # 是否输出到文件\n        self.log_file_path = log_file_path                  # 日志文件路径\n        self.print_sequence = max(1, print_sequence)         # 打印间隔频率\n        self._eval_counter = 0                              # evaluate_syntax_tree执行计数器\n        self._should_print = True                           # 当前是否应该打印",
  "oldString": "def __init__(self, log_to_file: bool = True, log_file_path: str = \"hlsl_interpreter.log\"):\n        self.structs: Dict[str, StructDefinition] = {}      # 解析的结构体定义\n        self.cbuffers: Dict[str, CbufferDefinition] = {}    # 解析的cbuffer定义\n        self.variables: Dict[str, Any] = {}                 # 全局变量\n        self.debug = True                                   # 调试模式开关\n        self.printSyntaxTree = False                        # 打印语法树开关\n        self.syntax_parser = SyntaxTreeParser()             # 语法树解析器\n        self.log_to_file = log_to_file                      # 是否输出到文件\n        self.log_file_path = log_file_path                  # 日志文件路径"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 24.0s)

_Thinking:_

Now I need to modify the `evaluate_syntax_tree` method to use this counter and decide whether to print or not. The method starts at line 899.

Let me look at what prints happen in `evaluate_syntax_tree`. Looking at the code:

The `evaluate_syntax_tree` itself doesn't print directly - it calls `execute_binary_op` and `execute_unary_op` which have `self.debug_print` calls.

But looking at the user's example, they show logs like:
```
[STMT] Executing: float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist)
[SYNTAX TREE]
BinaryOp(/)
  ...
[BINARY OP] left=0.0000, right=498.6749, op=*, result=0.0000
...
[STMT] float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist) => att = 0.0000
```

These logs come from:
1. `[STMT] Executing:` from `execute_statement` at line 1301
2. `[SYNTAX TREE]` from `evaluate_expression` at line 894-895 (when printSyntaxTree is True)
3. `[BINARY OP]` from `execute_binary_op` at line 769
4. `[STMT] ... =>` from `execute_statement` at line 1312

So the printing happens through `debug_print` which checks `self.debug`. The issue is that `debug_print` is called for EVERY operation.

The user wants to control printing based on `print_sequence` - meaning only print once every N times `evaluate_syntax_tree` is called.

So the logic should be:
1. Increment counter on each `evaluate_syntax_tree` call
2. When counter reaches print_sequence, set _should_print = True and reset counter


3. All the debug prints inside `evaluate_syntax_tree` (and its callees like `execute_binary_op`) should check `_should_print` before outputting

I need to modify `debug_print` to check this flag, or add the check at each print statement. Since the printing cascades through multiple nested calls - from `evaluate_syntax_tree` into `execute_binary_op`, `execute_unary_op`, and `execute_function_node` - the cleanest approach would be to have `debug_print` itself respect the `_should_print` flag so all debug output respects this gating mechanism.


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "newString": "def debug_print(self, msg: str):\n        \"\"\"调试打印\"\"\"\n        if self.debug and self._should_print:\n            self.log_output(msg)",
  "oldString": "def debug_print(self, msg: str):\n        \"\"\"调试打印\"\"\"\n        if self.debug:\n            self.log_output(msg)"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 10.8s)

_Thinking:_

Now I need to modify `evaluate_syntax_tree` to increment the counter and update `_should_print` before each execution. Let me find and modify that method.


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "newString": "def evaluate_syntax_tree(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:\n        \"\"\"\n        对语法树节点求值\n        node: 语法树节点\n        local_vars: 局部变量字典\n        返回: 求值结果\n        \"\"\"\n        self._eval_counter += 1\n        self._should_print = (self._eval_counter % self.print_sequence == 0)\n\n        if node is None:\n            return None\n\n        if node.node_type == 'value':",
  "oldString": "def evaluate_syntax_tree(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:\n        \"\"\"\n        对语法树节点求值\n        node: 语法树节点\n        local_vars: 局部变量字典\n        返回: 求值结果\n        \"\"\"\n        if node is None:\n            return None\n\n        if node.node_type == 'value':"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 7.8s)

_Thinking:_

Good, the edits were applied successfully. Let me verify the changes by reading the relevant section of the file to make sure everything looks correct.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "limit": 30,
  "offset": 383
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\hlsl_interpreter.py</path>
<type>file</type>
<content>
383: 
384:     def __init__(self, log_to_file: bool = True, log_file_path: str = "hlsl_interpreter.log", print_sequence: int = 1):
385:         self.structs: Dict[str, StructDefinition] = {}      # 解析的结构体定义
386:         self.cbuffers: Dict[str, CbufferDefinition] = {}    # 解析的cbuffer定义
387:         self.variables: Dict[str, Any] = {}                 # 全局变量
388:         self.debug = True                                   # 调试模式开关
389:         self.printSyntaxTree = False                        # 打印语法树开关
390:         self.syntax_parser = SyntaxTreeParser()             # 语法树解析器
391:         self.log_to_file = log_to_file                      # 是否输出到文件
392:         self.log_file_path = log_file_path                  # 日志文件路径
393:         self.print_sequence = max(1, print_sequence)         # 打印间隔频率
394:         self._eval_counter = 0                              # evaluate_syntax_tree执行计数器
395:         self._should_print = True                           # 当前是否应该打印
396: 
397:     def log_output(self, *args, **kwargs):
398:         """输出到stdout和日志文件"""
399:         msg = ' '.join(str(arg) for arg in args)
400:         print(*args, **kwargs)
401:         if self.log_to_file and self.log_file_path:
402:             with open(self.log_file_path, 'a', encoding='utf-8') as f:
403:                 f.write(msg + '\n')
404: 
405:     def debug_print(self, msg: str):
406:         """调试打印"""
407:         if self.debug and self._should_print:
408:             self.log_output(msg)
409: 
410:     def _format_float(self, val):
411:         """
412:         格式化浮点数输出

(Showing lines 383-412 of 1967. Use offset=413 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 11.0s)

_Thinking:_

Now let me check the evaluate_syntax_tree function to make sure my edit was applied correctly:


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\hlsl_interpreter\\hlsl_interpreter.py",
  "limit": 20,
  "offset": 899
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\hlsl_interpreter.py</path>
<type>file</type>
<content>
899: 
900:         return self.evaluate_syntax_tree(tree, local_vars)
901: 
902:     def evaluate_syntax_tree(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
903:         """
904:         对语法树节点求值
905:         node: 语法树节点
906:         local_vars: 局部变量字典
907:         返回: 求值结果
908:         """
909:         self._eval_counter += 1
910:         self._should_print = (self._eval_counter % self.print_sequence == 0)
911: 
912:         if node is None:
913:             return None
914: 
915:         if node.node_type == 'value':
916:             if node.value is None:
917:                 return None
918:             return self.get_value(node.value, local_vars)

(Showing lines 899-918 of 1967. Use offset=919 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 20.6s)

_Thinking:_

The changes are in place. Let me also check if there's a second hlsl_interpreter.py file that needs the same changes.




The changes have been applied. Here's a summary:

1. **Added `print_sequence` parameter** to `__init__` (default=1, meaning print every time)
2. **Added `_eval_counter` and `_should_print` tracking variables**
3. **Modified `evaluate_syntax_tree`** to increment counter and update `_should_print` based on `print_sequence`
4. **Modified `debug_print`** to check `_should_print` before outputting

**Usage:**
- `print_sequence=1` (default): Print every time (original behavior)
- `print_sequence=2`: Print every 2 executions
- `print_sequence=200`: Print every 200 executions

Note: There's another `hlsl_interpreter.py` at `C:\Development\Graphics\irrlicht-1.8.3\hlsl_interpreter\specular_too_shining\hlsl_interpreter.py` - should I apply the same changes there?

---

