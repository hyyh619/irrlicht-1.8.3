# Codebase Structure

**Analysis Date:** 2026-04-09

## Directory Layout

```
irrlicht-1.8.3/
├── bin/                    # Binary outputs for various platforms
├── doc/                    # Documentation and licenses
├── examples/               # Tutorial examples and demo applications
├── include/                # Public engine headers (API) - stable interface
├── lib/                    # Compiled static/dynamic libraries
├── media/                  # Assets for examples (textures, models)
├── source/                 # Engine implementation
│   └── Irrlicht/           # Core engine source code
│       ├── MacOSX/         # MacOS-specific implementation
│       ├── aesGladman      # AES encryption library
│       ├── bzip2           # Compression library
│       ├── jpeglib         # JPEG loading support
│       ├── libpng          # PNG loading support
│       ├── lzma            # Compression library
│       └── zlib            # Compression library
└── tools/                  # Useful utilities (GUI Editor, Font Tool)
```

## Directory Purposes

**include/:**
- Purpose: Public API headers - stable interface for applications
- Contains: Interface classes (I*), data structures (S*), enumerations (E*)
- Key files:
  - `irrlicht.h` - Main header, includes everything, defines `createDevice()`
  - `IrrlichtDevice.h` - Device interface
  - `IVideoDriver.h` - Video driver interface
  - `ISceneManager.h` - Scene manager interface
  - `IGUIEnvironment.h` - GUI environment interface
  - `IFileSystem.h` - File system interface

**source/Irrlicht/:**
- Purpose: Implementation of engine components
- Contains: C* classes (implementations), platform-specific code
- Key categories:
  - CIrrDevice*.cpp - Platform device implementations
  - C*Driver.cpp - Video driver implementations
  - C*SceneNode.cpp - Scene node implementations
  - C*Loader.cpp - Mesh file loaders
  - CGUI*.cpp - GUI element implementations

## Key File Locations

**Entry Points:**
- `include/irrlicht.h`: Main header, includes all public API
- `source/Irrlicht/Irrlicht.cpp`: `createDevice()` and `createDeviceEx()` implementations

**Configuration:**
- `include/IrrCompileConfig.h`: Compilation flags (what features are compiled)
- `include/SIrrCreationParameters.h`: Device creation parameters

**Core Logic:**
- `source/Irrlicht/CSceneManager.cpp`: Scene graph implementation
- `source/Irrlicht/CIrrDeviceStub.cpp`: Base device implementation
- `source/Irrlicht/CSceneNode.cpp`: Base scene node implementation

**Testing:**
- No formal test framework - examples serve as test cases
- `examples/` directory contains working demonstrations

## Naming Conventions

**Files:**
- Interfaces: `I*.h` (e.g., `IVideoDriver.h`, `ISceneManager.h`)
- Structures: `S*.h` (e.g., `SColor.h`, `SMaterial.h`)
- Implementations: `C*.cpp/h` (e.g., `COpenGLDriver.cpp`, `CSceneNode.cpp`)

**Classes:**
- Interfaces: Prefix `I` (e.g., `IVideoDriver`, `ISceneNode`)
- Structures: Prefix `S` (e.g., `SColor`, `SMaterial`)
- Implementations: Prefix `C` (e.g., `COpenGLDriver`, `CBillboardSceneNode`)

**Directories:**
- No special convention - descriptive names (MacOSX, tools)

## Where to Add New Code

**New Scene Node Type:**
- Header: `include/I*SceneNode.h` (public interface)
- Implementation: `source/Irrlicht/C*SceneNode.cpp/h`

**New GUI Element:**
- Header: `include/IGUI*.h` (public interface)
- Implementation: `source/Irrlicht/CGUI*.cpp/h`

**New Video Driver:**
- Implementation: `source/Irrlicht/C*Driver.cpp/h`
- Register in device creation logic

**New Mesh Loader:**
- Implementation: `source/Irrlicht/C*MeshFileLoader.cpp/h`
- Register with scene manager

## Special Directories

**source/Irrlicht/MacOSX/:**
- Purpose: Platform-specific code for macOS (Cocoa integration)
- Generated: No
- Committed: Yes

**source/Irrlicht/jpeglib/, libpng/, zlib/:**
- Purpose: Third-party libraries for image loading
- Generated: No (external)
- Committed: Yes (bundled)

**examples/:**
- Purpose: Working demonstrations of API usage
- Contains: Tutorial-style examples (01.HelloWorld, etc.)

## Scene Graph Organization

**Root:** `scene::ISceneManager` manages the root scene node

**Node Types:**
- `ISceneNode` - Base for all scene nodes
- `IMeshSceneNode` - Static mesh display
- `IAnimatedMeshSceneNode` - Animated mesh display
- `ICameraSceneNode` - Viewpoint control
- `ILightSceneNode` - Light sources
- `IBillboardSceneNode` - 2D sprites in 3D space
- `ITerrainSceneNode` - Terrain rendering
- `IParticleSystemSceneNode` - Particle effects
- `IVolumeLightSceneNode` - Volumetric light shafts
- `IShadowVolumeSceneNode` - Shadow volume rendering

**Node Hierarchy:**
- Parent-child relationships via `ISceneNode::addChild()`
- Transforms propagate from parent to children
- Visibility culls children when parent is hidden

## GUI System Structure

**Root:** `gui::IGUIEnvironment` manages the GUI hierarchy

**Element Types:**
- `IGUIElement` - Base class for all GUI elements
- `IGUIWindow` - Window container
- `IGUIButton` - Clickable button
- `IGUIEditBox` - Text input
- `IGUIListBox` - List selection
- `IGUIComboBox` - Dropdown selection
- `IGUIScrollBar` - Scroll control
- `IGUISlider` - Value slider
- `IGUITabControl` - Tabbed interface
- `IGUIContextMenu` - Right-click menu
- `IGUIImage` - Image display
- `IGUIStaticText` - Text display
- `IGUIFont` - Font rendering

**Element Hierarchy:**
- Parent-child via `IGUIElement::addChild()`
- Events bubble up to parent elements
- Drawing order: children drawn after parents

**Rendering:**
- GUI rendered via `IGUIEnvironment::drawAll()`
- Each element draws itself using video driver
- Text rendered via `IGUIFont` interface

---

*Structure analysis: 2026-04-09*
