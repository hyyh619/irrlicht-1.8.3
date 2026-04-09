# Testing Patterns

**Analysis Date:** 2026-04-09

## Test Framework

**None** - The Irrlicht Engine does NOT use a dedicated unit testing framework.

- No Google Test, Catch2, or similar
- No CppUnit or other C++ test frameworks
- No test runner configuration

## Validation Approach

The project validates functionality through:

### 1. Examples as Validation

Examples in `examples/` serve as functional validation:

```
examples/
├── 01.HelloWorld/        # Basic device creation, rendering loop
├── 02.Quake3Map/         # Mesh loading
├── 03.CustomSceneNode/   # Custom scene node creation
├── 04.Movement/          # Animation and movement
├── 05.UserInterface/    # GUI elements
├── 06.2DGraphics/       # 2D rendering
├── 07.Collision/        # Collision detection
├── 08.SpecialFX/        # Particle systems, effects
├── 09.Meshviewer/       # Mesh viewing
├── 10.Shaders/          # Shader programming
├── 11.PerPixelLighting/ # Advanced lighting
├── 12.TerrainRendering/ # Terrain
├── 13.RenderToTexture/  # Render targets
├── 14.Win32Window/      # Platform-specific
├── 15.LoadIrrFile/      # Custom file format
├── 16.Quake3MapShader/  # Quake3 shaders
├── 21.Quake3Explorer/   # Quake3 BSP loading
├── 25.XmlHandling/      # XML parsing
├── 26.OcclusionQuery/   # Hardware occlusion
└── Demo/                # Comprehensive demo
```

### 2. Compiled Binaries

Validation binaries are pre-built in `bin/`:

```
bin/
├── Linux/         # Linux executables
├── MacOSX/        # macOS executables (most common)
├── Win32-gcc/     # Windows (MinGW)
└── Win64-VisualStudio/  # Windows (MSVC)
```

Each example compiles to an executable that runs and demonstrates functionality.

## Test File Locations

### No Dedicated Tests Directory

The project does NOT have a `tests/` or `test/` directory.

### Third-Party Test Files

The bundled third-party libraries have their own test files:

- `source/Irrlicht/libpng/pngtest.c` - PNG library test
- `source/Irrlicht/jpeglib/testimg.jpg` - JPEG test images
- `source/Irrlicht/bzip2/dlltest.c` - bzip2 test

These are NOT unit tests for the engine itself.

## Building and Testing

### Build Commands

```bash
# Build engine
cd source/Irrlicht && make

# Build all examples (macOS)
cd examples && make all_macos

# Build single example
cd examples/01.HelloWorld && make -f Makefile.macosx
```

### Running Examples

Execute binaries from `bin/MacOSX/`:

```bash
./bin/MacOSX/01.HelloWorld
./bin/MacOSX/02.Quake3Map
```

### Validation Checklist

To verify the engine works:

1. Build succeeds without errors
2. Example executables launch without crashes
3. Window opens with expected rendering
4. No console errors on startup

## Test Code Patterns

### No Test Infrastructure

The engine contains NO patterns like:

```cpp
// NOT FOUND in codebase
#include <gtest/gtest.h>

TEST(SceneManager, AddNode) { ... }
```

### No Unit Test Files

Search results confirm:
- No `*test*.cpp` files in `source/Irrlicht/`
- No `*spec*.cpp` files
- No `*_test.cpp` files
- No Catch2/CppUnit headers

## Integration Testing

The "tests" are effectively integration tests via examples:

| Example | Tests |
|---------|-------|
| 01.HelloWorld | Device creation, render loop |
| 02.Quake3Map | BSP loading, Quake3 textures |
| 03.CustomSceneNode | Scene node registration |
| 07.Collision | Collision geometry |
| 10.Shaders | GLSL shader compilation |
| 25.XmlHandling | XML reading/writing |

## Code Coverage

**Not measured** - No code coverage tools or requirements.

## Recommendations for Adding Tests

If tests were to be added to this project:

1. **Use a lightweight framework** - Google Test or Catch2
2. **Create `tests/` directory** at root level
3. **Test core components:**
   - `core::array<T>` operations
   - `core::string<T>` operations
   - Reference counting (grab/drop)
   - Math utilities
4. **Build with test CMake/Makefile separate from engine**

---

*Testing analysis: 2026-04-09*
