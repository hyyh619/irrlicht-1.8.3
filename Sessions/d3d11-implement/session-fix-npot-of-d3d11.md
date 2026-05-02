# D3D11驱动NPOT纹理支持检查

**Session ID:** ses_219833717ffe7z6uEZhMK5yTAs
**Created:** 2026/5/2 10:20:30
**Updated:** 2026/5/2 10:34:19

---

## User

bool CD3D11Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const没有检查EVDF_TEXTURE_NPOT，请根据D3D11的Spec查看是否支持NPOT

---

## Assistant (Build · MiniMax-M2.7 · 10.1s)

_Thinking:_

用户想要我检查 `CD3D11Driver::queryFeature` 函数，看看它是否支持 EVDF_TEXTURE_NPOT（non-power-of-two textures）特性。他们想根据 D3D11 的规范来确认这一点。

首先，我需要查看这个函数的实现，以及了解 D3D11 是否支持 NPOT 纹理。


**Tool: grep**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht",
  "pattern": "queryFeature"
}
```

**Output:**
```
Found 95 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp:
  Line 244:                 !Driver->queryFeature(EVDF_TEXTURE_NPOT),

  Line 245:                 !Driver->queryFeature(EVDF_TEXTURE_NSQUARE),


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp:
  Line 331:         bool CD3D11Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const

  Line 1113:             if (queryFeature(video::EVDF_PIXEL_SHADER_1_1) && queryFeature(video::EVDF_VERTEX_SHADER_1_1))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h:
  Line 76:             virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSoftwareDriver.cpp:
  Line 161:         bool CSoftwareDriver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSoftwareDriver.h:
  Line 28:             virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSoftwareDriver2.cpp:
  Line 676:         bool CBurningVideoDriver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSoftwareDriver2.h:
  Line 31:             virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSphereSceneNode.cpp:
  Line 86:             if (!SceneManager->getVideoDriver()->queryFeature(video::EVDF_STENCIL_BUFFER))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CQuake3ShaderSceneNode.cpp:
  Line 528:             if (!SceneManager->getVideoDriver()->queryFeature(video::EVDF_STENCIL_BUFFER))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COctreeSceneNode.cpp:
  Line 313:             if (!SceneManager->getVideoDriver()->queryFeature(video::EVDF_STENCIL_BUFFER))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLDriver.cpp:
  Line 680:             if (queryFeature(EVDF_ARB_GLSL))

  Line 1339:             if (!queryFeature(EVDF_OCCLUSION_QUERY))

  Line 1435:                     if (queryFeature(EVDF_OCCLUSION_QUERY))

  Line 3118:             if (queryFeature(EVDF_BLEND_OPERATIONS) &&

  Line 3236:             if (queryFeature(EVDF_POLYGON_OFFSET) && (resetAllRenderStates ||

  Line 4155:             if (queryFeature(EVDF_FRAMEBUFFER_OBJECT))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLDriver.h:
  Line 144:             virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const

  Line 146:                 return FeatureEnabled[feature] && COpenGLExtensionHandler::queryFeature(feature);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLExtensionHandler.cpp:
  Line 628:             if (queryFeature(EVDF_GEOMETRY_SHADER))

  Line 749:         bool COpenGLExtensionHandler::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLExtensionHandler.h:
  Line 927:             bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;

  Line 2474:             if (queryFeature(EVDF_GEOMETRY_SHADER))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLMaterialRenderer.h:
  Line 179:                     if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 207:                 if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 445:                     if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 513:                 if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 549:                     if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 569:                 if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 631:                     if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 656:                 if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 664:                 if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 703:                     if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 735:                 if (Driver->queryFeature(EVDF_MULTITEXTURE))

  Line 743:                 if (Driver->queryFeature(EVDF_MULTITEXTURE))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLNormalMapRenderer.cpp:
  Line 170:             if (!driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1) ||

  Line 171:                 !driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))

  Line 224:             if (Driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1) &&

  Line 225:                 Driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLParallaxMapRenderer.cpp:
  Line 204:             if (!driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1) ||

  Line 205:                 !driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))

  Line 270:             if (Driver->queryFeature(video::EVDF_ARB_FRAGMENT_PROGRAM_1) &&

  Line 271:                 Driver->queryFeature(video::EVDF_ARB_VERTEX_PROGRAM_1))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLSLMaterialRenderer.cpp:
  Line 61:             if (!Driver->queryFeature(EVDF_ARB_GLSL))

  Line 150:             if (geometryShaderProgram && Driver->queryFeature(EVDF_GEOMETRY_SHADER))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLTexture.cpp:
  Line 320:             TextureSize = ImageSize.getOptimalSize(!Driver->queryFeature(EVDF_TEXTURE_NPOT));

  Line 356:                 if (HasMipMaps && !mipmapData && Driver->queryFeature(EVDF_MIP_MAP_AUTO_UPDATE))

  Line 367:                     if (!Driver->queryFeature(EVDF_FRAMEBUFFER_OBJECT))

  Line 435:                 if (Driver->queryFeature(EVDF_MIP_MAP_AUTO_UPDATE))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CMeshSceneNode.cpp:
  Line 302:             if (!SceneManager->getVideoDriver()->queryFeature(video::EVDF_STENCIL_BUFFER))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CNullDriver.cpp:
  Line 335:         bool CNullDriver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CNullDriver.h:
  Line 62:             virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9NormalMapRenderer.cpp:
  Line 183:             if (!driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1) ||

  Line 184:                 !driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1))

  Line 212:                 if (driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0))

  Line 250:             if (Driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1) &&

  Line 251:                 Driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9ParallaxMapRenderer.cpp:
  Line 242:             if (!driver->queryFeature(video::EVDF_PIXEL_SHADER_1_4) ||

  Line 243:                 !driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1))

  Line 327:             if (Driver->queryFeature(video::EVDF_PIXEL_SHADER_1_4) &&

  Line 328:                 Driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Texture.cpp:
  Line 108:             if (!Driver->queryFeature(EVDF_TEXTURE_NPOT))

  Line 114:             TextureSize = TextureSize.getOptimalSize(!Driver->queryFeature(EVDF_TEXTURE_NPOT), !Driver->queryFeature(EVDF_TEXTURE_NSQUARE), true, Driver->Caps.MaxTextureWidth);

  Line 267:             core::dimension2d<u32> optSize = ImageSize.getOptimalSize(!Driver->queryFeature(EVDF_TEXTURE_NPOT), !Driver->queryFeature(EVDF_TEXTURE_NSQUARE), true, Driver->Caps.MaxTextureWidth);

  Line 315:             if (mipmaps && Driver->queryFeature(EVDF_MIP_MAP_AUTO_UPDATE))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CCubeSceneNode.cpp:
  Line 155:             if (!SceneManager->getVideoDriver()->queryFeature(video::EVDF_STENCIL_BUFFER))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Driver.cpp:
  Line 578:         bool CD3D8Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const

  Line 740:             if (flag == video::ETCF_CREATE_MIP_MAPS && !queryFeature(EVDF_MIP_MAP))

  Line 1634:             if (queryFeature(EVDF_COLOR_MASK) &&

  Line 1645:             if (queryFeature(EVDF_BLEND_OPERATIONS) &&

  Line 1684:             if (queryFeature(EVDF_POLYGON_OFFSET) && (resetAllRenderstates ||


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Driver.h:
  Line 50:             virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8NormalMapRenderer.cpp:
  Line 129:             if (!driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1) ||

  Line 130:                 !driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1))

  Line 192:             if (Driver->queryFeature(video::EVDF_PIXEL_SHADER_1_1) &&

  Line 193:                 Driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8ParallaxMapRenderer.cpp:
  Line 164:             if (!driver->queryFeature(video::EVDF_PIXEL_SHADER_1_4) ||

  Line 165:                 !driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1))

  Line 239:             if (Driver->queryFeature(video::EVDF_PIXEL_SHADER_1_4) &&

  Line 240:                 Driver->queryFeature(video::EVDF_VERTEX_SHADER_1_1))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Texture.cpp:
  Line 101:             core::dimension2d<u32> optSize = ImageSize.getOptimalSize(!Driver->queryFeature(EVDF_TEXTURE_NPOT), !Driver->queryFeature(EVDF_TEXTURE_NSQUARE), true, Driver->Caps.MaxTextureWidth);

  Line 541:             TextureSize = TextureSize.getOptimalSize(!Driver->queryFeature(EVDF_TEXTURE_NPOT), !Driver->queryFeature(EVDF_TEXTURE_NSQUARE), true, Driver->Caps.MaxTextureWidth);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp:
  Line 643:         bool CD3D9Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const

  Line 838:             if (flag == video::ETCF_CREATE_MIP_MAPS && !queryFeature(EVDF_MIP_MAP))

  Line 1030:                 if (i < 4 && (i == 0 || queryFeature(EVDF_MRT_COLOR_MASK)))

  Line 1380:             if (!queryFeature(EVDF_OCCLUSION_QUERY))

  Line 2397:             if (queryFeature(EVDF_COLOR_MASK) &&

  Line 2408:             if (queryFeature(EVDF_BLEND_OPERATIONS) &&

  Line 2447:             if (queryFeature(EVDF_POLYGON_OFFSET) && (resetAllRenderstates ||

  Line 3676:                 !queryFeature(EVDF_TEXTURE_NPOT),

  Line 3677:                 !queryFeature(EVDF_TEXTURE_NSQUARE), true);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.h:
  Line 74:             virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const;


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9HLSLMaterialRenderer.cpp:
  Line 174:             if (Driver->queryFeature(video::EVDF_VERTEX_SHADER_2_0) || Driver->queryFeature(video::EVDF_VERTEX_SHADER_3_0))


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CAnimatedMeshSceneNode.cpp:
  Line 556:             if (!SceneManager->getVideoDriver()->queryFeature(video::EVDF_STENCIL_BUFFER))

```

**Tool: grep**

**Input:**
```json
{
  "include": "*.cpp",
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht",
  "pattern": "EVDF_TEXTURE_NPOT"
}
```

**Output:**
```
Found 11 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Texture.cpp:
  Line 244:                 !Driver->queryFeature(EVDF_TEXTURE_NPOT),


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLExtensionHandler.cpp:
  Line 791:                 case EVDF_TEXTURE_NPOT:


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLTexture.cpp:
  Line 320:             TextureSize = ImageSize.getOptimalSize(!Driver->queryFeature(EVDF_TEXTURE_NPOT));


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Texture.cpp:
  Line 108:             if (!Driver->queryFeature(EVDF_TEXTURE_NPOT))

  Line 114:             TextureSize = TextureSize.getOptimalSize(!Driver->queryFeature(EVDF_TEXTURE_NPOT), !Driver->queryFeature(EVDF_TEXTURE_NSQUARE), true, Driver->Caps.MaxTextureWidth);

  Line 267:             core::dimension2d<u32> optSize = ImageSize.getOptimalSize(!Driver->queryFeature(EVDF_TEXTURE_NPOT), !Driver->queryFeature(EVDF_TEXTURE_NSQUARE), true, Driver->Caps.MaxTextureWidth);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Driver.cpp:
  Line 629:                 case EVDF_TEXTURE_NPOT:


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp:
  Line 704:                 case EVDF_TEXTURE_NPOT:

  Line 3676:                 !queryFeature(EVDF_TEXTURE_NPOT),


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Texture.cpp:
  Line 101:             core::dimension2d<u32> optSize = ImageSize.getOptimalSize(!Driver->queryFeature(EVDF_TEXTURE_NPOT), !Driver->queryFeature(EVDF_TEXTURE_NSQUARE), true, Driver->Caps.MaxTextureWidth);

  Line 541:             TextureSize = TextureSize.getOptimalSize(!Driver->queryFeature(EVDF_TEXTURE_NPOT), !Driver->queryFeature(EVDF_TEXTURE_NSQUARE), true, Driver->Caps.MaxTextureWidth);

```

---

## Assistant (Build · MiniMax-M2.7 · 8.7s)

_Thinking:_

Let me look at the CD3D11Driver::queryFeature implementation to see what features are currently handled, and compare with D3D9's implementation to understand how NPOT should be handled.

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "limit": 50,
  "offset": 325
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
325:                 return false;
326: 
327:             return true;
328:         }
329: 
330: 
331:         bool CD3D11Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const
332:         {
333:             switch (feature)
334:             {
335:                 case EVDF_RENDER_TO_TARGET:
336:                     return true;
337: 
338:                 case EVDF_MULTITEXTURE:
339:                     return true;
340: 
341:                 case EVDF_BILINEAR_FILTER:
342:                     return true;
343: 
344:                 case EVDF_MIP_MAP:
345:                     return true;
346: 
347:                 case EVDF_MIP_MAP_AUTO_UPDATE:
348:                     return false;
349: 
350:                 case EVDF_VERTEX_SHADER_1_1:
351:                 case EVDF_VERTEX_SHADER_2_0:
352:                 case EVDF_VERTEX_SHADER_3_0:
353:                     return true;
354: 
355:                 case EVDF_PIXEL_SHADER_1_1:
356:                 case EVDF_PIXEL_SHADER_2_0:
357:                 case EVDF_PIXEL_SHADER_3_0:
358:                     return true;
359: 
360:                 case EVDF_HARDWARE_TL:
361:                     return true;
362: 
363:                 case EVDF_TEXTURE_NSQUARE:
364:                     return true;
365: 
366:                 case EVDF_STENCIL_BUFFER:
367:                     return true;
368: 
369:                 case EVDF_ALPHA_TO_COVERAGE:
370:                     return AlphaToCoverageSupport;
371: 
372:                 case EVDF_COLOR_MASK:
373:                     return true;
374: 

(Showing lines 325-374 of 1198. Use offset=375 to continue.)
</content>

<system-reminder>
Instructions from: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\AGENTS.md
# SOURCE KNOWLEDGE BASE

## OVERVIEW
Implementation of the Irrlicht Engine including rendering drivers, scene management, and 3rd party libraries.

## STRUCTURE
```
../include         # header files, including interface definition.
source/Irrlicht/
├── aesGladman/    # AES encryption
├── bzip2/         # bzip2 compression
├── jpeglib/       # JPEG image support
├── libpng/        # PNG image support
├── lzma/          # LZMA compression
├── MacOSX/        # macOS platform specifics (Cocoa/OpenGL)
└── zlib/          # zlib compression
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Rendering | `C*Driver.cpp` | `CNullDriver`, `COpenGLDriver`, `CD3D9Driver` |
| Scene Graph | `C*SceneNode.cpp` | `CSceneManager`, `CMeshSceneNode`, etc. |
| GUI | `CGUI*.cpp` | `CGUIEnvironment`, `CGUIButton`, etc. |
| IO | `C*Reader.cpp` | File system and archive handling |

## CONVENTIONS
- Internal classes prefixed with `C` (e.g., `CSceneManager` implements `ISceneManager`).
- Heavy use of `irr::core` containers for internal logic.
- Platform-specific code isolated in `CIrrDevice*.cpp` and OS subdirectories.

## ANTI-PATTERNS
- Do not add dependencies on `std::`.
- Avoid direct OS API calls outside of `CIrrDevice` or platform directories.

</system-reminder>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D9Driver.cpp",
  "limit": 60,
  "offset": 640
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp</path>
<type>file</type>
<content>
640: 
641: 
642:         //! queries the features of the driver, returns true if feature is available
643:         bool CD3D9Driver::queryFeature(E_VIDEO_DRIVER_FEATURE feature) const
644:         {
645:             if (!FeatureEnabled[feature])
646:                 return false;
647: 
648:             switch (feature)
649:             {
650:                 case EVDF_MULTITEXTURE:
651:                 case EVDF_BILINEAR_FILTER:
652:                     return true;
653: 
654:                 case EVDF_RENDER_TO_TARGET:
655:                     return Caps.NumSimultaneousRTs > 0;
656: 
657:                 case EVDF_HARDWARE_TL:
658:                     return (Caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0;
659: 
660:                 case EVDF_MIP_MAP:
661:                     return (Caps.TextureCaps & D3DPTEXTURECAPS_MIPMAP) != 0;
662: 
663:                 case EVDF_MIP_MAP_AUTO_UPDATE:
664:                     // always return false because a lot of drivers claim they do
665:                     // this but actually don't do this at all.
666:                     return false; // (Caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) != 0;
667: 
668:                 case EVDF_STENCIL_BUFFER:
669:                     return Params.Stencilbuffer && Caps.StencilCaps;
670: 
671:                 case EVDF_VERTEX_SHADER_1_1:
672:                     return Caps.VertexShaderVersion >= D3DVS_VERSION(1, 1);
673: 
674:                 case EVDF_VERTEX_SHADER_2_0:
675:                     return Caps.VertexShaderVersion >= D3DVS_VERSION(2, 0);
676: 
677:                 case EVDF_VERTEX_SHADER_3_0:
678:                     return Caps.VertexShaderVersion >= D3DVS_VERSION(3, 0);
679: 
680:                 case EVDF_PIXEL_SHADER_1_1:
681:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(1, 1);
682: 
683:                 case EVDF_PIXEL_SHADER_1_2:
684:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(1, 2);
685: 
686:                 case EVDF_PIXEL_SHADER_1_3:
687:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(1, 3);
688: 
689:                 case EVDF_PIXEL_SHADER_1_4:
690:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(1, 4);
691: 
692:                 case EVDF_PIXEL_SHADER_2_0:
693:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(2, 0);
694: 
695:                 case EVDF_PIXEL_SHADER_3_0:
696:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(3, 0);
697: 
698:                 case EVDF_HLSL:
699:                     return Caps.VertexShaderVersion >= D3DVS_VERSION(1, 1);

(Showing lines 640-699 of 3792. Use offset=700 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 5.6s)

_Thinking:_

Let me continue reading the D3D9 driver to see how they handle EVDF_TEXTURE_NPOT and EVDF_TEXTURE_NSQUARE, and also read more of the D3D11 driver to see if there's a switch case for those features.

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D9Driver.cpp",
  "limit": 40,
  "offset": 698
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp</path>
<type>file</type>
<content>
698:                 case EVDF_HLSL:
699:                     return Caps.VertexShaderVersion >= D3DVS_VERSION(1, 1);
700: 
701:                 case EVDF_TEXTURE_NSQUARE:
702:                     return (Caps.TextureCaps & D3DPTEXTURECAPS_SQUAREONLY) == 0;
703: 
704:                 case EVDF_TEXTURE_NPOT:
705:                     return (Caps.TextureCaps & D3DPTEXTURECAPS_POW2) == 0;
706: 
707:                 case EVDF_COLOR_MASK:
708:                     return (Caps.PrimitiveMiscCaps & D3DPMISCCAPS_COLORWRITEENABLE) != 0;
709: 
710:                 case EVDF_MULTIPLE_RENDER_TARGETS:
711:                     return Caps.NumSimultaneousRTs > 1;
712: 
713:                 case EVDF_MRT_COLOR_MASK:
714:                     return (Caps.PrimitiveMiscCaps & D3DPMISCCAPS_INDEPENDENTWRITEMASKS) != 0;
715: 
716:                 case EVDF_MRT_BLEND:
717:                     return (Caps.PrimitiveMiscCaps & D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING) != 0;
718: 
719:                 case EVDF_OCCLUSION_QUERY:
720:                     return OcclusionQuerySupport;
721: 
722:                 case EVDF_POLYGON_OFFSET:
723:                     return (Caps.RasterCaps & (D3DPRASTERCAPS_DEPTHBIAS | D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS)) != 0;
724: 
725:                 case EVDF_BLEND_OPERATIONS:
726:                 case EVDF_TEXTURE_MATRIX:
727: #ifdef _IRR_COMPILE_WITH_CG_
728:                 // available iff. define is present
729:                 case EVDF_CG:
730: #endif
731:                     return true;
732: 
733:                 default:
734:                     return false;
735:             }
736: 
737:             ;

(Showing lines 698-737 of 3792. Use offset=738 to continue.)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "limit": 50,
  "offset": 370
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
370:                     return AlphaToCoverageSupport;
371: 
372:                 case EVDF_COLOR_MASK:
373:                     return true;
374: 
375:                 case EVDF_GEOMETRY_SHADER:
376:                     return true;
377: 
378:                 case EVDF_OCCLUSION_QUERY:
379:                     return OcclusionQuerySupport;
380: 
381:                 case EVDF_POLYGON_OFFSET:
382:                     return true;
383: 
384:                 case EVDF_BLEND_OPERATIONS:
385:                     return true;
386: 
387:                 case EVDF_TEXTURE_MATRIX:
388:                     return true;
389:             }
390: 
391:             return false;
392:         }
393: 
394: 
395:         void CD3D11Driver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4 &mat)
396:         {
397:             Matrices[state] = mat;
398:             if (state == ETS_WORLD)
399:                 Transformation3DChanged = true;
400:         }
401: 
402: 
403:         void CD3D11Driver::setMaterial(const SMaterial &material)
404:         {
405:             Material = material;
406: 
407:             for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
408:             {
409:                 setActiveTexture(i, material.getTexture(i));
410:             }
411: 
412:             setBasicRenderStates(material, LastMaterial, true);
413:             LastMaterial = material;
414:         }
415: 
416: 
417:         bool CD3D11Driver::setRenderTarget(video::ITexture *texture, bool clearBackBuffer,
418:                                            bool clearZBuffer, SColor color)
419:         {

(Showing lines 370-419 of 1198. Use offset=420 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 17.0s)

_Thinking:_

Now I understand the issue. Looking at D3D11's spec:

1. **D3D11 always supports NPOT (Non-Power-of-Two) textures** - Unlike D3D9 which had hardware limitations, Direct3D 11 was designed with modern GPU capabilities in mind. NPOT textures are fully supported without any special caps checks.

2. Looking at the D3D9 implementation (lines 704-705):
```cpp
case EVDF_TEXTURE_NPOT:
    return (Caps.TextureCaps & D3DPTEXTURECAPS_POW2) == 0;
```
This returns true only if the POW2 flag is NOT set, meaning the hardware doesn't require power-of-two textures.

3. For D3D11, the `queryFeature` function is missing the `EVDF_TEXTURE_NPOT` case entirely. Since D3D11 spec states that all GPUs supporting Direct3D 11 must support NPOT textures (there is no `D3D11_FORMAT_CAP_NOT_POWER_OF_TWO` flag in D3D11), D3D11 should always return `true` for `EVDF_TEXTURE_NPOT`.

Let me also check what the CD3D11Texture.cpp line 244 is doing:
```cpp
!Driver->queryFeature(EVDF_TEXTURE_NPOT),
```

The code in CD3D11Texture.cpp uses this feature check, but since it's not handled in queryFeature, it falls through to `return false` at line 391, meaning NPOT textures would be treated as unsupported even though D3D11 fully supports them.

I should add the missing case. Let me also look at D3D11's Feature Flags to understand how to properly implement this.

According to Microsoft's D3D11 documentation:
- Direct3D 11 GPUs always support NPOT textures


- The feature level directly indicates this capability, so I can check the device's feature level to confirm NPOT support is always available on D3D11 hardware




根据D3D11规范，Direct3D 11硬件**始终支持NPOT纹理**。D3D11的Feature Level机制保证所有D3D11 GPU都支持NPOT，不像D3D9有`D3DPTEXTURECAPS_POW2`限制。

`CD3D11Driver::queryFeature`缺少`EVDF_TEXTURE_NPOT` case，导致默认返回`false`，这是不正确的。需要在switch中添加：

```cpp
case EVDF_TEXTURE_NPOT:
    return true;
```

D3D11无需像D3D9那样检查caps位，直接返回`true`即可。

需要我添加这个case吗？

---

## User

添加

---

## Assistant (Build · MiniMax-M2.7 · 5.7s)

_Thinking:_

The user wants me to add the EVDF_TEXTURE_NPOT case to the CD3D11Driver::queryFeature function. Let me add it in an appropriate location - it should go near the other texture-related cases like EVDF_TEXTURE_NSQUARE.

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "newString": "                case EVDF_TEXTURE_NSQUARE:\n                    return true;\n\n                case EVDF_TEXTURE_NPOT:\n                    return true;\n\n                case EVDF_STENCIL_BUFFER:",
  "oldString": "                case EVDF_TEXTURE_NSQUARE:\n                    return true;\n\n                case EVDF_STENCIL_BUFFER:"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 11.8s)

_Thinking:_

The edit was applied successfully. The change adds the `EVDF_TEXTURE_NPOT` case that returns `true` in the `queryFeature` method of `CD3D11Driver`. This correctly reflects that D3D11 always supports non-power-of-two textures.



已添加 `EVDF_TEXTURE_NPOT` case，返回 `true`。

---

