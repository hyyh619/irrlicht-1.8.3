// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D11Shader.h"
#include "CD3D11Driver.h"
#include "os.h"

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

namespace irr
{
    namespace video
    {
        CD3D11Shader::CD3D11Shader(CD3D11Driver *driver)
            : m_Driver(driver), m_ShaderType(EDST_COUNT), m_Compiled(false),
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
                m_InputLayout->Release();

            if (m_InputLayoutDesc)
                delete[] m_InputLayoutDesc;

            if (m_ShaderBlob)
                m_ShaderBlob->Release();

            if (m_VertexShader)
                m_VertexShader->Release();

            if (m_HullShader)
                m_HullShader->Release();

            if (m_DomainShader)
                m_DomainShader->Release();

            if (m_GeometryShader)
                m_GeometryShader->Release();

            if (m_PixelShader)
                m_PixelShader->Release();

            if (m_ComputeShader)
                m_ComputeShader->Release();
        }

        void CD3D11Shader::drop()
        {
            IReferenceCounted::drop();
        }

        bool CD3D11Shader::compile(E_D3D11_SHADER_TYPE type, const c8 *hlslSource, const c8 *entryPoint, const c8 *profile)
        {
            if (!hlslSource || !entryPoint || !profile)
                return false;

            m_HLSLSource    = hlslSource;
            m_EntryPoint    = entryPoint;
            m_Profile       = profile;
            m_ShaderType    = type;

            ID3DBlob    *errorBlob = 0;

            HRESULT    hr = D3DCompile(hlslSource, strlen(hlslSource), 0, 0, 0, entryPoint,
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

            return SUCCEEDED(hr);
        }
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_