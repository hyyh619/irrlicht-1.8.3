# External Integrations

**Analysis Date:** 2026-04-09

## Third-Party Libraries

**Image Format Support (bundled):**
| Library | Purpose | Location |
|---------|---------|----------|
| libpng | PNG image loading/writing | `source/Irrlicht/libpng/` |
| jpeglib (IJG) | JPEG image loading/writing | `source/Irrlicht/jpeglib/` |
| zlib | PNG compression, general compression | `source/Irrlicht/zlib/` |
| bzip2 | .bz2 archive compression | `source/Irrlicht/bzip2/` |

**Compression/Encryption:**
| Library | Purpose |
|---------|---------|
| lzma | LZMA decompression (archive reading) |
| aesGladman | AES encryption (for encrypted archives) |

## File Format Support

**3D Mesh Formats (built-in loaders):**
| Format | Loader File | Notes |
|--------|-------------|-------|
| Quake 3 (.md3) | `CMD3MeshFileLoader.cpp` | Animated meshes |
| Quake 3 (.pk3) | `CZipReader.cpp` | Archive format |
| Quake 2 (.md2) | `CMD2MeshFileLoader.cpp` | Animated meshes |
| Quake 3 BSP (.bsp) | `CBSPMeshFileLoader.cpp` | Level geometry |
| Doom 3 (.md5) | `CMD3MeshFileLoader.cpp` | Skeletal animation |
| OBJ | `COBJMeshFileLoader.cpp` | Wavefront OBJ |
| Collada (.dae) | `CColladaFileLoader.cpp` | XML-based |
| 3DS (.3ds) | `C3DSMeshFileLoader.cpp` | Autodesk 3DS |
| B3D (.b3d) | `CB3DMeshFileLoader.cpp` | Blitz3D |
| MS3D (.ms3d) | `CMS3DMeshFileLoader.cpp` | MilkShape 3D |
| LMTS | `CLMTSMeshFileLoader.cpp` | LMTS format |
| Ogre (.mesh) | `COgreMeshFileLoader.cpp` | Ogre3D |
| X (.x) | `CXMeshFileLoader.cpp` | DirectX format |
| STL | `CSTLMeshFileLoader.cpp` | Stereo Lithography |
| PLY | `CPLYMeshFileLoader.cpp` | Polygon File Format |
| DMF | `CDMFLoader.cpp` | Darkstar DMF |
| CSM | `CCSMLoader.cpp` | Character Studio Motion |
| MY3D | `CMY3DMeshFileLoader.cpp` | My3D format |
| OCT | `COCTLoader.cpp` | Octree format |
| SMF | `CSMFMeshFileLoader.cpp` | Simple Model Format |
| LWO | `CLWOMeshFileLoader.cpp` | LightWave Object |
| Irrlicht Scene | `CSceneLoaderIrr.cpp` | `.irr` XML format |

**Image Formats (built-in loaders/writers):**
| Format | Loader | Writer |
|--------|--------|--------|
| BMP | `CImageLoaderBMP.cpp` | `CImageWriterBMP.cpp` |
| JPEG | `CImageLoaderJPG.cpp` | `CImageWriterJPG.cpp` |
| PNG | `CImageLoaderPNG.cpp` | `CImageWriterPNG.cpp` |
| TGA | `CImageLoaderTGA.cpp` | `CImageWriterTGA.cpp` |
| PCX | `CImageLoaderPCX.cpp` | `CImageWriterPCX.cpp` |
| DDS | `CImageLoaderDDS.cpp` | - |
| PSD | `CImageLoaderPSD.cpp` | `CImageWriterPSD.cpp` |
| PPM | `CImageLoaderPPM.cpp` | `CImageWriterPPM.cpp` |
| WAL | `CImageLoaderWAL.cpp` | - |
| RGB | `CImageLoaderRGB.cpp` | - |

## Archive/IO Systems

**Archive Formats (built-in readers):**
| Format | Reader | Purpose |
|--------|--------|---------|
| ZIP | `CZipReader.cpp` | ZIP archives |
| GZip | zlib | .gz files |
| PAK | `CPakReader.cpp` | Quake PAK archives |
| WAD | `CWADReader.cpp` | WAD archives |
| TAR | `CTarReader.cpp` | TAR archives |
| NPK | `CNPKReader.cpp` | Nereid Package |
| LZMA | lzma | LZMA compressed |

**XML Support:**
- `irrXML.cpp` - Lightweight XML parser (internal implementation)
- `CXMLReader.cpp` / `CXMLWriter.cpp` - XML reading/writing
- Scene serialization to `.irr` XML format

## Network/IO Systems

**File System:**
- `CFileSystem.cpp` - Virtual file system with mount points
- `CFileList.cpp` - Directory listing
- Supports archive mounting (transparent archive access)

**No built-in network stack** - Applications must implement networking separately.

## Plugin/Extension Systems

**Scene Node Factories:**
- `CDefaultSceneNodeFactory.cpp` - Default scene node creation
- Custom factories can be registered via `ISceneNodeFactory`

**Scene Node Animator Factories:**
- `CDefaultSceneNodeAnimatorFactory.cpp` - Default animators
- FPS camera, Maya-style camera, collision response, etc.

**Material Renderers:**
- `IGPUProgrammingServices` - Custom shader registration
- OpenGL: `COpenGLShaderMaterialRenderer`, `COpenGLSLMaterialRenderer`
- DirectX: `CD3D8ShaderMaterialRenderer`, `CD3D9HLSLMaterialRenderer`

**GUI Element Factory:**
- `CDefaultGUIElementFactory.cpp` - Default GUI controls
- Custom GUI elements via `IGUIElementFactory`

## Shader Systems

**OpenGL:**
- GLSL (OpenGL Shading Language) via `COpenGLSLMaterialRenderer`
- Assembly shaders (legacy)

**DirectX:**
- HLSL (High-Level Shading Language) via `CD3D9HLSLMaterialRenderer`
- Assembly shaders (legacy)

---

*Integration audit: 2026-04-09*