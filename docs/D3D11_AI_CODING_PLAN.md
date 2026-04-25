# AI Coding Development Plan: D3D9 to D3D11 API Migration

## Plan Overview

This document provides a step-by-step AI coding implementation plan for migrating Irrlicht Engine's Direct3D 9 rendering backend to Direct3D 11. Each task includes precise file locations, code implementations, and verification steps.

**Total Estimated Tasks**: 24 major tasks across 6 phases
**AI Agent Capability Level**: Senior C++ with Graphics API experience
**Prerequisites**: Irrlicht Engine 1.8.3 codebase, Windows SDK 10+

---

## Phase 1: Foundation Layer (Tasks 1.1 - 1.5)

### Task 1.1: Create CD3D11Driver Skeleton

**Files to Create**:
- `source/Irrlicht/CD3D11Driver.h` (new)
- `source/Irrlicht/CD3D11Driver.cpp` (new)

**Implementation Steps**:
1. Create header file with class definition inheriting from `CNullDriver` and `IMaterialRendererServices`
2. Add required D3D11 include: `<d3d11.h>`, `<dxgi1_2.h>`, `<D3Dcompiler.h>`
3. Define member variables: `ID3D11Device*`, `ID3D11DeviceContext*`, `IDXGISwapChain*`
4. Add constructor/destructor stubs
5. Create `initDriver(HWND)` stub returning false

**Verification**: File compiles with D3D11 headers included

---

### Task 1.2: Implement Device Creation

**Reference File**: `CD3D9Driver.cpp` lines 181-421

**Implementation Steps**:
```cpp
// In CD3D11Driver::initDriver(HWND hwnd)
1. Define DXGI_SWAP_CHAIN_DESC (backbuffer, windowed, format)
2. Call D3D11CreateDeviceAndSwapChain()
   - Try D3D11_FEATURE_LEVEL_11_1 first, then fallback to 11_0, 10_1, 10_0
3. On success: Get immediate context: Device->GetImmediateContext()
4. Create render target view for backbuffer
5. Query device capabilities (Caps)
```

**Key D3D11 API**:
- `D3D11CreateDeviceAndSwapChain()`
- `ID3D11Device::GetImmediateContext()`
- `IDXGISwapChain::GetBuffer()` → `CreateRenderTargetView()`

**Verification**: Device created successfully, swap chain functional

---

### Task 1.3: Port Present/Swap Chain Management

**Reference File**: `CD3D9Driver.cpp` line 611 (GetSwapChain → Present)

**Implementation Steps**:
```cpp
1. In endScene(): 
   - SwapChain->Present(syncInterval, flags)
2. Handle fullscreen transitions:
   - SwapChain->SetFullscreenState()
3. Handle resize:
   - Check new backbuffer dimensions
   - Resize buffers if needed
```

**Verification**: Window displays, fullscreen toggle works

---

### Task 1.4: Implement beginScene/endScene Basic Flow

**Reference File**: `CD3D9Driver.cpp` lines 527-610

**Implementation Steps**:
```cpp
// beginScene()
1. Clear(D3D11_CLEAR_TARGET | D3D11_CLEAR_DEPTH, color, 1.0, 0)
2. DeviceContext->OMSetRenderTargets(1, &rtv, dsv)

// endScene()  
1. Handle device lost (DXGI_ERROR_DEVICE_REMOVED)
2. SwapChain->Present()
```

**Key D3D11 API**:
- `ID3D11DeviceContext::ClearRenderTargetView()`
- `ID3D11DeviceContext::ClearDepthStencilView()`
- `ID3D11DeviceContext::OMSetRenderTargets()`

**Verification**: Basic frame clears and presents

---

### Task 1.5: Add Driver Registration to CIrrDeviceWin32

**Reference File**: `source/Irrlicht/CIrrDeviceWin32.cpp` (search for "DIRECT3D_9")

**Implementation Steps**:
1. Add `#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_` section
2. Add driver creation case in device factory
3. Update Makefile to include new source files

**Verification**: Driver selectable at runtime

---

## Phase 2: Rendering Pipeline (Tasks 2.1 - 2.5)

### Task 2.1: Implement Vertex/Index Buffer Creation

**Reference File**: `CD3D9Driver.cpp` lines 1137-1220

**Implementation Steps**:
```cpp
// SHWBufferLink_d3d11 struct
struct SHWBufferLink_d3d11 : SHWBufferLink {
    ID3D11Buffer* VertexBuffer;
    ID3D11Buffer* IndexBuffer;
};

// CreateBuffer call
D3D11_BUFFER_DESC bd = {};
bd.ByteWidth = size;
bd.Usage = D3D11_USAGE_DYNAMIC;
bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;  // or INDEX_BUFFER
bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

Device->CreateBuffer(&bd, nullptr, &buffer);
```

**Key API**: `ID3D11Device::CreateBuffer()`

**Verification**: Buffers created with correct stride/binding

---

### Task 2.2: Implement Buffer Binding

**Reference File**: `CD3D9Driver.cpp` lines 1350-1368

**Implementation Steps**:
```cpp
// Vertex buffers
IASetVertexBuffers(0, 1, &vbv, &offset, &stride);

// Index buffer  
IASetIndexBuffer(ib, format, offset);

// Clear bindings
IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
```

**Key API**: 
- `ID3D11DeviceContext::IASetVertexBuffers()`
- `ID3D11DeviceContext::IASetIndexBuffer()`

**Verification**: Correct data flows to vertex shader

---

### Task 2.3: Implement Draw Calls

**Reference File**: `CD3D9Driver.cpp` line 1623 (DrawIndexedPrimitive)

**Implementation Steps**:
```cpp
// Set primitive topology once
DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

// Draw call
DeviceContext->DrawIndexedInstanced(
    indexCount,    // primitives
    instanceCount, // 1 for non-instanced
    startIndex,
    baseVertex,
    startInstance  // 0
);
```

**Key API**: 
- `ID3D11DeviceContext::IASetPrimitiveTopology()`
- `ID3D11DeviceContext::DrawIndexed()`

**Verification**: Triangle renders correctly

---

### Task 2.4: Port setTransform Matrix Management

**Reference File**: `CD3D9Driver.cpp` lines 745-760

**Implementation Steps**:
```cpp
// Store matrix in Constant Buffer
// D3D11 requires shader-visible constant buffers

VS_CB_Matrices cb;
cb.World = Matrices[ETS_WORLD];
cb.View = Matrices[ETS_VIEW];
cb.Projection = Matrices[ETS_PROJECTION];

DeviceContext->UpdateSubresource(matrixBuffer, 0, nullptr, &cb, 0, 0);
DeviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
```

**Key API**: 
- `ID3D11DeviceContext::UpdateSubresource()`
- `ID3D11DeviceContext::VSSetConstantBuffers()`

**Verification**: Transforms apply correctly

---

### Task 2.5: Implement State Objects

**Reference File**: `CD3D9Driver.cpp` lines 2300-2500 (SetRenderState calls)

**Implementation Steps**:

**A) Depth-Stencil State**
```cpp
D3D11_DEPTH_STENCIL_DESC dsd = {};
dsd.DepthEnable = TRUE;
dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
// ... stencil settings

Device->CreateDepthStencilState(&dsd, &depthStencilState);
DeviceContext->OMSetDepthStencilState(depthStencilState, stencilRef);
```

**B) Blend State**
```cpp
D3D11_BLEND_DESC bd = {};
bd.RenderTarget[0].BlendEnable = TRUE;
bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
// ...

Device->CreateBlendState(&bd, &blendState);
DeviceContext->OMSetBlendState(blendState, blendFactor, mask);
```

**C) Rasterizer State**
```cpp
D3D11_RASTERIZER_DESC rd = {};
rd.CullMode = D3D11_CULL_BACK;
rd.FillMode = D3D11_FILL_SOLID;
// ...

Device->CreateRasterizerState(&rd, &rasterizerState);
DeviceContext->RSSetState(rasterizerState);
```

**Verification**: Render states match D3D9 behavior

---

## Phase 3: Texture Management (Tasks 3.1 - 3.5)

### Task 3.1: Create CD3D11Texture Class

**Reference File**: `CD3D9Texture.h` structure

**Implementation Steps**:
```cpp
// CD3D11Texture.h
class CD3D11Texture : public ITexture
{
    ID3D11Texture2D* Texture;
    ID3D11ShaderResourceView* SRV;
    ID3D11RenderTargetView* RTV;
    ID3D11DepthStencilView* DSV;
    
    // Standard ITexture implementation methods
    virtual const void* lock() override;
    virtual void unlock() override;
    // ...
};
```

**Verification**: Class compiles and interfaces correctly

---

### Task 3.2: Implement Texture Loading

**Reference**: CNullDriver texture creation flow

**Implementation Steps**:
```cpp
// In CD3D11Driver::createTextureFromImage(IImage* image)
1. Convert image format to DXGI_FORMAT
2. Create D3D11 texture
3. Copy image data (map/write/unmap)
4. Create ShaderResourceView
```

**Key API**:
- `ID3D11Device::CreateTexture2D()`
- `ID3D11Device::CreateShaderResourceView()`

**Verification**: Textures render correctly

---

### Task 3.3: Implement Render Target Textures

**Reference**: `CD3D9Texture.cpp` createRenderTarget

**Implementation Steps**:
```cpp
D3D11_TEXTURE2D_DESC td = {};
td.Usage = D3D11_USAGE_DEFAULT;
td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
td.Format = dxFormat;

Device->CreateTexture2D(&td, nullptr, &rtTexture);
Device->CreateRenderTargetView(rtTexture, nullptr, &rtv);
```

**Verification**: RTT displays to screen

---

### Task 3.4: Implement Texture Shader Resource Views

**Implementation Steps**:
```cpp
// Already covered in Task 3.2 - SRV created with texture
// Bind to shader: PSSetShaderResources(0, 1, &srv)
// Bind to vertex shader: VSSetShaderResources(0, 1, &srv)
```

**Verification**: Textures visible in shaders

---

### Task 3.5: Implement Mipmap Generation

**Reference**: `CD3D9Texture.cpp` GenerateMipSubLevels

**Implementation Steps**:
```cpp
// Auto-generate: Set MipLevels=0 in texture desc
// OR manual:
DeviceContext->GenerateMips(srv);  // If supporting auto-mipmap
```

**Verification**: Mipmaps render correctly at distance

---

## Phase 4: Shader System (Tasks 4.1 - 4.4)

### Task 4.1: Implement D3D11 Shader Compilation

**Reference**: `CD3D9HLSLMaterialRenderer.cpp` (shader compile)

**Implementation Steps**:
```cpp
// Use D3DCompiler (separate DLL)
#include <D3DCompiler.h>

ID3DBlob* errors = nullptr;
D3DCompile(
    hlslSource.c_str(), 
    hlslSource.size(),
    nullptr,  // define
    nullptr,  // include
    "main",   // entrypoint
    "vs_5_0", // target (shader model)
    flags, 
    0,        // effect flags
    &blob, 
    &errors
);

if (errors) {
    // Log errors
}

// Create shader
Device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vs);
```

**Key API**:
- `D3DCompiler.h` : `D3DCompile()`
- `ID3D11Device::CreateVertexShader()`

**Verification**: Shader compiles without errors

---

### Task 4.2: Create Shader Constant Buffer Management

**Implementation Steps**:
```cpp
// Create constant buffer
D3D11_BUFFER_DESC cbd = {};
cbd.ByteWidth = 256;  // Align to 16-byte boundary
cbd.Usage = D3D11_USAGE_DYNAMIC;
cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

Device->CreateBuffer(&cbd, nullptr, &cb);

// Update and bind
DeviceContext->UpdateSubresource(cb, 0, nullptr, data, 0, 0);
DeviceContext->VSSetConstantBuffers(0, 1, &cb);
DeviceContext->PSSetConstantBuffers(0, 1, &cb);
```

**Verification**: Uniforms pass to shaders correctly

---

### Task 4.3: Port CD3D9HLSLMaterialRenderer to D3D11

**Reference File**: `source/Irrlicht/CD3D9HLSLMaterialRenderer.cpp`

**Implementation Steps**:
1. Create `CD3D11HLSLMaterialRenderer` class
2. Adapt shader compilation to D3D11 (Task 4.1)
3. Replace SetVertexShader/SetPixelShader with new D3D11 calls
4. Adapt constant buffer updates
5. Implement OnSetMaterial/OnRender hooks

**Verification**: Existing materials render identically

---

### Task 4.4: Implement Input Layout System

**Reference**: D3D9 FVF system

**Implementation Steps**:
```cpp
D3D11_INPUT_ELEMENT_DESC layout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
    // Add tangent for normal mapping support
};

ID3D11InputLayout* inputLayout;
Device->CreateInputLayout(layout, count, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

DeviceContext->IASetInputLayout(inputLayout);
```

**Vertex Type Mapping**:

| Irrlicht Vertex Type | D3D11 Input Layout |
|--------------------|--------------------|
| EVT_STANDARD | POSITION, NORMAL, TEXCOORD |
| EVT_2TCOORDS | POSITION, NORMAL, TEXCOORD0, TEXCOORD1 |
| EVT_TANGENTS | +TANGENT, BINORMAL |

**Verification**: All vertex types render correctly

---

## Phase 5: Advanced Features (Tasks 5.1 - 5.3)

### Task 5.1: Compute Shaders (Optional)

**Implementation**: Follows same pattern as vertex/pixel shaders
- Entry point: `[numthreads(X,Y,Z)] void main()`
- Target: `cs_5_0`
- Dispatch: `Dispatch(x,y,z)`

---

### Task 5.2: Tiled Resources (Optional)

**Implementation**: Use `ID3D11TiledResource` APIs for large textures

---

### Task 5.3: Conservative Depth Output

**Implementation**:
```cpp
D3D11_DEPTH_STENCIL_DESC dsd = {};
dsd.DepthEnable = TRUE;
dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
dsd.ExtendedDepthStencilSupportValue = TRUE;  // Conservative depth
```

---

## Phase 6: Testing and Optimization (Tasks 6.1 - 6.4)

### Task 6.1: Feature Parity Testing

**Test Matrix**:

| Feature | Test Method | Expected |
|---------|-------------|----------|
| Basic triangle | 01.HelloWorld | Renders colored triangle |
| Camera | 02.Quake3Map | Camera transforms work |
| Materials | 10.Shaders | Shader materials render |
| Textures | examples with textures | Textures visible |
| Render targets | RTT examples | Offscreen renders work |
| Shadows | 07.Collision | Shadow mapping works |

---

### Task 6.2: Memory Profiling

**Metrics to Collect**:
- Texture memory: `ID3D11Device::GetPrivateData()` with D3D11_PRIVATE_DATA
- Buffer memory: Sum of all buffer sizes
- Total: Compare to D3D9 baseline

---

### Task 6.3: Performance Optimization

**Common Optimizations**:
1. Use `D3D11_USAGE_DEFAULT` vs `D3D11_USAGE_DYNAMIC`
2. Map vs UpdateSubresource patterns
3. Pipeline state caching
4. Resource sharing

---

### Task 6.4: Compatibility Testing

**Test Hardware**:
- [ ] NVIDIA GPU (GTX/RTX series)
- [ ] AMD GPU (Radeon series)
- [ ] Intel GPU (UHD/Iris)
- [ ] Windows 10 vs Windows 11

---

## Implementation Quick Reference

### D3D9 to D3D11 API Cheat Sheet

| Category | D3D9 | D3D11 |
|----------|------|-------|
| Init | `Direct3DCreate9()` | `D3D11CreateDeviceAndSwapChain()` |
| Frame Start | `BeginScene()` | `OMSetRenderTargets()` |
| Frame End | `EndScene()` + `Present()` | `DrawIndexed()` + `Present()` |
| Vertex Buffer | `CreateVertexBuffer()` + `SetStreamSource()` | `CreateBuffer()` + `IASetVertexBuffers()` |
| Texture | `CreateTexture()` + `LockRect()` | `CreateTexture2D()` + `Map()` |
| State | `SetRenderState()` | State Objects + OMSetXXX() |
| Vertex Format | `FVF` | `InputLayout` |
| Shader | `CreateVertexShader()` | `CreateVertexShader()` + Input Layout |
| Constants | `SetVertexShaderConstantF()` | Constant Buffer |

### Critical Include Files

```cpp
#include <d3d11.h>       // Core D3D11
#include <d3d11_1.h>     // D3D11.1 features
#include <dxgi1_2.h>    // DXGI (swap chain)
#include <D3DCompiler.h> // Shader compilation
#include <D3DX11Effect.h> // Effect system (optional)
```

### Build Configuration

```
// In Irrlicht Makefile
_IRR_COMPILE_WITH_DIRECT3D_11_
D3D11_LIBS := d3d11 dxgi d3dcompiler
```

---

*Plan Version*: 1.0
*Generated*: For AI Agent D3D11 Implementation
*Based On*: CD3D9Driver Analysis + D3D11 Migration Plan