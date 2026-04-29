<!-- refreshed: 2026-04-29 -->
# Architecture

**Analysis Date:** 2026-04-29

## System Overview

```text
┌─────────────────────────────────────────────────────────────┐
│                   Application Layer                        │
│              (examples/, user code)                       │
├─────────────────────────────────────────────────────────────┤
│                      IrrlichtDevice                         │
│            `source/Irrlicht/Irrlicht.cpp`                  │
├─────────────────────────────────────────────────────────┬─┴───────────────┤
│   Video Driver    │  Scene Manager  │  GUI Env  │ FileSys │
│  IVideoDriver    │ ISceneManager   │IGUIEnv   │IFileSys│
│ `IVideoDriver.h` │`ISceneManager.h`│`IGUIEnv.h`│`IFileS.│
├─────────────────┴─────────────────┴─────────────┴─────────┤
│              Platform Abstraction Layer                    │
│   CIrrDeviceWin32  │  CIrrDeviceLinux  │  CIrrDeviceSDL │
│ `CIrrDeviceWin32` │ `CIrrDeviceLinux`│ `CIrrDeviceS │
└─────────────────────────────────────────────────────────┘
```

## Component Responsibilities

| Component | Responsibility | File |
|-----------|----------------|------|
| IrrlichtDevice | Central engine hub, owns all managers, handles window/input | `source/Irrlicht/Irrlicht.cpp:66` |
| IVideoDriver | Rendering, textures, materials, meshes | `include/IVideoDriver.h` |
| ISceneManager | Scene graph, nodes, animators, collision | `include/ISceneManager.h` |
| IGUIEnvironment | GUI elements, events, rendering | `include/IGUIEnvironment.h` |
| IFileSystem | File I/O, archives, zip reading | `include/IFileSystem.h` |
| IEventReceiver | Input event handling | `include/IEventReceiver.h` |

## Pattern Overview

**Overall:** Component-Based Service Locator with Reference Counting

**Key Characteristics:**
- Engine core (IrrlichtDevice) acts as service locator for all subsystems
- Public API uses `I` prefix for interfaces, `C` prefix for implementations
- Reference counting via `IReferenceCounted` (grab()/drop()) for memory management
- Platform-specific code isolated in CIrrDevice*.cpp files
- Heavy use of irr::core containers (core::array, core::string)

## Layers

**Application Layer:**
- Purpose: User code and examples
- Location: `examples/`, user applications
- Contains: Tutorial examples, demo applications
- Depends on: Irrlicht public API

**Public API Layer:**
- Purpose: Stable interface definitions for applications
- Location: `include/*.h`
- Contains: I-prefixed interfaces (IVideoDriver, ISceneManager, etc.)
- Depends on: None (pure interfaces)

**Engine Implementation Layer:**
- Purpose: Core engine functionality
- Location: `source/Irrlicht/`
- Contains: C-prefixed implementations
- Depends on: Public API headers

**Platform Abstraction Layer:**
- Purpose: OS/windowing system abstraction
- Location: `source/Irrlicht/CIrrDevice*.cpp`
- Contains: CIrrDeviceWin32, CIrrDeviceLinux, CIrrDeviceSDL, etc.
- Depends on: Platform APIs (Windows API, X11, SDL)

**Rendering Driver Layer:**
- Purpose: Graphics API implementation
- Location: `source/Irrlicht/C*Driver.cpp`
- Contains: COpenGLDriver, CD3D9Driver, CSoftwareDriver2
- Depends on: Graphics APIs (OpenGL, Direct3D)

**Embedded Libraries Layer:**
- Purpose: Compression, image decode
- Location: `source/Irrlicht/{zlib,libpng,jpeglib,bzip2,lzma,aesGladman}/`
- Contains: Third-party libraries
- Used by: File system, image loaders

## Data Flow

### Primary Rendering Path

1. **Application creates device** (`source/Irrlicht/Irrlicht.cpp:48`)
   - Calls `createDevice()` or `createDeviceEx()`
   - Creates platform-specific CIrrDevice*

2. **Device runs main loop** (`CIrrDeviceWin32.cpp`)
   - Handles window messages
   - Polls input devices

3. **Scene rendering** (`CSceneManager.cpp`)
   - `drawAll()` traverses scene graph
   - Registers nodes for render passes
   - Calls video driver to render

4. **Video driver renders** (`COpenGLDriver.cpp` / `CD3D9Driver.cpp`)
   - `beginScene()` / `endScene()`
   - `drawMeshBuffer()` for each geometry
   - Manages textures and materials

### Event Flow

1. **OS generates input** (mouse, keyboard)
2. **Device captures** (`CIrrDeviceWin32.cpp:msg`)
3. **Device posts event** (`postEventFromUser()`)
4. **Event propagates** through receivers:
   - User event receiver first
   - GUI environment
   - Scene manager (active camera)
   - Application handles viaOnEvent()

### File Loading Path

1. **Application requests mesh** (`sceneManager->getMesh()`)
2. **Scene manager finds loader** (IMeshLoader registry)
3. **Loader reads file** (C*MeshFileLoader.cpp)
4. **Creates animated mesh** (IAnimatedMesh)
5. **Creates scene node** (addMeshToScene())

## Key Abstractions

**IReferenceCounted:**
- Purpose: Reference-counted base for all engine objects
- Examples: All IVideoDriver, ISceneManager, ITexture
- Pattern: grab() increments, drop() decrements, auto-delete at 0

**ISceneNode:**
- Purpose: Base for all scene graph objects
- Examples: CMeshSceneNode, CCameraSceneNode, CLightSceneNode
- Pattern: Parent-child hierarchy, render() called by manager

**IVideoDriver:**
- Purpose: Graphics API abstraction
- Examples: COpenGLDriver, CD3D9Driver, CSoftwareDriver2
- Pattern: Single driver per device, manages all rendering

**CIrrDevice* (Platform):**
- Purpose: Platform-specific window/input
- Examples: CIrrDeviceWin32, CIrrDeviceLinux, CIrrDeviceSDL
- Pattern: Create one per IrrlichtDevice instance

**ISceneManager:**
- Purpose: Scene graph orchestration
- Example: `CSceneManager`
- Pattern: Owns scene root, traverses for rendering

**IGUIEnvironment:**
- Purpose: GUI system management
- Example: `CGUIEnvironment`
- Pattern: Owns root GUI element, draws after scene

## Entry Points

**createDevice():**
- Location: `source/Irrlicht/Irrlicht.cpp:48`
- Triggers: Application call
- Responsibilities: Creates IrrlichtDevice, initializes graphics driver

**IrrlichtDevice::run():**
- Location: `source/Irrlicht/CIrrDeviceWin32.cpp` (platform-specific)
- Triggers: Application main loop
- Responsibilities: Process window messages, input polling

**ISceneManager::drawAll():**
- Location: `source/Irrlicht/CSceneManager.cpp`
- Triggers: Each frame from application
- Responsibilities: Render entire scene, GUI on top

## Architectural Constraints

- **Threading:** Single-threaded rendering; OpenGL/D3D manage their own threads
- **Global state:** `irr::core::IdentityMatrix`, `irr::video::IdentityMaterial` singletons (`Irrlicht.cpp:118-125`)
- **Circular imports:** None detected - clean interface/implementation separation
- **Memory:** No smart pointers; manual grab()/drop() required
- **No std:::** Uses irr::core containers exclusively

## Anti-Patterns

### Using std:: Containers Internally

**What happens:** Code uses `std::vector`, `std::string` in engine internals
**Why it's wrong:** Inconsistent with codebase patterns, potential ABI issues
**Do this instead:** Use `irr::core::array`, `irr::core::stringc` in `source/Irrlicht/`

### Direct OS API Outside Device Layer

**What happens:** Windows API calls outside CIrrDevice*.cpp
**Why it's wrong:** Breaks cross-platform abstraction
**Do this instead:** Keep OS-specific code in platform device implementations only

### Forgetting to drop() Objects

**What happens:** Memory leaks when dropping reference counting
**Why it's wrong:** Engine relies on reference counting for cleanup
**Do this instead:** Always pair createX() with object->drop()

## Error Handling

**Strategy:** Return codes and null checks

**Patterns:**
- Return 0/null on failure (e.g., `getMesh()` returns 0 if not found)
- Log warnings via ILogger
- Device continues but reports errors

## Cross-Cutting Concerns

**Logging:** ILogger interface, log to file/console
**Validation:** Parameters checked in public API methods
**Authentication:** N/A (graphics engine)

---

*Architecture analysis: 2026-04-29*