# Codebase Concerns

**Analysis Date:** 2026-04-09

## Technical Debt

### Custom Container Usage
- **Issue:** Engine uses custom containers (`irr::core::array`, `irr::core::stringc`, `irr::core::list`, `irr::core::map`) instead of STL equivalents
- **Files:** Throughout `source/Irrlicht/` and `include/irrlicht.h`
- **Impact:** Developers unfamiliar with Irrlicht may accidentally use `std::vector` or `std::string`, breaking memory management patterns
- **Fix approach:** Document this convention prominently; could consider migration to STL with proper wrapper for reference counting

### No Exceptions / No RTTI
- **Issue:** Engine built with `-fno-exceptions` and `-fno-rtti`
- **Files:** `include/IrrCompileConfig.h`
- **Impact:** Cannot use `try`/`catch`/`throw`, cannot use `dynamic_cast`
- **Fix approach:** Use custom type system and error code returns; avoid C++ features requiring RTTI

### Intrusive Reference Counting
- **Issue:** Memory management via `grab()`/`drop()` pattern rather than smart pointers
- **Files:** `include/IReferenceCounted.h`
- **Impact:** Easy to cause memory leaks if `grab()` not called when retaining objects
- **Fix approach:** Requires careful adherence to ownership conventions

### Quaternion Workarounds
- **Issue:** Quaternion-to-matrix conversion inverts rotations; code uses workarounds like `getMatrix_transposed()` for downward compatibility
- **Files:** `source/Irrlicht/CSkinnedMesh.cpp`, `source/Irrlicht/COgreMeshFileLoader.cpp`, `source/Irrlicht/CXMeshFileLoader.cpp`, `source/Irrlicht/CMS3DMeshFileLoader.cpp`
- **Impact:** Complex code paths; can be enabled for testing with `IRR_TEST_BROKEN_QUATERNION_USE`
- **Fix approach:** Clean up after ensuring no user code depends on old behavior

---

## Known Bugs and Limitations

### Software Driver Issues (OSX)
- **Issue:** Software driver doesn't work properly under OSX 10.9
- **Files:** `changes.txt` line 34
- **Trigger:** Running software renderer on OSX 10.9+
- **Workaround:** Use OpenGL or Direct3D drivers on OSX

### CGUIEditBox Crash
- **Issue:** Can crash with wordwrap enabled when spaces entered beyond border followed by cursor key press
- **Files:** `source/Irrlicht/CGUIEditBox.cpp` (multiple fixes in `changes.txt`)
- **Trigger:** Specific sequence of user input
- **Workaround:** Avoid combination of wordwrap + spaces at border + cursor movement

### 3DS Mesh Loader
- **Issue:** Multiple unimplemented features in 3DS format loader
- **Files:** `source/Irrlicht/C3DSMeshFileLoader.cpp` (lines 651, 655, 1035)
- **Impact:** Some 3DS model features not loaded correctly

### XML Comment Handling
- **Issue:** XML reader doesn't properly handle comments inside elements
- **Files:** `source/Irrlicht/CIrrMeshFileLoader.cpp` (lines 486, 499, 513), `source/Irrlicht/CColladaFileLoader.cpp` (lines 2585, 2599, 2632)

---

## Platform-Specific Issues

### MacOS XCode Project
- **Issue:** XCode project builds static library only, not the full demo applications
- **Files:** `source/Irrlicht/MacOSX/`
- **Impact:** Limited out-of-box experience for Mac developers
- **Fix approach:** Add XCode scheme for building demo applications

### Linux File System
- **Issue:** Path handling needs normalization; drive letter support incomplete
- **Files:** `source/Irrlicht/CFileSystem.cpp` (lines 329, 871, 891)
- **Impact:** Cross-platform path handling issues

### Windows 8 Cursor
- **Issue:** Cursor visibility update problems on Windows 8
- **Files:** `changes.txt` (1.8.1 notes)

---

## Unfinished Features (TODO)

### Font/Texture Cache
- **Issue:** `IGUIEnvironment::removeFont` does not remove texture from cache
- **Files:** `include/IGUIEnvironment.h`, `changes.txt` line 428
- **Impact:** Potential memory leak when removing fonts

### Quake3 Explorer
- **Issue:** Texture handling incomplete; dynamic loading for other OSes not implemented
- **Files:** `examples/21.Quake3Explorer/main.cpp` (line 1056), `examples/21.Quake3Explorer/q3factory.cpp` (lines 571, 769)

### B3D Loader
- **Issue:** Color key texture creation and cube map support not implemented
- **Files:** `source/Irrlicht/CB3DMeshFileLoader.cpp` (lines 1003, 1007)

### GUI Elements
- **Issue:** Multiple GUI improvements pending (text clipping, scrolling, alignment)
- **Files:** `source/Irrlicht/CGUITabControl.cpp` (lines 665, 685), `source/Irrlicht/CGUIEditBox.cpp` (lines 1434, 1447), `source/Irrlicht/CGUIStaticText.cpp` (line 562)

### Collada Writer
- **Issue:** Second UV coordinates ignored; tangents not supported
- **Files:** `source/Irrlicht/CColladaMeshWriter.cpp` (lines 5, 964, 1493)

### Scene Manager Debug
- **Issue:** Performance parameters not updated in release builds
- **Files:** `changes.txt` (1.8.1 notes)
- **Impact:** Cannot debug scene rendering performance without `_IRR_SCENEMANAGER_DEBUG`

---

## Performance Considerations

### Matrix4 Operations
- **Issue:** Quaternion operations marked as needing speed optimization
- **Files:** `include/quaternion.h` (lines 242, 256, 268, 280)
- **Impact:** Slower rotation calculations

### Scene Node Culling
- **Issue:** Point light culling not implemented
- **Files:** `source/Irrlicht/CSceneManager.cpp` (line 1263)

### XML Writing
- **Issue:** `reserve()` call slows down XML writing
- **Files:** `source/Irrlicht/CXMLWriter.cpp` (line 207)

### Array Reallocation
- **Issue:** Previously had excessive reallocation; now controllable but defaults could be optimized
- **Files:** `changes.txt` (1.8 notes about reallocate function)

---

## Security Considerations

### ZIP Password Support
- **Issue:** Previously had 64-bit password handling bug
- **Files:** `changes.txt` (line 154 - fixed in 1.8)
- **Current status:** Fixed in version 1.8

### No User Authentication
- **Issue:** Engine has no built-in user authentication or permission system
- **Impact:** Applications must implement their own security
- **Recommendation:** Document security requirements for application developers

---

## Code Quality Observations

### Large Embedded Libraries
- **Issue:** Contains significant embedded code from zlib, libpng, libjpeg, and bzip2
- **Files:** `source/Irrlicht/zlib/`, `source/Irrlicht/libpng/`, `source/Irrlicht/jpeglib/`, `source/Irrlicht/bzip2/`
- **Impact:** Large codebase; security patches from upstream need manual merging
- **Recommendation:** Consider using system libraries or submodule approach

### Missing Doxygen Updates
- **Issue:** Doxygen config has TODO list generation enabled but likely outdated
- **Files:** `source/Irrlicht/Doxyfile` (lines 613, 617)

### API Documentation
- **Issue:** Some functions have minimal or missing documentation
- **Impact:** Developers must read source to understand some APIs

---

## Missing Documentation

### Upgrade Path
- **Issue:** Last major release (1.8.3) in 2015; limited documentation for modern development
- **Files:** `doc/upgrade-guide.txt`
- **Impact:** New users may struggle with outdated tutorials

### Platform-Specific Setup
- **Issue:** Limited instructions for modern development environments (VS2015+, modern Linux distros)
- **Impact:** Users report build issues with newer compilers

### Shader Development
- **Issue:** Advanced shader features (Cg, HLSL, GLSL) lack comprehensive tutorials
- **Files:** `source/Irrlicht/CD3D9CgMaterialRenderer.cpp`, `source/Irrlicht/COpenGLCgMaterialRenderer.cpp`

---

## Test Coverage

### No Built-in Test Framework
- **Issue:** Engine lacks comprehensive unit tests
- **Impact:** Changes may introduce regressions without detection

---

## Deprecated / Historical

### Old Compiler Support
- **Issue:** Readme lists support for older compilers (GCC 4.x, VS 2008-2012)
- **Files:** `readme.txt` (lines 77-79)
- **Current status:** Likely outdated

### D3D8 Support
- **Issue:** DirectX 8 support requires older DirectX SDK (prior to May 2006)
- **Files:** `readme.txt` (line 88)
- **Impact:** Effectively deprecated

---

*Concerns audit: 2026-04-09*
