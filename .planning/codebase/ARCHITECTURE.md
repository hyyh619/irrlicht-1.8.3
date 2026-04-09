# Architecture

**Analysis Date:** 2026-04-09

## Pattern Overview

**Overall:** Layered component-based architecture with interface-driven design

**Key Characteristics:**
- Central `IrrlichtDevice` acts as the root object and service locator
- Interface-based abstractions for all major subsystems (video, scene, GUI, IO)
- Reference-counted memory management via `IReferenceCounted`
- Multiple rendering driver backends (OpenGL, Direct3D 8/9, Software)
- Cross-platform device abstraction (Win32, Linux, MacOSX, SDL, Framebuffer)

## Layers

**Device Layer:**
- Purpose: Root object providing access to all engine subsystems
- Location: `include/IrrlichtDevice.h`, `source/Irrlicht/CIrrDevice*.cpp`
- Contains: `IrrlichtDevice` interface with platform-specific implementations
- Depends on: Nothing (other than OS APIs)
- Used by: Application code via `createDevice()`

**Video Driver Layer:**
- Purpose: All 2D and 3D rendering, texture management
- Location: `include/IVideoDriver.h`, `source/Irrlicht/C*Driver.cpp`
- Contains: `IVideoDriver` interface, concrete drivers (COpenGLDriver, CD3D9Driver, CSoftwareDriver, CNullDriver)
- Depends on: Device layer
- Used by: Scene manager, GUI environment, application code

**Scene Manager Layer:**
- Purpose: Scene graph management, mesh loading, scene node creation
- Location: `include/ISceneManager.h`, `source/Irrlicht/CSceneManager.cpp`
- Contains: `ISceneManager` interface, scene nodes, animators, mesh loaders
- Depends on: Video driver, GUI environment
- Used by: Application code

**GUI Layer:**
- Purpose: Graphical user interface management
- Location: `include/IGUIEnvironment.h`, `source/Irrlicht/CGUIEnvironment.cpp`
- Contains: `IGUIEnvironment` interface, GUI elements (buttons, windows, etc.)
- Depends on: Video driver
- Used by: Application code

**IO Layer:**
- Purpose: File system, archive access, XML handling
- Location: `include/IFileSystem.h`, `source/Irrlicht/CFileSystem.cpp`
- Contains: `IFileSystem` interface, archive readers (ZIP, TAR, WAD)
- Used by: Scene manager, GUI environment, application code

## Data Flow

**Main Render Loop:**

1. Application calls `device->run()`
2. Device pumps window messages
3. Application calls `driver->beginScene()`
4. Application calls `sceneManager->drawAll()`
5. Scene manager renders scene nodes in pass order (camera, light, skybox, solid, transparent, shadow)
6. Application calls `guiEnvironment->drawAll()`
7. Application calls `driver->endScene()`

**Scene Graph Traversal:**
- Root scene node contains children
- Each node's `render()` calls `registerNodeForRendering()` with a render pass
- Scene manager iterates registered nodes in proper order
- Each node draws itself via video driver

**State Management:**
- Video driver maintains global transform matrices (world, view, projection)
- Current material set on driver before drawing
- Scene nodes manage their own position, rotation, scale via `ISceneNode`
- Global override material applies to all rendered objects

## Key Abstractions

**IrrlichtDevice:**
- Purpose: Central engine interface and service locator
- Examples: `include/IrrlichtDevice.h`
- Pattern: Factory + service locator

**IVideoDriver:**
- Purpose: Rendering abstraction
- Examples: `include/IVideoDriver.h`
- Pattern: Interface with multiple implementations

**ISceneManager:**
- Purpose: Scene graph and resource management
- Examples: `include/ISceneManager.h`
- Pattern: Factory + manager

**IGUIEnvironment:**
- Purpose: GUI element factory and manager
- Examples: `include/IGUIEnvironment.h`
- Pattern: Composite + factory

**IReferenceCounted:**
- Purpose: Base class for reference-counted memory management
- Examples: `include/IReferenceCounted.h`
- Pattern: Intrusive reference counting

## Entry Points

**createDevice:**
- Location: `include/irrlicht.h`, `source/Irrlicht/Irrlicht.cpp`
- Triggers: Application startup
- Responsibilities: Create platform-specific device, initialize all subsystems

**IrrlichtDevice (interface):**
- Location: `include/IrrlichtDevice.h`
- Triggers: After device creation
- Responsibilities: Provides access to video driver, scene manager, GUI, file system, timer

## Error Handling

**Strategy:** Return codes and null checks

**Patterns:**
- Methods return `bool` for success/failure
- `0`/`nullptr` returned on error conditions
- Debug assertions via `_IRR_DEBUG_BREAK_IF`
- No exceptions (engine built with `-fno-exceptions`)

## Cross-Cutting Concerns

**Logging:** `ILogger` interface for debug output
**Validation:** `IAttributeExchangingObject` for serialization
**Authentication:** Not applicable (local graphics engine)
**Timing:** `ITimer` interface for frame timing

---

*Architecture analysis: 2026-04-09*
