# CD3D9Driver 实现分析报告

## 概述

本报告详细分析 Irrlicht Engine 的 CD3D9Driver 实现,展示如何使用 Direct3D 9 API 构建一个完整的渲染驱动层。

---

## 1. 初始化流程

### 1.1 构造函数 (CD3D9Driver.cpp, 行 34-78)

```cpp
CD3D9Driver::CD3D9Driver(const SIrrlichtCreationParameters &params, io::IFileSystem *io)
    : CNullDriver(io, params.WindowSize), CurrentRenderMode(ERM_NONE),
      ResetRenderStates(true), Transformation3DChanged(false),
      D3DLibrary(0), pID3D(0), pID3DDevice(0), ...
```

**关键成员变量:**
- `D3DLibrary`: d3d9.dll 句柄
- `pID3D`: IDirect3D9* 主接口
- `pID3DDevice`: IDirect3DDevice9* 渲染设备
- `Params`: 创建参数 (窗口大小、抗锯齿、Vsync 等)

### 1.2 设备创建 (initDriver)

**步骤 1: 加载 D3D9 运行时 (行 182-211)**
```cpp
D3DLibrary = LoadLibrary(__TEXT("d3d9.dll"));
typedef IDirect3D9* (__stdcall * D3DCREATETYPE)(UINT);
D3DCREATETYPE d3dCreate = (D3DCREATETYPE) GetProcAddress(D3DLibrary, "Direct3DCreate9");
pID3D = (*d3dCreate)(D3D_SDK_VERSION);  // 等价于 Direct3DCreate9(D3D_SDK_VERSION)
```

**步骤 2: 获取适配器信息 (行 213-247)**
```cpp
D3DADAPTER_IDENTIFIER9 dai;
pID3D->GetAdapterIdentifier(Params.DisplayAdapter, 0, &dai);
// 获取厂商名称: NVIDIA (0x10DE), ATI (0x1002), Intel (0x8086)
```

**步骤 3: 配置呈现参数 (D3DPRESENT_PARAMETERS, 行 256-331)**
```cpp
D3DPRESENT_PARAMETERS present;
present.BackBufferCount        = 1;
present.EnableAutoDepthStencil = TRUE;
present.PresentationInterval = Params.Vsync ? D3DPRESENT_INTERVAL_ONE 
                                           : D3DPRESENT_INTERVAL_IMMEDIATE;

// 全屏模式
present.BackBufferWidth  = Params.WindowSize.Width;
present.BackBufferHeight = Params.WindowSize.Height;
present.BackBufferFormat = (Params.Bits == 32) ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5;
present.SwapEffect       = D3DSWAPEFFECT_FLIP;
present.Windowed         = FALSE;

// 窗口模式
present.BackBufferFormat = d3ddm.Format;  // 从显示模式获取
present.SwapEffect    = D3DSWAPEFFECT_DISCARD;
present.Windowed     = TRUE;
```

**步骤 4: 抗锯齿探测 (行 305-331)**
```cpp
while (Params.AntiAlias > 0)
{
    pID3D->CheckDeviceMultiSampleType(adapter, devtype, present.BackBufferFormat, 
                                  !Params.Fullscreen, (D3DMULTISAMPLE_TYPE)Params.AntiAlias, 
                                  &qualityLevels);
    // 成功则设置 MultiSampleType/MultiSampleQuality
}
```

**步骤 5: 深度/模板缓冲探测 (行 333-387)**
```cpp
// 优先尝试: D3DFMT_D24S8 -> D3DFMT_D24X4S4 -> D3DFMT_D15S1
pID3D->CheckDeviceFormat(adapter, devtype, ..., D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, ...);
```

**步骤 6: 创建设备 (行 389-421)**
```cpp
// 尝试순서: HARDWARE -> MIXED -> SOFTWARE -> REF
pID3D->CreateDevice(adapter, D3DDEVTYPE_HAL, hwnd,
    fpuPrecision | multithreaded | D3DCREATE_HARDWARE_VERTEXPROCESSING, 
    &present, &pID3DDevice);
```

**步骤 7: 获取设备能力 (行 422-456)**
```cpp
pID3DDevice->GetDeviceCaps(&Caps);
pID3DDevice->GetAvailableTextureMem();
// 查询功能支持
MaxTextureUnits = core::min_(Caps.MaxSimultaneousTextures, MATERIAL_MAX_TEXTURES);
OcclusionQuerySupport = (pID3DDevice->CreateQuery(D3DQUERYTYPE_OCCLUSION, NULL) == S_OK);
```

---

## 2. 渲染流程

### 2.1 beginScene() (行 528-583)

```cpp
bool CD3D9Driver::beginScene(bool backBuffer, bool zBuffer, SColor color, ...)
{
    // 1. 设备丢失处理
    if (DeviceLost)
    {
        HRESULT hr = pID3DDevice->TestCooperativeLevel();
        if (hr == D3DERR_DEVICELOST)
            return false;  // 等待恢复
        if ((hr == D3DERR_DEVICENOTRESET) && !reset())
            return false;
    }

    // 2. 清除缓冲区
    DWORD flags = 0;
    if (backBuffer) flags |= D3DCLEAR_TARGET;
    if (zBuffer)  flags |= D3DCLEAR_ZBUFFER;
    if (Params.Stencilbuffer) flags |= D3DCLEAR_STENCIL;
    pID3DDevice->Clear(0, NULL, flags, color.color, 1.0, 0);

    // 3. 开始场景
    pID3DDevice->BeginScene();
    return true;
}
```

**关键 API:**
| API | 用途 |
|---|---|
| `IDirect3DDevice9::TestCooperativeLevel()` | 检测设备状态 |
| `IDirect3DDevice9::Clear()` | 清除颜色/深度/模板缓冲 |
| `IDirect3DDevice9::BeginScene()` | 开始渲染场景 |

### 2.2 endScene() (行 587-599)

```cpp
bool CD3D9Driver::endScene()
{
    pID3DDevice->EndScene();
    
    // 获取交换链并呈现
    IDirect3DSwapChain9 *swChain;
    pID3DDevice->GetSwapChain(0, &swChain);
    
    // 呈现参数
    swChain->Present(NULL, 0, WindowId, NULL, 0);  // 可选 srGB: D3DPRESENT_LINEAR_CONTENT
    return true;
}
```

**关键 API:**
| API | 用途 |
|---|---|
| `IDirect3DDevice9::GetSwapChain()` | 获取交换链 |
| `IDirect3DSwapChain9::Present()` | 呈现到屏幕 |

### 2.3 绘制入口 (drawVertexPrimitiveList, 行 1477-1492)

```cpp
void CD3D9Driver::drawVertexPrimitiveList(const void *vertices, u32 vertexCount,
    const void *indexList, u32 primitiveCount, E_VERTEX_TYPE vType, 
    scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType)
{
    // 绑定硬件缓冲区并绘制
    if (!vertexCount || !primitiveCount) return;
    draw2D3DVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount,
        vType, pType, iType, true);  // is3D = true
}
```

### 2.4 实际绘制 (draw2D3DVertexPrimitiveList, 行 1514-1644)

```cpp
void CD3D9Driver::draw2D3DVertexPrimitiveList(...)
{
    // 1. 设置顶点格式
    setVertexShader(vType);  // SetFVF 或配置顶点着色器

    // 2. 设置 3D 渲染状态
    if (is3D) setRenderStates3DMode();

    // 3. 设置材质渲染器 (OnSetMaterial -> OnRender)
    video::IMaterialRenderer *mr = MaterialRenderers[Material.MaterialType].Renderer;
    if (mr)
    {
        mr->OnSetMaterial(Material, LastMaterial, ResetRenderStates, this);
        mr->OnRender(this, vType);
    }
    
    // 4. 绘制图元
    pID3DDevice->DrawIndexedPrimitive(
        D3DPT_TRIANGLELIST,  // 图元类型
        0,                 // BaseVertexIndex
        0,                 // MinIndex
        vertexCount,        // NumVertices
        indexCount * 3,     // StartIndex
        primitiveCount      // PrimitiveCount
    );
}
```

**关键 API:**
| API | 用途 |
|---|---|
| `IDirect3DDevice9::SetFVF()` | 设置顶点格式 (固定功能管线) |
| `IDirect3DDevice9::SetTexture()` | 绑定纹理 |
| `IDirect3DDevice9::SetRenderState()` | 设置渲染状态 |
| `IDirect3DDevice9::DrawIndexedPrimitive()` | 绘制索引图元 |

---

## 3. 状态管理

### 3.1 setTransform (行 2150-2160)

```cpp
void CD3D9Driver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4 &mat)
{
    Matrices[state] = mat;
    
    switch(state)
    {
        case ETS_VIEW:
            pID3DDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)mat.pointer());
            break;
        case ETS_WORLD:
            pID3DDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)mat.pointer());
            break;
        case ETS_PROJECTION:
            pID3DDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)mat.pointer());
            break;
        // 纹理坐标变换
        case ETS_TEXTURE_0 ... ETS_TEXTURE_7:
            if (mat.isIdentity())
                pID3DDevice->SetTextureStageState(state - ETS_TEXTURE_0, D3DTSS_TEXTURETRANSFORMFLAGS, 
                                          D3DTTFF_DISABLE);
            else
            {
                pID3DDevice->SetTextureStageState(state - ETS_TEXTURE_0, D3DTSS_TEXTURETRANSFORMFLAGS, 
                                          D3DTTFF_COUNT2);
                pID3DDevice->SetTransform((D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + state - ETS_TEXTURE_0), 
                                        (D3DMATRIX*)mat.pointer());
            }
            break;
    }
}
```

### 3.2 setMaterial (行 2248-2380)

```cpp
void CD3D9Driver::setMaterial(const SMaterial &material)
{
    Material = material;
    
    // 设置所有纹理阶段
    for (u32 i = 0; i < MaxTextureUnits; ++i)
    {
        setActiveTexture(i, Material.getTexture(i));
        setTransform((E_TRANSFORMATION_STATE)(ETS_TEXTURE_0 + i), Material.getTextureMatrix(i));
    }
    
    // 渲染状态
    setRenderStates3DMode();  // Z缓冲、混合、Alpha测试等
    
    // 固定功能光照
    pID3DDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
    pID3DDevice->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
    // ... 更多状态
}
```

### 3.3 关键渲染状态映射

| Irrlicht 枚举 | Direct3D9 状态 | 说明 |
|---|---|---|
| `EMT_SOLID` | D3DRS_CULLMODE | 背面剔除 |
| `EMT_SOLID` | D3DRS_ZENABLE | 深度测试 |
| `EMT_TRANSPARENT` | D3DRS_ALPHABLENDENABLE | Alpha 混合 |
| `EMT_ONETEXTURE_BLEND` | D3DRS_BLENDOP | 混合操作 |

---

## 4. 纹理管理

### 4.1 纹理绑定 (setActiveTexture, 行 795-807)

```cpp
bool CD3D9Driver::setActiveTexture(u32 stage, const video::ITexture *texture)
{
    if (!texture)
    {
        pID3DDevice->SetTexture(stage, 0);
        pID3DDevice->SetTextureStageState(stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    }
    else
    {
        // 获取 D3D9 纹理指针
        IDirect3DBaseTexture9 *tex = ((CD3D9Texture*)(texture->getSurface()))->getDX9Texture();
        pID3DDevice->SetTexture(stage, tex);
    }
}
```

### 4.2 CD3D9Texture 接口

```cpp
class CD3D9Texture
{
    IDirect3DTexture9* Texture;      // 主纹理
    IDirect3DSurface9* RTTSurface;   // 渲染目标表面
    
public:
    IDirect3DBaseTexture9* getDX9Texture() { return Texture; }
    IDirect3DSurface9* getRenderTargetSurface() { return RTTSurface; }
};
```

---

## 5. 硬件缓冲区管理

### 5.1 创建顶点缓冲区 (行 1137-1155)

```cpp
SHWBufferLink_d3d9 *HWBuffer = new SHWBufferLink_d3d9(...);
DWORD flags = D3DUSAGE_WRITEONLY;
DWORD FVF = ...;  // 根据顶点类型

pID3DDevice->CreateVertexBuffer(bufSize, flags, FVF, D3DPOOL_DEFAULT, 
                                &HWBuffer->vertexBuffer, NULL);
```

### 5.2 创建索引缓冲区 (行 1205-1220)

```cpp
pID3DDevice->CreateIndexBuffer(bufSize, flags, 
    (indexType == EIT_32BIT) ? D3DFMT_INDEX32 : D3DFMT_INDEX16,
    D3DPOOL_DEFAULT, &HWBuffer->indexBuffer, NULL);
```

### 5.3 绑定和绘制 (行 1350-1368)

```cpp
// 绑定顶点流
pID3DDevice->SetStreamSource(0, HWBuffer->vertexBuffer, 0, stride);

// 绑定索引
pID3DDevice->SetIndices(HWBuffer->indexBuffer);

// 绘制
pID3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, ...);
```

---

## 6. 着色器支持 (HLSL)

### 6.1 创建 HLSL 材质渲染器 (addHighLevelShaderMaterial)

```cpp
s32 CD3D9Driver::addHighLevelShaderMaterial(
    const c8 *vertexShaderProgram,     // HLSL 顶点着色器
    const c8 *pixelShaderProgram,   // HLSL 像素着色器
    IShaderConstantSetCallBack *callback, ...)
{
    // 创建 HLSL 材质渲染器
    return-shaderIndex;
}
```

### 6.2 着色器绑定 (在材质渲染器内部)

```cpp
// CD3D9HLSLMaterialRenderer::OnSetMaterial()
pID3DDevice->SetVertexShader(pVertexShader);
pID3DDevice->SetPixelShader(pPixelShader);

// 设置常量
pID3DDevice->SetVertexShaderConstantF(startRegister, data, count);
pID3DDevice->SetPixelShaderConstantF(startRegister, data, count);
```

---

## 7. 完整渲染流程图

```
SceneManager::drawAll()
    │
    ▼
CD3D9Driver::beginScene()
    ├─ TestCooperativeLevel() 检查设备状态
    ├─ Clear() 清除缓冲区
    └─ BeginScene()
    │
    ▼
对于每个可见节点 ──→ CD3D9Driver::drawVertexPrimitiveList()
    │
    ├─ setVertexShader(vType)     // SetFVF
    ├─ setRenderStates3DMode()   // 渲染状态
    ├─ setMaterial()            // 纹理 + 材质
    │   ├─ setActiveTexture()  // SetTexture
    │   └─ setTransform()     // 矩阵
    ├─ OnSetMaterial()        // 着色器设置
    ├─ OnRender()            // 绘制调用
    │   └─ DrawIndexedPrimitive()
    │
    ▼
CD3D9Driver::endScene()
    ├─ EndScene()
    └─ GetSwapChain()->Present()
```

---

## 8. 关键 Direct3D9 API 参考表

### 设备生命周期
| API | 行号 | 用途 |
|---|---|---|
| `Direct3DCreate9()` | 204 | 创建 D3D9 接口 |
| `CreateDevice()` | 395-414 | 创建渲染设备 |
| `GetDeviceCaps()` | 423 | 获取设备能力 |
| `GetAvailableTextureMem()` | 425 | 查询显存 |

### 渲染帧
| API | 行号 | 用途 |
|---|---|---|
| `TestCooperativeLevel()` | 542 | 检测设备状态 |
| `Clear()` | 570 | 清除缓冲区 |
| `BeginScene()` | 575 | 开始场景 |
| `EndScene()` | 593 | 结束场景 |
| `Present()` | 611+ | 呈现画面 |

### 绘制
| API | 行号 | 用途 |
|---|---|---|
| `SetFVF()` | 3199 | 设置顶点格式 |
| `SetRenderState()` | ~162+ | 设置渲染状态 |
| `SetTexture()` | 805 | 绑定纹理 |
| `SetTransform()` | 750-758 | 设置变换矩阵 |
| `SetStreamSource()` | 1355 | 绑定顶点缓冲 |
| `SetIndices()` | 1364 | 绑定索引缓冲 |
| `DrawIndexedPrimitive()` | 1623 | 绘制图元 |

---

*Generated by Sisyphus for Irrlicht Engine Documentation*