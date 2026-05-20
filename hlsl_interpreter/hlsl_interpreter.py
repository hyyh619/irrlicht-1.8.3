import csv
import json
import math
import re
import os
import time
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from typing import Any, Dict, List, Union, Optional

from hlsl_syntax_tree import SyntaxTreeNode, SyntaxTreeParser, _COMPILED_PATTERNS

try:
    from mesh_view import MeshView, VertexData
    MESHVIEW_AVAILABLE = True
except ImportError:
    MESHVIEW_AVAILABLE = False


DATA_TYPE_LIST = [
    'float4x4', 'float3x3',  # 矩阵类型
    'float4', 'float3', 'float2', 'float',  # 浮点向量/标量
    'uint4', 'uint3', 'uint2', 'uint',  # 无符号整数
    'int4', 'int3', 'int2', 'int',  # 有符号整数
    'bool'  # 布尔类型
]

D3D_PRIMITIVE_TOPOLOGY_UNDEFINED = 0
D3D_PRIMITIVE_TOPOLOGY_POINTLIST = 1
D3D_PRIMITIVE_TOPOLOGY_LINELIST = 2
D3D_PRIMITIVE_TOPOLOGY_LINESTRIP = 3
D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST = 4
D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP = 5
D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN = 6


@dataclass
class ShaderVariable:
    """着色器变量定义"""
    name: str       # 变量名
    type: str       # 变量类型
    value: Any      # 变量值


@dataclass
class FieldDefinition:
    """结构体或cbuffer的字段定义"""
    field_type: str      # 字段类型，如 float3, float4x4
    name: str           # 字段名
    semantic: str       # 语义名称，如 POSITION, NORMAL
    data: List[Any] = None  # 字段数据值

@dataclass
class StructDefinition:
    """HLSL结构体定义"""
    name: str                     # 结构体名称
    fields: List[FieldDefinition]  # 结构体字段列表

@dataclass
class CbufferDefinition:
    """HLSL常量缓冲区定义"""
    name: str                     # cbuffer名称
    fields: List[FieldDefinition]  # cbuffer字段列表


class HLSLInterpreter:
    """
    HLSL解释器 - 解析和执行HLSL着色器代码
    支持: 结构体定义、cbuffer定义、函数解析、表达式求值
    """

    def __init__(self,
                log_to_file: bool = True,
                log_file_path: str = "hlsl_interpreter.log",
                print_sequence: int = 1,
                log_file_mode: str = 'a',
                printSyntaxTree: bool = True,
                print_interpreter_result: bool = True,
                max_workers: int = 1,
                primitive_topology: int = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
                log_cache_size: int = 10 * 1024 * 1024):
        self.structs: Dict[str, StructDefinition] = {}      # 解析的结构体定义
        self.cbuffers: Dict[str, CbufferDefinition] = {}    # 解析的cbuffer定义
        self.variables: Dict[str, Any] = {}                 # 全局变量
        self.debug = True                                   # 调试模式开关
        self.printSyntaxTree = printSyntaxTree              # 打印语法树开关
        self.syntax_parser = SyntaxTreeParser()             # 语法树解析器
        self.log_to_file = log_to_file                      # 是否输出到文件
        self.log_file_path = log_file_path                  # 日志文件路径
        self.log_file_mode = log_file_mode                  # 文件模式: 'a'=追加, 'w'=覆盖
        self.print_sequence = max(1, print_sequence)        # 打印间隔频率
        self.print_interpreter_result = print_interpreter_result  # 是否打印HLSL Interpreter Result
        self._eval_counter = 0                              # evaluate_syntax_tree执行计数器
        self._should_print = True                           # 当前是否应该打印
        self._log_file = None                               # 日志文件句柄
        self.hlsl_code = None                               # 加载的HLSL代码
        self.max_workers = max_workers                       # 线程池最大工作线程数
        self._parsed_func_cache = {}                         # 解析过的函数体缓存
        self.primitive_topology = primitive_topology         # 图元拓扑类型
        self._mesh_view = None                               # MeshView实例(用于显示输入和输出)
        self._mesh_view_enabled = False                      # 是否启用MeshView
        self._log_cache = []                                 # 日志缓存
        self._log_cache_size = log_cache_size                # 日志缓存大小(字节)
        self._log_cache_bytes = 0                            # 当前缓存已用字节数

        # 预编译的正则表达式模式字典
        type_pattern = '|'.join(DATA_TYPE_LIST)
        self.patterns: Dict[str, re.Pattern] = {
            # execute_statement: 变量声明语句，如 "float4 pos = ...;"
            'variable_declaration': re.compile(rf'^({type_pattern})\s+(\w+)\s*=\s*(.+?);?$'),

            # execute_statement: output字段赋值语句，如 "output.Color = ...;" 或 "output.Color.r = ...;"
            'output_field_assignment': re.compile(r'output\.(\w+)(?:\.([xyzwrgba]+))?\s*=\s*(.+)'),

            # execute_statement: 一般赋值语句，如 "var = ...;"
            'simple_assignment': re.compile(r'(\w+)\s*=\s*(.+?);?$'),

            # execute_statement: if条件语句，如 "if(condition) { ... }"
            'if_statement': re.compile(r'if\s*\((.+?)\)\s*(.+)$', re.DOTALL),

            # parse_struct: 结构体定义，如 "struct VS_INPUT { ... }"
            'struct_definition': re.compile(r'struct\s+(\w+)\s*\{([^}]+)\}'),

            # parse_cbuffer: cbuffer定义，如 "cbuffer MyBuffer : register(b0) { ... }"
            'cbuffer_definition': re.compile(r'cbuffer\s+(\w+)\s*:.*?\{([^}]+)\}', re.DOTALL),

            # parse_function: 函数定义，如 "float4 main(VS_INPUT input) { ... }"
            'function_definition': re.compile(r'(\w+)\s+(\w+)\s*\(([^)]*)\)\s*\{([^}]+(?:\{[^}]*\}[^}]*)*)\}', re.DOTALL),

            # load_hlsl_code_from_file / executeVS: 查找struct定义（用于finditer）
            'struct_finditer': re.compile(r'struct\s+\w+\s*\{[^}]+\}'),

            # load_hlsl_code_from_file: 查找cbuffer定义（用于finditer）
            'cbuffer_finditer': re.compile(r'cbuffer\s+\w+[^}]+\}'),
        }

        if self.log_to_file and self.log_file_path:
            self._log_file = open(self.log_file_path, self.log_file_mode, encoding='utf-8')

    def __del__(self):
        """对象销毁时关闭日志文件"""
        if self._log_cache:
            self._flush_log_cache()
        if self._log_file:
            self._log_file.close()
            self._log_file = None

    def enable_mesh_view(self, enable: bool = True):
        """
        启用或禁用MeshView
        enable: 是否启用MeshView
        """
        if enable and not MESHVIEW_AVAILABLE:
            self.log_output("Warning: MeshView not available (tkinter may not be installed)")
            return
        self._mesh_view_enabled = enable
        if enable and self._mesh_view is None:
            self._mesh_view = MeshView(title="HLSL Interpreter - Input/Output Mesh")

        self.log_output(f"MeshView {'enabled' if enable else 'disabled'}")

    def show_input_mesh(self, vs_input: str, row_index: int = None):
        """
        显示当前输入的mesh数据
        vs_input: 输入结构体名
        row_index: 指定行索引，如果为None则显示所有行
        """
        if not self._mesh_view_enabled or not MESHVIEW_AVAILABLE:
            return

        input_struct = self.structs.get(vs_input)
        if not input_struct:
            self.log_output(f"Cannot find vs input struct: {vs_input}")
            return

        positions = []
        normals = []
        colors = []

        num_rows = 0
        for field in input_struct.fields:
            if field.data:
                num_rows = max(num_rows, len(field.data))

        if row_index is not None:
            num_rows = min(row_index + 1, num_rows)
            row_start = row_index
            row_end = row_index + 1
        else:
            row_start = 0
            row_end = num_rows

        for field in input_struct.fields:
            if not field.data:
                continue
            if 'pos' in field.name.lower() or 'position' in field.name.lower() or field.semantic.upper() == 'POSITION':
                for i in range(row_start, min(row_end, len(field.data))):
                    pos = field.data[i]
                    if isinstance(pos, list) and len(pos) >= 3:
                        positions.append(pos[:3])
            elif 'normal' in field.name.lower() or field.semantic.upper() == 'NORMAL':
                for i in range(row_start, min(row_end, len(field.data))):
                    norm = field.data[i]
                    if isinstance(norm, list) and len(norm) >= 3:
                        normals.append(norm[:3])
            elif 'color' in field.name.lower() or field.semantic.upper() == 'COLOR':
                for i in range(row_start, min(row_end, len(field.data))):
                    col = field.data[i]
                    if isinstance(col, list) and len(col) >= 4:
                        colors.append(col[:4])

        if positions:
            self._mesh_view.clear()
            self._mesh_view.set_primitive_topology(self.primitive_topology)
            self._mesh_view.set_input_data(positions, normals if normals else None, colors if colors else None)
            self._mesh_view.show(blocking=False)
        else:
            self.log_output(f"No position data found in {vs_input}")

    def show_result_mesh(self, results: List[Dict[str, Any]], output_struct_name: str = None):
        """
        显示executeVS执行完毕后的results mesh数据
        results: executeVS返回的输出结构体字典列表
        output_struct_name: 输出结构体名(可选)
        """
        if not self._mesh_view_enabled or not MESHVIEW_AVAILABLE:
            return

        if not results:
            self.log_output("No results to display in result mesh view")
            return

        positions = []
        normals = []
        colors = []

        # not pos keywords
        notPosWords = ['worldpos']

        for result in results:
            if not result:
                continue
            for key, value in result.items():
                key_lower = key.lower()
                if 'pos' in key_lower or 'position' in key_lower or key.upper() == 'SV_POSITION':
                    if key_lower in notPosWords:
                        continue

                    if isinstance(value, list) and len(value) >= 3:
                        positions.append(value[:3])
                elif 'normal' in key_lower:
                    if isinstance(value, list) and len(value) >= 3:
                        normals.append(value[:3])
                elif 'color' in key_lower:
                    if isinstance(value, list) and len(value) >= 4:
                        colors.append(value[:4])

        if positions:
            # self._mesh_view.clear()
            self._mesh_view.set_primitive_topology(self.primitive_topology)
            self._mesh_view.set_output_data(positions, normals if normals else None, colors if colors else None)
            self._mesh_view.show(blocking=False)
            self.log_output(f"Result mesh displayed: {len(positions)} vertices")
        else:
            self.log_output("No position data found in results")

    def _flush_log_cache(self):
        """将缓存中的日志写入文件"""
        if self._log_cache and self._log_file:
            self._log_file.write(''.join(self._log_cache))
            self._log_file.flush()
            self._log_cache = []
            self._log_cache_bytes = 0

    def log_output(self, *args, **kwargs):
        """输出到stdout和日志文件"""
        msg = ' '.join(str(arg) for arg in args)
        print(*args, **kwargs)
        if self.log_to_file and self._log_file:
            msg_bytes = (msg + '\n').encode('utf-8')
            if self._log_cache_bytes + len(msg_bytes) >= self._log_cache_size:
                self._flush_log_cache()
            self._log_cache.append(msg + '\n')
            self._log_cache_bytes += len(msg_bytes)

    def debug_print(self, msg: str):
        """调试打印"""
        if self.debug and self._should_print:
            self.log_output(msg)

    def _format_float(self, val):
        """
        格式化浮点数输出
        val: 值
        返回: 格式化后的字符串(保留4位小数)
        """
        if isinstance(val, float):
            return f"{val:.4f}"
        if isinstance(val, list):
            if val and isinstance(val[0], list):
                return self._format_matrix(val)
            return [self._format_float(v) for v in val]
        return val

    def _format_matrix(self, val):
        """
        格式化矩阵输出
        val: 矩阵(二维列表)
        返回: 格式化后的矩阵字符串
        """
        if not val or not isinstance(val[0], list):
            return str(val)
        formatted = [[self._format_float(v) for v in row] for row in val]
        col_widths = [0] * len(formatted[0])
        for row in formatted:
            for j, cell in enumerate(row):
                col_widths[j] = max(col_widths[j], len(cell))
        lines = []
        for row in formatted:
            cells = [cell.rjust(col_widths[j]) for j, cell in enumerate(row)]
            lines.append("[" + " ".join(cells) + "]")
        return "\n".join(lines)

    def _format_value(self, val):
        """格式化值输出(矩阵或标量/向量)"""
        if isinstance(val, list) and val and isinstance(val[0], list):
            return self._format_matrix(val)
        return self._format_float(val)

    def _format_msg(self, *args):
        """格式化多个值用于调试输出"""
        formatted = []
        for arg in args:
            formatted.append(self._format_float(arg))
        return formatted

    def load_json(self, filepath: str):
        """从JSON文件加载数据"""
        with open(filepath, 'r') as f:
            data = json.load(f)
        return data

    def load_csv(self, filepath: str) -> List[List[str]]:
        """从CSV文件加载数据，返回二维列表"""
        rows = []
        with open(filepath, 'r') as f:
            reader = csv.reader(f)
            for row in reader:
                rows.append(row)
        return rows

    def get_type_size(self, field_type: str) -> int:
        """
        获取HLSL类型的大小(字节数)
        field_type: HLSL类型名，如 float4x4, float3, int
        返回: 类型占用的字节数
        """
        if 'float4x4' in field_type:
            return 64  # 4x4矩阵 = 16 floats * 4 bytes
        elif 'float3x3' in field_type:
            return 36  # 3x3矩阵 = 9 floats * 4 bytes
        elif 'float4' in field_type:
            return 16  # 4 floats * 4 bytes
        elif 'float3' in field_type:
            return 12  # 3 floats * 4 bytes
        elif 'float2' in field_type:
            return 8   # 2 floats * 4 bytes
        elif 'float' in field_type:
            return 4   # 1 float * 4 bytes
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
        """
        根据类型解析字符串值为对应类型的Python对象
        value_str: 值的字符串表示
        field_type: HLSL类型名
        返回: 解析后的值
        """
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
        """
        解析HLSL类型字符串为标准类型名
        type_str: 类型字符串，如 "float4x4", "float3", "int2"
        返回: 标准类型名
        """
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
        """
        解析HLSL结构体定义
        code: 结构体代码，如 "struct VS_INPUT { float3 Pos : POSITION; }"
        返回: StructDefinition对象
        """
        match = self.patterns['struct_definition'].search(code)
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
        """
        解析HLSL常量缓冲区定义
        code: cbuffer代码
        返回: CbufferDefinition对象
        """
        match = self.patterns['cbuffer_definition'].search(code)
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
        """
        解析HLSL函数定义
        code: 函数代码，如 "float4 main(VS_INPUT input) { ... }"
        返回: (返回类型, 函数名, 参数字典, 函数体) 元组
        """
        match = self.patterns['function_definition'].search(code)
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
        """
        执行一元运算符
        op: 运算符 '-' 或 '!'
        val: 操作数
        """
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
        self.debug_print(f"[UNARY OP] operand={self._format_value(val)}, op={op}, result={self._format_value(result)}")
        return result

    def execute_binary_op(self, op: str, left: Any, right: Any) -> Any:
        """
        执行二元运算符
        op: 运算符 '+', '-', '*', '/', '.'
        left, right: 左右操作数
        """
        if left is None or right is None:
            result = None
            self.debug_print(f"[BINARY OP] left={self._format_value(left)}, right={self._format_value(right)}, op={op}, result={self._format_value(result)}")
            return None
        if op == '+':
            if isinstance(left, list) and isinstance(right, list):
                result = [l + r for l, r in zip(left, right)]
            elif isinstance(left, list) and isinstance(right, (int, float)):
                result = [v + right for v in left]
            elif isinstance(right, list) and isinstance(left, (int, float)):
                result = [left + v for v in right]
            else:
                result = left + right
        elif op == '-':
            if isinstance(left, list) and isinstance(right, list):
                result = [l - r for l, r in zip(left, right)]
            elif isinstance(left, list) and isinstance(right, (int, float)):
                result = [v - right for v in left]
            elif isinstance(right, list) and isinstance(left, (int, float)):
                result = [left - v for v in right]
            else:
                result = left - right
        elif op == '*':
            if isinstance(left, list) and isinstance(right, (int, float)):
                result = [v * right for v in left]
            elif isinstance(right, list) and isinstance(left, (int, float)):
                result = [v * left for v in right]
            elif isinstance(left, list) and isinstance(right, list):
                result = [l * r for l, r in zip(left, right)]
            else:
                result = left * right
        elif op == '/':
            if isinstance(left, list) and isinstance(right, (int, float)):
                result = [v / right for v in left]
            elif isinstance(left, list) and isinstance(right, list):
                result = [l / r for l, r in zip(left, right)]
            else:
                result = left / right
        elif op == '.':
            result = (left, right)
        elif op == '==':
            result = left == right
        elif op == '!=':
            result = left != right
        elif op == '<':
            result = left < right
        elif op == '>':
            result = left > right
        elif op == '<=':
            result = left <= right
        elif op == '>=':
            result = left >= right
        elif op == '&&':
            result = bool(left and right)
        elif op == '||':
            result = bool(left or right)
        else:
            result = None
        self.debug_print(f"[BINARY OP] left={self._format_float(left)}, right={self._format_float(right)}, op={op}, result={self._format_float(result)}")
        return result

    def transpose_matrix(self, m: List[List[float]]) -> List[List[float]]:
        """
        矩阵转置
        m: 输入矩阵(4x4或3x3)
        返回: 转置后的矩阵
        """
        if len(m) == 4:
            return [[m[j][i] for j in range(4)] for i in range(4)]
        elif len(m) == 3:
            return [[m[j][i] for j in range(3)] for i in range(3)]
        return m

    def mul_matrix_vector(self, m: List[List[float]], v: List[float]) -> List[float]:
        """
        矩阵乘向量: result = m * v
        m: 4x4或3x3矩阵
        v: 向量(4维或3维)
        返回: 计算后的向量
        """
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
        """
        矩阵乘法: result = a * b
        a, b: n x n 方阵
        返回: 结果矩阵
        """
        n = len(a)
        result = [[0.0] * n for _ in range(n)]
        for i in range(n):
            for j in range(n):
                for k in range(n):
                    result[i][j] += a[i][k] * b[k][j]
        return result

    def length_vec(self, v: List[float]) -> float:
        """计算向量长度(模)"""
        return math.sqrt(sum(x * x for x in v))

    def normalize_vec(self, v: List[float]) -> List[float]:
        """
        向量归一化
        v: 输入向量
        返回: 归一化后的向量，长度为1
        """
        l = self.length_vec(v)
        if l < 1e-8:
            return v
        return [x / l for x in v]

    def dot_product(self, a: List[float], b: List[float]) -> float:
        """
        向量点积: a · b
        a, b: 同维度向量
        返回: 点积结果
        """
        if not isinstance(a, list) or not isinstance(b, list):
            return 0.0
        if len(a) != len(b):
            return 0.0
        return sum(x * y for x, y in zip(a, b))

    def reflect_vec(self, I: List[float], N: List[float]) -> List[float]:
        """
        计算反射向量 R = I - 2 * (N · I) * N
        I: 入射向量
        N: 法线向量(需要归一化)
        返回: 反射向量
        """
        if not isinstance(I, list) or not isinstance(N, list):
            return [0, 0, 0]
        dot = self.dot_product(N, I)
        result = []
        for i_val, n_val in zip(I, N):
            result.append(i_val - 2 * n_val * dot)
        return result

    def find_top_level_comma(self, expr: str) -> int:
        """
        查找表达式顶层逗号(不在括号内)
        用于分割函数多参数
        expr: 表达式字符串
        返回: 逗号位置索引，或-1表示未找到
        """
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
        """
        对HLSL表达式求值
        expr: 表达式字符串
        local_vars: 局部变量字典
        返回: 求值结果
        """
        expr = expr.strip()
        if not expr:
            return None

        if expr == 'return':
            return None

        if expr.startswith('return '):
            return self.evaluate_expression(expr[7:], local_vars)

        # 使用语法树解析器处理所有表达式（包括三元运算符）
        tree = self.syntax_parser.parse(expr)

        # Print syntax tree
        if self.printSyntaxTree == True:
            self.debug_print(f"[SYNTAX TREE]\n{tree}")

        result = self.evaluate_syntax_tree(tree, local_vars)
        return result

    def evaluate_syntax_tree(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
        """
        对语法树节点求值
        node: 语法树节点
        local_vars: 局部变量字典
        返回: 求值结果
        """

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

        elif node.node_type == 'ternary':
            cond = self.evaluate_syntax_tree(node.left, local_vars)
            if cond:
                return self.evaluate_syntax_tree(node.right, local_vars)
            else:
                return self.evaluate_syntax_tree(node.third_child, local_vars)

        elif node.node_type == 'cast':
            inner = self.evaluate_syntax_tree(node.left, local_vars)
            if inner is None:
                return None
            cast_type = node.value
            # float3x3转换: 从4x4矩阵提取前3x3
            if cast_type == 'float3x3' and isinstance(inner, list) and len(inner) == 4:
                return [row[:3] for row in inner[:3]]
            # float2x2转换: 从4x4矩阵提取前2x2
            if cast_type == 'float2x2' and isinstance(inner, list) and len(inner) == 4:
                return [row[:2] for row in inner[:2]]
            # float2x2转换: 从3x3矩阵提取前2x2
            if cast_type == 'float2x2' and isinstance(inner, list) and len(inner) == 3:
                return [row[:2] for row in inner[:2]]
            return inner

        return None

    def execute_function_node(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
        """
        执行函数调用语法树节点
        node: 函数调用节点
        local_vars: 局部变量字典
        返回: 函数执行结果
        """
        func_name = node.value
        args = node.args

        # transpose: 矩阵转置函数
        # 计算矩阵的转置，将行列互换
        if func_name == 'transpose':
            if len(args) != 1:
                self.debug_print(f"[ERROR] transpose requires 1 arg, got {len(args)} at line {node.line_number}")
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            result = self.transpose_matrix(val)
            self.debug_print(f"[FUNC] transpose(\n{self._format_value(val)}) =\n{self._format_value(result)}")
            return result

        # normalize: 向量归一化函数
        # 将输入向量缩放到单位长度，即长度为1
        elif func_name == 'normalize':
            if len(args) != 1:
                self.debug_print(f"[ERROR] normalize requires 1 arg, got {len(args)} at line {node.line_number}")
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = self.normalize_vec(val)
                self.debug_print(f"[FUNC] normalize({self._format_float(val)}) = {self._format_float(result)}")
                return result
            return val

        # length: 向量长度函数
        # 计算向量的欧几里得长度(模)
        elif func_name == 'length':
            if len(args) != 1:
                self.debug_print(f"[ERROR] length requires 1 arg, got {len(args)} at line {node.line_number}")
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            result = self.length_vec(val)
            self.debug_print(f"[FUNC] length({self._format_float(val)}) = {self._format_float(result)}")
            return result

        # dot: 向量点积函数
        # 计算两个向量的点积，结果为标量
        elif func_name == 'dot':
            if len(args) != 2:
                self.debug_print(f"[ERROR] dot requires 2 args, got {len(args)} at line {node.line_number}")
                return None
            a = self.evaluate_syntax_tree(args[0], local_vars)
            b = self.evaluate_syntax_tree(args[1], local_vars)
            if a is None or b is None:
                return None
            result = self.dot_product(a, b)
            self.debug_print(f"[FUNC] dot({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
            return result

        # reflect: 反射向量函数
        # 计算光线关于法向量的反射向量，公式: R = I - 2 * N * dot(I, N)
        elif func_name == 'reflect':
            if len(args) != 2:
                self.debug_print(f"[ERROR] reflect requires 2 args, got {len(args)} at line {node.line_number}")
                return None
            I = self.evaluate_syntax_tree(args[0], local_vars)
            N = self.evaluate_syntax_tree(args[1], local_vars)
            if I is None or N is None:
                return None
            result = self.reflect_vec(I, N)
            self.debug_print(f"[FUNC] reflect({self._format_float(I)}, {self._format_float(N)}) = {self._format_float(result)}")
            return result

        # max: 最大值函数
        # 返回两个值中的较大者
        elif func_name == 'max':
            if len(args) != 2:
                self.debug_print(f"[ERROR] max requires 2 args, got {len(args)} at line {node.line_number}")
                return None
            a = self.evaluate_syntax_tree(args[0], local_vars)
            b = self.evaluate_syntax_tree(args[1], local_vars)
            if a is None or b is None:
                return None
            result = max(a, b)
            self.debug_print(f"[FUNC] max({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
            return result

        # min: 最小值函数
        # 返回两个值中的较小者
        elif func_name == 'min':
            if len(args) != 2:
                self.debug_print(f"[ERROR] min requires 2 args, got {len(args)} at line {node.line_number}")
                return None
            a = self.evaluate_syntax_tree(args[0], local_vars)
            b = self.evaluate_syntax_tree(args[1], local_vars)
            if a is None or b is None:
                return None
            result = min(a, b)
            self.debug_print(f"[FUNC] min({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
            return result

        # pow: 幂函数
        # 计算base的exp次幂，即 base ^ exp
        elif func_name == 'pow':
            if len(args) != 2:
                self.debug_print(f"[ERROR] pow requires 2 args, got {len(args)} at line {node.line_number}")
                return None
            base = self.evaluate_syntax_tree(args[0], local_vars)
            exp = self.evaluate_syntax_tree(args[1], local_vars)
            if base is None or exp is None:
                return None
            result = math.pow(base, exp)
            self.debug_print(f"[FUNC] pow({self._format_float(base)}, {self._format_float(exp)}) = {self._format_float(result)}")
            return result

        # abs: 绝对值函数
        # 返回数值的绝对值，对列表则对每个元素取绝对值
        elif func_name == 'abs':
            if len(args) != 1:
                self.debug_print(f"[ERROR] abs requires 1 arg, got {len(args)} at line {node.line_number}")
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [abs(v) for v in val]
            else:
                result = abs(val)
            self.debug_print(f"[FUNC] abs({self._format_float(val)}) = {self._format_float(result)}")
            return result

        # sin: 正弦函数
        # 计算弧度的正弦值，对列表则对每个元素计算
        elif func_name == 'sin':
            if len(args) != 1:
                self.debug_print(f"[ERROR] sin requires 1 arg, got {len(args)} at line {node.line_number}")
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [math.sin(v) for v in val]
            else:
                result = math.sin(val)
            self.debug_print(f"[FUNC] sin({self._format_float(val)}) = {self._format_float(result)}")
            return result

        # cos: 余弦函数
        # 计算弧度的余弦值，对列表则对每个元素计算
        elif func_name == 'cos':
            if len(args) != 1:
                self.debug_print(f"[ERROR] cos requires 1 arg, got {len(args)} at line {node.line_number}")
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [math.cos(v) for v in val]
            else:
                result = math.cos(val)
            self.debug_print(f"[FUNC] cos({self._format_float(val)}) = {self._format_float(result)}")
            return result

        # mul: 矩阵乘法函数
        # 执行4x4或3x3矩阵乘法运算
        elif func_name == 'mul':
            if len(args) != 2:
                self.debug_print(f"[ERROR] mul requires 2 args, got {len(args)} at line {node.line_number}")
                return None
            left = self.evaluate_syntax_tree(args[0], local_vars)
            right = self.evaluate_syntax_tree(args[1], local_vars)
            if left is None or right is None:
                return None
            if isinstance(left, list) and isinstance(right, list):
                if len(left) == 4 and len(right) == 4:
                    result = self.mul_matrix_vector(right, left)
                    self.debug_print(f"[FUNC] mul(\nleft={self._format_value(left)},\nright={self._format_value(right)}) =\n{self._format_value(result)}")
                    return result
                elif len(left) == 3 and len(right) == 3:
                    result = self.mul_matrix_vector(right, left)
                    self.debug_print(f"[FUNC] mul(\nleft={self._format_value(left)},\nright={self._format_value(right)}) =\n{self._format_value(result)}")
                    return result
            return None

        # float2/float3/float4: 向量构造函数
        # 将参数展平合并为指定长度的向量
        elif func_name in ['float2', 'float3', 'float4']:
            # 向量构造函数: 将参数展平合并
            result = []
            for arg in args:
                val = self.evaluate_syntax_tree(arg, local_vars)
                if isinstance(val, list):
                    result.extend(val)
                else:
                    result.append(val)
            self.debug_print(f"[FUNC] {func_name}(args={self._format_float(args)}) = {self._format_float(result)}")
            return result

        return None

    def apply_swizzle(self, obj: Any, swizzle: str) -> Any:
        """
        对向量应用swizzle操作
        obj: 向量对象(列表)
        swizzle: swizzle模式字符串，如 'xyz', 'xxx', 'xxyy', 'xz' 等
        返回: 应用swizzle后的结果
        """
        if obj is None:
            return None

        if not isinstance(obj, list):
            if swizzle == 'x':
                return obj
            return None

        valid_chars = {'x': 0, 'y': 1, 'z': 2, 'w': 3}
        result = []
        for c in swizzle:
            if c.lower() in valid_chars:
                idx = valid_chars[c.lower()]
                if idx < len(obj):
                    result.append(obj[idx])
                else:
                    result.append(0)
            elif c in 'rgb':
                idx = {'r': 0, 'g': 1, 'b': 2}[c]
                if idx < len(obj):
                    result.append(obj[idx])
                else:
                    result.append(0)

        if len(result) == 1:
            return result[0]

        numeric_types = (int, float)
        if all(isinstance(v, numeric_types) for v in result):
            if all(isinstance(v, int) for v in result):
                return [int(v) for v in result]
            return result

        return result

    def get_value(self, name: str, local_vars: Dict[str, Any]) -> Any:
        """
        获取变量或常量的值
        name: 变量名/常量名，支持结构体字段访问(如 input.Pos)
        local_vars: 局部变量字典
        返回: 变量值，如果未找到返回0.0
        """
        name = name.strip()

        # 处理布尔常量
        if name == 'true':
            return True
        if name == 'false':
            return False

        # 尝试解析为数字
        try:
            return float(name)
        except ValueError:
            pass

        # 检查是否包含swizzle操作 (如 LightPos.xyz, LightPos.xxx, input.Pos.xy)
        if '.' in name:
            parts = name.split('.')
            if len(parts) >= 2:
                base_name = parts[0]

                # 判断是否为swizzle模式（全是xyzwrgb组成的字符串）
                # 对于 input.Color.g, parts = ['input', 'Color', 'g']
                # 只有当最后一部分是纯swizzle字符时，才认为是swizzle操作
                last_part = parts[-1]
                is_single_swizzle = len(parts) == 2 and last_part and all(c in 'xyzwrgb' for c in last_part.lower())
                is_multi_swizzle = len(parts) == 2 and last_part and all(c in 'xyzwrgb' for c in last_part.lower()) and len(last_part) > 1

                if is_single_swizzle or is_multi_swizzle:
                    # 两级访问: input.Pos 或 input.Color.rgb
                    swizzle_str = last_part
                    # 先检查 base_name + '.' + swizzle_str 是否直接存在
                    full_swizzle_name = f'{base_name}.{swizzle_str}'
                    if full_swizzle_name in local_vars:
                        obj = local_vars[full_swizzle_name]
                        if isinstance(obj, (int, float)):
                            return obj
                        if isinstance(obj, list):
                            return obj

                    obj = local_vars.get(base_name)
                    if obj is None:
                        obj = self.variables.get(base_name)
                    if obj is not None:
                        return self.apply_swizzle(obj, swizzle_str)

                    # 尝试从cbuffer获取
                    for cb_name, cb_def in self.cbuffers.items():
                        if isinstance(cb_def, CbufferDefinition):
                            for field in cb_def.fields:
                                if field.name == base_name:
                                    if field.data is not None:
                                        return self.apply_swizzle(field.data, swizzle_str)
                                    return 0

                    # 检查是否在output对象中
                    if base_name in local_vars:
                        obj = local_vars[base_name]
                        if isinstance(obj, dict):
                            return self.apply_swizzle(obj.get(swizzle_str), swizzle_str) if isinstance(obj.get(swizzle_str), list) else self.apply_swizzle(obj, swizzle_str)
                        return self.apply_swizzle(obj, swizzle_str)

                    return 0
                else:
                    # 多级访问: input.Color.g (Color不是纯swizzle字符)
                    if len(parts) == 2:
                        # 两级访问但不是swizzle模式: input.Color
                        # 直接查local_vars中是否存在 'input.Color'
                        full_name = f'{base_name}.{parts[1]}'
                        if full_name in local_vars:
                            return local_vars[full_name]
                        # 检查 base_name 是否在local_vars中作为dict
                        if base_name in local_vars:
                            obj = local_vars[base_name]
                            if isinstance(obj, dict):
                                return obj.get(parts[1], 0)
                            elif isinstance(obj, list):
                                # base_name是列表(比如input.Pos是float3),parts[1]是访问其元素
                                idx_map = {'x': 0, 'y': 1, 'z': 2, 'w': 3, 'r': 0, 'g': 1, 'b': 2, 'a': 3}
                                if parts[1].lower() in idx_map:
                                    idx = idx_map[parts[1].lower()]
                                    return obj[idx] if idx < len(obj) else 0
                        # 检查cbuffer
                        for cb_name, cb_def in self.cbuffers.items():
                            if isinstance(cb_def, CbufferDefinition):
                                for field in cb_def.fields:
                                    if field.name == base_name:
                                        if field.data is not None:
                                            return self.apply_swizzle(field.data, parts[1])
                                        return 0
                        return 0
                    elif len(parts) == 3:
                        # input.Color.g -> 获取 input.Color, 然后对结果应用 .g
                        # 直接查找 input.Color 是否在local_vars中
                        full_name = f'{base_name}.{parts[1]}'  # 'input.Color'
                        if full_name in local_vars:
                            base_val = local_vars[full_name]
                        else:
                            base_val = self.get_value(f'{base_name}.{parts[1]}', local_vars)
                        if isinstance(base_val, list):
                            idx_map = {'x': 0, 'y': 1, 'z': 2, 'w': 3, 'r': 0, 'g': 1, 'b': 2, 'a': 3}
                            swizzle_ch = parts[2].lower()
                            if swizzle_ch in idx_map:
                                return base_val[idx_map[swizzle_ch]] if idx_map[swizzle_ch] < len(base_val) else 0
                        return 0
                    else:
                        # 超过3级,递归处理
                        return self.get_value('.'.join(parts[1:]), local_vars)

        # 局部变量查找
        if name in local_vars:
            val = local_vars[name]
            return val

        base_name = name.split('.')[0] if '.' in name else name

        # cbuffer字段查找
        for cb_name, cb_def in self.cbuffers.items():
            if isinstance(cb_def, CbufferDefinition):
                for field in cb_def.fields:
                    if field.name == base_name:
                        return field.data if field.data is not None else 0

        # 全局变量查找
        if name in self.variables:
            return self.variables[name]

        # 嵌套cbuffer查找
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
        """
        执行单条HLSL语句
        stmt: 语句字符串，如 "float3 pos = input.Pos;" 或 "output.Color = float4(1,0,0,1);"
        local_vars: 局部变量字典
        """
        stmt = stmt.strip()
        if not stmt:
            return None

        self.debug_print(f"\n[STMT] Executing: {stmt}")
        input_snapshot = {k: v for k, v in local_vars.items() if k.startswith('input.') or k == 'output'}

        # if-else条件语句处理
        if stmt.startswith('if'):
            self.execute_if_statement(stmt, local_vars)
            return None

        # 变量声明语句: float4 pos = ...;
        match = self.patterns['variable_declaration'].match(stmt)
        if match:
            var_name = match.group(2)
            value = self.evaluate_expression(match.group(3), local_vars)
            local_vars[var_name] = value
            self.debug_print(f"[STMT] {stmt} => {var_name} = {self._format_value(value)}")
            return None

        # output字段赋值: output.Color = ...; 或 output.Color.r = ...;
        if 'output.' in stmt:
            match = self.patterns['output_field_assignment'].match(stmt)
            if match:
                field_name = match.group(1)
                swizzle = match.group(2)
                value_expr = match.group(3).rstrip(';').strip()
                value = self.evaluate_expression(value_expr, local_vars)

                if 'output' not in local_vars:
                    local_vars['output'] = {}

                if swizzle is None:
                    local_vars['output'][field_name] = value
                else:
                    if field_name not in local_vars['output']:
                        local_vars['output'][field_name] = [0.0, 0.0, 0.0, 0.0]
                    current = local_vars['output'][field_name]
                    if not isinstance(current, list):
                        current = [current, 0.0, 0.0, 0.0]

                    swizzle_map = {'x': 0, 'y': 1, 'z': 2, 'w': 3, 'r': 0, 'g': 1, 'b': 2, 'a': 3}
                    if isinstance(value, list):
                        for i, ch in enumerate(swizzle.lower()):
                            if ch in swizzle_map and i < len(value):
                                current[swizzle_map[ch]] = value[i]
                    else:
                        ch = swizzle.lower()[0] if swizzle else 'x'
                        if ch in swizzle_map:
                            current[swizzle_map[ch]] = value

                    local_vars['output'][field_name] = current
                self.debug_print(f"[STMT] {stmt} => output.{field_name}" + (f".{swizzle}" if swizzle else "") + f" = {self._format_float(value)}")
                return None

        # 一般赋值语句: var = ...;
        if '=' in stmt and stmt.count('=') == 1:
            match = self.patterns['simple_assignment'].match(stmt)
            if match:
                var_name = match.group(1)
                value = self.evaluate_expression(match.group(2), local_vars)
                local_vars[var_name] = value
                self.debug_print(f"[STMT] {stmt} => {var_name} = {value}")
                return None

        self.debug_print(f"[STMT] {stmt} => (no assignment)")
        return None

    def execute_if_statement(self, stmt: str, local_vars: Dict[str, Any]):
        """
        执行if-else条件语句
        stmt: if语句字符串
        local_vars: 局部变量字典
        """
        stmt = stmt.strip()

        if_match = self.patterns['if_statement'].match(stmt)
        if not if_match:
            return

        condition_expr = if_match.group(1).strip()
        then_branch = if_match.group(2).strip()

        cond_value = self.evaluate_expression(condition_expr, local_vars)
        self.debug_print(f"[IF] condition: {condition_expr} => {cond_value}")

        if cond_value:
            if then_branch.startswith('{'):
                self.execute_block(then_branch, local_vars)
            elif not then_branch.startswith('else'):
                self.execute_statement(then_branch, local_vars)
        else:
            else_pos = self.find_else_branch(then_branch)
            if else_pos >= 0:
                else_branch = then_branch[else_pos:].strip()
                if else_branch.startswith('else'):
                    else_branch = else_branch[4:].strip()
                    if else_branch.startswith('{'):
                        self.execute_block(else_branch, local_vars)
                    else:
                        self.execute_statement(else_branch, local_vars)

    def find_else_branch(self, stmt: str) -> int:
        """
        查找else分支的起始位置(不在嵌套括号内)
        stmt: 语句字符串
        返回: else关键字位置，或-1表示未找到
        """
        depth = 0
        pos = 0
        while pos < len(stmt):
            char = stmt[pos]
            if char == '(':
                depth += 1
            elif char == ')':
                depth -= 1
            elif char == '{':
                depth += 1
            elif char == '}':
                depth -= 1
            elif depth == 0 and stmt[pos:pos+4] == 'else':
                return pos
            pos += 1
        return -1

    def execute_block(self, block: str, local_vars: Dict[str, Any]):
        """
        执行语句块(被大括号包围的语句列表)
        block: 语句块字符串
        local_vars: 局部变量字典
        """
        block = block.strip()
        if not block.startswith('{') or not block.endswith('}'):
            return

        inner = block[1:-1].strip()
        if not inner:
            return

        statements = self.GenerateStmts(inner)
        for stmt in statements:
            self.execute_statement(stmt, local_vars)

    def GenerateStmts(self, code: str):
        statements = []
        current_stmt = []
        brace_count = 0
        paren_count = 0
        in_string = False
        string_char = None

        for char in code:
            if char == '{':
                brace_count += 1
                if brace_count > 0:
                    current_stmt.append(char)
            elif char == '}':
                if brace_count > 0:
                    current_stmt.append(char)
                brace_count -= 1
                if brace_count == 0 and current_stmt:
                    stmt = ''.join(current_stmt).strip()
                    if stmt:
                        statements.append(stmt)
                    current_stmt = []
            elif char == '(':
                paren_count += 1
                current_stmt.append(char)
            elif char == ')':
                paren_count -= 1
                current_stmt.append(char)
            elif char in '"\'':
                if not in_string:
                    in_string = True
                    string_char = char
                elif char == string_char:
                    in_string = False
                    string_char = None
                current_stmt.append(char)
            elif char == ';' and brace_count == 0 and paren_count == 0 and not in_string:
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

        return statements


    def execute_main_function(self, code: str, main_func: str, input_struct_name: str, row_index: int, data: Dict[str, Any]):
        """
        执行HLSL main函数
        code: HLSL代码
        main_func: main函数名
        input_struct_name: 输入结构体名
        row_index: 数据行索引
        data: 输入数据字典
        返回: output结构体字典
        """
        input_struct = self.structs.get(input_struct_name)
        if not input_struct:
            self.log_output(f"Cannot find input_struct: {input_struct_name}\n")
            return None

        input_fields = {}
        for field in input_struct.fields:
            input_fields[field.name] = field.field_type

        # 查找main函数签名
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

        cache_key = f"{output_struct_name}_{main_func}_{input_struct_name_from_func}"
        if cache_key in self._parsed_func_cache:
            cached = self._parsed_func_cache[cache_key]
            body = cached['body']
            statements = cached['statements']
        else:
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
            if body.startswith('{'):
                body = body[1:].strip()
            if body.endswith('}'):
                body = body[:-1].strip()

            statements = self.GenerateStmts(body)
            self._parsed_func_cache[cache_key] = {'body': body, 'statements': statements}

        # 初始化局部变量
        local_vars = {'data': data}

        # 设置input字段变量
        for field_name, field_value in data.items():
            local_vars[f'input.{field_name}'] = field_value

        # 初始化output对象
        output_obj = {}
        for field in output_fields:
            output_obj[field] = None
        local_vars['output'] = output_obj

        ret_val = None

        self._eval_counter += 1
        self._should_print = ((self._eval_counter - 1) % self.print_sequence == 0)

        self.debug_print(f"******************************************************")
        self.debug_print(f"**************Begin {self._eval_counter}**************")
        self.debug_print(f"******************************************************\n")

        self.debug_print(f"\n=== INPUT DATA ===")
        for k, v in local_vars.items():
            if k.startswith('input.') or k == 'output':
                self.debug_print(f"  {k} = {v}")
        self.debug_print(f"==================")

        # 顺序执行语句
        i = 0
        while i < len(statements):
            stmt = statements[i]
            if stmt is None:
                i += 1
                continue

            if 'return' in stmt and 'output' in stmt:
                ret_val = local_vars.get('output')
                i += 1
                continue

            # 检查是否是if语句，且下一条是else
            if stmt.startswith('if'):
                next_i = i + 1
                # 查找下一个非None的语句
                while next_i < len(statements) and statements[next_i] is None:
                    next_i += 1
                
                if next_i < len(statements) and statements[next_i].startswith('else'):
                    # 合并if和else为完整语句
                    full_if_stmt = stmt + '\n' + statements[next_i]
                    self.execute_if_statement(full_if_stmt, local_vars)
                    statements[next_i] = None  # 标记else已处理
                else:
                    self.execute_if_statement(stmt, local_vars)
            else:
                self.execute_statement(stmt, local_vars)

            i += 1

        self.debug_print(f"******************************************************")
        self.debug_print(f"**************End {self._eval_counter}**************")
        self.debug_print(f"******************************************************\n")

        return ret_val

    def interpret(self, hlsl_file_path: str, csv_folder_path: str = None):
        """
        解释HLSL代码 - 解析结构体和cbuffer定义
        hlsl_file_path: HLSL文件路径
        csv_folder_path: CSV文件夹路径（如果为None则不加载CSV数据）
        """
        if not os.path.exists(hlsl_file_path):
            self.log_output(f"Error: HLSL file not found: {hlsl_file_path}")
            return

        with open(hlsl_file_path, 'r', encoding='utf-8') as f:
            self.hlsl_code = f.read()

        code = self.hlsl_code

        if csv_folder_path is None:
            csv_folder_path = os.path.dirname(hlsl_file_path)

        # 解析struct定义
        for struct_match in self.patterns['struct_finditer'].finditer(code):
            struct_def = self.parse_struct(struct_match.group())
            if struct_def:
                self.structs[struct_def.name] = struct_def

        # 解析cbuffer定义
        for cb_match in self.patterns['cbuffer_finditer'].finditer(code):
            cb_def = self.parse_cbuffer(cb_match.group())
            if cb_def:
                self.cbuffers[cb_def.name] = cb_def

        # 从CSV加载struct数据
        for struct_name in self.structs:
            csv_path = os.path.join(csv_folder_path, f'{struct_name}.csv')
            if os.path.exists(csv_path):
                self.load_struct_data_from_csv(struct_name, csv_path)

        # 从CSV加载cbuffer数据
        for cb_name in self.cbuffers:
            csv_path = os.path.join(csv_folder_path, f'{cb_name}.csv')
            if os.path.exists(csv_path):
                self.load_cbuffer_data_from_csv(cb_name, csv_path)

    def executeVS(self, main_func: str, vs_input: str, code: str = None, execute_count: int = None):
        """
        执行顶点着色器
        main_func: 入口函数名
        vs_input: 输入结构体名
        code: HLSL代码（如果为None则使用self.hlsl_code）
        execute_count: 执行次数（如果为None则使用input_struct.fields计算行数）
        返回: 输出结构体字典列表
        """
        if code is None:
            code = self.hlsl_code
        input_struct = self.structs.get(vs_input)
        if not input_struct:
            self.log_output(f"Cannot find vs input: {vs_input}\n")
            return None

        # clear eval counter
        self._eval_counter = 0

        if execute_count is None:
            num_rows = 0
            for field in input_struct.fields:
                if field.data:
                    num_rows = max(num_rows, len(field.data))
            execute_count = num_rows

        if self.max_workers > 1:
            def execute_row(row_index: int):
                data = {}
                for field in input_struct.fields:
                    if field.data and row_index < len(field.data):
                        data[field.name] = field.data[row_index]
                result = self.execute_main_function(code, main_func, vs_input, row_index, data)
                return row_index, result

            print(f"Run thread workers")
            results = [None] * execute_count
            with ThreadPoolExecutor(max_workers=self.max_workers) as executor:
                futures = [executor.submit(execute_row, i) for i in range(execute_count)]
                for future in futures:
                    idx, result = future.result()
                    results[idx] = result
        else:
            print(f"Run single thread")
            results = []
            for row_index in range(execute_count):
                data = {}
                for field in input_struct.fields:
                    if field.data and row_index < len(field.data):
                        data[field.name] = field.data[row_index]
                result = self.execute_main_function(code, main_func, vs_input, row_index, data)
                results.append(result)

        return results

    def executePS(self, code: str, main_func: str, ps_input: str):
        """
        执行像素着色器(当前为占位函数)
        """
        pass

    def load_struct_data_from_csv(self, struct_name: str, csv_path: str):
        """
        从CSV文件加载struct数据
        struct_name: 结构体名称
        csv_path: CSV文件路径
        """
        if struct_name not in self.structs:
            return
        struct_def = self.structs[struct_name]
        rows = self.load_csv(csv_path)
        if not rows or len(rows) < 2:
            return

        header = rows[0]
        data_rows = rows[1:]

        # 建立字段列索引映射
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

        # 填充字段数据
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
                self.log_output(f"Field '{field.semantic}' ({field.field_type}): {values[0] if values else 'N/A'}")

    def load_cbuffer_data_from_csv(self, cb_name: str, csv_path: str):
        """
        从CSV文件加载cbuffer数据
        cb_name: cbuffer名称
        csv_path: CSV文件路径
        """
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

            # 跳过空值
            if value_str == '':
                continue

            # 矩阵行处理(如 World.row0, World.row1)
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

        # 填充字段数据
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

        # 打印cbuffer内容
        cb_n = cb_name
        cb_d = cb_def
        self.log_output(f"Cbuffer {cb_n}:")
        for f in cb_d.fields:
            data = f.data
            ft = f.field_type
            if 'float4x4' in ft:
                self.log_output(f"  {f.name} ({ft}):")
                for row in data:
                    row_str = '  '.join(f"{v:12.5f}" for v in row)
                    self.log_output(f"    [{row_str}]")
            elif 'float3x3' in ft:
                self.log_output(f"  {f.name} ({ft}):")
                for row in data:
                    row_str = '  '.join(f"{v:12.5f}" for v in row)
                    self.log_output(f"    [{row_str}]")
            elif 'float4' in ft:
                self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
            elif 'float3' in ft:
                self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
            elif 'float2' in ft:
                self.log_output(f"  {f.name} ({ft}): [{', '.join(f'{v:.5f}' for v in data)}]")
            elif 'float' in ft:
                self.log_output(f"  {f.name} ({ft}): {data:.5f}")
            elif 'uint4' in ft:
                self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'uint3' in ft:
                self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'uint2' in ft:
                self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'uint' in ft:
                self.log_output(f"  {f.name} ({ft}): {data}")
            elif 'int4' in ft:
                self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'int3' in ft:
                self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'int2' in ft:
                self.log_output(f"  {f.name} ({ft}): [{', '.join(str(v) for v in data)}]")
            elif 'int' in ft:
                self.log_output(f"  {f.name} ({ft}): {data}")
            elif 'bool' in ft:
                self.log_output(f"  {f.name} ({ft}): {data}")
            else:
                self.log_output(f"  {f.name} ({ft}): {data}")

    def load_vs_output_golden_from_csv(self, csv_path: str):
        """
        从CSV文件加载VS_OUTPUT的golden数据
        csv_path: CSV文件路径
        """
        if "VS_OUTPUT" not in self.structs:
            self.log_output("Error: VS_OUTPUT struct not defined")
            return False

        vs_output_def = self.structs["VS_OUTPUT"]
        rows = self.load_csv(csv_path)
        if not rows or len(rows) < 2:
            self.log_output(f"Error: CSV file {csv_path} is empty or has no data rows")
            return False

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

        for field in vs_output_def.fields:
            if field.semantic in field_col_indices:
                col_dict = field_col_indices[field.semantic]
                values = []
                for row in data_rows:
                    try:
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
                    except (ValueError, IndexError) as e:
                        self.log_output(f"Warning: Failed to parse {field.semantic} at row: {e}")
                        values.append(None)
                field.data = values

        self.log_output(f"Loaded {len(data_rows)} golden data rows for VS_OUTPUT")
        return True

    def compare_vs_output_with_golden(self, hlsl_output: List[Dict], output_struct_name: str = "VS_OUTPUT", float_tolerance: float = 0.0001, execute_count: int = None) -> bool:
        """
        比较HLSL执行结果与golden数据
        hlsl_output: executeVS返回的输出结构体字典列表
        output_struct_name: 输出结构体名称，用于获取field name (默认"VS_OUTPUT")
        float_tolerance: 浮点类型数据的比较误差容忍度
        execute_count: 执行次数（如果为None则使用golden数据计算行数）
        返回: True表示所有数据匹配, False表示存在不匹配
        """
        if output_struct_name not in self.structs:
            self.log_output(f"Error: {output_struct_name} struct not found")
            return False

        vs_output_def = self.structs[output_struct_name]
        golden_data = {}
        semantic_to_field = {}

        for field in vs_output_def.fields:
            if field.data:
                golden_data[field.semantic] = field.data
            semantic_to_field[field.semantic] = field.name

        num_golden_rows = 0
        for field_data in golden_data.values():
            if field_data:
                num_golden_rows = max(num_golden_rows, len(field_data))

        if execute_count is not None:
            num_golden_rows = execute_count

        if not hlsl_output:
            self.log_output("Error: No HLSL output to compare")
            return False

        if len(hlsl_output) != num_golden_rows:
            self.log_output(f"Error: Row count mismatch - HLSL output has {len(hlsl_output)} rows, golden has {num_golden_rows} rows")
            return False

        all_match = True
        passed_count = 0
        field_type_map = {}
        for field in vs_output_def.fields:
            field_type_map[field.semantic] = field.field_type

        for row_idx in range(len(hlsl_output)):
            output_row = hlsl_output[row_idx]
            row_match = True
            for semantic, golden_values in golden_data.items():
                if row_idx >= len(golden_values):
                    continue

                field_name = semantic_to_field.get(semantic, semantic)
                if field_name not in output_row:
                    continue

                output_value = output_row[field_name]
                golden_value = golden_values[row_idx]

                if output_value is None or golden_value is None:
                    continue

                field_type = field_type_map.get(semantic, '')

                if isinstance(output_value, list) and isinstance(golden_value, list):
                    if len(output_value) != len(golden_value):
                        self.log_output(f"Error: Row {row_idx}, {field_name}: length mismatch output={len(output_value)} golden={len(golden_value)}")
                        row_match = False
                        continue

                    is_float = 'float' in field_type
                    for comp_idx in range(len(output_value)):
                        out_comp = output_value[comp_idx]
                        gold_comp = golden_value[comp_idx]

                        if is_float:
                            if isinstance(out_comp, float) and isinstance(gold_comp, float):
                                if abs(out_comp - gold_comp) > float_tolerance:
                                    self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp:.6f} golden={gold_comp:.6f} diff={abs(out_comp - gold_comp):.6f} > tolerance={float_tolerance}")
                                    row_match = False
                            elif out_comp != gold_comp:
                                self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp} golden={gold_comp} (float comparison failed)")
                                row_match = False
                        else:
                            if out_comp != gold_comp:
                                self.log_output(f"Error: Row {row_idx}, {field_name}[{comp_idx}]: output={out_comp} golden={gold_comp} (strict equality failed)")
                                row_match = False

            if row_match:
                passed_count += 1
            else:
                all_match = False

        self.log_output(f"Total PASSED rows: {passed_count}/{num_golden_rows}")
        if all_match:
            self.log_output("Comparison PASSED: All output data matches golden data within tolerance")
        else:
            self.log_output("Comparison FAILED: Some output data does not match golden data")

        return all_match


def main():
    import sys

    if len(sys.argv) < 2:
        print("Usage: python hlsl_interpreter.py <config.json>")
        print("Config JSON should contain: hlsl_file_path, csv_folder_path, log_file_path")
        config_path = './wrong_constant_attenuation.json'
    else:
        config_path = sys.argv[1]

    if not os.path.exists(config_path):
        print(f"Error: Config file not found: {config_path}")
        sys.exit(1)

    config = {}
    with open(config_path, 'r', encoding='utf-8') as f:
        config = json.load(f)

    hlsl_file_path = config.get('hlsl_file_path', '')
    csv_folder_path = config.get('csv_folder_path', '')
    log_file_path = config.get('log_file_path', 'hlsl_interpreter.log')
    log_file_mode = config.get('log_file_mode', 'a')
    print_sequence = config.get('print_sequence', 1)
    log_to_file = config.get('log_to_file', True)
    printSyntaxTree = config.get('printSyntaxTree', True)
    print_interpreter_result = config.get('print_interpreter_result', True)
    float_tolerance = config.get('float_tolerance', 0.0001)
    output_struct_name = config.get('output_struct_name', 'VS_OUTPUT')
    execute_count = config.get('execute_count', None)
    max_workers = config.get('max_workers', 1)
    primitive_topology = config.get('primitive_topology', D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
    mesh_view_enabled = config.get('mesh_view_enabled', False)

    if not hlsl_file_path:
        print("Error: hlsl_file_path not specified in config")
        sys.exit(1)

    if not os.path.exists(hlsl_file_path):
        print(f"Error: HLSL file not found: {hlsl_file_path}")
        sys.exit(1)

    if csv_folder_path and not os.path.exists(csv_folder_path):
        print(f"Error: CSV folder not found: {csv_folder_path}")
        sys.exit(1)

    interpreter = HLSLInterpreter(
        log_to_file=log_to_file,
        log_file_path=log_file_path,
        log_file_mode=log_file_mode,
        print_sequence=print_sequence,
        printSyntaxTree=printSyntaxTree,
        print_interpreter_result=print_interpreter_result,
        max_workers=max_workers,
        primitive_topology=primitive_topology)

    if mesh_view_enabled:
        interpreter.enable_mesh_view(True)

    total_start = time.time()

    interpret_start = time.time()
    interpreter.interpret(hlsl_file_path, csv_folder_path)
    interpret_time = time.time() - interpret_start

    golden_csv_path = os.path.join(csv_folder_path, 'VS_OUTPUT.csv') if csv_folder_path else None
    load_golden_start = time.time()
    if golden_csv_path and os.path.exists(golden_csv_path):
        interpreter.load_vs_output_golden_from_csv(golden_csv_path)
    load_golden_time = time.time() - load_golden_start

    if mesh_view_enabled:
        interpreter.log_output("Displaying input mesh before executeVS...")
        interpreter.show_input_mesh("VS_INPUT")

    execute_start = time.time()
    results = interpreter.executeVS("main", "VS_INPUT", execute_count=execute_count)
    execute_time = time.time() - execute_start

    if mesh_view_enabled and results:
        interpreter.log_output("Displaying result mesh after executeVS...")
        interpreter.show_result_mesh(results)

    if interpreter.print_interpreter_result:
        interpreter.log_output("HLSL Interpreter Result:")
        interpreter.log_output("=" * 40)
        if results:
            for idx, result in enumerate(results):
                interpreter.log_output(f"\n--- Row {idx} ---")
                if result:
                    for key, value in result.items():
                        if isinstance(value, list):
                            if len(value) == 4:
                                interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]")
                            elif len(value) == 3:
                                interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]")
                            elif len(value) == 2:
                                interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}]")
                            else:
                                interpreter.log_output(f"{key}: {value}")
                        else:
                            interpreter.log_output(f"{key}: {value}")
        else:
            interpreter.log_output("No result produced")

        if results and results[-1] and 'Color' in results[-1]:
            color = results[-1]['Color']
            if color and isinstance(color, list) and len(color) == 4:
                interpreter.log_output("\nFinal Output Color (RGBA):")
                interpreter.log_output(f"  R: {color[0]:.4f}")
                interpreter.log_output(f"  G: {color[1]:.4f}")
                interpreter.log_output(f"  B: {color[2]:.4f}")
                interpreter.log_output(f"  A: {color[3]:.4f}")
            else:
                interpreter.log_output(f"\nColor result: {color}")

        interpreter.log_output("\n" + "=" * 40)
    interpreter.log_output("Comparing with golden data...")
    interpreter.log_output("=" * 40)
    compare_start = time.time()
    interpreter.compare_vs_output_with_golden(results, output_struct_name=output_struct_name, float_tolerance=float_tolerance, execute_count=execute_count)
    compare_time = time.time() - compare_start

    total_time = time.time() - total_start

    interpreter.log_output("\n" + "=" * 40)
    interpreter.log_output("Timing Summary:")
    interpreter.log_output("=" * 40)
    interpreter.log_output(f"interpreter.interpret():             {interpret_time:.4f}s")
    interpreter.log_output(f"interpreter.load_vs_output_golden_from_csv(): {load_golden_time:.4f}s")
    interpreter.log_output(f"interpreter.executeVS():           {execute_time:.4f}s")
    interpreter.log_output(f"compare_vs_output_with_golden():    {compare_time:.4f}s")
    interpreter.log_output(f"Total execution time:               {total_time:.4f}s")

    while True:
        user_input = input("\nEnter 'x' to exit, 'o' to open MeshView, 'r' to rerun executeVS: ")
        user_input = user_input.strip().lower()
        if user_input == 'x':
            interpreter._mesh_view.close()
            break
        elif user_input == 'o':
            if interpreter._mesh_view:
                interpreter._mesh_view.show(blocking=False)
                interpreter.log_output("MeshView reopened")
        elif user_input == 'r':
            results = []
            execute_start = time.time()
            results = interpreter.executeVS("main", "VS_INPUT", execute_count=execute_count)
            execute_time = time.time() - execute_start
            interpreter.log_output(f"Re-executed executeVS in {execute_time:.4f}s")
            if mesh_view_enabled and results:
                interpreter.log_output("Displaying result mesh after re-execution...")
                interpreter.show_result_mesh(results)
            if interpreter.print_interpreter_result:
                interpreter.log_output("HLSL Interpreter Result (re-run):")
                interpreter.log_output("=" * 40)
                if results:
                    for idx, result in enumerate(results):
                        interpreter.log_output(f"\n--- Row {idx} ---")
                        if result:
                            for key, value in result.items():
                                if isinstance(value, list):
                                    if len(value) == 4:
                                        interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}, {value[3]:.4f}]")
                                    elif len(value) == 3:
                                        interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}, {value[2]:.4f}]")
                                    elif len(value) == 2:
                                        interpreter.log_output(f"{key}: [{value[0]:.4f}, {value[1]:.4f}]")
                                    else:
                                        interpreter.log_output(f"{key}: {value}")
                                else:
                                    interpreter.log_output(f"{key}: {value}")
                else:
                    interpreter.log_output("No result produced")
                interpreter.log_output("=" * 40)


if __name__ == '__main__':
    main()