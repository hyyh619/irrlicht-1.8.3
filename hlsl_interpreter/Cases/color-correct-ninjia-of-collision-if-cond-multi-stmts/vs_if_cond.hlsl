struct VS_INPUT {
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
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
    float padding0;
    float3 Attenuation;
    float padding1;
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
    if (LightRadius < 600.0)
    {
        output.Color = float4(0.8, 0.0, 0.0, 1.0);
    }
    else
    {
        output.Color = float4(0.0, 0.0, 0.0, 1.0);
        output.Color.r = 0.8;
        output.Color.g = input.Color.b;
    }
    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));
    output.WorldPos = output.Pos.xyz;
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    output.TexCoord2 = input.TexCoord;
    return output;
}