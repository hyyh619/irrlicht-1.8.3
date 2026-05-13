// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D11Shader.h"
#include "CD3D11Driver.h"
#include "CD3D11ObjectTracker.h"
#include "os.h"

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

namespace irr
{
    namespace video
    {
        const char    VERTEX_SHADER_STANDARD[] =
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
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Color = input.Color;"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord;"
            "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    return output;"
            "}";

const char    VERTEX_SHADER_STANDARD_DIRECTIONAL[] =
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
            "    float3 WorldPos : TEXCOORD2;"
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
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord;"
            "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    float3 lightDir = normalize(-LightDir.xyz);"
            "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
            "    float NdotL = max(dot(normal, lightDir), 0.0);"
            "    float3 diffuse = MaterialDiffuseColor.rgb * DiffuseColor.rgb * NdotL;"
            "    float3 R = reflect(-lightDir, normal);"
            "    float RdotV = max(dot(R, viewDir), 0.0);"
            "    float3 specular = MaterialSpecularColor.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
            "    float3 ambient = MaterialAmbientColor.rgb * AmbientColor.rgb;"
            "    float3 emissive = MaterialEmissiveColor.rgb;"
            "    output.Color = float4(ambient + diffuse + specular + emissive, 1.0);"
            "    return output;"
            "}";

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
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord;"
            "    float3 lightDistant = LightPos.xyz - worldPos.xyz;"
            "    float dist = length(lightDistant);"
            "    float3 lightDir = normalize(lightDistant);"
            "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
            "    float NdotL = max(dot(normal, lightDir), 0.0);"
            "    float3 diffuse = MaterialDiffuseColor.rgb * DiffuseColor.rgb * NdotL;"
            "    float3 R = reflect(-lightDir, normal);"
            "    float RdotV = max(dot(R, viewDir), 0.0);"
            "    float3 specular = MaterialSpecularColor.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
            "    float3 ambient = MaterialAmbientColor.rgb * AmbientColor.rgb;"
            "    float3 emissive = MaterialEmissiveColor.rgb;"
            "    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);"
            "    float cond = dist <= LightRadius ? 1.0 : 0.0;"
            "    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * cond, 1.0);"
            "    return output;"
            "}";

        const char    VERTEX_SHADER_STANDARD_SPOT[] =
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
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord;"
            "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    float3 lightDir = LightPos.xyz - worldPos.xyz;"
            "    float dist = length(lightDir);"
            "    lightDir = normalize(lightDir);"
            "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
            "    float NdotL = max(dot(normal, lightDir), 0.0);"
            "    float3 diffuse = MaterialDiffuseColor.rgb * DiffuseColor.rgb * NdotL;"
            "    float3 R = reflect(-lightDir, normal);"
            "    float RdotV = max(dot(R, viewDir), 0.0);"
            "    float3 specular = MaterialSpecularColor.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
            "    float3 ambient = MaterialAmbientColor.rgb * AmbientColor.rgb;"
            "    float3 emissive = MaterialEmissiveColor.rgb;"
            "    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);"
            "    float cosAngle = dot(-lightDir, normalize(LightDir.xyz));"
            "    float cosOuter = cos(radians(OuterCone * 0.5));"
            "    float cosInner = cos(radians(InnerCone * 0.5));"
            "    float spotFactor = smoothstep(cosOuter, cosInner, cosAngle);"
            "    float cond = dist <= LightRadius ? 1.0 : 0.0;"
            "    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * spotFactor * cond, 1.0);"
            "    return output;"
            "}";

        const char    VERTEX_SHADER_2TCOORDS_DIRECTIONAL[] =
            "struct VS_INPUT {"
            "    float3 Pos : POSITION;"
            "    float3 Normal : NORMAL;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float2 TexCoord2 : TEXCOORD1;"
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
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord2;"
            "    float3 lightDir = -LightDir.xyz;"
            "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
            "    float NdotL = max(dot(normal, normalize(lightDir)), 0.0);"
            "    float3 diffuse = MaterialDiffuseColor.rgb * DiffuseColor.rgb * NdotL;"
            "    float3 R = reflect(-normalize(lightDir), normal);"
            "    float RdotV = max(dot(R, viewDir), 0.0);"
            "    float3 specular = MaterialSpecularColor.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
            "    float3 ambient = MaterialAmbientColor.rgb * AmbientColor.rgb;"
            "    float3 emissive = MaterialEmissiveColor.rgb;"
            "    output.Color = float4(ambient + diffuse + specular + emissive, 1.0);"
            "    return output;"
            "}";

        const char    VERTEX_SHADER_2TCOORDS_POINT[] =
            "struct VS_INPUT {"
            "    float3 Pos : POSITION;"
            "    float3 Normal : NORMAL;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float2 TexCoord2 : TEXCOORD1;"
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
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord2;"
            "    float3 lightDir = LightPos.xyz - worldPos.xyz;"
            "    float dist = length(lightDir);"
            "    lightDir = normalize(lightDir);"
            "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
            "    float NdotL = max(dot(normal, lightDir), 0.0);"
            "    float3 diffuse = MaterialDiffuseColor.rgb * DiffuseColor.rgb * NdotL;"
            "    float3 R = reflect(-lightDir, normal);"
            "    float RdotV = max(dot(R, viewDir), 0.0);"
            "    float3 specular = MaterialSpecularColor.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
            "    float3 ambient = MaterialAmbientColor.rgb * AmbientColor.rgb;"
            "    float3 emissive = MaterialEmissiveColor.rgb;"
            "    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);"
            "    float cond = dist <= LightRadius ? 1.0 : 0.0;"
            "    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * cond, 1.0);"
            "    return output;"
            "}";

const char    VERTEX_SHADER_2TCOORDS_SPOT[] =
            "struct VS_INPUT {"
            "    float3 Pos : POSITION;"
            "    float3 Normal : NORMAL;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float2 TexCoord2 : TEXCOORD1;"
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
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord2;"
            "    float3 lightDir = LightPos.xyz - worldPos.xyz;"
            "    float dist = length(lightDir);"
            "    lightDir = normalize(lightDir);"
            "    float3 normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    float3 viewDir = normalize(cameraPos - worldPos.xyz);"
            "    float NdotL = max(dot(normal, lightDir), 0.0);"
            "    float3 diffuse = MaterialDiffuseColor.rgb * DiffuseColor.rgb * NdotL;"
            "    float3 R = reflect(-lightDir, normal);"
            "    float RdotV = max(dot(R, viewDir), 0.0);"
            "    float3 specular = MaterialSpecularColor.rgb * SpecularColor.rgb * pow(RdotV, Shininess);"
            "    float3 ambient = MaterialAmbientColor.rgb * AmbientColor.rgb;"
            "    float3 emissive = MaterialEmissiveColor.rgb;"
            "    float att = 1.0 / (Attenuation.x + Attenuation.y * dist + Attenuation.z * dist * dist);"
            "    float cosAngle = dot(-lightDir, normalize(LightDir.xyz));"
            "    float cosOuter = cos(radians(OuterCone * 0.5));"
            "    float cosInner = cos(radians(InnerCone * 0.5));"
            "    float spotFactor = smoothstep(cosOuter, cosInner, cosAngle);"
            "    float cond = dist <= LightRadius ? 1.0 : 0.0;"
            "    output.Color = float4((ambient + diffuse * att + specular * att + emissive) * spotFactor * cond, 1.0);"
            "    return output;"
            "}";

        const char    VERTEX_SHADER_2TCOORDS[] =
            "struct VS_INPUT {"
            "    float3 Pos : POSITION;"
            "    float3 Normal : NORMAL;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float2 TexCoord2 : TEXCOORD1;"
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
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Color = input.Color;"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord2;"
            "    output.Normal = normalize(mul(input.Normal, (float3x3)transpose(World)));"
            "    return output;"
            "}";

        const char    VERTEX_SHADER_RECTANGLE[] =
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "    float4x4 World;"
            "};"
            "struct VS_INPUT {"
            "    float3 Pos : POSITION;"
            "    float4 Color : COLOR;"
            "};"
            "struct VS_OUTPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float3 WorldPos : WORLDPOS;"
            "};"
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Color = input.Color;"
            "    return output;"
            "}";

        const char    PIXEL_SHADER_RECTANGLE[] =
            "struct PS_INPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float3 WorldPos : WORLDPOS;"
            "};"
            "float4 main(PS_INPUT input) : SV_TARGET {"
            "    return input.Color;"
            "}";

        const char    VERTEX_SHADER_TANGENTS[] =
            "struct VS_INPUT {"
            "    float3 Pos : POSITION;"
            "    float3 Normal : NORMAL;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float3 Tangent : TANGENT;"
            "    float3 Binormal : BINORMAL;"
            "};"
            "struct VS_OUTPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float3 Normal : NORMAL;"
            "    float3 Tangent : TEXCOORD2;"
            "    float3 Binormal : TEXCOORD3;"
            "    float3 WorldPos : WORLDPOS;"
            "};"
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "    float4x4 World;"
            "};"
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    float4 worldPos = mul(float4(input.Pos, 1.0), transpose(World));"
            "    output.WorldPos = worldPos.xyz;"
            "    output.Color = input.Color;"
            "    output.TexCoord = input.TexCoord;"
            "    output.Normal = input.Normal;"
            "    output.Tangent = input.Tangent;"
            "    output.Binormal = input.Binormal;"
            "    return output;"
            "}";

        const char    PIXEL_SHADER_STANDARD[] =
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "};"
            "struct PS_INPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float3 Normal : TEXCOORD1;"
            "    float3 WorldPos : WORLDPOS;"
            "};"
            "Texture2D DiffuseTexture : register(t0);"
            "SamplerState LinearSampler : register(s0);"
            "float4 main(PS_INPUT input) : SV_TARGET {"
            "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
            "    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);"
            "}";

        const char    PIXEL_SHADER_2TCOORDS[] =
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "};"
            "struct PS_INPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float2 TexCoord2 : TEXCOORD1;"
            "    float3 Normal : NORMAL;"
            "    float3 WorldPos : WORLDPOS;"
            "};"
            "Texture2D DiffuseTexture : register(t0);"
            "SamplerState LinearSampler : register(s0);"
            "float4 main(PS_INPUT input) : SV_TARGET {"
            "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
            "    return input.Color * texColor;"
            "}";

        const char    PIXEL_SHADER_TANGENTS[] =
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "};"
            "struct PS_INPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float3 Normal : NORMAL;"
            "    float3 Tangent : TEXCOORD2;"
            "    float3 Binormal : TEXCOORD3;"
            "    float3 WorldPos : WORLDPOS;"
            "};"
            "Texture2D DiffuseTexture : register(t0);"
            "SamplerState LinearSampler : register(s0);"
            "float4 main(PS_INPUT input) : SV_TARGET {"
            "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
            "    return input.Color * texColor;"
            "}";

        CD3D11Shader::CD3D11Shader(CD3D11Driver *driver)
            : m_Driver(driver), m_ShaderType(EDST_COUNT), m_VertexType(EVT_STANDARD),
            m_MaterialType(EMT_SOLID), m_Compiled(false),
            m_ShaderBlob(0), m_VertexShader(0), m_HullShader(0), m_DomainShader(0),
            m_GeometryShader(0), m_PixelShader(0), m_ComputeShader(0),
            m_InputLayout(0), m_InputLayoutDesc(0), m_InputLayoutElementCount(0)
        {
#ifdef _DEBUG
            setDebugName("CD3D11Shader");
#endif
        }

        CD3D11Shader::~CD3D11Shader()
        {
            if (m_InputLayout)
            {
                IRR_D3D11_IL_RELEASE(m_InputLayout, "InputLayout");
                m_InputLayout->Release();
            }

            if (m_InputLayoutDesc)
                delete[] m_InputLayoutDesc;

            if (m_ShaderBlob)
                m_ShaderBlob->Release();

            if (m_VertexShader)
            {
                IRR_D3D11_VS_RELEASE(m_VertexShader, "VertexShader");
                m_VertexShader->Release();
            }

            if (m_HullShader)
            {
                IRR_D3D11_HS_RELEASE(m_HullShader, "HullShader");
                m_HullShader->Release();
            }

            if (m_DomainShader)
            {
                IRR_D3D11_DS_RELEASE(m_DomainShader, "DomainShader");
                m_DomainShader->Release();
            }

            if (m_GeometryShader)
            {
                IRR_D3D11_GS_RELEASE(m_GeometryShader, "GeometryShader");
                m_GeometryShader->Release();
            }

            if (m_PixelShader)
            {
                IRR_D3D11_PS_RELEASE(m_PixelShader, "PixelShader");
                m_PixelShader->Release();
            }

            if (m_ComputeShader)
            {
                IRR_D3D11_CS_RELEASE(m_ComputeShader, "ComputeShader");
                m_ComputeShader->Release();
            }
        }

        void CD3D11Shader::drop()
        {
            IReferenceCounted::drop();
        }

        bool CD3D11Shader::compile(E_D3D11_SHADER_TYPE type, const c8 *hlslSource, const c8 *entryPoint, const c8 *profile, const c8 *hlslSourcePart2)
        {
            if (!hlslSource || !entryPoint || !profile)
                return false;

            if (hlslSourcePart2)
            {
                m_HLSLSource = hlslSource;
                m_HLSLSource += hlslSourcePart2;
            }
            else
            {
                m_HLSLSource = hlslSource;
            }

            m_EntryPoint    = entryPoint;
            m_Profile       = profile;
            m_ShaderType    = type;

            ID3DBlob    *errorBlob = 0;

            const c8* sourceToCompile = hlslSource;
            size_t sourceLen = strlen(hlslSource);
            if (hlslSourcePart2)
            {
                sourceToCompile = m_HLSLSource.c_str();
                sourceLen = m_HLSLSource.size();
            }

            HRESULT    hr = D3DCompile(sourceToCompile, sourceLen, 0, 0, 0, entryPoint,
                                       profile, D3DCOMPILE_SKIP_VALIDATION, 0, &m_ShaderBlob, &errorBlob);

            if (FAILED(hr))
            {
                if (errorBlob)
                {
                    os::Printer::log("Shader compilation failed:", ELL_ERROR);
                    os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);
                    errorBlob->Release();
                }

                return false;
            }

            m_Compiled = true;
            return true;
        }

        bool CD3D11Shader::createVertexShader()
        {
            if (!m_Compiled || m_ShaderType != EDST_VERTEX || !m_ShaderBlob)
                return false;

            if (m_VertexShader)
            {
                m_VertexShader->Release();
                m_VertexShader = 0;
            }

            HRESULT    hr = m_Driver->m_pID3DDevice->CreateVertexShader(
                m_ShaderBlob->GetBufferPointer(),
                m_ShaderBlob->GetBufferSize(),
                nullptr,
                &m_VertexShader);

            IRR_D3D11_VS_CREATE(m_VertexShader, "VertexShader");
            return SUCCEEDED(hr);
        }

        bool CD3D11Shader::createHullShader()
        {
            if (!m_Compiled || m_ShaderType != EDST_HULL || !m_ShaderBlob)
                return false;

            if (m_HullShader)
            {
                m_HullShader->Release();
                m_HullShader = 0;
            }

            HRESULT    hr = m_Driver->m_pID3DDevice->CreateHullShader(
                m_ShaderBlob->GetBufferPointer(),
                m_ShaderBlob->GetBufferSize(),
                nullptr,
                &m_HullShader);

            IRR_D3D11_HS_CREATE(m_HullShader, "HullShader");
            return SUCCEEDED(hr);
        }

        bool CD3D11Shader::createDomainShader()
        {
            if (!m_Compiled || m_ShaderType != EDST_DOMAIN || !m_ShaderBlob)
                return false;

            if (m_DomainShader)
            {
                m_DomainShader->Release();
                m_DomainShader = 0;
            }

            HRESULT    hr = m_Driver->m_pID3DDevice->CreateDomainShader(
                m_ShaderBlob->GetBufferPointer(),
                m_ShaderBlob->GetBufferSize(),
                nullptr,
                &m_DomainShader);

            IRR_D3D11_DS_CREATE(m_DomainShader, "DomainShader");
            return SUCCEEDED(hr);
        }

        bool CD3D11Shader::createGeometryShader()
        {
            if (!m_Compiled || m_ShaderType != EDST_GEOMETRY || !m_ShaderBlob)
                return false;

            if (m_GeometryShader)
            {
                m_GeometryShader->Release();
                m_GeometryShader = 0;
            }

            HRESULT    hr = m_Driver->m_pID3DDevice->CreateGeometryShader(
                m_ShaderBlob->GetBufferPointer(),
                m_ShaderBlob->GetBufferSize(),
                nullptr,
                &m_GeometryShader);

            IRR_D3D11_GS_CREATE(m_GeometryShader, "GeometryShader");
            return SUCCEEDED(hr);
        }

        bool CD3D11Shader::createPixelShader()
        {
            if (!m_Compiled || m_ShaderType != EDST_PIXEL || !m_ShaderBlob)
                return false;

            if (m_PixelShader)
            {
                m_PixelShader->Release();
                m_PixelShader = 0;
            }

            HRESULT    hr = m_Driver->m_pID3DDevice->CreatePixelShader(
                m_ShaderBlob->GetBufferPointer(),
                m_ShaderBlob->GetBufferSize(),
                nullptr,
                &m_PixelShader);

            IRR_D3D11_PS_CREATE(m_PixelShader, "PixelShader");
            return SUCCEEDED(hr);
        }

        bool CD3D11Shader::createComputeShader()
        {
            if (!m_Compiled || m_ShaderType != EDST_COMPUTE || !m_ShaderBlob)
                return false;

            if (m_ComputeShader)
            {
                m_ComputeShader->Release();
                m_ComputeShader = 0;
            }

            HRESULT    hr = m_Driver->m_pID3DDevice->CreateComputeShader(
                m_ShaderBlob->GetBufferPointer(),
                m_ShaderBlob->GetBufferSize(),
                nullptr,
                &m_ComputeShader);

            IRR_D3D11_CS_CREATE(m_ComputeShader, "ComputeShader");
            return SUCCEEDED(hr);
        }

        bool CD3D11Shader::createInputLayout(const D3D11_INPUT_ELEMENT_DESC *layout, u32 elementCount)
        {
            if (!m_Compiled || m_ShaderType != EDST_VERTEX || !m_ShaderBlob)
                return false;

            if (m_InputLayout)
            {
                m_InputLayout->Release();
                m_InputLayout = 0;
            }

            if (m_InputLayoutDesc)
            {
                delete[] m_InputLayoutDesc;
                m_InputLayoutDesc = 0;
            }

            m_InputLayoutDesc = new D3D11_INPUT_ELEMENT_DESC[elementCount];
            memcpy(m_InputLayoutDesc, layout, sizeof(D3D11_INPUT_ELEMENT_DESC) * elementCount);
            m_InputLayoutElementCount = elementCount;

            HRESULT    hr = m_Driver->m_pID3DDevice->CreateInputLayout(
                layout,
                elementCount,
                m_ShaderBlob->GetBufferPointer(),
                m_ShaderBlob->GetBufferSize(),
                &m_InputLayout);

            IRR_D3D11_IL_CREATE(m_InputLayout, "InputLayout");
            return SUCCEEDED(hr);
        }
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_