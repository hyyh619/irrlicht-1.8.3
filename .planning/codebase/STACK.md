# Technology Stack

**Analysis Date:** 2026-04-29

## Languages

**Primary:**
- C++ (C++03 standard with some modern features) - Core engine, rendering drivers, scene management, GUI, all core implementation

**Secondary:**
- C - Embedded libraries (zlib, bzip2), some legacy code
- Assembly - Minimal inline assembly for performance-critical paths in image processing

## Runtime

**Environment:**
- Native C++ compilation (no managed runtime)
- Platform-specific: Windows (Win32/Win64), Linux (X11/Framebuffer), macOS (Cocoa), SDL (cross-platform)

**Build System:**
- Hand-written Makefiles (GNU Make on Linux/macOS)
- Visual Studio project files (.vcxproj for VS2010-2019)
- No modern CMake or other build system

## Frameworks

**Core:**
- Irrlicht Engine 1.8.3 - 3D graphics engine
  - Rendering drivers: OpenGL, Direct3D 8/9, Software renderer
  - Scene graph management
  - GUI system (built-in)
  - Particle system
  - Physics/collision detection

**Bundled Libraries (embedded in source):**
- zlib 1.2.x - ZIP archive support and compression
- libpng - PNG image loading/saving
- jpeglib (IJG) - JPEG image loading/saving
- bzip2 - BZ2 archive support and compression
- LZMA SDK - LZMA compression for NPK archives
- aesGladman - AES encryption for archives

**Testing:**
- No formal test framework - manual testing only
- Conformance tests exist via skill (`.opencode/skills/irrlicht-conform-test/`)
- Build with `CONFORM_TEST=1` to enable screenshot comparison tests

**Build/Dev:**
- Visual Studio 2010-2019 (.vcxproj)
- GNU Make
- GCC/Clang on Linux
- MinGW on Windows

## Key Dependencies

**Critical (bundled):**
- No external dependencies - all libraries are bundled in source
- zlib - Archive handling, mesh compression
- libpng - Texture loading
- jpeglib - Texture loading

**Infrastructure:**
- Platform graphics APIs:
  - OpenGL (all platforms)
  - Direct3D 8/9 (Windows only)
  - SDL 1.x (optional, for cross-platform device)
- OS windowing systems: Win32, X11, Cocoa, FB (framebuffer)

## Configuration

**Build Configuration:**
- `source/Irrlicht/Makefile` - Main engine build
  - `NDEBUG=1` - Release mode
  - Platform targets: `win32`, `linux`, `darwin`
- `IrrCompileConfig.h` - Compile-time feature flags
  - Device selection: Windows Device, X11, SDL, Console, Framebuffer
  - Driver selection: OpenGL, Direct3D 8/9, Software
  - Optional features: ZIP support, GUI, textures, etc.

**Environment:**
- No runtime configuration files - all via API
- No .env or environment variable handling in engine core

## Platform Requirements

**Development:**
- C++ compiler with RTTI support
- Platform SDK (DirectX SDK for Windows, X11 dev libs for Linux)
- Make or MSVC 2010+

**Production:**
- Platform runtime (Windows, Linux, macOS)
- For Windows: DirectX 8/9 or OpenGL driver
- For Linux: OpenGL + X11 libs
- No additional runtime dependencies (static linking typical)

---

*Stack analysis: 2026-04-29*