import csv
import json
import math
import re
import os
from dataclasses import dataclass, field
from typing import Any, Dict, List, Union, Optional


DATA_TYPE_LIST = ['float4x4', 'float3x3',
                'float4', 'float3', 'float2', 'float',
                'uint4', 'uint3', 'uint2', 'uint',
                'int4', 'int3', 'int2', 'int',
                'bool']


class SyntaxTreeNode:
    def __init__(self, node_type: str, value: Any = None, left: Optional['SyntaxTreeNode'] = None, right: Optional['SyntaxTreeNode'] = None, args: Optional[List['SyntaxTreeNode']] = None):
        self.node_type = node_type
        self.value = value
        self.left = left
        self.right = right
        self.args = args if args is not None else []

    def __repr__(self):
        return self._pretty(0)

    def _pretty(self, indent: int) -> str:
        prefix = "  " * indent
        if self.node_type == 'function':
            lines = [f"Function({self.value})"]
            for i, arg in enumerate(self.args):
                lines.append(f"{prefix}  arg[{i}]:")
                lines.append(arg._pretty(indent + 2))
            return "\n".join(lines)
        elif self.node_type == 'binary_op':
            lines = [f"BinaryOp({self.value})"]
            lines.append(f"{prefix}  left:")
            lines.append(self.left._pretty(indent + 2))
            lines.append(f"{prefix}  right:")
            lines.append(self.right._pretty(indent + 2))
            return "\n".join(lines)
        elif self.node_type == 'unary_op':
            lines = [f"UnaryOp({self.value})"]
            lines.append(f"{prefix}  child:")
            lines.append(self.left._pretty(indent + 2))
            return "\n".join(lines)
        elif self.node_type == 'cast':
            lines = [f"Cast({self.value})"]
            lines.append(f"{prefix}  inner:")
            lines.append(self.left._pretty(indent + 2))
            return "\n".join(lines)
        else:
            return f"{prefix}Value({self.value})"


class SyntaxTreeParser:
    def __init__(self):
        self.operators = {
            '||': 1, '&&': 2,
            '==': 3, '!=': 3,
            '<': 4, '>': 4, '<=': 4, '>=': 4,
            '+': 5, '-': 5,
            '*': 6, '/': 6,
        }

    def parse(self, expr: str) -> SyntaxTreeNode:
        expr = expr.strip()
        return self._parse_expression(expr)

    def _find_top_level_operator(self, expr: str) -> Optional[tuple]:
        depth = 0
        for i in range(len(expr) - 1, -1, -1):
            char = expr[i]
            if char == ')':
                depth += 1
            elif char == '(':
                depth -= 1
            elif depth == 0:
                if i >= 1:
                    two_char = expr[i-1:i+1]
                    if two_char in self.operators:
                        return (i-1, two_char)
                if char in self.operators:
                    return (i, char)
        return None

    def _parse_expression(self, expr: str) -> SyntaxTreeNode:
        expr = expr.strip()
        if not expr:
            return SyntaxTreeNode('value', None)

        cast_match = re.match(r'\((\w+)\)\s*(.+)', expr, re.DOTALL)
        if cast_match:
            cast_type = cast_match.group(1)
            rest = cast_match.group(2).strip()
            inner_node = self._parse_expression(rest)
            if inner_node.node_type == 'value':
                return inner_node
            return SyntaxTreeNode('cast', cast_type, inner_node)

        if expr.startswith('(') and expr.endswith(')'):
            inner = expr[1:-1].strip()
            paren_depth = 0
            is_proper_paren = True
            for j, c in enumerate(inner):
                if c == '(':
                    paren_depth += 1
                elif c == ')':
                    paren_depth -= 1
                if paren_depth < 0:
                    is_proper_paren = False
                    break
            if is_proper_paren:
                return self._parse_expression(inner)

        op_info = self._find_top_level_operator(expr)
        if op_info:
            pos, op = op_info
            if op in ['||', '&&', '==', '!=', '<', '>', '<=', '>=', '+', '-', '*', '/']:
                left_expr = expr[:pos].strip()
                right_expr = expr[pos+len(op):].strip()
                left_node = self._parse_expression(left_expr)
                right_node = self._parse_expression(right_expr)
                return SyntaxTreeNode('binary_op', op, left_node, right_node)

        if re.match(r'float[234]\s*\(', expr):
            return self._parse_function_call(expr)

        if re.match(r'\w+\s*\(', expr):
            return self._parse_function_call(expr)

        return SyntaxTreeNode('value', expr)

    def _parse_function_call(self, expr: str) -> SyntaxTreeNode:
        expr = expr.strip()
        if expr.startswith('('):
            match = re.match(r'\((\w+)\)\s*(.+)', expr, re.DOTALL)
            if match:
                cast_type = match.group(1)
                rest = match.group(2).strip()
                inner_node = self._parse_expression(rest)
                if inner_node.node_type == 'value':
                    return inner_node
                return SyntaxTreeNode('cast', cast_type, inner_node)

        match = re.match(r'^(\w+)\s*\(', expr)
        if not match:
            return SyntaxTreeNode('value', expr)

        func_name = match.group(1)

        depth = 0
        paren_start = -1
        for i, char in enumerate(expr):
            if char == '(':
                depth += 1
                if depth == 1:
                    paren_start = i
            elif char == ')':
                depth -= 1
                if depth == 0:
                    args_str = expr[paren_start+1:i]
                    if func_name in ['transpose', 'normalize', 'length', 'reflect', 'pow', 'max', 'abs', 'sin', 'cos', 'dot']:
                        inner_node = self._parse_expression(args_str.strip())
                        return SyntaxTreeNode('function', func_name, args=[inner_node])
                    elif func_name in ['mul', 'float2', 'float3', 'float4']:
                        args = self._split_args(args_str)
                        arg_nodes = [self._parse_expression(arg.strip()) for arg in args]
                        return SyntaxTreeNode('function', func_name, args=arg_nodes)
                    args = self._split_args(args_str)
                    arg_nodes = [self._parse_expression(arg.strip()) for arg in args]
                    return SyntaxTreeNode('function', func_name, args=arg_nodes)

        return SyntaxTreeNode('value', expr)

    def _split_args(self, args_str: str) -> List[str]:
        if not args_str.strip():
            return []
        args = []
        depth = 0
        current = ''
        for char in args_str:
            if char == '(':
                depth += 1
                current += char
            elif char == ')':
                depth -= 1
                current += char
            elif char == ',' and depth == 0:
                args.append(current.strip())
                current = ''
            else:
                current += char
        if current.strip():
            args.append(current.strip())
        return args


@dataclass
class ShaderVariable:
    name: str
    type: str
    value: Any


@dataclass
class FieldDefinition:
    field_type: str
    name: str
    semantic: str
    data: List[Any] = None

@dataclass
class StructDefinition:
    name: str
    fields: List[FieldDefinition]

@dataclass
class CbufferDefinition:
    name: str
    fields: List[FieldDefinition]


class HLSLInterpreter:
    def __init__(self):
        self.structs: Dict[str, StructDefinition] = {}
        self.cbuffers: Dict[str, CbufferDefinition] = {}
        self.variables: Dict[str, Any] = {}
        self.debug = True
        self.syntax_parser = SyntaxTreeParser()

    def debug_print(self, msg: str):
        if self.debug:
            print(msg)

    def load_json(self, filepath: str):
        with open(filepath, 'r') as f:
            data = json.load(f)
        return data

    def load_csv(self, filepath: str) -> List[List[str]]:
        rows = []
        with open(filepath, 'r') as f:
            reader = csv.reader(f)
            for row in reader:
                rows.append(row)
        return rows

    def get_type_size(self, field_type: str) -> int:
        if 'float4x4' in field_type:
            return 64
        elif 'float3x3' in field_type:
            return 36
        elif 'float4' in field_type:
            return 16
        elif 'float3' in field_type:
            return 12
        elif 'float2' in field_type:
            return 8
        elif 'float' in field_type:
            return 4
        elif 'uint4' in field_type:
            return 16
        elif 'uint3' in field_type:
            return 12
        elif 'uint2' in field_type:
            return 8
        elif 'uint' in field_type:
            return 4
        elif 'int4' in field_type:
            return 16
        elif 'int3' in field_type:
            return 12
        elif 'int2' in field_type:
            return 8
        elif 'int' in field_type:
            return 4
        elif 'bool' in field_type:
            return 4
        return 0

    def parse_value_by_type(self, value_str: str, field_type: str) -> Any:
        value_str = value_str.strip().strip('"')
        if 'float4x4' in field_type:
            parts = value_str.split(',')
            if len(parts) >= 16:
                matrix = []
                for i in range(4):
                    row = [float(parts[j]) for j in range(i*4, i*4+4)]
                    matrix.append(row)
                return matrix
        elif 'float3x3' in field_type:
            parts = value_str.split(',')
            if len(parts) >= 9:
                matrix = []
                for i in range(3):
                    row = [float(parts[j]) for j in range(i*3, i*3+3)]
                    matrix.append(row)
                return matrix
        elif 'float4' in field_type:
            parts = value_str.split(',')
            return [float(p) for p in parts[:4]]
        elif 'float3' in field_type:
            parts = value_str.split(',')
            return [float(p) for p in parts[:3]]
        elif 'float2' in field_type:
            parts = value_str.split(',')
            return [float(p) for p in parts[:2]]
        elif 'uint4' in field_type:
            parts = value_str.split(',')
            return [int(p) for p in parts[:4]]
        elif 'uint3' in field_type:
            parts = value_str.split(',')
            return [int(p) for p in parts[:3]]
        elif 'uint2' in field_type:
            parts = value_str.split(',')
            return [int(p) for p in parts[:2]]
        elif 'uint' in field_type:
            return int(value_str)
        elif 'int4' in field_type:
            parts = value_str.split(',')
            return [int(p) for p in parts[:4]]
        elif 'int3' in field_type:
            parts = value_str.split(',')
            return [int(p) for p in parts[:3]]
        elif 'int2' in field_type:
            parts = value_str.split(',')
            return [int(p) for p in parts[:2]]
        elif 'int' in field_type:
            return int(value_str)
        elif 'bool' in field_type:
            return value_str.lower() in ('true', '1', 'yes')
        try:
            return float(value_str)
        except:
            return value_str

    def parse_type(self, type_str: str) -> str:
        type_str = type_str.strip()
        if type_str in DATA_TYPE_LIST:
            return type_str
        if type_str.startswith('float'):
            if 'x3' in type_str:
                return 'float3x3'
            elif 'x4' in type_str:
                return 'float4x4'
            elif type_str == 'float':
                return 'float'
            return 'float'
        elif type_str.startswith('int'):
            if type_str == 'int':
                return 'int'
            elif '2' in type_str:
                return 'int2'
            elif '3' in type_str:
                return 'int3'
            elif '4' in type_str:
                return 'int4'
            return 'int'
        elif type_str.startswith('uint'):
            if type_str == 'uint':
                return 'uint'
            elif '2' in type_str:
                return 'uint2'
            elif '3' in type_str:
                return 'uint3'
            elif '4' in type_str:
                return 'uint4'
            return 'uint'
        elif type_str.startswith('bool'):
            return 'bool'
        return type_str

    def parse_struct(self, code: str) -> StructDefinition:
        match = re.search(r'struct\s+(\w+)\s*\{([^}]+)\}', code)
        if not match:
            return None
        name = match.group(1)
        fields_str = match.group(2)
        fields = []
        for line in fields_str.split(';'):
            line = line.strip()
            if not line:
                continue
            parts = line.split(':')
            if len(parts) == 2:
                type_and_name = parts[0].strip().split()
                semantic = parts[1].strip()
                if len(type_and_name) >= 2:
                    field_type = type_and_name[0]
                    field_name = type_and_name[-1]
                else:
                    field_type = type_and_name[0]
                    field_name = ''
                fields.append(FieldDefinition(field_type, field_name, semantic))
        return StructDefinition(name, fields)

    def parse_cbuffer(self, code: str) -> CbufferDefinition:
        match = re.search(r'cbuffer\s+(\w+)\s*:.*?\{([^}]+)\}', code, re.DOTALL)
        if not match:
            return None
        name = match.group(1)
        fields = []
        lines = code[match.start():match.end()].split('\n')[1:]
        for line in lines:
            line = line.strip().rstrip(';')
            if not line or line.startswith('}'):
                continue
            if any(t in line for t in DATA_TYPE_LIST):
                parts = line.split()
                if len(parts) >= 2:
                    field_type = parts[0]
                    field_name = parts[1]
                    fields.append(FieldDefinition(field_type, field_name, ''))
        return CbufferDefinition(name, fields)

    def parse_function(self, code: str) -> tuple:
        match = re.search(r'(\w+)\s+(\w+)\s*\(([^)]*)\)\s*\{([^}]+(?:\{[^}]*\}[^}]*)*)\}', code, re.DOTALL)
        if not match:
            return None, None, None, None
        ret_type = match.group(1)
        func_name = match.group(2)
        params_str = match.group(3)
        body = match.group(4)
        params = {}
        if params_str.strip():
            for param in params_str.split(','):
                param = param.strip()
                parts = param.split()
                if len(parts) >= 2:
                    param_type = parts[0]
                    param_name = parts[1]
                    params[param_name] = param_type
        return ret_type, func_name, params, body

    def execute_unary_op(self, op: str, val: Any) -> Any:
        result = val
        if op == '-':
            if isinstance(val, (int, float)):
                result = -val
            elif isinstance(val, list):
                result = [-v for v in val]
        elif op == '!':
            if isinstance(val, bool):
                result = not val
            result = not bool(val)
        self.debug_print(f"[UNARY OP] operand={val}, op={op}, result={result}")
        return result

    def execute_binary_op(self, op: str, left: Any, right: Any) -> Any:
        if left is None or right is None:
            result = None
            self.debug_print(f"[BINARY OP] left={left}, right={right}, op={op}, result={result}")
            return None
        if op == '+':
            if isinstance(left, list) and isinstance(right, list):
                result = [l + r for l, r in zip(left, right)]
            else:
                result = left + right
        elif op == '-':
            if isinstance(left, list) and isinstance(right, list):
                result = [l - r for l, r in zip(left, right)]
            else:
                result = left - right
        elif op == '*':
            if isinstance(left, list) and isinstance(right, (int, float)):
                result = [v * right for v in left]
            elif isinstance(right, list) and isinstance(left, (int, float)):
                result = [v * left for v in right]
            else:
                result = left * right
        elif op == '/':
            if isinstance(left, list):
                result = [v / right for v in left]
            else:
                result = left / right
        elif op == '.':
            result = (left, right)
        else:
            result = None
        self.debug_print(f"[BINARY OP] left={left}, right={right}, op={op}, result={result}")
        return result

    def transpose_matrix(self, m: List[List[float]]) -> List[List[float]]:
        if len(m) == 4:
            return [[m[j][i] for j in range(4)] for i in range(4)]
        elif len(m) == 3:
            return [[m[j][i] for j in range(3)] for i in range(3)]
        return m

    def mul_matrix_vector(self, m: List[List[float]], v: List[float]) -> List[float]:
        if not v or any(x is None for x in v):
            return [0, 0, 0, 0]
        if not m:
            return [0, 0, 0, 0]
        num_cols = len(m[0]) if m else 0
        result = []
        for j in range(num_cols):
            s = sum(v[i] * m[i][j] for i in range(len(v)))
            result.append(s)
        return result

    def mul_matrix_matrix(self, a: List[List[float]], b: List[List[float]]) -> List[List[float]]:
        n = len(a)
        result = [[0.0] * n for _ in range(n)]
        for i in range(n):
            for j in range(n):
                for k in range(n):
                    result[i][j] += a[i][k] * b[k][j]
        return result

    def length_vec(self, v: List[float]) -> float:
        return math.sqrt(sum(x * x for x in v))

    def normalize_vec(self, v: List[float]) -> List[float]:
        l = self.length_vec(v)
        if l < 1e-8:
            return v
        return [x / l for x in v]

    def dot_product(self, a: List[float], b: List[float]) -> float:
        if not isinstance(a, list) or not isinstance(b, list):
            return 0.0
        if len(a) != len(b):
            return 0.0
        return sum(x * y for x, y in zip(a, b))

    def reflect_vec(self, I: List[float], N: List[float]) -> List[float]:
        if not isinstance(I, list) or not isinstance(N, list):
            return [0, 0, 0]
        dot = self.dot_product(N, I)
        result = []
        for i_val, n_val in zip(I, N):
            result.append(i_val - 2 * n_val * dot)
        return result

    def find_top_level_comma(self, expr: str) -> int:
        depth = 0
        for i, char in enumerate(expr):
            if char == '(':
                depth += 1
            elif char == ')':
                depth -= 1
            elif char == ',' and depth == 0:
                return i
        return -1

    def evaluate_expression(self, expr: str, local_vars: Dict[str, Any]) -> Any:
        expr = expr.strip()
        if not expr:
            return None

        if expr == 'return':
            return None

        # Check if expression is a simple function call or needs syntax tree parsing
        if re.match(r'\w+\s*\(', expr) and expr.strip().endswith(')'):
            if not any(op in expr for op in ['+', '-', '*', '/', '==', '!=', '<', '>', '<=', '>=', '||', '&&']):
                tree = self.syntax_parser.parse(expr)
                self.debug_print(f"[SYNTAX TREE]\n{tree}")
                return self.evaluate_syntax_tree(tree, local_vars)

        if expr.startswith('return '):
            return self.evaluate_expression(expr[7:], local_vars)

        if '||' in expr:
            self.debug_print(f"[EVAL] OR: {expr}")
            parts = expr.split('||')
            for p in parts:
                val = self.evaluate_expression(p.strip(), local_vars)
                if val:
                    self.debug_print(f"[EVAL] OR result: True")
                    return True
            self.debug_print(f"[EVAL] OR result: False")
            return False

        if '&&' in expr:
            self.debug_print(f"[EVAL] AND: {expr}")
            parts = expr.split('&&')
            for p in parts:
                val = self.evaluate_expression(p.strip(), local_vars)
                if not val:
                    self.debug_print(f"[EVAL] AND result: False")
                    return False
            self.debug_print(f"[EVAL] AND result: True")
            return True

        if '?' in expr and expr.count('?') == 1 and expr.count(':') == 1:
            self.debug_print(f"[EVAL] TERNARY: {expr}")
            match = re.match(r'(.+?)\s*\?\s*(.+?)\s*:\s*(.+)', expr)
            if match:
                cond = self.evaluate_expression(match.group(1), local_vars)
                if cond:
                    self.debug_print(f"[EVAL] TERNARY true branch")
                    return self.evaluate_expression(match.group(2), local_vars)
                else:
                    self.debug_print(f"[EVAL] TERNARY false branch")
                    return self.evaluate_expression(match.group(3), local_vars)

        if '<=' in expr and not re.search(r'[<>=!<]=', expr[:-2]):
            self.debug_print(f"[EVAL] LTE: {expr}")
            match = re.match(r'(.+?)\s*<=\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                self.debug_print(f"[EVAL] LTE result: {left} <= {right} = {left <= right}")
                return left <= right

        if '>=' in expr and not re.search(r'[<>][>=]', expr):
            self.debug_print(f"[EVAL] GTE: {expr}")
            match = re.match(r'(.+?)\s*>=\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                self.debug_print(f"[EVAL] GTE result: {left} >= {right} = {left >= right}")
                return left >= right

        if '<' in expr and not re.search(r'<=', expr):
            self.debug_print(f"[EVAL] LT: {expr}")
            match = re.match(r'(.+?)\s*<\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                self.debug_print(f"[EVAL] LT result: {left} < {right} = {left < right}")
                return left < right

        if '>' in expr and not re.search(r'>=', expr):
            self.debug_print(f"[EVAL] GT: {expr}")
            match = re.match(r'(.+?)\s*>\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                self.debug_print(f"[EVAL] GT result: {left} > {right} = {left > right}")
                return left > right

        if '==' in expr:
            self.debug_print(f"[EVAL] EQ: {expr}")
            match = re.match(r'(.+?)\s*==\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                self.debug_print(f"[EVAL] EQ result: {left} == {right} = {left == right}")
                return left == right

        if '!=' in expr:
            self.debug_print(f"[EVAL] NEQ: {expr}")
            match = re.match(r'(.+?)\s*!=\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                self.debug_print(f"[EVAL] NEQ result: {left} != {right} = {left != right}")
                return left != right

        if re.match(r'-\s*\w', expr):
            self.debug_print(f"[EVAL] UNARY NEG: {expr}")
            match = re.match(r'-\s*(\w+)', expr)
            if match:
                val = self.get_value(match.group(1), local_vars)
                result = self.execute_unary_op('-', val)
                self.debug_print(f"[EVAL] UNARY NEG result: -{val} = {result}")
                return result

        if expr.startswith('!'):
            self.debug_print(f"[EVAL] NOT: {expr}")
            val = self.evaluate_expression(expr[1:], local_vars)
            result = self.execute_unary_op('!', val)
            self.debug_print(f"[EVAL] NOT result: not {val} = {result}")
            return result

        if expr.startswith('-') and len(expr) > 1 and expr[1] != ' ':
            self.debug_print(f"[EVAL] UNARY SUB: {expr}")
            match = re.match(r'-(.+)', expr)
            if match:
                val = self.evaluate_expression(match.group(1), local_vars)
                result = self.execute_unary_op('-', val)
                self.debug_print(f"[EVAL] UNARY SUB result: -{val} = {result}")
                return result

        # =====================================================================
        # 向量构造函数: float2/float3/float4
        # 正则 r'float[234]\s*\(' 匹配以下形式的字符串:
        #   - "float2("  -> 创建二维向量
        #   - "float3("  -> 创建三维向量
        #   - "float4("  -> 创建四维向量
        # 例如: "float3(1.0, 2.0, 3.0)" 或 "float4(float3(1,2,3), 4.0)"
        # =====================================================================
        if re.match(r'float[234]\s*\(', expr):
            self.debug_print(f"[EVAL] FLOAT234: {expr}")
            match = re.match(r'float[234]\s*\(([^)]+)\)', expr)
            if match:
                args_str = match.group(1)
                args = []
                depth = 0
                current_arg = ''
                # 手动解析参数，处理嵌套括号的情况
                # 例如: float3(float2(1,2), 3, 4) 会被正确解析
                for char in args_str:
                    if char == ',' and depth == 0:
                        args.append(current_arg.strip())
                        current_arg = ''
                    else:
                        if char == '(':
                            depth += 1
                        elif char == ')':
                            depth -= 1
                        current_arg += char
                if current_arg.strip():
                    args.append(current_arg.strip())
                # 对每个参数递归求值并展平嵌套列表
                result = []
                for arg in args:
                    val = self.evaluate_expression(arg, local_vars)
                    if isinstance(val, list):
                        result.extend(val)
                    else:
                        result.append(val)
                self.debug_print(f"[EVAL] FLOAT234 result: {result}")
                return result

        # =====================================================================
        # 矩阵运算: transpose - 转置矩阵 (only if transpose is the main operation)
        # =====================================================================
        if re.match(r'transpose\s*\(', expr):
            self.debug_print(f"[EVAL] TRANSPOSE: {expr}")
            match = re.search(r'transpose\s*\(([^)]+)\)', expr)
            if match:
                val = self.get_value(match.group(1), local_vars)
                if val is None:
                    self.debug_print(f"[EVAL] WARNING: val is None for {expr}")
                    return None
                result = self.transpose_matrix(val)
                self.debug_print(f"[EVAL] TRANSPOSE result: {result}")
                return result

        # =====================================================================
        # normalize - 归一化向量
        # =====================================================================
        if 'normalize' in expr:
            self.debug_print(f"[EVAL] NORMALIZE: {expr}")
            match = re.search(r'normalize\s*\(([^)]+)\)', expr)
            if match:
                val = self.get_value(match.group(1), local_vars)
                if val is None:
                    self.debug_print(f"[EVAL] WARNING: val is None for {expr}")
                    return None
                if isinstance(val, list):
                    result = self.normalize_vec(val)
                    self.debug_print(f"[EVAL] NORMALIZE result: {result}")
                    return result
                return val

        # =====================================================================
        # length - 计算向量长度
        # =====================================================================
        if 'length' in expr:
            self.debug_print(f"[EVAL] LENGTH: {expr}")
            match = re.search(r'length\s*\(([^)]+)\)', expr)
            if match:
                val = self.get_value(match.group(1), local_vars)
                if val is None:
                    self.debug_print(f"[EVAL] WARNING: val is None for {expr}")
                    return None
                result = self.length_vec(val)
                self.debug_print(f"[EVAL] LENGTH result: {result}")
                return result

        # =====================================================================
        # dot - 向量点积
        # 手动解析逗号位置（处理嵌套括号）
        # =====================================================================
        if 'dot' in expr:
            self.debug_print(f"[EVAL] DOT: {expr}")
            depth = 0
            comma_pos = -1
            for i, char in enumerate(expr):
                if char == '(':
                    depth += 1
                elif char == ')':
                    depth -= 1
                elif char == ',' and depth == 0:
                    comma_pos = i
                    break
            if comma_pos > 0:
                arg1 = expr[4:comma_pos].strip()
                arg2 = expr[comma_pos+1:].strip().rstrip(')')
                a = self.evaluate_expression(arg1, local_vars)
                b = self.evaluate_expression(arg2, local_vars)
                if a is None or b is None:
                    self.debug_print(f"[EVAL] WARNING: arg is None for DOT: a={a}, b={b}")
                    return None
                result = self.dot_product(a, b)
                self.debug_print(f"[EVAL] DOT result: {result}")
                return result
            match = re.match(r'dot\s*\(([^,]+),\s*([^)]+)\)', expr)
            if match:
                a = self.get_value(match.group(1), local_vars)
                b = self.get_value(match.group(2), local_vars)
                if a is None or b is None:
                    self.debug_print(f"[EVAL] WARNING: arg is None for DOT: a={a}, b={b}")
                    return None
                result = self.dot_product(a, b)
                self.debug_print(f"[EVAL] DOT result: {result}")
                return result

        # =====================================================================
        # reflect - 反射向量计算 (I - 2 * dot(N, I) * N)
        # =====================================================================
        if 'reflect' in expr:
            self.debug_print(f"[EVAL] REFLECT: {expr}")
            match = re.match(r'reflect\s*\(([^,]+),\s*([^)]+)\)', expr)
            if match:
                I = self.get_value(match.group(1), local_vars)
                N = self.get_value(match.group(2), local_vars)
                if I is None or N is None:
                    self.debug_print(f"[EVAL] WARNING: arg is None for REFLECT: I={I}, N={N}")
                    return None
                result = self.reflect_vec(I, N)
                self.debug_print(f"[EVAL] REFLECT result: {result}")
                return result

        # =====================================================================
        # max - 取两个值中的最大值
        # =====================================================================
        if 'max' in expr:
            self.debug_print(f"[EVAL] MAX: {expr}")
            depth = 0
            comma_pos = -1
            for i, char in enumerate(expr):
                if char == '(':
                    depth += 1
                elif char == ')':
                    depth -= 1
                elif char == ',' and depth == 0:
                    comma_pos = i
                    break
            if comma_pos > 0:
                arg1 = expr[4:comma_pos].strip()
                arg2 = expr[comma_pos+1:].strip().rstrip(')')
                a = self.evaluate_expression(arg1, local_vars)
                b = self.evaluate_expression(arg2, local_vars)
                if a is None or b is None:
                    self.debug_print(f"[EVAL] WARNING: arg is None for MAX: a={a}, b={b}")
                    return None
                result = max(a, b)
                self.debug_print(f"[EVAL] MAX result: {result}")
                return result

        # =====================================================================
        # mul - 矩阵乘法 (矩阵 × 向量, 支持 4x4 和 3x3)
        # =====================================================================
        if 'mul' in expr:
            self.debug_print(f"[EVAL] MUL: {expr}")
            depth = 0
            comma_pos = -1
            for i, char in enumerate(expr):
                if char == '(':
                    depth += 1
                elif char == ')':
                    depth -= 1
                elif char == ',' and depth == 0:
                    comma_pos = i
                    break
            if comma_pos > 0:
                arg1 = expr[4:comma_pos].strip()
                arg2 = expr[comma_pos+1:].strip().rstrip(')')
                left = self.evaluate_expression(arg1, local_vars)
                right = self.evaluate_expression(arg2, local_vars)
                if left is None or right is None:
                    self.debug_print(f"[EVAL] WARNING: arg is None for MUL: left={left}, right={right}")
                    return None
                if isinstance(left, list) and isinstance(right, list):
                    if len(left) == 4 and len(right) == 4:
                        result = self.mul_matrix_vector(right, left)
                        self.debug_print(f"[EVAL] MUL result: {result}")
                        return result
                    elif len(left) == 3 and len(right) == 3:
                        result = self.mul_matrix_vector(right, left)
                        self.debug_print(f"[EVAL] MUL result: {result}")
                        return result
                return None

        # =====================================================================
        # pow - 幂运算
        # =====================================================================
        if 'pow' in expr:
            self.debug_print(f"[EVAL] POW: {expr}")
            match = re.match(r'pow\s*\(([^,]+),\s*([^)]+)\)', expr)
            if match:
                base = self.evaluate_expression(match.group(1), local_vars)
                exp = self.evaluate_expression(match.group(2), local_vars)
                if base is None or exp is None:
                    self.debug_print(f"[EVAL] WARNING: arg is None for POW: base={base}, exp={exp}")
                    return None
                result = math.pow(base, exp)
                self.debug_print(f"[EVAL] POW result: {result}")
                return result

        # =====================================================================
        # 类型转换和向量分量访问 (swizzle: .x, .y, .z, .w)
        # 匹配形式: (value).component 或 (type)expression
        # =====================================================================
        match = re.match(r'\(([^)]+)\)\s*(.+)', expr)
        if match:
            self.debug_print(f"[EVAL] CAST/SWIZZLE: {expr}")
            inner = self.evaluate_expression(match.group(1), local_vars)
            rest = match.group(2).strip()
            if rest.startswith('.'):
                field = rest[1:]
                if isinstance(inner, tuple):
                    return inner[1]
                if isinstance(inner, list) and field in ['x', 'y', 'z', 'w']:
                    idx = ['x', 'y', 'z', 'w'].index(field)
                    result = inner[idx] if idx < len(inner) else 0
                    self.debug_print(f"[EVAL] SWIZZLE .{field} result: {result}")
                    return result
                self.debug_print(f"[EVAL] CAST result: {inner}")
                return inner
            self.debug_print(f"[EVAL] Expression result: {inner}")
            return inner

        if '*' in expr:
            self.debug_print(f"[EVAL] MUL: {expr}")
            parts = expr.split('*')
            if len(parts) == 2:
                left = self.evaluate_expression(parts[0], local_vars)
                right = self.evaluate_expression(parts[1], local_vars)
                result = self.execute_binary_op('*', left, right)
                self.debug_print(f"[EVAL] MUL result: {left} * {right} = {result}")
                return result

        if '/' in expr:
            self.debug_print(f"[EVAL] DIV: {expr}")
            parts = expr.split('/')
            if len(parts) == 2:
                left = self.evaluate_expression(parts[0], local_vars)
                right = self.evaluate_expression(parts[1], local_vars)
                result = self.execute_binary_op('/', left, right)
                self.debug_print(f"[EVAL] DIV result: {left} / {right} = {result}")
                return result

        if '-' in expr:
            self.debug_print(f"[EVAL] SUB: {expr}")
            parts = expr.split('-', 1)
            if len(parts) == 2 and parts[0].strip():
                left = self.evaluate_expression(parts[0], local_vars)
                right = self.evaluate_expression(parts[1], local_vars)
                if left is None or right is None:
                    self.debug_print(f"[EVAL] WARNING: arg is None for SUB: left={left}, right={right}")
                    return None
                if isinstance(left, list) and isinstance(right, list):
                    result = [l - r for l, r in zip(left, right)]
                    self.debug_print(f"[EVAL] SUB result: {result}")
                    return result
                elif isinstance(left, list) and isinstance(right, (int, float)):
                    result = [v - right for v in left]
                    self.debug_print(f"[EVAL] SUB result: {result}")
                    return result
                elif isinstance(right, list) and isinstance(left, (int, float)):
                    result = [left - v for v in right]
                    self.debug_print(f"[EVAL] SUB result: {result}")
                    return result
                result = left - right
                self.debug_print(f"[EVAL] SUB result: {left} - {right} = {result}")
                return result

        if '+' in expr:
            self.debug_print(f"[EVAL] ADD: {expr}")
            parts = expr.split('+')
            result = self.evaluate_expression(parts[0], local_vars)
            if result is None:
                self.debug_print(f"[EVAL] WARNING: result is None for ADD expression")
                return None
            for p in parts[1:]:
                right = self.evaluate_expression(p, local_vars)
                if right is None:
                    self.debug_print(f"[EVAL] WARNING: right is None for ADD at '{p}'")
                    return None
                if isinstance(result, list) and isinstance(right, list):
                    result = [r + v for r, v in zip(result, right)]
                else:
                    result = result + right
            self.debug_print(f"[EVAL] ADD result: {result}")
            return result

        self.debug_print(f"[EVAL] GET_VALUE: {expr}")
        result = self.get_value(expr, local_vars)
        self.debug_print(f"[EVAL] GET_VALUE result: {result}")
        return result

    def evaluate_syntax_tree(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
        if node is None:
            return None

        if node.node_type == 'value':
            if node.value is None:
                return None
            return self.get_value(node.value, local_vars)

        elif node.node_type == 'binary_op':
            left = self.evaluate_syntax_tree(node.left, local_vars)
            right = self.evaluate_syntax_tree(node.right, local_vars)
            return self.execute_binary_op(node.value, left, right)

        elif node.node_type == 'unary_op':
            child = self.evaluate_syntax_tree(node.left, local_vars)
            return self.execute_unary_op(node.value, child)

        elif node.node_type == 'function':
            return self.execute_function_node(node, local_vars)

        elif node.node_type == 'cast':
            inner = self.evaluate_syntax_tree(node.left, local_vars)
            if inner is None:
                return None
            cast_type = node.value
            if cast_type == 'float3x3' and isinstance(inner, list) and len(inner) == 4:
                return [row[:3] for row in inner[:3]]
            return inner

        return None

    def execute_function_node(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
        func_name = node.value
        args = node.args

        if func_name == 'transpose':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            result = self.transpose_matrix(val)
            self.debug_print(f"[FUNC] transpose({val}) = {result}")
            return result

        elif func_name == 'normalize':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = self.normalize_vec(val)
                self.debug_print(f"[FUNC] normalize({val}) = {result}")
                return result
            return val

        elif func_name == 'length':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            result = self.length_vec(val)
            self.debug_print(f"[FUNC] length({val}) = {result}")
            return result

        elif func_name == 'dot':
            if len(args) != 2:
                return None
            a = self.evaluate_syntax_tree(args[0], local_vars)
            b = self.evaluate_syntax_tree(args[1], local_vars)
            if a is None or b is None:
                return None
            result = self.dot_product(a, b)
            self.debug_print(f"[FUNC] dot({a}, {b}) = {result}")
            return result

        elif func_name == 'reflect':
            if len(args) != 2:
                return None
            I = self.evaluate_syntax_tree(args[0], local_vars)
            N = self.evaluate_syntax_tree(args[1], local_vars)
            if I is None or N is None:
                return None
            result = self.reflect_vec(I, N)
            self.debug_print(f"[FUNC] reflect({I}, {N}) = {result}")
            return result

        elif func_name == 'max':
            if len(args) != 2:
                return None
            a = self.evaluate_syntax_tree(args[0], local_vars)
            b = self.evaluate_syntax_tree(args[1], local_vars)
            if a is None or b is None:
                return None
            result = max(a, b)
            self.debug_print(f"[FUNC] max({a}, {b}) = {result}")
            return result

        elif func_name == 'min':
            if len(args) != 2:
                return None
            a = self.evaluate_syntax_tree(args[0], local_vars)
            b = self.evaluate_syntax_tree(args[1], local_vars)
            if a is None or b is None:
                return None
            result = min(a, b)
            self.debug_print(f"[FUNC] min({a}, {b}) = {result}")
            return result

        elif func_name == 'pow':
            if len(args) != 2:
                return None
            base = self.evaluate_syntax_tree(args[0], local_vars)
            exp = self.evaluate_syntax_tree(args[1], local_vars)
            if base is None or exp is None:
                return None
            result = math.pow(base, exp)
            self.debug_print(f"[FUNC] pow({base}, {exp}) = {result}")
            return result

        elif func_name == 'abs':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [abs(v) for v in val]
            else:
                result = abs(val)
            self.debug_print(f"[FUNC] abs({val}) = {result}")
            return result

        elif func_name == 'sin':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [math.sin(v) for v in val]
            else:
                result = math.sin(val)
            self.debug_print(f"[FUNC] sin({val}) = {result}")
            return result

        elif func_name == 'cos':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [math.cos(v) for v in val]
            else:
                result = math.cos(val)
            self.debug_print(f"[FUNC] cos({val}) = {result}")
            return result

        elif func_name == 'mul':
            if len(args) != 2:
                return None
            left = self.evaluate_syntax_tree(args[0], local_vars)
            right = self.evaluate_syntax_tree(args[1], local_vars)
            if left is None or right is None:
                return None
            if isinstance(left, list) and isinstance(right, list):
                if len(left) == 4 and len(right) == 4:
                    result = self.mul_matrix_vector(right, left)
                    self.debug_print(f"[FUNC] mul(left={left}, right={right}) = {result}")
                    return result
                elif len(left) == 3 and len(right) == 3:
                    result = self.mul_matrix_vector(right, left)
                    self.debug_print(f"[FUNC] mul(left={left}, right={right}) = {result}")
                    return result
            return None

        elif func_name in ['float2', 'float3', 'float4']:
            result = []
            for arg in args:
                val = self.evaluate_syntax_tree(arg, local_vars)
                if isinstance(val, list):
                    result.extend(val)
                else:
                    result.append(val)
            self.debug_print(f"[FUNC] {func_name}(args={args}) = {result}")
            return result

        return None

    def get_value(self, name: str, local_vars: Dict[str, Any]) -> Any:
        name = name.strip()

        if name == 'true':
            return True
        if name == 'false':
            return False

        try:
            return float(name)
        except ValueError:
            pass

        if name in local_vars:
            val = local_vars[name]
            return val

        base_name = name.split('.')[0] if '.' in name else name

        for cb_name, cb_def in self.cbuffers.items():
            if isinstance(cb_def, CbufferDefinition):
                for field in cb_def.fields:
                    if field.name == base_name:
                        return field.data if field.data is not None else 0

        if '.' in name:
            parts = name.split('.')
            obj = local_vars.get(parts[0])
            if obj is None:
                obj = self.variables.get(parts[0])
            if obj is not None and len(parts) > 1:
                field = parts[1]
                if field == 'xyz' and isinstance(obj, list) and len(obj) >= 3:
                    return obj[:3]
                if field == 'rgb' and isinstance(obj, list) and len(obj) >= 3:
                    return obj[:3]
                if field in ['x', 'y', 'z', 'w'] and isinstance(obj, list):
                    idx = ['x', 'y', 'z', 'w'].index(field)
                    return obj[idx] if idx < len(obj) else 0
            return obj

        if name in self.variables:
            return self.variables[name]

        try:
            if '.' in name:
                parts = name.split('.')
                base = parts[0]
                for cb_name, cb_data in self.cbuffers.items():
                    if base in cb_data:
                        val = cb_data[base]
                        for p in parts[1:]:
                            if isinstance(val, list) and p in ['x', 'y', 'z', 'w']:
                                idx = ['x', 'y', 'z', 'w'].index(p)
                                val = val[idx] if idx < len(val) else 0
                            else:
                                break
                        return val
        except:
            pass

        return 0.0

    def execute_statement(self, stmt: str, local_vars: Dict[str, Any]):
        stmt = stmt.strip()
        if not stmt:
            return None

        self.debug_print(f"\n[STMT] Executing: {stmt}")
        input_snapshot = {k: v for k, v in local_vars.items() if k.startswith('input.') or k == 'output'}

        type_pattern = '|'.join(DATA_TYPE_LIST)
        pattern = rf'^({type_pattern})\s+(\w+)\s*=\s*(.+?);?$'
        match = re.match(pattern, stmt)
        if match:
            var_name = match.group(2)
            value = self.evaluate_expression(match.group(3), local_vars)
            local_vars[var_name] = value
            self.debug_print(f"[STMT] {stmt} => {var_name} = {value}")
            return None

        if 'output.' in stmt or 'output[' in stmt:
            match = re.match(r'output\.(\w+)\s*=\s*(.+)', stmt)
            if match:
                field_name = match.group(1)
                value_expr = match.group(2).rstrip(';').strip()
                value = self.evaluate_expression(value_expr, local_vars)
                if 'output' not in local_vars:
                    local_vars['output'] = {}
                local_vars['output'][field_name] = value
                self.debug_print(f"[STMT] {stmt} => output.{field_name} = {value}")
                return None

        if '=' in stmt and stmt.count('=') == 1:
            match = re.match(r'(\w+)\s*=\s*(.+?);?$', stmt)
            if match:
                var_name = match.group(1)
                value = self.evaluate_expression(match.group(2), local_vars)
                local_vars[var_name] = value
                self.debug_print(f"[STMT] {stmt} => {var_name} = {value}")
                return None

        self.debug_print(f"[STMT] {stmt} => (no assignment)")
        return None

    def execute_main_function(self, code: str, main_func: str, input_struct_name: str, row_index: int, data: Dict[str, Any]):
        input_struct = self.structs.get(input_struct_name)
        if not input_struct:
            print(f"Cannot find input_struct: {input_struct_name}\n")
            return None

        input_fields = {}
        for field in input_struct.fields:
            input_fields[field.name] = field.field_type

        func_signature_pattern = r'(\w+)\s+' + re.escape(main_func) + r'\s*\(\s*(\w+)\s+input\s*\)'
        func_signature_match = re.search(func_signature_pattern, code)
        if not func_signature_match:
            return None

        output_struct_name = func_signature_match.group(1)
        input_struct_name_from_func = func_signature_match.group(2)

        if output_struct_name not in self.structs:
            return None

        output_struct = self.structs[output_struct_name]
        output_fields = {}
        for field in output_struct.fields:
            output_fields[field.name] = field.field_type

        func_signature = rf'{output_struct_name}\s+{main_func}\s*\(\s*{input_struct_name_from_func}\s+input\s*\)'
        func_start = re.search(func_signature, code)
        if not func_start:
            return None

        open_brace_pos = func_start.end()
        brace_depth = 1
        pos = open_brace_pos
        while pos < len(code) and brace_depth > 0:
            if code[pos] == '{':
                brace_depth += 1
            elif code[pos] == '}':
                brace_depth -= 1
            pos += 1

        body = code[open_brace_pos+1:pos-1].strip()
        if body.startswith('{') and body.endswith('}'):
            body = body[1:-1].strip()

        local_vars = {'data': data}

        for field_name, field_value in data.items():
            local_vars[f'input.{field_name}'] = field_value

        output_obj = {}
        for field in output_fields:
            output_obj[field] = None

        local_vars['output'] = output_obj

        statements = []
        current_stmt = []
        brace_count = 0
        in_string = False

        for char in body:
            if char == '{':
                brace_count += 1
                current_stmt.append(char)
            elif char == '}':
                brace_count -= 1
                current_stmt.append(char)
            elif char == ';' and brace_count == 0 and not in_string:
                stmt = ''.join(current_stmt).strip()
                if stmt:
                    statements.append(stmt)
                current_stmt = []
            else:
                current_stmt.append(char)

        if current_stmt:
            stmt = ''.join(current_stmt).strip()
            if stmt:
                statements.append(stmt)

        ret_val = None
        for stmt in statements:
            if 'return' in stmt and 'output' in stmt:
                ret_val = local_vars.get('output')
                continue
            self.debug_print(f"\n=== INPUT DATA ===")
            for k, v in local_vars.items():
                if k.startswith('input.') or k == 'output':
                    self.debug_print(f"  {k} = {v}")
            self.debug_print(f"==================")
            self.execute_statement(stmt, local_vars)

        return ret_val

    def interpret(self, code: str):
        script_dir = os.path.dirname(os.path.abspath(__file__))

        struct_pattern = r'struct\s+\w+\s*\{[^}]+\}'
        for struct_match in re.finditer(struct_pattern, code):
            struct_def = self.parse_struct(struct_match.group())
            if struct_def:
                self.structs[struct_def.name] = struct_def

        cbuffer_pattern = r'cbuffer\s+\w+[^}]+\}'
        for cb_match in re.finditer(cbuffer_pattern, code, re.DOTALL):
            cb_def = self.parse_cbuffer(cb_match.group())
            if cb_def:
                self.cbuffers[cb_def.name] = cb_def

        for struct_name in self.structs:
            csv_path = os.path.join(script_dir, f'{struct_name}.csv')
            if os.path.exists(csv_path):
                self.load_struct_data_from_csv(struct_name, csv_path)

        for cb_name in self.cbuffers:
            csv_path = os.path.join(script_dir, f'{cb_name}.csv')
            if os.path.exists(csv_path):
                self.load_cbuffer_data_from_csv(cb_name, csv_path)

    def executeVS(self, code: str, main_func: str, vs_input: str):
        input_struct = self.structs.get(vs_input)
        if not input_struct:
            print(f"Cannot find vs input: {vs_input}\n")
            return None

        num_rows = 0
        for field in input_struct.fields:
            if field.data:
                num_rows = max(num_rows, len(field.data))

        results = []
        for row_index in range(num_rows):
            data = {}
            for field in input_struct.fields:
                if field.data and row_index < len(field.data):
                    data[field.name] = field.data[row_index]
            result = self.execute_main_function(code, main_func, vs_input, row_index, data)
            results.append(result)
        return results

    def executePS(self, code: str, main_func: str, ps_input: str):
        pass

    def load_struct_data_from_csv(self, struct_name: str, csv_path: str):
        if struct_name not in self.structs:
            return
        struct_def = self.structs[struct_name]
        rows = self.load_csv(csv_path)
        if not rows or len(rows) < 2:
            return

        header = rows[0]
        data_rows = rows[1:]

        field_col_indices = {}
        for i, col in enumerate(header):
            col_clean = col.strip()
            if '.' in col_clean:
                parts = col_clean.split('.')
                base_name = parts[0]
                suffix = parts[1]
                if base_name not in field_col_indices:
                    field_col_indices[base_name] = {}
                field_col_indices[base_name][suffix] = i

        for field in struct_def.fields:
            if field.semantic in field_col_indices:
                col_dict = field_col_indices[field.semantic]
                values = []
                for row in data_rows:
                    if 'x' in col_dict and 'y' in col_dict and 'z' in col_dict and 'w' in col_dict:
                        x = float(row[col_dict['x']].strip())
                        y = float(row[col_dict['y']].strip())
                        z = float(row[col_dict['z']].strip())
                        w = float(row[col_dict['w']].strip())
                        values.append([x, y, z, w])
                    elif 'x' in col_dict and 'y' in col_dict and 'z' in col_dict:
                        x = float(row[col_dict['x']].strip())
                        y = float(row[col_dict['y']].strip())
                        z = float(row[col_dict['z']].strip())
                        values.append([x, y, z])
                    elif 'x' in col_dict and 'y' in col_dict:
                        x = float(row[col_dict['x']].strip())
                        y = float(row[col_dict['y']].strip())
                        values.append([x, y])
                    else:
                        val_str = row[col_dict['x']].strip().strip('"')
                        values.append(self.parse_value_by_type(val_str, field.field_type))
                field.data = values
                print(f"Field '{field.semantic}' ({field.field_type}): {values[0] if values else 'N/A'}")

    def load_cbuffer_data_from_csv(self, cb_name: str, csv_path: str):
        if cb_name not in self.cbuffers:
            return
        cb_def = self.cbuffers[cb_name]
        rows = self.load_csv(csv_path)
        if not rows or len(rows) < 2:
            return

        header = rows[0]
        name_idx = header.index('Name') if 'Name' in header else -1
        value_idx = header.index('Value') if 'Value' in header else -1
        type_idx = header.index('Type') if 'Type' in header else -1

        if name_idx == -1 or value_idx == -1:
            return

        matrix_rows = {}
        scalar_vars = {}

        for row in rows[1:]:
            if len(row) <= max(name_idx, value_idx):
                continue
            var_name = row[name_idx].strip().strip('"')
            value_str = row[value_idx].strip().strip('"') if value_idx < len(row) else ''
            type_str = row[type_idx].strip().strip('"') if type_idx != -1 and type_idx < len(row) else ''

            # just skip null value string.
            # ['WorldViewProj', '', '0', 'float4x4 (column_major)']
            if value_str == '':
                continue

            if '.' in var_name:
                parts = var_name.split('.')
                base_name = parts[0]
                suffix = parts[1]
                if suffix.startswith('row'):
                    row_idx = int(suffix[3:])
                    if base_name not in matrix_rows:
                        matrix_rows[base_name] = {}
                    matrix_rows[base_name][row_idx] = (value_str, type_str)
            else:
                scalar_vars[var_name] = (value_str, type_str)

        for field in cb_def.fields:
            if field.name in matrix_rows:
                row_dict = matrix_rows[field.name]
                if all(i in row_dict for i in range(4)):
                    matrix = []
                    for i in range(4):
                        value_str, type_str = row_dict[i]
                        parts = value_str.split(',')
                        matrix.append([float(p.strip()) for p in parts[:4]])
                    field.data = matrix
            elif field.name in scalar_vars:
                value_str, type_str = scalar_vars[field.name]
                field.data = self.parse_value_by_type(value_str, type_str)

        cb_n = cb_name
        cb_d = cb_def
        print(f"Cbuffer {cb_n}:")
        for f in cb_d.fields:
            data = f.data
            ft = f.field_type
            if 'float4x4' in ft:
                print(f"  {f.name} ({ft}):")
                for row in data:
                    row_str = '  '.join(f"{v:12.5f}" for v in row)
                    print(f"    [{row_str}]")
            elif 'float3x3' in ft:
                print(f"  {f.name} ({ft}):")
                for row in data:
                    row_str = '  '.join(f"{v:12.5f}" for v in row)
                    print(f"    [{row_str}]")
            elif 'float4' in ft:
                print(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
            elif 'float3' in ft:
                print(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
            elif 'float2' in ft:
                print(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
            elif 'float' in ft:
                print(f"  {f.name} ({ft}): {data:.5f}")
            elif 'uint4' in ft:
                print(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'uint3' in ft:
                print(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'uint2' in ft:
                print(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'uint' in ft:
                print(f"  {f.name} ({ft}): {data}")
            elif 'int4' in ft:
                print(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'int3' in ft:
                print(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'int2' in ft:
                print(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'int' in ft:
                print(f"  {f.name} ({ft}): {data}")
            elif 'bool' in ft:
                print(f"  {f.name} ({ft}): {data}")
            else:
                print(f"  {f.name} ({ft}): {data}")


def main():
    interpreter = HLSLInterpreter()

    code = '''
    struct VS_INPUT {
        float3 Pos : POSITION;
        float3 Normal : NORMAL;
        float4 Color : COLOR;
        float2 TexCoord : TEXCOORD0;
    };
    struct VS_OUTPUT {
        float4 Pos : SV_POSITION;
        float4 Color : COLOR;
        float2 TexCoord : TEXCOORD0;
        float2 TexCoord2 : TEXCOORD1;
        float3 Normal : NORMAL;
        float3 WorldPos : WORLDPOS;
    };
    cbuffer MatrixBuffer : register(b0) {
        float4x4 WorldViewProj;
        float4x4 World;
    };
    cbuffer LightBuffer : register(b1) {
        float4 AmbientColor;
        float4 DiffuseColor;
        float4 SpecularColor;
        float3 LightPos;
        float LightRadius;
        float3 LightDir;
        float3 Attenuation;
        float OuterCone;
        float InnerCone;
    };
    cbuffer MaterialBuffer : register(b2) {
        float4 MaterialDiffuseColor;
        float4 MaterialAmbientColor;
        float4 MaterialSpecularColor;
        float4 MaterialEmissiveColor;
        float Shininess;
        uint ColorMaterialMode;
        float2 Padding;
    };
    cbuffer CameraBuffer : register(b3) {
        float3 cameraPos;
    };
    VS_OUTPUT main(VS_INPUT input) {
        VS_OUTPUT output;
        output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));
        float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));
        float3 nor = normalize(input.Normal);
        float3 normal = normalize(mul(nor, (float3x3)World));
        output.WorldPos = worldPos.xyz;
        output.Normal = normal;
        output.TexCoord = input.TexCoord;
        output.TexCoord2 = input.TexCoord;
        float3 lightDistant = LightPos.xyz - worldPos.xyz;
        float dist = length(lightDistant);
        float3 lightDir = normalize(lightDistant);
        float3 viewDir = cameraPos;
        float NdotL = max(dot(normal, lightDir), 0.0);
        float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;
        float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;
        float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;
        float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;
        float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;
        float3 R = reflect(lightDir, normal);
        float RdotV = max(dot(R, viewDir), 0.0);
        float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);
        float3 ambient = matAmbient.rgb * AmbientColor.rgb;
        float3 emissive = matEmissive.rgb;
        float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);
        float cond = dist <= LightRadius ? 1.0 : 0.0;
        output.Color = float4((ambient + diffuse * att + specular * att + emissive) * cond, 1.0);
        return output;
    }
    '''

    script_dir = os.path.dirname(os.path.abspath(__file__))
    interpreter.interpret(code)

    results = interpreter.executeVS(code, "main", "VS_INPUT")

    print("HLSL Interpreter Result:")
    print("=" * 40)
    if results:
        for idx, result in enumerate(results):
            print(f"\n--- Row {idx} ---")
            if result:
                for key, value in result.items():
                    if isinstance(value, list):
                        if len(value) == 4:
                            print(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]")
                        elif len(value) == 3:
                            print(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]")
                        elif len(value) == 2:
                            print(f"{key}: [{value[0]:.4f}, {value[1]:.4f}]")
                        else:
                            print(f"{key}: {value}")
                    else:
                        print(f"{key}: {value}")
    else:
        print("No result produced")

    if results and results[-1] and 'Color' in results[-1]:
        color = results[-1]['Color']
        if color and isinstance(color, list) and len(color) == 4:
            print("\nFinal Output Color (RGBA):")
            print(f"  R: {color[0]:.4f}")
            print(f"  G: {color[1]:.4f}")
            print(f"  B: {color[2]:.4f}")
            print(f"  A: {color[3]:.4f}")
        else:
            print(f"\nColor result: {color}")


if __name__ == '__main__':
    main()