# 3‑D Rendering Pipeline – Core Architecture & Extension Blueprint

## TL;DR
- **Deliverables**: 1) High‑level diagram; 2) List of core classes to touch; 3) Dependency checklist (shader cache, texture formats). 
- **Effort**: Medium (≈ 6‑8h of analysis + design). 
- **Parallel**: NO – all steps are linear; each block depends on the previous. 
- **Critical Path**: Draft diagram → Identify classes → Resolve dependencies.

## Context
- **Original request**: Full high‑level overview of the Irrlicht rendering pipeline. 
- **Scope**: Non‑modifying – no code changes, only design artifacts.

## Deliverables & Acceptance Criteria
- **Diagram**: Mermaid or PNG matching the pipeline sequence above, all stages labeled. 
- **Class list**: Includes: `CIrrDevice*`, `IVideoDriver*`, `ISceneManager*`, `ISceneNode*` (specific nodes listed), `ITexture`, `IMaterialRenderer*`, `CMeshCache`, `CTextureCache`. 
- **Dependency set**: Shader cache (`CShaderCache`), texture loaders (`CImageLoader*`), LOD selectors (`CMetaTriangleSelector`). All referenced files must exist and be reachable.
- **Design doc**: `.sisyphus/drafts/3d-rendering-pipeline-design.md`. Includes rationale and trade‑offs.

## Work Plan
1. **Collect core file paths** – `source/Irrlicht/CIrrDevice*.cpp`, `COpenGL*.cpp`, `CD3D8*.cpp`, `CD3D9*.cpp`, `CSceneLoaderIrr.cpp`, `CDefaultSceneNodeFactory.cpp`, `CAnimatedMeshSceneNode.cpp`, `CTerrainSceneNode.cpp`, `CLightSceneNode.cpp`, `CCameraSceneNode.cpp`, `IMaterialRendererManager.cpp`, `IMaterialRenderer.cpp`, `COpenGLSLMaterialRenderer.cpp`, `CMeshCache.cpp`, `CTextureCache.cpp`. 
2. **Draft the pipeline diagram** – Mermaid outline (Device init → Driver → SceneManager → Scene graph → Node traversal → Material rendering → Frame swap). 
3. **Identify minimal set to modify** – Pinpoint driver‑specific files, material renderers, node render hooks. 
4. **List dependency resolvers** – Confirm `CShaderCache`, `CImageLoaderDDS/TGA/PCX/PPM` existence. 
5. **Write design doc** – Combine findings, diagram, list, and rationale. 
6. **Review & refine** – Self‑review, trigger spec‑reviewer sub‑agent. 
7. **Final sign‑off** – Await user approval.

## Diagram (Mermaid)
```
flowchart TD
    A[Application starts] --> B[CIrrDevice* creates platform window]
    B --> C[CIrrDevice* selects IVideoDriver]
    C --> D[Driver (OpenGL/DirectX) sets up GPU context]
    D --> E[ISceneManager creates root node]
    E --> F[Scene graph built (nodes, transforms, materials)]
    F --> G[Scene graph traversal
       - on render: ISceneNode
g --> H[IVideoDriver::beginScene]
    G --> I[Material renderers set GPU state]
    I --> J[IVideoDriver::drawMesh]
    J --> K[IVideoDriver::endScene]
    K --> L[swap buffers]
```

## Key Files & Classes
- `source/Irrlicht/CIrrDevice*.cpp` – platform abstraction
- `source/Irrlicht/COpenGL*.cpp`, `CD3D8*.cpp`, `CD3D9*.cpp` – driver implementations
- `source/Irrlicht/ISceneManager.h` & `CSceneLoaderIrr.cpp` – scene graph
- `source/Irrlicht/ISceneNode.h` & specific node cpp files – geometry & transforms
- `source/Irrlicht/IMaterialRendererManager.cpp` – renderer registry
- `source/Irrlicht/COpenGLSLMaterialRenderer.cpp` – GL shader renderer
- `source/Irrlicht/CMeshCache.cpp`, `CTextureCache.cpp` – resource caching
- `source/Irrlicht/CShaderCache.cpp` – shader compilation and reuse
- `source/Irrlicht/CImageLoader*.cpp` – texture loading (DDS, TGA, PCX, PPM)

## Dependencies
- **Shader Cache**: `source/Irrlicht/CShaderCache.cpp`
- **Texture Loaders**: `source/Irrlicht/CImageLoaderDDS.cpp`, `CTGA.cpp`, `PCX.cpp`, `PPM.cpp`
- **Geometry Caching**: `CMeshCache.cpp`
- **Material Caching**: `CTextureCache.cpp`

## Design Rationale
- Keep driver abstraction for easy backend switch.
- Use material renderer registry for plug‑in effects.
- Reference counting for resources to avoid leaks.
- No STL or exception overhead by design.

---

For the actual implementation plan, I’ll draft a .mmd file with task details and then store it in `.sisyphus/plans/3d-rendering-pipeline.md`. All source files remain untouched as per the constraints.
