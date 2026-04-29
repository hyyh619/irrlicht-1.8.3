# External Integrations

**Analysis Date:** 2026-04-29

## Overview

The Irrlicht Engine is a standalone 3D graphics engine. It does not integrate with external cloud services, databases, or authentication providers. Instead, it focuses on rendering APIs and file format support.

---

## File Format Integrations

### Supported Mesh Formats
| Format | Extension | Description |
|--------|-----------|-------------|
| DirectX | `.x` | DirectX mesh files |
| Maya | `.mb`, `.ma` | Maya ASCII format |
| 3DS Max | `.3ds` | 3D Studio Max export |
| Quake 3 | `.bsp` | Quake 3 Arena maps |
| Lightwave | `.lwo` | Lightwave objects |
| OBJ | `.obj` | Wavefront OBJ |
| MS3D | `.ms3d` | Milkshape 3D |
| B3D | `.b3d` | BlitzBasic 3D |
| XGL | `.xgl`, `.xml` | XML-based format |

### Supported Texture Formats
| Format | Extensions | Library |
|--------|------------|----------|
| PNG | `.png` | libpng |
| JPEG | `.jpg`, `.jpeg` | jpeglib |
| TGA | `.tga`, `.bmp`, `.dib` | Built-in |
| DDS | `.dds` | Built-in (DirectX format) |
| PCX | `.pcx` | Built-in |
| PSD | `.psd` | Built-in ( Photoshop) |
| GIF | `.gif` | Built-in |
| TIFF | `.tiff`, `.tif` | Built-in |

---

## Rendering API Integrations

### Supported Drivers
| Platform | API | Device Class |
|------------|-----|---------------|
| Windows | Direct3D 9 | `CIrrDeviceWin32` |
| Windows | OpenGL | `CIrrDeviceWin32` |
| Linux | OpenGL | `CIrrDeviceLinux` |
| macOS | OpenGL | `CIrrDeviceMacOSX` |
| Cross-platform | SDL + OpenGL | `CIrrDeviceSDL` |

---

## Compression Libraries (Bundled)

The engine includes embedded copies of:
- **zlib** - ZIP compression
- **libpng** - PNG image format
- **jpeglib** - JPEG image format
- **lzma** - LZMA compression (7-zip)
- **bzip2** - bzip2 compression

---

## Audio

The engine does not include a built-in audio subsystem. External audio libraries can be integrated via the `ISoundDriver` interface if needed.

---

## Network

No built-in networking. Network features can be implemented by users using platform-specific APIs or third-party networking libraries.

---

## Summary

Irrlicht Engine is a self-contained rendering engine with no external cloud or SaaS integrations. It provides file I/O for common 3D model and texture formats, and can target multiple rendering APIs (Direct3D, OpenGL) across different platforms.