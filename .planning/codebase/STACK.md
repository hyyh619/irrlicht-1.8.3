# Technology Stack

**Analysis Date:** 2026-04-09

## Languages

**Primary:**
- **C++** (pre-11 standard) - Core engine implementation
  - Compiled with `-fno-exceptions -fno-rtti` (exceptions and RTTI disabled)
  - Uses custom container types (`irr::core`) instead of STL

**Secondary:**
- **C** - Third-party libraries (zlib, libpng, jpeglib, bzip2)
- **Objective-C++** - macOS platform integration (`*.mm` files in `MacOSX/`)

## Build System

**Primary:** GNU Make
- `source/Irrlicht/Makefile` - Main engine build (Linux/Unix)
- `source/Irrlicht/MacOSX/Makefile` - macOS specific compilation (arm64)
- `examples/Makefile` - Example applications
- Individual example Makefiles in `examples/*/`

**Build Output:**
- Static library: `libIrrlicht.a` (Linux/macOS)
- Shared library: `libIrrlicht.so` (Linux)
- Prebuilt binaries: `lib/` folder for Windows, Linux, macOS

**Build Variants:**
- Debug: `make` (includes `-g -O0 -D_DEBUG`)
- Release: `make NDEBUG=1` (includes `-O3`)
- Shared library: `make sharedlib`
- macOS: `make staticlib_osx` or `make sharedlib_osx`

## Key Dependencies

**Bundled (in source/Irrlicht/):**
- **zlib** (1.2.8) - Compression library (embedded)
- **libpng** (1.5.x) - PNG image format support
- **jpeglib** (Independent JPEG Group) - JPEG image support
- **bzip2** - Compression library
- **lzma** - LZMA compression (for archive reading)
- **aesGladman** - AES encryption (for archive formats)

**System (required):**
- **Linux:** X11 (XServer with dev headers), OpenGL (optional)
- **macOS:** Cocoa framework, OpenGL headers
- **Windows:** Platform SDK, DirectX SDK (optional for D3D8/D3D9)

## Platform Support

| Platform | Renderer Backends | Device Type |
|----------|-------------------|-------------|
| Windows | OpenGL, Direct3D 8/9, Software | `CIrrDeviceWin32` |
| Linux | OpenGL, Software, Framebuffer | `CIrrDeviceLinux`, `CIrrDeviceFB` |
| macOS | OpenGL | `CIrrDeviceMacOSX` (Cocoa) |
| SDL | Cross-platform | `CIrrDeviceSDL` |
| Console | Software | `CIrrDeviceConsole` |

**Supported Architectures:**
- x86 (32-bit)
- x86_64 (64-bit)
- arm64 (Apple Silicon)

## Renderer Backends

**OpenGL Driver:** `COpenGLDriver.cpp`
- GLSL shader support
- Normal mapping, parallax mapping
- VBO (Vertex Buffer Objects)
- RTT (Render To Texture)

**Direct3D Drivers:**
- `CD3D8Driver.cpp` - DirectX 8 (legacy)
- `CD3D9Driver.cpp` - DirectX 9

**Software Renderer:** `CSoftwareDriver.cpp` (`source/Irrlicht/CSoftwareDriver.cpp`)
- Fallback renderer for systems without GPU
- "Burnings Video" software rasterizer (advanced 2D primitives)

## Configuration

**Compiler Requirements:**
- GCC 4.x
- Visual Studio 2008-2012
- Code::Blocks with gcc/Visual Studio

**Key Compiler Flags:**
```makefile
-fno-exceptions    # No exception support
-fno-rtti          # No RTTI
-fstrict-aliasing  # Aggressive aliasing optimization
```

---

*Stack analysis: 2026-04-09*