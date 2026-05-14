import csv
import json
import math
import re
import os
from dataclasses import dataclass
from typing import Any, Dict, List, Union


DATA_TYPE_LIST = ['float4x4', 'float4', 'float3', 'float2', 'uint']


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
        elif 'float4' in field_type:
            return 16
        elif 'float3' in field_type:
            return 12
        elif 'float2' in field_type:
            return 8
        elif 'uint' in field_type:
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
        elif 'float4' in field_type:
            parts = value_str.split(',')
            return [float(p) for p in parts[:4]]
        elif 'float3' in field_type:
            parts = value_str.split(',')
            return [float(p) for p in parts[:3]]
        elif 'float2' in field_type:
            parts = value_str.split(',')
            return [float(p) for p in parts[:2]]
        elif 'uint' in field_type:
            return int(value_str)
        try:
            return float(value_str)
        except:
            return value_str

    def parse_type(self, type_str: str) -> str:
        type_str = type_str.strip()
        if type_str.startswith('float'):
            if 'x3' in type_str:
                return 'float3x3'
            elif 'x4' in type_str:
                return 'float4x4'
            return 'float'
        elif type_str.startswith('int'):
            return 'int'
        elif type_str.startswith('uint'):
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
        if op == '-':
            if isinstance(val, (int, float)):
                return -val
            elif isinstance(val, list):
                return [-v for v in val]
        elif op == '!':
            if isinstance(val, bool):
                return not val
            return not bool(val)
        return val

    def execute_binary_op(self, op: str, left: Any, right: Any) -> Any:
        if left is None or right is None:
            return None
        if op == '+':
            if isinstance(left, list) and isinstance(right, list):
                return [l + r for l, r in zip(left, right)]
            return left + right
        elif op == '-':
            if isinstance(left, list) and isinstance(right, list):
                return [l - r for l, r in zip(left, right)]
            return left - right
        elif op == '*':
            if isinstance(left, list) and isinstance(right, (int, float)):
                return [v * right for v in left]
            if isinstance(right, list) and isinstance(left, (int, float)):
                return [v * left for v in right]
            return left * right
        elif op == '/':
            if isinstance(left, list):
                return [v / right for v in left]
            return left / right
        elif op == '.':
            return (left, right)
        return None

    def transpose_matrix(self, m: List[List[float]]) -> List[List[float]]:
        if len(m) == 4:
            return [[m[j][i] for j in range(4)] for i in range(4)]
        elif len(m) == 3:
            return [[m[j][i] for j in range(3)] for i in range(3)]
        return m

    def mul_matrix_vector(self, m: List[List[float]], v: List[float]) -> List[float]:
        if not v or any(x is None for x in v):
            return [0, 0, 0, 0]
        result = []
        for row in m:
            s = sum(row[i] * v[i] for i in range(len(v)))
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

        if expr.startswith('return '):
            return self.evaluate_expression(expr[7:], local_vars)

        if '||' in expr:
            parts = expr.split('||')
            for p in parts:
                val = self.evaluate_expression(p.strip(), local_vars)
                if val:
                    return True
            return False

        if '&&' in expr:
            parts = expr.split('&&')
            for p in parts:
                val = self.evaluate_expression(p.strip(), local_vars)
                if not val:
                    return False
            return True

        if '?' in expr and expr.count('?') == 1 and expr.count(':') == 1:
            match = re.match(r'(.+?)\s*\?\s*(.+?)\s*:\s*(.+)', expr)
            if match:
                cond = self.evaluate_expression(match.group(1), local_vars)
                if cond:
                    return self.evaluate_expression(match.group(2), local_vars)
                else:
                    return self.evaluate_expression(match.group(3), local_vars)

        if '<=' in expr and not re.search(r'[<>=!<]=', expr[:-2]):
            match = re.match(r'(.+?)\s*<=\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                return left <= right

        if '>=' in expr and not re.search(r'[<>][>=]', expr):
            match = re.match(r'(.+?)\s*>=\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                return left >= right

        if '<' in expr and not re.search(r'<=', expr):
            match = re.match(r'(.+?)\s*<\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                return left < right

        if '>' in expr and not re.search(r'>=', expr):
            match = re.match(r'(.+?)\s*>\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                return left > right

        if '==' in expr:
            match = re.match(r'(.+?)\s*==\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                return left == right

        if '!=' in expr:
            match = re.match(r'(.+?)\s*!=\s*(.+)', expr)
            if match:
                left = self.evaluate_expression(match.group(1), local_vars)
                right = self.evaluate_expression(match.group(2), local_vars)
                return left != right

        if re.match(r'-\s*\w', expr):
            match = re.match(r'-\s*(\w+)', expr)
            if match:
                val = self.get_value(match.group(1), local_vars)
                return self.execute_unary_op('-', val)

        if expr.startswith('!'):
            val = self.evaluate_expression(expr[1:], local_vars)
            return self.execute_unary_op('!', val)

        if expr.startswith('-') and len(expr) > 1 and expr[1] != ' ':
            match = re.match(r'-(.+)', expr)
            if match:
                val = self.evaluate_expression(match.group(1), local_vars)
                return self.execute_unary_op('-', val)

        if re.match(r'float[234]\s*\(', expr):
            match = re.match(r'float[234]\s*\(([^)]+)\)', expr)
            if match:
                args_str = match.group(1)
                args = []
                depth = 0
                current_arg = ''
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
                result = []
                for arg in args:
                    val = self.evaluate_expression(arg, local_vars)
                    if isinstance(val, list):
                        result.extend(val)
                    else:
                        result.append(val)
                return result


            if 'transpose' in expr:
                match = re.search(r'transpose\s*\(([^)]+)\)', expr)
                if match:
                    val = self.get_value(match.group(1), local_vars)
                    if val is None:
                        return None
                    return self.transpose_matrix(val)

            if 'normalize' in expr:
                match = re.search(r'normalize\s*\(([^)]+)\)', expr)
                if match:
                    val = self.get_value(match.group(1), local_vars)
                    if val is None:
                        return None
                    if isinstance(val, list):
                        return self.normalize_vec(val)
                    return val

            if 'length' in expr:
                match = re.search(r'length\s*\(([^)]+)\)', expr)
                if match:
                    val = self.get_value(match.group(1), local_vars)
                    if val is None:
                        return None
                    return self.length_vec(val)

            if 'dot' in expr:
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
                        return None
                    return self.dot_product(a, b)
                match = re.match(r'dot\s*\(([^,]+),\s*([^)]+)\)', expr)
                if match:
                    a = self.get_value(match.group(1), local_vars)
                    b = self.get_value(match.group(2), local_vars)
                    if a is None or b is None:
                        return None
                    return self.dot_product(a, b)

            if 'reflect' in expr:
                match = re.match(r'reflect\s*\(([^,]+),\s*([^)]+)\)', expr)
                if match:
                    I = self.get_value(match.group(1), local_vars)
                    N = self.get_value(match.group(2), local_vars)
                    if I is None or N is None:
                        return None
                    return self.reflect_vec(I, N)

            if 'max' in expr:
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
                        return None
                    return max(a, b)

            if 'pow' in expr:
                match = re.match(r'pow\s*\(([^,]+),\s*([^)]+)\)', expr)
                if match:
                    base = self.evaluate_expression(match.group(1), local_vars)
                    exp = self.evaluate_expression(match.group(2), local_vars)
                    if base is None or exp is None:
                        return None
                    return math.pow(base, exp)

            match = re.match(r'\(([^)]+)\)\s*(.+)', expr)
            if match:
                inner = self.evaluate_expression(match.group(1), local_vars)
                rest = match.group(2).strip()
                if rest.startswith('.'):
                    field = rest[1:]
                    if isinstance(inner, tuple):
                        return inner[1]
                    if isinstance(inner, list) and field in ['x', 'y', 'z', 'w']:
                        idx = ['x', 'y', 'z', 'w'].index(field)
                        return inner[idx] if idx < len(inner) else 0
                    return inner
                return inner

        if '*' in expr:
            parts = expr.split('*')
            if len(parts) == 2:
                left = self.evaluate_expression(parts[0], local_vars)
                right = self.evaluate_expression(parts[1], local_vars)
                return self.execute_binary_op('*', left, right)

        if '/' in expr:
            parts = expr.split('/')
            if len(parts) == 2:
                left = self.evaluate_expression(parts[0], local_vars)
                right = self.evaluate_expression(parts[1], local_vars)
                return self.execute_binary_op('/', left, right)

        if '-' in expr:
            parts = expr.split('-', 1)
            if len(parts) == 2 and parts[0].strip():
                left = self.evaluate_expression(parts[0], local_vars)
                right = self.evaluate_expression(parts[1], local_vars)
                if left is None or right is None:
                    return None
                if isinstance(left, list) and isinstance(right, list):
                    return [l - r for l, r in zip(left, right)]
                elif isinstance(left, list) and isinstance(right, (int, float)):
                    return [v - right for v in left]
                elif isinstance(right, list) and isinstance(left, (int, float)):
                    return [left - v for v in right]
                return left - right

        if '+' in expr:
            parts = expr.split('+')
            result = self.evaluate_expression(parts[0], local_vars)
            if result is None:
                return None
            for p in parts[1:]:
                right = self.evaluate_expression(p, local_vars)
                if right is None:
                    return None
                if isinstance(result, list) and isinstance(right, list):
                    result = [r + v for r, v in zip(result, right)]
                else:
                    result = result + right
            return result

        return self.get_value(expr, local_vars)

    def get_value(self, name: str, local_vars: Dict[str, Any]) -> Any:
        name = name.strip()

        if name == 'true':
            return True
        if name == 'false':
            return False

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

        if stmt.startswith('float4 ') or stmt.startswith('float3 ') or stmt.startswith('float ') or stmt.startswith('float2 ') or stmt.startswith('int ') or stmt.startswith('uint ') or stmt.startswith('bool '):
            match = re.match(r'(?:float4|float3|float2|float|int|uint|bool)\s+(\w+)\s*=\s*(.+?);?$', stmt)
            if match:
                var_name = match.group(1)
                value = self.evaluate_expression(match.group(2), local_vars)
                local_vars[var_name] = value
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
                return None

        if '=' in stmt and stmt.count('=') == 1:
            match = re.match(r'(\w+)\s*=\s*(.+?);?$', stmt)
            if match:
                var_name = match.group(1)
                value = self.evaluate_expression(match.group(2), local_vars)
                local_vars[var_name] = value
                return None

        return None

    def execute_function(self, code: str, params: Dict[str, Any], input_data: Dict[str, Any]):
        struct_match = re.search(r'struct\s+VS_INPUT\s*\{([^}]+)\}', code)
        if struct_match:
            vs_input_fields = {}
            for line in struct_match.group(1).split(';'):
                line = line.strip()
                if not line:
                    continue
                parts = line.split(':')
                if len(parts) == 2:
                    type_and_name = parts[0].strip().split()
                    if len(type_and_name) == 2:
                        field_name = type_and_name[1]
                        vs_input_fields[field_name] = type_and_name[0]

        struct_match_out = re.search(r'struct\s+VS_OUTPUT\s*\{([^}]+)\}', code)
        vs_output_fields = {}
        if struct_match_out:
            for line in struct_match_out.group(1).split(';'):
                line = line.strip()
                if not line:
                    continue
                parts = line.split(':')
                if len(parts) == 2:
                    type_and_name = parts[0].strip().split()
                    if len(type_and_name) == 2:
                        vs_output_fields[type_and_name[1]] = type_and_name[0]

        vs_match = re.search(r'VS_OUTPUT\s+main\s*\(\s*VS_INPUT\s+input\s*\)\s*\{(.*?)\n\s*\};?\s*$', code, re.DOTALL)
        if not vs_match:
            vs_match = re.search(r'VS_OUTPUT\s+main\s*\(\s*VS_INPUT\s+input\s*\)\s*\{(.*?)^', code, re.DOTALL | re.MULTILINE)

        local_vars = {}
        for p_name, p_val in params.items():
            local_vars[p_name] = p_val

        for field_name, field_type in vs_input_fields.items():
            if field_name in input_data:
                local_vars[f'input.{field_name}'] = input_data[field_name]

        if 'input.Pos' in str(code):
            local_vars['input.Pos'] = input_data.get('Pos', [0,0,0])
        if 'input.Normal' in str(code):
            local_vars['input.Normal'] = input_data.get('Normal', [0,0,0])
        if 'input.Color' in str(code):
            local_vars['input.Color'] = input_data.get('Color', [0,0,0,0])
        if 'input.TexCoord' in str(code):
            local_vars['input.TexCoord'] = input_data.get('TexCoord', [0,0])

        if vs_match:
            body = vs_match.group(1)
        else:
            func_match = re.search(r'VS_OUTPUT\s+main\s*\([^)]*\)\s*\{(.*?)return output;\s*\}', code, re.DOTALL)
            if func_match:
                body = func_match.group(1)
            else:
                return None

        output_obj = {}
        for field in vs_output_fields:
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

        for stmt in statements:
            if 'return output' in stmt:
                continue
            self.execute_statement(stmt, local_vars)

        return local_vars.get('output') or local_vars.get('output.Color')

    def interpret(self, code: str, data: Dict[str, Any]):
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

        result = self.execute_function(code, {}, data.get('input', {}))
        return result

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

        if name_idx == -1 or value_idx == -1:
            return

        matrix_rows = {}
        scalar_vars = {}

        for row in rows[1:]:
            if len(row) <= max(name_idx, value_idx):
                continue
            var_name = row[name_idx].strip().strip('"')
            value_str = row[value_idx].strip().strip('"') if value_idx < len(row) else ''

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
                    matrix_rows[base_name][row_idx] = value_str
            else:
                scalar_vars[var_name] = value_str

        for field in cb_def.fields:
            if field.name in matrix_rows:
                row_dict = matrix_rows[field.name]
                if all(i in row_dict for i in range(4)):
                    matrix = []
                    for i in range(4):
                        parts = row_dict[i].split(',')
                        matrix.append([float(p.strip()) for p in parts[:4]])
                    field.data = matrix
            elif field.name in scalar_vars:
                field.data = self.parse_value_by_type(scalar_vars[field.name], field.field_type)

        for cb_n, cb_d in self.cbuffers.items():
            print(f"Cbuffer {cb_n}:")
            for f in cb_d.fields:
                print(f"  {f.name} ({f.field_type}): data={f.data}")


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
    json_path = os.path.join(script_dir, 'test_data.json')
    data = interpreter.load_json(json_path)
    result = interpreter.interpret(code, data)

    print("HLSL Interpreter Result:")
    print("=" * 40)
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

    if result and 'Color' in result:
        color = result['Color']
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