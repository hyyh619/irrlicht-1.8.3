# Irrlicht 1.8.3 D3D11 Video Driver Development Document

## Overview

This document describes the development history and technical details of the Direct3D 11 video driver implementation for Irrlicht Engine 1.8.3. The D3D11 driver provides Direct3D 11 rendering support for Windows, complementing the existing Direct3D 9 and OpenGL drivers.

**Development Timeline**: ~66 implementation sessions (April-May 2026)
**Total Commits**: 130+ (skipping 46 pure documentation commits)

---

## Architecture Overview

### Class Hierarchy

```
CD3D11Driver (main driver class)
├── Inherits from: CNullDriver, IMaterialRendererServices
├── Manages D3D11 device, device context, swap chain
├── Contains shader pool, sampler pool, render state cache
│
├── CD3D11Texture (texture management)
│   ├── ID3D11Texture2D for GPU storage
│   ├── ShaderResourceView for shader access
│   ├── RenderTargetView for render targets
│   └── StagingTexture for CPU access
│
├── CD3D11Shader (shader compilation/management)
│   ├── VS/HS/DS/GS/PS/CS shader types
│   ├── InputLayout management per vertex type
│   └── Compiled shader blob storage
│
├── CSampler (sampler state management)
│   └── D3D11_SAMPLER_DESC with filter, address, LOD
│
├── CD3D11MaterialRenderer (material renderer + PS HLSL)
├── CD3D11ShaderMaterialRenderer (custom shader materials)
├── CD3D11NormalMapRenderer (normal mapping)
├── CD3D11ParallaxMapRenderer (parallax mapping)
├── CD3D11HLSLMaterialRenderer (Cg/HLSL shaders)
└── CD3D11CgMaterialRenderer (Cg shading)
```

### Key Components

#### 1. Device and Context
```cpp
ID3D11Device* m_pID3DDevice;           // D3D11 device
ID3D11DeviceContext* m_pID3DDeviceContext;  // Immediate context
ID3D11Device1* m_pID3DDevice1;         // D3D11.1 feature level
IDXGISwapChain* m_SwapChain;           // DXGI swap chain
```

#### 2. Shader Pool
```cpp
core::map<io::path, CD3D11Shader*> m_ShaderPool;
```
Caches compiled shaders to avoid re-compilation.

#### 3. Sampler Pool
```cpp
core::map<uint32_t, CSampler*> m_SamplerPool;
```
Caches sampler states based on filter/address/LOD key.

#### 4. Render State Sets
```cpp
struct SRenderStateSet {
    ID3D11RasterizerState* RasterizerState;
    ID3D11DepthStencilState* DepthStencilState;
    ID3D11BlendState* BlendState;
};
core::map<uint64_t, SRenderStateSet> m_RenderStateSets;
```

---

## Constant Buffer Layout

| Slot | Register | Name | Purpose |
|------|----------|------|---------|
| 0 | b0 (VS) | MatrixBuffer | WorldViewProj, World matrices |
| 1 | b1 (VS/PS) | LightBuffer | Ambient, diffuse, specular colors; position; direction; attenuation |
| 2 | b2 (VS/PS) | MaterialBuffer | Material colors with ColorMaterialMode |
| 3 | b3 (PS) | CameraBuffer | Camera position for specular calculations |

### HLSL Definition
```hlsl
cbuffer MatrixBuffer : register(b0) {
    row_major matrix WorldViewProj;
    row_major matrix World;
    float padding0;
    float padding1;
    float padding2;
};

cbuffer LightBuffer : register(b1) {
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 LightPosition;
    float padding3;
    float3 LightDirection;
    float padding4;
    float3 Attenuation;
    float padding5;
};

cbuffer MaterialBuffer : register(b2) {
    float4 AmbientColor;
    float4 DiffuseColor;
    float4 SpecularColor;
    float4 EmissiveColor;
    int ColorMaterial;
    float padding6;
    float padding7;
    float padding8;
};

cbuffer CameraBuffer : register(b3) {
    float3 CameraPosition;
    float padding9;
};
```

**Note**: Explicit padding fields ensure 16-byte alignment required by D3D11.

---

## Vertex Shader Types (EVT_*)

| Type | Description |
|------|-------------|
| `EVT_STANDARD` | Position, Normal, Color, TexCoord |
| `EVT_2TCOORDS` | Position, Normal, Color, TexCoord0, TexCoord1 (lightmaps) |
| `EVT_TANGENTS` | Tangent space for normal/parallax mapping |

### Lighting Variants
- `EVT_STANDARD_DIRECTIONAL` - Directional light shader
- `EVT_STANDARD_POINT` - Point light shader
- `EVT_STANDARD_SPOT` - Spot light shader

---

## Material Shader Types (EMT_*)

The driver implements pixel shaders for all standard Irrlicht material types:

### Solid Materials
- `EMT_SOLID` - Basic solid rendering
- `EMT_SOLID_2_LAYER` - Two-layer alpha blend

### Lightmap Materials
- `EMT_LIGHTMAP` - Lightmap with alpha mask
- `EMT_LIGHTMAP_ADD` - Additive lightmap blending
- `EMT_LIGHTMAP_M2` - Multi-texture 2
- `EMT_LIGHTMAP_M4` - Multi-texture 4

### Transparent Materials
- `EMT_TRANSPARENT` - Alpha channel transparency
- `EMT_TRANSPARENT_ALPHA_CHANNEL` - Alpha reference transparency
- `EMT_TRANSPARENT_ALPHA_CHANNEL_REF` - Alpha test transparency
- `EMT_TRANSPARENT_ADD_COLOR` - Additive color transparency
- `EMT_TRANSPARENT_VERTEX_ALPHA` - Vertex alpha transparency
- `EMT_TRANSPARENT_ADD_COLOR_WITH_LIGHT` - Additive with lighting

### Effect Materials
- `EMT_NORMAL_MAP_SOLID` - Normal mapping solid
- `EMT_NORMAL_MAP_TRANSPARENT` - Normal mapping transparent
- `EMT_PARALLAX_MAP_SOLID` - Parallax mapping solid
- `EMT_PARALLAX_MAP_TRANSPARENT` - Parallax mapping transparent

### Lighting Materials
- `EMT_SOLID_LIGHTING` - Full lighting calculation
- `EMT_SOLID_LIGHTING_FLAT` - Flat shading (per-face)
- `EMT_SOLID_LIGHTING_GOURAUD` - Gouraud shading (per-vertex)
- `EMT_REFLECTION_2_LAYER` - Reflection with second layer

---

## Development Timeline

### Phase 1: Foundation (Steps 1-10)
- Initial port from CD3D9* to CD3D11* class structure
- Device initialization with adapter enumeration
- Swap chain setup and fullscreen support
- Basic texture creation (including NPOT verification)
- Vendor name detection (NVIDIA, AMD, Intel)
- Member variable naming convention standardization (m_ prefix)

### Phase 2: Core Rendering (Steps 11-25)
- `initDriver` implementation with device creation
- Hardware buffer management (vertex/index buffers)
- Input layout creation for all EVT_* vertex types
- MVP matrix calculation and constant buffer setup
- Basic draw calls via `draw2D3DVertexPrimitiveList`
- Render state management (rasterizer, depth stencil, blend)
- Sampler class creation and pooling
- Draw call dumping for debugging

### Phase 3: Material System (Steps 26-45)
- Material renderer creation via `createMaterialRenderers`
- PS HLSL generation for each material type
- VS/PS shader separation via `CD3D11Shader` class
- `draw2DRectangle` with gradient colors
- `draw2DImageBatch` for batched sprite rendering
- Alpha blend state management per material
- Draw call debugging with `_IRR_DUMP_DRAW_CALLS_FILE`

### Phase 4: Advanced Features (Steps 46-65)
- Texture lock/unlock implementation with staging resources
- Render state key computation for 3D materials
- Anti-aliasing and color mask settings
- Gouraud/Flat shading (`EMT_SOLID_LIGHTING_GOURAUD`, `EMT_SOLID_LIGHTING_FLAT`)
- Vertex shader lighting (directional, point, spot)
- Material color handling with ColorMaterialMode
- Camera position passing to PS for specular calculations
- Constant buffer padding fixes for HLSL compiler

### Phase 5: Light Integration (Steps 66+)
- Fixing RdotV zero issue in specular calculation
- `EMT_TRANSPARENT_ADD_COLOR_WITH_LIGHT` implementation
- `EMT_REFLECTION_2_LAYER_WITH_LIGHT` implementation
- Adding texcoord2 to vertex shaders for multi-texture
- Material color to VS integration

---

## Key Technical Decisions

### 1. Shader Management Strategy

**HLSL Source Storage**:
- Vertex shaders: defined as `static const char*` arrays in `CD3D11Shader.cpp`
- Pixel shaders: embedded in `CD3D11MaterialRenderer.h` as `PS_MaterialShaders_Part1/Part2`

**Compilation Flow**:
```
CD3D11Shader::compileShader()
  → D3DCompile()
  → ID3DBlob* pBlob
  → Create*Shader(pBlob->GetBufferPointer(), pBlob->GetBufferSize())
  → Store in m_ShaderPool
```

**Input Layout Management**:
```cpp
core::map<E_VERTEX_TYPE, ID3D11InputLayout*> m_InputLayoutPool;
```
Created on first use for each EVT_* type.

### 2. Render State Caching

States cached to avoid redundant D3D11 state changes:

```cpp
struct SRenderStateKey {
    uint32_t ZWriteEnabled     : 1;
    uint32_t AlphaBlendEnable : 1;
    uint32_t SrcBlend         : 4;
    uint32_t DstBlend         : 4;
    // ... more fields
    uint32_t AntiAlasing      : 2;
};
```

**2D vs 3D Differentiation**:
- 2D: Simple key based on alpha, texture, alphaChannel flags
- 3D: Two-key system (`m_Current3DStateKeyKey1`, `m_Current3DStateKeyKey2`) to avoid collisions

### 3. Sampler Management

```cpp
class CSampler {
    D3D11_SAMPLER_DESC Desc;
    ID3D11SamplerState* m_D3D11SamplerState;
public:
    uint32_t getKey() const;  // Filter(4) | Address(12) | LOD(4) = 20-bit key
};
```

Key computation:
```cpp
uint32_t key = (Filter << 24) | (AddressU << 16) | (AddressV << 8) | MaxLOD;
```

### 4. Texture Lock/Unlock

```cpp
bool CD3D11Texture::lock(E_LOCK_MODE mode, E_TEXTURE_LOCK_FLAG flag) {
    if (m_DynamicTexture) {
        // Direct GPU access
        m_pID3DDeviceContext->Map(m_Texture, subResource, ...);
    } else {
        // Staging texture for CPU access
        m_pID3DDeviceContext->Map(m_StagingTexture, subResource, ...);
    }
}
```

### 5. Memory Management

All D3D11 objects use `IRR_D3D11_*_RELEASE` macros:
```cpp
#define IRR_D3D11_DEVICE_RELEASE(p) if (p) { (p)->Release(); (p) = nullptr; }
```

Debug builds include `ID3D11Debug::ReportLiveDeviceObjects()` for leak detection.

---

## Debugging Features

### Texture Dumping
```cpp
#ifdef _IRR_DUMP_TEXTURE
    dumpTexture(texture, filename);
#endif
```

### Draw Call Logging
```cpp
#ifdef _IRR_DUMP_DRAW_CALLS_FILE
    // Writes to draw_calls_dump.txt
#endif
#ifdef _IRR_DUMP_DRAW_CALLS_PRINT
    // Outputs to logger
#endif
```

### Object Tracking
```cpp
#ifdef _IRR_D3D11_OBJECT_TRACKING
    CD3D11ObjectTracker - tracks all D3D11 object creation/destruction
#endif
```

---

## File Structure

| File | Purpose |
|------|---------|
| `source/Irrlicht/CD3D11Driver.h/cpp` | Main driver class |
| `source/Irrlicht/CD3D11Texture.h/cpp` | Texture management |
| `source/Irrlicht/CD3D11Shader.h/cpp` | Shader compilation/management |
| `source/Irrlicht/CD3D11MaterialRenderer.h/cpp` | Material renderers + HLSL sources |
| `source/Irrlicht/CD3D11NormalMapRenderer.h/cpp` | Normal map effects |
| `source/Irrlicht/CD3D11ParallaxMapRenderer.h/cpp` | Parallax mapping |
| `source/Irrlicht/CD3D11ShaderMaterialRenderer.h/cpp` | Custom shader materials |
| `source/Irrlicht/CD3D11HLSLMaterialRenderer.h/cpp` | HLSL material renderer |
| `source/Irrlicht/CD3D11CgMaterialRenderer.h/cpp` | Cg material renderer |
| `source/Irrlicht/CD3D11Debug.h` | Debug utilities |
| `source/Irrlicht/CD3D11ObjectTracker.h` | Object creation tracking |

---

## Current Implementation Status

| Feature | Status |
|---------|--------|
| Device/Context initialization | Complete |
| Swap chain management | Complete |
| Texture creation/loading | Complete |
| Hardware buffers (VBO/IBO) | Complete |
| All EVT_* vertex types | Complete |
| All EMT_* material types | Complete |
| 2D drawing (rect, line, image) | Complete |
| 3D rendering with lighting | Complete |
| Normal/Parallax mapping | Complete |
| Gouraud/Flat shading | Complete |
| Sampler state management | Complete |
| Render target support | Complete |
| Depth stencil management | Complete |
| Lock/unlock texture | Complete |
| Draw call debugging | Complete |

---

## Development Sessions Reference

- **Prompt**: `Prompt/D3D11_Implement.md` - Development task list (91 tasks)
- **Sessions**: `Sessions/d3d11-implement/session-d3d11-step*.md` - 66+ session notes

Session naming convention: `session-d3d11-step[N]-[description].md`

Example sessions:
- `session-d3d11-step24-create-shader-class.md`
- `session-d3d11-step47-create-sampler-by-material.md`
- `session-d3d11-step66-fix-RdotV-zero.md`