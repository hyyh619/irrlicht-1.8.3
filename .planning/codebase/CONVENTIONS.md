# Coding Conventions

**Analysis Date:** 2026-04-29

## Naming Patterns

### Files
- **Headers:** `I` prefix for interfaces (e.g., `IVideoDriver.h`, `ISceneManager.h`) in `include/`
- **Implementations:** `C` prefix for implementations (e.g., `COpenGLDriver.cpp`, `CSceneManager.cpp`) in `source/Irrlicht/`
- **Platform-specific:** `CIrrDevice` prefix with platform suffix (e.g., `CIrrDeviceWin32.cpp`, `CIrrDeviceLinux.cpp`)

### Classes
- **Interfaces:** `I` prefix - e.g., `IReferenceCounted`, `IVideoDriver`, `ISceneManager`
- **Implementations:** `C` prefix - e.g., `CNullDriver`, `COpenGLDriver`, `CSceneManager`
- **Structs:** `S` prefix for data structures - e.g., `SMaterial`, `SLight`, `SColor`

### Variables
- **Member variables:** Underscore suffix (e.g., `ReferenceCounter`, `DebugName`)
- **Types:** Uses custom typedefs - `u8`, `s8`, `u16`, `s16`, `u32`, `s32`, `f32`, `f64` (see `include/irrTypes.h`)
- **Constants:** `E` prefix for enumerations - e.g., `ELL_WARNING`, `EMF_LIGHTING`

### Functions
- **Methods:** camelCase (e.g., `createDevice`, `getVideoDriver`, `run`)
- **Factory methods:** `create` prefix - e.g., `createDevice()`, `createTexture()`
- **Getters:** `get` prefix - e.g., `getVideoDriver()`, `getReferenceCount()`
- **Predicates:** `is` prefix - e.g., `isEmpty()`, `isFrontFacing()`

## Code Style

### Formatting
- **Standard:** Not enforced via auto-formatter - uses hand-written Makefiles
- **Indentation:** 4 spaces (tabs not used in main source)
- **Line length:** No strict limit - typical 80-120 characters

### Linting
- **Tool:** None - no automated linting in build system
- **Manual review:** Code is manually maintained

### Namespace Usage
- **Headers:** Uses explicit `irr::` prefix (e.g., `irr::core::array`, `irr::video::IVideoDriver`)
- **Implementation:** Often uses `using namespace irr;` after includes
- **Sub-namespaces:** `core`, `video`, `scene`, `gui`, `io`, `os`

### Header Organization
```cpp
// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and see copyright notice in irrlicht.h

#ifndef __IRR_STRING_H_INCLUDED__
#define __IRR_STRING_H_INCLUDED__

#include "irrTypes.h"
// ... other includes

namespace irr
{
    namespace core
    {
        // ... class definitions
    } // end namespace core
} // end namespace irr

#endif
```

## Memory Management

### Reference Counting
- Most engine objects inherit from `IReferenceCounted` (see `include/IReferenceCounted.h`)
- Objects created via `create*()` or `add*()` require manual `drop()` when done
- Methods without `create` prefix return owned objects that don't need `drop()`

```cpp
// Example from examples/01.HelloWorld/main.cpp
IrrlichtDevice *device = createDevice(video::EDT_DIRECT3D9, ...);
if (!device)
    return 1;
// ... use device
device->drop();  // Must drop objects created with create*

// Textures loaded via getTexture() don't need drop():
ITexture* tex = driver->getTexture("texture.png");  // Managed by driver
```

### Custom Allocators
- Uses custom `irrAllocator<T>` template (see `include/irrAllocator.h`)
- Array class uses allocator pattern:
```cpp
// From include/irrArray.h
template<class T, typename TAlloc = irrAllocator<T>>
class array
{
    T* data;
    TAlloc allocator;
    // ...
};
```

## Containers

### irr::core Containers (NOT std::)
- `core::array<T>` - Dynamic array (see `include/irrArray.h`)
- `core::string<T>` - String class (see `include/irrString.h`)
- `core::list<T>` - Linked list (see `include/irrList.h`)
- `core::map<K,V>` - Map (see `include/irrMap.h`)
- **IMPORTANT:** Do NOT use `std::` containers in core engine code

### Container Patterns
```cpp
// Dynamic array
core::array<video::IImage*> Images;

// String
core::stringc filename = "example.txt";  // c8 char string
core::stringw wfilename = L"example.txt"; // wchar_t string
```

## Import Organization

### Include Order
1. Project header (e.g., `"IrrCompileConfig.h"`)
2. Corresponding header (e.g., `"CSceneManager.h"`)
3. Interface headers (e.g., `"IVideoDriver.h"`, `"IFileSystem.h"`)
4. Other engine headers
5. System/platform headers (e.g., `<winuser.h>`, `<dinput.h>`)

Example from `source/Irrlicht/CSceneManager.cpp`:
```cpp
#include "IrrCompileConfig.h"
#include "CSceneManager.h"
#include "IVideoDriver.h"
#include "IFileSystem.h"
// ... more engine headers

#include "os.h"  // OS utilities last

#ifdef _IRR_COMPILE_WITH_XXX_LOADER_
#include "CXXXLoader.h"
#endif
```

### Conditional Compilation
- Use `#ifdef` blocks to include optional loaders/renderers
- Example from `source/Irrlicht/CSceneManager.cpp`:
```cpp
#ifdef _IRR_COMPILE_WITH_IRR_MESH_LOADER_
#include "CIrrMeshFileLoader.h"
#endif

#ifdef _IRR_COMPILE_WITH_BSP_LOADER_
#include "CBSPMeshFileLoader.h"
#endif
```

## Error Handling

### Logging
- Use `os::Printer::log()` function (see `source/Irrlicht/os.h`)
- Log levels: `ELL_INFORMATION`, `ELL_WARNING`, `ELL_ERROR`, `ELL_DEBUG`

```cpp
// From source/Irrlicht/CIrrDeviceWin32.cpp
os::Printer::log("Could not create DirectInput8 Object", ELL_WARNING);
```

### Debug Assertions
- Use `_IRR_DEBUG_BREAK_IF(condition)` for debug asserts
- Use `#ifdef _IRR_DEBUG_*` for conditional debug code

```cpp
// From include/IrrCompileConfig.h
_IRR_DEBUG_BREAK_IF(ReferenceCounter <= 0)

// Conditional debug logging
#ifdef _IRR_DEBUG_OBJ_LOADER_
    os::Printer::log("Loading object", objectName.c_str(), ELL_DEBUG);
#endif
```

### Null Checks
- Check return values explicitly
- Early returns on error conditions
```cpp
IAnimatedMesh *mesh = smgr->getMesh("../../media/sydney.md2");
if (!mesh)
{
    device->drop();
    return 1;
}
```

## Comments

### Doxygen Format
- Headers use Doxygen-formatted comments (`/** ... */`)
- Method descriptions with `\param`, `\return`, `\see`
- Class descriptions with brief and detailed documentation

Example from `include/IrrlichtDevice.h`:
```cpp
//! The Irrlicht device. You can create it with createDevice() or createDeviceEx().
/** This is the most important class of the Irrlicht Engine. You can
*  access everything in the engine if you have a pointer to an instance of
*  this class.  There should be only one instance of this class at any
*  time.
*/
class IrrlichtDevice : public virtual IReferenceCounted
{
    //! Runs the device.
    /** Also increments the virtual timer by calling
    *  ITimer::tick();. You can prevent this
    *  by calling ITimer::stop(); before and ITimer::start() after
    *  calling IrrlichtDevice::run(). Returns false if device wants
    *  to be deleted.
    *  \return Returns false if device wants to be deleted. */
    virtual bool run() = 0;
};
```

### Inline Comments
- Use `//` for implementation notes
- Use `// ...` for omitted code

```cpp
// copy old data
const s32 end = used < new_size ? used : new_size;

// data[i] = old_data[i];
allocator.construct(&data[i], old_data[i]);
```

## Platform-Specific Code

### Platform Isolation
- Platform-specific code in `CIrrDevice*.cpp` files
- OS subdirectories for platform details (e.g., `MacOSX/`, `Linux/`)
- Avoid direct OS API calls outside device layer

### Platform Detection
From `include/IrrCompileConfig.h`:
```cpp
#if defined(_WIN32) || defined(_WIN64)
#define _IRR_WINDOWS_
#define _IRR_WINDOWS_API_
#endif

#if defined(__APPLE__) || defined(MACOSX)
#define _IRR_OSX_PLATFORM_
#endif

#ifndef _IRR_WINDOWS_API_
#define _IRR_LINUX_PLATFORM_
#define _IRR_POSIX_API_
#endif
```

### Conditional Compilation with Preprocessor
```cpp
#ifdef _IRR_COMPILE_WITH_WINDOWS_DEVICE_
#include "CIrrDeviceWin32.h"
#endif

#ifdef _IRR_COMPILE_WITH_OPENGL_
IVideoDriver* createOpenGLDriver(...);
#endif
```

## Function Design

### Parameter Order
1. Output parameters (pointers/references that get modified)
2. Input parameters
3. Optional parameters with defaults

### Return Values
- Return pointers for objects needing reference counting
- Return `bool` for success/failure checks
- Return by value for simple types (integers, floats)

### Virtual Methods
- Use `virtual` keyword for overrideable methods
- Use `= 0` for pure virtual (abstract) methods
- Use `override` suffix comment to indicate overriding (older C++ style)

---

*Convention analysis: 2026-04-29*