// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include "CD3D11ParallaxMapRenderer.h"
#include "IVideoDriver.h"
#include "IMaterialRendererServices.h"
#include "os.h"
#include "SLight.h"

namespace irr
{
    namespace video
    {
        const char D3D11_PARALLAX_MAP_VSH[] =
            "// Irrlicht Engine D3D11 render path parallax mapping vertex shader\n"
            "// c0-3: Transposed world matrix \n"
            "// c4: Eye position \n"
            "// c8-11: Transposed worldViewProj matrix \n"
            "// c12: Light01 position \n"
            "// c13: x,y,z: Light01 color; .w: 1/LightRadius?\n"
            "// c14: Light02 position \n"
            "// c15: x,y,z: Light02 color; .w: 1/LightRadius?\n"
            "cbuffer cbWorldMatrix : register(b0) { matrix WorldMatrix; } \n"
            "cbuffer cbWorldViewProj : register(b1) { matrix WorldViewProjMatrix; } \n"
            "cbuffer cbLights : register(b2) { float4 LightPos[2]; float4 LightColor[2]; float4 EyePos; } \n"
            "\n"
            "struct VS_INPUT { \n"
            "    float3 Position : POSITION; \n"
            "    float3 Normal : NORMAL; \n"
            "    float4 Color : COLOR; \n"
            "    float2 TexCoord0 : TEXCOORD0; \n"
            "    float3 Tangent : TANGENT; \n"
            "    float3 Binormal : BINORMAL; \n"
            "}; \n"
            "\n"
            "struct VS_OUTPUT { \n"
            "    float4 Position : SV_POSITION; \n"
            "    float2 TexCoord0 : TEXCOORD0; \n"
            "    float3 LightVec1 : TEXCOORD1; \n"
            "    float3 LightVec2 : TEXCOORD2; \n"
            "    float3 EyeVec : TEXCOORD3; \n"
            "    float4 DiffuseColor1 : COLOR0; \n"
            "    float4 DiffuseColor2 : COLOR1; \n"
            "    float4 Alpha : COLOR2; \n"
            "}; \n"
            "\n"
            "VS_OUTPUT main(VS_INPUT input) { \n"
            "    VS_OUTPUT output; \n"
            "    float4 worldPos = mul(float4(input.Position, 1), WorldMatrix); \n"
            "    output.Position = mul(worldPos, WorldViewProjMatrix); \n"
            "    output.TexCoord0 = input.TexCoord0; \n"
            "    \n"
            "    float3 worldNormal = mul(input.Normal, (float3x3)WorldMatrix); \n"
            "    float3 worldTangent = mul(input.Tangent, (float3x3)WorldMatrix); \n"
            "    float3 worldBinormal = mul(input.Binormal, (float3x3)WorldMatrix); \n"
            "    \n"
            "    float3 lightVec1 = LightPos[0].xyz - worldPos.xyz; \n"
            "    float3 lightVec2 = LightPos[1].xyz - worldPos.xyz; \n"
            "    float3 eyeVec = EyePos.xyz - worldPos.xyz; \n"
            "    \n"
            "    output.LightVec1 = normalize(lightVec1); \n"
            "    output.LightVec2 = normalize(lightVec2); \n"
            "    output.EyeVec = normalize(eyeVec); \n"
            "    \n"
            "    float attenuation1 = 1.0 - saturate(length(lightVec1) * LightColor[0].w); \n"
            "    float attenuation2 = 1.0 - saturate(length(lightVec2) * LightColor[1].w); \n"
            "    \n"
            "    output.DiffuseColor1 = float4(LightColor[0].rgb * attenuation1, 1); \n"
            "    output.DiffuseColor2 = float4(LightColor[1].rgb * attenuation2, 1); \n"
            "    output.Alpha = input.Color; \n"
            "    \n"
            "    return output; \n"
            "} \n";

        const char D3D11_PARALLAX_MAP_PSH[] =
            "// Irrlicht Engine D3D11 render path parallax mapping pixel shader\n"
            "Texture2D colorMap : register(t0); \n"
            "Texture2D normalMap : register(t1); \n"
            "SamplerState sampleLinear : register(s0); \n"
            "\n"
            "cbuffer cbMaterial : register(b3) { float4 MaterialParams; } \n"
            "\n"
            "struct PS_INPUT { \n"
            "    float4 Position : SV_POSITION; \n"
            "    float2 TexCoord0 : TEXCOORD0; \n"
            "    float3 LightVec1 : TEXCOORD1; \n"
            "    float3 LightVec2 : TEXCOORD2; \n"
            "    float3 EyeVec : TEXCOORD3; \n"
            "    float4 DiffuseColor1 : COLOR0; \n"
            "    float4 DiffuseColor2 : COLOR1; \n"
            "    float4 Alpha : COLOR2; \n"
            "}; \n"
            "\n"
            "float4 main(PS_INPUT input) : SV_TARGET { \n"
            "    float height = normalMap.Sample(sampleLinear, input.TexCoord0).a; \n"
            "    \n"
            "    float scale = MaterialParams.x > 0 ? MaterialParams.x : 0.02; \n"
            "    float3 eyeVec = normalize(input.EyeVec * 2.0 - 1.0); \n"
            "    float2 texCoord = input.TexCoord0 + (height * scale) * eyeVec.xy; \n"
            "    \n"
            "    float4 color = colorMap.Sample(sampleLinear, texCoord); \n"
            "    float3 normal = normalMap.Sample(sampleLinear, texCoord).rgb * 2.0 - 1.0; \n"
            "    \n"
            "    float3 lightVec1 = input.LightVec1 * 2.0 - 1.0; \n"
            "    float3 lightVec2 = input.LightVec2 * 2.0 - 1.0; \n"
            "    \n"
            "    float diff1 = saturate(dot(normal, normalize(lightVec1))); \n"
            "    float diff2 = saturate(dot(normal, normalize(lightVec2))); \n"
            "    \n"
            "    float4 litColor1 = float4(color.rgb * diff1 * input.DiffuseColor1.rgb, 1); \n"
            "    float4 litColor2 = float4(color.rgb * diff2 * input.DiffuseColor2.rgb, 1); \n"
            "    \n"
            "    float4 finalColor = litColor1 + litColor2; \n"
            "    finalColor.a = input.Alpha.a; \n"
            "    \n"
            "    return finalColor; \n"
            "} \n";

        CD3D11ParallaxMapRenderer::CD3D11ParallaxMapRenderer(ID3D11Device *d3dDevice,
            ID3D11DeviceContext *d3dContext, video::IVideoDriver *driver,
            s32 &outMaterialTypeNr, IMaterialRenderer *baseMaterial)
            : CD3D11ShaderMaterialRenderer(d3dDevice, d3dContext, driver, 0, baseMaterial),
            m_CurrentScale(0.0f)
        {
#ifdef _DEBUG
            setDebugName("CD3D11ParallaxMapRenderer");
#endif

            m_CallBack = this;

            if (!createVertexShader(D3D11_PARALLAX_MAP_VSH))
            {
                outMaterialTypeNr = driver->addMaterialRenderer(this);
                return;
            }

            if (!createPixelShader(D3D11_PARALLAX_MAP_PSH))
            {
                outMaterialTypeNr = driver->addMaterialRenderer(this);
                return;
            }

            outMaterialTypeNr = driver->addMaterialRenderer(this);
        }


        CD3D11ParallaxMapRenderer::~CD3D11ParallaxMapRenderer()
        {
            if (m_CallBack == this)
                m_CallBack = 0;
        }


        bool CD3D11ParallaxMapRenderer::OnRender(IMaterialRendererServices *service, E_VERTEX_TYPE vtxtype)
        {
            if (vtxtype != video::EVT_TANGENTS)
            {
                os::Printer::log("Error: Parallax map renderer only supports vertices of type EVT_TANGENTS", ELL_ERROR);
                return false;
            }

            return CD3D11ShaderMaterialRenderer::OnRender(service, vtxtype);
        }


        void CD3D11ParallaxMapRenderer::OnSetMaterial(const video::SMaterial &material)
        {
            m_CurrentScale = material.MaterialTypeParam;
        }


        void CD3D11ParallaxMapRenderer::OnSetMaterial(const video::SMaterial &material,
            const video::SMaterial &lastMaterial, bool resetAllRenderstates,
            video::IMaterialRendererServices *services)
        {
            CD3D11ShaderMaterialRenderer::OnSetMaterial(material, lastMaterial,
                resetAllRenderstates, services);

            m_CurrentScale = material.MaterialTypeParam;
        }


        s32 CD3D11ParallaxMapRenderer::getRenderCapability() const
        {
            return 0;
        }


        void CD3D11ParallaxMapRenderer::OnSetConstants(IMaterialRendererServices *services, s32 userData)
        {
            video::IVideoDriver *driver = services->getVideoDriver();

            core::matrix4 worldMatrix = driver->getTransform(video::ETS_WORLD);
            core::matrix4 worldViewProj;
            worldViewProj = driver->getTransform(video::ETS_PROJECTION);
            worldViewProj *= driver->getTransform(video::ETS_VIEW);
            worldViewProj *= worldMatrix;

            services->setVertexShaderConstant(worldMatrix.getTransposed().pointer(), 0, 16);
            services->setVertexShaderConstant(worldViewProj.getTransposed().pointer(), 16, 16);

            f32 eyePos[4] = {0, 0, 0, 1};
            core::matrix4 viewInv = driver->getTransform(video::ETS_VIEW);
            viewInv.makeInverse();
            viewInv.multiplyWith1x4Matrix(eyePos);
            services->setVertexShaderConstant(eyePos, 12, 1);

            u32 cnt = driver->getDynamicLightCount();

            f32 lightPos[8];
            f32 lightColor[8];

            for (u32 i = 0; i < 2; ++i)
            {
                SLight light;
                if (i < cnt)
                    light = driver->getDynamicLight(i);
                else
                    light.DiffuseColor.set(0, 0, 0);

                lightPos[i * 4 + 0] = light.Position.X;
                lightPos[i * 4 + 1] = light.Position.Y;
                lightPos[i * 4 + 2] = light.Position.Z;
                lightPos[i * 4 + 3] = 1.0f / (light.Radius * light.Radius);

                lightColor[i * 4 + 0] = light.DiffuseColor.r;
                lightColor[i * 4 + 1] = light.DiffuseColor.g;
                lightColor[i * 4 + 2] = light.DiffuseColor.b;
                lightColor[i * 4 + 3] = 1.0f;
            }

            services->setVertexShaderConstant(lightPos, 32, 2);
            services->setVertexShaderConstant(lightColor, 40, 2);

            f32 factor = (m_CurrentScale != 0) ? m_CurrentScale : 0.02f;
            f32 materialParams[4] = {factor, 0, 0, 0};
            services->setPixelShaderConstant(materialParams, 0, 1);
        }
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_