// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D11Driver.h"

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
            "    output.Pos = mul(float4(input.Pos, 1.0), WorldViewProj);"
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
            "    output.Pos = mul(float4(input.Pos, 1.0), WorldViewProj);"
            "    output.Color = input.Color;"
            "    output.TexCoord = input.TexCoord;"
            "    output.TexCoord2 = input.TexCoord2;"
            "    output.Normal = input.Normal;"
            "    return output;"
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
            "    output.Pos = mul(float4(input.Pos, 1.0), WorldViewProj);"
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
            "float4 main(PS_INPUT input) : SV_TARGET {"
            "    return input.Color;"
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
            m_BuiltInShadersInitialized(false),
            m_TempVertexBuffer(0), m_TempIndexBuffer(0), m_MatrixConstantBuffer(0),
            m_TempVertexBufferSize(0), m_TempIndexBufferSize(0),
            m_TempIndexType(EIT_16BIT),
            m_RasterizerState(0), m_DepthStencilState(0), m_BlendState(0), m_DefaultSampler(0),
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
                m_CurrentTexture[i]                 = 0;
                m_LastTextureMipMapsAvailable[i]    = false;
            }

            m_MaxLightDistance = sqrtf(FLT_MAX);

            for (u32 i = 0; i < 3; ++i)
            {
                m_InputLayout[i]            = 0;
                m_BuiltInVertexShader[i]    = 0;
                m_BuiltInPixelShader[i]     = 0;
            }

            m_DefaultSampler = new CSampler(this);
        }


        CSampler::CSampler(CD3D11Driver *driver)
            : m_Driver(driver), m_D3D11SamplerState(0),
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
                m_D3D11SamplerState->Release();
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
            return SUCCEEDED(hr);
        }


        CD3D11Driver::~CD3D11Driver()
        {
            deleteMaterialRenders();
            deleteAllTextures();
            removeAllOcclusionQueries();
            removeAllHardwareBuffers();

            for (u32 i = 0; i < m_DepthBuffers.size(); ++i)
            {
                m_DepthBuffers[i]->drop();
            }

            m_DepthBuffers.clear();

            for (u32 i = 0; i < 3; ++i)
            {
                if (m_InputLayout[i])
                    m_InputLayout[i]->Release();

                if (m_BuiltInVertexShader[i])
                    m_BuiltInVertexShader[i]->Release();

                if (m_BuiltInPixelShader[i])
                    m_BuiltInPixelShader[i]->Release();
            }

            if (m_TempVertexBuffer)
                m_TempVertexBuffer->Release();

            if (m_TempIndexBuffer)
                m_TempIndexBuffer->Release();

            if (m_MatrixConstantBuffer)
                m_MatrixConstantBuffer->Release();

            if (m_RasterizerState)
                m_RasterizerState->Release();

            if (m_DepthStencilState)
                m_DepthStencilState->Release();

            if (m_BlendState)
                m_BlendState->Release();

            if (m_DefaultSampler)
                m_DefaultSampler->drop();

            if (m_pID3DDeviceContext)
                m_pID3DDeviceContext->Release();

            if (m_pID3DDevice1)
                m_pID3DDevice1->Release();

            if (m_pID3DDevice)
                m_pID3DDevice->Release();

            if (m_SwapChain)
                m_SwapChain->Release();

            if (m_DXGIFactory)
                m_DXGIFactory->Release();

            if (m_Adapter)
                m_Adapter->Release();

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

#ifdef _DEBUG
            hr = m_pID3DDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_pID3D11Debug);
#endif

            hr = m_pID3DDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&m_pID3DDevice1);
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

            m_DXGIFactory->MakeWindowAssociation(hwnd, 0);

            ID3D11Texture2D    *backBuffer = 0;
            hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
            if (FAILED(hr))
            {
                os::Printer::log("Could not get back buffer.", ELL_ERROR);
                return false;
            }

            hr = m_pID3DDevice->CreateRenderTargetView(backBuffer, 0, &m_BackBufferRenderTargetView);
            backBuffer->Release();
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

            hr = m_pID3DDevice->CreateDepthStencilView(depthTexture, 0, &m_DepthStencilView);
            depthTexture->Release();
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
            if (FAILED(hr))
            {
                os::Printer::log("Could not create matrix constant buffer.", ELL_ERROR);
                return false;
            }

            if (!createDefaultStates())
            {
                os::Printer::log("Could not create default render states.", ELL_ERROR);
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

            m_pID3DDeviceContext->OMSetRenderTargets(1, &m_BackBufferRenderTargetView, m_DepthStencilView);
            m_pID3DDeviceContext->RSSetViewports(1, &m_Viewport);

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


        void CD3D11Driver::setMaterial(const SMaterial &material)
        {
            m_Material = material;
            OverrideMaterial.apply(m_Material);

            for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
            {
                setActiveTexture(i, material.getTexture(i));
            }

            setBasicRenderStates(material, m_LastMaterial, true);
            m_LastMaterial = material;
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

            if (m_CurrentTexture[stage] == texture)
                return true;

            if (texture)
            {
                if (texture->getDriverType() != EDT_DIRECT3D11)
                    return false;
            }

            m_CurrentTexture[stage] = texture;
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
            m_CurrentRenderMode = ERM_2D;
            setRenderStates(ERM_2D, alpha);
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
                    hwBufferD3D->vertexBuffer->Release();
                    hwBufferD3D->vertexBuffer = 0;
                }

                if (hwBufferD3D->indexBuffer)
                {
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

            setShadersByType(vType);

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
            setShadersByType(vType);

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
                    m_TempVertexBuffer->Release();

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
                        m_TempIndexBuffer->Release();

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
        }


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
        {}


        void CD3D11Driver::draw2DRectangle(const core::rect<s32> &pos,
                                           SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
                                           const core::rect<s32> *clip)
        {
            core::rect<s32>    clippedRect(pos);

            if (clip)
                clippedRect.clipAgainst(*clip);

            if (!clippedRect.isValid())
                return;

            core::position2d<s32>    pos2[4];
            pos2[0] = clippedRect.UpperLeftCorner;
            pos2[1] = clippedRect.LowerRightCorner;
            pos2[2] = core::position2d<s32>(clippedRect.LowerRightCorner.X, clippedRect.UpperLeftCorner.Y);
            pos2[3] = core::position2d<s32>(clippedRect.UpperLeftCorner.X, clippedRect.LowerRightCorner.Y);

            s32    indices[6] = { 0, 1, 2, 2, 1, 3 };

            setRenderStates2DMode(false, false, false);

            for (s32 i = 0; i < 4; ++i)
                drawPixel(pos2[i].X, pos2[i].Y, colorLeftUp);
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
            return 0;
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

            new CD3D11MaterialRenderer(this, matType, "solid");
            new CD3D11MaterialRenderer(this, matType, "solid_lightmap");
            new CD3D11MaterialRenderer(this, matType, "solid_2_layer");
            new CD3D11MaterialRenderer(this, matType, "translucent");
            new CD3D11MaterialRenderer(this, matType, "translucent_2_layer");
            new CD3D11MaterialRenderer(this, matType, "translucent_add_color");
            new CD3D11MaterialRenderer(this, matType, "translucent_vertex_alpha");
            new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel");
            new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel_ref");
            new CD3D11MaterialRenderer(this, matType, "one_texture_blend");
            new CD3D11MaterialRenderer(this, matType, "lightmap_blend");
            new CD3D11MaterialRenderer(this, matType, "detail_map");
            new CD3D11MaterialRenderer(this, matType, "sphere_map");
            new CD3D11MaterialRenderer(this, matType, "reflection_2_layer");
            new CD3D11MaterialRenderer(this, matType, "transparent_reflection_2_layer");

            if (queryFeature(video::EVDF_PIXEL_SHADER_1_1) && queryFeature(video::EVDF_VERTEX_SHADER_1_1))
            {
                new CD3D11NormalMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));
                new CD3D11ParallaxMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));
            }
        }


        D3D11_TEXTURE_ADDRESS_MODE CD3D11Driver::getTextureWrapMode(const u8 clamp) const
        {
            return D3D11_TEXTURE_ADDRESS_WRAP;
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


        void CD3D11Driver::setShadersByType(video::E_VERTEX_TYPE newType)
        {
            if (newType != m_LastVertexType || !m_BuiltInShadersInitialized)
            {
                if (!m_BuiltInShadersInitialized)
                {
                    for (u32 i = 0; i < 3; ++i)
                    {
                        createBuiltInVertexShader((E_VERTEX_TYPE)i);
                        createBuiltInPixelShader((E_VERTEX_TYPE)i);
                    }

                    m_BuiltInShadersInitialized = true;
                }

                if (newType >= 0 && newType < 3 && m_BuiltInVertexShader[newType])
                {
                    m_pID3DDeviceContext->VSSetShader(m_BuiltInVertexShader[newType], 0, 0);
                    if (m_BuiltInPixelShader[newType])
                    {
                        m_pID3DDeviceContext->PSSetShader(m_BuiltInPixelShader[newType], 0, 0);
                    }

                    if (m_InputLayout[newType])
                    {
                        m_pID3DDeviceContext->IASetInputLayout(m_InputLayout[newType]);
                    }
                }

                m_LastVertexType = newType;
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

            ID3DBlob    *shaderBlob = 0;
            ID3DBlob    *errorBlob  = 0;

            HRESULT    hr = D3DCompile(shaderSource, strlen(shaderSource), 0, 0, 0, "main",
                                       "vs_4_0", D3DCOMPILE_SKIP_VALIDATION, 0, &shaderBlob, &errorBlob);

            if (FAILED(hr))
            {
                if (errorBlob)
                {
                    os::Printer::log("Vertex shader compilation failed:", ELL_ERROR);
                    os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);
                    errorBlob->Release();
                }

                return false;
            }

            if (shaderBlob)
            {
                hr = m_pID3DDevice->CreateVertexShader(shaderBlob->GetBufferPointer(),
                                                       shaderBlob->GetBufferSize(),
                                                       nullptr,
                                                       &m_BuiltInVertexShader[type]);
                if (SUCCEEDED(hr))
                {
                    createInputLayout(type, shaderBlob);
                }

                shaderBlob->Release();
            }

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

            ID3DBlob    *shaderBlob = 0;
            ID3DBlob    *errorBlob  = 0;

            HRESULT    hr = D3DCompile(shaderSource, strlen(shaderSource), 0, 0, 0, "main",
                                       "ps_4_0", D3DCOMPILE_SKIP_VALIDATION, 0, &shaderBlob, &errorBlob);

            if (FAILED(hr))
            {
                if (errorBlob)
                {
                    os::Printer::log("Pixel shader compilation failed:", ELL_ERROR);
                    os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);
                    errorBlob->Release();
                }

                return false;
            }

            if (shaderBlob)
            {
                hr = m_pID3DDevice->CreatePixelShader(shaderBlob->GetBufferPointer(),
                                                      shaderBlob->GetBufferSize(),
                                                      nullptr,
                                                      &m_BuiltInPixelShader[type]);
                shaderBlob->Release();
            }

            return SUCCEEDED(hr);
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


        void CD3D11Driver::updateMatrixConstantBuffer()
        {
            core::matrix4    mvp = m_Matrices[ETS_WORLD] * m_Matrices[ETS_VIEW] * m_Matrices[ETS_PROJECTION];

            D3D11_MAPPED_SUBRESOURCE    mapped;

            if (SUCCEEDED(m_pID3DDeviceContext->Map(m_MatrixConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            {
                memcpy(mapped.pData, mvp.pointer(), sizeof(core::matrix4));
                m_pID3DDeviceContext->Unmap(m_MatrixConstantBuffer, 0);
            }

            m_pID3DDeviceContext->VSSetConstantBuffers(0, 1, &m_MatrixConstantBuffer);
        }


        bool CD3D11Driver::createDefaultStates()
        {
            D3D11_RASTERIZER_DESC1    rasterizerDesc;

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

            HRESULT    hr = E_FAIL;
            if (m_pID3DDevice1)
                hr = m_pID3DDevice1->CreateRasterizerState1(&rasterizerDesc, &m_RasterizerState);

            if (FAILED(hr))
                return false;

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

            hr = m_pID3DDevice->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
            if (FAILED(hr))
                return false;

            D3D11_BLEND_DESC1    blendDesc;
            blendDesc.AlphaToCoverageEnable     = false;
            blendDesc.IndependentBlendEnable    = false;

            for (u32 i = 0; i < 8; ++i)
            {
                blendDesc.RenderTarget[i].BlendEnable           = true;
                blendDesc.RenderTarget[i].BlendOp               = D3D11_BLEND_OP_ADD;
                blendDesc.RenderTarget[i].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
                blendDesc.RenderTarget[i].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[i].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
                blendDesc.RenderTarget[i].SrcBlendAlpha         = D3D11_BLEND_ONE;
                blendDesc.RenderTarget[i].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
                blendDesc.RenderTarget[i].LogicOpEnable         = false;
                blendDesc.RenderTarget[i].LogicOp               = D3D11_LOGIC_OP_NOOP;
                blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            }

            hr = E_FAIL;
            if (m_pID3DDevice1)
                hr = m_pID3DDevice1->CreateBlendState1(&blendDesc, &m_BlendState);

            if (FAILED(hr))
                return false;

            m_DefaultSampler = new CSampler(this);
            if (!m_DefaultSampler->createDefault())
                return false;

            return true;
        }


        void CD3D11Driver::setRenderStates(E_RENDER_MODE mode, bool alpha)
        {
            m_pID3DDeviceContext->RSSetViewports(1, &m_DefaultViewport);
            m_pID3DDeviceContext->RSSetScissorRects(1, &m_DefaultScissorRect);
            m_pID3DDeviceContext->RSSetState(m_RasterizerState);
            m_pID3DDeviceContext->OMSetDepthStencilState(m_DepthStencilState, 0);

            FLOAT    blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            if (alpha)
            {
                D3D11_BLEND_DESC1    blendDesc;
                blendDesc.AlphaToCoverageEnable     = false;
                blendDesc.IndependentBlendEnable    = false;

                for (u32 i = 0; i < 8; ++i)
                {
                    blendDesc.RenderTarget[i].BlendEnable           = true;
                    blendDesc.RenderTarget[i].BlendOp               = D3D11_BLEND_OP_ADD;
                    blendDesc.RenderTarget[i].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
                    blendDesc.RenderTarget[i].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
                    blendDesc.RenderTarget[i].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
                    blendDesc.RenderTarget[i].SrcBlendAlpha         = D3D11_BLEND_ONE;
                    blendDesc.RenderTarget[i].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
                    blendDesc.RenderTarget[i].LogicOpEnable         = false;
                    blendDesc.RenderTarget[i].LogicOp               = D3D11_LOGIC_OP_NOOP;
                    blendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
                }

                ID3D11BlendState1    *alphaBlendState = 0;
                if (m_pID3DDevice1 && SUCCEEDED(m_pID3DDevice1->CreateBlendState1(&blendDesc, &alphaBlendState)))
                {
                    m_pID3DDeviceContext->OMSetBlendState(alphaBlendState, blendFactor, 0xFFFFFFFF);
                    alphaBlendState->Release();
                }
                else
                {
                    m_pID3DDeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xFFFFFFFF);
                }
            }
            else
            {
                m_pID3DDeviceContext->OMSetBlendState(m_BlendState, blendFactor, 0xFFFFFFFF);
            }

            ID3D11SamplerState    *samplerState = m_DefaultSampler->getD3D11SamplerState();
            m_pID3DDeviceContext->PSSetSamplers(0, 1, &samplerState);
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