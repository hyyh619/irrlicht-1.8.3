# D3D11 shader类设计实现

**Session ID:** ses_20a04d475ffeQlxLgW9t3EjUR4
**Created:** 2026/5/5 10:32:51
**Updated:** 2026/5/5 10:35:14

---

## User

创建shader类
1. 该类表示d3d11的所有shader类型，包括vs/hs/ds/gs/ps/cs
2. 该类保存shader HLSL源代码
3. 如果是vs，还需要保存input layout信息以及创建的ID3D11InputLayout
4. HLSL经过D3DCompile编译后生成的ID3DBlob也需要保存
5. 需要保存创建的ID3D11VertexShader，ID3D11PixelShader，ID3D11DomainShader，ID3D11HullShader,ID3D11GeometryShader,ID3D11ComputeShader等对象
6. 该类的对象由CD3D11Driver负责创建，管理和销毁
7. 把下列shader源码的编译，创建都使用该shader类来管理
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
            "    return DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
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

---

## Assistant (Build · MiniMax-M2.7 · 10.4s)

_Thinking:_

用户要求创建一个shader类来管理D3D11的所有shader类型。让我先了解一下项目结构，特别是D3D11相关的代码在哪里。

我需要：
1. 了解CD3D11Driver的位置和结构
2. 创建新的shader类
3. 理解现有的shader编译和使用方式

让我先搜索相关文件。

**Tool: grep**

**Input:**
```json
{
  "pattern": "CD3D11Driver",
  "include": "*.cpp"
}
```

**Output:**
```
Found 110 matches (showing first 100)
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp:
  Line 6: #include "CD3D11Driver.h"

  Line 162:         CD3D11Driver::CD3D11Driver(const SIrrlichtCreationParameters &params, io::IFileSystem *io)

  Line 181:             setDebugName("CD3D11Driver");

  Line 205:         CSampler::CSampler(CD3D11Driver *driver)

  Line 269:         CD3D11Driver::~CD3D11Driver()

  Line 339:         bool CD3D11Driver::initDriver(HWND hwnd, bool pureSoftware)

  Line 586:         bool CD3D11Driver::beginScene(bool backBuffer, bool zBuffer, SColor color,

  Line 637:         bool CD3D11Driver::endScene()

  Line 649:         bool CD3D11Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const

  Line 716:         void CD3D11Driver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4 &mat)

  Line 724:         void CD3D11Driver::setMaterial(const SMaterial &material)

  Line 739:         bool CD3D11Driver::setRenderTarget(video::ITexture *texture, bool clearBackBuffer,

  Line 746:         bool CD3D11Driver::setRenderTarget(const core::array<video::IRenderTarget> &rtList,

  Line 753:         void CD3D11Driver::setViewPort(const core::rect<s32> &area)

  Line 775:         const core::rect<s32>&CD3D11Driver::getViewPort() const

  Line 781:         bool CD3D11Driver::setActiveTexture(u32 stage, const video::ITexture *texture)

  Line 800:         const core::dimension2d<u32>&CD3D11Driver::getCurrentRenderTargetSize() const

  Line 806:         void CD3D11Driver::setBasicRenderStates(const SMaterial &material, const SMaterial &lastMaterial,

  Line 830:         bool CD3D11Driver::setRenderStates3DMode()

  Line 841:         void CD3D11Driver::setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel)

  Line 848:         bool CD3D11Driver::updateVertexHardwareBuffer(SHWBufferLink_d3d11 *hwBuffer)

  Line 900:         bool CD3D11Driver::updateIndexHardwareBuffer(SHWBufferLink_d3d11 *hwBuffer)

  Line 953:         bool CD3D11Driver::updateHardwareBuffer(SHWBufferLink *hwBuffer)

  Line 970:         CD3D11Driver::SHWBufferLink* CD3D11Driver::createHardwareBuffer(const scene::IMeshBuffer *mb)

  Line 976:         void CD3D11Driver::deleteHardwareBuffer(SHWBufferLink *hwBuffer)

  Line 999:         void CD3D11Driver::drawHardwareBuffer(SHWBufferLink *hwBuffer)

  Line 1049:         void CD3D11Driver::addOcclusionQuery(scene::ISceneNode *node, const scene::IMesh *mesh)

  Line 1053:         void CD3D11Driver::removeOcclusionQuery(scene::ISceneNode *node)

  Line 1057:         void CD3D11Driver::runOcclusionQuery(scene::ISceneNode *node, bool visible)

  Line 1061:         void CD3D11Driver::updateOcclusionQuery(scene::ISceneNode *node, bool block)

  Line 1065:         u32 CD3D11Driver::getOcclusionQueryResult(scene::ISceneNode *node) const

  Line 1071:         void CD3D11Driver::drawVertexPrimitiveList(const void *vertices, u32 vertexCount,

  Line 1080:         void CD3D11Driver::draw2DVertexPrimitiveList(const void *vertices, u32 vertexCount,

  Line 1089:         void CD3D11Driver::draw2D3DVertexPrimitiveList(const void *vertices,

  Line 1245:         void CD3D11Driver::draw2DImage(const video::ITexture *texture, const core::position2d<s32> &destPos,

  Line 1251:         void CD3D11Driver::draw2DImage(const video::ITexture *texture, const core::rect<s32> &destRect,

  Line 1257:         void CD3D11Driver::draw2DImageBatch(const video::ITexture *texture,

  Line 1265:         void CD3D11Driver::draw2DRectangle(const core::rect<s32> &pos,

  Line 1292:         void CD3D11Driver::draw2DLine(const core::position2d<s32> &start,

  Line 1417:         void CD3D11Driver::drawPixel(u32 x, u32 y, const SColor &color)

  Line 1421:         void CD3D11Driver::draw3DLine(const core::vector3df &start, const core::vector3df &end, SColor color)

  Line 1435:         const wchar_t* CD3D11Driver::getName() const

  Line 1441:         void CD3D11Driver::deleteAllDynamicLights()

  Line 1451:         s32 CD3D11Driver::addDynamicLight(const SLight &light)

  Line 1461:         void CD3D11Driver::turnLightOn(s32 lightIndex, bool turnOn)

  Line 1467:         u32 CD3D11Driver::getMaximalDynamicLightAmount() const

  Line 1473:         void CD3D11Driver::setAmbientLight(const SColorf &color)

  Line 1479:         void CD3D11Driver::drawStencilShadowVolume(const core::array<core::vector3df> &triangles, bool zfail, u32 debugDataVisible)

  Line 1483:         void CD3D11Driver::drawStencilShadow(bool clearStencilBuffer, video::SColor leftUpEdge, video::SColor rightUpEdge,

  Line 1488:         u32 CD3D11Driver::getMaximalPrimitiveCount() const

  Line 1494:         void CD3D11Driver::setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled)

  Line 1500:         void CD3D11Driver::setFog(SColor color, E_FOG_TYPE fogType, f32 start, f32 end,

  Line 1505:         void CD3D11Driver::OnResize(const core::dimension2d<u32> &size)

  Line 1511:         E_DRIVER_TYPE CD3D11Driver::getDriverType() const

  Line 1517:         const core::matrix4&CD3D11Driver::getTransform(E_TRANSFORMATION_STATE state) const

  Line 1523:         void CD3D11Driver::setVertexShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount)

  Line 1527:         void CD3D11Driver::setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount)

  Line 1531:         bool CD3D11Driver::setVertexShaderConstant(const c8 *name, const f32 *floats, int count)

  Line 1537:         bool CD3D11Driver::setVertexShaderConstant(const c8 *name, const bool *bools, int count)

  Line 1543:         bool CD3D11Driver::setVertexShaderConstant(const c8 *name, const s32 *ints, int count)

  Line 1549:         bool CD3D11Driver::setPixelShaderConstant(const c8 *name, const f32 *floats, int count)

  Line 1555:         bool CD3D11Driver::setPixelShaderConstant(const c8 *name, const bool *bools, int count)

  Line 1561:         bool CD3D11Driver::setPixelShaderConstant(const c8 *name, const s32 *ints, int count)

  Line 1567:         IVideoDriver* CD3D11Driver::getVideoDriver()

  Line 1573:         ITexture* CD3D11Driver::addRenderTargetTexture(const core::dimension2d<u32> &size,

  Line 1580:         void CD3D11Driver::clearZBuffer()

  Line 1586:         IImage* CD3D11Driver::createScreenShot(video::ECOLOR_FORMAT format, video::E_RENDER_TARGET target)

  Line 1592:         bool CD3D11Driver::setClipPlane(u32 index, const core::plane3df &plane, bool enable)

  Line 1598:         void CD3D11Driver::enableClipPlane(u32 index, bool enable)

  Line 1602:         void CD3D11Driver::enableMaterial2D(bool enable)

  Line 1606:         void CD3D11Driver::removeDepthSurface(SD3D11DepthStencilView *depth)

  Line 1620:         ECOLOR_FORMAT CD3D11Driver::getColorFormat() const

  Line 1626:         core::dimension2du CD3D11Driver::getMaxTextureSize() const

  Line 1632:         DXGI_FORMAT CD3D11Driver::getDXGIFormatFromColorFormat(ECOLOR_FORMAT format) const

  Line 1672:         ECOLOR_FORMAT CD3D11Driver::getColorFormatFromDXGIFormat(DXGI_FORMAT format) const

  Line 1709:         void CD3D11Driver::createMaterialRenderers()

  Line 1737:         D3D11_TEXTURE_ADDRESS_MODE CD3D11Driver::getTextureWrapMode(const u8 clamp) const

  Line 1743:         bool CD3D11Driver::reset()

  Line 1749:         video::ITexture* CD3D11Driver::createDeviceDependentTexture(IImage *surface, const io::path &name, void *mipmapData)

  Line 1755:         void CD3D11Driver::checkDepthBuffer(ITexture *tex)

  Line 1759:         s32 CD3D11Driver::addShaderMaterial(const c8 *vertexShaderProgram, const c8 *pixelShaderProgram,

  Line 1766:         s32 CD3D11Driver::addHighLevelShaderMaterial(

  Line 1780:         void CD3D11Driver::setShadersByType(video::E_VERTEX_TYPE newType)

  Line 1816:         void CD3D11Driver::setPSTextureAndSamplerState()

  Line 1845:         bool CD3D11Driver::createBuiltInVertexShader(E_VERTEX_TYPE type)

  Line 1903:         bool CD3D11Driver::createBuiltInPixelShader(E_VERTEX_TYPE type)

  Line 1956:         bool CD3D11Driver::createInputLayout(E_VERTEX_TYPE type, ID3DBlob *shaderBlob)

  Line 2020:         void CD3D11Driver::updateMatrixConstantBuffer()

  Line 2036:         bool CD3D11Driver::createDefaultStates()

  Line 2124:         void CD3D11Driver::setRenderStates(E_RENDER_MODE mode, bool alpha)

  Line 2170:         void CD3D11Driver::setRenderStatesStencilFillMode(bool alpha)

  Line 2176:         void CD3D11Driver::setRenderStatesStencilShadowMode(bool zfail, u32 debugDataVisible)

  Line 2186:             CD3D11Driver    *dx11           = new CD3D11Driver(params, io);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp:
  Line 10: #include "CD3D11Driver.h"

  Line 18:         CD3D11Texture::CD3D11Texture(CD3D11Driver *driver, const core::dimension2d<u32> &size,

  Line 38:         CD3D11Texture::CD3D11Texture(IImage *image, CD3D11Driver *driver,


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11CgMaterialRenderer.cpp:
  Line 9: #include "CD3D11Driver.h"

  Line 37:         CD3D11CgMaterialRenderer::CD3D11CgMaterialRenderer(CD3D11Driver *driver, s32 &materialType,


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11MaterialRenderer.cpp:
  Line 10: #include "CD3D11Driver.h"

  Line 18:         CD3D11MaterialRenderer::CD3D11MaterialRenderer(CD3D11Driver *driver, s32 materialType,


(Results truncated: showing 100 of 110 matches (10 hidden). Consider using a more specific path or pattern.)
```

**Tool: grep**

**Input:**
```json
{
  "pattern": "ID3D11VertexShader",
  "include": "*.h"
}
```

**Output:**
```
Found 3 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h:
  Line 490:             ID3D11VertexShader              *m_BuiltInVertexShader[3];


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ShaderMaterialRenderer.h:
  Line 64:             ID3D11VertexShader          *m_VertexShader;

  Line 65:             ID3D11VertexShader          *m_OldVertexShader;

```

**Tool: glob**

**Input:**
```json
{
  "pattern": "**/CD3D11*.{h,cpp}"
}
```

**Output:**
```
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11NormalMapRenderer.cpp
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11CgMaterialRenderer.cpp
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11CgMaterialRenderer.h
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11HLSLMaterialRenderer.cpp
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11HLSLMaterialRenderer.h
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ParallaxMapRenderer.cpp
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ParallaxMapRenderer.h
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ShaderMaterialRenderer.cpp
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ShaderMaterialRenderer.h
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11MaterialRenderer.cpp
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11MaterialRenderer.h
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.h
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11NormalMapRenderer.h
```

---

## Assistant (Build · MiniMax-M2.7 · 4.7s)

_Thinking:_

让我查看CD3D11Driver.h和CD3D11ShaderMaterialRenderer来了解现有的shader管理结构。

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.h"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #ifndef __C_VIDEO_DIRECTX_11_H_INCLUDED__
6: #define __C_VIDEO_DIRECTX_11_H_INCLUDED__
7: 
8: #include "IrrCompileConfig.h"
9: 
10: #ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
11: 
12: #ifdef _IRR_WINDOWS_
13: #define WIN32_LEAN_AND_MEAN
14: #include <windows.h>
15: #endif
16: 
17: #include "CNullDriver.h"
18: #include "SIrrCreationParameters.h"
19: #include "IMaterialRendererServices.h"
20: #if defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
21: #include "irrMath.h"
22: #endif
23: #include <d3d11.h>
24: #include <d3d11_1.h>
25: #include <d3dcompiler.h>
26: #include <windef.h>
27: #include <dxgitype.h>
28: #include <dxgi1_2.h>
29: 
30: namespace irr
31: {
32:     namespace video
33:     {
34:         class CD3D11Driver;
35: 
36:         class CSampler : public IReferenceCounted
37:         {
38: public:
39:             CSampler(CD3D11Driver *driver);
40:             virtual ~CSampler();
41: 
42:             bool createDefault();
43:             bool create(const D3D11_SAMPLER_DESC &desc);
44: 
45:             ID3D11SamplerState* getD3D11SamplerState() const
46:             {
47:                 return m_D3D11SamplerState;
48:             }
49: 
50:             D3D11_FILTER getFilter() const
51:             {
52:                 return m_Filter;
53:             }
54:             void setFilter(D3D11_FILTER filter)
55:             {
56:                 m_Filter = filter;
57:             }
58: 
59:             D3D11_TEXTURE_ADDRESS_MODE getAddressU() const
60:             {
61:                 return m_AddressU;
62:             }
63:             void setAddressU(D3D11_TEXTURE_ADDRESS_MODE mode)
64:             {
65:                 m_AddressU = mode;
66:             }
67: 
68:             D3D11_TEXTURE_ADDRESS_MODE getAddressV() const
69:             {
70:                 return m_AddressV;
71:             }
72:             void setAddressV(D3D11_TEXTURE_ADDRESS_MODE mode)
73:             {
74:                 m_AddressV = mode;
75:             }
76: 
77:             D3D11_TEXTURE_ADDRESS_MODE getAddressW() const
78:             {
79:                 return m_AddressW;
80:             }
81:             void setAddressW(D3D11_TEXTURE_ADDRESS_MODE mode)
82:             {
83:                 m_AddressW = mode;
84:             }
85: 
86:             f32 getMipLODBias() const
87:             {
88:                 return m_MipLODBias;
89:             }
90:             void setMipLODBias(f32 bias)
91:             {
92:                 m_MipLODBias = bias;
93:             }
94: 
95:             u32 getMaxAnisotropy() const
96:             {
97:                 return m_MaxAnisotropy;
98:             }
99:             void setMaxAnisotropy(u32 max)
100:             {
101:                 m_MaxAnisotropy = max;
102:             }
103: 
104:             D3D11_COMPARISON_FUNC getComparisonFunc() const
105:             {
106:                 return m_ComparisonFunc;
107:             }
108:             void setComparisonFunc(D3D11_COMPARISON_FUNC func)
109:             {
110:                 m_ComparisonFunc = func;
111:             }
112: 
113:             f32 getMinLOD() const
114:             {
115:                 return m_MinLOD;
116:             }
117:             void setMinLOD(f32 lod)
118:             {
119:                 m_MinLOD = lod;
120:             }
121: 
122:             f32 getMaxLOD() const
123:             {
124:                 return m_MaxLOD;
125:             }
126:             void setMaxLOD(f32 lod)
127:             {
128:                 m_MaxLOD = lod;
129:             }
130: 
131: private:
132:             CD3D11Driver            *m_Driver;
133:             ID3D11SamplerState      *m_D3D11SamplerState;
134: 
135:             D3D11_FILTER                    m_Filter;
136:             D3D11_TEXTURE_ADDRESS_MODE      m_AddressU;
137:             D3D11_TEXTURE_ADDRESS_MODE      m_AddressV;
138:             D3D11_TEXTURE_ADDRESS_MODE      m_AddressW;
139:             f32                             m_MipLODBias;
140:             u32                             m_MaxAnisotropy;
141:             D3D11_COMPARISON_FUNC           m_ComparisonFunc;
142:             f32                             m_MinLOD;
143:             f32                             m_MaxLOD;
144: 
145: #ifdef _DEBUG
146:             core::stringc    m_DebugName;
147: #endif
148:         };
149: 
150:         struct SD3D11DepthStencilView : public IReferenceCounted
151:         {
152:             SD3D11DepthStencilView() : Surface(0)
153:             {
154: #ifdef _DEBUG
155:                 setDebugName("SD3D11DepthStencilView");
156: #endif
157:             }
158: 
159:             virtual ~SD3D11DepthStencilView()
160:             {
161:                 if (Surface)
162:                     Surface->Release();
163:             }
164: 
165:             ID3D11DepthStencilView  *Surface;
166:             core::dimension2du      Size;
167:         };
168: 
169:         class CD3D11ShaderMaterialRenderer;
170:         class CD3D11NormalMapRenderer;
171:         class CD3D11ParallaxMapRenderer;
172: 
173: #if defined(_IRR_COMPILE_WITH_CG_)
174:         class CD3D11CgMaterialRenderer;
175: #endif
176: 
177:         class CD3D11Driver : public CNullDriver, IMaterialRendererServices
178:         {
179: public:
180: 
181:             friend class CD3D11Texture;
182:             friend class CSampler;
183: 
184:             CD3D11Driver(const SIrrlichtCreationParameters &params, io::IFileSystem *io);
185: 
186:             virtual ~CD3D11Driver();
187: 
188:             virtual bool beginScene(bool backBuffer = true, bool zBuffer = true,
189:                                     SColor color = SColor(255, 0, 0, 0),
190:                                     const SExposedVideoData &videoData = SExposedVideoData(),
191:                                     core::rect<s32> *sourceRect = 0);
192: 
193:             virtual bool endScene();
194: 
195:             virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;
196: 
197:             virtual void setTransform(E_TRANSFORMATION_STATE state, const core::matrix4 &mat);
198: 
199:             virtual void setMaterial(const SMaterial &material);
200: 
201:             virtual bool setRenderTarget(video::ITexture *texture,
202:                                          bool clearBackBuffer = true, bool clearZBuffer = true,
203:                                          SColor color = video::SColor(0, 0, 0, 0));
204: 
205:             virtual bool setRenderTarget(const core::array<video::IRenderTarget> &texture,
206:                                          bool clearBackBuffer = true, bool clearZBuffer = true,
207:                                          SColor color = video::SColor(0, 0, 0, 0));
208: 
209:             virtual void setViewPort(const core::rect<s32> &area);
210: 
211:             virtual const core::rect<s32>&getViewPort() const;
212: 
213:             struct SHWBufferLink_d3d11 : public SHWBufferLink
214:             {
215:                 SHWBufferLink_d3d11(const scene::IMeshBuffer *_MeshBuffer) :
216:                     SHWBufferLink(_MeshBuffer),
217:                     vertexBuffer(0), indexBuffer(0),
218:                     vertexBufferSize(0), indexBufferSize(0) {}
219: 
220:                 ID3D11Buffer    *vertexBuffer;
221:                 ID3D11Buffer    *indexBuffer;
222: 
223:                 u32 vertexBufferSize;
224:                 u32 indexBufferSize;
225:             };
226: 
227:             bool updateVertexHardwareBuffer(SHWBufferLink_d3d11 *HWBuffer);
228:             bool updateIndexHardwareBuffer(SHWBufferLink_d3d11 *HWBuffer);
229: 
230:             virtual bool updateHardwareBuffer(SHWBufferLink *HWBuffer);
231: 
232:             virtual SHWBufferLink* createHardwareBuffer(const scene::IMeshBuffer *mb);
233: 
234:             virtual void deleteHardwareBuffer(SHWBufferLink *HWBuffer);
235: 
236:             virtual void drawHardwareBuffer(SHWBufferLink *HWBuffer);
237: 
238:             virtual void addOcclusionQuery(scene::ISceneNode *node,
239:                                            const scene::IMesh *mesh = 0);
240: 
241:             virtual void removeOcclusionQuery(scene::ISceneNode *node);
242: 
243:             virtual void runOcclusionQuery(scene::ISceneNode *node, bool visible = false);
244: 
245:             virtual void updateOcclusionQuery(scene::ISceneNode *node, bool block = true);
246: 
247:             virtual u32 getOcclusionQueryResult(scene::ISceneNode *node) const;
248: 
249:             virtual void drawVertexPrimitiveList(const void *vertices, u32 vertexCount,
250:                                                  const void *indexList, u32 primitiveCount,
251:                                                  E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
252:                                                  E_INDEX_TYPE iType);
253: 
254:             virtual void draw2DVertexPrimitiveList(const void *vertices, u32 vertexCount,
255:                                                    const void *indexList, u32 primitiveCount,
256:                                                    E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
257:                                                    E_INDEX_TYPE iType);
258: 
259:             void draw2D3DVertexPrimitiveList(const void *vertices,
260:                                              u32 vertexCount, const void *indexList, u32 primitiveCount,
261:                                              E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
262:                                              E_INDEX_TYPE iType, bool is3D);
263: 
264:             virtual void draw2DImage(const video::ITexture *texture, const core::position2d<s32> &destPos,
265:                                      const core::rect<s32> &sourceRect, const core::rect<s32> *clipRect = 0,
266:                                      SColor color = SColor(255, 255, 255, 255), bool useAlphaChannelOfTexture = false);
267: 
268:             virtual void draw2DImage(const video::ITexture *texture, const core::rect<s32> &destRect,
269:                                      const core::rect<s32> &sourceRect, const core::rect<s32> *clipRect = 0,
270:                                      const video::SColor* const colors = 0, bool useAlphaChannelOfTexture = false);
271: 
272:             virtual void draw2DImageBatch(const video::ITexture *texture,
273:                                           const core::array<core::position2d<s32> > &positions,
274:                                           const core::array<core::rect<s32> > &sourceRects,
275:                                           const core::rect<s32> *clipRect = 0,
276:                                           SColor color = SColor(255, 255, 255, 255),
277:                                           bool useAlphaChannelOfTexture = false);
278: 
279:             virtual void draw2DRectangle(const core::rect<s32> &pos,
280:                                          SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
281:                                          const core::rect<s32> *clip);
282: 
283:             virtual void draw2DLine(const core::position2d<s32> &start,
284:                                     const core::position2d<s32> &end,
285:                                     SColor color = SColor(255, 255, 255, 255));
286: 
287:             virtual void drawPixel(u32 x, u32 y, const SColor &color);
288: 
289:             virtual void draw3DLine(const core::vector3df &start,
290:                                     const core::vector3df &end, SColor color = SColor(255, 255, 255, 255));
291: 
292:             bool initDriver(HWND hwnd, bool pureSoftware);
293: 
294:             virtual const wchar_t* getName() const;
295: 
296:             virtual void deleteAllDynamicLights();
297: 
298:             virtual s32 addDynamicLight(const SLight &light);
299: 
300:             virtual void turnLightOn(s32 lightIndex, bool turnOn);
301: 
302:             virtual u32 getMaximalDynamicLightAmount() const;
303: 
304:             virtual void setAmbientLight(const SColorf &color);
305: 
306:             virtual void drawStencilShadowVolume(const core::array<core::vector3df> &triangles, bool zfail = true, u32 debugDataVisible = 0);
307: 
308:             virtual void drawStencilShadow(bool clearStencilBuffer = false,
309:                                            video::SColor leftUpEdge = video::SColor(0, 0, 0, 0),
310:                                            video::SColor rightUpEdge = video::SColor(0, 0, 0, 0),
311:                                            video::SColor leftDownEdge = video::SColor(0, 0, 0, 0),
312:                                            video::SColor rightDownEdge = video::SColor(0, 0, 0, 0));
313: 
314:             virtual u32 getMaximalPrimitiveCount() const;
315: 
316:             virtual void setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled);
317: 
318:             virtual void setFog(SColor color, E_FOG_TYPE fogType, f32 start,
319:                                 f32 end, f32 density, bool pixelFog, bool rangeFog);
320: 
321:             virtual void OnResize(const core::dimension2d<u32> &size);
322: 
323:             virtual void setBasicRenderStates(const SMaterial &material, const SMaterial &lastMaterial,
324:                                               bool resetAllRenderstates);
325: 
326:             virtual E_DRIVER_TYPE getDriverType() const;
327: 
328:             virtual const core::matrix4&getTransform(E_TRANSFORMATION_STATE state) const;
329: 
330:             virtual void setVertexShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount = 1);
331: 
332:             virtual void setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount = 1);
333: 
334:             virtual bool setVertexShaderConstant(const c8 *name, const f32 *floats, int count);
335: 
336:             virtual bool setVertexShaderConstant(const c8 *name, const bool *bools, int count);
337: 
338:             virtual bool setVertexShaderConstant(const c8 *name, const s32 *ints, int count);
339: 
340:             virtual bool setPixelShaderConstant(const c8 *name, const f32 *floats, int count);
341: 
342:             virtual bool setPixelShaderConstant(const c8 *name, const bool *bools, int count);
343: 
344:             virtual bool setPixelShaderConstant(const c8 *name, const s32 *ints, int count);
345: 
346:             virtual IVideoDriver* getVideoDriver();
347: 
348:             virtual ITexture* addRenderTargetTexture(const core::dimension2d<u32> &size,
349:                                                      const io::path &name, const ECOLOR_FORMAT format = ECOLOR_FORMAT::ECF_UNKNOWN);
350: 
351:             virtual void clearZBuffer();
352: 
353:             virtual IImage* createScreenShot(video::ECOLOR_FORMAT format = video::ECOLOR_FORMAT::ECF_UNKNOWN, video::E_RENDER_TARGET target = video::ERT_FRAME_BUFFER);
354: 
355:             virtual bool setClipPlane(u32 index, const core::plane3df &plane, bool enable = false);
356: 
357:             virtual void enableClipPlane(u32 index, bool enable);
358: 
359:             virtual core::stringc getVendorInfo()
360:             {
361:                 return m_VendorName;
362:             }
363: 
364:             virtual void enableMaterial2D(bool enable = true);
365: 
366:             virtual bool checkDriverReset()
367:             {
368:                 return m_DriverWasReset;
369:             }
370: 
371:             virtual ECOLOR_FORMAT getColorFormat() const;
372: 
373:             virtual core::dimension2du getMaxTextureSize() const;
374: 
375:             DXGI_FORMAT getDXGIFormatFromColorFormat(ECOLOR_FORMAT format) const;
376: 
377:             ECOLOR_FORMAT getColorFormatFromDXGIFormat(DXGI_FORMAT format) const;
378: 
379:             void createMaterialRenderers();
380: 
381:             D3D11_TEXTURE_ADDRESS_MODE getTextureWrapMode(const u8 clamp) const;
382: 
383:             inline FLOAT* colorToD3D(const SColor &col, FLOAT *f)
384:             {
385:                 f[0]    = col.getRed() / 255.0f;
386:                 f[1]    = col.getGreen() / 255.0f;
387:                 f[2]    = col.getBlue() / 255.0f;
388:                 f[3]    = col.getAlpha() / 255.0f;
389:                 return f;
390:             }
391: 
392:             ID3D11Device                *m_pID3DDevice;
393:             ID3D11DeviceContext         *m_pID3DDeviceContext;
394:             ID3D11Device1               *m_pID3DDevice1;
395:             IDXGISwapChain              *m_SwapChain;
396:             ID3D11RenderTargetView      *m_BackBufferRenderTargetView;
397:             ID3D11DepthStencilView      *m_DepthStencilView;
398: #ifdef _DEBUG
399:             ID3D11Debug    *m_pID3D11Debug;
400: #endif
401: 
402: private:
403: 
404:             enum E_RENDER_MODE
405:             {
406:                 ERM_NONE = 0,
407:                 ERM_2D,
408:                 ERM_3D,
409:                 ERM_STENCIL_FILL,
410:                 ERM_SHADOW_VOLUME_ZFAIL,
411:                 ERM_SHADOW_VOLUME_ZPASS
412:             };
413: 
414:             void setShadersByType(video::E_VERTEX_TYPE newType);
415: 
416:             void setPSTextureAndSamplerState();
417: 
418:             bool setRenderStates3DMode();
419: 
420:             void setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel);
421: 
422:             void setRenderStatesStencilFillMode(bool alpha);
423: 
424:             void setRenderStatesStencilShadowMode(bool zfail, u32 debugDataVisible);
425: 
426:             bool setActiveTexture(u32 stage, const video::ITexture *texture);
427: 
428:             bool reset();
429: 
430:             virtual video::ITexture* createDeviceDependentTexture(IImage *surface, const io::path &name, void *mipmapData = 0);
431: 
432:             virtual const core::dimension2d<u32>&getCurrentRenderTargetSize() const;
433: 
434:             void checkDepthBuffer(ITexture *tex);
435: 
436:             s32 addShaderMaterial(const c8 *vertexShaderProgram, const c8 *pixelShaderProgram,
437:                                   IShaderConstantSetCallBack *callback,
438:                                   E_MATERIAL_TYPE baseMaterial, s32 userData);
439: 
440:             virtual s32 addHighLevelShaderMaterial(
441:                 const c8 *vertexShaderProgram,
442:                 const c8 *vertexShaderEntryPointName = "main",
443:                 E_VERTEX_SHADER_TYPE vsCompileTarget = EVST_VS_4_0,
444:                 const c8 *pixelShaderProgram = 0,
445:                 const c8 *pixelShaderEntryPointName = "main",
446:                 E_PIXEL_SHADER_TYPE psCompileTarget = EPST_PS_4_0,
447:                 const c8 *geometryShaderProgram = 0,
448:                 const c8 *geometryShaderEntryPointName = "main",
449:                 E_GEOMETRY_SHADER_TYPE gsCompileTarget = EGST_GS_4_0,
450:                 scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
451:                 scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
452:                 u32 verticesOut = 0,
453:                 IShaderConstantSetCallBack *callback = 0,
454:                 E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
455:                 s32 userData = 0,
456:                 E_GPU_SHADING_LANGUAGE shadingLang = EGSL_DEFAULT);
457: 
458:             bool createBuiltInVertexShader(E_VERTEX_TYPE type);
459:             bool createBuiltInPixelShader(E_VERTEX_TYPE type);
460:             bool createInputLayout(E_VERTEX_TYPE type, ID3DBlob *shaderBlob);
461:             void updateMatrixConstantBuffer();
462:             void setRenderStates(E_RENDER_MODE mode, bool alpha);
463:             bool createDefaultStates();
464: 
465:             core::array<SD3D11DepthStencilView*>    m_DepthBuffers;
466: 
467:             void removeDepthSurface(SD3D11DepthStencilView *depth);
468:             DXGI_MODE_DESC          m_SwapChainBufferDesc;
469:             DXGI_SWAP_CHAIN_DESC    m_SwapChainDesc;
470: 
471:             SMaterial           m_Material, m_LastMaterial;
472:             bool                m_ResetRenderStates;
473:             bool                m_Transformation3DChanged;
474:             const ITexture      *m_CurrentTexture[MATERIAL_MAX_TEXTURES];
475:             bool                m_LastTextureMipMapsAvailable[MATERIAL_MAX_TEXTURES];
476:             core::matrix4       m_Matrices[ETS_COUNT];
477: 
478:             HMODULE                         m_D3D11Library;
479:             IDXGIFactory1                   *m_DXGIFactory;
480:             IDXGIAdapter1                   *m_Adapter;
481:             core::dimension2d<u32>          m_CurrentRendertargetSize;
482:             D3D11_VIEWPORT                  m_Viewport;
483:             HWND                            m_WindowId;
484:             core::rect<s32>                 m_ViewPort;
485:             core::rect<s32>                 *m_SceneSourceRect;
486:             UINT                            m_Caps;
487:             SIrrlichtCreationParameters     m_Params;
488:             E_VERTEX_TYPE                   m_LastVertexType;
489:             ID3D11InputLayout               *m_InputLayout[3];
490:             ID3D11VertexShader              *m_BuiltInVertexShader[3];
491:             ID3D11PixelShader               *m_BuiltInPixelShader[3];
492:             bool                            m_BuiltInShadersInitialized;
493: 
494:             SColorf             m_AmbientLight;
495:             core::stringc       m_VendorName;
496:             u16                 m_VendorID;
497: 
498:             u32     m_MaxTextureUnits;
499:             u32     m_MaxUserClipPlanes;
500:             u32     m_MaxMRTs;
501:             u32     m_NumSetMRTs;
502:             f32     m_MaxLightDistance;
503:             s32     m_LastSetLight;
504: 
505:             ECOLOR_FORMAT       m_ColorFormat;
506:             DXGI_FORMAT         m_DXGIFormat;
507:             bool                m_DeviceRemoved;
508:             bool                m_DriverWasReset;
509:             bool                m_OcclusionQuerySupport;
510:             bool                m_AlphaToCoverageSupport;
511: 
512:             ID3D11Buffer            *m_TempVertexBuffer;
513:             ID3D11Buffer            *m_TempIndexBuffer;
514:             ID3D11Buffer            *m_MatrixConstantBuffer;
515:             u32                     m_TempVertexBufferSize;
516:             u32                     m_TempIndexBufferSize;
517:             E_INDEX_TYPE            m_TempIndexType;
518: 
519:             D3D11_VIEWPORT                      m_DefaultViewport;
520:             D3D11_RECT                          m_DefaultScissorRect;
521:             ID3D11RasterizerState1              *m_RasterizerState;
522:             ID3D11DepthStencilState             *m_DepthStencilState;
523:             ID3D11BlendState1                   *m_BlendState;
524:             CSampler                            *m_DefaultSampler;
525: 
526:             E_RENDER_MODE    m_CurrentRenderMode;
527:         };
528: 
529:         IVideoDriver* createDirectX11Driver(const SIrrlichtCreationParameters &params,
530:                                             io::IFileSystem *io, HWND hwnd);
531:     } // end namespace video
532: } // end namespace irr
533: #endif // _IRR_COMPILE_WITH_DIRECT3D_11_
534: #endif // __C_VIDEO_DIRECTX_11_H_INCLUDED__

(End of file - total 534 lines)
</content>

<system-reminder>
Instructions from: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\AGENTS.md
# SOURCE KNOWLEDGE BASE

## OVERVIEW
Implementation of the Irrlicht Engine including rendering drivers, scene management, and 3rd party libraries.

## STRUCTURE
```
../include         # header files, including interface definition.
source/Irrlicht/
├── aesGladman/    # AES encryption
├── bzip2/         # bzip2 compression
├── jpeglib/       # JPEG image support
├── libpng/        # PNG image support
├── lzma/          # LZMA compression
├── MacOSX/        # macOS platform specifics (Cocoa/OpenGL)
└── zlib/          # zlib compression
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Rendering | `C*Driver.cpp` | `CNullDriver`, `COpenGLDriver`, `CD3D9Driver` |
| Scene Graph | `C*SceneNode.cpp` | `CSceneManager`, `CMeshSceneNode`, etc. |
| GUI | `CGUI*.cpp` | `CGUIEnvironment`, `CGUIButton`, etc. |
| IO | `C*Reader.cpp` | File system and archive handling |

## CONVENTIONS
- Internal classes prefixed with `C` (e.g., `CSceneManager` implements `ISceneManager`).
- Heavy use of `irr::core` containers for internal logic.
- Platform-specific code isolated in `CIrrDevice*.cpp` and OS subdirectories.

## ANTI-PATTERNS
- Do not add dependencies on `std::`.
- Avoid direct OS API calls outside of `CIrrDevice` or platform directories.

</system-reminder>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11ShaderMaterialRenderer.h"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ShaderMaterialRenderer.h</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #ifndef __C_D3D11_SHADER_MATERIAL_RENDERER_H_INCLUDED__
6: #define __C_D3D11_SHADER_MATERIAL_RENDERER_H_INCLUDED__
7: 
8: #include "IrrCompileConfig.h"
9: #ifdef _IRR_WINDOWS_
10: 
11: #ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
12: #if defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
13: #include "irrMath.h"
14: #endif
15: #include <d3d11.h>
16: #include <d3dcompiler.h>
17: 
18: #include "IMaterialRenderer.h"
19: 
20: namespace irr
21: {
22:     namespace video
23:     {
24:         class IVideoDriver;
25:         class IShaderConstantSetCallBack;
26:         class IMaterialRenderer;
27: 
28:         class CD3D11ShaderMaterialRenderer : public IMaterialRenderer
29:         {
30: public:
31: 
32:             CD3D11ShaderMaterialRenderer(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext,
33:                 video::IVideoDriver *driver, s32 &outMaterialTypeNr,
34:                 const c8 *vertexShaderProgram, const c8 *pixelShaderProgram,
35:                 IShaderConstantSetCallBack *callback, IMaterialRenderer *baseMaterial, s32 userData);
36: 
37:             ~CD3D11ShaderMaterialRenderer();
38: 
39:             virtual void OnSetMaterial(const video::SMaterial &material, const video::SMaterial &lastMaterial,
40:                 bool resetAllRenderstates, video::IMaterialRendererServices *services);
41: 
42:             virtual void OnUnsetMaterial();
43: 
44:             virtual bool OnRender(IMaterialRendererServices *service, E_VERTEX_TYPE vtxtype);
45: 
46:             virtual bool isTransparent() const;
47: 
48: protected:
49: 
50:             CD3D11ShaderMaterialRenderer(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext,
51:                 video::IVideoDriver *driver, IShaderConstantSetCallBack *callback,
52:                 IMaterialRenderer *baseMaterial, s32 userData = 0);
53: 
54:             void init(s32 &outMaterialTypeNr, const c8 *vertexShaderProgram, const c8 *pixelShaderProgram);
55:             bool createPixelShader(const c8 *pxsh);
56:             bool createVertexShader(const char *vtxsh);
57: 
58:             ID3D11Device                *m_pID3DDevice;
59:             ID3D11DeviceContext         *m_pID3DDeviceContext;
60:             video::IVideoDriver         *m_Driver;
61:             IShaderConstantSetCallBack  *m_CallBack;
62:             IMaterialRenderer           *m_BaseMaterial;
63: 
64:             ID3D11VertexShader          *m_VertexShader;
65:             ID3D11VertexShader          *m_OldVertexShader;
66:             ID3D11PixelShader           *m_PixelShader;
67:             ID3D11InputLayout           *m_InputLayout;
68:             s32                         m_UserData;
69:         };
70:     } // end namespace video
71: } // end namespace irr
72: #endif
73: #endif
74: #endif

(End of file - total 74 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 3.9s)

_Thinking:_

现在我了解了现有的结构。我需要创建一个新的shader类来管理D3D11的所有shader类型。让我查看一下现有的shader相关代码和编译部分。

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "offset": 1845,
  "limit": 200
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
1845:         bool CD3D11Driver::createBuiltInVertexShader(E_VERTEX_TYPE type)
1846:         {
1847:             const char    *shaderSource = 0;
1848: 
1849:             switch (type)
1850:             {
1851:                 case EVT_STANDARD:
1852:                     shaderSource = VERTEX_SHADER_STANDARD;
1853:                     break;
1854: 
1855:                 case EVT_2TCOORDS:
1856:                     shaderSource = VERTEX_SHADER_2TCOORDS;
1857:                     break;
1858: 
1859:                 case EVT_TANGENTS:
1860:                     shaderSource = VERTEX_SHADER_TANGENTS;
1861:                     break;
1862: 
1863:                 default:
1864:                     return false;
1865:             }
1866: 
1867:             ID3DBlob    *shaderBlob = 0;
1868:             ID3DBlob    *errorBlob  = 0;
1869: 
1870:             HRESULT    hr = D3DCompile(shaderSource, strlen(shaderSource), 0, 0, 0, "main",
1871:                                        "vs_4_0", D3DCOMPILE_SKIP_VALIDATION, 0, &shaderBlob, &errorBlob);
1872: 
1873:             if (FAILED(hr))
1874:             {
1875:                 if (errorBlob)
1876:                 {
1877:                     os::Printer::log("Vertex shader compilation failed:", ELL_ERROR);
1878:                     os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);
1879:                     errorBlob->Release();
1880:                 }
1881: 
1882:                 return false;
1883:             }
1884: 
1885:             if (shaderBlob)
1886:             {
1887:                 hr = m_pID3DDevice->CreateVertexShader(shaderBlob->GetBufferPointer(),
1888:                                                        shaderBlob->GetBufferSize(),
1889:                                                        nullptr,
1890:                                                        &m_BuiltInVertexShader[type]);
1891:                 if (SUCCEEDED(hr))
1892:                 {
1893:                     createInputLayout(type, shaderBlob);
1894:                 }
1895: 
1896:                 shaderBlob->Release();
1897:             }
1898: 
1899:             return true;
1900:         }
1901: 
1902: 
1903:         bool CD3D11Driver::createBuiltInPixelShader(E_VERTEX_TYPE type)
1904:         {
1905:             const char    *shaderSource = 0;
1906: 
1907:             switch (type)
1908:             {
1909:                 case EVT_STANDARD:
1910:                     shaderSource = PIXEL_SHADER_STANDARD;
1911:                     break;
1912: 
1913:                 case EVT_2TCOORDS:
1914:                     shaderSource = PIXEL_SHADER_2TCOORDS;
1915:                     break;
1916: 
1917:                 case EVT_TANGENTS:
1918:                     shaderSource = PIXEL_SHADER_TANGENTS;
1919:                     break;
1920: 
1921:                 default:
1922:                     return false;
1923:             }
1924: 
1925:             ID3DBlob    *shaderBlob = 0;
1926:             ID3DBlob    *errorBlob  = 0;
1927: 
1928:             HRESULT    hr = D3DCompile(shaderSource, strlen(shaderSource), 0, 0, 0, "main",
1929:                                        "ps_4_0", D3DCOMPILE_SKIP_VALIDATION, 0, &shaderBlob, &errorBlob);
1930: 
1931:             if (FAILED(hr))
1932:             {
1933:                 if (errorBlob)
1934:                 {
1935:                     os::Printer::log("Pixel shader compilation failed:", ELL_ERROR);
1936:                     os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);
1937:                     errorBlob->Release();
1938:                 }
1939: 
1940:                 return false;
1941:             }
1942: 
1943:             if (shaderBlob)
1944:             {
1945:                 hr = m_pID3DDevice->CreatePixelShader(shaderBlob->GetBufferPointer(),
1946:                                                       shaderBlob->GetBufferSize(),
1947:                                                       nullptr,
1948:                                                       &m_BuiltInPixelShader[type]);
1949:                 shaderBlob->Release();
1950:             }
1951: 
1952:             return SUCCEEDED(hr);
1953:         }
1954: 
1955: 
1956:         bool CD3D11Driver::createInputLayout(E_VERTEX_TYPE type, ID3DBlob *shaderBlob)
1957:         {
1958:             D3D11_INPUT_ELEMENT_DESC    *layout     = 0;
1959:             u32                         numElements = 0;
1960: 
1961:             switch (type)
1962:             {
1963:                 case EVT_STANDARD:
1964:                 {
1965:                     static D3D11_INPUT_ELEMENT_DESC    standardLayout[] =
1966:                     {
1967:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
1968:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
1969:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
1970:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
1971:                     };
1972:                     layout      = standardLayout;
1973:                     numElements = 4;
1974:                     break;
1975:                 }
1976: 
1977:                 case EVT_2TCOORDS:
1978:                 {
1979:                     static D3D11_INPUT_ELEMENT_DESC    twoTexLayout[] =
1980:                     {
1981:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
1982:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
1983:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
1984:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
1985:                         {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
1986:                     };
1987:                     layout      = twoTexLayout;
1988:                     numElements = 5;
1989:                     break;
1990:                 }
1991: 
1992:                 case EVT_TANGENTS:
1993:                 {
1994:                     static D3D11_INPUT_ELEMENT_DESC    tangentLayout[] =
1995:                     {
1996:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
1997:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
1998:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
1999:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
2000:                         {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
2001:                         {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
2002:                     };
2003:                     layout      = tangentLayout;
2004:                     numElements = 6;
2005:                     break;
2006:                 }
2007: 
2008:                 default:
2009:                     return false;
2010:             }
2011: 
2012:             HRESULT    hr = m_pID3DDevice->CreateInputLayout(layout, numElements,
2013:                                                              shaderBlob->GetBufferPointer(),
2014:                                                              shaderBlob->GetBufferSize(),
2015:                                                              &m_InputLayout[type]);
2016:             return SUCCEEDED(hr);
2017:         }
2018: 
2019: 
2020:         void CD3D11Driver::updateMatrixConstantBuffer()
2021:         {
2022:             core::matrix4    mvp = m_Matrices[ETS_PROJECTION] * m_Matrices[ETS_VIEW] * m_Matrices[ETS_WORLD];
2023: 
2024:             D3D11_MAPPED_SUBRESOURCE    mapped;
2025: 
2026:             if (SUCCEEDED(m_pID3DDeviceContext->Map(m_MatrixConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
2027:             {
2028:                 memcpy(mapped.pData, mvp.pointer(), sizeof(core::matrix4));
2029:                 m_pID3DDeviceContext->Unmap(m_MatrixConstantBuffer, 0);
2030:             }
2031: 
2032:             m_pID3DDeviceContext->VSSetConstantBuffers(0, 1, &m_MatrixConstantBuffer);
2033:         }
2034: 
2035: 
2036:         bool CD3D11Driver::createDefaultStates()
2037:         {
2038:             HRESULT                     hr = E_FAIL;
2039:             D3D11_RASTERIZER_DESC1      rasterizerDesc;
2040:             D3D11_DEPTH_STENCIL_DESC    depthStencilDesc;
2041:             D3D11_BLEND_DESC1           blendDesc;
2042: 
2043:             rasterizerDesc.AntialiasedLineEnable    = false;
2044:             rasterizerDesc.CullMode                 = D3D11_CULL_BACK;

(Showing lines 1845-2044 of 2198. Use offset=2045 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 3.1s)

_Thinking:_

Now I have a good understanding of the structure. Let me look at the beginning of the file to see the shader source strings:

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "limit": 180
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
6: #include "CD3D11Driver.h"
7: 
8: #ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
9: 
10: #include "os.h"
11: #include "S3DVertex.h"
12: #include "SColor.h"
13: #include "CD3D11Texture.h"
14: #include "CD3D11MaterialRenderer.h"
15: #include "CD3D11ShaderMaterialRenderer.h"
16: #include "CD3D11NormalMapRenderer.h"
17: #include "CD3D11ParallaxMapRenderer.h"
18: #include "CD3D11HLSLMaterialRenderer.h"
19: #include "SIrrCreationParameters.h"
20: 
21: namespace irr
22: {
23:     namespace video
24:     {
25:         static const char    VERTEX_SHADER_STANDARD[] =
26:             "struct VS_INPUT {"
27:             "    float3 Pos : POSITION;"
28:             "    float3 Normal : NORMAL;"
29:             "    float4 Color : COLOR;"
30:             "    float2 TexCoord : TEXCOORD0;"
31:             "};"
32:             "struct VS_OUTPUT {"
33:             "    float4 Pos : SV_POSITION;"
34:             "    float4 Color : COLOR;"
35:             "    float2 TexCoord : TEXCOORD0;"
36:             "    float3 Normal : TEXCOORD1;"
37:             "};"
38:             "cbuffer MatrixBuffer : register(b0) {"
39:             "    float4x4 WorldViewProj;"
40:             "};"
41:             "VS_OUTPUT main(VS_INPUT input) {"
42:             "    VS_OUTPUT output;"
43:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
44:             "    output.Color = input.Color;"
45:             "    output.TexCoord = input.TexCoord;"
46:             "    output.Normal = input.Normal;"
47:             "    return output;"
48:             "}";
49: 
50:         static const char    VERTEX_SHADER_2TCOORDS[] =
51:             "struct VS_INPUT {"
52:             "    float3 Pos : POSITION;"
53:             "    float3 Normal : NORMAL;"
54:             "    float4 Color : COLOR;"
55:             "    float2 TexCoord : TEXCOORD0;"
56:             "    float2 TexCoord2 : TEXCOORD1;"
57:             "};"
58:             "struct VS_OUTPUT {"
59:             "    float4 Pos : SV_POSITION;"
60:             "    float4 Color : COLOR;"
61:             "    float2 TexCoord : TEXCOORD0;"
62:             "    float2 TexCoord2 : TEXCOORD1;"
63:             "    float3 Normal : TEXCOORD2;"
64:             "};"
65:             "cbuffer MatrixBuffer : register(b0) {"
66:             "    float4x4 WorldViewProj;"
67:             "};"
68:             "VS_OUTPUT main(VS_INPUT input) {"
69:             "    VS_OUTPUT output;"
70:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
71:             "    output.Color = input.Color;"
72:             "    output.TexCoord = input.TexCoord;"
73:             "    output.TexCoord2 = input.TexCoord2;"
74:             "    output.Normal = input.Normal;"
75:             "    return output;"
76:             "}";
77: 
78:         static const char    VERTEX_SHADER_TANGENTS[] =
79:             "struct VS_INPUT {"
80:             "    float3 Pos : POSITION;"
81:             "    float3 Normal : NORMAL;"
82:             "    float4 Color : COLOR;"
83:             "    float2 TexCoord : TEXCOORD0;"
84:             "    float3 Tangent : TANGENT;"
85:             "    float3 Binormal : BINORMAL;"
86:             "};"
87:             "struct VS_OUTPUT {"
88:             "    float4 Pos : SV_POSITION;"
89:             "    float4 Color : COLOR;"
90:             "    float2 TexCoord : TEXCOORD0;"
91:             "    float3 Normal : TEXCOORD1;"
92:             "    float3 Tangent : TEXCOORD2;"
93:             "    float3 Binormal : TEXCOORD3;"
94:             "};"
95:             "cbuffer MatrixBuffer : register(b0) {"
96:             "    float4x4 WorldViewProj;"
97:             "};"
98:             "VS_OUTPUT main(VS_INPUT input) {"
99:             "    VS_OUTPUT output;"
100:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
101:             "    output.Color = input.Color;"
102:             "    output.TexCoord = input.TexCoord;"
103:             "    output.Normal = input.Normal;"
104:             "    output.Tangent = input.Tangent;"
105:             "    output.Binormal = input.Binormal;"
106:             "    return output;"
107:             "}";
108: 
109:         static const char    PIXEL_SHADER_STANDARD[] =
110:             "cbuffer MatrixBuffer : register(b0) {"
111:             "    float4x4 WorldViewProj;"
112:             "};"
113:             "struct PS_INPUT {"
114:             "    float4 Pos : SV_POSITION;"
115:             "    float4 Color : COLOR;"
116:             "    float2 TexCoord : TEXCOORD0;"
117:             "    float3 Normal : TEXCOORD1;"
118:             "};"
119:             "Texture2D DiffuseTexture : register(t0);"
120:             "SamplerState LinearSampler : register(s0);"
121:             "float4 main(PS_INPUT input) : SV_TARGET {"
122:             "    return DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
123:             "}";
124: 
125:         static const char    PIXEL_SHADER_2TCOORDS[] =
126:             "cbuffer MatrixBuffer : register(b0) {"
127:             "    float4x4 WorldViewProj;"
128:             "};"
129:             "struct PS_INPUT {"
130:             "    float4 Pos : SV_POSITION;"
131:             "    float4 Color : COLOR;"
132:             "    float2 TexCoord : TEXCOORD0;"
133:             "    float2 TexCoord2 : TEXCOORD1;"
134:             "    float3 Normal : TEXCOORD2;"
135:             "};"
136:             "Texture2D DiffuseTexture : register(t0);"
137:             "SamplerState LinearSampler : register(s0);"
138:             "float4 main(PS_INPUT input) : SV_TARGET {"
139:             "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
140:             "    return input.Color * texColor;"
141:             "}";
142: 
143:         static const char    PIXEL_SHADER_TANGENTS[] =
144:             "cbuffer MatrixBuffer : register(b0) {"
145:             "    float4x4 WorldViewProj;"
146:             "};"
147:             "struct PS_INPUT {"
148:             "    float4 Pos : SV_POSITION;"
149:             "    float4 Color : COLOR;"
150:             "    float2 TexCoord : TEXCOORD0;"
151:             "    float3 Normal : TEXCOORD1;"
152:             "    float3 Tangent : TEXCOORD2;"
153:             "    float3 Binormal : TEXCOORD3;"
154:             "};"
155:             "Texture2D DiffuseTexture : register(t0);"
156:             "SamplerState LinearSampler : register(s0);"
157:             "float4 main(PS_INPUT input) : SV_TARGET {"
158:             "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
159:             "    return input.Color * texColor;"
160:             "}";
161: 
162:         CD3D11Driver::CD3D11Driver(const SIrrlichtCreationParameters &params, io::IFileSystem *io)
163:             : CNullDriver(io, params.WindowSize), m_CurrentRenderMode(ERM_NONE),
164:             m_ResetRenderStates(true), m_Transformation3DChanged(false),
165:             m_D3D11Library(0), m_DXGIFactory(0), m_Adapter(0), m_pID3DDevice(0), m_pID3DDeviceContext(0), m_pID3DDevice1(0), m_SwapChain(0),
166:             m_BackBufferRenderTargetView(0), m_DepthStencilView(0),
167:             m_WindowId(0), m_SceneSourceRect(0),
168:             m_LastVertexType((video::E_VERTEX_TYPE)-1), m_VendorID(0),
169:             m_BuiltInShadersInitialized(false),
170:             m_TempVertexBuffer(0), m_TempIndexBuffer(0), m_MatrixConstantBuffer(0),
171:             m_TempVertexBufferSize(0), m_TempIndexBufferSize(0),
172:             m_TempIndexType(EIT_16BIT),
173:             m_RasterizerState(0), m_DepthStencilState(0), m_BlendState(0), m_DefaultSampler(0),
174:             m_MaxTextureUnits(0), m_MaxUserClipPlanes(0), m_MaxMRTs(1), m_NumSetMRTs(1),
175:             m_MaxLightDistance(0.f), m_LastSetLight(-1),
176:             m_ColorFormat(ECOLOR_FORMAT::ECF_A8R8G8B8), m_DeviceRemoved(false),
177:             m_DriverWasReset(true), m_OcclusionQuerySupport(false),
178:             m_AlphaToCoverageSupport(false), m_Params(params)
179:         {
180: #ifdef _DEBUG

(Showing lines 1-180 of 2198. Use offset=181 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 12.9s)

_Thinking:_

Now I understand the structure. I need to create a new shader class `CD3D11Shader` that:
1. Manages all D3D11 shader types (vs/hs/ds/gs/ps/cs)
2. Stores the HLSL source code
3. For VS, stores input layout info and ID3D11InputLayout
4. Stores the compiled ID3DBlob
5. Stores all the shader objects (ID3D11VertexShader, ID3D11PixelShader, etc.)
6. Created, managed, and destroyed by CD3D11Driver

Let me create the header and cpp files for this new class.

Now I'll create the shader class. First the header:


**Tool: write**

**Input:**
```json
{
  "content": "// Copyright (C) 2002-2012 Nikolaus Gebhardt\n// This file is part of the \"Irrlicht Engine\".\n// For conditions of distribution and use, see copyright notice in irrlicht.h\n\n#ifndef __C_D3D11_SHADER_H_INCLUDED__\n#define __C_D3D11_SHADER_H_INCLUDED__\n\n#include \"IrrCompileConfig.h\"\n\n#ifdef _IRR_WINDOWS_\n\n#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_\n\n#include <d3d11.h>\n#include <d3dcompiler.h>\n#include \"IReferenceCounted.h\"\n\nnamespace irr\n{\n    namespace video\n    {\n        class CD3D11Driver;\n\n        enum E_D3D11_SHADER_TYPE\n        {\n            EDST_VERTEX = 0,\n            EDST_HULL,\n            EDST_DOMAIN,\n            EDST_GEOMETRY,\n            EDST_PIXEL,\n            EDST_COMPUTE,\n            EDST_COUNT\n        };\n\n        class CD3D11Shader : public IReferenceCounted\n        {\n        public:\n            CD3D11Shader(CD3D11Driver *driver);\n            virtual ~CD3D11Shader();\n\n            bool compile(E_D3D11_SHADER_TYPE type, const c8 *hlslSource, const c8 *entryPoint, const c8 *profile);\n            bool createVertexShader();\n            bool createHullShader();\n            bool createDomainShader();\n            bool createGeometryShader();\n            bool createPixelShader();\n            bool createComputeShader();\n            bool createInputLayout(const D3D11_INPUT_ELEMENT_DESC *layout, u32 elementCount);\n            void drop();\n\n            ID3D11VertexShader* getVertexShader() const { return m_VertexShader; }\n            ID3D11HullShader* getHullShader() const { return m_HullShader; }\n            ID3D11DomainShader* getDomainShader() const { return m_DomainShader; }\n            ID3D11GeometryShader* getGeometryShader() const { return m_GeometryShader; }\n            ID3D11PixelShader* getPixelShader() const { return m_PixelShader; }\n            ID3D11ComputeShader* getComputeShader() const { return m_ComputeShader; }\n            ID3D11InputLayout* getInputLayout() const { return m_InputLayout; }\n            ID3DBlob* getShaderBlob() const { return m_ShaderBlob; }\n            const core::stringc& getHLSLSource() const { return m_HLSLSource; }\n            E_D3D11_SHADER_TYPE getShaderType() const { return m_ShaderType; }\n            bool isCompiled() const { return m_Compiled; }\n\n        private:\n            CD3D11Driver *m_Driver;\n\n            core::stringc m_HLSLSource;\n            core::stringc m_EntryPoint;\n            core::stringc m_Profile;\n\n            E_D3D11_SHADER_TYPE m_ShaderType;\n            bool m_Compiled;\n\n            ID3DBlob *m_ShaderBlob;\n\n            ID3D11VertexShader *m_VertexShader;\n            ID3D11HullShader *m_HullShader;\n            ID3D11DomainShader *m_DomainShader;\n            ID3D11GeometryShader *m_GeometryShader;\n            ID3D11PixelShader *m_PixelShader;\n            ID3D11ComputeShader *m_ComputeShader;\n\n            ID3D11InputLayout *m_InputLayout;\n            D3D11_INPUT_ELEMENT_DESC *m_InputLayoutDesc;\n            u32 m_InputLayoutElementCount;\n        };\n\n    } // end namespace video\n} // end namespace irr\n\n#endif // _IRR_COMPILE_WITH_DIRECT3D_11_\n#endif // _IRR_WINDOWS_\n#endif // __C_D3D11_SHADER_H_INCLUDED__",
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Shader.h"
}
```

**Output:**
```
Wrote file successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 17.3s)

_Thinking:_

Now I need to create the implementation file:

**Tool: write**

**Input:**
```json
{
  "content": "// Copyright (C) 2002-2012 Nikolaus Gebhardt\n// This file is part of the \"Irrlicht Engine\".\n// For conditions of distribution and use, see copyright notice in irrlicht.h\n\n#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE\n#include \"CD3D11Shader.h\"\n#include \"CD3D11Driver.h\"\n#include \"os.h\"\n\n#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_\n\nnamespace irr\n{\n    namespace video\n    {\n        CD3D11Shader::CD3D11Shader(CD3D11Driver *driver)\n            : m_Driver(driver), m_ShaderType(EDST_COUNT), m_Compiled(false),\n            m_ShaderBlob(0), m_VertexShader(0), m_HullShader(0), m_DomainShader(0),\n            m_GeometryShader(0), m_PixelShader(0), m_ComputeShader(0),\n            m_InputLayout(0), m_InputLayoutDesc(0), m_InputLayoutElementCount(0)\n        {\n#ifdef _DEBUG\n            setDebugName(\"CD3D11Shader\");\n#endif\n        }\n\n        CD3D11Shader::~CD3D11Shader()\n        {\n            if (m_InputLayout)\n                m_InputLayout->Release();\n\n            if (m_InputLayoutDesc)\n                delete[] m_InputLayoutDesc;\n\n            if (m_ShaderBlob)\n                m_ShaderBlob->Release();\n\n            if (m_VertexShader)\n                m_VertexShader->Release();\n\n            if (m_HullShader)\n                m_HullShader->Release();\n\n            if (m_DomainShader)\n                m_DomainShader->Release();\n\n            if (m_GeometryShader)\n                m_GeometryShader->Release();\n\n            if (m_PixelShader)\n                m_PixelShader->Release();\n\n            if (m_ComputeShader)\n                m_ComputeShader->Release();\n        }\n\n        void CD3D11Shader::drop()\n        {\n            if (dropRefCount())\n                delete this;\n        }\n\n        bool CD3D11Shader::compile(E_D3D11_SHADER_TYPE type, const c8 *hlslSource, const c8 *entryPoint, const c8 *profile)\n        {\n            if (!hlslSource || !entryPoint || !profile)\n                return false;\n\n            m_HLSLSource = hlslSource;\n            m_EntryPoint = entryPoint;\n            m_Profile = profile;\n            m_ShaderType = type;\n\n            ID3DBlob *errorBlob = 0;\n\n            HRESULT hr = D3DCompile(hlslSource, strlen(hlslSource), 0, 0, 0, entryPoint,\n                                    profile, D3DCOMPILE_SKIP_VALIDATION, 0, &m_ShaderBlob, &errorBlob);\n\n            if (FAILED(hr))\n            {\n                if (errorBlob)\n                {\n                    os::Printer::log(\"Shader compilation failed:\", ELL_ERROR);\n                    os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);\n                    errorBlob->Release();\n                }\n                return false;\n            }\n\n            m_Compiled = true;\n            return true;\n        }\n\n        bool CD3D11Shader::createVertexShader()\n        {\n            if (!m_Compiled || m_ShaderType != EDST_VERTEX || !m_ShaderBlob)\n                return false;\n\n            if (m_VertexShader)\n            {\n                m_VertexShader->Release();\n                m_VertexShader = 0;\n            }\n\n            HRESULT hr = m_Driver->m_pID3DDevice->CreateVertexShader(\n                m_ShaderBlob->GetBufferPointer(),\n                m_ShaderBlob->GetBufferSize(),\n                nullptr,\n                &m_VertexShader);\n\n            return SUCCEEDED(hr);\n        }\n\n        bool CD3D11Shader::createHullShader()\n        {\n            if (!m_Compiled || m_ShaderType != EDST_HULL || !m_ShaderBlob)\n                return false;\n\n            if (m_HullShader)\n            {\n                m_HullShader->Release();\n                m_HullShader = 0;\n            }\n\n            HRESULT hr = m_Driver->m_pID3DDevice->CreateHullShader(\n                m_ShaderBlob->GetBufferPointer(),\n                m_ShaderBlob->GetBufferSize(),\n                nullptr,\n                &m_HullShader);\n\n            return SUCCEEDED(hr);\n        }\n\n        bool CD3D11Shader::createDomainShader()\n        {\n            if (!m_Compiled || m_ShaderType != EDST_DOMAIN || !m_ShaderBlob)\n                return false;\n\n            if (m_DomainShader)\n            {\n                m_DomainShader->Release();\n                m_DomainShader = 0;\n            }\n\n            HRESULT hr = m_Driver->m_pID3DDevice->CreateDomainShader(\n                m_ShaderBlob->GetBufferPointer(),\n                m_ShaderBlob->GetBufferSize(),\n                nullptr,\n                &m_DomainShader);\n\n            return SUCCEEDED(hr);\n        }\n\n        bool CD3D11Shader::createGeometryShader()\n        {\n            if (!m_Compiled || m_ShaderType != EDST_GEOMETRY || !m_ShaderBlob)\n                return false;\n\n            if (m_GeometryShader)\n            {\n                m_GeometryShader->Release();\n                m_GeometryShader = 0;\n            }\n\n            HRESULT hr = m_Driver->m_pID3DDevice->CreateGeometryShader(\n                m_ShaderBlob->GetBufferPointer(),\n                m_ShaderBlob->GetBufferSize(),\n                nullptr,\n                &m_GeometryShader);\n\n            return SUCCEEDED(hr);\n        }\n\n        bool CD3D11Shader::createPixelShader()\n        {\n            if (!m_Compiled || m_ShaderType != EDST_PIXEL || !m_ShaderBlob)\n                return false;\n\n            if (m_PixelShader)\n            {\n                m_PixelShader->Release();\n                m_PixelShader = 0;\n            }\n\n            HRESULT hr = m_Driver->m_pID3DDevice->CreatePixelShader(\n                m_ShaderBlob->GetBufferPointer(),\n                m_ShaderBlob->GetBufferSize(),\n                nullptr,\n                &m_PixelShader);\n\n            return SUCCEEDED(hr);\n        }\n\n        bool CD3D11Shader::createComputeShader()\n        {\n            if (!m_Compiled || m_ShaderType != EDST_COMPUTE || !m_ShaderBlob)\n                return false;\n\n            if (m_ComputeShader)\n            {\n                m_ComputeShader->Release();\n                m_ComputeShader = 0;\n            }\n\n            HRESULT hr = m_Driver->m_pID3DDevice->CreateComputeShader(\n                m_ShaderBlob->GetBufferPointer(),\n                m_ShaderBlob->GetBufferSize(),\n                nullptr,\n                &m_ComputeShader);\n\n            return SUCCEEDED(hr);\n        }\n\n        bool CD3D11Shader::createInputLayout(const D3D11_INPUT_ELEMENT_DESC *layout, u32 elementCount)\n        {\n            if (!m_Compiled || m_ShaderType != EDST_VERTEX || !m_ShaderBlob)\n                return false;\n\n            if (m_InputLayout)\n            {\n                m_InputLayout->Release();\n                m_InputLayout = 0;\n            }\n\n            if (m_InputLayoutDesc)\n            {\n                delete[] m_InputLayoutDesc;\n                m_InputLayoutDesc = 0;\n            }\n\n            m_InputLayoutDesc = new D3D11_INPUT_ELEMENT_DESC[elementCount];\n            memcpy(m_InputLayoutDesc, layout, sizeof(D3D11_INPUT_ELEMENT_DESC) * elementCount);\n            m_InputLayoutElementCount = elementCount;\n\n            HRESULT hr = m_Driver->m_pID3DDevice->CreateInputLayout(\n                layout,\n                elementCount,\n                m_ShaderBlob->GetBufferPointer(),\n                m_ShaderBlob->GetBufferSize(),\n                &m_InputLayout);\n\n            return SUCCEEDED(hr);\n        }\n\n    } // end namespace video\n} // end namespace irr\n\n#endif // _IRR_COMPILE_WITH_DIRECT3D_11_",
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Shader.cpp"
}
```

**Output:**
```
Wrote file successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 4.3s)

_Thinking:_

Now I need to update CD3D11Driver.h and CD3D11Driver.cpp to:
1. Include the new CD3D11Shader.h
2. Add the shader management methods and members
3. Modify the existing shader creation code to use the new CD3D11Shader class

Let me first check how the existing code uses the shaders, particularly the `createBuiltInVertexShader` and `createBuiltInPixelShader` methods, and how the setShadersByType function works.

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "offset": 1770,
  "limit": 80
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
1770:             const c8 *geometryShaderProgram, const c8 *geometryShaderEntryPointName,
1771:             E_GEOMETRY_SHADER_TYPE gsCompileTarget, scene::E_PRIMITIVE_TYPE inType,
1772:             scene::E_PRIMITIVE_TYPE outType, u32 verticesOut,
1773:             IShaderConstantSetCallBack *callback, E_MATERIAL_TYPE baseMaterial,
1774:             s32 userData, E_GPU_SHADING_LANGUAGE shadingLang)
1775:         {
1776:             return -1;
1777:         }
1778: 
1779: 
1780:         void CD3D11Driver::setShadersByType(video::E_VERTEX_TYPE newType)
1781:         {
1782:             if (newType != m_LastVertexType || !m_BuiltInShadersInitialized)
1783:             {
1784:                 if (!m_BuiltInShadersInitialized)
1785:                 {
1786:                     for (u32 i = 0; i < 3; ++i)
1787:                     {
1788:                         createBuiltInVertexShader((E_VERTEX_TYPE)i);
1789:                         createBuiltInPixelShader((E_VERTEX_TYPE)i);
1790:                     }
1791: 
1792:                     m_BuiltInShadersInitialized = true;
1793:                 }
1794: 
1795:                 if (newType >= 0 && newType < 3 && m_BuiltInVertexShader[newType])
1796:                 {
1797:                     m_pID3DDeviceContext->VSSetShader(m_BuiltInVertexShader[newType], 0, 0);
1798:                     if (m_BuiltInPixelShader[newType])
1799:                     {
1800:                         m_pID3DDeviceContext->PSSetShader(m_BuiltInPixelShader[newType], 0, 0);
1801:                     }
1802: 
1803:                     if (m_InputLayout[newType])
1804:                     {
1805:                         m_pID3DDeviceContext->IASetInputLayout(m_InputLayout[newType]);
1806:                     }
1807:                 }
1808: 
1809:                 m_LastVertexType = newType;
1810:             }
1811: 
1812:             setPSTextureAndSamplerState();
1813:         }
1814: 
1815: 
1816:         void CD3D11Driver::setPSTextureAndSamplerState()
1817:         {
1818:             for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
1819:             {
1820:                 if (m_CurrentTexture[i])
1821:                 {
1822:                     CD3D11Texture               *tex    = static_cast<CD3D11Texture*>(const_cast<ITexture*>(m_CurrentTexture[i]));
1823:                     ID3D11ShaderResourceView    *srv    = tex->getShaderResourceView();
1824: 
1825:                     m_pID3DDeviceContext->PSSetShaderResources(i, 1, &srv);
1826: 
1827:                     if (m_DefaultSampler)
1828:                     {
1829:                         ID3D11SamplerState    *pSampler = m_DefaultSampler->getD3D11SamplerState();
1830:                         m_pID3DDeviceContext->PSSetSamplers(i, 1, &pSampler);
1831:                     }
1832:                 }
1833:                 else
1834:                 {
1835:                     ID3D11SamplerState          *pSampler   = nullptr;
1836:                     ID3D11ShaderResourceView    *nullSrv    = 0;
1837: 
1838:                     m_pID3DDeviceContext->PSSetShaderResources(i, 1, &nullSrv);
1839:                     m_pID3DDeviceContext->PSSetSamplers(i, 1, &pSampler);
1840:                 }
1841:             }
1842:         }
1843: 
1844: 
1845:         bool CD3D11Driver::createBuiltInVertexShader(E_VERTEX_TYPE type)
1846:         {
1847:             const char    *shaderSource = 0;
1848: 
1849:             switch (type)

(Showing lines 1770-1849 of 2198. Use offset=1850 to continue.)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "offset": 1700,
  "limit": 100
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
1700:                 case DXGI_FORMAT_R32G32B32A32_FLOAT:
1701:                     return ECOLOR_FORMAT::ECF_A32B32G32R32F;
1702: 
1703:                 default:
1704:                     return ECOLOR_FORMAT::ECF_A8R8G8B8;
1705:             }
1706:         }
1707: 
1708: 
1709:         void CD3D11Driver::createMaterialRenderers()
1710:         {
1711:             s32    matType = -1;
1712: 
1713:             new CD3D11MaterialRenderer(this, matType, "solid");
1714:             new CD3D11MaterialRenderer(this, matType, "solid_lightmap");
1715:             new CD3D11MaterialRenderer(this, matType, "solid_2_layer");
1716:             new CD3D11MaterialRenderer(this, matType, "translucent");
1717:             new CD3D11MaterialRenderer(this, matType, "translucent_2_layer");
1718:             new CD3D11MaterialRenderer(this, matType, "translucent_add_color");
1719:             new CD3D11MaterialRenderer(this, matType, "translucent_vertex_alpha");
1720:             new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel");
1721:             new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel_ref");
1722:             new CD3D11MaterialRenderer(this, matType, "one_texture_blend");
1723:             new CD3D11MaterialRenderer(this, matType, "lightmap_blend");
1724:             new CD3D11MaterialRenderer(this, matType, "detail_map");
1725:             new CD3D11MaterialRenderer(this, matType, "sphere_map");
1726:             new CD3D11MaterialRenderer(this, matType, "reflection_2_layer");
1727:             new CD3D11MaterialRenderer(this, matType, "transparent_reflection_2_layer");
1728: 
1729:             if (queryFeature(video::EVDF_PIXEL_SHADER_1_1) && queryFeature(video::EVDF_VERTEX_SHADER_1_1))
1730:             {
1731:                 new CD3D11NormalMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));
1732:                 new CD3D11ParallaxMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));
1733:             }
1734:         }
1735: 
1736: 
1737:         D3D11_TEXTURE_ADDRESS_MODE CD3D11Driver::getTextureWrapMode(const u8 clamp) const
1738:         {
1739:             return D3D11_TEXTURE_ADDRESS_WRAP;
1740:         }
1741: 
1742: 
1743:         bool CD3D11Driver::reset()
1744:         {
1745:             return false;
1746:         }
1747: 
1748: 
1749:         video::ITexture* CD3D11Driver::createDeviceDependentTexture(IImage *surface, const io::path &name, void *mipmapData)
1750:         {
1751:             return new CD3D11Texture(surface, this, TextureCreationFlags, name, mipmapData);
1752:         }
1753: 
1754: 
1755:         void CD3D11Driver::checkDepthBuffer(ITexture *tex)
1756:         {}
1757: 
1758: 
1759:         s32 CD3D11Driver::addShaderMaterial(const c8 *vertexShaderProgram, const c8 *pixelShaderProgram,
1760:                                             IShaderConstantSetCallBack *callback, E_MATERIAL_TYPE baseMaterial, s32 userData)
1761:         {
1762:             return -1;
1763:         }
1764: 
1765: 
1766:         s32 CD3D11Driver::addHighLevelShaderMaterial(
1767:             const c8 *vertexShaderProgram, const c8 *vertexShaderEntryPointName,
1768:             E_VERTEX_SHADER_TYPE vsCompileTarget, const c8 *pixelShaderProgram,
1769:             const c8 *pixelShaderEntryPointName, E_PIXEL_SHADER_TYPE psCompileTarget,
1770:             const c8 *geometryShaderProgram, const c8 *geometryShaderEntryPointName,
1771:             E_GEOMETRY_SHADER_TYPE gsCompileTarget, scene::E_PRIMITIVE_TYPE inType,
1772:             scene::E_PRIMITIVE_TYPE outType, u32 verticesOut,
1773:             IShaderConstantSetCallBack *callback, E_MATERIAL_TYPE baseMaterial,
1774:             s32 userData, E_GPU_SHADING_LANGUAGE shadingLang)
1775:         {
1776:             return -1;
1777:         }
1778: 
1779: 
1780:         void CD3D11Driver::setShadersByType(video::E_VERTEX_TYPE newType)
1781:         {
1782:             if (newType != m_LastVertexType || !m_BuiltInShadersInitialized)
1783:             {
1784:                 if (!m_BuiltInShadersInitialized)
1785:                 {
1786:                     for (u32 i = 0; i < 3; ++i)
1787:                     {
1788:                         createBuiltInVertexShader((E_VERTEX_TYPE)i);
1789:                         createBuiltInPixelShader((E_VERTEX_TYPE)i);
1790:                     }
1791: 
1792:                     m_BuiltInShadersInitialized = true;
1793:                 }
1794: 
1795:                 if (newType >= 0 && newType < 3 && m_BuiltInVertexShader[newType])
1796:                 {
1797:                     m_pID3DDeviceContext->VSSetShader(m_BuiltInVertexShader[newType], 0, 0);
1798:                     if (m_BuiltInPixelShader[newType])
1799:                     {

(Showing lines 1700-1799 of 2198. Use offset=1800 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 5.4s)

_Thinking:_

Now I need to update CD3D11Driver.h to:
1. Add include for CD3D11Shader.h
2. Add new shader management members and methods

And update CD3D11Driver.cpp to:
1. Include the new CD3D11Shader.h
2. Modify createBuiltInVertexShader and createBuiltInPixelShader to use CD3D11Shader

Let me modify the CD3D11Driver.h first:

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.h",
  "oldString": "class CD3D11ShaderMaterialRenderer;\n        class CD3D11NormalMapRenderer;\n        class CD3D11ParallaxMapRenderer;",
  "newString": "class CD3D11ShaderMaterialRenderer;\n        class CD3D11NormalMapRenderer;\n        class CD3D11ParallaxMapRenderer;\n        class CD3D11Shader;"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 4.2s)

_Thinking:_

Now I need to add a core::array to store CD3D11Shader objects and modify the existing shader storage. Let me add the member to CD3D11Driver.h:

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.h",
  "oldString": "void removeDepthSurface(SD3D11DepthStencilView *depth);",
  "newString": "void removeDepthSurface(SD3D11DepthStencilView *depth);\n\n            core::array<CD3D11Shader*> m_ShaderPool;"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 4.8s)

_Thinking:_

Now I need to modify the CD3D11Driver.cpp to use the new CD3D11Shader class. I'll:
1. Add include for CD3D11Shader.h
2. Modify the createBuiltInVertexShader and createBuiltInPixelShader to use CD3D11Shader

Let me update the includes first:

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "oldString": "#include \"CD3D11NormalMapRenderer.h\"\n#include \"CD3D11ParallaxMapRenderer.h\"\n#include \"CD3D11HLSLMaterialRenderer.h\"",
  "newString": "#include \"CD3D11NormalMapRenderer.h\"\n#include \"CD3D11ParallaxMapRenderer.h\"\n#include \"CD3D11HLSLMaterialRenderer.h\"\n#include \"CD3D11Shader.h\""
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 3.1s)

_Thinking:_

Now I need to modify the createBuiltInVertexShader to use CD3D11Shader. Let me first look at the full method again:

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "offset": 1845,
  "limit": 60
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
1845: 
1846:         bool CD3D11Driver::createBuiltInVertexShader(E_VERTEX_TYPE type)
1847:         {
1848:             const char    *shaderSource = 0;
1849: 
1850:             switch (type)
1851:             {
1852:                 case EVT_STANDARD:
1853:                     shaderSource = VERTEX_SHADER_STANDARD;
1854:                     break;
1855: 
1856:                 case EVT_2TCOORDS:
1857:                     shaderSource = VERTEX_SHADER_2TCOORDS;
1858:                     break;
1859: 
1860:                 case EVT_TANGENTS:
1861:                     shaderSource = VERTEX_SHADER_TANGENTS;
1862:                     break;
1863: 
1864:                 default:
1865:                     return false;
1866:             }
1867: 
1868:             ID3DBlob    *shaderBlob = 0;
1869:             ID3DBlob    *errorBlob  = 0;
1870: 
1871:             HRESULT    hr = D3DCompile(shaderSource, strlen(shaderSource), 0, 0, 0, "main",
1872:                                        "vs_4_0", D3DCOMPILE_SKIP_VALIDATION, 0, &shaderBlob, &errorBlob);
1873: 
1874:             if (FAILED(hr))
1875:             {
1876:                 if (errorBlob)
1877:                 {
1878:                     os::Printer::log("Vertex shader compilation failed:", ELL_ERROR);
1879:                     os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);
1880:                     errorBlob->Release();
1881:                 }
1882: 
1883:                 return false;
1884:             }
1885: 
1886:             if (shaderBlob)
1887:             {
1888:                 hr = m_pID3DDevice->CreateVertexShader(shaderBlob->GetBufferPointer(),
1889:                                                        shaderBlob->GetBufferSize(),
1890:                                                        nullptr,
1891:                                                        &m_BuiltInVertexShader[type]);
1892:                 if (SUCCEEDED(hr))
1893:                 {
1894:                     createInputLayout(type, shaderBlob);
1895:                 }
1896: 
1897:                 shaderBlob->Release();
1898:             }
1899: 
1900:             return true;
1901:         }
1902: 
1903: 
1904:         bool CD3D11Driver::createBuiltInPixelShader(E_VERTEX_TYPE type)

(Showing lines 1845-1904 of 2199. Use offset=1905 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 15.4s)

_Thinking:_

Now I need to replace the createBuiltInVertexShader to use the new CD3D11Shader class. The new version will:1. Create a CD3D11Shader object
2. Compile it using the shader source3. Create the vertex shader4. Create the input layout

Let me also look at the createInputLayout function to see the layout definitions:

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "oldString": "bool CD3D11Driver::createBuiltInVertexShader(E_VERTEX_TYPE type)\n        {\n            const char    *shaderSource = 0;\n\n            switch (type)\n            {\n                case EVT_STANDARD:\n                    shaderSource = VERTEX_SHADER_STANDARD;\n                    break;\n\n                case EVT_2TCOORDS:\n                    shaderSource = VERTEX_SHADER_2TCOORDS;\n                    break;\n\n                case EVT_TANGENTS:\n                    shaderSource = VERTEX_SHADER_TANGENTS;\n                    break;\n\n                default:\n                    return false;\n            }\n\n            ID3DBlob    *shaderBlob = 0;\n            ID3DBlob    *errorBlob  = 0;\n\n            HRESULT    hr = D3DCompile(shaderSource, strlen(shaderSource), 0, 0, 0, \"main\",\n                                       \"vs_4_0\", D3DCOMPILE_SKIP_VALIDATION, 0, &shaderBlob, &errorBlob);\n\n            if (FAILED(hr))\n            {\n                if (errorBlob)\n                {\n                    os::Printer::log(\"Vertex shader compilation failed:\", ELL_ERROR);\n                    os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);\n                    errorBlob->Release();\n                }\n\n                return false;\n            }\n\n            if (shaderBlob)\n            {\n                hr = m_pID3DDevice->CreateVertexShader(shaderBlob->GetBufferPointer(),\n                                                       shaderBlob->GetBufferSize(),\n                                                       nullptr,\n                                                       &m_BuiltInVertexShader[type]);\n                if (SUCCEEDED(hr))\n                {\n                    createInputLayout(type, shaderBlob);\n                }\n\n                shaderBlob->Release();\n            }\n\n            return true;\n        }",
  "newString": "bool CD3D11Driver::createBuiltInVertexShader(E_VERTEX_TYPE type)\n        {\n            const char    *shaderSource = 0;\n\n            switch (type)\n            {\n                case EVT_STANDARD:\n                    shaderSource = VERTEX_SHADER_STANDARD;\n                    break;\n\n                case EVT_2TCOORDS:\n                    shaderSource = VERTEX_SHADER_2TCOORDS;\n                    break;\n\n                case EVT_TANGENTS:\n                    shaderSource = VERTEX_SHADER_TANGENTS;\n                    break;\n\n                default:\n                    return false;\n            }\n\n            CD3D11Shader *shader = new CD3D11Shader(this);\n            if (!shader->compile(EDST_VERTEX, shaderSource, \"main\", \"vs_4_0\"))\n            {\n                shader->drop();\n                return false;\n            }\n\n            if (!shader->createVertexShader())\n            {\n                shader->drop();\n                return false;\n            }\n\n            D3D11_INPUT_ELEMENT_DESC *layout = 0;\n            u32 numElements = 0;\n\n            switch (type)\n            {\n                case EVT_STANDARD:\n                {\n                    static D3D11_INPUT_ELEMENT_DESC standardLayout[] =\n                    {\n                        {\"POSITION\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"NORMAL\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"COLOR\", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"TEXCOORD\", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                    };\n                    layout = standardLayout;\n                    numElements = 4;\n                    break;\n                }\n\n                case EVT_2TCOORDS:\n                {\n                    static D3D11_INPUT_ELEMENT_DESC twoTexLayout[] =\n                    {\n                        {\"POSITION\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"NORMAL\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"COLOR\", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"TEXCOORD\", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"TEXCOORD\", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                    };\n                    layout = twoTexLayout;\n                    numElements = 5;\n                    break;\n                }\n\n                case EVT_TANGENTS:\n                {\n                    static D3D11_INPUT_ELEMENT_DESC tangentLayout[] =\n                    {\n                        {\"POSITION\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"NORMAL\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"COLOR\", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"TEXCOORD\", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"TANGENT\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                        {\"BINORMAL\", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},\n                    };\n                    layout = tangentLayout;\n                    numElements = 6;\n                    break;\n                }\n\n                default:\n                    shader->drop();\n                    return false;\n            }\n\n            if (!shader->createInputLayout(layout, numElements))\n            {\n                shader->drop();\n                return false;\n            }\n\n            m_ShaderPool.push_back(shader);\n\n            if (m_BuiltInVertexShader[type])\n                m_BuiltInVertexShader[type]->Release();\n            m_BuiltInVertexShader[type] = shader->getVertexShader();\n            m_BuiltInVertexShader[type]->AddRef();\n\n            if (m_InputLayout[type])\n                m_InputLayout[type]->Release();\n            m_InputLayout[type] = shader->getInputLayout();\n            m_InputLayout[type]->AddRef();\n\n            return true;\n        }"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 2.7s)

_Thinking:_

Now I need to update createBuiltInPixelShader similarly:

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "offset": 1940,
  "limit": 70
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
1940:             }
1941: 
1942:             m_ShaderPool.push_back(shader);
1943: 
1944:             if (m_BuiltInVertexShader[type])
1945:                 m_BuiltInVertexShader[type]->Release();
1946:             m_BuiltInVertexShader[type] = shader->getVertexShader();
1947:             m_BuiltInVertexShader[type]->AddRef();
1948: 
1949:             if (m_InputLayout[type])
1950:                 m_InputLayout[type]->Release();
1951:             m_InputLayout[type] = shader->getInputLayout();
1952:             m_InputLayout[type]->AddRef();
1953: 
1954:             return true;
1955:         }
1956: 
1957: 
1958:         bool CD3D11Driver::createBuiltInPixelShader(E_VERTEX_TYPE type)
1959:         {
1960:             const char    *shaderSource = 0;
1961: 
1962:             switch (type)
1963:             {
1964:                 case EVT_STANDARD:
1965:                     shaderSource = PIXEL_SHADER_STANDARD;
1966:                     break;
1967: 
1968:                 case EVT_2TCOORDS:
1969:                     shaderSource = PIXEL_SHADER_2TCOORDS;
1970:                     break;
1971: 
1972:                 case EVT_TANGENTS:
1973:                     shaderSource = PIXEL_SHADER_TANGENTS;
1974:                     break;
1975: 
1976:                 default:
1977:                     return false;
1978:             }
1979: 
1980:             ID3DBlob    *shaderBlob = 0;
1981:             ID3DBlob    *errorBlob  = 0;
1982: 
1983:             HRESULT    hr = D3DCompile(shaderSource, strlen(shaderSource), 0, 0, 0, "main",
1984:                                        "ps_4_0", D3DCOMPILE_SKIP_VALIDATION, 0, &shaderBlob, &errorBlob);
1985: 
1986:             if (FAILED(hr))
1987:             {
1988:                 if (errorBlob)
1989:                 {
1990:                     os::Printer::log("Pixel shader compilation failed:", ELL_ERROR);
1991:                     os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);
1992:                     errorBlob->Release();
1993:                 }
1994: 
1995:                 return false;
1996:             }
1997: 
1998:             if (shaderBlob)
1999:             {
2000:                 hr = m_pID3DDevice->CreatePixelShader(shaderBlob->GetBufferPointer(),
2001:                                                       shaderBlob->GetBufferSize(),
2002:                                                       nullptr,
2003:                                                       &m_BuiltInPixelShader[type]);
2004:                 shaderBlob->Release();
2005:             }
2006: 
2007:             return SUCCEEDED(hr);
2008:         }
2009: 

(Showing lines 1940-2009 of 2253. Use offset=2010 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 7.4s)

_Thinking:_

Now I'll update the createBuiltInPixelShader method to use CD3D11Shader:

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "oldString": "bool CD3D11Driver::createBuiltInPixelShader(E_VERTEX_TYPE type)\n        {\n            const char    *shaderSource = 0;\n\n            switch (type)\n            {\n                case EVT_STANDARD:\n                    shaderSource = PIXEL_SHADER_STANDARD;\n                    break;\n\n                case EVT_2TCOORDS:\n                    shaderSource = PIXEL_SHADER_2TCOORDS;\n                    break;\n\n                case EVT_TANGENTS:\n                    shaderSource = PIXEL_SHADER_TANGENTS;\n                    break;\n\n                default:\n                    return false;\n            }\n\n            ID3DBlob    *shaderBlob = 0;\n            ID3DBlob    *errorBlob  = 0;\n\n            HRESULT    hr = D3DCompile(shaderSource, strlen(shaderSource), 0, 0, 0, \"main\",\n                                       \"ps_4_0\", D3DCOMPILE_SKIP_VALIDATION, 0, &shaderBlob, &errorBlob);\n\n            if (FAILED(hr))\n            {\n                if (errorBlob)\n                {\n                    os::Printer::log(\"Pixel shader compilation failed:\", ELL_ERROR);\n                    os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);\n                    errorBlob->Release();\n                }\n\n                return false;\n            }\n\n            if (shaderBlob)\n            {\n                hr = m_pID3DDevice->CreatePixelShader(shaderBlob->GetBufferPointer(),\n                                                      shaderBlob->GetBufferSize(),\n                                                      nullptr,\n                                                      &m_BuiltInPixelShader[type]);\n                shaderBlob->Release();\n            }\n\n            return SUCCEEDED(hr);\n        }",
  "newString": "bool CD3D11Driver::createBuiltInPixelShader(E_VERTEX_TYPE type)\n        {\n            const char    *shaderSource = 0;\n\n            switch (type)\n            {\n                case EVT_STANDARD:\n                    shaderSource = PIXEL_SHADER_STANDARD;\n                    break;\n\n                case EVT_2TCOORDS:\n                    shaderSource = PIXEL_SHADER_2TCOORDS;\n                    break;\n\n                case EVT_TANGENTS:\n                    shaderSource = PIXEL_SHADER_TANGENTS;\n                    break;\n\n                default:\n                    return false;\n            }\n\n            CD3D11Shader *shader = new CD3D11Shader(this);\n            if (!shader->compile(EDST_PIXEL, shaderSource, \"main\", \"ps_4_0\"))\n            {\n                shader->drop();\n                return false;\n            }\n\n            if (!shader->createPixelShader())\n            {\n                shader->drop();\n                return false;\n            }\n\n            m_ShaderPool.push_back(shader);\n\n            if (m_BuiltInPixelShader[type])\n                m_BuiltInPixelShader[type]->Release();\n            m_BuiltInPixelShader[type] = shader->getPixelShader();\n            m_BuiltInPixelShader[type]->AddRef();\n\n            return true;\n        }"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 3.0s)

_Thinking:_

Now I need to check if the old `createInputLayout` method is still used and if I should remove it since I integrated it into `createBuiltInVertexShader`. Let me search for any remaining references to `createInputLayout`:

**Tool: grep**

**Input:**
```json
{
  "pattern": "createInputLayout",
  "include": "*.cpp"
}
```

**Output:**
```
Found 3 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp:
  Line 1936:             if (!shader->createInputLayout(layout, numElements))

  Line 2004:         bool CD3D11Driver::createInputLayout(E_VERTEX_TYPE type, ID3DBlob *shaderBlob)


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Shader.cpp:
  Line 213:         bool CD3D11Shader::createInputLayout(const D3D11_INPUT_ELEMENT_DESC *layout, u32 elementCount)

```

---

## Assistant (Build · MiniMax-M2.7 · 5.1s)

_Thinking:_

The old `CD3D11Driver::createInputLayout` method is no longer being used, but let me check if it's still defined in the header file and if I should keep it or remove it. Let me also verify that the code compiles properly. Let me check if there are any compilation issues.

Actually, since we have the new `CD3D11Shader::createInputLayout` method, we don't need the old `CD3D11Driver::createInputLayout` anymore. But since it's still in the code, I should leave it for now in case some other code uses it.

Let me verify the implementation is complete. Let me look at the current state of the CD3D11Driver.cpp to make sure everything is properly connected:

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "offset": 2000,
  "limit": 100
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
2000:             return true;
2001:         }
2002: 
2003: 
2004:         bool CD3D11Driver::createInputLayout(E_VERTEX_TYPE type, ID3DBlob *shaderBlob)
2005:         {
2006:             D3D11_INPUT_ELEMENT_DESC    *layout     = 0;
2007:             u32                         numElements = 0;
2008: 
2009:             switch (type)
2010:             {
2011:                 case EVT_STANDARD:
2012:                 {
2013:                     static D3D11_INPUT_ELEMENT_DESC    standardLayout[] =
2014:                     {
2015:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
2016:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
2017:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
2018:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
2019:                     };
2020:                     layout      = standardLayout;
2021:                     numElements = 4;
2022:                     break;
2023:                 }
2024: 
2025:                 case EVT_2TCOORDS:
2026:                 {
2027:                     static D3D11_INPUT_ELEMENT_DESC    twoTexLayout[] =
2028:                     {
2029:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
2030:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
2031:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
2032:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
2033:                         {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
2034:                     };
2035:                     layout      = twoTexLayout;
2036:                     numElements = 5;
2037:                     break;
2038:                 }
2039: 
2040:                 case EVT_TANGENTS:
2041:                 {
2042:                     static D3D11_INPUT_ELEMENT_DESC    tangentLayout[] =
2043:                     {
2044:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
2045:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
2046:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
2047:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
2048:                         {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
2049:                         {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
2050:                     };
2051:                     layout      = tangentLayout;
2052:                     numElements = 6;
2053:                     break;
2054:                 }
2055: 
2056:                 default:
2057:                     return false;
2058:             }
2059: 
2060:             HRESULT    hr = m_pID3DDevice->CreateInputLayout(layout, numElements,
2061:                                                              shaderBlob->GetBufferPointer(),
2062:                                                              shaderBlob->GetBufferSize(),
2063:                                                              &m_InputLayout[type]);
2064:             return SUCCEEDED(hr);
2065:         }
2066: 
2067: 
2068:         void CD3D11Driver::updateMatrixConstantBuffer()
2069:         {
2070:             core::matrix4    mvp = m_Matrices[ETS_PROJECTION] * m_Matrices[ETS_VIEW] * m_Matrices[ETS_WORLD];
2071: 
2072:             D3D11_MAPPED_SUBRESOURCE    mapped;
2073: 
2074:             if (SUCCEEDED(m_pID3DDeviceContext->Map(m_MatrixConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
2075:             {
2076:                 memcpy(mapped.pData, mvp.pointer(), sizeof(core::matrix4));
2077:                 m_pID3DDeviceContext->Unmap(m_MatrixConstantBuffer, 0);
2078:             }
2079: 
2080:             m_pID3DDeviceContext->VSSetConstantBuffers(0, 1, &m_MatrixConstantBuffer);
2081:         }
2082: 
2083: 
2084:         bool CD3D11Driver::createDefaultStates()
2085:         {
2086:             HRESULT                     hr = E_FAIL;
2087:             D3D11_RASTERIZER_DESC1      rasterizerDesc;
2088:             D3D11_DEPTH_STENCIL_DESC    depthStencilDesc;
2089:             D3D11_BLEND_DESC1           blendDesc;
2090: 
2091:             rasterizerDesc.AntialiasedLineEnable    = false;
2092:             rasterizerDesc.CullMode                 = D3D11_CULL_BACK;
2093:             rasterizerDesc.DepthBias                = D3D11_DEFAULT_DEPTH_BIAS;
2094:             rasterizerDesc.DepthBiasClamp           = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
2095:             rasterizerDesc.DepthClipEnable          = true;
2096:             rasterizerDesc.FillMode                 = D3D11_FILL_SOLID;
2097:             rasterizerDesc.ForcedSampleCount        = 0;
2098:             rasterizerDesc.FrontCounterClockwise    = false;
2099:             rasterizerDesc.MultisampleEnable        = false;

(Showing lines 2000-2099 of 2246. Use offset=2100 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 6.6s)

_Thinking:_

The old `createInputLayout` method is still there but no longer being used by `createBuiltInVertexShader`. However, since it's a private method of CD3D11Driver, it's not causing any harm. But I should probably remove it to clean up the code. However, for safety, let me leave it for now.

Let me verify the changes by checking if everything is in place. The implementation looks correct. Let me summarize what was done:

1. Created `CD3D11Shader.h` - a new class that:
   - Manages all D3D11 shader types (VS, HS, DS, GS, PS, CS)
   - Stores HLSL source code
   - Stores compiled ID3DBlob
   - Stores ID3D11InputLayout for vertex shaders
   - Stores all shader objects (ID3D11VertexShader, ID3D11PixelShader, etc.)
   - Uses IReferenceCounted for memory management

2. Created `CD3D11Shader.cpp` - implementation of the shader class

3. Updated `CD3D11Driver.h`:
   - Added forward declaration for CD3D11Shader
   - Added m_ShaderPool core::array to hold all shader objects

4. Updated `CD3D11Driver.cpp`:
   - Added include for CD3D11Shader.h
   - Modified `createBuiltInVertexShader` to use CD3D11Shader
   - Modified `createBuiltInPixelShader` to use CD3D11Shader

The code should compile now. Let me check if there are any issues.

Let me verify the implementation is correct by checking the full code structure:


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Shader.h"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Shader.h</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #ifndef __C_D3D11_SHADER_H_INCLUDED__
6: #define __C_D3D11_SHADER_H_INCLUDED__
7: 
8: #include "IrrCompileConfig.h"
9: 
10: #ifdef _IRR_WINDOWS_
11: 
12: #ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
13: 
14: #include <d3d11.h>
15: #include <d3dcompiler.h>
16: #include "IReferenceCounted.h"
17: 
18: namespace irr
19: {
20:     namespace video
21:     {
22:         class CD3D11Driver;
23: 
24:         enum E_D3D11_SHADER_TYPE
25:         {
26:             EDST_VERTEX = 0,
27:             EDST_HULL,
28:             EDST_DOMAIN,
29:             EDST_GEOMETRY,
30:             EDST_PIXEL,
31:             EDST_COMPUTE,
32:             EDST_COUNT
33:         };
34: 
35:         class CD3D11Shader : public IReferenceCounted
36:         {
37:         public:
38:             CD3D11Shader(CD3D11Driver *driver);
39:             virtual ~CD3D11Shader();
40: 
41:             bool compile(E_D3D11_SHADER_TYPE type, const c8 *hlslSource, const c8 *entryPoint, const c8 *profile);
42:             bool createVertexShader();
43:             bool createHullShader();
44:             bool createDomainShader();
45:             bool createGeometryShader();
46:             bool createPixelShader();
47:             bool createComputeShader();
48:             bool createInputLayout(const D3D11_INPUT_ELEMENT_DESC *layout, u32 elementCount);
49:             void drop();
50: 
51:             ID3D11VertexShader* getVertexShader() const { return m_VertexShader; }
52:             ID3D11HullShader* getHullShader() const { return m_HullShader; }
53:             ID3D11DomainShader* getDomainShader() const { return m_DomainShader; }
54:             ID3D11GeometryShader* getGeometryShader() const { return m_GeometryShader; }
55:             ID3D11PixelShader* getPixelShader() const { return m_PixelShader; }
56:             ID3D11ComputeShader* getComputeShader() const { return m_ComputeShader; }
57:             ID3D11InputLayout* getInputLayout() const { return m_InputLayout; }
58:             ID3DBlob* getShaderBlob() const { return m_ShaderBlob; }
59:             const core::stringc& getHLSLSource() const { return m_HLSLSource; }
60:             E_D3D11_SHADER_TYPE getShaderType() const { return m_ShaderType; }
61:             bool isCompiled() const { return m_Compiled; }
62: 
63:         private:
64:             CD3D11Driver *m_Driver;
65: 
66:             core::stringc m_HLSLSource;
67:             core::stringc m_EntryPoint;
68:             core::stringc m_Profile;
69: 
70:             E_D3D11_SHADER_TYPE m_ShaderType;
71:             bool m_Compiled;
72: 
73:             ID3DBlob *m_ShaderBlob;
74: 
75:             ID3D11VertexShader *m_VertexShader;
76:             ID3D11HullShader *m_HullShader;
77:             ID3D11DomainShader *m_DomainShader;
78:             ID3D11GeometryShader *m_GeometryShader;
79:             ID3D11PixelShader *m_PixelShader;
80:             ID3D11ComputeShader *m_ComputeShader;
81: 
82:             ID3D11InputLayout *m_InputLayout;
83:             D3D11_INPUT_ELEMENT_DESC *m_InputLayoutDesc;
84:             u32 m_InputLayoutElementCount;
85:         };
86: 
87:     } // end namespace video
88: } // end namespace irr
89: 
90: #endif // _IRR_COMPILE_WITH_DIRECT3D_11_
91: #endif // _IRR_WINDOWS_
92: #endif // __C_D3D11_SHADER_H_INCLUDED__

(End of file - total 92 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 3.2s)

_Thinking:_

The header file looks good. Let me also check the implementation file to make sure it's complete:

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Shader.cpp"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Shader.cpp</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
6: #include "CD3D11Shader.h"
7: #include "CD3D11Driver.h"
8: #include "os.h"
9: 
10: #ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
11: 
12: namespace irr
13: {
14:     namespace video
15:     {
16:         CD3D11Shader::CD3D11Shader(CD3D11Driver *driver)
17:             : m_Driver(driver), m_ShaderType(EDST_COUNT), m_Compiled(false),
18:             m_ShaderBlob(0), m_VertexShader(0), m_HullShader(0), m_DomainShader(0),
19:             m_GeometryShader(0), m_PixelShader(0), m_ComputeShader(0),
20:             m_InputLayout(0), m_InputLayoutDesc(0), m_InputLayoutElementCount(0)
21:         {
22: #ifdef _DEBUG
23:             setDebugName("CD3D11Shader");
24: #endif
25:         }
26: 
27:         CD3D11Shader::~CD3D11Shader()
28:         {
29:             if (m_InputLayout)
30:                 m_InputLayout->Release();
31: 
32:             if (m_InputLayoutDesc)
33:                 delete[] m_InputLayoutDesc;
34: 
35:             if (m_ShaderBlob)
36:                 m_ShaderBlob->Release();
37: 
38:             if (m_VertexShader)
39:                 m_VertexShader->Release();
40: 
41:             if (m_HullShader)
42:                 m_HullShader->Release();
43: 
44:             if (m_DomainShader)
45:                 m_DomainShader->Release();
46: 
47:             if (m_GeometryShader)
48:                 m_GeometryShader->Release();
49: 
50:             if (m_PixelShader)
51:                 m_PixelShader->Release();
52: 
53:             if (m_ComputeShader)
54:                 m_ComputeShader->Release();
55:         }
56: 
57:         void CD3D11Shader::drop()
58:         {
59:             if (dropRefCount())
60:                 delete this;
61:         }
62: 
63:         bool CD3D11Shader::compile(E_D3D11_SHADER_TYPE type, const c8 *hlslSource, const c8 *entryPoint, const c8 *profile)
64:         {
65:             if (!hlslSource || !entryPoint || !profile)
66:                 return false;
67: 
68:             m_HLSLSource = hlslSource;
69:             m_EntryPoint = entryPoint;
70:             m_Profile = profile;
71:             m_ShaderType = type;
72: 
73:             ID3DBlob *errorBlob = 0;
74: 
75:             HRESULT hr = D3DCompile(hlslSource, strlen(hlslSource), 0, 0, 0, entryPoint,
76:                                     profile, D3DCOMPILE_SKIP_VALIDATION, 0, &m_ShaderBlob, &errorBlob);
77: 
78:             if (FAILED(hr))
79:             {
80:                 if (errorBlob)
81:                 {
82:                     os::Printer::log("Shader compilation failed:", ELL_ERROR);
83:                     os::Printer::log((const c8*)errorBlob->GetBufferPointer(), ELL_ERROR);
84:                     errorBlob->Release();
85:                 }
86:                 return false;
87:             }
88: 
89:             m_Compiled = true;
90:             return true;
91:         }
92: 
93:         bool CD3D11Shader::createVertexShader()
94:         {
95:             if (!m_Compiled || m_ShaderType != EDST_VERTEX || !m_ShaderBlob)
96:                 return false;
97: 
98:             if (m_VertexShader)
99:             {
100:                 m_VertexShader->Release();
101:                 m_VertexShader = 0;
102:             }
103: 
104:             HRESULT hr = m_Driver->m_pID3DDevice->CreateVertexShader(
105:                 m_ShaderBlob->GetBufferPointer(),
106:                 m_ShaderBlob->GetBufferSize(),
107:                 nullptr,
108:                 &m_VertexShader);
109: 
110:             return SUCCEEDED(hr);
111:         }
112: 
113:         bool CD3D11Shader::createHullShader()
114:         {
115:             if (!m_Compiled || m_ShaderType != EDST_HULL || !m_ShaderBlob)
116:                 return false;
117: 
118:             if (m_HullShader)
119:             {
120:                 m_HullShader->Release();
121:                 m_HullShader = 0;
122:             }
123: 
124:             HRESULT hr = m_Driver->m_pID3DDevice->CreateHullShader(
125:                 m_ShaderBlob->GetBufferPointer(),
126:                 m_ShaderBlob->GetBufferSize(),
127:                 nullptr,
128:                 &m_HullShader);
129: 
130:             return SUCCEEDED(hr);
131:         }
132: 
133:         bool CD3D11Shader::createDomainShader()
134:         {
135:             if (!m_Compiled || m_ShaderType != EDST_DOMAIN || !m_ShaderBlob)
136:                 return false;
137: 
138:             if (m_DomainShader)
139:             {
140:                 m_DomainShader->Release();
141:                 m_DomainShader = 0;
142:             }
143: 
144:             HRESULT hr = m_Driver->m_pID3DDevice->CreateDomainShader(
145:                 m_ShaderBlob->GetBufferPointer(),
146:                 m_ShaderBlob->GetBufferSize(),
147:                 nullptr,
148:                 &m_DomainShader);
149: 
150:             return SUCCEEDED(hr);
151:         }
152: 
153:         bool CD3D11Shader::createGeometryShader()
154:         {
155:             if (!m_Compiled || m_ShaderType != EDST_GEOMETRY || !m_ShaderBlob)
156:                 return false;
157: 
158:             if (m_GeometryShader)
159:             {
160:                 m_GeometryShader->Release();
161:                 m_GeometryShader = 0;
162:             }
163: 
164:             HRESULT hr = m_Driver->m_pID3DDevice->CreateGeometryShader(
165:                 m_ShaderBlob->GetBufferPointer(),
166:                 m_ShaderBlob->GetBufferSize(),
167:                 nullptr,
168:                 &m_GeometryShader);
169: 
170:             return SUCCEEDED(hr);
171:         }
172: 
173:         bool CD3D11Shader::createPixelShader()
174:         {
175:             if (!m_Compiled || m_ShaderType != EDST_PIXEL || !m_ShaderBlob)
176:                 return false;
177: 
178:             if (m_PixelShader)
179:             {
180:                 m_PixelShader->Release();
181:                 m_PixelShader = 0;
182:             }
183: 
184:             HRESULT hr = m_Driver->m_pID3DDevice->CreatePixelShader(
185:                 m_ShaderBlob->GetBufferPointer(),
186:                 m_ShaderBlob->GetBufferSize(),
187:                 nullptr,
188:                 &m_PixelShader);
189: 
190:             return SUCCEEDED(hr);
191:         }
192: 
193:         bool CD3D11Shader::createComputeShader()
194:         {
195:             if (!m_Compiled || m_ShaderType != EDST_COMPUTE || !m_ShaderBlob)
196:                 return false;
197: 
198:             if (m_ComputeShader)
199:             {
200:                 m_ComputeShader->Release();
201:                 m_ComputeShader = 0;
202:             }
203: 
204:             HRESULT hr = m_Driver->m_pID3DDevice->CreateComputeShader(
205:                 m_ShaderBlob->GetBufferPointer(),
206:                 m_ShaderBlob->GetBufferSize(),
207:                 nullptr,
208:                 &m_ComputeShader);
209: 
210:             return SUCCEEDED(hr);
211:         }
212: 
213:         bool CD3D11Shader::createInputLayout(const D3D11_INPUT_ELEMENT_DESC *layout, u32 elementCount)
214:         {
215:             if (!m_Compiled || m_ShaderType != EDST_VERTEX || !m_ShaderBlob)
216:                 return false;
217: 
218:             if (m_InputLayout)
219:             {
220:                 m_InputLayout->Release();
221:                 m_InputLayout = 0;
222:             }
223: 
224:             if (m_InputLayoutDesc)
225:             {
226:                 delete[] m_InputLayoutDesc;
227:                 m_InputLayoutDesc = 0;
228:             }
229: 
230:             m_InputLayoutDesc = new D3D11_INPUT_ELEMENT_DESC[elementCount];
231:             memcpy(m_InputLayoutDesc, layout, sizeof(D3D11_INPUT_ELEMENT_DESC) * elementCount);
232:             m_InputLayoutElementCount = elementCount;
233: 
234:             HRESULT hr = m_Driver->m_pID3DDevice->CreateInputLayout(
235:                 layout,
236:                 elementCount,
237:                 m_ShaderBlob->GetBufferPointer(),
238:                 m_ShaderBlob->GetBufferSize(),
239:                 &m_InputLayout);
240: 
241:             return SUCCEEDED(hr);
242:         }
243: 
244:     } // end namespace video
245: } // end namespace irr
246: 
247: #endif // _IRR_COMPILE_WITH_DIRECT3D_11_

(End of file - total 247 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 3.9s)

_Thinking:_

The implementation looks complete. Let me now verify that the CD3D11Driver.cpp is correctly using the new shader class. Let me look at the modified createBuiltInVertexShader and createBuiltInPixelShader functions:

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "offset": 1845,
  "limit": 120
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
1845: 
1846:         bool CD3D11Driver::createBuiltInVertexShader(E_VERTEX_TYPE type)
1847:         {
1848:             const char    *shaderSource = 0;
1849: 
1850:             switch (type)
1851:             {
1852:                 case EVT_STANDARD:
1853:                     shaderSource = VERTEX_SHADER_STANDARD;
1854:                     break;
1855: 
1856:                 case EVT_2TCOORDS:
1857:                     shaderSource = VERTEX_SHADER_2TCOORDS;
1858:                     break;
1859: 
1860:                 case EVT_TANGENTS:
1861:                     shaderSource = VERTEX_SHADER_TANGENTS;
1862:                     break;
1863: 
1864:                 default:
1865:                     return false;
1866:             }
1867: 
1868:             CD3D11Shader *shader = new CD3D11Shader(this);
1869:             if (!shader->compile(EDST_VERTEX, shaderSource, "main", "vs_4_0"))
1870:             {
1871:                 shader->drop();
1872:                 return false;
1873:             }
1874: 
1875:             if (!shader->createVertexShader())
1876:             {
1877:                 shader->drop();
1878:                 return false;
1879:             }
1880: 
1881:             D3D11_INPUT_ELEMENT_DESC *layout = 0;
1882:             u32 numElements = 0;
1883: 
1884:             switch (type)
1885:             {
1886:                 case EVT_STANDARD:
1887:                 {
1888:                     static D3D11_INPUT_ELEMENT_DESC standardLayout[] =
1889:                     {
1890:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
1891:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
1892:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
1893:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
1894:                     };
1895:                     layout = standardLayout;
1896:                     numElements = 4;
1897:                     break;
1898:                 }
1899: 
1900:                 case EVT_2TCOORDS:
1901:                 {
1902:                     static D3D11_INPUT_ELEMENT_DESC twoTexLayout[] =
1903:                     {
1904:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
1905:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
1906:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
1907:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
1908:                         {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
1909:                     };
1910:                     layout = twoTexLayout;
1911:                     numElements = 5;
1912:                     break;
1913:                 }
1914: 
1915:                 case EVT_TANGENTS:
1916:                 {
1917:                     static D3D11_INPUT_ELEMENT_DESC tangentLayout[] =
1918:                     {
1919:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
1920:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
1921:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
1922:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
1923:                         {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
1924:                         {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
1925:                     };
1926:                     layout = tangentLayout;
1927:                     numElements = 6;
1928:                     break;
1929:                 }
1930: 
1931:                 default:
1932:                     shader->drop();
1933:                     return false;
1934:             }
1935: 
1936:             if (!shader->createInputLayout(layout, numElements))
1937:             {
1938:                 shader->drop();
1939:                 return false;
1940:             }
1941: 
1942:             m_ShaderPool.push_back(shader);
1943: 
1944:             if (m_BuiltInVertexShader[type])
1945:                 m_BuiltInVertexShader[type]->Release();
1946:             m_BuiltInVertexShader[type] = shader->getVertexShader();
1947:             m_BuiltInVertexShader[type]->AddRef();
1948: 
1949:             if (m_InputLayout[type])
1950:                 m_InputLayout[type]->Release();
1951:             m_InputLayout[type] = shader->getInputLayout();
1952:             m_InputLayout[type]->AddRef();
1953: 
1954:             return true;
1955:         }
1956: 
1957: 
1958:         bool CD3D11Driver::createBuiltInPixelShader(E_VERTEX_TYPE type)
1959:         {
1960:             const char    *shaderSource = 0;
1961: 
1962:             switch (type)
1963:             {
1964:                 case EVT_STANDARD:

(Showing lines 1845-1964 of 2246. Use offset=1965 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 6.8s)

_Thinking:_

Good, the implementation looks correct. Now let me also check the header file to make sure the m_ShaderPool declaration is there:

**Tool: grep**

**Input:**
```json
{
  "pattern": "m_ShaderPool",
  "include": "*.h"
}
```

**Output:**
```
Found 1 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h:
  Line 470:             core::array<CD3D11Shader*> m_ShaderPool;

```

---

## Assistant (Build · MiniMax-M2.7 · 3.3s)

_Thinking:_

The implementation looks complete. Let me now verify the overall structure is correct. Let me also make sure the files are all in place:

**Tool: glob**

**Input:**
```json
{
  "pattern": "**/CD3D11Shader.{h,cpp}"
}
```

**Output:**
```
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Shader.cpp
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Shader.h
```

---

## Assistant (Build · MiniMax-M2.7 · 10.1s)

_Thinking:_

Good, both files are in place. The implementation is complete. Let me summarize what was done:

1. Created `CD3D11Shader.h` - A new shader class that:
   - Inherits from `IReferenceCounted` for memory management
   - Supports all D3D11 shader types (VS, HS, DS, GS, PS, CS) via `E_D3D11_SHADER_TYPE` enum
   - Stores HLSL source code, entry point, and profile strings
   - Stores the compiled `ID3DBlob`
   - Stores all shader objects (`ID3D11VertexShader`, `ID3D11HullShader`, `ID3D11DomainShader`, `ID3D11GeometryShader`, `ID3D11PixelShader`, `ID3D11ComputeShader`)
   - Stores `ID3D11InputLayout` and its description for vertex shaders
   - Provides methods to compile shaders and create shader objects

2. Created `CD3D11Shader.cpp` - Implementation of the shader class with:
   - Constructor/destructor with proper cleanup
   - `compile()` method using `D3DCompile`
   - Individual `create*Shader()` methods for each shader type
   - `createInputLayout()` for vertex shaders
   - `drop()` for IReferenceCounted compatibility

3. Updated `CD3D11Driver.h`:
   - Added forward declaration for `CD3D11Shader`
   - Added `m_ShaderPool` array to manage all shader objects

4. Updated `CD3D11Driver.cpp`:
   - Added include for `CD3D11Shader.h`
   - Modified `createBuiltInVertexShader()` to use `CD3D11Shader`
   - Modified `createBuiltInPixelShader()` to use `CD3D11Shader`

The existing built-in shaders (VERTEX_SHADER_STANDARD, VERTEX_SHADER_2TCOORDS, VERTEX_SHADER_TANGENTS, PIXEL_SHADER_STANDARD, PIXEL_SHADER_2TCOORDS, PIXEL_SHADER_TANGENTS) are now compiled and managed through the new `CD3D11Shader` class. The shader class properly manages memory using IReferenceCounted, and CD3D11Driver maintains ownership and lifecycle control through the shader pool.

创建完成。已创建:

**CD3D11Shader.h** - shader类:
- 管理所有D3D11 shader类型 (VS/HS/DS/GS/PS/CS)
- 保存HLSL源代码、入口点、profile
- 保存编译后的ID3DBlob
- 保存所有shader对象 (ID3D11VertexShader等)
- 对VS保存InputLayout信息和创建的ID3D11InputLayout
- 继承IReferenceCounted实现引用计数内存管理

**CD3D11Shader.cpp** - 实现:
- compile() 使用D3DCompile编译shader
- create*Shader() 创建各类shader对象
- createInputLayout() 为VS创建input layout

**CD3D11Driver.h/cpp** - 更新:
- 添加m_ShaderPool管理所有shader
- createBuiltInVertexShader和createBuiltInPixelShader改为使用CD3D11Shader类

所有提供的shader源码 (VERTEX_SHADER_STANDARD/2TCOORDS/TANGENTS, PIXEL_SHADER_STANDARD/2TCOORDS/TANGENTS) 现通过CD3D11Shader管理。

---

