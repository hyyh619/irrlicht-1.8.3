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
Git commit: 
1. evaluate_expression的每个执行分支都加一个打印输出其执行的操作和操作数，这个打印可以通过开关控制。
2. execute_statement执行的每一条语句都打印出来，包括语句变量的输入数据和输出数据


# 13
Git commit: 
'mul(float4(input.Pos, 1.0), transpose(WorldViewProj))'


# 14
Git commit: 


# 15
Git commit: 


# 16
Git commit: 


# 17
Git commit: 


# 18
Git commit: 


# 19
Git commit: 
