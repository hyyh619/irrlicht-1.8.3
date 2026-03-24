# PROJECT KNOWLEDGE BASE

**Generated:** 2026-03-24
**Branch:** master

## OVERVIEW
High-performance, real-time 3D engine written in C++. Features a cross-platform API for 2D and 3D graphics, supporting OpenGL, DirectX, and software rendering.

## STRUCTURE
```
irrlicht-1.8.3/
├── bin/            # Binary outputs for various platforms
├── doc/            # Documentation and licenses
├── examples/       # Tutorial examples and demo
├── include/        # Public engine headers (API)
├── lib/            # Compiled static/dynamic libraries
├── media/          # Assets for examples (textures, models)
├── source/         # Engine implementation
│   └── Irrlicht/   # Core engine source code
└── tools/          # Useful utilities (GUI Editor, Font Tool)
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| API Usage | `include/` | Start with `irrlicht.h` |
| Core Implementation | `source/Irrlicht/` | Rendering drivers, scene graph |
| Platform Logic | `source/Irrlicht/` | `CIrrDevice*.cpp` files |
| MacOS Specific | `source/Irrlicht/MacOSX/` | Cocoa/OpenGL integration |
| Examples | `examples/` | Structured by feature/tutorial |

## CODE MAP
| Symbol | Type | Location | Role |
|--------|------|----------|------|
| `createDevice` | Function | `include/irrlicht.h` | Main engine entry point |
| `IrrlichtDevice` | Class | `include/IrrlichtDevice.h` | Primary engine interface |
| `IVideoDriver` | Interface | `include/IVideoDriver.h` | Abstraction for 2D/3D rendering |
| `ISceneManager` | Interface | `include/ISceneManager.h` | Manages scene graph and nodes |
| `IGUIEnvironment` | Interface | `include/IGUIEnvironment.h` | Manages GUI elements |
| `IReferenceCounted`| Base Class| `include/IReferenceCounted.h`| Base for memory management |

## CONVENTIONS
- **Namespaces**: Core types in `irr::core`, scene graph in `irr::scene`, video in `irr::video`, GUI in `irr::gui`, IO in `irr::io`.
- **Memory Management**: Uses intrusive reference counting. Call `grab()` to keep and `drop()` to release. Objects created with `create...` or `add...` usually return a pointer you own (must `drop`).
- **Data Types**: Prefers `f32`, `s32`, `u32` over standard types for portability.
- **Containers**: Uses custom containers: `core::array`, `core::stringc`, `core::stringw`, `core::list`, `core::map`.

## ANTI-PATTERNS (THIS PROJECT)
- **STL Usage**: Avoid `std::vector`, `std::string` etc. Use `irr::core` equivalents.
- **Exceptions**: Engine is built with `-fno-exceptions`. Do not use `try`/`catch`/`throw`.
- **RTTI**: Built with `-fno-rtti`. Avoid `dynamic_cast`. Uses custom type system or `static_cast` where safe.

## COMMANDS
```bash
# Build engine (Linux/Unix)
cd source/Irrlicht && make

# Build examples (MacOSX)
cd examples && make all_macos
```

## NOTES
- Header files in `include/` often contain significant inline implementations for performance.
- Platform abstraction is handled via `CIrrDeviceStub` and its OS-specific subclasses.
