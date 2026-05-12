// Auto-generated HLSL Pixel Shaders for Irrlicht Material Types
// Reference: CD3D9MaterialRenderer implementation

#ifndef __PS_MATERIAL_SHADERS_H__
#define __PS_MATERIAL_SHADERS_H__

//==============================================================================
// Common Structures and Samplers
//==============================================================================

struct PS_INPUT_BASIC
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : TEXCOORD1;
};

struct PS_INPUT_2TEX
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float3 Normal : NORMAL;
};

struct PS_INPUT_TANGENTS
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TEXCOORD2;
    float3 Binormal : TEXCOORD3;
};

Texture2D DiffuseTexture : register(t0);
Texture2D LightmapTexture : register(t1);
Texture2D DetailTexture : register(t1);
Texture2D NormalMap : register(t1);
Texture2D SphereMap : register(t0);

SamplerState LinearSampler : register(s0);

cbuffer MaterialBuffer : register(b2)
{
    float4 DiffuseColor;
    float4 AmbientColor;
    float4 SpecularColor;
    float4 EmissiveColor;
    float Shininess;
    float ColorMaterialMode;
    float2 Padding;
};

cbuffer LightBuffer : register(b3)
{
    float4 LightDiffuse;
    float4 LightSpecular;
    float4 LightAmbient;
    float3 LightDirection;
    float LightRadius;
    float LightAttenuation0;
    float LightAttenuation1;
    float LightAttenuation2;
    float3 Padding2;
};

#define ECM_NONE              0
#define ECM_DIFFUSE           1
#define ECM_AMBIENT           2
#define ECM_SPECULAR          3
#define ECM_EMISSIVE          4
#define ECM_DIFFUSE_AND_AMBIENT 5

//==============================================================================
// EMT_SOLID - Standard solid material, first texture * diffuse
// ColorOp: D3DTOP_MODULATE, Tex * Diffuse
// Based on D3D9 fixed-function lighting:
// Diffuse = Material.DiffuseColor × Light.DiffuseColor × max(N·L, 0)
// Specular = Material.SpecularColor × Light.SpecularColor × pow(max(R·V, 0), Material.Shininess)
// Ambient = Material.AmbientColor × Light.AmbientColor
// Emissive = Material.EmissiveColor
// D3DTA_DIFFUSE = Diffuse + Specular + Ambient + Emissive
// PixelColor = TextureColor(u,v) × D3DTA_DIFFUSE
//==============================================================================
float4 PS_SOLID(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);

    float3 nor = normalize(input.Normal);
    float3 lightDir = normalize(-LightDirection);

    float3 viewDir = normalize(float3(0.0, 0.0, 1.0));
    float3 reflectDir = reflect(-lightDir, nor);

    float nDotL = max(dot(nor, lightDir), 0.0);
    float rDotV = max(dot(reflectDir, viewDir), 0.0);

    float4 matDiffuse, matAmbient, matSpecular, matEmissive;

    if (ColorMaterialMode == ECM_DIFFUSE || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
        matDiffuse = input.Color;
    else
        matDiffuse = DiffuseColor;

    if (ColorMaterialMode == ECM_AMBIENT || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
        matAmbient = input.Color;
    else
        matAmbient = AmbientColor;

    if (ColorMaterialMode == ECM_SPECULAR)
        matSpecular = input.Color;
    else
        matSpecular = SpecularColor;

    if (ColorMaterialMode == ECM_EMISSIVE)
        matEmissive = input.Color;
    else
        matEmissive = EmissiveColor;

    float4 diffuse = matDiffuse * LightDiffuseColor * nDotL;
    float4 specular = matSpecular * LightSpecularColor * pow(rDotV, Shininess);
    float4 ambient = matAmbient * LightAmbientColor;
    float4 emissive = matEmissive;

    float4 lightingResult = diffuse + specular + ambient + emissive;

    float4 finalColor = texColor * lightingResult;

    return float4(finalColor.rgb * input.Color.a, texColor.a * input.Color.a);
}

//==============================================================================
// EMT_SOLID_2_LAYER - Two layers blended with vertex alpha
// Stage 0: D3DTA_TEXTURE
// Stage 1: D3DTOP_BLENDDIFFUSEALPHA, TexCoord1 = TexCoord0
//==============================================================================
float4 PS_SOLID_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 layer1 = DiffuseTexture.Sample(LinearSampler, input.TexCoord1); // Same texture, stage 1
    float alpha = input.Color.a;
    return lerp(layer0, layer1, alpha);
}

//==============================================================================
// EMT_LIGHTMAP - Lightmap modulated with diffuse
// Stage 0: D3DTA_TEXTURE (no lighting)
// Stage 1: D3DTOP_MODULATE, texture * current
//==============================================================================
float4 PS_LIGHTMAP(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * lightmap * 2.0;
}

//==============================================================================
// EMT_LIGHTMAP_ADD - Lightmap added to diffuse
// Stage 1: D3DTOP_ADD
//==============================================================================
float4 PS_LIGHTMAP_ADD(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse + lightmap;
}

//==============================================================================
// EMT_LIGHTMAP_M2 - Lightmap * 2 then modulated with diffuse
// Stage 1: D3DTOP_MODULATE2X
//==============================================================================
float4 PS_LIGHTMAP_M2(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * (lightmap * 2.0);
}

//==============================================================================
// EMT_LIGHTMAP_M4 - Lightmap * 4 then modulated with diffuse
// Stage 1: D3DTOP_MODULATE4X
//==============================================================================
float4 PS_LIGHTMAP_M4(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * (lightmap * 4.0);
}

//==============================================================================
// EMT_LIGHTMAP_LIGHTING - Lightmap with dynamic lighting
// Stage 0: D3DTOP_MODULATE, Tex * Diffuse
//==============================================================================
float4 PS_LIGHTMAP_LIGHTING(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * lightmap * 2.0;
}

//==============================================================================
// EMT_SOLID_LIGHTING_GOURAUD - Solid with Gouraud shading (lighting interpolation)
// Similar to EMT_SOLID but performs per-pixel lighting using interpolated normals
// Ambient + Diffuse lighting model
//==============================================================================
float4 PS_SOLID_LIGHTING_GOURAUD(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);

    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));

    float ambient = 0.3f;
    float diffuse = max(dot(normal, lightDir), 0.0f);

    float lighting = ambient + diffuse;
    float3 litColor = texColor.rgb * lighting;

    return float4(litColor, texColor.a * input.Color.a);
}

//==============================================================================
// EMT_SOLID_LIGHTING_FLAT - Solid with Flat shading (constant per face lighting)
// Uses dFdx/dFdy to compute face normal, lighting computed once per face
// Ambient + Diffuse lighting model
//==============================================================================
float4 PS_SOLID_LIGHTING_FLAT(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);

    float3 dPosX = dFdx(input.Pos.xyz);
    float3 dPosY = dFdy(input.Pos.xyz);
    float3 faceNormal = normalize(cross(dPosX, dPosY));

    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));

    float ambient = 0.3f;
    float diffuse = max(dot(faceNormal, lightDir), 0.0f);

    float lighting = ambient + diffuse;
    float3 litColor = texColor.rgb * lighting;

    return float4(litColor, texColor.a * input.Color.a);
}

//==============================================================================
// EMT_SOLID_1_LAYER - 2tex coordinates but one texture
//==============================================================================
float4 PS_SOLID_1_LAYER(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);

    float4 diffuse, ambient, specular, emissive;

    if (ColorMaterialMode == ECM_DIFFUSE || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
        diffuse = input.Color;
    else
        diffuse = DiffuseColor;

    if (ColorMaterialMode == ECM_AMBIENT || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
        ambient = input.Color;
    else
        ambient = AmbientColor;

    if (ColorMaterialMode == ECM_SPECULAR)
        specular = input.Color;
    else
        specular = SpecularColor;

    if (ColorMaterialMode == ECM_EMISSIVE)
        emissive = input.Color;
    else
        emissive = EmissiveColor;

    float4 finalColor = texColor * diffuse;
    finalColor.rgb *= ambient.rgb;
    finalColor.rgb += emissive.rgb;

    return float4(finalColor.rgb * input.Color.a, texColor.a * input.Color.a);
}

//==============================================================================
// EMT_LIGHTMAP_LIGHTING_M4
//==============================================================================
float4 PS_LIGHTMAP_LIGHTING_M4(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * (lightmap * 4.0);
}

//==============================================================================
// EMT_DETAIL_MAP - Diffuse + Detail map using ADD_SIGNED
// Stage 0: D3DTOP_MODULATE, Tex * Diffuse
// Stage 1: D3DTOP_ADDSIGNED, Detail * CURRENT
//==============================================================================
float4 PS_DETAIL_MAP(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 detail = DetailTexture.Sample(LinearSampler, input.TexCoord1);
    float4 base = diffuse * input.Color;
    // ADD_SIGNED: base + detail - 0.5
    return base + (detail - 0.5);
}

//==============================================================================
// EMT_SPHERE_MAP - Environment reflection using sphere map
// Uses D3DTS_TEXTURE0 with sphere map transform
// TCI_CAMERASPACENORMAL
//==============================================================================
float4 PS_SPHERE_MAP(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
    float3 reflectVec = reflect(-viewDir, input.Normal);
    float2 sphereUV = reflectVec.xy * 0.5 + 0.5;
    float4 sphereColor = SphereMap.Sample(LinearSampler, sphereUV);
    return texColor * sphereColor * 2.0;
}

//==============================================================================
// EMT_REFLECTION_2_LAYER - Two layer reflection
// Stage 0: D3DTOP_MODULATE, Tex * Diffuse
// Stage 1: D3DTOP_MODULATE, Tex1 * CURRENT
// TCI_CAMERASPACEREFLECTIONVECTOR on stage 1
//==============================================================================
float4 PS_REFLECTION_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);
    return layer0 * input.Color * reflection * 2.0;
}

//==============================================================================
// EMT_TRANSPARENT_ADD_COLOR - Add source to dest (no alpha blend)
// SrcBlend: D3DBLEND_ONE
// DestBlend: D3DBLEND_INVSRCCOLOR
// ColorOp: D3DTOP_MODULATE, Tex * Diffuse
//==============================================================================
float4 PS_TRANSPARENT_ADD_COLOR(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float4 result = texColor * input.Color;
    return result; // Blend func: result + dest * (1 - result.rgb)
}

//==============================================================================
// EMT_TRANSPARENT_ALPHA_CHANNEL - Alpha blend with texture alpha
// SrcBlend: D3DBLEND_SRCALPHA
// DestBlend: D3DBLEND_INVSRCALPHA
// AlphaOp: D3DTOP_MODULATE, Tex * Diffuse (on stage 0)
// AlphaRef: MaterialTypeParam * 255
//==============================================================================
float4 PS_TRANSPARENT_ALPHA_CHANNEL(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float4 result = texColor * input.Color;
    result.a = texColor.a * input.Color.a;
    return result;
}

//==============================================================================
// EMT_TRANSPARENT_ALPHA_CHANNEL_REF - Alpha test only, no blending
// AlphaRef: 127
// AlphaFunc: D3DCMP_GREATEREQUAL
// AlphaOp: D3DTOP_SELECTARG1, D3DTA_TEXTURE
//==============================================================================
float4 PS_TRANSPARENT_ALPHA_CHANNEL_REF(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float4 result = texColor * input.Color;
    clip(result.a - 0.5);
    return result;
}

//==============================================================================
// EMT_TRANSPARENT_VERTEX_ALPHA - Alpha from vertex color
// SrcBlend: D3DBLEND_SRCALPHA
// DestBlend: D3DBLEND_INVSRCALPHA
// ColorOp: D3DTOP_MODULATE, Tex * Diffuse
// AlphaOp: D3DTOP_SELECTARG1, D3DTA_DIFFUSE
//==============================================================================
float4 PS_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);
}

//==============================================================================
// EMT_TRANSPARENT_REFLECTION_2_LAYER - Transparent reflection with vertex alpha
// SrcBlend: D3DBLEND_SRCALPHA, DestBlend: D3DBLEND_INVSRCALPHA
// Both stages use alpha from diffuse
// TCI_CAMERASPACEREFLECTIONVECTOR on stage 1
//==============================================================================
float4 PS_TRANSPARENT_REFLECTION_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);
    float4 result = layer0 * reflection * 2.0;
    result.a *= input.Color.a;
    return result;
}

//==============================================================================
// EMT_NORMAL_MAP_SOLID - Tangent space normal mapping with lighting
// Two lights supported (nearest two)
//==============================================================================
float4 PS_NORMAL_MAP_SOLID(PS_INPUT_TANGENTS input) : SV_TARGET
{
    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;

    float3x3 TBN = float3x3(
        normalize(input.Tangent),
        normalize(input.Binormal),
        normalize(input.Normal)
    );
    float3 normal = normalize(mul(normalTex, TBN));

    float3 lightDir1 = normalize(float3(1.0, 1.0, 1.0));
    float3 lightDir2 = normalize(float3(-1.0, 0.5, 0.8));
    float lighting = max(dot(normal, lightDir1), 0.0) + max(dot(normal, lightDir2), 0.0) * 0.5;

    return float4(diffuseColor.rgb * lighting, diffuseColor.a);
}

//==============================================================================
// EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR
//==============================================================================
float4 PS_NORMAL_MAP_TRANSPARENT_ADD_COLOR(PS_INPUT_TANGENTS input) : SV_TARGET
{
    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;

    float3x3 TBN = float3x3(
        normalize(input.Tangent),
        normalize(input.Binormal),
        normalize(input.Normal)
    );
    float3 normal = normalize(mul(normalTex, TBN));

    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
    float lighting = max(dot(normal, lightDir), 0.0);

    return float4(diffuseColor.rgb * lighting, diffuseColor.a);
}

//==============================================================================
// EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA
//==============================================================================
float4 PS_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_TANGENTS input) : SV_TARGET
{
    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;

    float3x3 TBN = float3x3(
        normalize(input.Tangent),
        normalize(input.Binormal),
        normalize(input.Normal)
    );
    float3 normal = normalize(mul(normalTex, TBN));

    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
    float lighting = max(dot(normal, lightDir), 0.0);

    float4 result = diffuseColor;
    result.rgb *= lighting;
    result.a *= input.Color.a;
    return result;
}

//==============================================================================
// EMT_PARALLAX_MAP_SOLID - Normal map with height offset (parallax mapping)
// Height scale from MaterialTypeParam (default 0.02)
//==============================================================================
float4 PS_PARALLAX_MAP_SOLID(PS_INPUT_TANGENTS input) : SV_TARGET
{
    const float heightScale = 0.02f;
    
    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
    float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;
    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;

    float3x3 TBN = float3x3(
        normalize(input.Tangent),
        normalize(input.Binormal),
        normalize(input.Normal)
    );
    float3 normal = normalize(mul(normalTex, TBN));

    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
    float2 parallaxOffset = height * heightScale * viewDir.xy;
    float2 offsetTexCoord = input.TexCoord + parallaxOffset;

    float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;

    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
    float lighting = max(dot(normal, lightDir), 0.0);

    return float4(texColor.rgb * lighting, texColor.a);
}

//==============================================================================
// EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR
//==============================================================================
float4 PS_PARALLAX_MAP_TRANSPARENT_ADD_COLOR(PS_INPUT_TANGENTS input) : SV_TARGET
{
    const float heightScale = 0.02f;
    
    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
    float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;
    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;

    float3x3 TBN = float3x3(
        normalize(input.Tangent),
        normalize(input.Binormal),
        normalize(input.Normal)
    );
    float3 normal = normalize(mul(normalTex, TBN));

    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
    float2 parallaxOffset = height * heightScale * viewDir.xy;
    float2 offsetTexCoord = input.TexCoord + parallaxOffset;

    float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;

    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
    float lighting = max(dot(normal, lightDir), 0.0);

    return float4(texColor.rgb * lighting, diffuseColor.a);
}

//==============================================================================
// EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA
//==============================================================================
float4 PS_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_TANGENTS input) : SV_TARGET
{
    const float heightScale = 0.02f;
    
    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;
    float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;
    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;

    float3x3 TBN = float3x3(
        normalize(input.Tangent),
        normalize(input.Binormal),
        normalize(input.Normal)
    );
    float3 normal = normalize(mul(normalTex, TBN));

    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
    float2 parallaxOffset = height * heightScale * viewDir.xy;
    float2 offsetTexCoord = input.TexCoord + parallaxOffset;

    float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;

    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));
    float lighting = max(dot(normal, lightDir), 0.0);

    float4 result = texColor;
    result.rgb *= lighting;
    result.a *= input.Color.a;
    return result;
}

//==============================================================================
// EMT_ONETEXTURE_BLEND - Generic blend function
// BlendFunc = source * sourceFactor + dest * destFactor
// srcFact/dstFact from MaterialTypeParam
// modulate: 1x=MODULATE, 2x=MODULATE2X, 4x=MODULATE4X
//==============================================================================
float4 PS_ONETEXTURE_BLEND(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float4 result = texColor * input.Color;

    // Generic blend - typically handled by blend state in D3D11
    // This shader returns the pre-blended color
    return result;
}

//==============================================================================
// dummy for d3d11 compiler
//==============================================================================
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
    float3 Normal : NORMAL;
};
cbuffer MatrixBuffer : register(b0) {
    float4x4 WorldViewProj;
};
VS_OUTPUT main(VS_INPUT input) {
    VS_OUTPUT output;
    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));
    output.Color = input.Color;
    output.TexCoord = input.TexCoord;
    output.Normal = input.Normal;
    return output;
};
#endif // __PS_MATERIAL_SHADERS_H__