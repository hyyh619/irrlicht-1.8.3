// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__
#define __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include "IMaterialRenderer.h"
#include "SColor.h"

namespace irr
{
    namespace video
    {
        const char    PS_MaterialShaders_Part1[] = R"(
// Auto-generated HLSL Pixel Shaders for Irrlicht Material Types
// Reference: CD3D9MaterialRenderer implementation

struct PS_INPUT_BASIC
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float2 TexCoord2 : TEXCOORD1;
    float3 Normal : NORMAL;
    float3 WorldPos : WORLDPOS;
};

struct PS_INPUT_2TEX
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float3 Normal : NORMAL;
    float3 WorldPos : WORLDPOS;
};

struct PS_INPUT_TANGENTS
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float3 Normal : NORMAL;
    float3 Tangent : TEXCOORD2;
    float3 Binormal : TEXCOORD3;
    float3 WorldPos : WORLDPOS;
};

#define ECM_NONE              0
#define ECM_DIFFUSE           1
#define ECM_AMBIENT           2
#define ECM_SPECULAR          3
#define ECM_EMISSIVE          4
#define ECM_DIFFUSE_AND_AMBIENT 5

cbuffer LightBuffer : register(b1) {
    float4 LightAmbient;
    float4 LightDiffuse;
    float4 LightSpecular;
    float3 LightPosition;
    float LightRadius;
    float3 LightDirection;
    float3 LightAttenuation;
    float LightOuterCone;
    float LightInnerCone;
    float LightFalloff;
    float LightType;
};

cbuffer MaterialBuffer : register(b2)
{
    float4 DiffuseColor;
    float4 AmbientColor;
    float4 SpecularColor;
    float4 EmissiveColor;
    float Shininess;
    uint ColorMaterialMode;
    float2 Padding;
};

cbuffer CameraBuffer : register(b3)
{
    float3 cameraPos;
};

Texture2D DiffuseTexture : register(t0);
Texture2D LightmapTexture : register(t1);
Texture2D DetailTexture : register(t1);
Texture2D NormalMap : register(t1);
Texture2D SphereMap : register(t2);

SamplerState LinearSampler : register(s0);

float4 PS_SOLID_COLOR_ONLY(PS_INPUT_BASIC input) : SV_TARGET
{
    return input.Color;
}

float4 PS_SOLID(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    return texColor * input.Color;
}

float4 PS_SOLID_WITH_LIGHT(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float3 nor = normalize(input.Normal);
    float3 lightDir = normalize(-LightDirection);
    float3 viewDir = normalize(cameraPos - input.WorldPos);
    float3 reflectDir = reflect(-lightDir, nor);
    float nDotL = max(dot(nor, lightDir), 0.0);
    float rDotV = max(dot(reflectDir, viewDir), 0.0);
    // ColorMaterialģʽ�ж�
    float4 matDiffuse = (ColorMaterialMode == ECM_DIFFUSE || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT) 
                        ? input.Color : DiffuseColor;
    float4 matAmbient = (ColorMaterialMode == ECM_AMBIENT || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT) 
                        ? input.Color : AmbientColor;
    float4 matSpecular = (ColorMaterialMode == ECM_SPECULAR) 
                        ? input.Color : SpecularColor;
    float4 matEmissive = (ColorMaterialMode == ECM_EMISSIVE) 
                        ? input.Color : EmissiveColor;
    // D3D9���չ�ʽ
    float4 diffuse = matDiffuse * LightDiffuse * nDotL;
    float4 specular = matSpecular * LightSpecular* pow(rDotV, Shininess);
    float4 ambient = matAmbient * LightAmbient;
    float4 emissive = matEmissive;
    float4 lightingResult = diffuse + specular + ambient + emissive;
    float4 finalColor = texColor * lightingResult;
    return float4(finalColor.rgb * input.Color.a, texColor.a * input.Color.a);
}

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

#if 0
    return float4(finalColor.rgb * input.Color.a, texColor.a * input.Color.a);
#else
    return finalColor;
#endif
}

float4 PS_SOLID_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 layer1 = DiffuseTexture.Sample(LinearSampler, input.TexCoord1);
    float alpha = input.Color.a;
    return lerp(layer0, layer1, alpha);
}

float4 PS_LIGHTMAP(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * lightmap * 2.0;
}

float4 PS_LIGHTMAP_ADD(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse + lightmap;
}

float4 PS_LIGHTMAP_M2(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * (lightmap * 2.0);
}

float4 PS_LIGHTMAP_M4(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * (lightmap * 4.0);
}

float4 PS_LIGHTMAP_LIGHTING(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * lightmap * 2.0;
}

float4 PS_LIGHTMAP_LIGHTING_M2(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * (lightmap * 2.0);
}

float4 PS_LIGHTMAP_LIGHTING_M4(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;
    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);
    return diffuse * (lightmap * 4.0);
}

float4 PS_DETAIL_MAP(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 detail = DetailTexture.Sample(LinearSampler, input.TexCoord1);
    float4 base = diffuse * input.Color;
    return base + (detail - 0.5);
}

float4 PS_SPHERE_MAP(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);
    float3 reflectVec = reflect(-viewDir, input.Normal);
    float2 sphereUV = reflectVec.xy * 0.5 + 0.5;
    float4 sphere_color = SphereMap.Sample(LinearSampler, sphereUV);
    return texColor * sphere_color * 2.0;
}

float4 PS_REFLECTION_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);
    return layer0 * input.Color * reflection * 2.0;
}

float4 PS_REFLECTION_2_LAYER_WITH_LIGHT(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);
    float3 nor = normalize(input.Normal);
    float3 lightDir = normalize(-LightDirection);
    float3 viewDir = normalize(cameraPos - input.WorldPos);
    float3 reflectDir = reflect(-lightDir, nor);
    float nDotL = max(dot(nor, lightDir), 0.0);
    float rDotV = max(dot(reflectDir, viewDir), 0.0);
    float4 matDiffuse = (ColorMaterialMode == ECM_DIFFUSE || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
                        ? input.Color : DiffuseColor;
    float4 matAmbient = (ColorMaterialMode == ECM_AMBIENT || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
                        ? input.Color : AmbientColor;
    float4 matSpecular = (ColorMaterialMode == ECM_SPECULAR)
                        ? input.Color : SpecularColor;
    float4 matEmissive = (ColorMaterialMode == ECM_EMISSIVE)
                        ? input.Color : EmissiveColor;
    float4 diffuse = matDiffuse * LightDiffuse * nDotL;
    float4 specular = matSpecular * LightSpecular * pow(rDotV, Shininess);
    float4 ambient = matAmbient * LightAmbient;
    float4 emissive = matEmissive;
    float4 lightingResult = diffuse + specular + ambient + emissive;
    float4 baseColor = texColor * lightingResult;
    return baseColor * reflection * 2.0;
}

float4 PS_TRANSPARENT_ADD_COLOR(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float4 result = texColor * input.Color;
    return result;
}

float4 PS_TRANSPARENT_ADD_COLOR_WITH_LIGHT(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float3 nor = normalize(input.Normal);
    float3 lightDir = normalize(-LightDirection);
    float3 viewDir = normalize(cameraPos - input.WorldPos);
    float3 reflectDir = reflect(-lightDir, nor);
    float nDotL = max(dot(nor, lightDir), 0.0);
    float rDotV = max(dot(reflectDir, viewDir), 0.0);
    float4 matDiffuse = (ColorMaterialMode == ECM_DIFFUSE || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
                        ? input.Color : DiffuseColor;
    float4 matAmbient = (ColorMaterialMode == ECM_AMBIENT || ColorMaterialMode == ECM_DIFFUSE_AND_AMBIENT)
                        ? input.Color : AmbientColor;
    float4 matSpecular = (ColorMaterialMode == ECM_SPECULAR)
                        ? input.Color : SpecularColor;
    float4 matEmissive = (ColorMaterialMode == ECM_EMISSIVE)
                        ? input.Color : EmissiveColor;
    float4 diffuse = matDiffuse * LightDiffuse * nDotL;
    float4 specular = matSpecular * LightSpecular * pow(rDotV, Shininess);
    float4 ambient = matAmbient * LightAmbient;
    float4 emissive = matEmissive;
    float4 lightingResult = diffuse + specular + ambient + emissive;
    float4 result = texColor * lightingResult;
    return float4(result.rgb, texColor.a * input.Color.a);
}

float4 PS_TRANSPARENT_ALPHA_CHANNEL(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float4 result = texColor * input.Color;
    result.a = texColor.a * input.Color.a;
    return result;
}

float4 PS_TRANSPARENT_ALPHA_CHANNEL_REF(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    clip(texColor.a - 0.5);
    return texColor * input.Color;
}

float4 PS_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);
}

float4 PS_TRANSPARENT_REFLECTION_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET
{
    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);
    float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);
    float4 result = layer0 * reflection * 2.0;
    result.a *= input.Color.a;
    return result;
}

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
)";

        const char    PS_MaterialShaders_Part2[] = R"(
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

float4 PS_ONETEXTURE_BLEND(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);
    float4 result = texColor * input.Color;
    return result;
}

//==============================================================================
// EMT_SOLID_LIGHTING_GOURAUD - Solid with Gouraud shading (lighting interpolation)
// Similar to EMT_SOLID but performs per-pixel lighting using interpolated normals
// Ambient + Diffuse lighting model
// Uses light data from LightBuffer cbuffer (register b1)
// LightType: 0=Point, 1=Spot, 2=Directional
//==============================================================================
float4 PS_SOLID_LIGHTING_GOURAUD(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);

    float3 normal = normalize(input.Normal);
    float3 lightDir;
    float attenuation = 1.0f;

    if (LightType < 0.5f)
    {
        float3 toLight = LightPosition - input.Pos.xyz;
        lightDir = normalize(toLight);
        float dist = length(toLight);
        float distFactor = 1.0 - clamp(dist / LightRadius, 0.0, 1.0);
        attenuation = distFactor / (LightAttenuation * dist + 1.0);
    }
    else if (LightType < 1.5f)
    {
        float3 toLight = LightPosition - input.Pos.xyz;
        lightDir = normalize(toLight);
        float dist = length(toLight);

        float spotCos = dot(-lightDir, normalize(LightDirection));
        float spotAngle = acos(spotCos);
        float innerConeRad = LightInnerCone * 3.14159f / 180.0f;
        float outerConeRad = LightOuterCone * 3.14159f / 180.0f;

        float spotFactor = 0.0f;
        if (spotAngle < outerConeRad && innerConeRad > outerConeRad)
        {
            spotFactor = pow((outerConeRad - spotAngle) / (outerConeRad - innerConeRad), LightFalloff);
            spotFactor = clamp(spotFactor, 0.0, 1.0);
        }

        float distFactor = 1.0 - clamp(dist / LightRadius, 0.0, 1.0);
        attenuation = distFactor * spotFactor / (LightAttenuation * dist + 1.0);
    }
    else
    {
        lightDir = normalize(-LightDirection);
    }

    float4 ambient = LightAmbient;
    float4 diffuse = max(dot(normal, lightDir), 0.0f);
    float4 lighting = ambient + diffuse * attenuation;
    float4 litColor = texColor * lighting * LightDiffuse;

    return float4(litColor.rgb, texColor.a * input.Color.a);
}

//==============================================================================
// EMT_SOLID_LIGHTING_FLAT - Solid with Flat shading (constant per face lighting)
// Uses dFdx/dFdy to compute face normal, lighting computed once per face
// Ambient + Diffuse lighting model
// Uses light data from LightBuffer cbuffer (register b1)
// LightType: 0=Point, 1=Spot, 2=Directional
//==============================================================================
float4 PS_SOLID_LIGHTING_FLAT(PS_INPUT_BASIC input) : SV_TARGET
{
    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);

    float3 dPosX = ddx(input.Pos.xyz);
    float3 dPosY = ddy(input.Pos.xyz);
    float3 faceNormal = normalize(cross(dPosX, dPosY));

    float3 lightDir;
    float attenuation = 1.0f;

    if (LightType < 0.5f)
    {
        float3 toLight = LightPosition - input.Pos.xyz;
        lightDir = normalize(toLight);
        float dist = length(toLight);
        float distFactor = 1.0 - clamp(dist / LightRadius, 0.0, 1.0);
        attenuation = distFactor / (LightAttenuation * dist + 1.0);
    }
    else if (LightType < 1.5f)
    {
        float3 toLight = LightPosition - input.Pos.xyz;
        lightDir = normalize(toLight);
        float dist = length(toLight);

        float spotCos = dot(-lightDir, normalize(LightDirection));
        float spotAngle = acos(spotCos);
        float innerConeRad = LightInnerCone * 3.14159f / 180.0f;
        float outerConeRad = LightOuterCone * 3.14159f / 180.0f;

        float spotFactor = 0.0f;
        if (spotAngle < outerConeRad && innerConeRad > outerConeRad)
        {
            spotFactor = pow((outerConeRad - spotAngle) / (outerConeRad - innerConeRad), LightFalloff);
            spotFactor = clamp(spotFactor, 0.0, 1.0);
        }

        float distFactor = 1.0 - clamp(dist / LightRadius, 0.0, 1.0);
        attenuation = distFactor * spotFactor / (LightAttenuation * dist + 1.0);
    }
    else
    {
        lightDir = normalize(-LightDirection);
    }

    float4 ambient = LightAmbient;
    float4 diffuse = max(dot(faceNormal, lightDir), 0.0f);
    float4 lighting = ambient + diffuse * attenuation;
    float4 litColor = texColor * lighting * LightDiffuse;

    return float4(litColor.rgb, texColor.a * input.Color.a);
}

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
     float3 WorldPos : WORLDPOS;
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
 )";

        class CD3D11Driver;

        class CD3D11MaterialRenderer : public IMaterialRenderer
        {
public:

            CD3D11MaterialRenderer(CD3D11Driver *driver, s32 materialType,
                                   const c8 *name);

            virtual ~CD3D11MaterialRenderer();

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);
            virtual bool OnSetTexture(u32 textureIndex, ITexture *texture);
            virtual void OnSetConstants(IMaterialRendererServices *services, s32 userData);
            virtual void PostRender();

protected:

            CD3D11Driver    *m_Driver;
            s32             m_MaterialType;
        };

        class CD3D11MaterialRenderer_SOLID : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_SOLID(CD3D11Driver *p, video::IVideoDriver *d);

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);
        };

        class CD3D11MaterialRenderer_SOLID_2_LAYER : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_SOLID_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d);

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);
        };

        class CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR(CD3D11Driver *p, video::IVideoDriver *d);

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual bool isTransparent() const;
        };

        class CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(CD3D11Driver *p, video::IVideoDriver *d);

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual bool isTransparent() const;
        };

        class CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(CD3D11Driver *p, video::IVideoDriver *d);

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual void OnUnsetMaterial();

            virtual bool isTransparent() const;
        };

        class CD3D11MaterialRenderer_ONETEXTURE_BLEND : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_ONETEXTURE_BLEND(CD3D11Driver *p, video::IVideoDriver *d);

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual bool isTransparent() const;

private:
            u32 getD3D11Blend(E_BLEND_FACTOR factor) const;
            u32 getD3D11Modulate(E_MODULATE_FUNC func) const;
            bool    m_Transparent;
        };

        class CD3D11MaterialRenderer_LIGHTMAP : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_LIGHTMAP(CD3D11Driver *p, video::IVideoDriver *d);

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);
        };

        class CD3D11MaterialRenderer_DETAIL_MAP : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_DETAIL_MAP(CD3D11Driver *p, video::IVideoDriver *d);

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);
        };

        class CD3D11MaterialRenderer_SPHERE_MAP : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_SPHERE_MAP(CD3D11Driver *p, video::IVideoDriver *d);

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual void OnUnsetMaterial();
        };

        class CD3D11MaterialRenderer_REFLECTION_2_LAYER : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_REFLECTION_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d);

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                                       bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual void OnUnsetMaterial();
        };
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_
#endif // __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__