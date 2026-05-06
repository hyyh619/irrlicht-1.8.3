// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_VIDEO_DIRECTX_11_H_INCLUDED__
#define __C_VIDEO_DIRECTX_11_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#ifdef _IRR_WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "CNullDriver.h"
#include "SIrrCreationParameters.h"
#include "IMaterialRendererServices.h"
#if defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#include "irrMath.h"
#endif
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <windef.h>
#include <dxgitype.h>
#include <dxgi1_2.h>

namespace irr
{
    namespace video
    {
        class CD3D11Driver;

        class CSampler : public IReferenceCounted
        {
public:
            CSampler(CD3D11Driver *driver);
            virtual ~CSampler();

            bool createDefault();
            bool create(const D3D11_SAMPLER_DESC &desc);

            ID3D11SamplerState* getD3D11SamplerState() const
            {
                return m_D3D11SamplerState;
            }

            D3D11_FILTER getFilter() const
            {
                return m_Filter;
            }
            void setFilter(D3D11_FILTER filter)
            {
                m_Filter = filter;
            }

            D3D11_TEXTURE_ADDRESS_MODE getAddressU() const
            {
                return m_AddressU;
            }
            void setAddressU(D3D11_TEXTURE_ADDRESS_MODE mode)
            {
                m_AddressU = mode;
            }

            D3D11_TEXTURE_ADDRESS_MODE getAddressV() const
            {
                return m_AddressV;
            }
            void setAddressV(D3D11_TEXTURE_ADDRESS_MODE mode)
            {
                m_AddressV = mode;
            }

            D3D11_TEXTURE_ADDRESS_MODE getAddressW() const
            {
                return m_AddressW;
            }
            void setAddressW(D3D11_TEXTURE_ADDRESS_MODE mode)
            {
                m_AddressW = mode;
            }

            f32 getMipLODBias() const
            {
                return m_MipLODBias;
            }
            void setMipLODBias(f32 bias)
            {
                m_MipLODBias = bias;
            }

            u32 getMaxAnisotropy() const
            {
                return m_MaxAnisotropy;
            }
            void setMaxAnisotropy(u32 max)
            {
                m_MaxAnisotropy = max;
            }

            D3D11_COMPARISON_FUNC getComparisonFunc() const
            {
                return m_ComparisonFunc;
            }
            void setComparisonFunc(D3D11_COMPARISON_FUNC func)
            {
                m_ComparisonFunc = func;
            }

            f32 getMinLOD() const
            {
                return m_MinLOD;
            }
            void setMinLOD(f32 lod)
            {
                m_MinLOD = lod;
            }

            f32 getMaxLOD() const
            {
                return m_MaxLOD;
            }
            void setMaxLOD(f32 lod)
            {
                m_MaxLOD = lod;
            }

private:
            CD3D11Driver            *m_Driver;
            ID3D11SamplerState      *m_D3D11SamplerState;

            D3D11_FILTER                    m_Filter;
            D3D11_TEXTURE_ADDRESS_MODE      m_AddressU;
            D3D11_TEXTURE_ADDRESS_MODE      m_AddressV;
            D3D11_TEXTURE_ADDRESS_MODE      m_AddressW;
            f32                             m_MipLODBias;
            u32                             m_MaxAnisotropy;
            D3D11_COMPARISON_FUNC           m_ComparisonFunc;
            f32                             m_MinLOD;
            f32                             m_MaxLOD;

#ifdef _DEBUG
            core::stringc    m_DebugName;
#endif
        };

        struct SD3D11DepthStencilView : public IReferenceCounted
        {
            SD3D11DepthStencilView() : Surface(0)
            {
#ifdef _DEBUG
                setDebugName("SD3D11DepthStencilView");
#endif
            }

            virtual ~SD3D11DepthStencilView()
            {
                if (Surface)
                    Surface->Release();
            }

            ID3D11DepthStencilView  *Surface;
            core::dimension2du      Size;
        };

        class CD3D11ShaderMaterialRenderer;
        class CD3D11NormalMapRenderer;
        class CD3D11ParallaxMapRenderer;
        class CD3D11Shader;

#if defined(_IRR_COMPILE_WITH_CG_)
        class CD3D11CgMaterialRenderer;
#endif

        class CD3D11Driver : public CNullDriver, IMaterialRendererServices
        {
public:

            friend class CD3D11Texture;
            friend class CSampler;

            CD3D11Driver(const SIrrlichtCreationParameters &params, io::IFileSystem *io);

            virtual ~CD3D11Driver();

            virtual bool beginScene(bool backBuffer = true, bool zBuffer = true,
                                    SColor color = SColor(255, 0, 0, 0),
                                    const SExposedVideoData &videoData = SExposedVideoData(),
                                    core::rect<s32> *sourceRect = 0);

            virtual bool endScene();

            virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;

            virtual void setTransform(E_TRANSFORMATION_STATE state, const core::matrix4 &mat);

            virtual void setMaterial(const SMaterial &material);

            virtual bool setRenderTarget(video::ITexture *texture,
                                         bool clearBackBuffer = true, bool clearZBuffer = true,
                                         SColor color = video::SColor(0, 0, 0, 0));

            virtual bool setRenderTarget(const core::array<video::IRenderTarget> &texture,
                                         bool clearBackBuffer = true, bool clearZBuffer = true,
                                         SColor color = video::SColor(0, 0, 0, 0));

            virtual void setViewPort(const core::rect<s32> &area);

            virtual const core::rect<s32>&getViewPort() const;

            struct SHWBufferLink_d3d11 : public SHWBufferLink
            {
                SHWBufferLink_d3d11(const scene::IMeshBuffer *_MeshBuffer) :
                    SHWBufferLink(_MeshBuffer),
                    vertexBuffer(0), indexBuffer(0),
                    vertexBufferSize(0), indexBufferSize(0) {}

                ID3D11Buffer    *vertexBuffer;
                ID3D11Buffer    *indexBuffer;

                u32 vertexBufferSize;
                u32 indexBufferSize;
            };

            bool updateVertexHardwareBuffer(SHWBufferLink_d3d11 *HWBuffer);
            bool updateIndexHardwareBuffer(SHWBufferLink_d3d11 *HWBuffer);

            virtual bool updateHardwareBuffer(SHWBufferLink *HWBuffer);

            virtual SHWBufferLink* createHardwareBuffer(const scene::IMeshBuffer *mb);

            virtual void deleteHardwareBuffer(SHWBufferLink *HWBuffer);

            virtual void drawHardwareBuffer(SHWBufferLink *HWBuffer);

            virtual void addOcclusionQuery(scene::ISceneNode *node,
                                           const scene::IMesh *mesh = 0);

            virtual void removeOcclusionQuery(scene::ISceneNode *node);

            virtual void runOcclusionQuery(scene::ISceneNode *node, bool visible = false);

            virtual void updateOcclusionQuery(scene::ISceneNode *node, bool block = true);

            virtual u32 getOcclusionQueryResult(scene::ISceneNode *node) const;

            virtual void drawVertexPrimitiveList(const void *vertices, u32 vertexCount,
                                                 const void *indexList, u32 primitiveCount,
                                                 E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                                                 E_INDEX_TYPE iType);

            virtual void draw2DVertexPrimitiveList(const void *vertices, u32 vertexCount,
                                                   const void *indexList, u32 primitiveCount,
                                                   E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                                                   E_INDEX_TYPE iType);

            void draw2D3DVertexPrimitiveList(const void *vertices,
                                             u32 vertexCount, const void *indexList, u32 primitiveCount,
                                             E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                                             E_INDEX_TYPE iType, bool is3D);

            virtual void draw2DImage(const video::ITexture *texture, const core::position2d<s32> &destPos,
                                     const core::rect<s32> &sourceRect, const core::rect<s32> *clipRect = 0,
                                     SColor color = SColor(255, 255, 255, 255), bool useAlphaChannelOfTexture = false);

            virtual void draw2DImage(const video::ITexture *texture, const core::rect<s32> &destRect,
                                     const core::rect<s32> &sourceRect, const core::rect<s32> *clipRect = 0,
                                     const video::SColor* const colors = 0, bool useAlphaChannelOfTexture = false);

            virtual void draw2DImageBatch(const video::ITexture *texture,
                                          const core::array<core::position2d<s32> > &positions,
                                          const core::array<core::rect<s32> > &sourceRects,
                                          const core::rect<s32> *clipRect = 0,
                                          SColor color = SColor(255, 255, 255, 255),
                                          bool useAlphaChannelOfTexture = false);

            virtual void draw2DRectangle(const core::rect<s32> &pos,
                                         SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
                                         const core::rect<s32> *clip);

            virtual void draw2DLine(const core::position2d<s32> &start,
                                    const core::position2d<s32> &end,
                                    SColor color = SColor(255, 255, 255, 255));

            virtual void drawPixel(u32 x, u32 y, const SColor &color);

            virtual void draw3DLine(const core::vector3df &start,
                                    const core::vector3df &end, SColor color = SColor(255, 255, 255, 255));

            bool initDriver(HWND hwnd, bool pureSoftware);

            virtual const wchar_t* getName() const;

            virtual void deleteAllDynamicLights();

            virtual s32 addDynamicLight(const SLight &light);

            virtual void turnLightOn(s32 lightIndex, bool turnOn);

            virtual u32 getMaximalDynamicLightAmount() const;

            virtual void setAmbientLight(const SColorf &color);

            virtual void drawStencilShadowVolume(const core::array<core::vector3df> &triangles, bool zfail = true, u32 debugDataVisible = 0);

            virtual void drawStencilShadow(bool clearStencilBuffer = false,
                                           video::SColor leftUpEdge = video::SColor(0, 0, 0, 0),
                                           video::SColor rightUpEdge = video::SColor(0, 0, 0, 0),
                                           video::SColor leftDownEdge = video::SColor(0, 0, 0, 0),
                                           video::SColor rightDownEdge = video::SColor(0, 0, 0, 0));

            virtual u32 getMaximalPrimitiveCount() const;

            virtual void setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled);

            virtual void setFog(SColor color, E_FOG_TYPE fogType, f32 start,
                                f32 end, f32 density, bool pixelFog, bool rangeFog);

            virtual void OnResize(const core::dimension2d<u32> &size);

            virtual void setBasicRenderStates(const SMaterial &material, const SMaterial &lastMaterial,
                                              bool resetAllRenderstates);

            virtual E_DRIVER_TYPE getDriverType() const;

            virtual const core::matrix4&getTransform(E_TRANSFORMATION_STATE state) const;

            virtual void setVertexShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount = 1);

            virtual void setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount = 1);

            virtual bool setVertexShaderConstant(const c8 *name, const f32 *floats, int count);

            virtual bool setVertexShaderConstant(const c8 *name, const bool *bools, int count);

            virtual bool setVertexShaderConstant(const c8 *name, const s32 *ints, int count);

            virtual bool setPixelShaderConstant(const c8 *name, const f32 *floats, int count);

            virtual bool setPixelShaderConstant(const c8 *name, const bool *bools, int count);

            virtual bool setPixelShaderConstant(const c8 *name, const s32 *ints, int count);

            virtual IVideoDriver* getVideoDriver();

            virtual ITexture* addRenderTargetTexture(const core::dimension2d<u32> &size,
                                                     const io::path &name, const ECOLOR_FORMAT format = ECOLOR_FORMAT::ECF_UNKNOWN);

            virtual void clearZBuffer();

            virtual IImage* createScreenShot(video::ECOLOR_FORMAT format = video::ECOLOR_FORMAT::ECF_UNKNOWN, video::E_RENDER_TARGET target = video::ERT_FRAME_BUFFER);

            virtual bool setClipPlane(u32 index, const core::plane3df &plane, bool enable = false);

            virtual void enableClipPlane(u32 index, bool enable);

            virtual core::stringc getVendorInfo()
            {
                return m_VendorName;
            }

            virtual void enableMaterial2D(bool enable = true);

            virtual bool checkDriverReset()
            {
                return m_DriverWasReset;
            }

            virtual ECOLOR_FORMAT getColorFormat() const;

            virtual core::dimension2du getMaxTextureSize() const;

            DXGI_FORMAT getDXGIFormatFromColorFormat(ECOLOR_FORMAT format) const;

            ECOLOR_FORMAT getColorFormatFromDXGIFormat(DXGI_FORMAT format) const;

            void createMaterialRenderers();

            D3D11_TEXTURE_ADDRESS_MODE getTextureWrapMode(const u8 clamp) const;

            inline FLOAT* colorToD3D(const SColor &col, FLOAT *f)
            {
                f[0]    = col.getRed() / 255.0f;
                f[1]    = col.getGreen() / 255.0f;
                f[2]    = col.getBlue() / 255.0f;
                f[3]    = col.getAlpha() / 255.0f;
                return f;
            }

            ID3D11Device                *m_pID3DDevice;
            ID3D11DeviceContext         *m_pID3DDeviceContext;
            ID3D11Device1               *m_pID3DDevice1;
            IDXGISwapChain              *m_SwapChain;
            ID3D11RenderTargetView      *m_BackBufferRenderTargetView;
            ID3D11DepthStencilView      *m_DepthStencilView;
#ifdef _DEBUG
            ID3D11Debug    *m_pID3D11Debug;
#endif

private:

            enum E_RENDER_MODE
            {
                ERM_NONE = 0,
                ERM_2D,
                ERM_3D,
                ERM_STENCIL_FILL,
                ERM_SHADOW_VOLUME_ZFAIL,
                ERM_SHADOW_VOLUME_ZPASS
            };

            void setShadersByType(video::E_VERTEX_TYPE newType);

            void setPSTextureAndSamplerState();

            bool setRenderStates3DMode();

            void setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel);

            void setRenderStatesStencilFillMode(bool alpha);

            void setRenderStatesStencilShadowMode(bool zfail, u32 debugDataVisible);

            bool setActiveTexture(u32 stage, const video::ITexture *texture);

            bool reset();

            virtual video::ITexture* createDeviceDependentTexture(IImage *surface, const io::path &name, void *mipmapData = 0);

            virtual const core::dimension2d<u32>&getCurrentRenderTargetSize() const;

            void checkDepthBuffer(ITexture *tex);

            s32 addShaderMaterial(const c8 *vertexShaderProgram, const c8 *pixelShaderProgram,
                                  IShaderConstantSetCallBack *callback,
                                  E_MATERIAL_TYPE baseMaterial, s32 userData);

            virtual s32 addHighLevelShaderMaterial(
                const c8 *vertexShaderProgram,
                const c8 *vertexShaderEntryPointName = "main",
                E_VERTEX_SHADER_TYPE vsCompileTarget = EVST_VS_4_0,
                const c8 *pixelShaderProgram = 0,
                const c8 *pixelShaderEntryPointName = "main",
                E_PIXEL_SHADER_TYPE psCompileTarget = EPST_PS_4_0,
                const c8 *geometryShaderProgram = 0,
                const c8 *geometryShaderEntryPointName = "main",
                E_GEOMETRY_SHADER_TYPE gsCompileTarget = EGST_GS_4_0,
                scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
                scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
                u32 verticesOut = 0,
                IShaderConstantSetCallBack *callback = 0,
                E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
                s32 userData = 0,
                E_GPU_SHADING_LANGUAGE shadingLang = EGSL_DEFAULT);

            bool createBuiltInVertexShader(E_VERTEX_TYPE type);
            bool createBuiltInPixelShader(E_VERTEX_TYPE type);
            bool createInputLayout(E_VERTEX_TYPE type, ID3DBlob *shaderBlob);
            bool createRectangleShaders();
            void updateMatrixConstantBuffer();
            void setRenderStates(E_RENDER_MODE mode, bool alpha);
            bool createDefaultStates();

            core::array<SD3D11DepthStencilView*>    m_DepthBuffers;
            core::array<IMaterialRenderer*>         m_MaterialRenderers;

            void removeDepthSurface(SD3D11DepthStencilView *depth);

            core::array<CD3D11Shader*>      m_ShaderPool;
            DXGI_MODE_DESC                  m_SwapChainBufferDesc;
            DXGI_SWAP_CHAIN_DESC            m_SwapChainDesc;

            SMaterial           m_Material, m_LastMaterial;
            bool                m_ResetRenderStates;
            bool                m_Transformation3DChanged;
            const ITexture      *m_CurrentTexture[MATERIAL_MAX_TEXTURES];
            bool                m_LastTextureMipMapsAvailable[MATERIAL_MAX_TEXTURES];
            core::matrix4       m_Matrices[ETS_COUNT];

            HMODULE                         m_D3D11Library;
            IDXGIFactory1                   *m_DXGIFactory;
            IDXGIAdapter1                   *m_Adapter;
            core::dimension2d<u32>          m_CurrentRendertargetSize;
            D3D11_VIEWPORT                  m_Viewport;
            HWND                            m_WindowId;
            core::rect<s32>                 m_ViewPort;
            core::rect<s32>                 *m_SceneSourceRect;
            UINT                            m_Caps;
            SIrrlichtCreationParameters     m_Params;
            E_VERTEX_TYPE                   m_LastVertexType;
            ID3D11InputLayout               *m_InputLayout[EVT_TANGENTS + 1];
            ID3D11VertexShader              *m_BuiltInVertexShader[EVT_TANGENTS + 1];
            ID3D11PixelShader               *m_BuiltInPixelShader[EVT_TANGENTS + 1];
            bool                            m_BuiltInShadersInitialized;

            ID3D11VertexShader              *m_RectangleVertexShader;
            ID3D11PixelShader               *m_RectanglePixelShader;
            ID3D11InputLayout               *m_RectangleInputLayout;

            SColorf             m_AmbientLight;
            core::stringc       m_VendorName;
            u16                 m_VendorID;

            u32     m_MaxTextureUnits;
            u32     m_MaxUserClipPlanes;
            u32     m_MaxMRTs;
            u32     m_NumSetMRTs;
            f32     m_MaxLightDistance;
            s32     m_LastSetLight;

            ECOLOR_FORMAT       m_ColorFormat;
            DXGI_FORMAT         m_DXGIFormat;
            bool                m_DeviceRemoved;
            bool                m_DriverWasReset;
            bool                m_OcclusionQuerySupport;
            bool                m_AlphaToCoverageSupport;

            ID3D11Buffer            *m_TempVertexBuffer;
            ID3D11Buffer            *m_TempIndexBuffer;
            ID3D11Buffer            *m_MatrixConstantBuffer;
            u32                     m_TempVertexBufferSize;
            u32                     m_TempIndexBufferSize;
            E_INDEX_TYPE            m_TempIndexType;

            D3D11_VIEWPORT                      m_DefaultViewport;
            D3D11_RECT                          m_DefaultScissorRect;
            ID3D11RasterizerState1              *m_RasterizerState;
            ID3D11DepthStencilState             *m_DepthStencilState;
            ID3D11BlendState1                   *m_BlendState;
            CSampler                            *m_DefaultSampler;

            E_RENDER_MODE    m_CurrentRenderMode;
        };

        IVideoDriver* createDirectX11Driver(const SIrrlichtCreationParameters &params,
                                            io::IFileSystem *io, HWND hwnd);
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_
#endif // __C_VIDEO_DIRECTX_11_H_INCLUDED__