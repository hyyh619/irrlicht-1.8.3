# Irrlicht Engine Repository

## Overview

This is the **Irrlicht Engine 1.8.3** - an open-source C++ 3D graphics engine. The repo contains:
- Engine source code (`source/Irrlicht/`)
- Public API headers (`include/`)
- Example tutorials (`examples/`)
- Pre-built libraries (`lib/`)
- Media assets (`media/`)
- Tools (`tools/`)

## Directory Structure

| Directory | Purpose |
|-----------|----------|
| `include/` | Public API headers (I-prefixed interfaces) |
| `source/Irrlicht/` | Engine implementation (C-prefixed classes) |
| `examples/` | 26 tutorial examples |
| `lib/` | Pre-built static/dynamic libraries |
| `media/` | Textures, models, shaders for demos |
| `tools/` | MeshConverter, GUIEditor, IrrFontTool |

## Build Commands

### Build Engine Library
```bash
cd source/Irrlicht && make           # Debug build (default)
cd source/Irrlicht && make NDEBUG=1  # Release build
```

### Build Single Example
```bash
cd examples/01.HelloWorld && make          # Linux
cd examples/01.HelloWorld && make all_macos  # macOS
```

### Build All Examples
```bash
cd examples && make
```

## Key Conventions

### Naming
- **Interfaces**: `I` prefix (e.g., `IVideoDriver`, `ISceneManager`)
- **Implementations**: `C` prefix (e.g., `COpenGLDriver`, `CSceneManager`)
- **Platform devices**: `CIrrDevice*` (e.g., `CIrrDeviceWin32`, `CIrrDeviceLinux`)

### Memory Management
- Objects created via `create...()` or `add...()` need `drop()` when done
- Public API uses `IReferenceCounted` with `grab()`/`drop()`

### Containers
- Use `irr::core` containers (e.g., `core::array`, `core::string`), **NOT** `std::`

### Platform Code
- Isolated in `CIrrDevice*.cpp` and OS subdirectories (e.g., `MacOSX/`)
- Avoid direct OS API calls outside device layer

### Code Style
- Doxygen-formatted comments in headers
- `using namespace irr;` typical in examples, explicit `irr::` in core source

## Special Notes

### Embedded Libraries
Source includes bundled copies of: zlib, libpng, jpeglib, bzip2, lzma, aesGladman

### Platform Targets
- `CIrrDeviceWin32` - Windows (Direct3D + OpenGL)
- `CIrrDeviceLinux` - Linux (OpenGL)
- `CIrrDeviceSDL` - Cross-platform (OpenGL via SDL)
- `MacOSX/` - macOS ( Cocoa + OpenGL)

### Examples
| Example | Feature |
|---------|----------|
| 01.HelloWorld | Basic engine setup |
| 02.Quake3Map | Load BSP map, Octree optimization |
| 07.Collision | Collision detection |
| 10.Shaders | High-level shaders |
| Demo | Comprehensive tech demo |

## Important Gotchas

1. **No modern build system**: Uses hand-written Makefiles, not CMake
2. **No test suite**: No automated tests - verify manually
3. **macOS builds**: Use `all_macos` target, not default
4. **OpenGL version**: Examples default to OpenGL on macOS (uses `driverChoice.h`)
5. **Legacy code**: ~2010-era codebase - expect older C++ patterns

## Related Knowledge Bases

For specific areas, consult:

- **`include/AGENTS.md`** - Public API documentation
- **`source/Irrlicht/AGENTS.md`** - Engine implementation details  
- **`examples/AGENTS.md`** - Tutorial example guide