# Coding Conventions

**Analysis Date:** 2026-04-09

## Naming Patterns

### Classes

- **Interfaces:** Prefix with `I` (e.g., `IVideoDriver`, `ISceneManager`, `IGUIEnvironment`)
- **Implementations:** Prefix with `C` (e.g., `COpenGLDriver`, `CSceneManager`, `CGUIButton`)
- **Base classes:** Prefix with `I` for abstract interfaces (e.g., `ISceneNode`)

### Functions

- **Methods:** `camelCase` (e.g., `createDevice`, `drop`, `grab`)
- **Getters/setters:** Direct naming (e.g., `getReferenceCount()`, `setDebugName()`)

### Variables

- **Members:** Often prefixed with lowercase letter (e.g., `ReferenceCounter`, `DebugName`)
- **Globals:** Rarely used; avoid when possible
- **Pointers:** Raw pointers with `*` (e.g., `ITexture* texture`)

### Types

- **Custom types in `irrTypes.h`:** Use prefixed names (`f32`, `s32`, `u32`, `c8`, etc.)

## Code Style

### Indentation

- **Standard:** 4 spaces (or tabs, configured per-platform)
- ** bracing:** Allman style (braces on new lines) common in headers

### Formatting

- **Line length:** Not strictly enforced; wrap at ~100 characters when practical
- **Spacing:** Operators spaced (e.g., `a + b`, not `a+b`)

### Naming Files

- **Headers:** `.h` extension (e.g., `IReferenceCounted.h`)
- **Sources:** `.cpp` extension (e.g., `os.cpp`)
- **Pattern:** Class name matches filename (e.g., `IReferenceCounted` in `IReferenceCounted.h`)

## Type System

### Custom Integer Types

```cpp
// Defined in include/irrTypes.h
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
```

### Custom Floating-Point Types

```cpp
typedef float f32;      // 32-bit float
typedef double f64;     // 64-bit float
```

### Usage

- **Always prefer engine types** over standard types for portability
- Use `f32` instead of `float`, `s32` instead of `int`, etc.

## Memory Management

### Reference Counting (grab/drop)

The engine uses **intrusive reference counting** via `IReferenceCounted`:

```cpp
// include/IReferenceCounted.h
class IReferenceCounted
{
public:
    void grab() const noexcept { ++ReferenceCounter; }
    bool drop() const noexcept
    {
        --ReferenceCounter;
        if (!ReferenceCounter) { delete this; return true; }
        return false;
    }
};
```

### Ownership Rules

- Objects created with `create...()` methods: caller owns, MUST call `drop()` when done
- Objects returned from `load...()` methods: engine owns, do NOT call `drop()`
- Example from `include/IReferenceCounted.h`:
  ```cpp
  ITexture* texture = driver->createTexture(...);
  texture->drop();  // Must drop
  ```

### Anti-Patterns

- **Do NOT use `std::shared_ptr` or `std::unique_ptr`** - use grab/drop
- **Do NOT use `new`/`delete` directly** on engine objects - use factory methods
- **Do NOT call `drop()` twice** - causes double-delete

## Custom Containers

The engine provides custom containers in `include/`:

### Core Containers

| Container | Header | Purpose |
|-----------|--------|---------|
| `core::array<T>` | `irrArray.h` | Dynamic array (like `std::vector`) |
| `core::string<T>` | `irrString.h` | String class |
| `core::list<T>` | `irrList.h` | Linked list |
| `core::map<K,V>` | `irrMap.h` | Associative container |

### Usage Example

```cpp
#include <irrArray.h>
#include <irrString.h>
#include <irrList.h>

irr::core::array<s32> numbers;
irr::core::string<c8> filename;
irr::core::list<IMesh*> meshList;
```

### Anti-Pattern

- **Do NOT use `std::vector`, `std::string`, `std::list`, `std::map`** - use irr equivalents
- The engine is NOT linked against STL in some builds

## Error Handling

### Return Values

- **Boolean returns:** `true` for success, `false` for failure
- **Nullptr:** Return `0` or `nullptr` on failure (common in factory methods)
- **Error codes:** Limited use; prefer boolean returns

### Pattern Example

```cpp
// From source/Irrlicht/CIrrDeviceStub.cpp
if (!createContext()) return false;
// ... operation ...
return true;
```

### No Exceptions

The engine is compiled with `-fno-exceptions` and `-fno-rtti`:

- **Do NOT use `try`/`catch`/`throw`**
- Use error codes and return values instead
- Third-party libs (libpng, jpeglib) use `setjmp`/`longjmp` for error recovery

## RTTI and Type System

### No RTTI

- **Do NOT use `dynamic_cast`** - the engine has no RTTI
- Use custom type checking where needed (e.g., `getType()` methods)
- Use `static_cast` where type is known

### Custom Type Checking

```cpp
// Common pattern in scene nodes
virtual ESCENE_NODE_TYPE getType() const = 0;
```

## Namespace Organization

### Main Namespaces

- `irr` - root namespace
  - `irr::core` - math, containers, strings
  - `irr::scene` - scene graph, nodes
  - `irr::video` - rendering, drivers
  - `irr::gui` - GUI system
  - `irr::io` - file system, archives

### Usage

```cpp
irr::core::string<c8> str;
irr::scene::ISceneManager* smgr;
irr::video::IVideoDriver* driver;
```

---

*Convention analysis: 2026-04-09*
