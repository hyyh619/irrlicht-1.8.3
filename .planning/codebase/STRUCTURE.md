# Codebase Structure

**Analysis Date:** 2026-04-29

## Directory Layout

```
irrlicht-1.8.3/
├── include/               # Public API headers (I-prefixed interfaces)
├── source/Irrlicht/      # Engine implementation (C-prefixed classes)
├── examples/            # 26 tutorial examples + Demo
├── lib/                 # Pre-built static/dynamic libraries
├── media/               # Textures, models, shaders for demos
├── tools/               # MeshConverter, GUIEditor, IrrFontTool
├── doc/                # Documentation (old)
├── docs/                # Documentation (new/GSD)
├── bin/                 # Binary outputs
└── .planning/          # GSD planning documents
```

## Directory Purposes

**include/:**
- Purpose: Stable public API headers
- Contains: I-prefixed interfaces (IVideoDriver, ISceneManager, etc.)
- Key files: `irrlicht.h`, `IEventReceiver.h`, `IVideoDriver.h`

**source/Irrlicht/:**
- Purpose: Engine implementation
- Contains: C-prefixed implementations
- Key files: `Irrlicht.cpp`, `CSceneManager.cpp`, `COpenGLDriver.cpp`

**examples/:**
- Purpose: Tutorial examples
- Contains: 01.HelloWorld through 26.OcclusionQuery, plus Demo
- Key files: `main.cpp`, Makefiles

**media/:**
- Purpose: Test assets for examples
- Contains: Textures (.jpg, .png), models (.md2, .md3, .obj)
- Generated: No

**lib/:**
- Purpose: Pre-built libraries
- Contains: `Irrlicht.lib`, `Irrlicht.exp`
- Generated: Yes (from engine build)

**tools/:**
- Purpose: Utility applications
- Contains: MeshConverter, IrrFontTool
- Generated: Yes

## Key File Locations

**Entry Points:**
- `source/Irrlicht/Irrlicht.cpp`: createDevice(), createDeviceEx() - Main engine factory
- `include/irrlicht.h`: Public include for applications

**Configuration:**
- `source/Irrlicht/IrrCompileConfig.h`: Compile-time options
- `source/Irrlicht/Makefile`: Build configuration

**Core Logic:**
- `source/Irrlicht/CSceneManager.cpp`: Scene graph management
- `source/Irrlicht/CIrrDeviceWin32.cpp`: Windows device implementation
- `source/Irrlicht/COpenGLDriver.cpp`: OpenGL rendering driver
- `source/Irrlicht/CD3D9Driver.cpp`: Direct3D 9 rendering driver

**Scene Nodes (source/Irrlicht/):**
- `CMeshSceneNode.cpp`: Generic mesh rendering
- `CCameraSceneNode.cpp`: Camera controls
- `CLightSceneNode.cpp`: Light sources
- `CParticleSystemSceneNode.cpp`: Particle effects
- `CTerrainSceneNode.cpp`: Terrain rendering

**GUI (source/Irrlicht/):**
- `CGUIEnvironment.cpp`: GUI manager
- `CGUIButton.cpp`, `CGUIEditBox.cpp`, etc.

**File Loaders (source/Irrlicht/):**
- `CMD2MeshFileLoader.cpp`: Quake 2 models
- `COBJMeshFileLoader.cpp`: Wavefront OBJ
- `CImageLoaderPNG.cpp`, `CImageLoaderJPG.cpp`: Image formats
- `CZipReader.cpp`, `CPakReader.cpp`: Archive formats

**Rendering (source/Irrlicht/):**
- `CTR*.cpp`: Triangle renderers (CTRFlat, CTRGouraud, etc.)
- `COpenGLTexture.cpp`, `CD3D9Texture.cpp`: Texture management
- `COpenGLMaterialRenderer.cpp`: Shader materials

## Naming Conventions

**Files:**
- Interfaces: `I` prefix + camelCase (e.g., `IVideoDriver.h`)

- Implementations: `C` prefix + camelCase (e.g., `COpenGLDriver.cpp`)

- Scene loaders: `C` prefix + format + MeshFileLoader (e.g., `COBJMeshFileLoader.cpp`)

- Image loaders: `CImageLoader` + format (e.g., `CImageLoaderPNG.cpp`)

- GUI elements: `CGUI` + element name (e.g., `CGUIButton.cpp`)

- Material renderers: `CTR` + material type (e.g., `CTRGouraud.cpp`)

**Directories:**
- All lowercase, single nouns (e.g., `source/Irrlicht/`, `include/`, `media/`)

**Source Code:**
- Classes: `C` prefix for implementations, `I` prefix for interfaces
- Methods: camelCase (e.g., `getMesh()`, `addSceneNode()`)
- Variables: camelCase (e.g., `WindowSize`, `DriverType`)
- Constants: `E_` prefix for enumerations (e.g., `E_SCENE_NODE_RENDER_PASS`)

## Where to Add New Code

**New Scene Node:**
- Implementation: `source/Irrlicht/CNewSceneNode.cpp`
- Header: `source/Irrlicht/CNewSceneNode.h`
- Factory registration: `source/Irrlicht/CDefaultSceneNodeFactory.cpp`

**New Mesh Loader:**
- Implementation: `source/Irrlicht/CNewMeshFileLoader.cpp`
- Register in: `CSceneManager.cpp`

**New Image Format:**
- Implementation: `source/Irrlicht/CImageLoaderFORMAT.cpp`
- Register in: `COpenGLDriver.cpp` or `CD3D9Driver.cpp`

**New GUI Element:**
- Implementation: `source/Irrlicht/CGUINewElement.cpp`
- Header: `include/IGUINewElement.h`
- Factory: `source/Irrlicht/CDefaultGUIElementFactory.cpp`

**New Material Renderer:**
- Implementation: `source/Irrlicht/CTRNewMaterial.cpp`
- Register in: `IVideoDriver::registerMaterialRenderer()`

**New Platform Device:**
- Implementation: `source/Irrlicht/CIrrDeviceNEW.cpp`
- Include in: `source/Irrlicht/Irrlicht.cpp`

## Special Directories

**source/Irrlicht/zlib/:**
- Purpose: Compression library
- Generated: No (embedded third-party)
- Committed: Yes

**source/Irrlicht/libpng/:**
- Purpose: PNG image support
- Generated: No (embedded third-party)
- Committed: Yes

**source/Irrlicht/jpeglib/:**
- Purpose: JPEG image support
- Generated: No (embedded third-party)
- Committed: Yes

**source/Irrlicht/bzip2/:**
- Purpose: bzip2 compression
- Generated: No (embedded third-party)
- Committed: Yes

**source/Irrlicht/lzma/:**
- Purpose: LZMA compression
- Generated: No (embedded third-party)
- Committed: Yes

**source/Irrlicht/aesGladman/:**
- Purpose: AES encryption
- Generated: No (embedded third-party)
- Committed: Yes

**source/Irrlicht/MacOSX/:**
- Purpose: macOS platform specifics
- Generated: No
- Committed: Yes

---

*Structure analysis: 2026-04-29*