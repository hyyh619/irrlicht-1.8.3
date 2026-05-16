# 1
Git commit: Create hlsl interpreter code by MiniMax-M2.7.
1. 我需要写一个python程序，该程序能够解释执行一段HLSL代码，样例HLSL代码如下，请先帮我实现对下列样例代码的解释执行实现
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    float3 nor = normalize(input.Normal);"
            "    float3 normal = normalize(mul(nor, (float3x3)World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Normal = normal;"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord;"
            "    float3 lightDistant = LightPos.xyz - worldPos.xyz;"
            "    float dist = length(lightDistant);"
            "    float3 lightDir = normalize(lightDistant);"
            "    float3 viewDir = cameraPos;"
            "    float NdotL = max(dot(normal, lightDir), 0.0);"
            "    float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;"
            "    float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;"
            "    float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;"
            "    float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;"
            "    float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;"
            "    float3 R = reflect(lightDir, normal);"
            "    float RdotV = max(dot(R, viewDir), 0.0);"
            "    float3 specular = matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
            "    float3 ambient = matAmbient.rgb * AmbientColor.rgb;"
            "    float3 emissive = matEmissive.rgb;"
            "    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);"
            "    float cond = dist <= LightRadius ? 1.0 : 0.0;"
            "    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * cond, 1.0);"
            "    return output;"
            "}";

2. 这段代码会使用到以下的数据，我们使用json文件的方式来输入这些数据。
        const char    VERTEX_SHADER_STANDARD_POINT[] =
            "struct VS_INPUT {"
            "    float3 Pos : POSITION;"
            "    float3 Normal : NORMAL;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "};"
            "struct VS_OUTPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float2 TexCoord2 : TEXCOORD1;"
            "    float3 Normal : NORMAL;"
            "    float3 WorldPos : WORLDPOS;"
            "};"
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "    float4x4 World;"
            "};"
            "cbuffer LightBuffer : register(b1) {"
            "    float4 AmbientColor;"
            "    float4 DiffuseColor;"
            "    float4 SpecularColor;"
            "    float3 LightPos;"
            "    float LightRadius;"
            "    float3 LightDir;"
            "    float3 Attenuation;"
            "    float OuterCone;"
            "    float InnerCone;"
            "};"
            "cbuffer MaterialBuffer : register(b2) {"
            "    float4 MaterialDiffuseColor;"
            "    float4 MaterialAmbientColor;"
            "    float4 MaterialSpecularColor;"
            "    float4 MaterialEmissiveColor;"
            "    float Shininess;"
            "    uint ColorMaterialMode;"
            "    float2 Padding;"
            "};"
            "cbuffer CameraBuffer : register(b3) {"
            "    float3 cameraPos;"
            "};"


# 2
Git commit: hlsl-inter: struct parser refine by MiniMax-M2.7.
下列解析struct的函数，对于float3 Pos : POSITION; float3是数据类型名，Pos是HLSL里面用到的变量名，POSITION是语义名，用于与VS input layout对应，这三个名字都需要保存。因此重新构造一下StructDefinition
    struct VS_INPUT {
        float3 Pos : POSITION;
        float3 Normal : NORMAL;
        float4 Color : COLOR;
        float2 TexCoord : TEXCOORD0;
    };
    def parse_struct(self, code: str) -> StructDefinition:
        match = re.search(r'struct\s+(\w+)\s*\{([^}]+)\}', code)
        if not match:
            return None
        name = match.group(1)
        fields_str = match.group(2)
        fields = {}
        for line in fields_str.split(';'):
            line = line.strip()
            if not line:
                continue
            parts = line.split(':')
            if len(parts) == 2:
                field_name = parts[0].strip().split()[-1]
                field_type = parts[1].strip()
                fields[field_name] = field_type
        return StructDefinition(name, fields)

# 3
Git commit: Fix cbuffer/struct paser by MiniMax-M2.7.
['float4x4', 'float4', 'float3', 'float2', 'uint']数据类型列表单独作为全局变量定义，方便其它函数使用。
    def parse_cbuffer(self, code: str) -> tuple:
        match = re.search(r'cbuffer\s+(\w+)\s*:.*?\{([^}]+)\}', code, re.DOTALL)
        if not match:
            return None, None
        name = match.group(1)
        members = {}
        lines = code[match.start():match.end()].split('\n')[1:]
        current_type = None
        for line in lines:
            line = line.strip().rstrip(';')
            if not line or line.startswith('}'):
                continue
            if any(t in line for t in ['float4x4', 'float4', 'float3', 'float2', 'uint']):
                parts = line.split()
                if len(parts) >= 2:
                    type_str = parts[0]
                    var_name = parts[1]
                    members[var_name] = type_str
        return name, members

self.cbuffers[cb_name]没有保存解析出来的cb_members
        cbuffer_pattern = r'cbuffer\s+\w+[^}]+\}'
        for cb_match in re.finditer(cbuffer_pattern, code, re.DOTALL):
            cb_name, cb_members = self.parse_cbuffer(cb_match.group())
            if cb_name:
                self.cbuffers[cb_name] = {}

# 4
Git commit: hlsl-inter: load data from .csv to cbuffer and vertex input by MiniMax-M2.7.
1. StructDefinition的list保存了解析出来的HLSL struct的每个成员。这些成员被存在FieldDefinition中，请为FieldDefinition创建一个保存数据的成员，该成员根据下列数据结构来保存实际的数据
   DATA_TYPE_LIST = ['float4x4', 'float4', 'float3', 'float2', 'uint']
2. 像StructDefinition一样，为cbuffer也定义一个类似的数据结构CbufferDefinition，记录变量名，变量类型，以及用于存储变量对应的数据的成员
3. 根据self.cbuffers和self.struct的字典名字来查找对应的csv数据文件，然后给相应的成员变量初始化数据
4. StructDefinition表示的是顶点或者像素，因此一个成员变量会对应多组数据，CbufferDefinition对应的常量，一个成员只对应一组数据

# 5
Git commit: cbuffer parser should load both vector and matrix by MiniMax-M2.7.
cbuffer对应的csv文件如果data type是float4x4,其数据格式如下
WorldViewProj,,0,float4x4 (column_major)
WorldViewProj.row0,"1.03104, 0.00, -0.05065, 24.85304",,float4
WorldViewProj.row1,"0.00476, 1.37295, 0.09699, -98.08849",,float4
WorldViewProj.row2,"0.04896, -0.07058, 0.99664, 125.7131",,float4
WorldViewProj.row3,"0.04895, -0.07055, 0.99631, 126.6712",,float4
但是load_cbuffer_data_from_csv只能处理非矩阵类型数据
AmbientColor,"0.00, 0.00, 0.00, 0.00",0,float4
DiffuseColor,"1.00, 1.00, 1.00, 1.00",16,float4
SpecularColor,"1.00, 1.00, 1.00, 1.00",32,float4
请加入矩阵类型数据处理


# 6
Git commit: 
load_cbuffer_data_from_csv直接储存的value string，请根据csv每行数据最后的data type把字符串转换成对应的数据类型。
每行数据的样例如下：
WorldViewProj.row0,"1.03104, 0.00, -0.05065, 24.85304",,float4
LightRadius,"600.00",60,float
ColorMaterialMode,"1",68,uint
load_cbuffer_data_from_csv保存数据的代码如下，请按前面的要求修改。
                    matrix_rows[base_name][row_idx] = value_str
            else:
                scalar_vars[var_name] = value_str

load_cbuffer_data_from_csv打印data的代码如下，可以看到它直接打印数据，没有做打印的格式化
请按照数据类型分行，
1. 'float4x4', 'float3x3', 这种矩阵类型的数据，matrix row就打印一行，上下行的每列数据都要对齐
2. 'float4', 'float3', 'float2', 这种向量类型的数据，就直接打印一行
3. 'float', 'uint'，这种单数据类型打印一行
        for cb_n, cb_d in self.cbuffers.items():
            print(f"Cbuffer {cb_n}:")
            for f in cb_d.fields:
                print(f"  {f.name} ({f.field_type}): data={f.data}")

# 7
Git commit: hlsl-inter: print struct data by MiniMax-M2.7.
load_struct_data_from_csv在加载完strut数据后，请打印每个field的第一组数据

# 8
Git commit: Change HLSL cbuffer/struct data loading by MiniMax-M2.7.
之前cbuffer/struct加载的数据是从test_data.json加载
现在通过load_cbuffer_data_from_csv/load_struct_data_from_csv被存在HLSLInterpreter的cbuffers和structs中
1. 因此下列代码不需要再加载data
    json_path = os.path.join(script_dir, 'test_data.json')
    data = interpreter.load_json(json_path)
    result = interpreter.interpret(code, data)
2. execute_function也不需要data输入，而是直接查找cbuffers/structs中的对象来获取data
3. HLSLInterpreter.interpret中的execute_function需要根据当前VS_INPUT struct有多少组数据，循环执行每组数据

# 9 several loops to create some debug_test*.py.
Git commit: Create executeVS/executePS to execute the VS/PS code by MiniMax-M2.7.
1. HLSLInterpreter提供两个函数executeVS和executePS分别解释执行VS HLSL和PS HLSL
2. executeVS()三个输入参数
   a. code: HLSL源代码
   b. main_func: VS的main函数名字，VS解释执行从main函数开始
   c. vs_input: VS的顶点输入数据结构名称，main函数的输入从这里获取数据
3. executePS()三个输入参数
   a. code: HLSL源代码
   b. main_func: PS的main函数名字，PS解释执行从main函数开始
   c. ps_input: PS的像素输入数据结构名称，main函数的输入从这里获取数据
4. interpret(self, code: str)只负责解析源代码中的cbuffer/struct，以及加载数据
5. 调用executeVS来执行HLSL
6. executeVS需要对vs_input的每一组数据执行
7. executePS目前暂时不要调用执行，后续拓展解释器功能再调用

# 10
Git commit: hlsl-inter: refine execute_function by MiniMax-M2.7.
重构execute_function
def execute_function(self, code: str, main_func: str, input_struct_name: str, row_index: int)
1. execute_function作为普通执行任何VS/PS main函数，改名为execute_main_function
2. execute_main_function不需要在内部自己解析struct来获取data,由executeVS和executePS获得执行的数据，然后把每次执行的数据传递给execute_main_function

# 11
Git commit: hlsl-inter: add missed data type by MiniMax-M2.7.
DATA_TYPE_LIST = ['float4x4', 'float3x3',
                'float4', 'float3', 'float2', 'float',
                'uint4', 'uint3', 'uint2', 'uint',
                'int4', 'int3', 'int2', 'int',
                'bool']
1. get_type_size函数根据DATA_TYPE_LIST提供的type来返回size
2. parse_type, parse_value_by_type函数根据DATA_TYPE_LIST补全缺乏的type
3. execute_statement函数下列判断请使用DATA_TYPE_LIST
           if stmt.startswith('float4 ') or stmt.startswith('float3 ') or stmt.startswith('float ') or stmt.startswith('float2 ') or stmt.startswith('int ') or stmt.startswith('uint ') or stmt.startswith('bool '):
            match = re.match(r'(?:float4|float3|float2|float|int|uint|bool)\s+(\w+)\s*=\s*(.+?);?$', stmt)
4. load_cbuffer_data_from_csv打印cbuffer的data，也按照DATA_TYPE_LIST补齐缺乏的数据类型打印


# 12
Git commit: hlsl-inter: add eval print for debugging by MiniMax-M2.7.
1. evaluate_expression的每个执行分支都加一个打印输出其执行的操作和操作数，这个打印可以通过开关控制。
2. execute_statement执行的每一条语句都打印出来，包括语句变量的输入数据和输出数据


# 13
Git commit: 
告诉我下面这段代码的作用，给这段代码添加注释，尤其是if re.match(r'float[234]\s*\(', expr)这个语句匹配的是什么字符串
        if re.match(r'float[234]\s*\(', expr):
            self.debug_print(f"[EVAL] FLOAT234: {expr}")
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
                self.debug_print(f"[EVAL] FLOAT234 result: {result}")
                return result


            if 'transpose' in expr:
                self.debug_print(f"[EVAL] TRANSPOSE: {expr}")
                match = re.search(r'transpose\s*\(([^)]+)\)', expr)
                if match:
                    val = self.get_value(match.group(1), local_vars)
                    if val is None:
                        return None
                    result = self.transpose_matrix(val)
                    self.debug_print(f"[EVAL] TRANSPOSE result: {result}")
                    return result

            if 'normalize' in expr:
                self.debug_print(f"[EVAL] NORMALIZE: {expr}")
                match = re.search(r'normalize\s*\(([^)]+)\)', expr)
                if match:
                    val = self.get_value(match.group(1), local_vars)
                    if val is None:
                        return None
                    if isinstance(val, list):
                        result = self.normalize_vec(val)
                        self.debug_print(f"[EVAL] NORMALIZE result: {result}")
                        return result
                    return val

            if 'length' in expr:
                self.debug_print(f"[EVAL] LENGTH: {expr}")
                match = re.search(r'length\s*\(([^)]+)\)', expr)
                if match:
                    val = self.get_value(match.group(1), local_vars)
                    if val is None:
                        return None
                    result = self.length_vec(val)
                    self.debug_print(f"[EVAL] LENGTH result: {result}")
                    return result

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
                        return None
                    result = self.dot_product(a, b)
                    self.debug_print(f"[EVAL] DOT result: {result}")
                    return result
                match = re.match(r'dot\s*\(([^,]+),\s*([^)]+)\)', expr)
                if match:
                    a = self.get_value(match.group(1), local_vars)
                    b = self.get_value(match.group(2), local_vars)
                    if a is None or b is None:
                        return None
                    result = self.dot_product(a, b)
                    self.debug_print(f"[EVAL] DOT result: {result}")
                    return result

            if 'reflect' in expr:
                self.debug_print(f"[EVAL] REFLECT: {expr}")
                match = re.match(r'reflect\s*\(([^,]+),\s*([^)]+)\)', expr)
                if match:
                    I = self.get_value(match.group(1), local_vars)
                    N = self.get_value(match.group(2), local_vars)
                    if I is None or N is None:
                        return None
                    result = self.reflect_vec(I, N)
                    self.debug_print(f"[EVAL] REFLECT result: {result}")
                    return result

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
                        return None
                    result = max(a, b)
                    self.debug_print(f"[EVAL] MAX result: {result}")
                    return result

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

            if 'pow' in expr:
                self.debug_print(f"[EVAL] POW: {expr}")
                match = re.match(r'pow\s*\(([^,]+),\s*([^)]+)\)', expr)
                if match:
                    base = self.evaluate_expression(match.group(1), local_vars)
                    exp = self.evaluate_expression(match.group(2), local_vars)
                    if base is None or exp is None:
                        return None
                    result = math.pow(base, exp)
                    self.debug_print(f"[EVAL] POW result: {result}")
                    return result

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

# 14
Git commit: hlsl-inter: add warning log for val none by MiniMax-M2.7.
为evaluate_expression执行分支里的每处判断val为None时，增加一个警告打印
                if val is None:
                    return None


# 15
Git commit: hlsl-inter: create syntax tree for expr evalution by MiniMax-M2.7.
execute_statement函数执行下面语句时，只执行了transpose(WorldViewProj), 并未执行float4(),mul()
'output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj))'
1. 这个问题应该是因为当前解释器在执行一条语句时采用的简单匹配，因此先匹配到transpose，执行完毕后就直接赋值给了output.pos
2. 对于一个HLSL，我们应该根据其执行的操作符的优先级构造语法树，然后按照语法树节点来挨个执行这条语句的所有操作
3. 请加入语法树模块，负责分析一条语句构造语法树，然后基于该语法树执行所有操作

打印下面生成的syntax tree
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
                return self.evaluate_syntax_tree(tree, local_vars)

下列生成的打印'Function:'根据在tree的几级子节点加上缩进
Function: mul
  arg[0]:
Function: float4
      arg[0]:
        Value: input.Pos
      arg[1]:
        Value: 1.0
  arg[1]:
Function: transpose
      arg[0]:
        Value: WorldViewProj

# 16
Git commit: hlsl-inter: add execution print for evaluate_syntax_tree by MiniMax-M2.7
为下列函数执行具体某个操作符时，打印其操作数，操作符，以及结果
    def execute_binary_op(self, op: str, left: Any, right: Any) -> Any:
    def execute_unary_op(self, op: str, val: Any) -> Any:
    def execute_function_node(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:

# 17
Git commit: hlsl-inter: fix vector multiplies matrix. We should use col major by MiniMax-M2.7.
1. 前面为下列函数添加的打印也请用一个bool变量来控制是否打印
    def execute_binary_op(self, op: str, left: Any, right: Any) -> Any:
    def execute_unary_op(self, op: str, val: Any) -> Any:
    def execute_function_node(self, node: SyntaxTreeNode, local_vars: Dict[str, Any]) -> Any:
2. evaluate_expression打印生成的syntax tree也用一个bool变量控制是否打印
3. 函数def mul_matrix_vector(self, m: List[List[float]], v: List[float]) -> List[float]:的实现是vector乘以矩阵的行。而我们这个解释器里向量都是行向量，右乘矩阵，需要乘以矩阵的列，不是行。请修改
4. 请检查其他地方的矩阵和向量的乘法，是否都是行向量，乘以矩阵的列

# 18
Git commit: hlsl-inter: format matrix print by MiniMax-M2.7.
下面函数如果打印的操作数和结果是float4x4/float3x3这种矩阵数据，那么matrix row就打印一行，上下行的每列数据都要对齐
def execute_function_node
def execute_unary_op
def execute_binary_op


# 19
Git commit: hlsl-inter: add float3x3 cast to syntax tree by MiniMax-M2.7.
1. float3 normal = normalize(mul(nor, (float3x3)World));
上面这行HLSL代码中(float3x3)World的意思是从4x4 World矩阵中取前3行x前3列的3x3矩阵出来。
self.syntax_parser.parse(expr)生成的语法树里没有看到float3x3这个操作，请增加float3x3相应处理

Git commit: hlsl-inter: add comments to SyntaxTreeParser._parse_expression by MiniMax-M2.7.
2. 给class SyntaxTreeParser的_parse_expression内的代码添加注释


# 20
Git commit: hlsl-inter: add comments for the whole code by MiniMax-M2.7.
1. 给hlsl_interpreter.py的所有class, struct, function和function内部的关键代码增加注释
2. 已经有注释的就不用添加

# 21
Git commit: hlsl-inter: add float4x4->float2x2, float3x3->float2x2 cast by MiniMax-M2.7.
evaluate_syntax_tree处理cast的分支如下
        elif node.node_type == 'cast':
            inner = self.evaluate_syntax_tree(node.left, local_vars)
            if inner is None:
                return None
            cast_type = node.value
            # float3x3转换: 从4x4矩阵提取前3x3
            if cast_type == 'float3x3' and isinstance(inner, list) and len(inner) == 4:
                return [row[:3] for row in inner[:3]]
增加下列cast
1. float4x4->float2x2, float3x3->float2x2


# 22
Git commit: hlsl-inter: add comments for each branch of execute_function_node by MiniMax-M2.7.
给函数execute_function_node的每一个分支加上注释


# 23
Git commit: hlsl-inter: add golden data and compare between output and golden data by MiniMax-M2.7.
1. struct VS_OUTPUT的golden数据通过csv文件提供了。请加载该数据
2. 增加一个最后结果比对的函数，该函数功能如下
   a. 每一组VS_INPUT数据通过解析执行完HLSL后会得到一组OUTPUT数据
   b. 执行HLSL得到的OUTPUT数据与VS_OUTPUT的golden csv文件的对应组数据进行比对
   c. 如果是浮点类型数据比对，那么允许有一定的误差，这个误差值可以调整。如果OUTPUT的field的数据和golden数据之间的差值超过误差，则打印error
   d. 如果是其它数据类型比对，则要求严格相等，如果不等，则打印error


# 24
Git commit: hlsl-inter: refine output golden check code by MiniMax-M2.7.
1. compare_vs_output_with_golden增加一个输入参数，告知compare_vs_output_with_golden使用的OUTPUT struct是那一个，不要直接默认使用"VS_OUTPUT"
2. 不要通过函数内部自定义的semantic_to_field来获得field name，直接通过获得的vs_output_def来获取field name


# 25
Git commit: hlsl-inter: add error log for arg checking by MiniMax-M2.7.
给evaluate_syntax_tree所有判断len(args)的地方，如果不满足操作要求，返回None之前都加上一个error打印
            if len(args) != 2:
                return None
就加上一个打印,这个打印需要包括当前的行号和处理的操作是什么
            if len(args) != 2:
                self.debug_print()
                return None


# 26
Git commit: hlsl-inter: fix wrong arg number of max/dot function by ying.
SyntaxTreeParser._parse_expression没有看到对max/min/dot float NdotL = max(dot(normal, lightDir), 0.0)
[STMT] Executing: float NdotL = max(dot(normal, lightDir), 0.0)
[SYNTAX TREE]
Function(max)
  arg[0]:
Function(dot)
      arg[0]:
        Value(normal, lightDir)

# 27
Git commit: hlsl-inter: make all go to syntax tree and executed by MiniMax-M2.7.
hlsl_interpreter.py在解释执行下列语句时，没有使用语法树处理，导致不能得到正确的结果。请通过SyntaxTreeParser来处理和evaluate_syntax_tree执行
        float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;
        float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;
        float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;
        float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;
        float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;


# 28
Git commit: 
从hlsl_interpreter.py执行以下的语句来看，目前构造语法树的时候，没有考虑运算符的优先级。例如'+'的优先级低于'*'。而从下面输出的语法树来看，先执行的是加法，然后才执行乘法。
例如：
先执行了Attenuation.x + Attenuation.y，然后再乘以dist。实际应该是先执行Attenuation.y * dist，然后再加上Attenuation.x。请修复运算符优先级问题
[STMT] Executing: float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist)
[SYNTAX TREE]
BinaryOp(/)
  left:
    Value(1.0)
  right:
BinaryOp(*)
      left:
BinaryOp(*)
          left:
BinaryOp(+)
              left:
BinaryOp(*)
                  left:
BinaryOp(+)
                      left:
                        Value(Attenuation.x)
                      right:
                        Value(Attenuation.y)
                  right:
                    Value(dist)
              right:
                Value(Attenuation.z)
      right:
        Value(dist)
[BINARY OP] left=['0.0017', '0.0000', '45.0000'], right=['0.0017', '0.0000', '45.0000'], op=+, result=['0.0033', '0.0000', '90.0000']
[BINARY OP] left=['0.0033', '0.0000', '90.0000'], right=498.6748, op=*, result=['1.6656', '0.0000', '44880.7331']
[BINARY OP] left=['1.6656', '0.0000', '44880.7331'], right=['0.0017', '0.0000', '45.0000'], op=+, result=['1.6672', '0.0000', '44925.7331']   
[BINARY OP] left=['1.6672', '0.0000', '44925.7331'], right=498.6748, op=*, result=['831.4125', '0.0000', '22403331.5206']
[BINARY OP] left=['831.4125', '0.0000', '22403331.5206'], right=498.6748, op=*, result=['414604.4850', '0.0000', '11171977139.9078']


# 29
Git commit: 
从hlsl_interpreter.py执行以下的语句来看，目前构造语法树的时候，已经考虑运算符的优先级。但是缺乏'.'运算符导致，
1. Attenuation是一个float3，Attenuation.x表示使用float3向量的第一个成员值。
2. 对于float4, xyzw分别表示其向量里的第一个元素，第二个元素，第三个元素，第四个元素。float3,float2以此类推
3. 对于float4, rgba分别表示其向量里的第一个元素，第二个元素，第三个元素，第四个元素。float3,float2以此类推
4. 请增加一个运算符'.'表示获取某个向量里面的元素。该运算符的优先级最高。
5. 通过增加该运算符，Attenuation.x， Attenuation.y，Attenuation.z分别获取float3向量的第一，第二，第三个元素。
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
[BINARY OP] left=['0.0017', '0.0000', '45.0000'], right=498.6748, op=*, result=['0.8328', '0.0000', '22440.3666']
[BINARY OP] left=['0.0017', '0.0000', '45.0000'], right=['0.8328', '0.0000', '22440.3666'], op=+, result=['0.8345', '0.0000', '22485.3666']
[BINARY OP] left=['0.0017', '0.0000', '45.0000'], right=498.6748, op=*, result=['0.8328', '0.0000', '22440.3666']
[BINARY OP] left=['0.8328', '0.0000', '22440.3666'], right=498.6748, op=*, result=['415.2899', '0.0000', '11190445.5770']       
[BINARY OP] left=['0.8345', '0.0000', '22485.3666'], right=['415.2899', '0.0000', '11190445.5770'], op=+, result=['416.1243', '0.0000', '11212930.9436']

1. 新增的'.'操作符获取xyzw/rgba只对float4,float3,float2,int4,int3,int2,uint4,uint3,uint2等类型变量有效。
2. 对于struct结构数据，例如ouptut/input来说，'.'是获取struct内部的某个变量例如
   output.Pos就是获取VS_OUTPUT的Pos成员变量
            struct VS_OUTPUT {
                float4 Pos : SV_POSITION;
                float4 Color : COLOR;
                float2 TexCoord : TEXCOORD0;
                float2 TexCoord2 : TEXCOORD1;
                float3 Normal : NORMAL;
                float3 WorldPos : WORLDPOS;
            };
3. 请在解析'.'操作符时根据上面的描述分成两类操作，一类是获取向量的某个分量，一类是获取struct的某个成员变量。

1. 从最新的语法树打印来看，处理下面这个语句时，把浮点数1.0的'.'也当成了操作符来处理。请修复这个问题
[STMT] Executing: output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj))
[SYNTAX TREE]
Function(mul)
  arg[0]:
Function(float4)
      arg[0]:
BinaryOp(.)
          left:
            Value(input)
          right:
            Value(Pos)
      arg[1]:
BinaryOp(.)
          left:
            Value(1)
          right:
            Value(0)
  arg[1]:
Function(transpose)
      arg[0]:
        Value(WorldViewProj)

从hlsl_interpreter.py执行SyntaxTreeParser的parse为下面的HLSL语句构造语法树
output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj))
output.Pos是对于struct结构数据output获得其Pos，就是获取VS_OUTPUT的Pos成员变量
            struct VS_OUTPUT {
                float4 Pos : SV_POSITION;
                float4 Color : COLOR;
                float2 TexCoord : TEXCOORD0;
                float2 TexCoord2 : TEXCOORD1;
                float3 Normal : NORMAL;
                float3 WorldPos : WORLDPOS;
            };
因此不要把这类'.'当成操作符，而是应该把output.Pos作为一个整体通过get_value来获取其数据

# 30
Git commit: hlsl-inter: add print controller to make syntax tree log and evaluate_syntax_tree log controlled by MiniMax-M2.7.
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


# 31
Git commit: hlsl-inter: optimize log file by MiniMax-M2.7.
在class HLSLInterpreter中每次debug打印如果要写出到文件就调用log_output，但是log_output每次写一条消息到文件中都需要打开文件写入再关闭。
    def log_output(self, *args, **kwargs):
        """输出到stdout和日志文件"""
        msg = ' '.join(str(arg) for arg in args)
        print(*args, **kwargs)
        if self.log_to_file and self.log_file_path:
            with open(self.log_file_path, 'a', encoding='utf-8') as f:
                f.write(msg + '\n')

    def debug_print(self, msg: str):
        """调试打印"""
        if self.debug and self._should_print:
            self.log_output(msg)
1. 请不要在log_output中每次都打开文件写入再关闭，改成在HLSLInterpreter初始化时直接打开文件，在HLSLInterpreter对象销毁时关闭
2. 创建一个控制变量，来决定HLSLInterpreter log文件是否用覆盖写还是添加写。


# 32
Git commit: hlsl-inter: add execution time print by MiniMax-M2.7.
给HLSLInterpreter添加一个计时器，统计其执行时间。
1. 统计interpreter.interpret(code)执行时间
2. 统计interpreter.load_vs_output_golden_from_csv(golden_csv_path)执行时间
3. 统计results = interpreter.executeVS(code, "main", "VS_INPUT")执行时间
4. 统计最后的结果比对执行时间
5. 计算执行总时间
6. 把上述时间打印出来


# 33
Git commit: hlsl-inter: add json configure to load hlsl source code, cbuffer/input/output data and log path by MiniMax-M2.7.
class HLSLInterpreter目前是直接读取code字符串来解释执行HLSL，执行HLSL所需要的input,output,constant buffer等数据都是从hlsl_interpreter.py的执行目录，请做以下修改
1. 使用json文件输入当前要执行的HLSL文件的路径，要加载的csv文件所在的文件夹路径，输出的log文件路径
2. HLSLInterpreter不要把code字符串作为参数输入，采用输入HLSL的文件路径，读取文件来获得需要执行的HLSL
3. HLSLInterpreter不要默认读取当前目录下的csv，把csv所在的路径通过参数输入，根据输入路径读取参数


# 34
Git commit: hlsl-inter: move log_file_mode and print_sequence to config file by MiniMax-M2.7.
hlsl_interpreter.py的HLSLInterpreter创建时使用参数决定log_file_mode和print_sequence，请把输入参数改成与hlsl源码文件，csv数据文件路径一样，加入到json文件中，从json文件中读取log_file_mode和print_sequence参数
    interpreter = HLSLInterpreter(log_to_file=True, log_file_path=log_file_path, log_file_mode='w', print_sequence=100)


# 35
Git commit: hlsl-inter: fix wrong function body generation by ying.
MiniMax-M2.7 cannot find the root cause. 实际问题是body没有正常去除大括弧，导致无法识别语句

./hlsl_interpreter/hlsl_interpreter.py的execute_main_function函数在读取了hlsl源文件加载成字符串后，通过下面代码切分成HLSL一条条语句。看起来下面的代码不能正确的切分语句。导致把加载HLSL源代码当成了一条语句执行导致执行失败。
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
例如加载./hlsl_interpreter/constant_buffer_attenuation_wrong/VERTEX_SHADER_STANDARD_POINT.hlsl后得到的body如下，看起来上述代码不能正确切分
“
'{\n    VS_OUTPUT output;\n    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));\n    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));\n    float3 nor = normalize(input.Normal);\n    float3 normal = normalize(mul(nor, (float3x3)World));\n    output.WorldPos = worldPos.xyz;\n    output.Normal = normal;\n    output.TexCoord = input.TexCoord;\n    output.TexCoord2 = input.TexCoord;\n    float3 lightDistant = LightPos.xyz - worldPos.xyz;\n    float dist = length(lightDistant);\n    float3 lightDir = normalize(lightDistant);\n    float3 viewDir = cameraPos;\n    float NdotL = max(dot(normal, lightDir), 0.0);\n    float4 matDiffuse = (ColorMaterialMode == 1 || ColorMaterialMode == 5) ? input.Color : MaterialDiffuseColor;\n    float4 matAmbient = (ColorMaterialMode == 2 || ColorMaterialMode == 5) ? input.Color : MaterialAmbientColor;\n    float4 matSpecular = (ColorMaterialMode == 3) ? input.Color : MaterialSpecularColor;\n    float4 matEmissive = (ColorMaterialMode == 4) ? input.Color : MaterialEmissiveColor;\n    float3 diffuse = matDiffuse.rgb * DiffuseColor.rgb * NdotL;\n    float3 R = reflect(lightDir, normal);\n    float RdotV = max(dot(R, viewDir), 0.0);\n    float3 specular = RdotV > 0.0 ? matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess) : float3(0.0, 0.0, 0.0);\n    float3 ambient = matAmbient.rgb * AmbientColor.rgb;\n    float3 emissive = matEmissive.rgb;\n    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);\n    float cond = dist <= LightRadius ? 1.0 : 0.0;\n    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * cond, 1.0);\n    return output;'
”


# 36
Git commit: 
为hlsl_interpreter.py的json配置文件增加控制log输出的配置项
1. 增加是否打印语法树的控制， self.printSyntaxTree改成用json configure配置
2. 增加是否输出到文件的控制， self.log_to_file改成用json configure配置
3. 增加是否打印输出HLSL Interpreter Result的控制


# 37
Git commit: 


# 38
Git commit: 


# 39
Git commit: 
