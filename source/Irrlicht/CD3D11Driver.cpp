// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D11Driver.h"

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include "os.h"
#include "S3DVertex.h"
#include "CD3D11Texture.h"
#include "CD3D11MaterialRenderer.h"
#include "SIrrCreationParameters.h"

namespace irr
{
    namespace video
    {
        CD3D11Driver::CD3D11Driver(const SIrrlichtCreationParameters &params, io::IFileSystem *io)
            : CNullDriver(io, params.WindowSize), CurrentRenderMode(ERM_NONE),
            ResetRenderStates(true), Transformation3DChanged(false),
            D3D11Library(0), DXGIFactory(0), Adapter(0), pID3DDevice(0), pID3DDeviceContext(0), SwapChain(0),
            BackBufferRenderTargetView(0), DepthStencilView(0),
            WindowId(0), SceneSourceRect(0),
            LastVertexType((video::E_VERTEX_TYPE)-1), VendorID(0),
            MaxTextureUnits(0), MaxUserClipPlanes(0), MaxMRTs(1), NumSetMRTs(1),
            MaxLightDistance(0.f), LastSetLight(-1),
            ColorFormat(ECOLOR_FORMAT::ECF_A8R8G8B8), DeviceRemoved(false),
            DriverWasReset(true), OcclusionQuerySupport(false),
            AlphaToCoverageSupport(false), Params(params)
        {
#ifdef _DEBUG
            setDebugName("CD3D11Driver");
#endif

            printVersion();

            for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
            {
                CurrentTexture[i]               = 0;
                LastTextureMipMapsAvailable[i]  = false;
            }

            MaxLightDistance = sqrtf(FLT_MAX);
        }


        CD3D11Driver::~CD3D11Driver()
        {
            deleteMaterialRenders();
            deleteAllTextures();
            removeAllOcclusionQueries();
            removeAllHardwareBuffers();

            for (u32 i = 0; i < DepthBuffers.size(); ++i)
            {
                DepthBuffers[i]->drop();
            }

            DepthBuffers.clear();

            if (pID3DDeviceContext)
                pID3DDeviceContext->Release();

            if (pID3DDevice)
                pID3DDevice->Release();

            if (SwapChain)
                SwapChain->Release();

            if (DXGIFactory)
                DXGIFactory->Release();

            if (Adapter)
                Adapter->Release();

            if (D3D11Library)
                FreeLibrary(D3D11Library);
        }


        bool CD3D11Driver::initDriver(HWND hwnd, bool pureSoftware)
        {
            WindowId = hwnd;

            D3D11Library = LoadLibraryA("d3d11.dll");
            if (!D3D11Library)
            {
                os::Printer::log("Could not load d3d11.dll.", ELL_ERROR);
                return false;
            }

            typedef HRESULT (WINAPI * PFN_D3D11CreateDevice)(IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT, CONST D3D_FEATURE_LEVEL*, UINT, UINT, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext**);
            PFN_D3D11CreateDevice    D3D11CreateDevice = (PFN_D3D11CreateDevice)GetProcAddress(D3D11Library, "D3D11CreateDevice");
            if (!D3D11CreateDevice)
            {
                os::Printer::log("Could not find D3D11CreateDevice.", ELL_ERROR);
                return false;
            }

            D3D_FEATURE_LEVEL       featureLevel;
            HRESULT                 hr = D3D11CreateDevice(
                0,
                D3D_DRIVER_TYPE_HARDWARE,
                0,
                0,
                0,
                0,
                D3D11_SDK_VERSION,
                &pID3DDevice,
                &featureLevel,
                &pID3DDeviceContext);

            if (FAILED(hr))
            {
                os::Printer::log("Could not create D3D11 device.", ELL_ERROR);
                return false;
            }

            #ifdef _DEBUG
            hr = pID3DDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&pID3D11Debug);
            #endif

            pID3DDevice->CheckFormatSupport(DXGI_FORMAT_D24_UNORM_S8_UINT, &Caps);

            createMaterialRenderers();

            core::dimension2d<u32>    dim = Params.WindowSize;
            if (Params.Fullscreen)
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

            SwapChainBufferDesc.Width                   = currentDim.Width;
            SwapChainBufferDesc.Height                  = currentDim.Height;
            SwapChainBufferDesc.RefreshRate.Numerator   = 60;
            SwapChainBufferDesc.RefreshRate.Denominator = 1;
            SwapChainBufferDesc.Format                  = DXGI_FORMAT_B8G8R8A8_UNORM;
            SwapChainBufferDesc.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            SwapChainBufferDesc.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;

            SwapChainDesc.BufferDesc            = SwapChainBufferDesc;
            SwapChainDesc.SampleDesc.Count      = 1;
            SwapChainDesc.SampleDesc.Quality    = 0;
            SwapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            SwapChainDesc.BufferCount           = 1;
            SwapChainDesc.OutputWindow          = hwnd;
            SwapChainDesc.Windowed              = !Params.Fullscreen;
            SwapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_DISCARD;
            SwapChainDesc.Flags                 = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

            hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&DXGIFactory);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create DXGIFactory.", ELL_ERROR);
                return false;
            }

            hr = DXGIFactory->EnumAdapters1(0, &Adapter);
            if (FAILED(hr))
            {
                os::Printer::log("Could not enumerate adapters.", ELL_ERROR);
                return false;
            }

            DXGI_ADAPTER_DESC1    desc;
            Adapter->GetDesc1(&desc);
            VendorID    = desc.SubSysId;
            VendorName  = core::stringc("");

            hr = DXGIFactory->CreateSwapChain(pID3DDevice, &SwapChainDesc, &SwapChain);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create swap chain.", ELL_ERROR);
                return false;
            }

            DXGIFactory->MakeWindowAssociation(hwnd, 0);

            ID3D11Texture2D *backBuffer = 0;
            hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
            if (FAILED(hr))
            {
                os::Printer::log("Could not get back buffer.", ELL_ERROR);
                return false;
            }

            hr = pID3DDevice->CreateRenderTargetView(backBuffer, 0, &BackBufferRenderTargetView);
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

            ID3D11Texture2D *depthTexture = 0;
            hr = pID3DDevice->CreateTexture2D(&depthDesc, 0, &depthTexture);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create depth stencil texture.", ELL_ERROR);
                return false;
            }

            hr = pID3DDevice->CreateDepthStencilView(depthTexture, 0, &DepthStencilView);
            depthTexture->Release();
            if (FAILED(hr))
            {
                os::Printer::log("Could not create depth stencil view.", ELL_ERROR);
                return false;
            }

            Viewport.TopLeftX   = 0;
            Viewport.TopLeftY   = 0;
            Viewport.Width      = (FLOAT)currentDim.Width;
            Viewport.Height     = (FLOAT)currentDim.Height;
            Viewport.MinDepth   = 0.0f;
            Viewport.MaxDepth   = 1.0f;

            CurrentRendertargetSize = currentDim;
            Rectu32    driverInitArea(0, 0, currentDim.Width, currentDim.Height);
            setViewPort(driverInitArea);

            mcp = core::matrix4();
            setTransform(ETS_VIEW, mcp);
            setTransform(ETS_PROJECTION, mcp);
            setTransform(ETS_MODEL, mcp);

            return true;
        }


        bool CD3D11Driver::beginScene(bool backBuffer, bool zBuffer, SColor color,
                                      const SExposedVideoData &videoData, core::rect<s32> *sourceRect)
        {
            if (DeviceRemoved)
            {
                HRESULT    hr = pID3DDevice->GetDeviceRemovedReason();
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

            if (backBuffer || zBuffer)
            {
                UINT    flags = 0;
                if (!backBuffer)
                    flags |= D3D11_CLEAR_DEPTH;

                if (!zBuffer)
                    flags |= D3D11_CLEAR_STENCIL;

                if (flags != 0)
                {
                    FLOAT       depth   = 1.0f;
                    UINT8       stencil = 0;
                    pID3DDeviceContext->ClearDepthStencilView(DepthStencilView, flags, depth, stencil);
                }

                if (backBuffer)
                {
                    FLOAT    colorF[4];
                    colorToD3D(color, colorF);
                    pID3DDeviceContext->ClearRenderTargetView(BackBufferRenderTargetView, colorF);
                }
            }

            pID3DDeviceContext->OMSetRenderTargets(1, &BackBufferRenderTargetView, DepthStencilView);
            pID3DDeviceContext->RSSetViewports(1, &Viewport);

            SceneSourceRect = sourceRect;
            return true;
        }


        bool CD3D11Driver::endScene()
        {
            HRESULT    hr = SwapChain->Present(Params.Vsync ? 1 : 0, 0);

            DeviceRemoved = (hr == DXGI_ERROR_DEVICE_REMOVED);
            if (DeviceRemoved && !reset())
                return false;

            return true;
        }


        bool CD3D11Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const
        {
            switch (feature)
            {
                case EVDF_RENDER_TARGET:
                    return true;

                case EVDF_MULTITEXTURE:
                    return true;

                case EVDF_BILINEAR_FILTERING:
                    return true;

                case EVDF_MIPMAP:
                    return true;

                case EVDF_MIPMAP_AUTO_UPDATE:
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

                case EVDF_TEXTURE_NSCRIBE:
                    return true;

                case EVDF_STENCIL_BUFFER:
                    return true;

                case EVDF_ALPHA_TO_COVERAGE:
                    return AlphaToCoverageSupport;

                case EVDF_COLOR_BUFFER:
                    return true;

                case EVDF_DEPTH_BUFFER:
                    return true;

                case EVDFstencil_BUFFER:
                    return true;

                case EVDF_W_BUFFER:
                    return false;

                case EVDF_GEOMETRY_SHADER:
                    return true;

                case EVDF_OCCLUSION_QUERY:
                    return OcclusionQuerySupport;

                case EVDF_NIVERSE_CULLING:
                    return true;

                case EVDF_POLYGON_OFFSET:
                    return true;

                case EVDF_BLEND_OPERATIONS:
                    return true;

                case EVDF_BLEND_SEPARATE:
                    return true;

                case EVDF_TEXTURE_MIRROR:
                    return true;

                case EVDF_TEXTURE_WRAP:
                    return true;

                case EVDF_STEREO:
                    return false;

                case EVDF_COMPUTE_SHADER:
                    return false;

                case EVDF_MRT_INDEPNT_BIT_DEPTH:
                    return false;

                case EVDF_MRT_AUTOMSRT_BIND:
                    return false;

                case EVDF_EVALUATOR:
                    return true;

                case EVDF_POINT_SPRITE:
                    return true;

                case EVDF_VIRTUAL_COORDINATE:
                    return true;
            }

            return false;
        }


        void CD3D11Driver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4 &mat)
        {
            Matrices[state] = mat;
            if (state == ETS_MODEL)
                Transformation3DChanged = true;
        }


        void CD3D11Driver::setMaterial(const SMaterial &material)
        {
            Material = material;

            for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
            {
                setActiveTexture(i, material.getTexture(i));
            }

            setBasicRenderStates(material, LastMaterial, true);
            LastMaterial = material;
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
            core::rect<s32>             vp          = area;
            core::dimension2d<u32>      screenDim   = ScreenSize;

            vp.clip(screenDim);

            D3D11_VIEWPORT    vpD3D;
            vpD3D.TopLeftX  = (FLOAT)vp.UpperLeftCorner.X;
            vpD3D.TopLeftY  = (FLOAT)vp.UpperLeftCorner.Y;
            vpD3D.Width     = (FLOAT)vp.getWidth();
            vpD3D.Height    = (FLOAT)vp.getHeight();
            vpD3D.MinDepth  = 0.0f;
            vpD3D.MaxDepth  = 1.0f;

            Viewport = vpD3D;

            ViewPort = vp;
            pID3DDeviceContext->RSSetViewports(1, &Viewport);
        }


        const core::rect<s32>&CD3D11Driver::getViewPort() const
        {
            return ViewPort;
        }


        bool CD3D11Driver::setActiveTexture(u32 stage, const video::ITexture *texture)
        {
            if (stage >= MATERIAL_MAX_TEXTURES)
                return false;

            if (texture)
            {
                if (texture->getDriverType() != EDT_DIRECT3D11)
                    return false;
            }

            CurrentTexture[stage] = texture;
            return true;
        }


        const core::dimension2d<u32>&CD3D11Driver::getCurrentRenderTargetSize() const
        {
            return CurrentRendertargetSize;
        }


        void CD3D11Driver::setBasicRenderStates(const SMaterial &material, const SMaterial &lastMaterial,
                                                bool resetAllRenderstates)
        {
            if (resetAllRenderstates || lastMaterial.Wireframe != material.Wireframe)
            {
                pID3DDeviceContext->RSSetState(0);
            }

            if (resetAllRenderstates || lastMaterial.GouraudShading != material.GouraudShading)
            {}

            if (resetAllRenderstates || lastMaterial.Lighting != material.Lighting)
            {}

            if (resetAllRenderstates || lastMaterial.ZWriteEnable != material.ZWriteEnable)
            {
                pID3DDeviceContext->OMSetDepthStencilState(0, 0);
            }

            if (resetAllRenderstates || lastMaterial.FogEnable != material.FogEnable)
            {}
        }


        bool CD3D11Driver::setRenderStates3DMode()
        {
            if (CurrentRenderMode == ERM_3D)
                return true;

            CurrentRenderMode = ERM_3D;
            return true;
        }


        void CD3D11Driver::setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel)
        {
            CurrentRenderMode = ERM_2D;
        }


        bool CD3D11Driver::updateVertexHardwareBuffer(SHWBufferLink_d3d11 *hwBuffer)
        {
            return false;
        }


        bool CD3D11Driver::updateIndexHardwareBuffer(SHWBufferLink_d3d11 *hwBuffer)
        {
            return false;
        }


        bool CD3D11Driver::updateHardwareBuffer(SHWBufferLink *hwBuffer)
        {
            return false;
        }


        SHWBufferLink* CD3D11Driver::createHardwareBuffer(const scene::IMeshBuffer *mb)
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
        {}


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
            setRenderStates3DMode();

            if (CurrentTexture[0])
                setActiveTexture(0, CurrentTexture[0]);

            if (!is3D)
            {
                pID3DDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            }
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
                clippedRect.clip(*clip);

            if (clippedRect.isEmpty())
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
            pID3DDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

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
            video::SVertex3D    vertices[2];

            vertices[0].Pos     = start;
            vertices[0].Color   = color;
            vertices[1].Pos     = end;
            vertices[1].Color   = color;
            u16    index[2] = { 0, 1 };

            drawVertexPrimitiveList(vertices, 2, index, 1, video::EVT_STANDARD, scene::EPT_LINE_LIST, EIT_16BIT);
        }


        const wchar_t* CD3D11Driver::getName() const
        {
            return L"Direct3D 11.0";
        }


        void CD3D11Driver::deleteAllDynamicLights()
        {
            for (u32 i = 0; i < MaxLightDistance; ++i)
                Lights[i].position = core::vector3df(0, 0, 0);

            LightCount      = 0;
            LastSetLight    = -1;
        }


        s32 CD3D11Driver::addDynamicLight(const SLight &light)
        {
            if (LightCount >= MaxDynamicLights)
                return -1;

            Lights[LightCount] = light;
            return LightCount++;
        }


        void CD3D11Driver::turnLightOn(s32 lightIndex, bool turnOn)
        {
            LastSetLight = turnOn ? lightIndex : -1;
        }


        u32 CD3D11Driver::getMaximalDynamicLightAmount() const
        {
            return MaxDynamicLights;
        }


        void CD3D11Driver::setAmbientLight(const SColorf &color)
        {
            AmbientLight = color;
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
            return Matrices[state];
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
            pID3DDeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
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
            for (u32 i = 0; i < DepthBuffers.size(); ++i)
            {
                if (DepthBuffers[i] == depth)
                {
                    depth->drop();
                    DepthBuffers.erase(i);
                    break;
                }
            }
        }


        ECOLOR_FORMAT CD3D11Driver::getColorFormat() const
        {
            return ColorFormat;
        }


        core::dimension2du CD3D11Driver::getMaxTextureSize() const
        {
            return core::dimension2du(16384, 16384);
        }


        DXGI_FORMAT CD3D11Driver::getDXGIFormatFromColorFormat(ECOLOR_FORMAT format) const
        {
            switch (format)
            {
                case ECF_A1R5G5B5:
                    return DXGI_FORMAT_B5G5R5A1_UNORM;

                case ECF_R5G6B5:
                    return DXGI_FORMAT_B5G6R5_UNORM;

                case ECF_R8G8B8:
                    return DXGI_FORMAT_B8G8R8_UNORM;

                case ECF_A8R8G8B8:
                    return DXGI_FORMAT_B8G8R8A8_UNORM;

                case ECF_R16F:
                    return DXGI_FORMAT_R16_FLOAT;

                case ECF_G16R16F:
                    return DXGI_FORMAT_R16G16_FLOAT;

                case ECF_A16B16G16R16F:
                    return DXGI_FORMAT_R16G16B16A16_FLOAT;

                case ECF_R32F:
                    return DXGI_FORMAT_R32_FLOAT;

                case ECF_G32R32F:
                    return DXGI_FORMAT_R32G32_FLOAT;

                case ECF_A32B32G32R32F:
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
                    return ECF_A1R5G5B5;

                case DXGI_FORMAT_B5G6R5_UNORM:
                    return ECF_R5G6B5;

                case DXGI_FORMAT_B8G8R8_UNORM:
                    return ECF_R8G8B8;

                case DXGI_FORMAT_B8G8R8A8_UNORM:
                    return ECF_A8R8G8B8;

                case DXGI_FORMAT_R16_FLOAT:
                    return ECF_R16F;

                case DXGI_FORMAT_R16G16_FLOAT:
                    return ECF_G16R16F;

                case DXGI_FORMAT_R16G16B16A16_FLOAT:
                    return ECF_A16B16G16R16F;

                case DXGI_FORMAT_R32_FLOAT:
                    return ECF_R32F;

                case DXGI_FORMAT_R32G32_FLOAT:
                    return ECF_G32R32F;

                case DXGI_FORMAT_R32G32B32A32_FLOAT:
                    return ECF_A32B32G32R32F;

                default:
                    return ECF_A8R8G8B8;
            }
        }


        void CD3D11Driver::createMaterialRenderers()
        {}


        D3D11_TEXTURE_ADDRESS CD3D11Driver::getTextureWrapMode(const u8 clamp) const
        {
            return D3D11_TEXTURE_ADDRESS_WRAP;
        }


        bool CD3D11Driver::reset()
        {
            return false;
        }


        video::ITexture* CD3D11Driver::createDeviceDependentTexture(IImage *surface, const io::path &name, void *mipmapData)
        {
            return 0;
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


        void CD3D11Driver::setVertexShader(video::E_VERTEX_TYPE newType)
        {
            LastVertexType = newType;
        }


        void CD3D11Driver::setRenderStatesStencilFillMode(bool alpha)
        {
            CurrentRenderMode = ERM_STENCIL_FILL;
        }


        void CD3D11Driver::setRenderStatesStencilShadowMode(bool zfail, u32 debugDataVisible)
        {
            CurrentRenderMode = zfail ? ERM_SHADOW_VOLUME_ZFAIL : ERM_SHADOW_VOLUME_ZPASS;
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