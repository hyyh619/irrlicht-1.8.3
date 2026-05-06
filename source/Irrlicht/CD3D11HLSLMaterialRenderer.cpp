// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include "CD3D11HLSLMaterialRenderer.h"
#include "CD3D11ObjectTracker.h"
#include "IShaderConstantSetCallBack.h"
#include "IVideoDriver.h"
#include "os.h"
#include "irrString.h"

#ifndef _IRR_D3D_NO_SHADER_DEBUGGING
#include <stdio.h>
#endif

namespace irr
{
    namespace video
    {
        CD3D11HLSLMaterialRenderer::CD3D11HLSLMaterialRenderer(ID3D11Device *d3dDevice,
            ID3D11DeviceContext *d3dContext, video::IVideoDriver *driver, s32 &outMaterialTypeNr,
            const c8 *vertexShaderProgram, const c8 *vertexShaderEntryPointName,
            E_VERTEX_SHADER_TYPE vsCompileTarget, const c8 *pixelShaderProgram,
            const c8 *pixelShaderEntryPointName, E_PIXEL_SHADER_TYPE psCompileTarget,
            IShaderConstantSetCallBack *callback, IMaterialRenderer *baseMaterial, s32 userData)
            : CD3D11ShaderMaterialRenderer(d3dDevice, d3dContext, driver, callback, baseMaterial, userData)
        {
#ifdef _DEBUG
            setDebugName("CD3D11HLSLMaterialRenderer");
#endif

            outMaterialTypeNr = -1;

            if (vsCompileTarget < 0 || vsCompileTarget > EVST_COUNT)
            {
                os::Printer::log("Invalid HLSL vertex shader compilation target", ELL_ERROR);
                return;
            }

            if (!createHLSLVertexShader(vertexShaderProgram,
                vertexShaderEntryPointName, VERTEX_SHADER_TYPE_NAMES[vsCompileTarget]))
                return;

            if (!createHLSLPixelShader(pixelShaderProgram,
                pixelShaderEntryPointName, PIXEL_SHADER_TYPE_NAMES[psCompileTarget]))
                return;

            outMaterialTypeNr = m_Driver->addMaterialRenderer(this);
        }


        CD3D11HLSLMaterialRenderer::~CD3D11HLSLMaterialRenderer()
        {
        }


        bool CD3D11HLSLMaterialRenderer::createHLSLVertexShader(const char *vertexShaderProgram,
            const char *shaderEntryPointName, const char *shaderTargetName)
        {
            if (!vertexShaderProgram)
                return true;

            ID3DBlob *buffer = 0;
            ID3DBlob *errors = 0;

#ifdef _IRR_D3D_NO_SHADER_DEBUGGING
            HRESULT h = D3DCompile(vertexShaderProgram, (UINT)strlen(vertexShaderProgram),
                0, 0, 0, shaderEntryPointName, shaderTargetName, 0, 0, &buffer, &errors);
#else
            static int irr_dbg_hlsl_file_nr = 0;
            ++irr_dbg_hlsl_file_nr;
            char tmp[32];
            sprintf(tmp, "irr_d3d11_dbg_hlsl_%d.vsh", irr_dbg_hlsl_file_nr);

            FILE *f = fopen(tmp, "wb");
            fwrite(vertexShaderProgram, strlen(vertexShaderProgram), 1, f);
            fflush(f);
            fclose(f);

            HRESULT h = D3DCompileFromFile(utf8ToUtf16(tmp).c_str(), 0, 0,
                shaderEntryPointName, shaderTargetName,
                D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, &buffer, &errors);
#endif

            if (FAILED(h))
            {
                os::Printer::log("HLSL vertex shader compilation failed:", ELL_ERROR);
                os::Printer::log("Shader name (first 64 chars):", ELL_ERROR);
                os::Printer::log(core::stringc(vertexShaderProgram).subString(0, 64).c_str(), ELL_ERROR);
                if (errors)
                {
                    os::Printer::log((c8*)errors->GetBufferPointer(), ELL_ERROR);
                    errors->Release();
                }
                if (buffer)
                    buffer->Release();
                return false;
            }

            if (errors)
                errors->Release();

            if (buffer)
            {
                if (FAILED(m_pID3DDevice->CreateVertexShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), nullptr, &m_VertexShader)))
                {
                    os::Printer::log("Could not create hlsl vertex shader.", ELL_ERROR);
                    buffer->Release();
                    return false;
                }
                IRR_D3D11_VS_CREATE(m_VertexShader, "VertexShader");
                buffer->Release();
                return true;
            }

            return false;
        }


        bool CD3D11HLSLMaterialRenderer::createHLSLPixelShader(const char *pixelShaderProgram,
            const char *shaderEntryPointName, const char *shaderTargetName)
        {
            if (!pixelShaderProgram)
                return true;

            ID3DBlob *buffer = 0;
            ID3DBlob *errors = 0;

#ifdef _IRR_D3D_NO_SHADER_DEBUGGING
            HRESULT h = D3DCompile(pixelShaderProgram, (UINT)strlen(pixelShaderProgram),
                0, 0, 0, shaderEntryPointName, shaderTargetName, 0, 0, &buffer, &errors);
#else
            static int irr_dbg_hlsl_file_nr = 0;
            ++irr_dbg_hlsl_file_nr;
            char tmp[32];
            sprintf(tmp, "irr_d3d11_dbg_hlsl_%d.psh", irr_dbg_hlsl_file_nr);

            FILE *f = fopen(tmp, "wb");
            fwrite(pixelShaderProgram, strlen(pixelShaderProgram), 1, f);
            fflush(f);
            fclose(f);

            HRESULT h = D3DCompileFromFile(utf8ToUtf16(tmp).c_str(), 0, 0,
                shaderEntryPointName, shaderTargetName,
                D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, &buffer, &errors);
#endif

            if (FAILED(h))
            {
                os::Printer::log("HLSL pixel shader compilation failed:", ELL_ERROR);
                os::Printer::log("Shader name (first 64 chars):", ELL_ERROR);
                os::Printer::log(core::stringc(pixelShaderProgram).subString(0, 64).c_str(), ELL_ERROR);
                if (errors)
                {
                    os::Printer::log((c8*)errors->GetBufferPointer(), ELL_ERROR);
                    errors->Release();
                }
                if (buffer)
                    buffer->Release();
                return false;
            }

            if (errors)
                errors->Release();

            if (buffer)
            {
                if (FAILED(m_pID3DDevice->CreatePixelShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), nullptr, &m_PixelShader)))
                {
                    os::Printer::log("Could not create hlsl pixel shader.", ELL_ERROR);
                    buffer->Release();
                    return false;
                }
                IRR_D3D11_PS_CREATE(m_PixelShader, "PixelShader");
                buffer->Release();
                return true;
            }

            return false;
        }


        bool CD3D11HLSLMaterialRenderer::OnRender(IMaterialRendererServices *service, E_VERTEX_TYPE vtxtype)
        {
            return CD3D11ShaderMaterialRenderer::OnRender(service, vtxtype);
        }


        void CD3D11HLSLMaterialRenderer::printHLSLVariables(ID3DBlob *constantTable)
        {
            os::Printer::log("HLSL variables for shader.");
        }
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_