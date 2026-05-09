// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D11Driver.h"
#include "CD3D11ObjectTracker.h"
#include "CD3D11Debug.h"

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include "os.h"
#include "S3DVertex.h"
#include "SColor.h"
#include "CD3D11Texture.h"
#include "CD3D11MaterialRenderer.h"
#include "CD3D11ShaderMaterialRenderer.h"
#include "CD3D11NormalMapRenderer.h"
#include "CD3D11ParallaxMapRenderer.h"
#include "CD3D11HLSLMaterialRenderer.h"
#include "CD3D11Shader.h"
#include "SIrrCreationParameters.h"

namespace irr
{
    namespace video
    {
        static const char    VERTEX_SHADER_STANDARD[] =
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
            "    float3 Normal : TEXCOORD1;"
            "};"
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "};"
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    output.Color = input.Color;"
            "    output.TexCoord = input.TexCoord;"
            "    output.Normal = input.Normal;"
            "    return output;"
            "}";

        static const char    VERTEX_SHADER_2TCOORDS[] =
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
            "    float3 Normal : TEXCOORD2;"
            "};"
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "};"
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    output.Color = input.Color;"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord2;"
            "    output.Normal = input.Normal;"
            "    return output;"
            "}";

        static const char    VERTEX_SHADER_RECTANGLE[] =
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "};"
            "struct VS_INPUT {"
            "    float3 Pos : POSITION;"
            "    float4 Color : COLOR;"
            "};"
            "struct VS_OUTPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "};"
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    output.Color = input.Color;"
            "    return output;"
            "}";

        static const char    PIXEL_SHADER_RECTANGLE[] =
            "struct PS_INPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "};"
            "float4 main(PS_INPUT input) : SV_TARGET {"
            "    return input.Color;"
            "}";

        static const char    VERTEX_SHADER_TANGENTS[] =
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
            "    float3 Normal : TEXCOORD1;"
            "    float3 Tangent : TEXCOORD2;"
            "    float3 Binormal : TEXCOORD3;"
            "};"
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "};"
            "VS_OUTPUT main(VS_INPUT input) {"
            "    VS_OUTPUT output;"
            "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
            "    output.Color = input.Color;"
            "    output.TexCoord = input.TexCoord;"
            "    output.Normal = input.Normal;"
            "    output.Tangent = input.Tangent;"
            "    output.Binormal = input.Binormal;"
            "    return output;"
            "}";

        static const char    PIXEL_SHADER_STANDARD[] =
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "};"
            "struct PS_INPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float3 Normal : TEXCOORD1;"
            "};"
            "Texture2D DiffuseTexture : register(t0);"
            "SamplerState LinearSampler : register(s0);"
            "float4 main(PS_INPUT input) : SV_TARGET {"
            "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
            "    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);"
            "}";

        static const char    PIXEL_SHADER_2TCOORDS[] =
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "};"
            "struct PS_INPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float2 TexCoord2 : TEXCOORD1;"
            "    float3 Normal : TEXCOORD2;"
            "};"
            "Texture2D DiffuseTexture : register(t0);"
            "SamplerState LinearSampler : register(s0);"
            "float4 main(PS_INPUT input) : SV_TARGET {"
            "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
            "    return input.Color * texColor;"
            "}";

        static const char    PIXEL_SHADER_TANGENTS[] =
            "cbuffer MatrixBuffer : register(b0) {"
            "    float4x4 WorldViewProj;"
            "};"
            "struct PS_INPUT {"
            "    float4 Pos : SV_POSITION;"
            "    float4 Color : COLOR;"
            "    float2 TexCoord : TEXCOORD0;"
            "    float3 Normal : TEXCOORD1;"
            "    float3 Tangent : TEXCOORD2;"
            "    float3 Binormal : TEXCOORD3;"
            "};"
            "Texture2D DiffuseTexture : register(t0);"
            "SamplerState LinearSampler : register(s0);"
            "float4 main(PS_INPUT input) : SV_TARGET {"
            "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
            "    return input.Color * texColor;"
            "}";

        CD3D11Driver::CD3D11Driver(const SIrrlichtCreationParameters &params, io::IFileSystem *io)
            : CNullDriver(io, params.WindowSize), m_CurrentRenderMode(ERM_NONE),
            m_ResetRenderStates(true), m_Transformation3DChanged(false),
            m_D3D11Library(0), m_DXGIFactory(0), m_Adapter(0), m_pID3DDevice(0), m_pID3DDeviceContext(0), m_pID3DDevice1(0), m_SwapChain(0),
            m_BackBufferRenderTargetView(0), m_DepthStencilView(0),
            m_WindowId(0), m_SceneSourceRect(0),
            m_LastVertexType((video::E_VERTEX_TYPE)-1), m_VendorID(0),
            m_InputLayout(), m_BuiltInVertexShader(), m_BuiltInPixelShader(),
            m_BuiltInVSInitialized(false), m_MaterialPSInitialized(false),
            m_RectangleVertexShader(0), m_RectanglePixelShader(0), m_RectangleInputLayout(0),
            m_RectangleShaderInitialized(false),
            m_TempVertexBuffer(0), m_TempIndexBuffer(0), m_MatrixConstantBuffer(0),
            m_TempVertexBufferSize(0), m_TempIndexBufferSize(0),
            m_TempIndexType(EIT_16BIT),
            m_RenderStateSets(),
            m_MaxTextureUnits(0), m_MaxUserClipPlanes(0), m_MaxMRTs(1), m_NumSetMRTs(1),
            m_MaxLightDistance(0.f), m_LastSetLight(-1),
            m_ColorFormat(ECOLOR_FORMAT::ECF_A8R8G8B8), m_DeviceRemoved(false),
            m_DriverWasReset(true), m_OcclusionQuerySupport(false),
            m_AlphaToCoverageSupport(false), m_Params(params)
        {
#ifdef _DEBUG
            setDebugName("CD3D11Driver");
#endif

            printVersion();

            for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
            {
                m_CurrentTexture[i]                     = 0;
                m_PreviousTexture[i]                    = 0;
                m_LastTextureMipMapsAvailable[i]        = false;
                m_CurrentSampler[i]                     = 0;
                m_PreviousSampler[i]                    = 0;
            }

            m_MaxLightDistance = sqrtf(FLT_MAX);
        }


        CSampler::CSampler(CD3D11Driver *driver)
            : m_Driver(driver), m_D3D11SamplerState(0), m_SamplerKey(0),
            m_Filter(D3D11_FILTER_MIN_MAG_MIP_LINEAR),
            m_AddressU(D3D11_TEXTURE_ADDRESS_WRAP),
            m_AddressV(D3D11_TEXTURE_ADDRESS_WRAP),
            m_AddressW(D3D11_TEXTURE_ADDRESS_WRAP),
            m_MipLODBias(0.0f), m_MaxAnisotropy(1),
            m_ComparisonFunc(D3D11_COMPARISON_NEVER),
            m_MinLOD(-FLT_MAX), m_MaxLOD(FLT_MAX)
        {
#ifdef _DEBUG
            setDebugName("CSampler");
#endif
        }


        CSampler::~CSampler()
        {
            if (m_D3D11SamplerState)
            {
                IRR_D3D11_SAMPLER_RELEASE(m_D3D11SamplerState, "DefaultSamplerState");
                m_D3D11SamplerState->Release();
            }
        }


        bool CSampler::createDefault()
        {
            D3D11_SAMPLER_DESC    desc;

            desc.Filter             = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU           = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.AddressV           = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.AddressW           = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.MipLODBias         = 0.0f;
            desc.MaxAnisotropy      = 1;
            desc.ComparisonFunc     = D3D11_COMPARISON_NEVER;
            desc.MinLOD             = -FLT_MAX;
            desc.MaxLOD             = FLT_MAX;

            return create(desc);
        }


        bool CSampler::create(const D3D11_SAMPLER_DESC &desc)
        {
            m_Filter            = desc.Filter;
            m_AddressU          = desc.AddressU;
            m_AddressV          = desc.AddressV;
            m_AddressW          = desc.AddressW;
            m_MipLODBias        = desc.MipLODBias;
            m_MaxAnisotropy     = desc.MaxAnisotropy;
            m_ComparisonFunc    = desc.ComparisonFunc;
            m_MinLOD            = desc.MinLOD;
            m_MaxLOD            = desc.MaxLOD;

            HRESULT    hr = m_Driver->m_pID3DDevice->CreateSamplerState(&desc, &m_D3D11SamplerState);
            IRR_D3D11_SAMPLER_CREATE(m_D3D11SamplerState, "DefaultSamplerState");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create sampler state.", ELL_ERROR);
                return false;
            }

            return true;
        }


        CD3D11Driver::~CD3D11Driver()
        {
            deleteMaterialRenders();

            for (u32 i = 0; i < m_MaterialRenderers.size(); ++i)
            {
                m_MaterialRenderers[i]->drop();
            }

            m_MaterialRenderers.clear();

            deleteAllTextures();
            removeAllOcclusionQueries();
            removeAllHardwareBuffers();

            for (u32 i = 0; i < m_DepthBuffers.size(); ++i)
            {
                m_DepthBuffers[i]->drop();
            }

            m_DepthBuffers.clear();

            for (u32 i = 0; i < m_ShaderPool.size(); ++i)
            {
                m_ShaderPool[i]->drop();
            }

            m_ShaderPool.clear();

            for (core::map<u64, CSampler*>::ParentLastIterator it = m_SamplerPool.getParentLastIterator(); !it.atEnd(); it++)
            {
                it->getValue()->drop();
            }

            m_SamplerPool.clear();

            for (u32 i = 0; i < EVT_VERTEX_TYPE_MAX; ++i)
            {
                if (m_InputLayout[i])
                {
                    IRR_D3D11_IL_RELEASE(m_InputLayout[i], "BuiltInInputLayout");
                    m_InputLayout[i]->Release();
                }

                if (m_BuiltInVertexShader[i])
                {
                    IRR_D3D11_VS_RELEASE(m_BuiltInVertexShader[i], "BuiltInVertexShader");
                    m_BuiltInVertexShader[i]->Release();
                }
            }

            for (u32 i = 0; i < EMT_MATERIAL_MAX; ++i)
            {
                if (m_BuiltInPixelShader[i])
                {
                    IRR_D3D11_PS_RELEASE(m_BuiltInPixelShader[i], "BuiltInPixelShader");
                    m_BuiltInPixelShader[i]->Release();
                }
            }

            if (m_RectangleInputLayout)
            {
                IRR_D3D11_IL_RELEASE(m_RectangleInputLayout, "RectangleInputLayout");
                m_RectangleInputLayout->Release();
            }

            if (m_RectangleVertexShader)
            {
                IRR_D3D11_VS_RELEASE(m_RectangleVertexShader, "RectangleVertexShader");
                m_RectangleVertexShader->Release();
            }

            if (m_RectanglePixelShader)
            {
                IRR_D3D11_PS_RELEASE(m_RectanglePixelShader, "RectanglePixelShader");
                m_RectanglePixelShader->Release();
            }

            if (m_TempVertexBuffer)
            {
                IRR_D3D11_BUFFER_RELEASE(m_TempVertexBuffer, "TempVertexBuffer");
                m_TempVertexBuffer->Release();
            }

            if (m_TempIndexBuffer)
            {
                IRR_D3D11_BUFFER_RELEASE(m_TempIndexBuffer, "TempIndexBuffer");
                m_TempIndexBuffer->Release();
            }

            if (m_MatrixConstantBuffer)
            {
                IRR_D3D11_BUFFER_RELEASE(m_MatrixConstantBuffer, "MatrixConstantBuffer");
                m_MatrixConstantBuffer->Release();
            }

            for (u32 i = 0; i < ERM_RENDER_MODE_MAX; ++i)
            {
                for (core::map<u64, SRenderStateSet>::ParentLastIterator it = m_RenderStateSets[i].getParentLastIterator();
                     !it.atEnd(); it++)
                {
                    SRenderStateSet    &stateSet = it->getValue();

                    if (stateSet.RasterizerState)
                    {
                        IRR_D3D11_RS_RELEASE(stateSet.RasterizerState, "RasterizerState");
                        stateSet.RasterizerState->Release();
                    }

                    if (stateSet.DepthStencilState)
                    {
                        IRR_D3D11_DSS_RELEASE(stateSet.DepthStencilState, "DepthStencilState");
                        stateSet.DepthStencilState->Release();
                    }

                    if (stateSet.BlendState)
                    {
                        IRR_D3D11_BLEND_RELEASE(stateSet.BlendState, "BlendState");
                        stateSet.BlendState->Release();
                    }
                }

                m_RenderStateSets[i].clear();
            }

            if (m_BackBufferRenderTargetView)
            {
                IRR_D3D11_RTV_RELEASE(m_BackBufferRenderTargetView, "BackBufferRenderTargetView");
                m_BackBufferRenderTargetView->Release();
            }

            if (m_DepthStencilView)
            {
                IRR_D3D11_DSV_RELEASE(m_DepthStencilView, "DepthStencilView");
                m_DepthStencilView->Release();
            }

            if (m_pID3DDeviceContext)
            {
                IRR_D3D11_DEVICE_CONTEXT_RELEASE(m_pID3DDeviceContext, "DeviceContext");
                m_pID3DDeviceContext->Release();
            }

            if (m_pID3DDevice1)
            {
                IRR_D3D11_DEVICE1_RELEASE(m_pID3DDevice1, "Device1");
                m_pID3DDevice1->Release();
            }

            if (m_pID3DDevice)
            {
                IRR_D3D11_DEVICE_RELEASE(m_pID3DDevice, "Device");
                m_pID3DDevice->Release();
            }

            if (m_SwapChain)
            {
                IRR_D3D11_SWAPCHAIN_RELEASE(m_SwapChain, "SwapChain");
                m_SwapChain->Release();
            }

            if (m_DXGIFactory)
                m_DXGIFactory->Release();

            if (m_Adapter)
                m_Adapter->Release();

#ifdef _DEBUG
            if (m_pID3D11Debug)
            {
                m_pID3D11Debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
                IRR_D3D11_DEBUG_RELEASE(m_pID3D11Debug, "D3D11Debug");
                m_pID3D11Debug->Release();
            }
#endif

            if (m_D3D11Library)
                FreeLibrary(m_D3D11Library);
        }


        bool CD3D11Driver::initDriver(HWND hwnd, bool pureSoftware)
        {
            char    tmp[512];

            m_WindowId = hwnd;

            m_D3D11Library = LoadLibraryA("d3d11.dll");
            if (!m_D3D11Library)
            {
                os::Printer::log("Could not load d3d11.dll.", ELL_ERROR);
                return false;
            }

            typedef HRESULT (WINAPI * PFN_D3D11CreateDevice)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, CONST D3D_FEATURE_LEVEL*, UINT, UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
            PFN_D3D11CreateDevice    D3D11CreateDevice = (PFN_D3D11CreateDevice)GetProcAddress(m_D3D11Library, "D3D11CreateDevice");
            if (!D3D11CreateDevice)
            {
                os::Printer::log("Could not find D3D11CreateDevice.", ELL_ERROR);
                return false;
            }

            D3D_FEATURE_LEVEL       featureLevel;
            UINT                    deviceFlags = 0;
#ifdef _DEBUG
            deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
            HRESULT    hr = D3D11CreateDevice(
                0,
                D3D_DRIVER_TYPE_HARDWARE,
                0,
                deviceFlags,
                0,
                0,
                D3D11_SDK_VERSION,
                &m_pID3DDevice,
                &featureLevel,
                &m_pID3DDeviceContext);

            if (FAILED(hr))
            {
                os::Printer::log("Could not create D3D11 device.", ELL_ERROR);
                return false;
            }

            IRR_D3D11_DEVICE_CREATE(m_pID3DDevice, "MainDevice");
            IRR_D3D11_DEVICE_CONTEXT_CREATE(m_pID3DDeviceContext, "MainDeviceContext");

#ifdef _DEBUG
            hr = m_pID3DDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_pID3D11Debug);
            if (SUCCEEDED(hr))
                IRR_D3D11_DEBUG_ADDREF(m_pID3D11Debug, "D3D11Debug");
#endif

            hr = m_pID3DDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&m_pID3DDevice1);
            if (SUCCEEDED(hr))
                IRR_D3D11_DEVICE1_ADDREF(m_pID3DDevice1, "Device1");

            if (FAILED(hr))
            {
                os::Printer::log("Could not get D3D11Device1 interface.", ELL_WARNING);
                m_pID3DDevice1 = 0;
            }

            m_pID3DDevice->CheckFormatSupport(DXGI_FORMAT_D24_UNORM_S8_UINT, &m_Caps);

            createMaterialRenderers();

            core::dimension2d<u32>    dim = m_Params.WindowSize;
            if (m_Params.Fullscreen)
            {
                DEVMODEW            devmode;
                core::stringw       tmp;
                EnumDisplaySettingsW(0, ENUM_CURRENT_SETTINGS, &devmode);
                dim.Width   = devmode.dmPelsWidth;
                dim.Height  = devmode.dmPelsHeight;
            }

            RECT    rect;
            GetClientRect(hwnd, &rect);
            core::dimension2d<u32>    currentDim;
            currentDim.Width    = rect.right - rect.left;
            currentDim.Height   = rect.bottom - rect.top;

            if (currentDim.Width == 0)
                currentDim.Width = dim.Width;

            if (currentDim.Height == 0)
                currentDim.Height = dim.Height;

            currentDim = dim;

            m_SwapChainBufferDesc.Width                     = currentDim.Width;
            m_SwapChainBufferDesc.Height                    = currentDim.Height;
            m_SwapChainBufferDesc.RefreshRate.Numerator     = 60;
            m_SwapChainBufferDesc.RefreshRate.Denominator   = 1;
            m_SwapChainBufferDesc.Format                    = DXGI_FORMAT_B8G8R8A8_UNORM;
            m_SwapChainBufferDesc.ScanlineOrdering          = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            m_SwapChainBufferDesc.Scaling                   = DXGI_MODE_SCALING_UNSPECIFIED;

            m_SwapChainDesc.BufferDesc              = m_SwapChainBufferDesc;
            m_SwapChainDesc.SampleDesc.Count        = 1;
            m_SwapChainDesc.SampleDesc.Quality      = 0;
            m_SwapChainDesc.BufferUsage             = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            m_SwapChainDesc.BufferCount             = 1;
            m_SwapChainDesc.OutputWindow            = hwnd;
            m_SwapChainDesc.Windowed                = !m_Params.Fullscreen;
            m_SwapChainDesc.SwapEffect              = DXGI_SWAP_EFFECT_DISCARD;
            m_SwapChainDesc.Flags                   = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

            hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&m_DXGIFactory);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create DXGIFactory.", ELL_ERROR);
                return false;
            }

            hr = m_DXGIFactory->EnumAdapters1(0, &m_Adapter);
            if (FAILED(hr))
            {
                os::Printer::log("Could not enumerate adapters.", ELL_ERROR);
                return false;
            }

            DXGI_ADAPTER_DESC    desc;
            m_Adapter->GetDesc(&desc);

            m_VendorID = static_cast<u16>(desc.VendorId);

            switch (desc.VendorId)
            {
                case 0x1002: m_VendorName = "ATI Technologies Inc."; break;

                case 0x10DE: m_VendorName = "NVIDIA Corporation"; break;

                case 0x102B: m_VendorName = "Matrox Electronic Systems Ltd."; break;

                case 0x121A: m_VendorName = "3dfx Interactive Inc"; break;

                case 0x5333: m_VendorName = "S3 Graphics Co., Ltd."; break;

                case 0x8086: m_VendorName = "Intel Corporation"; break;

                case 0x05404c42: m_VendorName = "Parallel Desktop"; break;

                default: m_VendorName = "Unknown VendorId: "; m_VendorName += (u32)desc.VendorId; break;
            }

            sprintf(tmp, "vendor: %s", m_VendorName.c_str());
            os::Printer::log(tmp, ELL_INFORMATION);

            hr = m_DXGIFactory->CreateSwapChain(m_pID3DDevice, &m_SwapChainDesc, &m_SwapChain);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create swap chain.", ELL_ERROR);
                return false;
            }

            IRR_D3D11_SWAPCHAIN_CREATE(m_SwapChain, "MainSwapChain");
            m_DXGIFactory->MakeWindowAssociation(hwnd, 0);

            ID3D11Texture2D    *backBuffer = 0;
            hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
            if (FAILED(hr))
            {
                os::Printer::log("Could not get back buffer.", ELL_ERROR);
                return false;
            }

            IRR_D3D11_TEXTURE2D_CREATE(backBuffer, "BackBuffer");
            hr = m_pID3DDevice->CreateRenderTargetView(backBuffer, 0, &m_BackBufferRenderTargetView);
            IRR_D3D11_RTV_CREATE(m_BackBufferRenderTargetView, "BackBufferRTV");
            backBuffer->Release();
            IRR_D3D11_TEXTURE2D_RELEASE(backBuffer, "BackBuffer");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create render target view.", ELL_ERROR);
                return false;
            }

            D3D11_TEXTURE2D_DESC    depthDesc;
            depthDesc.Width                 = currentDim.Width;
            depthDesc.Height                = currentDim.Height;
            depthDesc.MipLevels             = 1;
            depthDesc.ArraySize             = 1;
            depthDesc.Format                = DXGI_FORMAT_D24_UNORM_S8_UINT;
            depthDesc.SampleDesc.Count      = 1;
            depthDesc.SampleDesc.Quality    = 0;
            depthDesc.Usage                 = D3D11_USAGE_DEFAULT;
            depthDesc.BindFlags             = D3D11_BIND_DEPTH_STENCIL;
            depthDesc.CPUAccessFlags        = 0;
            depthDesc.MiscFlags             = 0;

            ID3D11Texture2D    *depthTexture = 0;
            hr = m_pID3DDevice->CreateTexture2D(&depthDesc, 0, &depthTexture);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create depth stencil texture.", ELL_ERROR);
                return false;
            }

            IRR_D3D11_TEXTURE2D_CREATE(depthTexture, "DepthStencilTexture");
            hr = m_pID3DDevice->CreateDepthStencilView(depthTexture, 0, &m_DepthStencilView);
            IRR_D3D11_DSV_CREATE(m_DepthStencilView, "DepthStencilView");
            depthTexture->Release();
            IRR_D3D11_TEXTURE2D_RELEASE(depthTexture, "DepthStencilTexture");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create depth stencil view.", ELL_ERROR);
                return false;
            }

            m_Viewport.TopLeftX     = 0;
            m_Viewport.TopLeftY     = 0;
            m_Viewport.Width        = (FLOAT)currentDim.Width;
            m_Viewport.Height       = (FLOAT)currentDim.Height;
            m_Viewport.MinDepth     = 0.0f;
            m_Viewport.MaxDepth     = 1.0f;

            m_DefaultViewport.TopLeftX      = 0;
            m_DefaultViewport.TopLeftY      = 0;
            m_DefaultViewport.Width         = (FLOAT)currentDim.Width;
            m_DefaultViewport.Height        = (FLOAT)currentDim.Height;
            m_DefaultViewport.MinDepth      = 0.0f;
            m_DefaultViewport.MaxDepth      = 1.0f;

            m_DefaultScissorRect.left   = 0;
            m_DefaultScissorRect.top    = 0;
            m_DefaultScissorRect.right  = currentDim.Width;
            m_DefaultScissorRect.bottom = currentDim.Height;

            m_CurrentRendertargetSize = currentDim;
            core::rect<s32>    driverInitArea(0, 0, currentDim.Width, currentDim.Height);
            setViewPort(driverInitArea);

            D3D11_BUFFER_DESC    matrixBufferDesc;
            matrixBufferDesc.ByteWidth              = sizeof(core::matrix4);
            matrixBufferDesc.Usage                  = D3D11_USAGE_DYNAMIC;
            matrixBufferDesc.BindFlags              = D3D11_BIND_CONSTANT_BUFFER;
            matrixBufferDesc.CPUAccessFlags         = D3D11_CPU_ACCESS_WRITE;
            matrixBufferDesc.MiscFlags              = 0;
            matrixBufferDesc.StructureByteStride    = 0;
            hr                                      = m_pID3DDevice->CreateBuffer(&matrixBufferDesc, 0, &m_MatrixConstantBuffer);
            IRR_D3D11_BUFFER_CREATE(m_MatrixConstantBuffer, "MatrixConstantBuffer");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create matrix constant buffer.", ELL_ERROR);
                return false;
            }

            setTransform(ETS_VIEW, core::IdentityMatrix);
            setTransform(ETS_PROJECTION, core::IdentityMatrix);
            setTransform(ETS_WORLD, core::IdentityMatrix);

            return true;
        }


        bool CD3D11Driver::beginScene(bool backBuffer, bool zBuffer, SColor color,
                                      const SExposedVideoData &videoData, core::rect<s32> *sourceRect)
        {
            CNullDriver::beginScene(backBuffer, zBuffer, color, videoData, sourceRect);

            if (m_DeviceRemoved)
            {
                HRESULT    hr = m_pID3DDevice->GetDeviceRemovedReason();
                if (hr == DXGI_ERROR_DEVICE_REMOVED)
                {
                    os::Printer::log("Device lost. Reason: DXGI_ERROR_DEVICE_REMOVED", ELL_WARNING);
                    if (!reset())
                        return false;
                }
                else
                {
                    os::Printer::log("Device lost. Unknown reason.", ELL_WARNING);
                    return false;
                }
            }

            m_pID3DDeviceContext->OMSetRenderTargets(1, &m_BackBufferRenderTargetView, m_DepthStencilView);
            m_pID3DDeviceContext->RSSetViewports(1, &m_Viewport);

            UINT    flags = 0;

            if (zBuffer)
                flags |= D3D11_CLEAR_DEPTH;

            if (m_Params.Stencilbuffer)
                flags |= D3D11_CLEAR_STENCIL;

            if (flags)
            {
                FLOAT       depth   = 1.0f;
                UINT8       stencil = 0;
                m_pID3DDeviceContext->ClearDepthStencilView(m_DepthStencilView, flags, depth, stencil);
            }

            if (backBuffer)
            {
                FLOAT    colorF[4];
                colorToD3D(color, colorF);
                m_pID3DDeviceContext->ClearRenderTargetView(m_BackBufferRenderTargetView, colorF);
            }

            m_SceneSourceRect = sourceRect;
            return true;
        }


        bool CD3D11Driver::endScene()
        {
            HRESULT    hr = m_SwapChain->Present(m_Params.Vsync ? 1 : 0, 0);

            m_DeviceRemoved = (hr == DXGI_ERROR_DEVICE_REMOVED);
            if (m_DeviceRemoved && !reset())
                return false;

            return true;
        }


        bool CD3D11Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const
        {
            switch (feature)
            {
                case EVDF_RENDER_TO_TARGET:
                    return true;

                case EVDF_MULTITEXTURE:
                    return true;

                case EVDF_BILINEAR_FILTER:
                    return true;

                case EVDF_MIP_MAP:
                    return true;

                case EVDF_MIP_MAP_AUTO_UPDATE:
                    return false;

                case EVDF_VERTEX_SHADER_1_1:
                case EVDF_VERTEX_SHADER_2_0:
                case EVDF_VERTEX_SHADER_3_0:
                    return true;

                case EVDF_PIXEL_SHADER_1_1:
                case EVDF_PIXEL_SHADER_2_0:
                case EVDF_PIXEL_SHADER_3_0:
                    return true;

                case EVDF_HARDWARE_TL:
                    return true;

                case EVDF_TEXTURE_NSQUARE:
                    return true;

                case EVDF_TEXTURE_NPOT:
                    return true;

                case EVDF_STENCIL_BUFFER:
                    return true;

                case EVDF_ALPHA_TO_COVERAGE:
                    return m_AlphaToCoverageSupport;

                case EVDF_COLOR_MASK:
                    return true;

                case EVDF_GEOMETRY_SHADER:
                    return true;

                case EVDF_OCCLUSION_QUERY:
                    return m_OcclusionQuerySupport;

                case EVDF_POLYGON_OFFSET:
                    return true;

                case EVDF_BLEND_OPERATIONS:
                    return true;

                case EVDF_TEXTURE_MATRIX:
                    return true;
            }

            return false;
        }


        void CD3D11Driver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4 &mat)
        {
            m_Matrices[state] = mat;
            if (state == ETS_WORLD)
                m_Transformation3DChanged = true;
        }


        const c8* CD3D11Driver::getMaterialTypeName(video::E_MATERIAL_TYPE materialType)
        {
            switch (materialType)
            {
                case video::EMT_SOLID: return "EMT_SOLID";

                case video::EMT_SOLID_2_LAYER: return "EMT_SOLID_2_LAYER";

                case video::EMT_LIGHTMAP: return "EMT_LIGHTMAP";

                case video::EMT_LIGHTMAP_ADD: return "EMT_LIGHTMAP_ADD";

                case video::EMT_LIGHTMAP_M2: return "EMT_LIGHTMAP_M2";

                case video::EMT_LIGHTMAP_M4: return "EMT_LIGHTMAP_M4";

                case video::EMT_LIGHTMAP_LIGHTING: return "EMT_LIGHTMAP_LIGHTING";

                case video::EMT_LIGHTMAP_LIGHTING_M2: return "EMT_LIGHTMAP_LIGHTING_M2";

                case video::EMT_LIGHTMAP_LIGHTING_M4: return "EMT_LIGHTMAP_LIGHTING_M4";

                case video::EMT_DETAIL_MAP: return "EMT_DETAIL_MAP";

                case video::EMT_SPHERE_MAP: return "EMT_SPHERE_MAP";

                case video::EMT_REFLECTION_2_LAYER: return "EMT_REFLECTION_2_LAYER";

                case video::EMT_TRANSPARENT_ADD_COLOR: return "EMT_TRANSPARENT_ADD_COLOR";

                case video::EMT_TRANSPARENT_ALPHA_CHANNEL: return "EMT_TRANSPARENT_ALPHA_CHANNEL";

                case video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF: return "EMT_TRANSPARENT_ALPHA_CHANNEL_REF";

                case video::EMT_TRANSPARENT_VERTEX_ALPHA: return "EMT_TRANSPARENT_VERTEX_ALPHA";

                case video::EMT_TRANSPARENT_REFLECTION_2_LAYER: return "EMT_TRANSPARENT_REFLECTION_2_LAYER";

                case video::EMT_NORMAL_MAP_SOLID: return "EMT_NORMAL_MAP_SOLID";

                case video::EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR: return "EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR";

                case video::EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA: return "EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA";

                case video::EMT_PARALLAX_MAP_SOLID: return "EMT_PARALLAX_MAP_SOLID";

                case video::EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR: return "EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR";

                case video::EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA: return "EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA";

                case video::EMT_ONETEXTURE_BLEND: return "EMT_ONETEXTURE_BLEND";

                case video::EMT_2D_RECTANGLE: return "EMT_2D_RECTANGLE";

                case video::EMT_MATERIAL_MAX: return "EMT_MATERIAL_MAX";

                default: return "?";
            }
        }


        void CD3D11Driver::setMaterial(const SMaterial &material)
        {
            m_Material = material;
            OverrideMaterial.apply(m_Material);

            m_bHasTex = false;

            for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
            {
                setActiveTexture(i, material.getTexture(i));
            }

            setBasicRenderStates(material, m_LastMaterial, true);
            m_LastMaterial = material;

#ifdef _IRR_MATERIAL_PRINT
            core::stringc    msg = "Type=";
            msg += getMaterialTypeName(material.MaterialType);
            msg += ", Wireframe=";
            msg += material.Wireframe ? "1" : "0";
            msg += ", Lighting=";
            msg += material.Lighting ? "1" : "0";
            msg += ", ZBuffer=";
            msg += core::stringc(material.ZBuffer);
            msg += ", Diffuse=(";
            msg += core::stringc(material.DiffuseColor.getRed());
            msg += ",";
            msg += core::stringc(material.DiffuseColor.getGreen());
            msg += ",";
            msg += core::stringc(material.DiffuseColor.getBlue());
            msg += ",";
            msg += core::stringc(material.DiffuseColor.getAlpha());
            msg += ")";

            for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
            {
                if (material.getTexture(i))
                {
                    msg += ", Tex";
                    msg += core::stringc(i);
                    msg += "=";
                    msg += material.getTexture(i)->getName().getPath().c_str();
                }
            }
            os::Printer::log("CD3D11Driver::setMaterial", msg.c_str());
#endif
        }


        bool CD3D11Driver::setRenderTarget(video::ITexture *texture, bool clearBackBuffer,
                                           bool clearZBuffer, SColor color)
        {
            return false;
        }


        bool CD3D11Driver::setRenderTarget(const core::array<video::IRenderTarget> &rtList,
                                           bool clearBackBuffer, bool clearZBuffer, SColor color)
        {
            return false;
        }


        void CD3D11Driver::setViewPort(const core::rect<s32> &area)
        {
            core::rect<s32>         vp          = area;
            core::rect<s32>         rendert(0, 0, getCurrentRenderTargetSize().Width, getCurrentRenderTargetSize().Height);

            vp.clipAgainst(rendert);

            D3D11_VIEWPORT    vpD3D;
            vpD3D.TopLeftX  = (FLOAT)vp.UpperLeftCorner.X;
            vpD3D.TopLeftY  = (FLOAT)vp.UpperLeftCorner.Y;
            vpD3D.Width     = (FLOAT)vp.getWidth();
            vpD3D.Height    = (FLOAT)vp.getHeight();
            vpD3D.MinDepth  = 0.0f;
            vpD3D.MaxDepth  = 1.0f;

            m_Viewport = vpD3D;

            m_ViewPort = vp;
            m_pID3DDeviceContext->RSSetViewports(1, &m_Viewport);
        }


        const core::rect<s32>&CD3D11Driver::getViewPort() const
        {
            return m_ViewPort;
        }


        bool CD3D11Driver::setActiveTexture(u32 stage, const video::ITexture *texture)
        {
            if (stage >= MATERIAL_MAX_TEXTURES)
                return false;

            if (texture)
            {
                if (texture->getDriverType() != EDT_DIRECT3D11)
                    return false;

                m_bHasTex = true;
            }

            m_CurrentTexture[stage] = texture;

            if (texture)
                m_CurrentSampler[stage] = getSampler(m_Material.TextureLayer[stage]);
            else
                m_CurrentSampler[stage] = 0;

            return true;
        }


        const core::dimension2d<u32>&CD3D11Driver::getCurrentRenderTargetSize() const
        {
            return m_CurrentRendertargetSize;
        }


        void CD3D11Driver::setBasicRenderStates(const SMaterial &material, const SMaterial &lastMaterial,
                                                bool resetAllRenderstates)
        {
            if (resetAllRenderstates || lastMaterial.Wireframe != material.Wireframe)
            {
                m_pID3DDeviceContext->RSSetState(0);
            }

            if (resetAllRenderstates || lastMaterial.GouraudShading != material.GouraudShading)
            {}

            if (resetAllRenderstates || lastMaterial.Lighting != material.Lighting)
            {}

            if (resetAllRenderstates || lastMaterial.ZWriteEnable != material.ZWriteEnable)
            {
                m_pID3DDeviceContext->OMSetDepthStencilState(0, 0);
            }

            if (resetAllRenderstates || lastMaterial.FogEnable != material.FogEnable)
            {}
        }


        bool CD3D11Driver::setRenderStates3DMode()
        {
            if (m_CurrentRenderMode == ERM_3D)
                return true;

            m_CurrentRenderMode = ERM_3D;
            setRenderStates(ERM_3D, false);
            return true;
        }


        void CD3D11Driver::setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel)
        {
            if (m_CurrentRenderMode == ERM_2D)
                return;

            m_CurrentRenderMode = ERM_2D;

            SRenderStateSet    *stateSet = getOrCreateRenderStateSet(ERM_2D, alpha, texture, alphaChannel, m_Material);
            if (!stateSet)
                return;

            m_pID3DDeviceContext->RSSetViewports(1, &m_DefaultViewport);
            m_pID3DDeviceContext->RSSetScissorRects(1, &m_DefaultScissorRect);
            m_pID3DDeviceContext->RSSetState(stateSet->RasterizerState);
            m_pID3DDeviceContext->OMSetDepthStencilState(stateSet->DepthStencilState, 0);

            FLOAT    blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_pID3DDeviceContext->OMSetBlendState(stateSet->BlendState, blendFactor, 0xFFFFFFFF);
        }


        bool CD3D11Driver::updateVertexHardwareBuffer(SHWBufferLink_d3d11 *hwBuffer)
        {
            if (!hwBuffer)
                return false;

            const scene::IMeshBuffer    *mb         = hwBuffer->MeshBuffer;
            const void                  *vertices   = mb->getVertices();
            const u32                   vertexCount = mb->getVertexCount();
            const E_VERTEX_TYPE         vType       = mb->getVertexType();
            const u32                   vertexSize  = getVertexPitchFromType(vType);
            const u32                   bufSize     = vertexSize * vertexCount;

            if (!hwBuffer->vertexBuffer || (bufSize > hwBuffer->vertexBufferSize))
            {
                if (hwBuffer->vertexBuffer)
                {
                    hwBuffer->vertexBuffer->Release();
                    hwBuffer->vertexBuffer = 0;
                }

                D3D11_BUFFER_DESC    bufferDesc;
                bufferDesc.ByteWidth            = bufSize;
                bufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
                bufferDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
                bufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
                bufferDesc.MiscFlags            = 0;
                bufferDesc.StructureByteStride  = 0;

                D3D11_SUBRESOURCE_DATA    subData;
                subData.pSysMem             = vertices;
                subData.SysMemPitch         = 0;
                subData.SysMemSlicePitch    = 0;

                if (FAILED(m_pID3DDevice->CreateBuffer(&bufferDesc, &subData, &hwBuffer->vertexBuffer)))
                    return false;

                IRR_D3D11_BUFFER_CREATE(hwBuffer->vertexBuffer, "VertexBuffer");
                hwBuffer->vertexBufferSize = bufSize;
            }
            else
            {
                D3D11_MAPPED_SUBRESOURCE    mapped;
                if (SUCCEEDED(m_pID3DDeviceContext->Map(hwBuffer->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
                {
                    memcpy(mapped.pData, vertices, bufSize);
                    m_pID3DDeviceContext->Unmap(hwBuffer->vertexBuffer, 0);
                }
            }

            return true;
        }


        bool CD3D11Driver::updateIndexHardwareBuffer(SHWBufferLink_d3d11 *hwBuffer)
        {
            if (!hwBuffer)
                return false;

            const scene::IMeshBuffer    *mb         = hwBuffer->MeshBuffer;
            const void                  *indices    = mb->getIndices();
            const u32                   indexCount  = mb->getIndexCount();
            const u32                   indexSize   = (mb->getIndexType() == EIT_16BIT) ? 2 : 4;
            const u32                   bufSize     = indexSize * indexCount;

            if (!hwBuffer->indexBuffer || (bufSize > hwBuffer->indexBufferSize))
            {
                if (hwBuffer->indexBuffer)
                {
                    hwBuffer->indexBuffer->Release();
                    hwBuffer->indexBuffer = 0;
                }

                D3D11_BUFFER_DESC    bufferDesc;
                bufferDesc.ByteWidth            = bufSize;
                bufferDesc.Usage                = D3D11_USAGE_DYNAMIC;
                bufferDesc.BindFlags            = D3D11_BIND_INDEX_BUFFER;
                bufferDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
                bufferDesc.MiscFlags            = 0;
                bufferDesc.StructureByteStride  = 0;

                D3D11_SUBRESOURCE_DATA    subData;
                subData.pSysMem             = indices;
                subData.SysMemPitch         = 0;
                subData.SysMemSlicePitch    = 0;

                DXGI_FORMAT    format = (indexSize == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

                if (FAILED(m_pID3DDevice->CreateBuffer(&bufferDesc, &subData, &hwBuffer->indexBuffer)))
                    return false;

                IRR_D3D11_BUFFER_CREATE(hwBuffer->indexBuffer, "IndexBuffer");
                hwBuffer->indexBufferSize = bufSize;
            }
            else
            {
                D3D11_MAPPED_SUBRESOURCE    mapped;
                if (SUCCEEDED(m_pID3DDeviceContext->Map(hwBuffer->indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
                {
                    memcpy(mapped.pData, indices, bufSize);
                    m_pID3DDeviceContext->Unmap(hwBuffer->indexBuffer, 0);
                }
            }

            return true;
        }


        bool CD3D11Driver::updateHardwareBuffer(SHWBufferLink *hwBuffer)
        {
            if (!hwBuffer)
                return false;

            SHWBufferLink_d3d11    *hwBufferD3D = (SHWBufferLink_d3d11*)hwBuffer;

            if (!updateVertexHardwareBuffer(hwBufferD3D))
                return false;

            if (!updateIndexHardwareBuffer(hwBufferD3D))
                return false;

            return true;
        }


        CD3D11Driver::SHWBufferLink* CD3D11Driver::createHardwareBuffer(const scene::IMeshBuffer *mb)
        {
            return new SHWBufferLink_d3d11(mb);
        }


        void CD3D11Driver::deleteHardwareBuffer(SHWBufferLink *hwBuffer)
        {
            if (hwBuffer)
            {
                SHWBufferLink_d3d11    *hwBufferD3D = (SHWBufferLink_d3d11*)hwBuffer;

                if (hwBufferD3D->vertexBuffer)
                {
                    IRR_D3D11_BUFFER_RELEASE(hwBufferD3D->vertexBuffer, "VertexBuffer");
                    hwBufferD3D->vertexBuffer->Release();
                    hwBufferD3D->vertexBuffer = 0;
                }

                if (hwBufferD3D->indexBuffer)
                {
                    IRR_D3D11_BUFFER_RELEASE(hwBufferD3D->indexBuffer, "IndexBuffer");
                    hwBufferD3D->indexBuffer->Release();
                    hwBufferD3D->indexBuffer = 0;
                }

                delete hwBuffer;
            }
        }


        void CD3D11Driver::drawHardwareBuffer(SHWBufferLink *hwBuffer)
        {
            if (!hwBuffer)
                return;

            SHWBufferLink_d3d11    *hwBufferD3D = (SHWBufferLink_d3d11*)hwBuffer;

            updateHardwareBuffer(hwBuffer);

            hwBuffer->LastUsed = 0;

            const scene::IMeshBuffer    *mb     = hwBuffer->MeshBuffer;
            const E_VERTEX_TYPE         vType   = mb->getVertexType();
            const u32                   stride  = getVertexPitchFromType(vType);
            const void                  *vPtr   = mb->getVertices();
            const void                  *iPtr   = mb->getIndices();

            setVSByVertexType(vType);
            setPSByMaterialType(m_Material.MaterialType);

            if (hwBufferD3D->vertexBuffer)
            {
                ID3D11Buffer    *buffers[1] = { hwBufferD3D->vertexBuffer };
                UINT            offsets[1]  = { 0 };
                UINT            strides[1]  = { stride };
                m_pID3DDeviceContext->IASetVertexBuffers(0, 1, buffers, strides, offsets);
                vPtr = 0;
            }

            if (hwBufferD3D->indexBuffer)
            {
                DXGI_FORMAT    format = (mb->getIndexType() == EIT_16BIT) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
                m_pID3DDeviceContext->IASetIndexBuffer(hwBufferD3D->indexBuffer, format, 0);
                iPtr = 0;
            }

            m_pID3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            updateMatrixConstantBuffer();

            if (hwBufferD3D->indexBuffer)
            {
                m_pID3DDeviceContext->DrawIndexed(mb->getIndexCount(), 0, 0);
            }
            else
            {
                m_pID3DDeviceContext->Draw(mb->getVertexCount(), 0);
            }
        }


        void CD3D11Driver::addOcclusionQuery(scene::ISceneNode *node, const scene::IMesh *mesh)
        {}


        void CD3D11Driver::removeOcclusionQuery(scene::ISceneNode *node)
        {}


        void CD3D11Driver::runOcclusionQuery(scene::ISceneNode *node, bool visible)
        {}


        void CD3D11Driver::updateOcclusionQuery(scene::ISceneNode *node, bool block)
        {}


        u32 CD3D11Driver::getOcclusionQueryResult(scene::ISceneNode *node) const
        {
            return 0;
        }


        void CD3D11Driver::drawVertexPrimitiveList(const void *vertices, u32 vertexCount,
                                                   const void *indexList, u32 primitiveCount,
                                                   E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                                                   E_INDEX_TYPE iType)
        {
            draw2D3DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType, iType, true);
        }


        void CD3D11Driver::draw2DVertexPrimitiveList(const void *vertices, u32 vertexCount,
                                                     const void *indexList, u32 primitiveCount,
                                                     E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                                                     E_INDEX_TYPE iType)
        {
            draw2D3DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType, iType, false);
        }


        void CD3D11Driver::draw2D3DVertexPrimitiveList(const void *vertices,
                                                       u32 vertexCount, const void *indexList, u32 primitiveCount,
                                                       E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                                                       E_INDEX_TYPE iType, bool is3D)
        {
            setVSByVertexType(vType);
            setPSByMaterialType(m_Material.MaterialType);

            const u32       stride              = getVertexPitchFromType(vType);
            const u32       vertexBufferSize    = stride * vertexCount;
            const u32       indexSize           = (iType == EIT_16BIT) ? 2 : 4;
            const u32       indexBufferSize     = indexSize * primitiveCount * 3;

            DXGI_FORMAT    indexFormat = (iType == EIT_16BIT) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

            if (is3D)
            {
                if (!setRenderStates3DMode())
                    return;
            }
            else
            {
                if (m_Material.MaterialType == EMT_ONETEXTURE_BLEND)
                {
                    E_BLEND_FACTOR      srcFact;
                    E_BLEND_FACTOR      dstFact;
                    E_MODULATE_FUNC     modulo;
                    u32                 alphaSource;
                    unpack_textureBlendFunc(srcFact, dstFact, modulo, alphaSource, m_Material.MaterialTypeParam);
                    setRenderStates2DMode(alphaSource & video::EAS_VERTEX_COLOR, (m_Material.getTexture(0) != 0), (alphaSource&video::EAS_TEXTURE) != 0);
                }
                else
                    setRenderStates2DMode(m_Material.MaterialType == EMT_TRANSPARENT_VERTEX_ALPHA, (m_Material.getTexture(0) != 0), m_Material.MaterialType == EMT_TRANSPARENT_ALPHA_CHANNEL);
            }

            if (!m_TempVertexBuffer || m_TempVertexBufferSize < vertexBufferSize)
            {
                if (m_TempVertexBuffer)
                {
                    IRR_D3D11_BUFFER_RELEASE(m_TempVertexBuffer, "TempVertexBuffer");
                    m_TempVertexBuffer->Release();
                }

                D3D11_BUFFER_DESC    vbDesc;
                vbDesc.ByteWidth            = vertexBufferSize;
                vbDesc.Usage                = D3D11_USAGE_DYNAMIC;
                vbDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
                vbDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
                vbDesc.MiscFlags            = 0;
                vbDesc.StructureByteStride  = 0;

                if (FAILED(m_pID3DDevice->CreateBuffer(&vbDesc, 0, &m_TempVertexBuffer)))
                {
                    os::Printer::log("Failed to create vertex buffer", ELL_ERROR);
                    return;
                }

                IRR_D3D11_BUFFER_CREATE(m_TempVertexBuffer, "TempVertexBuffer");
                m_TempVertexBufferSize = vertexBufferSize;
            }

            D3D11_MAPPED_SUBRESOURCE    mapped;
            if (SUCCEEDED(m_pID3DDeviceContext->Map(m_TempVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            {
                memcpy(mapped.pData, vertices, vertexBufferSize);
                m_pID3DDeviceContext->Unmap(m_TempVertexBuffer, 0);
            }
            else
            {
                os::Printer::log("Failed to map vertex buffer", ELL_ERROR);
                return;
            }

            ID3D11Buffer    *indexBuffer = 0;
            if (indexList)
            {
                if (!m_TempIndexBuffer || m_TempIndexBufferSize < indexBufferSize || m_TempIndexType != iType)
                {
                    if (m_TempIndexBuffer)
                    {
                        IRR_D3D11_BUFFER_RELEASE(m_TempIndexBuffer, "TempIndexBuffer");
                        m_TempIndexBuffer->Release();
                    }

                    D3D11_BUFFER_DESC    ibDesc;
                    ibDesc.ByteWidth            = indexBufferSize;
                    ibDesc.Usage                = D3D11_USAGE_DYNAMIC;
                    ibDesc.BindFlags            = D3D11_BIND_INDEX_BUFFER;
                    ibDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
                    ibDesc.MiscFlags            = 0;
                    ibDesc.StructureByteStride  = 0;

                    if (FAILED(m_pID3DDevice->CreateBuffer(&ibDesc, 0, &m_TempIndexBuffer)))
                    {
                        os::Printer::log("Failed to create index buffer", ELL_ERROR);
                        return;
                    }

                    IRR_D3D11_BUFFER_CREATE(m_TempIndexBuffer, "TempIndexBuffer");
                    m_TempIndexBufferSize   = indexBufferSize;
                    m_TempIndexType         = iType;
                }

                if (SUCCEEDED(m_pID3DDeviceContext->Map(m_TempIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
                {
                    memcpy(mapped.pData, indexList, indexBufferSize);
                    m_pID3DDeviceContext->Unmap(m_TempIndexBuffer, 0);
                }
                else
                {
                    os::Printer::log("Failed to map index buffer", ELL_ERROR);
                    return;
                }

                indexBuffer = m_TempIndexBuffer;
            }

            ID3D11Buffer    *buffers[1] = { m_TempVertexBuffer };
            UINT            offsets[1]  = { 0 };
            UINT            strides[1]  = { stride };
            m_pID3DDeviceContext->IASetVertexBuffers(0, 1, buffers, strides, offsets);

            if (indexBuffer)
                m_pID3DDeviceContext->IASetIndexBuffer(indexBuffer, indexFormat, 0);

            D3D11_PRIMITIVE_TOPOLOGY    topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

            switch (pType)
            {
                case scene::EPT_POINTS:
                case scene::EPT_POINT_SPRITES:
                    topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
                    break;

                case scene::EPT_LINE_STRIP:
                    topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
                    break;

                case scene::EPT_LINE_LOOP:
                case scene::EPT_LINES:
                    topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
                    break;

                case scene::EPT_TRIANGLE_STRIP:
                    topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                    break;

                case scene::EPT_TRIANGLE_FAN:
                case scene::EPT_TRIANGLES:
                    topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                    break;
            }

            m_pID3DDeviceContext->IASetPrimitiveTopology(topology);

            if (is3D)
                updateMatrixConstantBuffer();

            if (indexBuffer)
                m_pID3DDeviceContext->DrawIndexed(primitiveCount * 3, 0, 0);
            else
                m_pID3DDeviceContext->Draw(vertexCount, 0);

#ifdef _IRR_DUMP_DRAW_CALLS_
            dumpDrawCall("draw2D3DVertexPrimitiveList");
#endif
        }

#ifdef _IRR_DUMP_DRAW_CALLS_
        void CD3D11Driver::dumpDrawCall(const c8 *drawTypeName)
        {
            ++DrawCallCounter;

#if _IRR_DUMP_DRAW_CALLS_PRINT
            os::Printer::log("DrawCall", core::stringc(DrawCallCounter).c_str(), ELL_INFORMATION);
            os::Printer::log(drawTypeName);
#endif

#if _IRR_DUMP_DRAW_CALLS_FILE
            IImage    *image = createScreenShot(ECOLOR_FORMAT::ECF_A8R8G8B8, video::ERT_FRAME_BUFFER);
            if (image)
            {
                core::stringc    filename = "draw_";
                filename    += DrawCallCounter;
                filename    += "_";
                filename    += drawTypeName;
                filename    += ".jpg";
                writeImageToFile(image, filename.c_str(), 90);
                image->drop();
            }
#endif
        }
#endif


        void CD3D11Driver::draw2DImage(const video::ITexture *texture, const core::position2d<s32> &destPos,
                                       const core::rect<s32> &sourceRect, const core::rect<s32> *clipRect,
                                       SColor color, bool useAlphaChannelOfTexture)
        {}


        void CD3D11Driver::draw2DImage(const video::ITexture *texture, const core::rect<s32> &destRect,
                                       const core::rect<s32> &sourceRect, const core::rect<s32> *clipRect,
                                       const video::SColor* const colors, bool useAlphaChannelOfTexture)
        {}


        void CD3D11Driver::draw2DImageBatch(const video::ITexture *texture,
                                            const core::array<core::position2d<s32> > &positions,
                                            const core::array<core::rect<s32> > &sourceRects,
                                            const core::rect<s32> *clipRect,
                                            SColor color, bool useAlphaChannelOfTexture)
        {
            if (!texture)
                return;

            if (!setActiveTexture(0, const_cast<video::ITexture*>(texture)))
                return;

            setRenderStates2DMode(color.getAlpha() < 255, true, useAlphaChannelOfTexture);

            const irr::u32    drawCount = core::min_<u32>(positions.size(), sourceRects.size());

            core::array<S3DVertex>      vtx(drawCount * 4);
            core::array<u16>            indices(drawCount * 6);

            for (u32 i = 0; i < drawCount; i++)
            {
                core::position2d<s32>       targetPos   = positions[i];
                core::position2d<s32>       sourcePos   = sourceRects[i].UpperLeftCorner;
                core::dimension2d<s32>      sourceSize(sourceRects[i].getSize());

                if (clipRect)
                {
                    if (targetPos.X < clipRect->UpperLeftCorner.X)
                    {
                        sourceSize.Width += targetPos.X - clipRect->UpperLeftCorner.X;
                        if (sourceSize.Width <= 0)
                            continue;

                        sourcePos.X     -= targetPos.X - clipRect->UpperLeftCorner.X;
                        targetPos.X     = clipRect->UpperLeftCorner.X;
                    }

                    if (targetPos.X + (s32)sourceSize.Width > clipRect->LowerRightCorner.X)
                    {
                        sourceSize.Width -= (targetPos.X + sourceSize.Width) - clipRect->LowerRightCorner.X;
                        if (sourceSize.Width <= 0)
                            continue;
                    }

                    if (targetPos.Y < clipRect->UpperLeftCorner.Y)
                    {
                        sourceSize.Height += targetPos.Y - clipRect->UpperLeftCorner.Y;
                        if (sourceSize.Height <= 0)
                            continue;

                        sourcePos.Y     -= targetPos.Y - clipRect->UpperLeftCorner.Y;
                        targetPos.Y     = clipRect->UpperLeftCorner.Y;
                    }

                    if (targetPos.Y + (s32)sourceSize.Height > clipRect->LowerRightCorner.Y)
                    {
                        sourceSize.Height -= (targetPos.Y + sourceSize.Height) - clipRect->LowerRightCorner.Y;
                        if (sourceSize.Height <= 0)
                            continue;
                    }
                }

                if (targetPos.X < 0)
                {
                    sourceSize.Width += targetPos.X;
                    if (sourceSize.Width <= 0)
                        continue;

                    sourcePos.X     -= targetPos.X;
                    targetPos.X     = 0;
                }

                const core::dimension2d<u32>    &renderTargetSize = getCurrentRenderTargetSize();

                if (targetPos.X + sourceSize.Width > (s32)renderTargetSize.Width)
                {
                    sourceSize.Width -= (targetPos.X + sourceSize.Width) - renderTargetSize.Width;
                    if (sourceSize.Width <= 0)
                        continue;
                }

                if (targetPos.Y < 0)
                {
                    sourceSize.Height += targetPos.Y;
                    if (sourceSize.Height <= 0)
                        continue;

                    sourcePos.Y     -= targetPos.Y;
                    targetPos.Y     = 0;
                }

                if (targetPos.Y + sourceSize.Height > (s32)renderTargetSize.Height)
                {
                    sourceSize.Height -= (targetPos.Y + sourceSize.Height) - renderTargetSize.Height;
                    if (sourceSize.Height <= 0)
                        continue;
                }

                core::rect<f32>    tcoords;
                tcoords.UpperLeftCorner.X   = (((f32)sourcePos.X)) / texture->getOriginalSize().Width;
                tcoords.UpperLeftCorner.Y   = (((f32)sourcePos.Y)) / texture->getOriginalSize().Height;
                tcoords.LowerRightCorner.X  = tcoords.UpperLeftCorner.X + ((f32)(sourceSize.Width) / texture->getOriginalSize().Width);
                tcoords.LowerRightCorner.Y  = tcoords.UpperLeftCorner.Y + ((f32)(sourceSize.Height) / texture->getOriginalSize().Height);

                const core::rect<s32>    poss(targetPos, sourceSize);

                vtx.push_back(S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.UpperLeftCorner.Y, 0.0f,
                                        0.0f, 0.0f, 0.0f, color,
                                        tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y));
                vtx.push_back(S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.UpperLeftCorner.Y, 0.0f,
                                        0.0f, 0.0f, 0.0f, color,
                                        tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y));
                vtx.push_back(S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.LowerRightCorner.Y, 0.0f,
                                        0.0f, 0.0f, 0.0f, color,
                                        tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y));
                vtx.push_back(S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.LowerRightCorner.Y, 0.0f,
                                        0.0f, 0.0f, 0.0f, color,
                                        tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y));

                const u32    curPos = vtx.size() - 4;
                indices.push_back(0 + curPos);
                indices.push_back(1 + curPos);
                indices.push_back(2 + curPos);

                indices.push_back(0 + curPos);
                indices.push_back(2 + curPos);
                indices.push_back(3 + curPos);
            }

            if (!vtx.size())
                return;

            setVSByVertexType(EVT_STANDARD);
            setPSByMaterialType(m_Material.MaterialType);

            core::matrix4    mvp;
            mvp.buildProjectionMatrixOrthoLH(f32(getCurrentRenderTargetSize().Width), f32(-(s32)getCurrentRenderTargetSize().Height), -1.0f, 1.0f);
            mvp.setTranslation(core::vector3df(-1.0f, 1.0f, 0.0f));

            D3D11_MAPPED_SUBRESOURCE    mappedMatrix;
            if (SUCCEEDED(m_pID3DDeviceContext->Map(m_MatrixConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedMatrix)))
            {
                memcpy(mappedMatrix.pData, mvp.pointer(), sizeof(core::matrix4));
                m_pID3DDeviceContext->Unmap(m_MatrixConstantBuffer, 0);
            }

            m_pID3DDeviceContext->VSSetConstantBuffers(0, 1, &m_MatrixConstantBuffer);

            const u32       vertexBufferSize    = vtx.size() * sizeof(S3DVertex);
            const u32       indexBufferSize     = indices.size() * sizeof(u16);

            if (!m_TempVertexBuffer || m_TempVertexBufferSize < vertexBufferSize)
            {
                if (m_TempVertexBuffer)
                {
                    IRR_D3D11_BUFFER_RELEASE(m_TempVertexBuffer, "TempVertexBuffer");
                    m_TempVertexBuffer->Release();
                }

                D3D11_BUFFER_DESC    vbDesc;
                vbDesc.ByteWidth            = vertexBufferSize;
                vbDesc.Usage                = D3D11_USAGE_DYNAMIC;
                vbDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
                vbDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
                vbDesc.MiscFlags            = 0;
                vbDesc.StructureByteStride  = 0;

                if (FAILED(m_pID3DDevice->CreateBuffer(&vbDesc, 0, &m_TempVertexBuffer)))
                {
                    os::Printer::log("Failed to create vertex buffer", ELL_ERROR);
                    return;
                }

                IRR_D3D11_BUFFER_CREATE(m_TempVertexBuffer, "TempVertexBuffer");
                m_TempVertexBufferSize = vertexBufferSize;
            }

            D3D11_MAPPED_SUBRESOURCE    mappedVB;
            if (SUCCEEDED(m_pID3DDeviceContext->Map(m_TempVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVB)))
            {
                memcpy(mappedVB.pData, vtx.pointer(), vertexBufferSize);
                m_pID3DDeviceContext->Unmap(m_TempVertexBuffer, 0);
            }
            else
            {
                os::Printer::log("Failed to map vertex buffer", ELL_ERROR);
                return;
            }

            if (!m_TempIndexBuffer || m_TempIndexBufferSize < indexBufferSize || m_TempIndexType != EIT_16BIT)
            {
                if (m_TempIndexBuffer)
                {
                    IRR_D3D11_BUFFER_RELEASE(m_TempIndexBuffer, "TempIndexBuffer");
                    m_TempIndexBuffer->Release();
                }

                D3D11_BUFFER_DESC    ibDesc;
                ibDesc.ByteWidth            = indexBufferSize;
                ibDesc.Usage                = D3D11_USAGE_DYNAMIC;
                ibDesc.BindFlags            = D3D11_BIND_INDEX_BUFFER;
                ibDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
                ibDesc.MiscFlags            = 0;
                ibDesc.StructureByteStride  = 0;

                if (FAILED(m_pID3DDevice->CreateBuffer(&ibDesc, 0, &m_TempIndexBuffer)))
                {
                    os::Printer::log("Failed to create index buffer", ELL_ERROR);
                    return;
                }

                IRR_D3D11_BUFFER_CREATE(m_TempIndexBuffer, "TempIndexBuffer");
                m_TempIndexBufferSize   = indexBufferSize;
                m_TempIndexType         = EIT_16BIT;
            }

            D3D11_MAPPED_SUBRESOURCE    mappedIB;
            if (SUCCEEDED(m_pID3DDeviceContext->Map(m_TempIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIB)))
            {
                memcpy(mappedIB.pData, indices.pointer(), indexBufferSize);
                m_pID3DDeviceContext->Unmap(m_TempIndexBuffer, 0);
            }
            else
            {
                os::Printer::log("Failed to map index buffer", ELL_ERROR);
                return;
            }

            ID3D11Buffer    *buffers[1] = { m_TempVertexBuffer };
            UINT            offsets[1]  = { 0 };
            UINT            strides[1]  = { sizeof(S3DVertex) };
            m_pID3DDeviceContext->IASetVertexBuffers(0, 1, buffers, strides, offsets);
            m_pID3DDeviceContext->IASetIndexBuffer(m_TempIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
            m_pID3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_pID3DDeviceContext->DrawIndexed(indices.size(), 0, 0);

#ifdef _IRR_DUMP_DRAW_CALLS_
            dumpDrawCall("draw2DImageBatch");
#endif
        }


        void CD3D11Driver::draw2DRectangle(const core::rect<s32> &pos,
                                           SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
                                           const core::rect<s32> *clip)
        {
            core::rect<s32>    clippedRect(pos);

            if (clip)
                clippedRect.clipAgainst(*clip);

            if (!clippedRect.isValid())
                return;

            struct SRectVertex
            {
                core::vector3df Pos;
                SColor          Color;
            };

            SRectVertex    vertices[4];
            vertices[0].Pos     = core::vector3df((f32)clippedRect.UpperLeftCorner.X, (f32)clippedRect.UpperLeftCorner.Y, 0.0f);
            vertices[0].Color   = colorLeftUp;
            vertices[1].Pos     = core::vector3df((f32)clippedRect.LowerRightCorner.X, (f32)clippedRect.UpperLeftCorner.Y, 0.0f);
            vertices[1].Color   = colorRightUp;
            vertices[2].Pos     = core::vector3df((f32)clippedRect.LowerRightCorner.X, (f32)clippedRect.LowerRightCorner.Y, 0.0f);
            vertices[2].Color   = colorRightDown;
            vertices[3].Pos     = core::vector3df((f32)clippedRect.UpperLeftCorner.X, (f32)clippedRect.LowerRightCorner.Y, 0.0f);
            vertices[3].Color   = colorLeftDown;

            u16    indices[6] = { 0, 1, 2, 0, 2, 3 };

            setRenderStates2DMode(colorLeftUp.getAlpha() < 255 ||
                                  colorRightUp.getAlpha() < 255 ||
                                  colorLeftDown.getAlpha() < 255 ||
                                  colorRightDown.getAlpha() < 255, false, false);

            set2DRectangleShader();

            core::matrix4    mvp;
            mvp.buildProjectionMatrixOrthoLH(f32(getCurrentRenderTargetSize().Width), f32(-(s32)getCurrentRenderTargetSize().Height), -1.0f, 1.0f);
            mvp.setTranslation(core::vector3df(-1.0f, 1.0f, 0.0f));

            D3D11_MAPPED_SUBRESOURCE    mappedMatrix;
            if (SUCCEEDED(m_pID3DDeviceContext->Map(m_MatrixConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedMatrix)))
            {
                memcpy(mappedMatrix.pData, mvp.pointer(), sizeof(core::matrix4));
                m_pID3DDeviceContext->Unmap(m_MatrixConstantBuffer, 0);
            }

            m_pID3DDeviceContext->VSSetConstantBuffers(0, 1, &m_MatrixConstantBuffer);

            const u32       vertexBufferSize    = sizeof(vertices);
            const u32       indexBufferSize     = sizeof(indices);

            if (!m_TempVertexBuffer || m_TempVertexBufferSize < vertexBufferSize)
            {
                if (m_TempVertexBuffer)
                {
                    IRR_D3D11_BUFFER_RELEASE(m_TempVertexBuffer, "TempVertexBuffer");
                    m_TempVertexBuffer->Release();
                }

                D3D11_BUFFER_DESC    vbDesc;
                vbDesc.ByteWidth            = vertexBufferSize;
                vbDesc.Usage                = D3D11_USAGE_DYNAMIC;
                vbDesc.BindFlags            = D3D11_BIND_VERTEX_BUFFER;
                vbDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
                vbDesc.MiscFlags            = 0;
                vbDesc.StructureByteStride  = 0;

                if (FAILED(m_pID3DDevice->CreateBuffer(&vbDesc, 0, &m_TempVertexBuffer)))
                {
                    os::Printer::log("Failed to create vertex buffer", ELL_ERROR);
                    return;
                }

                IRR_D3D11_BUFFER_CREATE(m_TempVertexBuffer, "TempVertexBuffer");
                m_TempVertexBufferSize = vertexBufferSize;
            }

            D3D11_MAPPED_SUBRESOURCE    mappedVB;
            if (SUCCEEDED(m_pID3DDeviceContext->Map(m_TempVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVB)))
            {
                memcpy(mappedVB.pData, vertices, vertexBufferSize);
                m_pID3DDeviceContext->Unmap(m_TempVertexBuffer, 0);
            }
            else
            {
                os::Printer::log("Failed to map vertex buffer", ELL_ERROR);
                return;
            }

            if (!m_TempIndexBuffer || m_TempIndexBufferSize < indexBufferSize || m_TempIndexType != EIT_16BIT)
            {
                if (m_TempIndexBuffer)
                {
                    IRR_D3D11_BUFFER_RELEASE(m_TempIndexBuffer, "TempIndexBuffer");
                    m_TempIndexBuffer->Release();
                }

                D3D11_BUFFER_DESC    ibDesc;
                ibDesc.ByteWidth            = indexBufferSize;
                ibDesc.Usage                = D3D11_USAGE_DYNAMIC;
                ibDesc.BindFlags            = D3D11_BIND_INDEX_BUFFER;
                ibDesc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;
                ibDesc.MiscFlags            = 0;
                ibDesc.StructureByteStride  = 0;

                if (FAILED(m_pID3DDevice->CreateBuffer(&ibDesc, 0, &m_TempIndexBuffer)))
                {
                    os::Printer::log("Failed to create index buffer", ELL_ERROR);
                    return;
                }

                IRR_D3D11_BUFFER_CREATE(m_TempIndexBuffer, "TempIndexBuffer");
                m_TempIndexBufferSize   = indexBufferSize;
                m_TempIndexType         = EIT_16BIT;
            }

            D3D11_MAPPED_SUBRESOURCE    mappedIB;
            if (SUCCEEDED(m_pID3DDeviceContext->Map(m_TempIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIB)))
            {
                memcpy(mappedIB.pData, indices, indexBufferSize);
                m_pID3DDeviceContext->Unmap(m_TempIndexBuffer, 0);
            }
            else
            {
                os::Printer::log("Failed to map index buffer", ELL_ERROR);
                return;
            }

            ID3D11Buffer    *buffers[1] = { m_TempVertexBuffer };
            UINT            offsets[1]  = { 0 };
            UINT            strides[1]  = { sizeof(SRectVertex) };
            m_pID3DDeviceContext->IASetVertexBuffers(0, 1, buffers, strides, offsets);
            m_pID3DDeviceContext->IASetIndexBuffer(m_TempIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
            m_pID3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_pID3DDeviceContext->DrawIndexed(6, 0, 0);

#ifdef _IRR_DUMP_DRAW_CALLS_
            dumpDrawCall("draw2DRectangle");
#endif
        }


        void CD3D11Driver::draw2DLine(const core::position2d<s32> &start,
                                      const core::position2d<s32> &end, SColor color)
        {
            m_pID3DDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

            s32     xdiff   = (end.X - start.X);
            s32     ydiff   = (end.Y - start.Y);
            s32     xabs    = xdiff >= 0 ? xdiff : -xdiff;
            s32     yabs    = ydiff >= 0 ? ydiff : -ydiff;
            s32     xstep   = 1;
            s32     ystep   = 1;

            if (xabs < yabs)
            {
                s32    t;
                xdiff   = (end.X - start.X);
                ydiff   = (end.Y - start.Y);

                if (xdiff < 0)
                {
                    xdiff   = -xdiff;
                    xstep   = -xstep;
                }

                if (ydiff < 0)
                {
                    ydiff   = -ydiff;
                    ystep   = -ystep;
                }

                if (xdiff == 0)
                {
                    t = -1;
                }
                else
                {
                    t = (ydiff - xdiff) / (2 * xdiff);
                }

                s32     x   = start.X;
                s32     y   = start.Y;
                s32     err = ydiff - 2 * xdiff;

                for (s32 i = 0; i <= xdiff; ++i)
                {
                    drawPixel(x, y, color);

                    if (t > 0)
                    {
                        y   += ystep;
                        err -= 2 * xdiff;
                    }
                    else
                    {
                        t += ydiff - 2 * xdiff;
                    }

                    if (err > 0)
                    {
                        y   += ystep;
                        err -= 2 * xdiff;
                    }

                    err += ydiff;
                    x   += xstep;
                }
            }
            else
            {
                s32    t;
                xdiff   = (end.X - start.X);
                ydiff   = (end.Y - start.Y);

                if (xdiff < 0)
                {
                    xdiff   = -xdiff;
                    xstep   = -xstep;
                }

                if (ydiff < 0)
                {
                    ydiff   = -ydiff;
                    ystep   = -ystep;
                }

                if (ydiff == 0)
                {
                    t = -1;
                }
                else
                {
                    t = (xdiff - ydiff) / (2 * ydiff);
                }

                s32     x   = start.X;
                s32     y   = start.Y;
                s32     err = xdiff - 2 * ydiff;

                for (s32 i = 0; i <= ydiff; ++i)
                {
                    drawPixel(x, y, color);

                    if (t > 0)
                    {
                        x   += xstep;
                        err -= 2 * ydiff;
                    }
                    else
                    {
                        t += xdiff - 2 * ydiff;
                    }

                    if (err > 0)
                    {
                        x   += xstep;
                        err -= 2 * ydiff;
                    }

                    err += xdiff;
                    y   += ystep;
                }
            }
        }


        void CD3D11Driver::drawPixel(u32 x, u32 y, const SColor &color)
        {}


        void CD3D11Driver::draw3DLine(const core::vector3df &start, const core::vector3df &end, SColor color)
        {
            video::S3DVertex    vertices[2];

            vertices[0].Pos     = start;
            vertices[0].Color   = color;
            vertices[1].Pos     = end;
            vertices[1].Color   = color;
            u16    index[2] = { 0, 1 };

            drawVertexPrimitiveList(vertices, 2, index, 1, video::EVT_STANDARD, scene::EPT_LINES, EIT_16BIT);
        }


        const wchar_t* CD3D11Driver::getName() const
        {
            return L"Direct3D 11.0";
        }


        void CD3D11Driver::deleteAllDynamicLights()
        {
            for (u32 i = 0; i < Lights.size(); ++i)
                Lights[i].Position = core::vector3df(0, 0, 0);

            Lights.clear();
            m_LastSetLight    = -1;
        }


        s32 CD3D11Driver::addDynamicLight(const SLight &light)
        {
            if (Lights.size() >= 32)
                return -1;

            Lights.push_back(light);
            return Lights.size() - 1;
        }


        void CD3D11Driver::turnLightOn(s32 lightIndex, bool turnOn)
        {
            m_LastSetLight = turnOn ? lightIndex : -1;
        }


        u32 CD3D11Driver::getMaximalDynamicLightAmount() const
        {
            return 32;
        }


        void CD3D11Driver::setAmbientLight(const SColorf &color)
        {
            m_AmbientLight = color;
        }


        void CD3D11Driver::drawStencilShadowVolume(const core::array<core::vector3df> &triangles, bool zfail, u32 debugDataVisible)
        {}


        void CD3D11Driver::drawStencilShadow(bool clearStencilBuffer, video::SColor leftUpEdge, video::SColor rightUpEdge,
                                             video::SColor leftDownEdge, video::SColor rightDownEdge)
        {}


        u32 CD3D11Driver::getMaximalPrimitiveCount() const
        {
            return 65535;
        }


        void CD3D11Driver::setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled)
        {
            CNullDriver::setTextureCreationFlag(flag, enabled);
        }


        void CD3D11Driver::setFog(SColor color, E_FOG_TYPE fogType, f32 start, f32 end,
                                  f32 density, bool pixelFog, bool rangeFog)
        {}


        void CD3D11Driver::OnResize(const core::dimension2d<u32> &size)
        {
            ScreenSize = size;
        }


        E_DRIVER_TYPE CD3D11Driver::getDriverType() const
        {
            return EDT_DIRECT3D11;
        }


        const core::matrix4&CD3D11Driver::getTransform(E_TRANSFORMATION_STATE state) const
        {
            return m_Matrices[state];
        }


        void CD3D11Driver::setVertexShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount)
        {}


        void CD3D11Driver::setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount)
        {}


        bool CD3D11Driver::setVertexShaderConstant(const c8 *name, const f32 *floats, int count)
        {
            return false;
        }


        bool CD3D11Driver::setVertexShaderConstant(const c8 *name, const bool *bools, int count)
        {
            return false;
        }


        bool CD3D11Driver::setVertexShaderConstant(const c8 *name, const s32 *ints, int count)
        {
            return false;
        }


        bool CD3D11Driver::setPixelShaderConstant(const c8 *name, const f32 *floats, int count)
        {
            return false;
        }


        bool CD3D11Driver::setPixelShaderConstant(const c8 *name, const bool *bools, int count)
        {
            return false;
        }


        bool CD3D11Driver::setPixelShaderConstant(const c8 *name, const s32 *ints, int count)
        {
            return false;
        }


        IVideoDriver* CD3D11Driver::getVideoDriver()
        {
            return this;
        }


        ITexture* CD3D11Driver::addRenderTargetTexture(const core::dimension2d<u32> &size,
                                                       const io::path &name, const ECOLOR_FORMAT format)
        {
            return 0;
        }


        void CD3D11Driver::clearZBuffer()
        {
            m_pID3DDeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        }


        IImage* CD3D11Driver::createScreenShot(video::ECOLOR_FORMAT format, video::E_RENDER_TARGET target)
        {
            if (target != video::ERT_FRAME_BUFFER)
                return 0;

            if (format == video::ECOLOR_FORMAT::ECF_UNKNOWN)
                format = m_ColorFormat;

            ID3D11Texture2D     *backBuffer = 0;
            HRESULT             hr          = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
            if (FAILED(hr) || !backBuffer)
                return 0;

            D3D11_TEXTURE2D_DESC    desc;
            backBuffer->GetDesc(&desc);
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            desc.Usage          = D3D11_USAGE_STAGING;
            desc.BindFlags      = 0;

            ID3D11Texture2D    *stagingTexture = 0;
            hr = m_pID3DDevice->CreateTexture2D(&desc, 0, &stagingTexture);
            if (FAILED(hr) || !stagingTexture)
            {
                backBuffer->Release();
                return 0;
            }

            m_pID3DDeviceContext->CopyResource(stagingTexture, backBuffer);

            D3D11_MAPPED_SUBRESOURCE    mapped;
            hr = m_pID3DDeviceContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);
            if (FAILED(hr))
            {
                stagingTexture->Release();
                backBuffer->Release();
                return 0;
            }

            IImage    *image = createImage(format, core::dimension2d<u32>(desc.Width, desc.Height));
            if (!image)
            {
                m_pID3DDeviceContext->Unmap(stagingTexture, 0);
                stagingTexture->Release();
                backBuffer->Release();
                return 0;
            }

            u8    *pixels = (u8*)image->lock();
            if (pixels)
            {
                u32    bytesPerPixel = 4;

                switch (format)
                {
                    case ECOLOR_FORMAT::ECF_A1R5G5B5:
                    case ECOLOR_FORMAT::ECF_R5G6B5:
                        bytesPerPixel = 2;
                        break;

                    case ECOLOR_FORMAT::ECF_R8G8B8:
                        bytesPerPixel = 3;
                        break;

                    case ECOLOR_FORMAT::ECF_A8R8G8B8:
                    default:
                        bytesPerPixel = 4;
                        break;
                }

                const u32       rowPitch        = mapped.RowPitch;
                const u32       imageRowPitch   = image->getPitch();

                for (u32 y = 0; y < desc.Height; ++y)
                {
                    u8      *srcRow = (u8*)mapped.pData + y * rowPitch;
                    u8      *dstRow = pixels + y * imageRowPitch;
                    memcpy(dstRow, srcRow, desc.Width * bytesPerPixel);
                }
            }

            image->unlock();

            m_pID3DDeviceContext->Unmap(stagingTexture, 0);
            stagingTexture->Release();
            backBuffer->Release();

            return image;
        }


        bool CD3D11Driver::setClipPlane(u32 index, const core::plane3df &plane, bool enable)
        {
            return false;
        }


        void CD3D11Driver::enableClipPlane(u32 index, bool enable)
        {}


        void CD3D11Driver::enableMaterial2D(bool enable)
        {}


        void CD3D11Driver::removeDepthSurface(SD3D11DepthStencilView *depth)
        {
            for (u32 i = 0; i < m_DepthBuffers.size(); ++i)
            {
                if (m_DepthBuffers[i] == depth)
                {
                    depth->drop();
                    m_DepthBuffers.erase(i);
                    break;
                }
            }
        }


        ECOLOR_FORMAT CD3D11Driver::getColorFormat() const
        {
            return m_ColorFormat;
        }


        core::dimension2du CD3D11Driver::getMaxTextureSize() const
        {
            return core::dimension2du(16384, 16384);
        }


        DXGI_FORMAT CD3D11Driver::getDXGIFormatFromColorFormat(ECOLOR_FORMAT format) const
        {
            switch (format)
            {
                case ECOLOR_FORMAT::ECF_A1R5G5B5:
                    return DXGI_FORMAT_B5G5R5A1_UNORM;

                case ECOLOR_FORMAT::ECF_R5G6B5:
                    return DXGI_FORMAT_B5G6R5_UNORM;

                case ECOLOR_FORMAT::ECF_R8G8B8:
                    return DXGI_FORMAT_B8G8R8X8_UNORM;

                case ECOLOR_FORMAT::ECF_A8R8G8B8:
                    return DXGI_FORMAT_B8G8R8A8_UNORM;

                case ECOLOR_FORMAT::ECF_R16F:
                    return DXGI_FORMAT_R16_FLOAT;

                case ECOLOR_FORMAT::ECF_G16R16F:
                    return DXGI_FORMAT_R16G16_FLOAT;

                case ECOLOR_FORMAT::ECF_A16B16G16R16F:
                    return DXGI_FORMAT_R16G16B16A16_FLOAT;

                case ECOLOR_FORMAT::ECF_R32F:
                    return DXGI_FORMAT_R32_FLOAT;

                case ECOLOR_FORMAT::ECF_G32R32F:
                    return DXGI_FORMAT_R32G32_FLOAT;

                case ECOLOR_FORMAT::ECF_A32B32G32R32F:
                    return DXGI_FORMAT_R32G32B32A32_FLOAT;

                default:
                    return DXGI_FORMAT_B8G8R8A8_UNORM;
            }
        }


        ECOLOR_FORMAT CD3D11Driver::getColorFormatFromDXGIFormat(DXGI_FORMAT format) const
        {
            switch (format)
            {
                case DXGI_FORMAT_B5G5R5A1_UNORM:
                    return ECOLOR_FORMAT::ECF_A1R5G5B5;

                case DXGI_FORMAT_B5G6R5_UNORM:
                    return ECOLOR_FORMAT::ECF_R5G6B5;

                case DXGI_FORMAT_B8G8R8X8_UNORM:
                    return ECOLOR_FORMAT::ECF_R8G8B8;

                case DXGI_FORMAT_R16_FLOAT:
                    return ECOLOR_FORMAT::ECF_R16F;

                case DXGI_FORMAT_R16G16_FLOAT:
                    return ECOLOR_FORMAT::ECF_G16R16F;

                case DXGI_FORMAT_R16G16B16A16_FLOAT:
                    return ECOLOR_FORMAT::ECF_A16B16G16R16F;

                case DXGI_FORMAT_R32_FLOAT:
                    return ECOLOR_FORMAT::ECF_R32F;

                case DXGI_FORMAT_R32G32_FLOAT:
                    return ECOLOR_FORMAT::ECF_G32R32F;

                case DXGI_FORMAT_R32G32B32A32_FLOAT:
                    return ECOLOR_FORMAT::ECF_A32B32G32R32F;

                default:
                    return ECOLOR_FORMAT::ECF_A8R8G8B8;
            }
        }


        void CD3D11Driver::createMaterialRenderers()
        {
            s32    matType = -1;

            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "solid"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "solid_lightmap"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "solid_2_layer"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent_2_layer"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent_add_color"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent_vertex_alpha"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel_ref"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "one_texture_blend"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "lightmap_blend"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "detail_map"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "sphere_map"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "reflection_2_layer"));
            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, "transparent_reflection_2_layer"));

            if (queryFeature(video::EVDF_PIXEL_SHADER_1_1) && queryFeature(video::EVDF_VERTEX_SHADER_1_1))
            {
                m_MaterialRenderers.push_back(new CD3D11NormalMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID)));
                m_MaterialRenderers.push_back(new CD3D11ParallaxMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID)));
            }
        }


        D3D11_TEXTURE_ADDRESS_MODE CD3D11Driver::getTextureWrapMode(const u8 clamp) const
        {
            switch (clamp)
            {
                case ETC_REPEAT:
                    return D3D11_TEXTURE_ADDRESS_WRAP;

                case ETC_CLAMP:
                    return D3D11_TEXTURE_ADDRESS_CLAMP;

                case ETC_CLAMP_TO_EDGE:
                    return D3D11_TEXTURE_ADDRESS_CLAMP;

                case ETC_CLAMP_TO_BORDER:
                    return D3D11_TEXTURE_ADDRESS_BORDER;

                case ETC_MIRROR:
                    return D3D11_TEXTURE_ADDRESS_MIRROR;

                case ETC_MIRROR_CLAMP:
                    return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;

                case ETC_MIRROR_CLAMP_TO_EDGE:
                    return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;

                case ETC_MIRROR_CLAMP_TO_BORDER:
                    return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;

                default:
                    return D3D11_TEXTURE_ADDRESS_WRAP;
            }
        }


        CSampler* CD3D11Driver::getSampler(const SMaterialLayer &layer)
        {
            const u8        filterType  = layer.TrilinearFilter ? 2 : (layer.BilinearFilter ? 1 : 0);
            const u64       key         = (u64(layer.TextureWrapU) << 0) |
                                          (u64(layer.TextureWrapV) << 4) |
                                          (u64(filterType) << 8) |
                                          (u64(layer.AnisotropicFilter) << 12) |
                                          (u64(layer.LODBias + 128) << 20);

            core::map<u64, CSampler*>::Node    *node = m_SamplerPool.find(key);

            if (node)
                return node->getValue();

            CSampler    *sampler = new CSampler(this);

            D3D11_SAMPLER_DESC    desc;
            desc.AddressU       = getTextureWrapMode(layer.TextureWrapU);
            desc.AddressV       = getTextureWrapMode(layer.TextureWrapV);
            desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.MipLODBias     = layer.LODBias / 8.0f;
            desc.MaxAnisotropy  = layer.AnisotropicFilter > 0 ? layer.AnisotropicFilter : 1;
            desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
            desc.MinLOD         = -FLT_MAX;
            desc.MaxLOD         = FLT_MAX;

            if (layer.AnisotropicFilter > 0)
                desc.Filter = D3D11_FILTER_ANISOTROPIC;
            else if (layer.TrilinearFilter)
                desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            else if (layer.BilinearFilter)
                desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            else
                desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

            sampler->setSamplerKey(key);
            sampler->create(desc);

            m_SamplerPool.set(key, sampler);

            return sampler;
        }


        bool CD3D11Driver::reset()
        {
            return false;
        }


        video::ITexture* CD3D11Driver::createDeviceDependentTexture(IImage *surface, const io::path &name, void *mipmapData)
        {
            return new CD3D11Texture(surface, this, TextureCreationFlags, name, mipmapData);
        }


        void CD3D11Driver::checkDepthBuffer(ITexture *tex)
        {}


        s32 CD3D11Driver::addShaderMaterial(const c8 *vertexShaderProgram, const c8 *pixelShaderProgram,
                                            IShaderConstantSetCallBack *callback, E_MATERIAL_TYPE baseMaterial, s32 userData)
        {
            return -1;
        }


        s32 CD3D11Driver::addHighLevelShaderMaterial(
            const c8 *vertexShaderProgram, const c8 *vertexShaderEntryPointName,
            E_VERTEX_SHADER_TYPE vsCompileTarget, const c8 *pixelShaderProgram,
            const c8 *pixelShaderEntryPointName, E_PIXEL_SHADER_TYPE psCompileTarget,
            const c8 *geometryShaderProgram, const c8 *geometryShaderEntryPointName,
            E_GEOMETRY_SHADER_TYPE gsCompileTarget, scene::E_PRIMITIVE_TYPE inType,
            scene::E_PRIMITIVE_TYPE outType, u32 verticesOut,
            IShaderConstantSetCallBack *callback, E_MATERIAL_TYPE baseMaterial,
            s32 userData, E_GPU_SHADING_LANGUAGE shadingLang)
        {
            return -1;
        }


        void CD3D11Driver::setVSByVertexType(video::E_VERTEX_TYPE newType)
        {
            if (newType != m_LastVertexType || !m_BuiltInVSInitialized)
            {
                if (!m_BuiltInVSInitialized)
                {
                    for (u32 i = 0; i < EVT_2D_RECTANGLE; ++i)
                    {
                        createBuiltInVertexShader((E_VERTEX_TYPE)i);
                    }

                    // createRectangleShaders();

                    m_BuiltInVSInitialized = true;
                }

                if (newType >= 0 && newType <= EVT_2D_RECTANGLE && m_BuiltInVertexShader[newType])
                {
                    m_pID3DDeviceContext->VSSetShader(m_BuiltInVertexShader[newType], 0, 0);

                    if (m_InputLayout[newType])
                    {
                        m_pID3DDeviceContext->IASetInputLayout(m_InputLayout[newType]);
                    }
                }

                m_LastVertexType = newType;
            }
        }


        void CD3D11Driver::setPSByMaterialType(video::E_MATERIAL_TYPE materialType)
        {
            if (!m_MaterialPSInitialized)
            {
                for (u32 i = EMT_SOLID; i < EMT_MATERIAL_MAX; ++i)
                {
                    createMaterialPixelShader((E_MATERIAL_TYPE)i);
                }

                m_MaterialPSInitialized = true;
            }

            // material type is only used for changing shader.
            // The draw has the same material type but it has different textures.
            if (m_LastMaterialType != materialType)
            {
                m_LastMaterialType = materialType;

                if (materialType == EMT_SOLID && m_bHasTex == false)
                    materialType = EMT_SOLID_COLOR;

                if (materialType >= EMT_SOLID && materialType <= EMT_MATERIAL_MAX && m_BuiltInPixelShader[materialType])
                {
                    m_pID3DDeviceContext->PSSetShader(m_BuiltInPixelShader[materialType], 0, 0);
                }
                else
                {
                    const CD3D11Shader    *pShader = getShaderByTypes((E_VERTEX_TYPE)0, EDST_PIXEL, materialType);

                    if (pShader && pShader->getPixelShader())
                    {
                        m_pID3DDeviceContext->PSSetShader(pShader->getPixelShader(), 0, 0);
                    }
                }
            }

            setPSTextureAndSamplerState();
        }


        CD3D11Shader* CD3D11Driver::getShaderByTypes(video::E_VERTEX_TYPE vertexType, E_D3D11_SHADER_TYPE shaderType, E_MATERIAL_TYPE materialType) const
        {
            for (u32 i = 0; i < m_ShaderPool.size(); ++i)
            {
                if (shaderType == EDST_PIXEL)
                {
                    if (m_ShaderPool[i]->getShaderType() == shaderType &&
                        m_ShaderPool[i]->getMaterialType() == materialType)
                        return m_ShaderPool[i];
                }
                else
                {
                    if (m_ShaderPool[i]->getVertexType() == vertexType &&
                        m_ShaderPool[i]->getShaderType() == shaderType &&
                        m_ShaderPool[i]->getMaterialType() == materialType)
                        return m_ShaderPool[i];
                }
            }

            return 0;
        }


        void CD3D11Driver::setPSTextureAndSamplerState()
        {
            for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
            {
                if (m_CurrentTexture[i] != m_PreviousTexture[i] || m_CurrentSampler[i] != m_PreviousSampler[i])
                {
                    if (m_CurrentTexture[i])
                    {
                        CD3D11Texture               *tex    = static_cast<CD3D11Texture*>(const_cast<ITexture*>(m_CurrentTexture[i]));
                        ID3D11ShaderResourceView    *srv    = tex->getShaderResourceView();

                        m_pID3DDeviceContext->PSSetShaderResources(i, 1, &srv);

                        if (m_CurrentSampler[i])
                        {
                            ID3D11SamplerState    *pSampler = m_CurrentSampler[i]->getD3D11SamplerState();
                            m_pID3DDeviceContext->PSSetSamplers(i, 1, &pSampler);
                        }
                    }
                    else
                    {
                        ID3D11SamplerState          *pSampler   = nullptr;
                        ID3D11ShaderResourceView    *nullSrv    = 0;

                        m_pID3DDeviceContext->PSSetShaderResources(i, 1, &nullSrv);
                        m_pID3DDeviceContext->PSSetSamplers(i, 1, &pSampler);
                    }

                    m_PreviousTexture[i]    = m_CurrentTexture[i];
                    m_PreviousSampler[i]    = m_CurrentSampler[i];
                }
            }
        }


        bool CD3D11Driver::createBuiltInVertexShader(E_VERTEX_TYPE type)
        {
            const char    *shaderSource = 0;

            switch (type)
            {
                case EVT_STANDARD:
                    shaderSource = VERTEX_SHADER_STANDARD;
                    break;

                case EVT_2TCOORDS:
                    shaderSource = VERTEX_SHADER_2TCOORDS;
                    break;

                case EVT_TANGENTS:
                    shaderSource = VERTEX_SHADER_TANGENTS;
                    break;

                default:
                    return false;
            }

            CD3D11Shader    *shader = new CD3D11Shader(this);
            if (!shader->compile(EDST_VERTEX, shaderSource, "main", "vs_4_0"))
            {
                shader->drop();
                return false;
            }

            if (!shader->createVertexShader())
            {
                shader->drop();
                return false;
            }

            D3D11_INPUT_ELEMENT_DESC    *layout     = 0;
            u32                         numElements = 0;

            switch (type)
            {
                case EVT_STANDARD:
                {
                    static D3D11_INPUT_ELEMENT_DESC    standardLayout[] =
                    {
                        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
                    };
                    layout      = standardLayout;
                    numElements = 4;
                    break;
                }

                case EVT_2TCOORDS:
                {
                    static D3D11_INPUT_ELEMENT_DESC    twoTexLayout[] =
                    {
                        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
                    };
                    layout      = twoTexLayout;
                    numElements = 5;
                    break;
                }

                case EVT_TANGENTS:
                {
                    static D3D11_INPUT_ELEMENT_DESC    tangentLayout[] =
                    {
                        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
                    };
                    layout      = tangentLayout;
                    numElements = 6;
                    break;
                }

                default:
                    shader->drop();
                    return false;
            }

            if (!shader->createInputLayout(layout, numElements))
            {
                shader->drop();
                return false;
            }

            m_ShaderPool.push_back(shader);

            if (m_BuiltInVertexShader[type])
            {
                IRR_D3D11_VS_RELEASE(m_BuiltInVertexShader[type], "BuiltInVertexShader");
                m_BuiltInVertexShader[type]->Release();
            }

            m_BuiltInVertexShader[type] = shader->getVertexShader();
            IRR_D3D11_VS_CREATE(m_BuiltInVertexShader[type], "BuiltInVertexShader");
            m_BuiltInVertexShader[type]->AddRef();

            if (m_InputLayout[type])
            {
                IRR_D3D11_IL_RELEASE(m_InputLayout[type], "BuiltInInputLayout");
                m_InputLayout[type]->Release();
            }

            m_InputLayout[type] = shader->getInputLayout();
            IRR_D3D11_IL_CREATE(m_InputLayout[type], "BuiltInInputLayout");
            m_InputLayout[type]->AddRef();

            return true;
        }


        bool CD3D11Driver::createBuiltInPixelShader(E_VERTEX_TYPE type)
        {
            const char    *shaderSource = 0;

            switch (type)
            {
                case EVT_STANDARD:
                    shaderSource = PIXEL_SHADER_STANDARD;
                    break;

                case EVT_2TCOORDS:
                    shaderSource = PIXEL_SHADER_2TCOORDS;
                    break;

                case EVT_TANGENTS:
                    shaderSource = PIXEL_SHADER_TANGENTS;
                    break;

                default:
                    return false;
            }

            CD3D11Shader    *shader = new CD3D11Shader(this);
            if (!shader->compile(EDST_PIXEL, shaderSource, "main", "ps_4_0"))
            {
                shader->drop();
                return false;
            }

            if (!shader->createPixelShader())
            {
                shader->drop();
                return false;
            }

            m_ShaderPool.push_back(shader);

            if (m_BuiltInPixelShader[type])
            {
                IRR_D3D11_PS_RELEASE(m_BuiltInPixelShader[type], "BuiltInPixelShader");
                m_BuiltInPixelShader[type]->Release();
            }

            m_BuiltInPixelShader[type] = shader->getPixelShader();
            IRR_D3D11_PS_CREATE(m_BuiltInPixelShader[type], "BuiltInPixelShader");
            m_BuiltInPixelShader[type]->AddRef();

            return true;
        }


        bool CD3D11Driver::createMaterialPixelShader(E_MATERIAL_TYPE materialType)
        {
            const char    *entryPoint = 0;

            switch (materialType)
            {
                case EMT_SOLID:                                 entryPoint = "PS_SOLID"; break;

                case EMT_SOLID_2_LAYER:                         entryPoint = "PS_SOLID_2_LAYER"; break;

                case EMT_LIGHTMAP:                              entryPoint = "PS_LIGHTMAP"; break;

                case EMT_LIGHTMAP_ADD:                          entryPoint = "PS_LIGHTMAP_ADD"; break;

                case EMT_LIGHTMAP_M2:                           entryPoint = "PS_LIGHTMAP_M2"; break;

                case EMT_LIGHTMAP_M4:                           entryPoint = "PS_LIGHTMAP_M4"; break;

                case EMT_LIGHTMAP_LIGHTING:                     entryPoint = "PS_LIGHTMAP_LIGHTING"; break;

                case EMT_LIGHTMAP_LIGHTING_M2:                  entryPoint = "PS_LIGHTMAP_LIGHTING_M2"; break;

                case EMT_LIGHTMAP_LIGHTING_M4:                  entryPoint = "PS_LIGHTMAP_LIGHTING_M4"; break;

                case EMT_DETAIL_MAP:                            entryPoint = "PS_DETAIL_MAP"; break;

                case EMT_SPHERE_MAP:                            entryPoint = "PS_SPHERE_MAP"; break;

                case EMT_REFLECTION_2_LAYER:                    entryPoint = "PS_REFLECTION_2_LAYER"; break;

                case EMT_TRANSPARENT_ADD_COLOR:                 entryPoint = "PS_TRANSPARENT_ADD_COLOR"; break;

                case EMT_TRANSPARENT_ALPHA_CHANNEL:             entryPoint = "PS_TRANSPARENT_ALPHA_CHANNEL"; break;

                case EMT_TRANSPARENT_ALPHA_CHANNEL_REF:         entryPoint = "PS_TRANSPARENT_ALPHA_CHANNEL_REF"; break;

                case EMT_TRANSPARENT_VERTEX_ALPHA:              entryPoint = "PS_TRANSPARENT_VERTEX_ALPHA"; break;

                case EMT_TRANSPARENT_REFLECTION_2_LAYER:        entryPoint = "PS_TRANSPARENT_REFLECTION_2_LAYER"; break;

                case EMT_NORMAL_MAP_SOLID:                      entryPoint = "PS_NORMAL_MAP_SOLID"; break;

                case EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR:      entryPoint = "PS_NORMAL_MAP_TRANSPARENT_ADD_COLOR"; break;

                case EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA:   entryPoint = "PS_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA"; break;

                case EMT_PARALLAX_MAP_SOLID:                    entryPoint = "PS_PARALLAX_MAP_SOLID"; break;

                case EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR:    entryPoint = "PS_PARALLAX_MAP_TRANSPARENT_ADD_COLOR"; break;

                case EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA: entryPoint = "PS_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA"; break;

                case EMT_ONETEXTURE_BLEND:                      entryPoint = "PS_ONETEXTURE_BLEND"; break;

                case EMT_SOLID_COLOR:                           entryPoint = "PS_SOLID_COLOR_ONLY"; break;

                default:
                    return false;
            }

            CD3D11Shader    *shader = new CD3D11Shader(this);
            shader->setMaterialType(materialType);

            if (!shader->compile(EDST_PIXEL, PS_MaterialShaders, entryPoint, "ps_4_0"))
            {
                shader->drop();
                return false;
            }

            if (!shader->createPixelShader())
            {
                shader->drop();
                return false;
            }

            m_ShaderPool.push_back(shader);

            if (m_BuiltInPixelShader[materialType])
            {
                IRR_D3D11_PS_RELEASE(m_BuiltInPixelShader[materialType], "BuiltInPixelShader");
                m_BuiltInPixelShader[materialType]->Release();
            }

            m_BuiltInPixelShader[materialType] = shader->getPixelShader();
            IRR_D3D11_PS_CREATE(m_BuiltInPixelShader[materialType], "MaterialPixelShader");
            m_BuiltInPixelShader[materialType]->AddRef();

            return true;
        }


        bool CD3D11Driver::createInputLayout(E_VERTEX_TYPE type, ID3DBlob *shaderBlob)
        {
            D3D11_INPUT_ELEMENT_DESC    *layout     = 0;
            u32                         numElements = 0;

            switch (type)
            {
                case EVT_STANDARD:
                {
                    static D3D11_INPUT_ELEMENT_DESC    standardLayout[] =
                    {
                        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
                    };
                    layout      = standardLayout;
                    numElements = 4;
                    break;
                }

                case EVT_2TCOORDS:
                {
                    static D3D11_INPUT_ELEMENT_DESC    twoTexLayout[] =
                    {
                        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
                    };
                    layout      = twoTexLayout;
                    numElements = 5;
                    break;
                }

                case EVT_TANGENTS:
                {
                    static D3D11_INPUT_ELEMENT_DESC    tangentLayout[] =
                    {
                        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
                        {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
                    };
                    layout      = tangentLayout;
                    numElements = 6;
                    break;
                }

                default:
                    return false;
            }

            HRESULT    hr = m_pID3DDevice->CreateInputLayout(layout, numElements,
                                                             shaderBlob->GetBufferPointer(),
                                                             shaderBlob->GetBufferSize(),
                                                             &m_InputLayout[type]);
            return SUCCEEDED(hr);
        }


        bool CD3D11Driver::createRectangleShaders()
        {
            CD3D11Shader    *vsShader = new CD3D11Shader(this);

            if (!vsShader->compile(EDST_VERTEX, VERTEX_SHADER_RECTANGLE, "main", "vs_4_0"))
            {
                vsShader->drop();
                return false;
            }

            if (!vsShader->createVertexShader())
            {
                vsShader->drop();
                return false;
            }

            D3D11_INPUT_ELEMENT_DESC    rectangleLayout[] =
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            if (!vsShader->createInputLayout(rectangleLayout, 2))
            {
                vsShader->drop();
                return false;
            }

            vsShader->setVertexType(EVT_2D_RECTANGLE);
            m_ShaderPool.push_back(vsShader);

            if (m_RectangleVertexShader)
            {
                IRR_D3D11_VS_RELEASE(m_RectangleVertexShader, "RectangleVertexShader");
                m_RectangleVertexShader->Release();
            }

            m_RectangleVertexShader = vsShader->getVertexShader();
            IRR_D3D11_VS_CREATE(m_RectangleVertexShader, "RectangleVertexShader");
            m_RectangleVertexShader->AddRef();

            if (m_RectangleInputLayout)
            {
                IRR_D3D11_IL_RELEASE(m_RectangleInputLayout, "RectangleInputLayout");
                m_RectangleInputLayout->Release();
            }

            m_RectangleInputLayout = vsShader->getInputLayout();
            IRR_D3D11_IL_CREATE(m_RectangleInputLayout, "RectangleInputLayout");
            m_RectangleInputLayout->AddRef();

            CD3D11Shader    *psShader = new CD3D11Shader(this);

            if (!psShader->compile(EDST_PIXEL, PIXEL_SHADER_RECTANGLE, "main", "ps_4_0"))
            {
                psShader->drop();
                return false;
            }

            if (!psShader->createPixelShader())
            {
                psShader->drop();
                return false;
            }

            psShader->setVertexType(EVT_2D_RECTANGLE);
            m_ShaderPool.push_back(psShader);

            if (m_RectanglePixelShader)
            {
                IRR_D3D11_PS_RELEASE(m_RectanglePixelShader, "RectanglePixelShader");
                m_RectanglePixelShader->Release();
            }

            m_RectanglePixelShader = psShader->getPixelShader();
            IRR_D3D11_PS_CREATE(m_RectanglePixelShader, "RectanglePixelShader");
            m_RectanglePixelShader->AddRef();

            return true;
        }


        void CD3D11Driver::set2DRectangleShader()
        {
            if (!m_RectangleShaderInitialized)
            {
                for (u32 i = 0; i < EVT_2D_RECTANGLE; ++i)
                {
                    createBuiltInVertexShader((E_VERTEX_TYPE)i);
                }

                createRectangleShaders();

                m_RectangleShaderInitialized = true;
            }

            // If the last shaders are 2d rectangle, we set nothing.
            if (m_LastMaterialType == EMT_2D_RECTANGLE && m_LastVertexType == EVT_2D_RECTANGLE)
                return;

            // We have to set m_LastVertexType and m_LastMaterialType
            m_LastVertexType    = EVT_2D_RECTANGLE;
            m_LastMaterialType  = EMT_2D_RECTANGLE;

            if (m_RectangleVertexShader)
            {
                m_pID3DDeviceContext->VSSetShader(m_RectangleVertexShader, 0, 0);
            }

            if (m_RectangleInputLayout)
            {
                m_pID3DDeviceContext->IASetInputLayout(m_RectangleInputLayout);
            }

            if (m_RectanglePixelShader)
            {
                m_pID3DDeviceContext->PSSetShader(m_RectanglePixelShader, 0, 0);
            }

            setPSTextureAndSamplerState();
        }


        void CD3D11Driver::updateMatrixConstantBuffer()
        {
            core::matrix4    mvp = m_Matrices[ETS_PROJECTION] * m_Matrices[ETS_VIEW] * m_Matrices[ETS_WORLD];

            D3D11_MAPPED_SUBRESOURCE    mapped;

            if (SUCCEEDED(m_pID3DDeviceContext->Map(m_MatrixConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            {
                memcpy(mapped.pData, mvp.pointer(), sizeof(core::matrix4));
                m_pID3DDeviceContext->Unmap(m_MatrixConstantBuffer, 0);
            }

            m_pID3DDeviceContext->VSSetConstantBuffers(0, 1, &m_MatrixConstantBuffer);
        }


        u64 CD3D11Driver::createRenderStateKey2D(bool alpha, bool texture, bool alphaChannel)
        {
            return u64(ERM_2D) | (u64(alpha) << 4) | (u64(texture) << 8) | (u64(alphaChannel) << 12);
        }

        u64 CD3D11Driver::createRenderStateKey3D(const SMaterial &material)
        {
            return u64(ERM_3D) |
                   (u64(material.MaterialType) << 4) |
                   (u64(*(u32*)&material.MaterialTypeParam) << 12) |
                   (u64(*(u32*)&material.Thickness) << 20) |
                   (u64(material.ZBuffer) << 28) |
                   (u64(material.AntiAliasing) << 32) |
                   (u64(material.ColorMask) << 36) |
                   (u64(material.BlendOperation) << 40) |
                   (u64(material.PolygonOffsetFactor) << 44) |
                   (u64(material.PolygonOffsetDirection) << 48) |
                   (u64(material.Wireframe) << 52) |
                   (u64(material.PointCloud) << 53) |
                   (u64(material.ZWriteEnable) << 54) |
                   (u64(material.BackfaceCulling) << 55) |
                   (u64(material.FrontfaceCulling) << 56);
        }

        u64 CD3D11Driver::createRenderStateKeyOther(E_RENDER_MODE mode)
        {
            _IRR_DEBUG_BREAK_IF(false);
            return u64(mode) << 0;
        }


        CD3D11Driver::SRenderStateSet* CD3D11Driver::getOrCreateRenderStateSet2D(bool alpha, bool texture, bool alphaChannel)
        {
            const u64                                   key     = createRenderStateKey2D(alpha, texture, alphaChannel);
            core::map<u64, SRenderStateSet>::Node       *node   = m_RenderStateSets[ERM_2D].find(key);

            if (node)
                return &node->getValue();

            SRenderStateSet    stateSet;
            stateSet.Key                = key;
            stateSet.RasterizerState    = 0;
            stateSet.DepthStencilState  = 0;
            stateSet.BlendState         = 0;

            HRESULT                     hr = E_FAIL;
            D3D11_RASTERIZER_DESC1      rasterizerDesc;
            rasterizerDesc.AntialiasedLineEnable    = false;
            rasterizerDesc.CullMode                 = D3D11_CULL_NONE;
            rasterizerDesc.DepthBias                = D3D11_DEFAULT_DEPTH_BIAS;
            rasterizerDesc.DepthBiasClamp           = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
            rasterizerDesc.DepthClipEnable          = false;
            rasterizerDesc.FillMode                 = D3D11_FILL_SOLID;
            rasterizerDesc.ForcedSampleCount        = 0;
            rasterizerDesc.FrontCounterClockwise    = false;
            rasterizerDesc.MultisampleEnable        = false;
            rasterizerDesc.ScissorEnable            = false;
            rasterizerDesc.SlopeScaledDepthBias     = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

            if (m_pID3DDevice1)
                hr = m_pID3DDevice1->CreateRasterizerState1(&rasterizerDesc, &stateSet.RasterizerState);

            if (FAILED(hr))
            {
                os::Printer::log("Could not create rasterizer state.", ELL_ERROR);
                return 0;
            }

            IRR_D3D11_RS_CREATE(stateSet.RasterizerState, "RasterizerState");

            D3D11_DEPTH_STENCIL_DESC    depthStencilDesc;
            depthStencilDesc.DepthEnable                    = false;
            depthStencilDesc.DepthWriteMask                 = D3D11_DEPTH_WRITE_MASK_ALL;
            depthStencilDesc.DepthFunc                      = D3D11_COMPARISON_LESS;
            depthStencilDesc.StencilEnable                  = false;
            depthStencilDesc.StencilReadMask                = D3D11_DEFAULT_STENCIL_READ_MASK;
            depthStencilDesc.StencilWriteMask               = D3D11_DEFAULT_STENCIL_WRITE_MASK;
            depthStencilDesc.FrontFace.StencilFunc          = D3D11_COMPARISON_ALWAYS;
            depthStencilDesc.FrontFace.StencilDepthFailOp   = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.FrontFace.StencilFailOp        = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.FrontFace.StencilPassOp        = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilFunc           = D3D11_COMPARISON_ALWAYS;
            depthStencilDesc.BackFace.StencilDepthFailOp    = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilFailOp         = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilPassOp         = D3D11_STENCIL_OP_KEEP;

            hr = m_pID3DDevice->CreateDepthStencilState(&depthStencilDesc, &stateSet.DepthStencilState);
            IRR_D3D11_DSS_CREATE(stateSet.DepthStencilState, "DepthStencilState");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create depth stencil state.", ELL_ERROR);
                return 0;
            }

            D3D11_BLEND_DESC1    blendDesc;
            blendDesc.AlphaToCoverageEnable     = false;
            blendDesc.IndependentBlendEnable    = false;

            const bool    enableAlphaBlend = alpha || (alphaChannel && texture);

            for (u32 i = 0; i < 8; ++i)
            {
                blendDesc.RenderTarget[i].BlendEnable           = enableAlphaBlend;
                blendDesc.RenderTarget[i].LogicOpEnable         = false;
                blendDesc.RenderTarget[i].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
                blendDesc.RenderTarget[i].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[i].BlendOp               = D3D11_BLEND_OP_ADD;
                blendDesc.RenderTarget[i].SrcBlendAlpha         = D3D11_BLEND_ONE;
                blendDesc.RenderTarget[i].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[i].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
                blendDesc.RenderTarget[i].LogicOp               = D3D11_LOGIC_OP_NOOP;
                blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            }

            hr = E_FAIL;
            if (m_pID3DDevice1)
                hr = m_pID3DDevice1->CreateBlendState1(&blendDesc, &stateSet.BlendState);

            IRR_D3D11_BLEND_CREATE(stateSet.BlendState, "BlendState");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create blend state.", ELL_ERROR);
                return 0;
            }

            m_RenderStateSets[ERM_2D].set(key, stateSet);

            core::map<u64, SRenderStateSet>::Node    *pNode = m_RenderStateSets[ERM_2D].find(key);
            if (pNode == nullptr)
            {
                os::Printer::log("Could not find the render states.", ELL_ERROR);
                return 0;
            }

            return &pNode->getValue();
        }


        CD3D11Driver::SRenderStateSet* CD3D11Driver::getOrCreateRenderStateSet3D(const SMaterial &material)
        {
            const u64                                   key     = createRenderStateKey3D(material);
            core::map<u64, SRenderStateSet>::Node       *node   = m_RenderStateSets[ERM_3D].find(key);

            if (node)
                return &node->getValue();

            SRenderStateSet    stateSet;
            stateSet.Key                = key;
            stateSet.RasterizerState    = 0;
            stateSet.DepthStencilState  = 0;
            stateSet.BlendState         = 0;

            HRESULT                     hr = E_FAIL;
            D3D11_RASTERIZER_DESC1      rasterizerDesc;
            rasterizerDesc.AntialiasedLineEnable    = false;
            rasterizerDesc.CullMode                 = D3D11_CULL_BACK;
            rasterizerDesc.DepthBias                = D3D11_DEFAULT_DEPTH_BIAS;
            rasterizerDesc.DepthBiasClamp           = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
            rasterizerDesc.DepthClipEnable          = true;
            rasterizerDesc.FillMode                 = D3D11_FILL_SOLID;
            rasterizerDesc.ForcedSampleCount        = 0;
            rasterizerDesc.FrontCounterClockwise    = false;
            rasterizerDesc.MultisampleEnable        = false;
            rasterizerDesc.ScissorEnable            = false;
            rasterizerDesc.SlopeScaledDepthBias     = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

            if (material.Wireframe)
                rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

            if (material.BackfaceCulling)
                rasterizerDesc.CullMode = D3D11_CULL_BACK;
            else if (material.FrontfaceCulling)
                rasterizerDesc.CullMode = D3D11_CULL_FRONT;
            else
                rasterizerDesc.CullMode = D3D11_CULL_NONE;

            if (m_pID3DDevice1)
                hr = m_pID3DDevice1->CreateRasterizerState1(&rasterizerDesc, &stateSet.RasterizerState);

            if (FAILED(hr))
            {
                os::Printer::log("Could not create rasterizer state.", ELL_ERROR);
                return 0;
            }

            IRR_D3D11_RS_CREATE(stateSet.RasterizerState, "RasterizerState");

            D3D11_DEPTH_STENCIL_DESC    depthStencilDesc;
            depthStencilDesc.DepthEnable                    = (material.ZBuffer != ECFN_NEVER);
            depthStencilDesc.DepthWriteMask                 = material.ZWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
            depthStencilDesc.StencilEnable                  = false;
            depthStencilDesc.StencilReadMask                = D3D11_DEFAULT_STENCIL_READ_MASK;
            depthStencilDesc.StencilWriteMask               = D3D11_DEFAULT_STENCIL_WRITE_MASK;
            depthStencilDesc.FrontFace.StencilFunc          = D3D11_COMPARISON_ALWAYS;
            depthStencilDesc.FrontFace.StencilDepthFailOp   = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.FrontFace.StencilFailOp        = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.FrontFace.StencilPassOp        = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilFunc           = D3D11_COMPARISON_ALWAYS;
            depthStencilDesc.BackFace.StencilDepthFailOp    = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilFailOp         = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilPassOp         = D3D11_STENCIL_OP_KEEP;

            switch (material.ZBuffer)
            {
                case ECFN_NEVER:
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_NEVER;
                    break;

                case ECFN_LESSEQUAL:
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
                    break;

                case ECFN_EQUAL:
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
                    break;

                case ECFN_LESS:
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
                    break;

                case ECFN_GREATEREQUAL:
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
                    break;

                case ECFN_NOTEQUAL:
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_NOT_EQUAL;
                    break;

                case ECFN_GREATER:
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
                    break;

                case ECFN_ALWAYS:
                default:
                    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
                    break;
            }

            hr = m_pID3DDevice->CreateDepthStencilState(&depthStencilDesc, &stateSet.DepthStencilState);
            IRR_D3D11_DSS_CREATE(stateSet.DepthStencilState, "DepthStencilState");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create depth stencil state.", ELL_ERROR);
                return 0;
            }

            D3D11_BLEND_DESC1    blendDesc;
            blendDesc.AlphaToCoverageEnable     = false;
            blendDesc.IndependentBlendEnable    = false;

            bool    blendEnable = material.BlendOperation != EBO_NONE;

            if (material.MaterialType == EMT_TRANSPARENT_ADD_COLOR ||
                material.MaterialType == EMT_TRANSPARENT_ALPHA_CHANNEL ||
                material.MaterialType == EMT_TRANSPARENT_VERTEX_ALPHA ||
                material.MaterialType == EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR ||
                material.MaterialType == EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA ||
                material.MaterialType == EMT_TRANSPARENT_ALPHA_CHANNEL_REF ||
                material.MaterialType == EMT_TRANSPARENT_REFLECTION_2_LAYER)
            {
                blendEnable = true;
            }

            D3D11_BLEND_OP    blendOp = D3D11_BLEND_OP_ADD;
            if (blendEnable)
            {
                switch (material.BlendOperation)
                {
                    case EBO_SUBTRACT:
                        blendOp = D3D11_BLEND_OP_SUBTRACT;
                        break;

                    case EBO_REVSUBTRACT:
                        blendOp = D3D11_BLEND_OP_REV_SUBTRACT;
                        break;

                    case EBO_MIN:
                    case EBO_MIN_FACTOR:
                    case EBO_MIN_ALPHA:
                        blendOp = D3D11_BLEND_OP_MIN;
                        break;

                    case EBO_MAX:
                    case EBO_MAX_FACTOR:
                    case EBO_MAX_ALPHA:
                        blendOp = D3D11_BLEND_OP_MAX;
                        break;

                    default:
                        blendOp = D3D11_BLEND_OP_ADD;
                        break;
                }
            }

            for (u32 i = 0; i < 8; ++i)
            {
                blendDesc.RenderTarget[i].BlendEnable           = blendEnable;
                blendDesc.RenderTarget[i].LogicOpEnable         = false;
                blendDesc.RenderTarget[i].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
                blendDesc.RenderTarget[i].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[i].BlendOp               = blendOp;
                blendDesc.RenderTarget[i].SrcBlendAlpha         = D3D11_BLEND_ONE;
                blendDesc.RenderTarget[i].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[i].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
                blendDesc.RenderTarget[i].LogicOp               = D3D11_LOGIC_OP_NOOP;
                blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            }

            hr = E_FAIL;
            if (m_pID3DDevice1)
                hr = m_pID3DDevice1->CreateBlendState1(&blendDesc, &stateSet.BlendState);

            IRR_D3D11_BLEND_CREATE(stateSet.BlendState, "BlendState");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create blend state.", ELL_ERROR);
                return 0;
            }

            m_RenderStateSets[ERM_3D].set(key, stateSet);

            core::map<u64, SRenderStateSet>::Node    *pNode = m_RenderStateSets[ERM_3D].find(key);
            if (pNode == nullptr)
            {
                os::Printer::log("Could not find the render states.", ELL_ERROR);
                return 0;
            }

            return &pNode->getValue();
        }


        CD3D11Driver::SRenderStateSet* CD3D11Driver::getOrCreateRenderStateSetOther(E_RENDER_MODE mode)
        {
            const u64                                   key     = createRenderStateKeyOther(mode);
            core::map<u64, SRenderStateSet>::Node       *node   = m_RenderStateSets[mode].find(key);

            if (node)
                return &node->getValue();

            SRenderStateSet    stateSet;
            stateSet.Key                = key;
            stateSet.RasterizerState    = 0;
            stateSet.DepthStencilState  = 0;
            stateSet.BlendState         = 0;

            HRESULT                     hr = E_FAIL;
            D3D11_RASTERIZER_DESC1      rasterizerDesc;
            rasterizerDesc.AntialiasedLineEnable    = false;
            rasterizerDesc.CullMode                 = D3D11_CULL_BACK;
            rasterizerDesc.DepthBias                = D3D11_DEFAULT_DEPTH_BIAS;
            rasterizerDesc.DepthBiasClamp           = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
            rasterizerDesc.DepthClipEnable          = true;
            rasterizerDesc.FillMode                 = D3D11_FILL_SOLID;
            rasterizerDesc.ForcedSampleCount        = 0;
            rasterizerDesc.FrontCounterClockwise    = false;
            rasterizerDesc.MultisampleEnable        = false;
            rasterizerDesc.ScissorEnable            = false;
            rasterizerDesc.SlopeScaledDepthBias     = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;

            if (m_pID3DDevice1)
                hr = m_pID3DDevice1->CreateRasterizerState1(&rasterizerDesc, &stateSet.RasterizerState);

            if (FAILED(hr))
            {
                os::Printer::log("Could not create rasterizer state.", ELL_ERROR);
                return 0;
            }

            IRR_D3D11_RS_CREATE(stateSet.RasterizerState, "RasterizerState");

            D3D11_DEPTH_STENCIL_DESC    depthStencilDesc;
            depthStencilDesc.DepthEnable                    = true;
            depthStencilDesc.DepthWriteMask                 = D3D11_DEPTH_WRITE_MASK_ALL;
            depthStencilDesc.DepthFunc                      = D3D11_COMPARISON_LESS;
            depthStencilDesc.StencilEnable                  = false;
            depthStencilDesc.StencilReadMask                = D3D11_DEFAULT_STENCIL_READ_MASK;
            depthStencilDesc.StencilWriteMask               = D3D11_DEFAULT_STENCIL_WRITE_MASK;
            depthStencilDesc.FrontFace.StencilFunc          = D3D11_COMPARISON_ALWAYS;
            depthStencilDesc.FrontFace.StencilDepthFailOp   = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.FrontFace.StencilFailOp        = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.FrontFace.StencilPassOp        = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilFunc           = D3D11_COMPARISON_ALWAYS;
            depthStencilDesc.BackFace.StencilDepthFailOp    = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilFailOp         = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilPassOp         = D3D11_STENCIL_OP_KEEP;

            hr = m_pID3DDevice->CreateDepthStencilState(&depthStencilDesc, &stateSet.DepthStencilState);
            IRR_D3D11_DSS_CREATE(stateSet.DepthStencilState, "DepthStencilState");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create depth stencil state.", ELL_ERROR);
                return 0;
            }

            D3D11_BLEND_DESC1    blendDesc;
            blendDesc.AlphaToCoverageEnable     = false;
            blendDesc.IndependentBlendEnable    = false;

            for (u32 i = 0; i < 8; ++i)
            {
                blendDesc.RenderTarget[i].BlendEnable           = false;
                blendDesc.RenderTarget[i].LogicOpEnable         = false;
                blendDesc.RenderTarget[i].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
                blendDesc.RenderTarget[i].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[i].BlendOp               = D3D11_BLEND_OP_ADD;
                blendDesc.RenderTarget[i].SrcBlendAlpha         = D3D11_BLEND_ONE;
                blendDesc.RenderTarget[i].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[i].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
                blendDesc.RenderTarget[i].LogicOp               = D3D11_LOGIC_OP_NOOP;
                blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            }

            hr = E_FAIL;
            if (m_pID3DDevice1)
                hr = m_pID3DDevice1->CreateBlendState1(&blendDesc, &stateSet.BlendState);

            IRR_D3D11_BLEND_CREATE(stateSet.BlendState, "BlendState");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create blend state.", ELL_ERROR);
                return 0;
            }

            m_RenderStateSets[mode].set(key, stateSet);

            core::map<u64, SRenderStateSet>::Node    *pNode = m_RenderStateSets[mode].find(key);
            if (pNode == nullptr)
            {
                os::Printer::log("Could not find the render states.", ELL_ERROR);
                return 0;
            }

            return &pNode->getValue();
        }


        CD3D11Driver::SRenderStateSet* CD3D11Driver::getOrCreateRenderStateSet(
            E_RENDER_MODE mode, bool alpha, bool texture, bool alphaChannel, const SMaterial &material)
        {
            if (mode == ERM_2D)
                return getOrCreateRenderStateSet2D(alpha, texture, alphaChannel);
            else if (mode == ERM_3D)
                return getOrCreateRenderStateSet3D(material);
            else
                return getOrCreateRenderStateSetOther(mode);
        }


        void CD3D11Driver::setRenderStates(E_RENDER_MODE mode, bool alpha)
        {
            m_pID3DDeviceContext->RSSetViewports(1, &m_DefaultViewport);
            m_pID3DDeviceContext->RSSetScissorRects(1, &m_DefaultScissorRect);

            SRenderStateSet    *stateSet = getOrCreateRenderStateSet(mode, alpha, false, false, m_Material);
            if (!stateSet)
                return;

            m_pID3DDeviceContext->RSSetState(stateSet->RasterizerState);
            m_pID3DDeviceContext->OMSetDepthStencilState(stateSet->DepthStencilState, 0);

            FLOAT    blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            m_pID3DDeviceContext->OMSetBlendState(stateSet->BlendState, blendFactor, 0xFFFFFFFF);
        }


        void CD3D11Driver::setRenderStatesStencilFillMode(bool alpha)
        {
            m_CurrentRenderMode = ERM_STENCIL_FILL;
        }


        void CD3D11Driver::setRenderStatesStencilShadowMode(bool zfail, u32 debugDataVisible)
        {
            m_CurrentRenderMode = zfail ? ERM_SHADOW_VOLUME_ZFAIL : ERM_SHADOW_VOLUME_ZPASS;
        }


        IVideoDriver* createDirectX11Driver(const SIrrlichtCreationParameters &params,
                                            io::IFileSystem *io, HWND window)
        {
            const bool      pureSoftware    = false;
            CD3D11Driver    *dx11           = new CD3D11Driver(params, io);

            if (!dx11->initDriver(window, pureSoftware))
            {
                dx11->drop();
                dx11 = 0;
            }

            return dx11;
        }
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_