// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include "CD3D11ShaderMaterialRenderer.h"
#include "CD3D11ObjectTracker.h"
#include "IShaderConstantSetCallBack.h"
#include "IMaterialRendererServices.h"
#include "IVideoDriver.h"
#include "os.h"

#ifndef _IRR_D3D_NO_SHADER_DEBUGGING
#include <stdio.h>
#endif

namespace irr
{
    namespace video
    {
        CD3D11ShaderMaterialRenderer::CD3D11ShaderMaterialRenderer(ID3D11Device *d3dDevice,
            ID3D11DeviceContext *d3dContext, video::IVideoDriver *driver, s32 &outMaterialTypeNr,
            const c8 *vertexShaderProgram, const c8 *pixelShaderProgram,
            IShaderConstantSetCallBack *callback, IMaterialRenderer *baseMaterial, s32 userData)
            : m_pID3DDevice(d3dDevice), m_pID3DDeviceContext(d3dContext), m_Driver(driver),
            m_CallBack(callback), m_BaseMaterial(baseMaterial),
            m_VertexShader(0), m_OldVertexShader(0), m_PixelShader(0), m_InputLayout(0), m_UserData(userData)
        {
#ifdef _DEBUG
            setDebugName("CD3D11ShaderMaterialRenderer");
#endif

            if (m_BaseMaterial)
                m_BaseMaterial->grab();

            if (m_CallBack)
                m_CallBack->grab();

            init(outMaterialTypeNr, vertexShaderProgram, pixelShaderProgram);
        }


        CD3D11ShaderMaterialRenderer::CD3D11ShaderMaterialRenderer(ID3D11Device *d3dDevice,
            ID3D11DeviceContext *d3dContext, video::IVideoDriver *driver,
            IShaderConstantSetCallBack *callback, IMaterialRenderer *baseMaterial, s32 userData)
            : m_pID3DDevice(d3dDevice), m_pID3DDeviceContext(d3dContext), m_Driver(driver),
            m_CallBack(callback), m_BaseMaterial(baseMaterial),
            m_VertexShader(0), m_OldVertexShader(0), m_PixelShader(0), m_InputLayout(0), m_UserData(userData)
        {
#ifdef _DEBUG
            setDebugName("CD3D11ShaderMaterialRenderer");
#endif

            if (m_BaseMaterial)
                m_BaseMaterial->grab();

            if (m_CallBack)
                m_CallBack->grab();
        }


        void CD3D11ShaderMaterialRenderer::init(s32 &outMaterialTypeNr,
            const c8 *vertexShaderProgram, const c8 *pixelShaderProgram)
        {
            outMaterialTypeNr = -1;

            if (!createVertexShader(vertexShaderProgram))
                return;

            if (!createPixelShader(pixelShaderProgram))
                return;

            outMaterialTypeNr = m_Driver->addMaterialRenderer(this);
        }


        CD3D11ShaderMaterialRenderer::~CD3D11ShaderMaterialRenderer()
        {
            if (m_CallBack)
                m_CallBack->drop();

            if (m_VertexShader)
            {
                IRR_D3D11_VS_RELEASE(m_VertexShader, "VertexShader");
                m_VertexShader->Release();
            }

            if (m_PixelShader)
            {
                IRR_D3D11_PS_RELEASE(m_PixelShader, "PixelShader");
                m_PixelShader->Release();
            }

            if (m_InputLayout)
            {
                IRR_D3D11_IL_RELEASE(m_InputLayout, "InputLayout");
                m_InputLayout->Release();
            }

            if (m_BaseMaterial)
                m_BaseMaterial->drop();
        }


        bool CD3D11ShaderMaterialRenderer::OnRender(IMaterialRendererServices *service, E_VERTEX_TYPE vtxtype)
        {
            if (m_CallBack && (m_VertexShader || m_PixelShader))
                m_CallBack->OnSetConstants(service, m_UserData);

            return true;
        }


        void CD3D11ShaderMaterialRenderer::OnSetMaterial(const video::SMaterial &material,
            const video::SMaterial &lastMaterial, bool resetAllRenderstates,
            video::IMaterialRendererServices *services)
        {
            if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
            {
                if (m_VertexShader)
                {
                    m_pID3DDeviceContext->VSSetShader(m_VertexShader, 0, 0);
                }

                if (m_PixelShader)
                {
                    m_pID3DDeviceContext->PSSetShader(m_PixelShader, 0, 0);
                }

                if (m_BaseMaterial)
                    m_BaseMaterial->OnSetMaterial(material, material, true, services);
            }

            if (m_CallBack)
                m_CallBack->OnSetMaterial(material);

            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        void CD3D11ShaderMaterialRenderer::OnUnsetMaterial()
        {
            m_pID3DDeviceContext->VSSetShader(0, 0, 0);
            m_pID3DDeviceContext->PSSetShader(0, 0, 0);

            if (m_BaseMaterial)
                m_BaseMaterial->OnUnsetMaterial();
        }


        bool CD3D11ShaderMaterialRenderer::isTransparent() const
        {
            return m_BaseMaterial ? m_BaseMaterial->isTransparent() : false;
        }


        bool CD3D11ShaderMaterialRenderer::createPixelShader(const c8 *pxsh)
        {
            if (!pxsh)
                return true;

            ID3DBlob *code = 0;
            ID3DBlob *errors = 0;

#ifdef _IRR_D3D_NO_SHADER_DEBUGGING
            HRESULT hr = D3DCompile(pxsh, (UINT)strlen(pxsh), 0, 0, 0, "main", "ps_5_0", 0, 0, &code, &errors);
#else
            static int irr_dbg_file_nr = 0;
            ++irr_dbg_file_nr;
            char tmp[32];
            sprintf(tmp, "irr_d3d11_dbg_shader_%d.psh", irr_dbg_file_nr);

            FILE *f = fopen(tmp, "wb");
            fwrite(pxsh, strlen(pxsh), 1, f);
            fflush(f);
            fclose(f);

            HRESULT hr = D3DCompileFromFile(utf8ToUtf16(tmp).c_str(), 0, 0, "main", "ps_5_0", 0, 0, &code, &errors);
#endif

            if (errors)
            {
                os::Printer::log("Pixel shader compilation failed:", ELL_ERROR);
                os::Printer::log("Shader name (first 64 chars):", ELL_ERROR);
                os::Printer::log(core::stringc(pxsh).subString(0, 64).c_str(), ELL_ERROR);
                os::Printer::log((c8*)errors->GetBufferPointer(), ELL_ERROR);
                errors->Release();
                if (code)
                    code->Release();
                return false;
            }

            if (FAILED(m_pID3DDevice->CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &m_PixelShader)))
            {
                os::Printer::log("Could not create pixel shader.", ELL_ERROR);
                if (code)
                    code->Release();
                return false;
            }

            if (code)
                code->Release();

            return true;
        }


        bool CD3D11ShaderMaterialRenderer::createVertexShader(const char *vtxsh)
        {
            if (!vtxsh)
                return true;

            ID3DBlob *code = 0;
            ID3DBlob *errors = 0;

#ifdef _IRR_D3D_NO_SHADER_DEBUGGING
            HRESULT hr = D3DCompile(vtxsh, (UINT)strlen(vtxsh), 0, 0, 0, "main", "vs_5_0", 0, 0, &code, &errors);
#else
            static int irr_dbg_file_nr = 0;
            ++irr_dbg_file_nr;
            char tmp[32];
            sprintf(tmp, "irr_d3d11_dbg_shader_%d.vsh", irr_dbg_file_nr);

            FILE *f = fopen(tmp, "wb");
            fwrite(vtxsh, strlen(vtxsh), 1, f);
            fflush(f);
            fclose(f);

            HRESULT hr = D3DCompileFromFile(utf8ToUtf16(tmp).c_str(), 0, 0, "main", "vs_5_0", 0, 0, &code, &errors);
#endif

            if (errors)
            {
                os::Printer::log("Vertex shader compilation failed:", ELL_ERROR);
                os::Printer::log("Shader name (first 64 chars):", ELL_ERROR);
                os::Printer::log(core::stringc(vtxsh).subString(0, 64).c_str(), ELL_ERROR);
                os::Printer::log((c8*)errors->GetBufferPointer(), ELL_ERROR);
                errors->Release();
                if (code)
                    code->Release();
                return false;
            }

            if (!code || FAILED(m_pID3DDevice->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &m_VertexShader)))
            {
                os::Printer::log("Could not create vertex shader.", ELL_ERROR);
                if (code)
                    code->Release();
                return false;
            }

            if (code)
                code->Release();

            return true;
        }
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_