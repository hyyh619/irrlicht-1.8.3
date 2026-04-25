# D3D9 to D3D11 API Migration Plan

## Executive Summary

This document outlines a comprehensive development plan to upgrade the Irrlicht Engine's rendering backend from Direct3D 9 (D3D9) to Direct3D 11 (D3D11). This migration leverages the existing CD3D9Driver architecture while fundamentally redesigning core components to utilize D3D11's modern API features.

---

## 1. Technical Context Overview

### 1.1 Current Architecture (D3D9)
Based on our analysis of CD3D9Driver, the D3D9 implementation consists of:

- **Core Device**: `IDirect3D9*` + `IDirect3DDevice9*` (lines 194-421, CD3D9Driver.cpp)
- **Rendering Pipeline**: BeginScene → DrawIndexedPrimitive → EndScene (lines 528-611)
- **State Management**: `SetRenderState()` for render states, `SetTextureStageState()` for textures
- **Hardware Buffers**: `CreateVertexBuffer`/`CreateIndexBuffer` with `SetStreamSource` binding
- **Shaders**: HLSL via `D3DXCompileShader` → `CreateVertexShader`/`CreatePixelShader`
- **Textures**: `IDirect3DTexture9` with `LockRect` for CPU access

### 1.2 Target Architecture (D3D11)

D3D11 introduces fundamental architectural changes:

| Component | D3D9 | D3D11 |
|-----------|------|-------|
| Device Creation | `Direct3DCreate9()` + `CreateDevice()` | `D3D11CreateDeviceAndSwapChain()` |
| API Type | COM-based | COM-based |
| Resource Management | Direct COM | Deferred destruction, logical device |
| Rendering Pipeline | Fixed function + shaders | Fully shader-based, pipeline state objects |
| Buffer Binding | `SetStreamSource` | `IASetVertexBuffers`, `IASetIndexBuffer` |
| Texture Format | `IDirect3DTexture9` | `ID3D11Texture2D` + Shader Resource Views |
| Shader Model | 2.0 | 4.0 / 5.0 |
| State Management | Individual `SetRenderState` calls | State objects (blend/depth/stencil) |
| Multithreading | Limited | Explicit support via deferred contexts |

---

## 2. Work Breakdown Structure

### Phase 1: Foundation Layer (Week 1-2)
**Objective**: Create D3D11 driver skeleton with device initialization parity

| Task ID | Task Name | Dependencies | Effort |
|---------|-----------|--------------|--------|
| 1.1 | Create CD3D11Driver.h/cpp skeleton | None | Medium |
| 1.2 | Implement device creation with adapter enumeration | 1.1 | High |
| 1.3 | Port present/swap chain management | 1.2 | Medium |
| 1.4 | Implement beginScene/endScene basic flow | 1.2 | Medium |
| 1.5 | Add driver registration to CIrrDeviceWin32 | 1.4 | Low |

**Deliverables**:
- New files: `source/Irrlicht/CD3D11Driver.h`, `source/Irrlicht/CD3D11Driver.cpp`
- Working device creation matching D3D9 capabilities

**Code Structure Template**:
```cpp
// CD3D11Driver.h
class CD3D11Driver : public CNullDriver, public IMaterialRendererServices
{
public:
    CD3D11Driver(const SIrrlichtCreationParameters &params, io::IFileSystem *io);
    virtual ~CD3D11Driver();
    
private:
    bool initDriver(HWND hwnd);
    
    ID3D11Device*           Device;           // Logical device
    ID3D11DeviceContext*    DeviceContext;   // Immediate context
    IDXGISwapChain*         SwapChain;
    ID3D11RenderTargetView* BackBufferView;
    ID3D11DepthStencilView* DepthStencilView;
    
    D3D_FEATURE_LEVEL FeatureLevel;  // 10_0, 10_1, 11_0, 11_1
};
```

### Phase 2: Rendering Pipeline (Week 2-3)
**Objective**: Implement core rendering with buffer and state management

| Task ID | Task Name | Dependencies | Effort |
|---------|-----------|--------------|--------|
| 2.1 | Implement vertex/index buffer creation | 1.1 | High |
| 2.2 | Implement buffer binding (IASetVertexBuffers) | 2.1 | Medium |
| 2.3 | Implement draw calls (DrawIndexed) | 2.2 | High |
| 2.4 | Port setTransform matrix management | 2.3 | Medium |
| 2.5 | Implement state objects (blend/depth/stencil) | 2.3 | High |

**Key D3D11 API Mappings**:

| D3D9 Call | D3D11 Equivalent |
|-----------|-----------------|
| `SetRenderState(D3DRS_ZENABLE, TRUE)` | DepthStencilState → OMSetDepthStencilState |
| `SetRenderState(D3DRS_CULLMODE, ...)` | RasterizerState → RSSetState |
| `SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE)` | BlendState → OMSetBlendState |
| `SetStreamSource(0, vb, 0, stride)` | IASetVertexBuffers(0, 1, &vbv, &offset, &stride) |
| `SetIndices(ib)` | IASetIndexBuffer(ib, format, offset) |
| `DrawIndexedPrimitive()` | DrawIndexedInstanced() |

**Code Example - Buffer Binding**:
```cpp
void CD3D11Driver::updateHardwareBuffer(SHWBufferLink *HWBuffer)
{
    D3D11_BUFFER_DESC bd;
    bd.ByteWidth = size;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = data;
    
    Device->CreateBuffer(&bd, &initData, &HWBuffer->Buffer11);
}
```

### Phase 3: Texture Management (Week 3-4)
**Objective**: Full texture support with render targets

| Task ID | Task Name | Dependencies | Effort |
|---------|-----------|--------------|--------|
| 3.1 | Create CD3D11Texture class | 1.1 | High |
| 3.2 | Implement texture loading via image loaders | 1.1 | Medium |
| 3.3 | Implement render target textures | 3.1 | High |
| 3.4 | Implement texture shader resource views | 3.1 | Medium |
| 3.5 | Implement mipmap generation | 3.3 | Medium |

**Texture Resource Mapping**:
```cpp
// D3D11 Texture Creation
ID3D11Texture2D* texture;
D3D11_TEXTURE2D_DESC td;
td.Width = size.Width;
td.Height = size.Height;
td.MipLevels = (mipmaps) ? 0 : 1;  // 0 = full chain
td.Format = getD3D11Format(colorFormat);
td.Usage = D3D11_USAGE_DEFAULT;
td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
if (renderTarget) td.BindFlags |= D3D11_BIND_RENDER_TARGET;

Device->CreateTexture2D(&td, nullptr, &texture);

// Shader Resource View (for binding to shaders)
D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
srvd.Format = td.Format;
srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
srvd.Texture2D.MipLevels = td.MipLevels;

Device->CreateShaderResourceView(texture, &srvd, &ShaderResourceView);
```

### Phase 4: Shader System (Week 4-5)
**Objective**: Port HLSL shader compilation and binding

| Task ID | Task Name | Dependencies | Effort |
|---------|-----------|--------------|--------|
| 4.1 | Implement D3D11 shader compilation | 2.3 | High |
| 4.2 | Create shader constant buffer management | 4.1 | High |
| 4.3 | Port existing CD3D9HLSLMaterialRenderer | 4.1 | High |
| 4.4 | Implement effect system | 4.3 | Medium |

**Shader Compilation Differences**:

| D3D9 | D3D11 |
|------|-------|
| `D3DXCompileShader()` | `D3DCompile()` from D3DCompiler.dll |
| `CreateVertexShader(hlsl_code)` | `CreateVertexShader(bytecode)` |
| Uses FVF for vertex format | Requires complete input layout |

**Input Layout**:
```cpp
// D3D11 requires explicit input layout
D3D11_INPUT_ELEMENT_DESC layout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

ID3D11InputLayout* inputLayout;
Device->CreateInputLayout(layout, _ARRAYSIZE(layout), shaderBytecode, 
                          shaderBytecodeSize, &inputLayout);
```

### Phase 5: Advanced Features (Week 5-6)
**Objective**: Implement D3D11-specific features

| Task ID | Task Name | Dependencies | Effort |
|---------|-----------|--------------|--------|
| 5.1 | Implement compute shaders (optional) | 4.4 | Medium |
| 5.2 | Implement tiled resources | 5.1 | Medium |
| 5.3 | Implement conservative depth output | 2.5 | Low |

### Phase 6: Testing and Optimization (Week 6-7)
**Objective**: Full validation and performance tuning

| Task ID | Task Name | Dependencies | Effort |
|---------|-----------|--------------|--------|
| 6.1 | Feature parity testing | All above | High |
| 6.2 | Memory profiling | 5.3 | Medium |
| 6.3 | Performance optimization | 6.2 | Medium |
| 6.4 | Compatibility testing with examples | 6.1 | Medium |

---

## 3. New D3D11 Features to Support

### 3.1 Pipeline State Objects (PSO)
D3D11 introduces PSOs which consolidate multiple state objects:

```cpp
// Create PSO
D3D11_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
psoDesc.InputLayout = inputLayout;
psoDesc.VS = vertexShader;
psoDesc.PS = pixelShader;
psoDesc.BlendState = blendState;
psoDesc.DepthStencilState = depthStencilState;
psoDesc.RasterizerState = rasterizerState;
psoDesc.SampleMask = D3D11_DEFAULT_SAMPLE_MASK;
psoDesc.PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

ID3D11PipelineState* pso;
Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
```

### 3.2 Deferred Contexts (Multithreading)
```cpp
// Create deferred context for multithreaded rendering
ID3D11DeviceContext* deferredContext;
Device->CreateDeferredContext(0, &deferredContext);

// Execute commands on worker thread
deferredContext->UpdateSubresource(...);
deferredContext->Draw(...);

// Copy to immediate context
immediateContext->CopyResource(dest, source);
```

### 3.3 Tiled Resources
For efficient memory management with large textures.

---

## 4. Error Handling Strategy

### 4.1 Device Loss Recovery
```cpp
// D3D11 uses different recovery pattern
bool CD3D11Driver::handleDeviceLost()
{
    HRESULT hr = Device->GetDeviceRemovedReason();
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
    {
        // Need full recreation
        waitForFence();
        destroyAll();
        return initDriver(Window);
    }
    return true;
}
```

### 4.2 Debug Layers
Enable D3D11 debug layer for validation:
```cpp
#if defined(_DEBUG)
ID3D11Debug* debug;
D3D11GetDebugInterface(IID_PPV_ARGS(&debug));
debug->ReportLiveDeviceObjects(D3D11_RDO_DETAIL);
#endif
```

---

## 5. Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| API breaking changes | High | High | Phased migration, feature flags |
| Performance regression | Medium | High | Early profiling, optimization pass |
| Shader compatibility | Medium | Medium | HLSL adjustments, shader 4.0+ features |
| New hardware issues | Low | High | Extensive device testing |

---

## 6. Acceptance Criteria

1. **Functional Parity**:
   - [ ] All 26 example applications run correctly
   - [ ] All material types render correctly
   - [ ] Texture/shader loading works
   - [ ] Render targets function properly

2. **Performance**:
   - [ ] Frame rate >= D3D9 baseline
   - [ ] Memory usage <= 2x D3D9 baseline

3. **Compatibility**:
   - [ ] Windows 10/11 support
   - [ ] AMD/NVIDIA/Intel GPU support
   - [ ] Windowed and fullscreen modes

---

## 7. Implementation Files Reference

### New Files Required
| File | Purpose |
|------|---------|
| `source/Irrlicht/CD3D11Driver.h` | Driver class definition |
| `source/Irrlicht/CD3D11Driver.cpp` | Driver implementation |
| `source/Irrlicht/CD3D11Texture.h` | Texture abstraction |
| `source/Irrlicht/CD3D11Texture.cpp` | Texture implementation |
| `source/Irrlicht/CD3D11ShaderMaterialRenderer.h` | Shader material |
| `source/Irrlicht/CD3D11ShaderMaterialRenderer.cpp` | Shader implementation |

### Modified Files
| File | Changes |
|------|---------|
| `source/Irrlicht/CIrrDeviceWin32.cpp` | Add D3D11 device creation |
| `source/Irrlicht/Makefile` | Add D3D11 driver build |
| `source/Irrlicht/CNullDriver.h` | Add createDeviceDependentTexture override |

---

*Generated by Sisyphus - D3D9 to D3D11 Migration Plan*
*Based on CD3D9Driver Analysis for Irrlicht Engine 1.8.3*