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

    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));
    float3 lightDistant = LightPos.xyz - worldPos.xyz;
    float dist = length(lightDistant);
    if (dist <= LightRadius)
        cond = 1.0;
    else
        cond = 0.0;

    output.TexCoord = input.TexCoord;
    output.TexCoord2 = input.TexCoord;
    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * cond, 1.0);
    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));
    output.WorldPos = worldPos.xyz;

    float3 nor = normalize(input.Normal);
    float3 normal = normalize(mul(nor, (float3x3)World));
    output.Normal = normal;

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
    float3 specular = RdotV > 0.0 ? matSpecular.rgb * SpecularColor.rgb * pow(RdotV, Shininess) : float3(0.0, 0.0, 0.0);
    float3 ambient = matAmbient.rgb * AmbientColor.rgb;
    float3 emissive = matEmissive.rgb;
    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);
    float cond = -1.0;

    return output;
}