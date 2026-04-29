# Testing Patterns

**Analysis Date:** 2026-04-29

## Test Framework

### Status
No formal automated test framework exists in this codebase.

**Framework:** None
- No unit test framework (gtest, Catch2, CppUnit)
- No automated test runner
- Manual verification required

**Verification Approach:**
- Manual testing via examples
- CONFORM_TEST macro for screenshot-based visual regression
- Build-time verification only

### Why No Framework Exists
This is a legacy C++ graphics engine from ~2010 era that predates widespread adoption of unit testing in C++ graphics codebases. The focus was on functionality and performance rather than test coverage.

## Test File Organization

### Location
No dedicated test directory exists.

**Pattern:** Not applicable - tests integrated into examples

### Existing "Tests"

1. **Example Programs** - Each example in `examples/` demonstrates a feature
   - `01.HelloWorld` - Basic setup
   - `02.Quake3Map` - BSP loading
   - `07.Collision` - Collision detection
   - `10.Shaders` - Shader system

2. **Media Assets** in `source/Irrlicht/jpeglib/` - Test images for JPEG loading
   - `testorig.jpg`, `testimg.jpg`, `testprog.jpg`

## Test Structure

### Manual Verification Pattern
Examples serve as functional tests. Each must compile and run without crashing.

```cpp
// From examples/01.HelloWorld/main.cpp - typical structure
int main()
{
    // Create device
    IrrlichtDevice *device = createDevice(video::EDT_DIRECT3D9, ...);
    if (!device)
        return 1;

    // Get subsystems
    IVideoDriver *driver = device->getVideoDriver();
    ISceneManager *smgr = device->getSceneManager();
    IGUIEnvironment *guienv = device->getGUIEnvironment();

    // Load/test features
    IAnimatedMesh *mesh = smgr->getMesh("../../media/sydney.md2");
    if (!mesh)
    {
        device->drop();
        return 1;
    }

    // Render loop
    while (device->run())
    {
        driver->beginScene(true, true, SColor(255, 100, 101, 140));
        smgr->drawAll();
        guienv->drawAll();
        driver->endScene();
    }

    device->drop();
    return 0;
}
```

## CONFORM_TEST - Visual Regression Testing

### Overview
CONFORM_TEST is a manual screenshot-based testing mechanism for visual regression.

### Pattern
Each example that supports CONFORM_TEST follows this pattern:

1. **Define CONFORM_TEST** (if not defined by build system):
```cpp
// From examples/01.HelloWorld/main.cpp (line 85-87)
#ifndef CONFORM_TEST
#define CONFORM_TEST 0
#endif
```

2. **Conditional Driver Selection**:
```cpp
// From examples/02.Quake3Map/main.cpp
#if CONFORM_TEST
    video::E_DRIVER_TYPE driverType = video::EDT_DIRECT3D9;
#else
    video::E_DRIVER_TYPE driverType = driverChoiceConsole();
#endif
```

3. **Screenshot Capture** (in render loop):
```cpp
// From examples/01.HelloWorld/main.cpp (lines 230-241)
#if CONFORM_TEST
#pragma message("CONFORM_TEST")

    video::IImage *image = device->getVideoDriver()->createScreenShot();
    if (image)
    {
        device->getVideoDriver()->writeImageToFile(image, "screenshot.bmp");
        image->drop();
    }

    break;
#endif
```

### Using the Conform Test Skill
The project includes an `irrlicht-conform-test` skill for automated testing:
- Builds project with `CONFORM_TEST=1` enabled
- Runs example programs to capture screenshots
- Compares with golden reference screenshots
- Reports any test failures

### Build Commands
```bash
# Debug build
cd source/Irrlicht && make

# Release build
cd source/Irrlicht && make NDEBUG=1

# Build all examples
cd examples && make

# Build single example
cd examples/01.HelloWorld && make
```

## Mocking

### Not Applicable
No mocking framework is used. The engine is:
- Entirely self-contained
- Uses custom containers (not std::)
- No dependency injection pattern

### What Could Be Mocked (if tests existed)
- File system operations (mock `IFileSystem`)
- Video driver (mock `IVideoDriver`)
- GUI environment (mock `IGUIEnvironment`)

## Test Fixtures and Data

### Media Assets Location
- `media/` - Textures, models, shaders for examples
  - `sydney.md2` - Quake 2 model
  - `sydney.bmp` - Texture
  - Various BSP maps for Quake 3 testing

### Test Patterns from jpeglib
The bundled `jpeglib` has test images in `source/Irrlicht/jpeglib/`:
- `testorig.jpg` - Original test image
- `testimg.jpg`, `testimg.bmp`, `testimg.ppm` - Output formats

## Coverage

### Requirements
None enforced. No code coverage tools configured.

### Current State
- Zero automated unit tests
- Integration testing only via examples
- No coverage measurement

## Test Types

### Manual Feature Testing
- Compile and run each example
- Verify visual output matches expected behavior
- Check for crashes/errors in console output

### Build Verification
- All examples must compile without errors
- Library must build for target platform

### Visual Regression
- Use CONFORM_TEST to capture screenshots
- Compare against known-good baseline

## Common Patterns (for future testing)

### If Adding Tests
```cpp
// Test structure for future unit tests (NOT currently in codebase)

// 1. Use irr::core containers (NOT std::)
core::array<int> testArray;
testArray.push_back(42);

// 2. Use engine types
u32 result = 0;
s32 signedResult = -1;

// 3. Reference counting
IReferenceCounted* obj = createSomething();
obj->drop();  // Must match create*/

// 4. Assertions in debug
_IRR_DEBUG_BREAK_IF(condition);

// 5. Logging tests
os::Printer::log("Test message", ELL_INFORMATION);
```

### Example Verification Checklist
- [ ] Device creation succeeds
- [ ] Subsystem access works (driver, scene manager, GUI)
- [ ] Resources load (mesh, textures)
- [ ] Render loop executes without crash
- [ ] Memory is properly released via drop()

---

*Testing analysis: 2026-04-29*