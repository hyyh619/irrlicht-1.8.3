#ifndef __IRR_D3D11_DEBUG_H_INCLUDED__
#define __IRR_D3D11_DEBUG_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

//==============================================================================
// D3D11 Debug Macros
//==============================================================================
// This file consolidates all debug/development macros for the D3D11 driver.
// To enable any macro, define it before including this header or in IrrCompileConfig.h
// To disable a macro, define NO_IRR_<MACRO_NAME> (e.g., NO_IRR_TEXTURE_DUMP)
//==============================================================================

//! _IRR_TEXTURE_DUMP: Dumps IImage content to disk when creating textures in CD3D11Texture::createTexture
// Default: disabled
// Usage: Define _IRR_TEXTURE_DUMP to save textures as "dump_<name>" files
//#define _IRR_TEXTURE_DUMP

//! _IRR_MATERIAL_PRINT: Prints material info to log whenever setMaterial is called
// Default: disabled
// Usage: Define _IRR_MATERIAL_PRINT to log material properties
#define _IRR_MATERIAL_PRINT

//! _IRR_DUMP_DRAW_CALLS: Enables draw call statistics logging
// Default: disabled
// Usage: Define _IRR_DUMP_DRAW_CALLS to track draw call counts
#ifndef _IRR_DUMP_DRAW_CALLS_PRINT
#define _IRR_DUMP_DRAW_CALLS_PRINT 1   // 1=print to log, 0=silent
#endif
#ifndef _IRR_DUMP_DRAW_CALLS_FILE
#define _IRR_DUMP_DRAW_CALLS_FILE 0    // 1=write to file, 0=no file output
#endif

//! _IRR_D3D11_OBJECT_TRACKING: Enables D3D11 object reference tracking
// Tracks Create/Release/AddRef calls for D3D11 objects to detect leaks
// Default: disabled
// Usage: Define _IRR_D3D11_OBJECT_TRACKING to enable object tracking
// #define _IRR_D3D11_OBJECT_TRACKING

//==============================================================================
// Disable macros via NO_IRR_<MACRO_NAME> prefix (override definitions above)
//==============================================================================
#ifdef NO_IRR_TEXTURE_DUMP
#undef _IRR_TEXTURE_DUMP
#endif

#ifdef NO_IRR_MATERIAL_PRINT
#undef _IRR_MATERIAL_PRINT
#endif

#ifdef NO_IRR_D3D11_OBJECT_TRACKING
#undef _IRR_D3D11_OBJECT_TRACKING
#endif

//==============================================================================
// Internal helper macros (used by the macros above)
//==============================================================================

#ifdef _IRR_D3D11_OBJECT_TRACKING
#define IRR_D3D11_TRACKING_ENABLED 1
#else
#define IRR_D3D11_TRACKING_ENABLED 0
#endif

#endif // _IRR_COMPILE_WITH_DIRECT3D_11_
#endif // __IRR_D3D11_DEBUG_H_INCLUDED__