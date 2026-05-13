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
Git commit: 
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
Git commit: 


# 5
Git commit: 


# 6
Git commit: 


# 7
Git commit: 
