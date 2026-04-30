// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include "CD3D11ShaderMaterialRenderer.h"
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
            : pID3DDevice(d3dDevice), pID3DDeviceContext(d3dContext), Driver(driver),
            CallBack(callback), BaseMaterial(baseMaterial),
            VertexShader(0), OldVertexShader(0), PixelShader(0), InputLayout(0), UserData(userData)
        {
#ifdef _DEBUG
            setDebugName("CD3D11ShaderMaterialRenderer");
#endif

            if (BaseMaterial)
                BaseMaterial->grab();

            if (CallBack)
                CallBack->grab();

            init(outMaterialTypeNr, vertexShaderProgram, pixelShaderProgram);
        }


        CD3D11ShaderMaterialRenderer::CD3D11ShaderMaterialRenderer(ID3D11Device *d3dDevice,
            ID3D11DeviceContext *d3dContext, video::IVideoDriver *driver,
            IShaderConstantSetCallBack *callback, IMaterialRenderer *baseMaterial, s32 userData)
            : pID3DDevice(d3dDevice), pID3DDeviceContext(d3dContext), Driver(driver),
            CallBack(callback), BaseMaterial(baseMaterial),
            VertexShader(0), OldVertexShader(0), PixelShader(0), InputLayout(0), UserData(userData)
        {
#ifdef _DEBUG
            setDebugName("CD3D11ShaderMaterialRenderer");
#endif

            if (BaseMaterial)
                BaseMaterial->grab();

            if (CallBack)
                CallBack->grab();
        }


        void CD3D11ShaderMaterialRenderer::init(s32 &outMaterialTypeNr,
            const c8 *vertexShaderProgram, const c8 *pixelShaderProgram)
        {
            outMaterialTypeNr = -1;

            if (!createVertexShader(vertexShaderProgram))
                return;

            if (!createPixelShader(pixelShaderProgram))
                return;

            outMaterialTypeNr = Driver->addMaterialRenderer(this);
        }


        CD3D11ShaderMaterialRenderer::~CD3D11ShaderMaterialRenderer()
        {
            if (CallBack)
                CallBack->drop();

            if (VertexShader)
                VertexShader->Release();

            if (PixelShader)
                PixelShader->Release();

            if (InputLayout)
                InputLayout->Release();

            if (BaseMaterial)
                BaseMaterial->drop();
        }


        bool CD3D11ShaderMaterialRenderer::OnRender(IMaterialRendererServices *service, E_VERTEX_TYPE vtxtype)
        {
            if (CallBack && (VertexShader || PixelShader))
                CallBack->OnSetConstants(service, UserData);

            return true;
        }


        void CD3D11ShaderMaterialRenderer::OnSetMaterial(const video::SMaterial &material,
            const video::SMaterial &lastMaterial, bool resetAllRenderstates,
            video::IMaterialRendererServices *services)
        {
            if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
            {
                if (VertexShader)
                {
                    pID3DDeviceContext->VSSetShader(VertexShader, 0, 0);
                }

                if (PixelShader)
                {
                    pID3DDeviceContext->PSSetShader(PixelShader, 0, 0);
                }

                if (BaseMaterial)
                    BaseMaterial->OnSetMaterial(material, material, true, services);
            }

            if (CallBack)
                CallBack->OnSetMaterial(material);

            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        void CD3D11ShaderMaterialRenderer::OnUnsetMaterial()
        {
            pID3DDeviceContext->VSSetShader(0, 0, 0);
            pID3DDeviceContext->PSSetShader(0, 0, 0);

            if (BaseMaterial)
                BaseMaterial->OnUnsetMaterial();
        }


        bool CD3D11ShaderMaterialRenderer::isTransparent() const
        {
            return BaseMaterial ? BaseMaterial->isTransparent() : false;
        }


        bool CD3D11ShaderMaterialRenderer::createPixelShader(const c8 *pxsh)
        {
            if (!pxsh)
                return true;

            ID3DBlob *code = 0;
            ID3DBlob *errors = 0;

#ifdef _IRR_D3D_NO_SHADER_DEBUGGING
            HRESULT hr = D3DCompile(pxsh, (UINT)strlen(pxsh), 0, 0, 0, "main", "ps_4_0", 0, 0, &code, &errors);
#else
            static int irr_dbg_file_nr = 0;
            ++irr_dbg_file_nr;
            char tmp[32];
            sprintf(tmp, "irr_d3d11_dbg_shader_%d.psh", irr_dbg_file_nr);

            FILE *f = fopen(tmp, "wb");
            fwrite(pxsh, strlen(pxsh), 1, f);
            fflush(f);
            fclose(f);

            HRESULT hr = D3DCompileFromFile(utf8ToUtf16(tmp).c_str(), 0, 0, "main", "ps_4_0", 0, 0, &code, &errors);
#endif

            if (errors)
            {
                os::Printer::log("Pixel shader compilation failed:", ELL_ERROR);
                os::Printer::log((c8*)errors->GetBufferPointer(), ELL_ERROR);
                errors->Release();
                if (code)
                    code->Release();
                return false;
            }

            if (FAILED(pID3DDevice->CreatePixelShader((DWORD*)code->GetBufferPointer(), &PixelShader)))
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
            HRESULT hr = D3DCompile(vtxsh, (UINT)strlen(vtxsh), 0, 0, 0, "main", "vs_4_0", 0, 0, &code, &errors);
#else
            static int irr_dbg_file_nr = 0;
            ++irr_dbg_file_nr;
            char tmp[32];
            sprintf(tmp, "irr_d3d11_dbg_shader_%d.vsh", irr_dbg_file_nr);

            FILE *f = fopen(tmp, "wb");
            fwrite(vtxsh, strlen(vtxsh), 1, f);
            fflush(f);
            fclose(f);

            HRESULT hr = D3DCompileFromFile(utf8ToUtf16(tmp).c_str(), 0, 0, "main", "vs_4_0", 0, 0, &code, &errors);
#endif

            if (errors)
            {
                os::Printer::log("Vertex shader compilation failed:", ELL_ERROR);
                os::Printer::log((c8*)errors->GetBufferPointer(), ELL_ERROR);
                errors->Release();
                if (code)
                    code->Release();
                return false;
            }

            if (!code || FAILED(pID3DDevice->CreateVertexShader((DWORD*)code->GetBufferPointer(), &VertexShader)))
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