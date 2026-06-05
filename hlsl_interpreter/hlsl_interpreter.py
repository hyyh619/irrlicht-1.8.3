import csv
import math
import re
import os
from concurrent.futures import ThreadPoolExecutor
from dataclasses import dataclass, field
from typing import Any, Dict, List, Union, Optional

from hlsl_syntax_tree import SyntaxTreeNode, SyntaxTreeParser, _COMPILED_PATTERNS

try:
    from texture import Texture, Sampler, TextureDesc, Sampler as SamplerClass
    TEXTURE_AVAILABLE = True
except ImportError:
    TEXTURE_AVAILABLE = False


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

from d3d import (
    D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,
    D3D_PRIMITIVE_TOPOLOGY_POINTLIST,
    D3D_PRIMITIVE_TOPOLOGY_LINELIST,
    D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
    D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN
)


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
class TextureBinding:
    """PS中的纹理绑定信息"""
    variable_name: str   # 变量名，如 DiffuseTexture
    register_id: int     # register(t0) 中的 t0，即纹理单元ID
    texture: Optional['Texture'] = None  # 实际的Texture对象


@dataclass
class SamplerBinding:
    """PS中的采样器绑定信息"""
    variable_name: str   # 变量名，如 LinearSampler
    register_id: int     # register(s0) 中的 s0，即采样器ID
    sampler: Optional['Sampler'] = None  # 实际的Sampler对象


@dataclass
class Vertex:
    """顶点对象 - 保存输入和输出顶点数据"""
    index: int = 0                          # 顶点索引（按输入顺序）
    input_data: Dict[str, Any] = None      # 输入顶点数据（所有字段）
    output_data: Dict[str, Any] = None     # 输出顶点数据（所有字段）
    input_position: List[float] = None     # 输入坐标
    input_normal: List[float] = None       # 输入法向量
    input_color: List[float] = None        # 输入颜色
    input_texcoord: List[float] = None    # 输入纹理坐标
    input_texcoord2: List[float] = None   # 输入第二纹理坐标
    output_position: List[float] = None    # 输出坐标
    output_normal: List[float] = None      # 输出法向量
    output_color: List[float] = None       # 输出颜色
    output_texcoord: List[float] = None    # 输出纹理坐标
    output_texcoord2: List[float] = None   # 输出第二纹理坐标

    def __post_init__(self):
        if self.input_data is None:
            self.input_data = {}
        if self.output_data is None:
            self.output_data = {}


class VertexPool:
    """顶点池 - 根据输入顺序保存所有顶点对象"""

    def __init__(self):
        self.vertices: List[Vertex] = []
        self._input_struct: Optional[StructDefinition] = None
        self._output_struct: Optional[StructDefinition] = None

    def clear(self):
        """清空顶点池"""
        self.vertices.clear()

    def set_input_struct(self, struct: StructDefinition):
        """设置输入结构体定义"""
        self._input_struct = struct

    def set_output_struct(self, struct: StructDefinition):
        """设置输出结构体定义"""
        self._output_struct = struct

    def add_vertex(self, vertex: Vertex):
        """添加顶点到池中"""
        self.vertices.append(vertex)

    def get_vertex(self, index: int) -> Optional[Vertex]:
        """根据索引获取顶点"""
        if 0 <= index < len(self.vertices):
            return self.vertices[index]
        return None

    def get_input_positions(self) -> List[List[float]]:
        """获取所有输入坐标"""
        return [v.input_position for v in self.vertices if v.input_position]

    def get_input_normals(self) -> List[List[float]]:
        """获取所有输入法向量"""
        return [v.input_normal for v in self.vertices if v.input_normal]

    def get_input_colors(self) -> List[List[float]]:
        """获取所有输入颜色"""
        return [v.input_color for v in self.vertices if v.input_color]

    def get_input_texcoords(self) -> List[List[float]]:
        """获取所有输入纹理坐标"""
        return [v.input_texcoord for v in self.vertices if v.input_texcoord]

    def get_input_texcoords2(self) -> List[List[float]]:
        """获取所有输入第二纹理坐标"""
        return [v.input_texcoord2 for v in self.vertices if v.input_texcoord2]

    def get_output_positions(self) -> List[List[float]]:
        """获取所有输出坐标"""
        return [v.output_position for v in self.vertices if v.output_position]

    def get_output_normals(self) -> List[List[float]]:
        """获取所有输出法向量"""
        return [v.output_normal for v in self.vertices if v.output_normal]

    def get_output_colors(self) -> List[List[float]]:
        """获取所有输出颜色"""
        return [v.output_color for v in self.vertices if v.output_color]

    def get_output_texcoords(self) -> List[List[float]]:
        """获取所有输出纹理坐标"""
        return [v.output_texcoord for v in self.vertices if v.output_texcoord]

    def get_output_texcoords2(self) -> List[List[float]]:
        """获取所有输出第二纹理坐标"""
        return [v.output_texcoord2 for v in self.vertices if v.output_texcoord2]

    def build_from_input(self, vs_input: str, input_data: Dict[str, Any], row_index: int):
        """
        根据输入数据构建顶点
        vs_input: 输入结构体名
        input_data: 输入数据字典
        row_index: 行索引
        """
        input_struct = self._input_struct
        if not input_struct:
            return

        vertex = Vertex(index=row_index, input_data=dict(input_data))

        for field in input_struct.fields:
            field_name_lower = field.name.lower()
            field_semantic_upper = field.semantic.upper()
            value = input_data.get(field.name)

            if value is None:
                continue

            if 'pos' in field_name_lower or 'position' in field_name_lower or field_semantic_upper == 'POSITION':
                if isinstance(value, list) and len(value) >= 3:
                    vertex.input_position = value[:3]
            elif 'normal' in field_name_lower or field_semantic_upper == 'NORMAL':
                if isinstance(value, list) and len(value) >= 3:
                    vertex.input_normal = value[:3]
            elif 'color' in field_name_lower or field_semantic_upper == 'COLOR':
                if isinstance(value, list) and len(value) >= 4:
                    vertex.input_color = value[:4]
                elif isinstance(value, list) and len(value) >= 3:
                    vertex.input_color = value[:3] + [1.0]
            elif 'texcoord' in field_name_lower or 'uv' in field_name_lower or field_semantic_upper.startswith('TEXCOORD'):
                if isinstance(value, list):
                    if 'texcoord2' in field_name_lower or 'texcoord1' in field_name_lower or field_semantic_upper == 'TEXCOORD1':
                        vertex.input_texcoord2 = value[:2] if len(value) >= 2 else value
                    else:
                        vertex.input_texcoord = value[:2] if len(value) >= 2 else value

        self.add_vertex(vertex)

    def update_output(self, row_index: int, result: Dict[str, Any]):
        """
        更新顶点的输出数据
        row_index: 行索引
        result: 输出结果字典
        """
        if row_index >= len(self.vertices):
            return

        vertex = self.vertices[row_index]
        vertex.output_data = dict(result) if result else {}

        output_struct = self._output_struct
        if not output_struct:
            for key, value in result.items() if result else {}.items():
                key_lower = key.lower()
                if 'pos' in key_lower or 'position' in key_lower or key.upper() == 'SV_POSITION':
                    if isinstance(value, list) and len(value) >= 3:
                        vertex.output_position = value[:3]
                elif 'normal' in key_lower:
                    if isinstance(value, list) and len(value) >= 3:
                        vertex.output_normal = value[:3]
                elif 'color' in key_lower:
                    if isinstance(value, list) and len(value) >= 4:
                        vertex.output_color = value[:4]
                    elif isinstance(value, list) and len(value) >= 3:
                        vertex.output_color = value[:3] + [1.0]
                elif 'texcoord' in key_lower or 'uv' in key_lower:
                    if isinstance(value, list):
                        if 'texcoord2' in key_lower or 'texcoord1' in key_lower:
                            vertex.output_texcoord2 = value[:2] if len(value) >= 2 else value
                        else:
                            vertex.output_texcoord = value[:2] if len(value) >= 2 else value
            return

        for field in output_struct.fields:
            field_name_lower = field.name.lower()
            field_semantic_upper = field.semantic.upper()
            value = result.get(field.name) if result else None

            if value is None:
                continue

            if 'pos' in field_name_lower or 'position' in field_name_lower or field_semantic_upper == 'POSITION' or field_semantic_upper == 'SV_POSITION':
                if isinstance(value, list) and len(value) >= 3:
                    vertex.output_position = value[:3]
            elif 'normal' in field_name_lower or field_semantic_upper == 'NORMAL':
                if isinstance(value, list) and len(value) >= 3:
                    vertex.output_normal = value[:3]
            elif 'color' in field_name_lower or field_semantic_upper == 'COLOR':
                if isinstance(value, list) and len(value) >= 4:
                    vertex.output_color = value[:4]
                elif isinstance(value, list) and len(value) >= 3:
                    vertex.output_color = value[:3] + [1.0]
            elif 'texcoord' in field_name_lower or 'uv' in field_name_lower or field_semantic_upper.startswith('TEXCOORD'):
                if isinstance(value, list):
                    if 'texcoord2' in field_name_lower or 'texcoord1' in field_name_lower or field_semantic_upper == 'TEXCOORD1':
                        vertex.output_texcoord2 = value[:2] if len(value) >= 2 else value
                    else:
                        vertex.output_texcoord = value[:2] if len(value) >= 2 else value

    def get_count(self) -> int:
        """获取顶点数量"""
        return len(self.vertices)


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
                log_cache_size: int = 10 * 1024 * 1024,
                texture_list: List['Texture'] = None,
                texture_desc_list: List['TextureDesc'] = None,
                sampler_list: List['Sampler'] = None):
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
        self._all_functions = {}                              # 所有解析的函数定义 {func_name: {'ret_type': ..., 'params': {...}, 'body': ...}}
        self.primitive_topology = primitive_topology         # 图元拓扑类型
        self._mesh_view = None                               # MeshView实例(用于显示输入和输出)
        self._mesh_view_enabled = False                      # 是否启用MeshView
        self.vertex_pool = VertexPool()                       # 顶点池
        self._log_cache = []                                 # 日志缓存
        self._log_cache_size = log_cache_size                # 日志缓存大小(字节)
        self._log_cache_bytes = 0                            # 当前缓存已用字节数

        # VS/PS纹理和采样器绑定
        self.texture_bindings: List[TextureBinding] = []     # VS/PS中的纹理绑定列表
        self.sampler_bindings: List[SamplerBinding] = []     # VS/PS中的采样器绑定列表
        self.texture_config_path: str = ""                   # 纹理配置文件路径
        self.sampler_config_path: str = ""                   # 采样器配置文件路径
        self._texture_exec: 'Texture' = texture_list[0] if texture_list else None
        self._texture_desc_list: List['TextureDesc'] = texture_desc_list if texture_desc_list else []
        self._sampler_list: List['Sampler'] = sampler_list if sampler_list else []

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

            # parse_texture_binding: 纹理绑定，如 "Texture2D DiffuseTexture : register(t0);"
            'texture_binding': re.compile(r'Texture2D(?:<[^>]+>)?\s+(\w+)\s*:\s*register\(t(\d+)\)\s*;?'),

            # parse_sampler_binding: 采样器绑定，如 "SamplerState LinearSampler : register(s0);"
            'sampler_binding': re.compile(r'SamplerState\s+(\w+)(?:_s)?\s*:\s*register\(s(\d+)\)\s*;?'),

            # multi-variable declaration without initializer: "float4 r0,r1,r2;"
            'multi_var_decl': re.compile(rf'^({type_pattern})\s+([\w][\w\s,]*)\s*;?$'),

            # swizzle assignment to local variable: "r0.xyzw = expr;"
            'swizzle_var_assign': re.compile(r'^(\w+)\.([xyzwrgba]+)\s*=\s*(.+?);?$'),

            # matrix _mRC accessor pattern: _m00_m10_m20_m30
            'matrix_accessor': re.compile(r'^_m\d\d(?:_m\d\d)*$'),
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

    def set_texture_and_sampler(self, texture_exec, texture_desc_list, sampler_list):
        """
        设置纹理采样执行器及其关联的texture_desc和sampler列表
        texture_exec: Texture对象（纹理采样执行器）
        texture_desc_list: TextureDesc对象列表（纹理参数和纹理数据）
        sampler_list: Sampler对象列表（采样参数）
        """
        self._texture_exec = texture_exec
        self._texture_desc_list = texture_desc_list if texture_desc_list else []
        self._sampler_list = sampler_list if sampler_list else []

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

        positions = self.vertex_pool.get_input_positions()
        normals = self.vertex_pool.get_input_normals()
        colors = self.vertex_pool.get_input_colors()
        texcoords = self.vertex_pool.get_input_texcoords()
        texcoords2 = self.vertex_pool.get_input_texcoords2()

        if not positions:
            self.log_output(f"No input vertices in vertex pool")
            return

        num_rows = len(positions)

        if row_index is not None:
            num_rows = min(row_index + 1, num_rows)
            row_start = row_index
            row_end = row_index + 1
        else:
            row_start = 0
            row_end = num_rows

        positions = positions[row_start:row_end]
        normals = normals[row_start:row_end] if normals and len(normals) >= row_end else None
        colors = colors[row_start:row_end] if colors and len(colors) >= row_end else None
        texcoords = texcoords[row_start:row_end] if texcoords and len(texcoords) >= row_end else None
        texcoords2 = texcoords2[row_start:row_end] if texcoords2 and len(texcoords2) >= row_end else None

        if positions:
            self._mesh_view.clear()
            self._mesh_view.set_primitive_topology(self.primitive_topology)
            self._mesh_view.set_input_data(positions, normals, colors, texcoords, texcoords2)
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

        positions = self.vertex_pool.get_output_positions()
        normals = self.vertex_pool.get_output_normals()
        colors = self.vertex_pool.get_output_colors()
        texcoords = self.vertex_pool.get_output_texcoords()
        texcoords2 = self.vertex_pool.get_output_texcoords2()

        if not positions:
            self.log_output("No output vertices in vertex pool")
            return

        if positions:
            self._mesh_view.set_primitive_topology(self.primitive_topology)
            self._mesh_view.set_output_data(positions, normals, colors, texcoords, texcoords2)
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
        return self._TYPE_SIZE_MAP.get(field_type, 0)

    _TYPE_SIZE_MAP = {
        'float4x4': 64, 'float3x3': 36, 'float4': 16, 'float3': 12,
        'float2': 8, 'float': 4, 'uint4': 16, 'uint3': 12, 'uint2': 8,
        'uint': 4, 'int4': 16, 'int3': 12, 'int2': 8, 'int': 4, 'bool': 4
    }

    def parse_value_by_type(self, value_str: str, field_type: str) -> Any:
        """
        根据类型解析字符串值为对应类型的Python对象
        value_str: 值的字符串表示
        field_type: HLSL类型名
        返回: 解析后的值
        """
        value_str = value_str.strip().strip('"')
        handler = self._PARSE_TYPE_HANDLERS.get(field_type)
        if handler:
            return handler(self, value_str)
        try:
            return float(value_str)
        except:
            return value_str

    def _parse_float4x4(self, value_str):
        parts = value_str.split(',')
        if len(parts) >= 16:
            return [[float(parts[j]) for j in range(i*4, i*4+4)] for i in range(4)]
        return None

    def _parse_float3x3(self, value_str):
        parts = value_str.split(',')
        if len(parts) >= 9:
            return [[float(parts[j]) for j in range(i*3, i*3+3)] for i in range(3)]
        return None

    def _parse_float_vector(self, value_str, count):
        return [float(p) for p in value_str.split(',')[:count]]

    def _parse_int_vector(self, value_str, count):
        return [int(p) for p in value_str.split(',')[:count]]

    def _parse_bool(self, value_str):
        return value_str.lower() in ('true', '1', 'yes')

    _PARSE_TYPE_HANDLERS = {
        'float4x4': _parse_float4x4,
        'float3x3': _parse_float3x3,
        'float4': lambda s, v: s._parse_float_vector(v, 4),
        'float3': lambda s, v: s._parse_float_vector(v, 3),
        'float2': lambda s, v: s._parse_float_vector(v, 2),
        'uint4': lambda s, v: s._parse_int_vector(v, 4),
        'uint3': lambda s, v: s._parse_int_vector(v, 3),
        'uint2': lambda s, v: s._parse_int_vector(v, 2),
        'uint': lambda s, v: int(v),
        'int4': lambda s, v: s._parse_int_vector(v, 4),
        'int3': lambda s, v: s._parse_int_vector(v, 3),
        'int2': lambda s, v: s._parse_int_vector(v, 2),
        'int': lambda s, v: int(v),
        'bool': _parse_bool,
    }

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

    def parse_all_functions(self, code: str):
        """
        解析代码中所有函数定义并存储到_all_functions字典
        code: HLSL代码
        """
        func_pattern = re.compile(r'(\w+)\s+(\w+)\s*\(([^)]*)\)\s*[:\w\s]*\{([^}]+(?:\{[^}]*\}[^}]*)*)\}', re.DOTALL)
        for match in func_pattern.finditer(code):
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
            self._all_functions[func_name] = {
                'ret_type': ret_type,
                'params': params,
                'body': body
            }

    def _get_function_body(self, func_name: str) -> Optional[str]:
        """
        根据函数名获取函数体
        func_name: 函数名
        返回: 函数体字符串，如果未找到返回None
        """
        if func_name in self._all_functions:
            return self._all_functions[func_name]['body']
        return None

    def _collect_function_statements(self, func_name: str, visited: set = None, is_main_func: bool = False) -> List[str]:
        """
        递归收集函数及其调用的其他函数的语句
        func_name: 函数名
        visited: 已访问的函数集合（防止循环调用）
        is_main_func: 是否是主函数（主函数的return语句需要保留）
        返回: 语句列表
        """
        if visited is None:
            visited = set()

        if func_name in visited:
            return []
        visited.add(func_name)

        body = self._get_function_body(func_name)
        if body is None:
            return []

        statements = self.GenerateStmts(body.strip())

        result_statements = []
        for stmt in statements:
            if stmt is None:
                continue

            called_funcs = self._find_function_calls_in_statement(stmt)
            for called_func in called_funcs:
                if called_func in self._all_functions and called_func not in visited:
                    nested_statements = self._collect_function_statements(called_func, visited, is_main_func=False)
                    result_statements.extend(nested_statements)

            result_statements.append(stmt)

        return result_statements

    def _find_function_calls_in_statement(self, stmt: str) -> List[str]:
        """
        从语句中查找用户定义的函数调用
        stmt: 语句字符串
        返回: 函数名列表
        """
        func_calls = []
        func_pattern = re.compile(r'(\w+)\s*\(')
        for match in func_pattern.finditer(stmt):
            func_name = match.group(1)
            if func_name not in ['if', 'for', 'while', 'do', 'switch']:
                func_calls.append(func_name)
        return func_calls

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
        def _neg(v):
            if isinstance(v, bool):
                return -1 if v else 0
            if isinstance(v, (int, float)):
                return -v
            return v

        if op == '-':
            if isinstance(val, list):
                result = [_neg(v) for v in val]
            else:
                result = _neg(val)
        else:
            result = not bool(val)
        if self.debug and self._should_print:
            self.debug_print(f"[UNARY OP] operand={self._format_value(val)}, op={op}, result={self._format_value(result)}")
        return result

    def execute_binary_op(self, op: str, left: Any, right: Any) -> Any:
        """
        执行二元运算符
        op: 运算符 '+', '-', '*', '/', '.'
        left, right: 左右操作数
        """
        if left is None and op == '-':
            # Unary negation encoded as binary with empty left side: -(expr)
            return self.execute_unary_op('-', right)
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
            def _safe_div(a, b):
                if b == 0 or b == 0.0:
                    return float('inf') if (a is None or a >= 0) else float('-inf')
                return a / b
            if isinstance(left, list) and isinstance(right, (int, float)):
                result = [_safe_div(v, right) for v in left]
            elif isinstance(left, list) and isinstance(right, list):
                result = [_safe_div(l, r) for l, r in zip(left, right)]
            elif isinstance(right, (int, float)):
                result = _safe_div(left, right)
            else:
                result = left / right if right else 0.0
        elif op == '.':
            result = (left, right)
        elif op == '==':
            if isinstance(left, list) or isinstance(right, list):
                lv = left if isinstance(left, list) else [left] * (len(right) if isinstance(right, list) else 1)
                rv = right if isinstance(right, list) else [right] * len(lv)
                result = [1 if l == r else 0 for l, r in zip(lv, rv)]
            else:
                result = left == right
        elif op == '!=':
            if isinstance(left, list) or isinstance(right, list):
                lv = left if isinstance(left, list) else [left] * (len(right) if isinstance(right, list) else 1)
                rv = right if isinstance(right, list) else [right] * len(lv)
                result = [1 if l != r else 0 for l, r in zip(lv, rv)]
            else:
                result = left != right
        elif op == '<':
            if isinstance(left, list) or isinstance(right, list):
                lv = left if isinstance(left, list) else [left] * (len(right) if isinstance(right, list) else 1)
                rv = right if isinstance(right, list) else [right] * len(lv)
                result = [1 if l < r else 0 for l, r in zip(lv, rv)]
            else:
                result = left < right
        elif op == '>':
            if isinstance(left, list) or isinstance(right, list):
                lv = left if isinstance(left, list) else [left] * (len(right) if isinstance(right, list) else 1)
                rv = right if isinstance(right, list) else [right] * len(lv)
                result = [1 if l > r else 0 for l, r in zip(lv, rv)]
            else:
                result = left > right
        elif op == '<=':
            if isinstance(left, list) or isinstance(right, list):
                lv = left if isinstance(left, list) else [left] * (len(right) if isinstance(right, list) else 1)
                rv = right if isinstance(right, list) else [right] * len(lv)
                result = [1 if l <= r else 0 for l, r in zip(lv, rv)]
            else:
                result = left <= right
        elif op == '>=':
            if isinstance(left, list) or isinstance(right, list):
                lv = left if isinstance(left, list) else [left] * (len(right) if isinstance(right, list) else 1)
                rv = right if isinstance(right, list) else [right] * len(lv)
                result = [1 if l >= r else 0 for l, r in zip(lv, rv)]
            else:
                result = left >= right
        elif op == '&&':
            result = bool(self._to_bool(left) and self._to_bool(right))
        elif op == '||':
            result = bool(self._to_bool(left) or self._to_bool(right))
        elif op == '|':
            if isinstance(left, list) and isinstance(right, list):
                result = [int(l) | int(r) for l, r in zip(left, right)]
            elif isinstance(left, list):
                result = [int(l) | int(right) for l in left]
            elif isinstance(right, list):
                result = [int(left) | int(r) for r in right]
            else:
                result = int(left) | int(right)
        elif op == '&':
            if isinstance(left, list) and isinstance(right, list):
                result = [int(l) & int(r) for l, r in zip(left, right)]
            elif isinstance(left, list):
                result = [int(l) & int(right) for l in left]
            elif isinstance(right, list):
                result = [int(left) & int(r) for r in right]
            else:
                result = int(left) & int(right)
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
        n = len(m)
        return [[m[j][i] for j in range(n)] for i in range(n)]

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
        return [sum(v[i] * m[i][j] for i in range(len(v))) for j in range(len(m[0]))]

    def mul_matrix_matrix(self, a: List[List[float]], b: List[List[float]]) -> List[List[float]]:
        """
        矩阵乘法: result = a * b
        a, b: n x n 方阵
        返回: 结果矩阵
        """
        n = len(a)
        return [[sum(a[i][k] * b[k][j] for k in range(n)) for j in range(n)] for i in range(n)]

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

        elif node.node_type == 'method_call':
            return self.execute_method_call_node(node, local_vars)

        elif node.node_type == 'ternary':
            cond = self.evaluate_syntax_tree(node.left, local_vars)
            if self._to_bool(cond):
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
            # int/uint cast: 转换为整数
            if cast_type in ('int', 'int2', 'int3', 'int4', 'uint', 'uint2', 'uint3', 'uint4'):
                if isinstance(inner, list):
                    return [int(v) for v in inner]
                return int(inner) if inner is not None else 0
            # float cast: 转换为浮点数
            if cast_type in ('float', 'float2', 'float3', 'float4'):
                if isinstance(inner, list):
                    return [float(v) for v in inner]
                return float(inner) if inner is not None else 0.0
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

        # max: 最大值函数 (支持标量和向量)
        elif func_name == 'max':
            if len(args) != 2:
                self.debug_print(f"[ERROR] max requires 2 args, got {len(args)} at line {node.line_number}")
                return None
            a = self.evaluate_syntax_tree(args[0], local_vars)
            b = self.evaluate_syntax_tree(args[1], local_vars)
            if a is None or b is None:
                return None
            if isinstance(a, list) and isinstance(b, list):
                result = [max(x, y) for x, y in zip(a, b)]
            elif isinstance(a, list):
                result = [max(x, b) for x in a]
            elif isinstance(b, list):
                result = [max(a, y) for y in b]
            else:
                result = max(a, b)
            self.debug_print(f"[FUNC] max({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
            return result

        # min: 最小值函数 (支持标量和向量)
        elif func_name == 'min':
            if len(args) != 2:
                self.debug_print(f"[ERROR] min requires 2 args, got {len(args)} at line {node.line_number}")
                return None
            a = self.evaluate_syntax_tree(args[0], local_vars)
            b = self.evaluate_syntax_tree(args[1], local_vars)
            if a is None or b is None:
                return None
            if isinstance(a, list) and isinstance(b, list):
                result = [min(x, y) for x, y in zip(a, b)]
            elif isinstance(a, list):
                result = [min(x, b) for x in a]
            elif isinstance(b, list):
                result = [min(a, y) for y in b]
            else:
                result = min(a, b)
            self.debug_print(f"[FUNC] min({self._format_float(a)}, {self._format_float(b)}) = {self._format_float(result)}")
            return result

        # rsqrt: 倒数平方根函数 1/sqrt(x)
        elif func_name == 'rsqrt':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [1.0 / math.sqrt(max(v, 1e-30)) if v > 0 else 0.0 for v in val]
            else:
                result = 1.0 / math.sqrt(max(val, 1e-30)) if val > 0 else 0.0
            self.debug_print(f"[FUNC] rsqrt({self._format_float(val)}) = {self._format_float(result)}")
            return result

        # sqrt: 平方根函数
        elif func_name == 'sqrt':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [math.sqrt(max(v, 0.0)) for v in val]
            else:
                result = math.sqrt(max(val, 0.0))
            self.debug_print(f"[FUNC] sqrt({self._format_float(val)}) = {self._format_float(result)}")
            return result

        # log2: 以2为底的对数
        elif func_name == 'log2':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [math.log2(max(v, 1e-30)) for v in val]
            else:
                result = math.log2(max(val, 1e-30)) if val > 0 else -1e30
            self.debug_print(f"[FUNC] log2({self._format_float(val)}) = {self._format_float(result)}")
            return result

        # exp2: 2的幂次方
        elif func_name == 'exp2':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [math.pow(2.0, v) for v in val]
            else:
                result = math.pow(2.0, val)
            self.debug_print(f"[FUNC] exp2({self._format_float(val)}) = {self._format_float(result)}")
            return result

        # clamp: 限制范围函数
        elif func_name == 'clamp':
            if len(args) != 3:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            lo = self.evaluate_syntax_tree(args[1], local_vars)
            hi = self.evaluate_syntax_tree(args[2], local_vars)
            if val is None:
                return None
            lo = lo if lo is not None else 0.0
            hi = hi if hi is not None else 1.0
            if isinstance(val, list):
                lo_v = lo if isinstance(lo, list) else [lo] * len(val)
                hi_v = hi if isinstance(hi, list) else [hi] * len(val)
                result = [max(min(v, h), l) for v, l, h in zip(val, lo_v, hi_v)]
            else:
                result = max(min(val, hi), lo)
            return result

        # saturate: 限制在[0,1]范围
        elif func_name == 'saturate':
            if len(args) != 1:
                return None
            val = self.evaluate_syntax_tree(args[0], local_vars)
            if val is None:
                return None
            if isinstance(val, list):
                result = [max(0.0, min(1.0, v)) for v in val]
            else:
                result = max(0.0, min(1.0, val))
            return result

        # lerp: 线性插值
        elif func_name == 'lerp':
            if len(args) != 3:
                return None
            a = self.evaluate_syntax_tree(args[0], local_vars)
            b = self.evaluate_syntax_tree(args[1], local_vars)
            t = self.evaluate_syntax_tree(args[2], local_vars)
            if a is None or b is None or t is None:
                return None
            if isinstance(a, list) and isinstance(b, list):
                t_v = t if isinstance(t, list) else [t] * len(a)
                result = [av + (bv - av) * tv for av, bv, tv in zip(a, b, t_v)]
            else:
                result = a + (b - a) * t
            return result

        # int2/int3/int4: 整数向量构造
        elif func_name in ('int2', 'int3', 'int4', 'uint2', 'uint3', 'uint4'):
            result = []
            for arg in args:
                val = self.evaluate_syntax_tree(arg, local_vars)
                if isinstance(val, list):
                    result.extend(int(v) for v in val)
                elif val is not None:
                    result.append(int(val))
            self.debug_print(f"[FUNC] {func_name}(...) = {result}")
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

        # Texture.Sample: 纹理采样函数
        # 格式: DiffuseTexture.Sample(LinearSampler, input.TexCoord)
        # DiffuseTexture 是 Texture2D，LinearSampler 是 SamplerState
        elif func_name == 'Sample' and len(args) == 2:
            if len(node.args) < 1:
                return None
            texture_node = node.args[0]
            texture_name = texture_node.value if texture_node and texture_node.node_type == 'value' else None
            if texture_name:
                sampler_node = args[0] if isinstance(args[0], SyntaxTreeNode) else None
                coords_node = args[1] if len(args) > 1 else None
                coords = self.evaluate_syntax_tree(coords_node, local_vars) if coords_node else None
                if coords and isinstance(coords, list) and len(coords) >= 2:
                    u, v = coords[0], coords[1]
                    w = coords[2] if len(coords) > 2 else 0.0
                    binding = self._find_texture_binding(texture_name)
                    if binding and self._texture_exec and self._texture_desc_list and self._sampler_list:
                        reg_id = binding.register_id
                        if reg_id < len(self._texture_desc_list) and reg_id < len(self._sampler_list):
                            texture_desc = self._texture_desc_list[reg_id]
                            sampler = self._sampler_list[reg_id]
                            result = self._texture_exec.sample(u, v, w, texture_desc, sampler)
                            self.debug_print(f"[FUNC] {texture_name}.Sample(..., ({u:.4f}, {v:.4f})) = {self._format_float(result)}")
                            return result
            return None

        return None

    def execute_method_call_node(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
        """
        执行方法调用语法树节点 (如 Texture.Sample)
        node: 方法调用节点
        local_vars: 局部变量字典
        返回: 方法执行结果
        """
        method_name = node.value
        obj_node = node.left
        args = node.args

        if method_name == 'Sample' and len(args) == 2:
            if obj_node is None:
                return None
            obj_name = obj_node.value if obj_node.node_type == 'value' else None
            if obj_name is None:
                return None
            coords = self.evaluate_syntax_tree(args[1], local_vars)
            if coords and isinstance(coords, list) and len(coords) >= 2:
                u, v = coords[0], coords[1]
                w = coords[2] if len(coords) > 2 else 0.0
                binding = self._find_texture_binding(obj_name)
                if binding and self._texture_exec and self._texture_desc_list and self._sampler_list:
                    reg_id = binding.register_id
                    if reg_id < len(self._texture_desc_list) and reg_id < len(self._sampler_list):
                        texture_desc = self._texture_desc_list[reg_id]
                        sampler = self._sampler_list[reg_id]
                        result = self._texture_exec.sample(u, v, w, texture_desc, sampler)
                        self.debug_print(f"[METHOD] {obj_name}.Sample(..., ({u:.4f}, {v:.4f})) = {self._format_float(result)}")
                        return result
            return None

        self.debug_print(f"[ERROR] Unknown method: {method_name}")
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
            return obj if swizzle == 'x' else None

        result = []
        for c in swizzle:
            if c.lower() in self._SWIZZLE_MAP:
                idx = self._SWIZZLE_MAP[c.lower()]
                result.append(obj[idx] if idx < len(obj) else 0)
            elif c in 'rgb':
                idx = {'r': 0, 'g': 1, 'b': 2}[c]
                result.append(obj[idx] if idx < len(obj) else 0)

        if len(result) == 1:
            return result[0]

        numeric_types = (int, float)
        if all(isinstance(v, numeric_types) for v in result):
            return [int(v) for v in result] if all(isinstance(v, int) for v in result) else result

        return result

    _SWIZZLE_MAP = {'x': 0, 'y': 1, 'z': 2, 'w': 3}

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
                    # 检查矩阵 _mRC 访问器: WorldViewProj._m01_m11_m21_m31
                    if len(parts) == 2 and self.patterns['matrix_accessor'].match(parts[1]):
                        base = parts[0]
                        prop = parts[1]
                        matrix = local_vars.get(base) or self.variables.get(base)
                        if matrix is None:
                            for cb_def in self.cbuffers.values():
                                if isinstance(cb_def, CbufferDefinition):
                                    for field in cb_def.fields:
                                        if field.name == base and field.data is not None:
                                            matrix = field.data
                                            break
                                if matrix is not None:
                                    break
                        if matrix is not None and isinstance(matrix, list) and matrix and isinstance(matrix[0], list):
                            accessor_parts = [p for p in prop.split('_') if p and p[0] == 'm' and len(p) == 3]
                            result = []
                            for ap in accessor_parts:
                                try:
                                    r, c = int(ap[1]), int(ap[2])
                                    result.append(matrix[r][c] if r < len(matrix) and c < len(matrix[r]) else 0.0)
                                except (ValueError, IndexError):
                                    result.append(0.0)
                            if len(result) == 1:
                                return result[0]
                            return result if result else 0.0

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

        # if-else条件语句处理
        if stmt.startswith('if'):
            self.execute_if_statement(stmt, local_vars)
            return None

        # 多变量声明无初始值: float4 r0,r1,r2; 或 uint4 bitmask, uiDest;
        if '=' not in stmt:
            match = self.patterns['multi_var_decl'].match(stmt)
            if match:
                var_type = match.group(1)
                var_names_str = match.group(2)
                var_names = [n.strip() for n in var_names_str.split(',') if n.strip() and re.match(r'^\w+$', n.strip())]
                if var_names:
                    default = self._default_value_for_type(var_type)
                    for vname in var_names:
                        if vname not in local_vars:
                            local_vars[vname] = list(default) if isinstance(default, list) else default
                    self.debug_print(f"[DECL] {var_type} {var_names}")
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

        # swizzle分量赋值: r0.xyzw = expr; 或 o0.xyz = expr;
        # Note: don't check count('=') here since RHS may contain >= <= == comparisons
        if '.' in stmt and '=' in stmt:
            match = self.patterns['swizzle_var_assign'].match(stmt)
            if match:
                var_name = match.group(1)
                swizzle = match.group(2)
                value = self.evaluate_expression(match.group(3).rstrip(';').strip(), local_vars)
                self._apply_swizzle_assign(var_name, swizzle, value, local_vars)
                self.debug_print(f"[STMT] {stmt} => {var_name}.{swizzle} = {self._format_float(value)}")
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


    def execute_main_function(self, code: str, main_func: str, input_struct_name: str, row_index: int, data: Dict[str, Any], shader_stage: int = None):
        """
        执行HLSL main函数
        code: HLSL代码
        main_func: main函数名
        input_struct_name: 输入结构体名
        row_index: 数据行索引
        data: 输入数据字典
        shader_stage: shader阶段常量 (SHADER_STAGE_VS/HS/DS/GS/PS/CS)
        返回: output结构体字典
        """
        from d3d import SHADER_STAGE_PS
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

        if output_struct_name in self.structs:
            output_struct = self.structs[output_struct_name]
        else:
            output_struct = None

        output_fields = {}
        is_ps = (shader_stage == SHADER_STAGE_PS)
        if output_struct is not None:
            for field in output_struct.fields:
                output_fields[field.name] = field.field_type
        else:
            if is_ps:
                output_fields['Color'] = 'float4'

        func_signature = rf'{output_struct_name}\s+{main_func}\s*\(\s*{input_struct_name_from_func}\s+input\s*\)'

        cache_key = f"{output_struct_name}_{main_func}_{input_struct_name_from_func}"
        if cache_key in self._parsed_func_cache:
            cached = self._parsed_func_cache[cache_key]
            body = cached['body']
            statements = cached['statements']
        else:
            statements = self._collect_function_statements(main_func)
            body = ""
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
                self.debug_print(f"  {k} = {self._format_float(v)}")
        self.debug_print(f"==================")

        # 顺序执行语句
        i = 0
        while i < len(statements):
            stmt = statements[i]
            if stmt is None:
                i += 1
                continue

            if 'return' in stmt and ('output' in stmt or is_ps):
                if is_ps:
                    return_val_match = re.search(r'return\s+(.+?)\s*$', stmt)
                    if return_val_match:
                        expr = return_val_match.group(1).strip()
                        ret_val = self.evaluate_expression(expr, local_vars)
                else:
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

        self._parse_texture_and_sampler_bindings(code)

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

        self.parse_all_functions(code)

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
        else:
            if not self._all_functions:
                self.parse_all_functions(code)
        self._last_executeVS_code = code
        input_struct = self.structs.get(vs_input)
        if not input_struct:
            self.log_output(f"Cannot find vs input: {vs_input}\n")
            return None

        output_struct_name = None
        func_signature_pattern = r'(\w+)\s+' + re.escape(main_func) + r'\s*\(\s*(\w+)\s+input\s*\)'
        func_signature_match = re.search(func_signature_pattern, code)
        if func_signature_match:
            output_struct_name = func_signature_match.group(1)

        output_struct = self.structs.get(output_struct_name) if output_struct_name else None

        self.vertex_pool.clear()
        self.vertex_pool.set_input_struct(input_struct)
        self.vertex_pool.set_output_struct(output_struct)

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
                self.vertex_pool.build_from_input(vs_input, data, row_index)
                from d3d import SHADER_STAGE_VS
                result = self.execute_main_function(code, main_func, vs_input, row_index, data, SHADER_STAGE_VS)
                self.vertex_pool.update_output(row_index, result)
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
                self.vertex_pool.build_from_input(vs_input, data, row_index)
                from d3d import SHADER_STAGE_VS
                result = self.execute_main_function(code, main_func, vs_input, row_index, data, SHADER_STAGE_VS)
                self.vertex_pool.update_output(row_index, result)
                results.append(result)

        return results

    def executePS(self, main_func: str, ps_input: str, pixels: List['Pixel']):
        """
        执行像素着色器
        code: HLSL代码
        main_func: 入口函数名
        ps_input: 输入结构体名
        pixels: 光栅化后的像素列表
        返回: 更新了ps_output_color的像素列表
        """
        code = self.hlsl_code
        if not self._all_functions:
            self.parse_all_functions(code)

        input_struct = self.structs.get(ps_input)
        if not input_struct:
            self.log_output(f"Cannot find ps input: {ps_input}\n")
            return pixels

        output_struct_name = None
        func_signature_pattern = r'(\w+)\s+' + re.escape(main_func) + r'\s*\(\s*(\w+)\s+input\s*\)'
        func_signature_match = re.search(func_signature_pattern, code)
        if func_signature_match:
            output_struct_name = func_signature_match.group(1)

        output_struct = self.structs.get(output_struct_name) if output_struct_name else None

        self._eval_counter = 0

        for pixel in pixels:
            pixel.ps_output_color = None

            data = {
                'Color': pixel.color if pixel.color else [1.0, 1.0, 1.0, 1.0],
                'TexCoord': pixel.texcoord if pixel.texcoord else [0.0, 0.0],
                'TexCoord2': pixel.texcoord2 if pixel.texcoord2 else [0.0, 0.0],
                'Normal': pixel.normal if pixel.normal else [0.0, 0.0, 1.0],
                'WorldPos': pixel.worldPos if pixel.worldPos else [0.0, 0.0, 0.0],
            }
            data.update(pixel.attributes)

            from d3d import SHADER_STAGE_PS
            result = self.execute_main_function(code, main_func, ps_input, 0, data, SHADER_STAGE_PS)

            pixel.ps_output_color = result

        return pixels

    def _parse_texture_and_sampler_bindings(self, code: str):
        """
        解析HLSL代码中的纹理和采样器绑定
        code: HLSL代码
        """
        self.texture_bindings = []
        self.sampler_bindings = []

        if not TEXTURE_AVAILABLE:
            self.log_output("Warning: texture module not available")
            return

        for match in self.patterns['texture_binding'].finditer(code):
            var_name = match.group(1)
            reg_id = int(match.group(2))
            binding = TextureBinding(variable_name=var_name, register_id=reg_id)
            self.texture_bindings.append(binding)

        for match in self.patterns['sampler_binding'].finditer(code):
            var_name = match.group(1)
            reg_id = int(match.group(2))
            binding = SamplerBinding(variable_name=var_name, register_id=reg_id)
            self.sampler_bindings.append(binding)

        for binding in self.texture_bindings:
            for sbinding in self.sampler_bindings:
                if binding.register_id == sbinding.register_id:
                    reg_id = binding.register_id
                    if self._texture_exec and reg_id < len(self._texture_desc_list) and reg_id < len(self._sampler_list):
                        binding.texture = self._texture_exec
                        binding.texture_desc = self._texture_desc_list[reg_id]
                        binding.sampler = self._sampler_list[reg_id]

    def get_pixel_shader_output(self, pixels: List['Pixel']) -> List[List[float]]:
        """
        获取像素着色器的输出颜色
        pixels: 像素列表
        返回: 输出颜色列表
        """
        return [p.ps_output_color if p.ps_output_color else p.color for p in pixels]

    def _find_texture_binding(self, texture_name: str) -> Optional[TextureBinding]:
        """
        根据纹理变量名查找纹理绑定
        texture_name: 纹理变量名，如 DiffuseTexture
        返回: TextureBinding对象或None
        """
        for binding in self.texture_bindings:
            if binding.variable_name == texture_name:
                return binding
        return None

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

    def get_cbuffer_data(self):
        """
        Get all cbuffer definitions and their data
        Returns: dict of cbuffer_name -> {fields: [{name, field_type, data}, ...]}
        """
        result = {}
        for cb_name, cb_def in self.cbuffers.items():
            if isinstance(cb_def, CbufferDefinition):
                fields_data = []
                for field in cb_def.fields:
                    fields_data.append({
                        'name': field.name,
                        'field_type': field.field_type,
                        'data': field.data
                    })
                result[cb_name] = {'fields': fields_data}
        return result

    def get_hlsl_code(self):
        """
        Get the current HLSL code
        Returns: str - the HLSL code
        """
        return self.hlsl_code

    def get_last_executeVS_code(self):
        """
        Get the last code used in executeVS
        Returns: str - the last code passed to executeVS, or None
        """
        return getattr(self, '_last_executeVS_code', None)

    # =========================================================
    # New param-based execution methods for void main(...) style
    # =========================================================

    def _to_bool(self, val) -> bool:
        """Truthiness for scalars and vectors (non-empty list is true only if any element nonzero)."""
        if val is None:
            return False
        if isinstance(val, list):
            return any(v is not None and v not in (0, 0.0, False) for v in val)
        if isinstance(val, float) and (val != val):  # NaN check
            return False
        return bool(val)

    def _default_value_for_type(self, type_str: str):
        """Return default (zero) value for an HLSL type."""
        defaults = {
            'float4x4': [[0.0, 0.0, 0.0, 0.0]] * 4,
            'float3x3': [[0.0, 0.0, 0.0]] * 3,
            'float4': [0.0, 0.0, 0.0, 0.0],
            'float3': [0.0, 0.0, 0.0],
            'float2': [0.0, 0.0],
            'float': 0.0,
            'uint4': [0, 0, 0, 0], 'uint3': [0, 0, 0], 'uint2': [0, 0], 'uint': 0,
            'int4': [0, 0, 0, 0], 'int3': [0, 0, 0], 'int2': [0, 0], 'int': 0,
            'bool': False,
        }
        v = defaults.get(type_str)
        if isinstance(v, list):
            return [list(row) if isinstance(row, list) else row for row in v]
        return v if v is not None else 0.0

    def _type_component_count(self, type_str: str) -> int:
        """Return number of scalar components for an HLSL type."""
        counts = {
            'float4x4': 16, 'float3x3': 9,
            'float4': 4, 'float3': 3, 'float2': 2, 'float': 1,
            'uint4': 4, 'uint3': 3, 'uint2': 2, 'uint': 1,
            'int4': 4, 'int3': 3, 'int2': 2, 'int': 1,
            'bool': 1,
        }
        return counts.get(type_str, 4)

    def _apply_swizzle_assign(self, var_name: str, swizzle: str, value, local_vars: dict):
        """Assign components of a vector variable via swizzle notation."""
        swizzle_map = {'x': 0, 'y': 1, 'z': 2, 'w': 3, 'r': 0, 'g': 1, 'b': 2, 'a': 3}
        current = local_vars.get(var_name)
        if current is None:
            current = [0.0, 0.0, 0.0, 0.0]
        elif not isinstance(current, list):
            current = [float(current), 0.0, 0.0, 0.0]
        else:
            current = list(current)
        # Extend list if swizzle accesses beyond current length
        indices = [swizzle_map[ch] for ch in swizzle.lower() if ch in swizzle_map]
        if indices:
            needed = max(indices) + 1
            while len(current) < needed:
                current.append(0.0)
        if isinstance(value, list):
            for i, ch in enumerate(swizzle.lower()):
                if ch in swizzle_map and i < len(value):
                    current[swizzle_map[ch]] = value[i]
        elif isinstance(value, (int, float)):
            for ch in swizzle.lower():
                if ch in swizzle_map:
                    current[swizzle_map[ch]] = float(value)
        local_vars[var_name] = current

    def preprocess_hlsl(self, code: str) -> str:
        """Preprocess HLSL code: expand #define macros, strip preprocessor directives."""
        defines = {}
        result_lines = []
        for line in code.split('\n'):
            stripped = line.strip()
            if stripped.startswith('#define'):
                parts = stripped.split(None, 2)
                if len(parts) >= 3:
                    defines[parts[1]] = parts[2].strip()
                elif len(parts) == 2:
                    defines[parts[1]] = ''
                continue
            elif stripped.startswith('#'):
                continue
            for name, replacement in defines.items():
                line = re.sub(r'\b' + re.escape(name) + r'\b', replacement, line)
            result_lines.append(line)
        return '\n'.join(result_lines)

    def parse_main_params_with_semantics(self, code: str, func_name: str = 'main') -> Optional[dict]:
        """
        Parse void main(...) parameter list.
        Returns {'inputs': [...], 'outputs': [...]} where each entry has:
          name, type, semantic, semantic_base, semantic_index, is_out, slot (-1 until mapped)
        """
        pattern = re.compile(
            r'void\s+' + re.escape(func_name) + r'\s*\(\s*(.*?)\s*\)\s*\{',
            re.DOTALL
        )
        match = pattern.search(code)
        if not match:
            return None

        params_str = match.group(1)
        param_list = [p.strip() for p in params_str.split(',') if p.strip()]
        params = []

        for param_str in param_list:
            param_str = param_str.strip()
            is_out = False
            if param_str.lower().startswith('out '):
                is_out = True
                param_str = param_str[4:].strip()
            elif param_str.lower().startswith('inout '):
                param_str = param_str[6:].strip()

            semantic = ''
            if ':' in param_str:
                colon_pos = param_str.index(':')
                type_name_part = param_str[:colon_pos].strip()
                semantic = param_str[colon_pos + 1:].strip()
            else:
                type_name_part = param_str

            parts = type_name_part.split()
            if len(parts) >= 2:
                param_type = parts[0]
                param_name = parts[-1]
            elif len(parts) == 1:
                param_type = parts[0]
                param_name = '_unnamed'
            else:
                continue

            # Parse semantic index: SV_POSITION0 -> base=SV_POSITION, index=0
            semantic_base = semantic
            semantic_index = 0
            idx_match = re.match(r'^(.*?)(\d+)$', semantic)
            if idx_match:
                semantic_base = idx_match.group(1)
                semantic_index = int(idx_match.group(2))

            params.append({
                'name': param_name,
                'type': param_type,
                'semantic': semantic,
                'semantic_base': semantic_base,
                'semantic_index': semantic_index,
                'is_out': is_out,
                'slot': -1,
            })

        return {
            'inputs': [p for p in params if not p['is_out']],
            'outputs': [p for p in params if p['is_out']],
        }

    def load_signature_from_csv(self, csv_path: str) -> dict:
        """
        Parse VS_input_output_signature.csv or PS_input_output_signature.csv.
        Returns {'inputs': [...], 'outputs': [...]} with slot/index/semantic info.
        """
        if not os.path.exists(csv_path):
            return {'inputs': [], 'outputs': []}
        rows = self.load_csv(csv_path)
        if not rows or len(rows) < 2:
            return {'inputs': [], 'outputs': []}

        header = [h.strip() for h in rows[0]]
        type_idx = header.index('Type') if 'Type' in header else 0
        slot_idx = header.index('Slot') if 'Slot' in header else 1
        index_idx = header.index('Index') if 'Index' in header else 2
        name_idx = header.index('SemanticName') if 'SemanticName' in header else 3

        inputs, outputs = [], []
        for row in rows[1:]:
            if len(row) < 4:
                continue
            sig_type = row[type_idx].strip()
            try:
                slot = int(row[slot_idx].strip())
                idx = int(row[index_idx].strip())
            except ValueError:
                continue
            semantic = row[name_idx].strip()
            entry = {'slot': slot, 'index': idx, 'semantic': semantic}
            if sig_type == 'Input':
                inputs.append(entry)
            elif sig_type == 'Output':
                outputs.append(entry)

        return {'inputs': inputs, 'outputs': outputs}

    def map_params_to_signature(self, params: list, signature_entries: list):
        """Fill in slot field for each param by matching semantic base and index."""
        for param in params:
            sem_base = param['semantic_base'].upper()
            sem_idx = param['semantic_index']
            for sig in signature_entries:
                if sig['semantic'].upper() == sem_base and sig['index'] == sem_idx:
                    param['slot'] = sig['slot']
                    break

    def load_ia_vertex_data(self, csv_path: str, vs_input_params: list) -> list:
        """
        Load ia_vertex_data.csv and map columns to VS input param names by semantic.
        Returns list of dicts {param_name: value} per vertex.
        """
        if not os.path.exists(csv_path):
            return []
        rows = self.load_csv(csv_path)
        if not rows or len(rows) < 2:
            return []

        header = [col.strip() for col in rows[0]]
        col_map = {col: i for i, col in enumerate(header)}

        # Build per-param column mapping
        param_col_map = {}
        for param in vs_input_params:
            sem_base = param['semantic_base']
            sem_idx = param['semantic_index']
            ptype = param['type']

            # Try SEMANTIC0 first, then SEMANTIC, then SEMANTIC + index
            candidates = [
                f'{sem_base}{sem_idx}',
                sem_base,
                f'{sem_base}_{sem_idx}',
            ]
            components = {}
            for cand in candidates:
                for suffix in ['x', 'y', 'z', 'w']:
                    col = f'{cand}.{suffix}'
                    if col in col_map:
                        components[suffix] = col_map[col]
                if components:
                    break

            param_col_map[param['name']] = {'type': ptype, 'components': components}

        vertices = []
        for row in rows[1:]:
            vertex = {}
            for param_name, info in param_col_map.items():
                ptype = info['type']
                comp = info['components']
                try:
                    if 'w' in comp:
                        vertex[param_name] = [float(row[comp['x']]), float(row[comp['y']]),
                                            float(row[comp['z']]), float(row[comp['w']])]
                    elif 'z' in comp:
                        vertex[param_name] = [float(row[comp['x']]), float(row[comp['y']]),
                                            float(row[comp['z']])]
                    elif 'y' in comp:
                        vertex[param_name] = [float(row[comp['x']]), float(row[comp['y']])]
                    elif 'x' in comp:
                        vertex[param_name] = float(row[comp['x']])
                    else:
                        vertex[param_name] = self._default_value_for_type(ptype)
                except (ValueError, IndexError):
                    vertex[param_name] = self._default_value_for_type(ptype)
            vertices.append(vertex)

        return vertices

    def load_vs_golden_from_mesh_csv(self, csv_path: str, vs_output_params: list) -> list:
        """
        Load MeshOut_vs_mesh.csv golden data. Returns list of dicts with canonical keys.
        """
        if not os.path.exists(csv_path):
            return []
        rows = self.load_csv(csv_path)
        if not rows or len(rows) < 2:
            return []

        header = [col.strip() for col in rows[0]]
        col_map = {col: i for i, col in enumerate(header)}
        sem_to_key = self._get_output_semantic_to_key_map()

        golden = []
        for row in rows[1:]:
            entry = {}
            for param in vs_output_params:
                sem_base = param['semantic_base']
                sem_idx = param['semantic_index']

                # Try both TEXCOORD0 style (with index) and SV_POSITION style (no index)
                candidates = [f'{sem_base}{sem_idx}', sem_base]
                comp = {}
                for cand in candidates:
                    for suffix in ['x', 'y', 'z', 'w']:
                        c = f'{cand}.{suffix}'
                        if c in col_map:
                            comp[suffix] = col_map[c]
                    if comp:
                        break

                # Choose canonical key
                sem_full = f'{sem_base.upper()}{sem_idx}' if sem_idx > 0 else sem_base.upper()
                key = sem_to_key.get(sem_full, sem_to_key.get(sem_base.upper(), sem_base))

                try:
                    if 'w' in comp:
                        val = [float(row[comp[s]]) for s in ['x','y','z','w']]
                    elif 'z' in comp:
                        val = [float(row[comp[s]]) for s in ['x','y','z']]
                    elif 'y' in comp:
                        val = [float(row[comp[s]]) for s in ['x','y']]
                    elif 'x' in comp:
                        val = float(row[comp['x']])
                    else:
                        val = None

                    if val is not None:
                        entry[key] = val
                except (ValueError, IndexError):
                    pass
            golden.append(entry)

        return golden

    def _get_output_semantic_to_key_map(self) -> dict:
        """Map VS/PS output semantic names to canonical dict keys for rasterizer/pixel."""
        return {
            'SV_POSITION': 'sv_position',
            'COLOR': 'Color',
            'COLOR0': 'Color',
            'TEXCOORD': 'TexCoord',
            'TEXCOORD0': 'TexCoord',
            'TEXCOORD1': 'TexCoord2',
            'NORMAL': 'Normal',
            'NORMAL0': 'Normal',
            'WORLDPOS': 'WorldPos',
            'WORLDPOS0': 'WorldPos',
            'SV_TARGET': 'Color',
            'SV_TARGET0': 'Color',
        }

    def load_all_cbuffers_from_combined_csv(self, csv_path: str):
        """Load all cbuffer data from a combined VS/PS cbuffer CSV file."""
        for cb_name in list(self.cbuffers.keys()):
            self.load_cbuffer_data_from_csv(cb_name, csv_path)

    def _execute_void_main(self, code: str, main_func: str, input_params: list,
                            output_params: list, input_data: dict, row_index: int) -> dict:
        """
        Execute a void main(...) style HLSL function.
        Returns dict {output_param_name: value}.
        """
        local_vars = {}
        # Set input params directly by name
        for param in input_params:
            pname = param['name']
            local_vars[pname] = input_data.get(pname, self._default_value_for_type(param['type']))

        # Initialize output params
        for param in output_params:
            pname = param['name']
            local_vars[pname] = self._default_value_for_type(param['type'])

        self._eval_counter += 1
        self._should_print = ((self._eval_counter - 1) % self.print_sequence == 0)

        self.debug_print(f"\n=== ROW {row_index} (void main) ===")

        # Get cached statements for this function
        cache_key = f'void_{main_func}_{id(code)}'
        if cache_key in self._parsed_func_cache:
            statements = list(self._parsed_func_cache[cache_key]['statements'])
        else:
            statements = self._collect_function_statements(main_func)
            self._parsed_func_cache[cache_key] = {'statements': statements, 'body': ''}

        # Execute statements
        i = 0
        while i < len(statements):
            stmt = statements[i]
            if stmt is None:
                i += 1
                continue

            stmt_stripped = stmt.strip()
            if stmt_stripped == 'return' or stmt_stripped.startswith('return;'):
                break

            if stmt_stripped.startswith('if'):
                next_i = i + 1
                while next_i < len(statements) and statements[next_i] is None:
                    next_i += 1
                if (next_i < len(statements) and statements[next_i] and
                        statements[next_i].strip().startswith('else')):
                    full_stmt = stmt + '\n' + statements[next_i]
                    self.execute_if_statement(full_stmt, local_vars)
                    statements[next_i] = None
                else:
                    self.execute_if_statement(stmt_stripped, local_vars)
            else:
                self.execute_statement(stmt_stripped, local_vars)
            i += 1

        # Collect output param values
        result = {}
        for param in output_params:
            result[param['name']] = local_vars.get(param['name'])
        return result

    def _resolve_slot_shared_params(self, output_params: list, result_params: dict):
        """
        Handle output params sharing the same output slot (e.g., TEXCOORD0 and TEXCOORD1).
        If a secondary param was never assigned, derive its value from the primary param's extra components.
        """
        slot_groups: Dict[int, list] = {}
        for param in output_params:
            slot = param.get('slot', -1)
            if slot < 0:
                continue
            if slot not in slot_groups:
                slot_groups[slot] = []
            slot_groups[slot].append(param)

        for slot, params in slot_groups.items():
            if len(params) <= 1:
                continue
            params_sorted = sorted(params, key=lambda p: p.get('semantic_index', 0))
            first = params_sorted[0]
            first_val = result_params.get(first['name'], [])
            first_comp = self._type_component_count(first['type'])

            if isinstance(first_val, list) and len(first_val) > first_comp:
                offset = first_comp
                for param in params_sorted[1:]:
                    cur = result_params.get(param['name'], [])
                    comp = self._type_component_count(param['type'])
                    is_default = not cur or all(
                        v in (0, 0.0, False) for v in (cur if isinstance(cur, list) else [cur])
                    )
                    if is_default:
                        result_params[param['name']] = first_val[offset:offset + comp]
                    offset += comp

    def executeVS_with_params(self, main_func: str, input_params: list, output_params: list,
                                vertex_data: list, execute_count: int = None) -> list:
        """
        Execute vertex shader using parameter-based I/O (void main(...) style).
        Returns list of dicts keyed by canonical names (sv_position, Color, TexCoord, etc.)
        """
        if execute_count is None or execute_count < 0:
            execute_count = len(vertex_data)
        execute_count = min(execute_count, len(vertex_data))

        self._eval_counter = 0
        self.vertex_pool.clear()
        sem_to_key = self._get_output_semantic_to_key_map()

        results = []
        for row_index in range(execute_count):
            row_data = vertex_data[row_index]
            result_params = self._execute_void_main(
                self.hlsl_code, main_func, input_params, output_params, row_data, row_index
            )

            # Handle slot-shared params (e.g., TEXCOORD0/TEXCOORD1 sharing slot 2)
            self._resolve_slot_shared_params(output_params, result_params)

            # Map to canonical keys
            canonical = {}
            for param in output_params:
                sem_base = param['semantic_base'].upper()
                sem_idx = param['semantic_index']
                sem_full = f'{sem_base}{sem_idx}' if sem_idx > 0 else sem_base
                key = sem_to_key.get(sem_full, sem_to_key.get(sem_base, param['semantic_base']))

                value = result_params.get(param['name'])
                comp_count = self._type_component_count(param['type'])
                if isinstance(value, list) and len(value) > comp_count:
                    value = value[:comp_count]
                canonical[key] = value

            results.append(canonical)

        return results

    def executePS_with_params(self, main_func: str, ps_input_params: list,
                                ps_output_params: list, pixels: list, ps_code: str = None) -> list:
        """Execute pixel shader using parameter-based I/O."""
        self._eval_counter = 0
        code = ps_code or self.hlsl_code

        # Mapping from PS input semantic to pixel attribute name
        sem_to_pixel = {
            'SV_POSITION': 'sv_pos',
            'COLOR': 'color', 'COLOR0': 'color',
            'TEXCOORD': 'texcoord', 'TEXCOORD0': 'texcoord',
            'TEXCOORD1': 'texcoord2',
            'NORMAL': 'normal', 'NORMAL0': 'normal',
            'WORLDPOS': 'worldPos', 'WORLDPOS0': 'worldPos',
        }

        for pixel in pixels:
            pixel.ps_output_color = None
            input_data = {}

            for param in ps_input_params:
                sem_base = param['semantic_base'].upper()
                sem_idx = param['semantic_index']
                sem_full = f'{sem_base}{sem_idx}' if sem_idx > 0 else sem_base
                attr_name = sem_to_pixel.get(sem_full, sem_to_pixel.get(sem_base, ''))

                if attr_name == 'sv_pos':
                    pixel_val = [float(pixel.x), float(pixel.y), float(pixel.depth), 1.0]
                elif attr_name:
                    pixel_val = getattr(pixel, attr_name, None)
                    if pixel_val is None:
                        pixel_val = self._default_value_for_type(param['type'])
                else:
                    pixel_val = self._default_value_for_type(param['type'])

                # Trim/pad to declared type component count
                comp = self._type_component_count(param['type'])
                if isinstance(pixel_val, list):
                    pixel_val = (pixel_val + [0.0] * comp)[:comp]
                input_data[param['name']] = pixel_val

            result_params = self._execute_void_main(
                code, main_func, ps_input_params, ps_output_params, input_data, 0
            )

            # SV_TARGET0 is the output color
            for param in ps_output_params:
                if 'SV_TARGET' in param['semantic_base'].upper():
                    pixel.ps_output_color = result_params.get(param['name'])
                    break
            if pixel.ps_output_color is None and result_params:
                pixel.ps_output_color = next(iter(result_params.values()))

        return pixels

    def compare_vs_output_with_golden_params(self, results: list, output_params: list,
                                            golden_rows: list, float_tolerance: float = 0.0001,
                                            execute_count: int = None) -> bool:
        """Compare VS output results against golden data (both using canonical key format)."""
        count = execute_count if execute_count and execute_count > 0 else len(results)
        count = min(count, len(results), len(golden_rows))

        passed = 0
        all_match = True

        for row_idx in range(count):
            result_row = results[row_idx]
            golden_row = golden_rows[row_idx]
            row_match = True

            for key, golden_val in golden_row.items():
                output_val = result_row.get(key)
                if output_val is None or golden_val is None:
                    continue

                if isinstance(output_val, list) and isinstance(golden_val, list):
                    min_len = min(len(output_val), len(golden_val))
                    for comp_idx in range(min_len):
                        ov = output_val[comp_idx]
                        gv = golden_val[comp_idx]
                        if gv is None:
                            continue
                        if isinstance(ov, float) and isinstance(gv, float):
                            if abs(ov - gv) > float_tolerance:
                                self.log_output(
                                    f"Error: Row {row_idx} {key}[{comp_idx}]: "
                                    f"output={ov:.6f} golden={gv:.6f} diff={abs(ov-gv):.6f}"
                                )
                                row_match = False
                        elif ov != gv:
                            self.log_output(f"Error: Row {row_idx} {key}[{comp_idx}]: output={ov} golden={gv}")
                            row_match = False
                elif isinstance(output_val, (int, float)) and isinstance(golden_val, (int, float)):
                    if abs(float(output_val) - float(golden_val)) > float_tolerance:
                        self.log_output(f"Error: Row {row_idx} {key}: output={output_val:.6f} golden={golden_val:.6f}")
                        row_match = False

            if row_match:
                passed += 1
            else:
                all_match = False

        self.log_output(f"Total PASSED rows: {passed}/{count}")
        if all_match:
            self.log_output("Comparison PASSED: All output data matches golden data within tolerance")
        else:
            self.log_output("Comparison FAILED: Some output data does not match golden data")
        return all_match

