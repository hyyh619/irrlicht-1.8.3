# CD3D11Driver MaterialRenderer内存泄漏修复

**Session ID:** ses_204ba96d7ffeC1IA8e0BSduXEK
**Created:** 2026/5/6 11:12:03
**Updated:** 2026/5/6 11:13:42

---

## User

CD3D11Driver::createMaterialRenderers创建了下列对象，但是只是到入CNullDriver的MaterialRenderers进行管理。
            new CD3D11MaterialRenderer(this, matType, "solid");
            new CD3D11MaterialRenderer(this, matType, "solid_lightmap");
            new CD3D11MaterialRenderer(this, matType, "solid_2_layer");
            new CD3D11MaterialRenderer(this, matType, "translucent");
            new CD3D11MaterialRenderer(this, matType, "translucent_2_layer");
            new CD3D11MaterialRenderer(this, matType, "translucent_add_color");
            new CD3D11MaterialRenderer(this, matType, "translucent_vertex_alpha");
            new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel");
            new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel_ref");
            new CD3D11MaterialRenderer(this, matType, "one_texture_blend");
            new CD3D11MaterialRenderer(this, matType, "lightmap_blend");
            new CD3D11MaterialRenderer(this, matType, "detail_map");
            new CD3D11MaterialRenderer(this, matType, "sphere_map");
            new CD3D11MaterialRenderer(this, matType, "reflection_2_layer");
            new CD3D11MaterialRenderer(this, matType, "transparent_reflection_2_layer");

            if (queryFeature(video::EVDF_PIXEL_SHADER_1_1) && queryFeature(video::EVDF_VERTEX_SHADER_1_1))
            {
                new CD3D11NormalMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));
                new CD3D11ParallaxMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));
            }
1. CD3D11MaterialRenderer创建时其ReferenceCounter就为1了
2. 调用CNullDriver::addMaterialRenderer加入到MaterialRenderers其ReferenceCounter为2
3. 但是在释放时只调用了CNullDriver::deleteMaterialRenders,因此其ReferenceCounter只会减到1，不会调用delete
4. 在CD3D11Driver::createMaterialRenderers创建MaterialRenderer对象时，请添加CD3D11Driver自己的管理对象，在CD3D11Driver释放时，先调用CNullDriver::deleteMaterialRenders，再调用CD3D11Driver自己的管理对象对MaterialRenderer对象进行释放。 

---

## Assistant (Build · MiniMax-M2.7 · 8.5s)

_Thinking:_

用户描述了一个引用计数问题：

1. CD3D11MaterialRenderer 创建时引用计数为1
2. 加入到 CNullDriver::addMaterialRenderer 后引用计数变为2
3. 释放时只调用了 CNullDriver::deleteMaterialRenders，引用计数减到1而不是0，所以不会delete
4. 用户建议在 CD3D11Driver 中添加自己的管理对象，在释放时先调用 CNullDriver::deleteMaterialRenders，再调用自己的管理对象释放

我需要先查看相关代码来理解问题。



让我先查看相关代码来理解问题。


**Tool: grep**

**Input:**
```json
{
  "path": "source/Irrlicht",
  "pattern": "createMaterialRenderers"
}
```

**Output:**
```
Found 12 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp:
  Line 476:             createMaterialRenderers();

  Line 1809:         void CD3D11Driver::createMaterialRenderers()


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h:
  Line 380:             void createMaterialRenderers();


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp:
  Line 115:         void CD3D9Driver::createMaterialRenderers()

  Line 456:             createMaterialRenderers();


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLDriver.cpp:
  Line 752:             createMaterialRenderers();

  Line 777:         void COpenGLDriver::createMaterialRenderers()


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Driver.cpp:
  Line 78:         void CD3D8Driver::createMaterialRenderers()

  Line 394:             createMaterialRenderers();


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.h:
  Line 416:             void createMaterialRenderers();


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLDriver.h:
  Line 452:             void createMaterialRenderers();


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Driver.h:
  Line 281:             void createMaterialRenderers();

```

**Tool: grep**

**Input:**
```json
{
  "path": "source/Irrlicht",
  "pattern": "addMaterialRenderer"
}
```

**Output:**
```
Found 85 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CNullDriver.cpp:
  Line 1792:             s32 i = addMaterialRenderer(m);

  Line 1802:         s32 CNullDriver::addMaterialRenderer(IMaterialRenderer *renderer, const char *name)

  Line 2412:                 nullDriver->addMaterialRenderer(imr);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ShaderMaterialRenderer.cpp:
  Line 75:             outMaterialTypeNr = m_Driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11HLSLMaterialRenderer.cpp:
  Line 51:             outMaterialTypeNr = m_Driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9NormalMapRenderer.cpp:
  Line 188:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 207:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 224:                 driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9ShaderMaterialRenderer.cpp:
  Line 80:             outMaterialTypeNr = m_Driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9HLSLMaterialRenderer.cpp:
  Line 61:             outMaterialTypeNr = m_Driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9ParallaxMapRenderer.cpp:
  Line 247:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 266:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 291:                 driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSoftwareDriver2.cpp:
  Line 467:             //!TODO: addMaterialRenderer depends on pushing order....

  Line 468:             addMaterialRenderer (smr); // EMT_SOLID

  Line 469:             addMaterialRenderer (smr); // EMT_SOLID_2_LAYER,

  Line 470:             addMaterialRenderer (smr); // EMT_LIGHTMAP,

  Line 471:             addMaterialRenderer (tmr); // EMT_LIGHTMAP_ADD,

  Line 472:             addMaterialRenderer (smr); // EMT_LIGHTMAP_M2,

  Line 473:             addMaterialRenderer (smr); // EMT_LIGHTMAP_M4,

  Line 474:             addMaterialRenderer (smr); // EMT_LIGHTMAP_LIGHTING,

  Line 475:             addMaterialRenderer (smr); // EMT_LIGHTMAP_LIGHTING_M2,

  Line 476:             addMaterialRenderer (smr); // EMT_LIGHTMAP_LIGHTING_M4,

  Line 477:             addMaterialRenderer (smr); // EMT_DETAIL_MAP,

  Line 478:             addMaterialRenderer (umr); // EMT_SPHERE_MAP,

  Line 479:             addMaterialRenderer (smr); // EMT_REFLECTION_2_LAYER,

  Line 480:             addMaterialRenderer (tmr); // EMT_TRANSPARENT_ADD_COLOR,

  Line 481:             addMaterialRenderer (tmr); // EMT_TRANSPARENT_ALPHA_CHANNEL,

  Line 482:             addMaterialRenderer (tmr); // EMT_TRANSPARENT_ALPHA_CHANNEL_REF,

  Line 483:             addMaterialRenderer (tmr); // EMT_TRANSPARENT_VERTEX_ALPHA,

  Line 484:             addMaterialRenderer (smr); // EMT_TRANSPARENT_REFLECTION_2_LAYER,

  Line 485:             addMaterialRenderer (smr); // EMT_NORMAL_MAP_SOLID,

  Line 486:             addMaterialRenderer (umr); // EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR,

  Line 487:             addMaterialRenderer (tmr); // EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA,

  Line 488:             addMaterialRenderer (smr); // EMT_PARALLAX_MAP_SOLID,

  Line 489:             addMaterialRenderer (tmr); // EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR,

  Line 490:             addMaterialRenderer (tmr); // EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA,

  Line 491:             addMaterialRenderer (tmr); // EMT_ONETEXTURE_BLEND


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp:
  Line 125:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP:

  Line 126:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_ADD:

  Line 127:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M2:

  Line 128:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M4:

  Line 129:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING:

  Line 130:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M2:

  Line 131:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M4:


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLSLMaterialRenderer.cpp:
  Line 179:             outMaterialTypeNr = Driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLParallaxMapRenderer.cpp:
  Line 209:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 226:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 236:                 outMaterialTypeNr = driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLShaderMaterialRenderer.cpp:
  Line 104:             outMaterialTypeNr = Driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLNormalMapRenderer.cpp:
  Line 175:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 192:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 202:                 outMaterialTypeNr = driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLCgMaterialRenderer.cpp:
  Line 242:                 materialType = Driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9CgMaterialRenderer.cpp:
  Line 220:                 materialType = m_Driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8ShaderMaterialRenderer.cpp:
  Line 96:             outMaterialTypeNr = Driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8ParallaxMapRenderer.cpp:
  Line 169:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 185:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 195:                 driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8NormalMapRenderer.cpp:
  Line 134:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 150:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 160:                 driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLDriver.cpp:
  Line 786:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP:

  Line 787:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_ADD:

  Line 788:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M2:

  Line 789:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M4:

  Line 790:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING:

  Line 791:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M2:

  Line 792:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M4:


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Driver.cpp:
  Line 87:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP:

  Line 88:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_ADD:

  Line 89:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M2:

  Line 90:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M4:

  Line 91:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING:

  Line 92:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M2:

  Line 93:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M4:


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11NormalMapRenderer.cpp:
  Line 123:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 129:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 133:             outMaterialTypeNr = driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11CgMaterialRenderer.cpp:
  Line 221:                 materialType = m_Driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ParallaxMapRenderer.cpp:
  Line 137:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 143:                 outMaterialTypeNr = driver->addMaterialRenderer(this);

  Line 147:             outMaterialTypeNr = driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11MaterialRenderer.cpp:
  Line 25:             driver->addMaterialRenderer(this);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CNullDriver.h:
  Line 480:             virtual s32 addMaterialRenderer(IMaterialRenderer *renderer,

```

**Tool: grep**

**Input:**
```json
{
  "path": "source/Irrlicht",
  "pattern": "deleteMaterialRenders"
}
```

**Output:**
```
Found 7 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CNullDriver.cpp:
  Line 228:             deleteMaterialRenders();

  Line 2038:         void CNullDriver::deleteMaterialRenders()


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp:
  Line 277:             deleteMaterialRenders();


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp:
  Line 84:             deleteMaterialRenders();


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLDriver.cpp:
  Line 626:             deleteMaterialRenders();


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Driver.cpp:
  Line 66:             deleteMaterialRenders();


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CNullDriver.h:
  Line 703:             void deleteMaterialRenders();

```

---

## Assistant (Build · MiniMax-M2.7 · 7.1s)

_Thinking:_

Let me read the relevant sections of the CD3D11Driver.cpp and CNullDriver.cpp to understand the issue better.

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "limit": 80,
  "offset": 1805
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
1805:             }
1806:         }
1807: 
1808: 
1809:         void CD3D11Driver::createMaterialRenderers()
1810:         {
1811:             s32    matType = -1;
1812: 
1813:             new CD3D11MaterialRenderer(this, matType, "solid");
1814:             new CD3D11MaterialRenderer(this, matType, "solid_lightmap");
1815:             new CD3D11MaterialRenderer(this, matType, "solid_2_layer");
1816:             new CD3D11MaterialRenderer(this, matType, "translucent");
1817:             new CD3D11MaterialRenderer(this, matType, "translucent_2_layer");
1818:             new CD3D11MaterialRenderer(this, matType, "translucent_add_color");
1819:             new CD3D11MaterialRenderer(this, matType, "translucent_vertex_alpha");
1820:             new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel");
1821:             new CD3D11MaterialRenderer(this, matType, "translucent_alpha_channel_ref");
1822:             new CD3D11MaterialRenderer(this, matType, "one_texture_blend");
1823:             new CD3D11MaterialRenderer(this, matType, "lightmap_blend");
1824:             new CD3D11MaterialRenderer(this, matType, "detail_map");
1825:             new CD3D11MaterialRenderer(this, matType, "sphere_map");
1826:             new CD3D11MaterialRenderer(this, matType, "reflection_2_layer");
1827:             new CD3D11MaterialRenderer(this, matType, "transparent_reflection_2_layer");
1828: 
1829:             if (queryFeature(video::EVDF_PIXEL_SHADER_1_1) && queryFeature(video::EVDF_VERTEX_SHADER_1_1))
1830:             {
1831:                 new CD3D11NormalMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));
1832:                 new CD3D11ParallaxMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));
1833:             }
1834:         }
1835: 
1836: 
1837:         D3D11_TEXTURE_ADDRESS_MODE CD3D11Driver::getTextureWrapMode(const u8 clamp) const
1838:         {
1839:             return D3D11_TEXTURE_ADDRESS_WRAP;
1840:         }
1841: 
1842: 
1843:         bool CD3D11Driver::reset()
1844:         {
1845:             return false;
1846:         }
1847: 
1848: 
1849:         video::ITexture* CD3D11Driver::createDeviceDependentTexture(IImage *surface, const io::path &name, void *mipmapData)
1850:         {
1851:             return new CD3D11Texture(surface, this, TextureCreationFlags, name, mipmapData);
1852:         }
1853: 
1854: 
1855:         void CD3D11Driver::checkDepthBuffer(ITexture *tex)
1856:         {}
1857: 
1858: 
1859:         s32 CD3D11Driver::addShaderMaterial(const c8 *vertexShaderProgram, const c8 *pixelShaderProgram,
1860:                                             IShaderConstantSetCallBack *callback, E_MATERIAL_TYPE baseMaterial, s32 userData)
1861:         {
1862:             return -1;
1863:         }
1864: 
1865: 
1866:         s32 CD3D11Driver::addHighLevelShaderMaterial(
1867:             const c8 *vertexShaderProgram, const c8 *vertexShaderEntryPointName,
1868:             E_VERTEX_SHADER_TYPE vsCompileTarget, const c8 *pixelShaderProgram,
1869:             const c8 *pixelShaderEntryPointName, E_PIXEL_SHADER_TYPE psCompileTarget,
1870:             const c8 *geometryShaderProgram, const c8 *geometryShaderEntryPointName,
1871:             E_GEOMETRY_SHADER_TYPE gsCompileTarget, scene::E_PRIMITIVE_TYPE inType,
1872:             scene::E_PRIMITIVE_TYPE outType, u32 verticesOut,
1873:             IShaderConstantSetCallBack *callback, E_MATERIAL_TYPE baseMaterial,
1874:             s32 userData, E_GPU_SHADING_LANGUAGE shadingLang)
1875:         {
1876:             return -1;
1877:         }
1878: 
1879: 
1880:         void CD3D11Driver::setShadersByType(video::E_VERTEX_TYPE newType)
1881:         {
1882:             if (newType != m_LastVertexType || !m_BuiltInShadersInitialized)
1883:             {
1884:                 if (!m_BuiltInShadersInitialized)

(Showing lines 1805-1884 of 2364. Use offset=1885 to continue.)
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
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CNullDriver.cpp",
  "limit": 30,
  "offset": 1790
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CNullDriver.cpp</path>
<type>file</type>
<content>
1790:         s32 CNullDriver::addAndDropMaterialRenderer(IMaterialRenderer *m)
1791:         {
1792:             s32 i = addMaterialRenderer(m);
1793: 
1794:             if (m)
1795:                 m->drop();
1796: 
1797:             return i;
1798:         }
1799: 
1800: 
1801:         //! Adds a new material renderer to the video device.
1802:         s32 CNullDriver::addMaterialRenderer(IMaterialRenderer *renderer, const char *name)
1803:         {
1804:             if (!renderer)
1805:                 return -1;
1806: 
1807:             SMaterialRenderer r;
1808:             r.Renderer = renderer;
1809:             r.Name     = name;
1810: 
1811:             if (name == 0 && (MaterialRenderers.size() < (sizeof(sBuiltInMaterialTypeNames) / sizeof(char*)) - 1))
1812:             {
1813:                 // set name of built in renderer so that we don't have to implement name
1814:                 // setting in all available renderers.
1815:                 r.Name = sBuiltInMaterialTypeNames[MaterialRenderers.size()];
1816:             }
1817: 
1818:             MaterialRenderers.push_back(r);
1819:             renderer->grab();

(Showing lines 1790-1819 of 2497. Use offset=1820 to continue.)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CNullDriver.cpp",
  "limit": 30,
  "offset": 2035
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CNullDriver.cpp</path>
<type>file</type>
<content>
2035: 
2036: 
2037:         //! deletes all material renderers
2038:         void CNullDriver::deleteMaterialRenders()
2039:         {
2040:             // delete material renderers
2041:             for (u32 i = 0; i < MaterialRenderers.size(); ++i)
2042:             {
2043:                 if (MaterialRenderers[i].Renderer)
2044:                 {
2045:                     MaterialRenderers[i].Renderer->drop();
2046:                 }
2047:             }
2048: 
2049:             MaterialRenderers.clear();
2050:         }
2051: 
2052: 
2053:         //! Returns pointer to material renderer or null
2054:         IMaterialRenderer* CNullDriver::getMaterialRenderer(u32 idx)
2055:         {
2056:             if (idx < MaterialRenderers.size())
2057:                 return MaterialRenderers[idx].Renderer;
2058:             else
2059:                 return 0;
2060:         }
2061: 
2062: 
2063:         //! Returns amount of currently available material renderers.
2064:         u32 CNullDriver::getMaterialRendererCount() const

(Showing lines 2035-2064 of 2497. Use offset=2065 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 8.0s)

_Thinking:_

Now I understand the issue:

1. In `CD3D11Driver::createMaterialRenderers()`, `new CD3D11MaterialRenderer(...)` creates objects with refcount=1
2. The constructor of `CD3D11MaterialRenderer` calls `driver->addMaterialRenderer(this)` which does `renderer->grab()`, so refcount becomes 2
3. When `CNullDriver::deleteMaterialRenders()` is called, it does `renderer->drop()` for each renderer, reducing refcount to 1 (not 0), so delete never happens
4. The fix should be to add the renderers to a CD3D11Driver-managed array and delete them explicitly when CD3D11Driver is destroyed

Let me look at the CD3D11Driver destructor and header to understand the class structure better.

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "limit": 30,
  "offset": 270
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
270: 
271:             return true;
272:         }
273: 
274: 
275:         CD3D11Driver::~CD3D11Driver()
276:         {
277:             deleteMaterialRenders();
278:             deleteAllTextures();
279:             removeAllOcclusionQueries();
280:             removeAllHardwareBuffers();
281: 
282:             for (u32 i = 0; i < m_DepthBuffers.size(); ++i)
283:             {
284:                 m_DepthBuffers[i]->drop();
285:             }
286: 
287:             m_DepthBuffers.clear();
288: 
289:             for (u32 i = 0; i < m_ShaderPool.size(); ++i)
290:             {
291:                 m_ShaderPool[i]->drop();
292:             }
293: 
294:             m_ShaderPool.clear();
295: 
296:             for (u32 i = 0; i < EVT_TANGENTS + 1; ++i)
297:             {
298:                 if (m_InputLayout[i])
299:                 {

(Showing lines 270-299 of 2364. Use offset=300 to continue.)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.h",
  "limit": 50,
  "offset": 370
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h</path>
<type>file</type>
<content>
370:             }
371: 
372:             virtual ECOLOR_FORMAT getColorFormat() const;
373: 
374:             virtual core::dimension2du getMaxTextureSize() const;
375: 
376:             DXGI_FORMAT getDXGIFormatFromColorFormat(ECOLOR_FORMAT format) const;
377: 
378:             ECOLOR_FORMAT getColorFormatFromDXGIFormat(DXGI_FORMAT format) const;
379: 
380:             void createMaterialRenderers();
381: 
382:             D3D11_TEXTURE_ADDRESS_MODE getTextureWrapMode(const u8 clamp) const;
383: 
384:             inline FLOAT* colorToD3D(const SColor &col, FLOAT *f)
385:             {
386:                 f[0]    = col.getRed() / 255.0f;
387:                 f[1]    = col.getGreen() / 255.0f;
388:                 f[2]    = col.getBlue() / 255.0f;
389:                 f[3]    = col.getAlpha() / 255.0f;
390:                 return f;
391:             }
392: 
393:             ID3D11Device                *m_pID3DDevice;
394:             ID3D11DeviceContext         *m_pID3DDeviceContext;
395:             ID3D11Device1               *m_pID3DDevice1;
396:             IDXGISwapChain              *m_SwapChain;
397:             ID3D11RenderTargetView      *m_BackBufferRenderTargetView;
398:             ID3D11DepthStencilView      *m_DepthStencilView;
399: #ifdef _DEBUG
400:             ID3D11Debug    *m_pID3D11Debug;
401: #endif
402: 
403: private:
404: 
405:             enum E_RENDER_MODE
406:             {
407:                 ERM_NONE = 0,
408:                 ERM_2D,
409:                 ERM_3D,
410:                 ERM_STENCIL_FILL,
411:                 ERM_SHADOW_VOLUME_ZFAIL,
412:                 ERM_SHADOW_VOLUME_ZPASS
413:             };
414: 
415:             void setShadersByType(video::E_VERTEX_TYPE newType);
416: 
417:             void setPSTextureAndSamplerState();
418: 
419:             bool setRenderStates3DMode();

(Showing lines 370-419 of 537. Use offset=420 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 5.8s)

_Thinking:_

Now I understand the structure. I need to:

1. Add a container in CD3D11Driver to store the MaterialRenderer pointers it creates
2. When createMaterialRenderers creates objects, also add them to this container
3. In the destructor, after calling deleteMaterialRenders(), also drop the references from this container

Let me look at the private section of CD3D11Driver.h to see where to add the container member, and look at how COpenGLDriver or CD3D9Driver handles this.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D9Driver.cpp",
  "limit": 60,
  "offset": 110
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp</path>
<type>file</type>
<content>
110:             }
111: #endif
112:         }
113: 
114: 
115:         void CD3D9Driver::createMaterialRenderers()
116:         {
117:             // create D3D9 material renderers
118: 
119:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SOLID(m_pID3DDevice, this));
120:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SOLID_2_LAYER(m_pID3DDevice, this));
121: 
122:             // add the same renderer for all lightmap types
123: 
124:             CD3D9MaterialRenderer_LIGHTMAP *lmr = new CD3D9MaterialRenderer_LIGHTMAP(m_pID3DDevice, this);
125:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP:
126:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_ADD:
127:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M2:
128:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_M4:
129:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING:
130:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M2:
131:             addMaterialRenderer(lmr); // for EMT_LIGHTMAP_LIGHTING_M4:
132:             lmr->drop();
133: 
134:             // add remaining fixed function pipeline material renderers
135: 
136:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_DETAIL_MAP(m_pID3DDevice, this));
137:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SPHERE_MAP(m_pID3DDevice, this));
138:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_REFLECTION_2_LAYER(m_pID3DDevice, this));
139:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR(m_pID3DDevice, this));
140:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(m_pID3DDevice, this));
141:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(m_pID3DDevice, this));
142:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(m_pID3DDevice, this));
143:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(m_pID3DDevice, this));
144: 
145:             // add normal map renderers
146: 
147:             s32                      tmp       = 0;
148:             video::IMaterialRenderer *renderer = 0;
149: 
150:             renderer = new CD3D9NormalMapRenderer(m_pID3DDevice, this, tmp,
151:                     MaterialRenderers[EMT_SOLID].Renderer);
152:             renderer->drop();
153: 
154:             renderer = new CD3D9NormalMapRenderer(m_pID3DDevice, this, tmp,
155:                     MaterialRenderers[EMT_TRANSPARENT_ADD_COLOR].Renderer);
156:             renderer->drop();
157: 
158:             renderer = new CD3D9NormalMapRenderer(m_pID3DDevice, this, tmp,
159:                     MaterialRenderers[EMT_TRANSPARENT_VERTEX_ALPHA].Renderer);
160:             renderer->drop();
161: 
162:             // add parallax map renderers
163: 
164:             renderer = new CD3D9ParallaxMapRenderer(m_pID3DDevice, this, tmp,
165:                     MaterialRenderers[EMT_SOLID].Renderer);
166:             renderer->drop();
167: 
168:             renderer = new CD3D9ParallaxMapRenderer(m_pID3DDevice, this, tmp,
169:                     MaterialRenderers[EMT_TRANSPARENT_ADD_COLOR].Renderer);

(Showing lines 110-169 of 3797. Use offset=170 to continue.)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D9Driver.h",
  "limit": 50,
  "offset": 410
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.h</path>
<type>file</type>
<content>
410:                 u32 verticesOut = 0,
411:                 IShaderConstantSetCallBack *callback = 0,
412:                 E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
413:                 s32 userData = 0,
414:                 E_GPU_SHADING_LANGUAGE shadingLang = EGSL_DEFAULT);
415: 
416:             void createMaterialRenderers();
417: 
418:             void draw2D3DVertexPrimitiveList(const void *vertices,
419:                 u32 vertexCount, const void *indexList, u32 primitiveCount,
420:                 E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
421:                 E_INDEX_TYPE iType, bool is3D);
422: 
423:             D3DTEXTUREADDRESS getTextureWrapMode(const u8 clamp);
424: 
425:             inline D3DCOLORVALUE colorToD3D(const SColor &col)
426:             {
427:                 constexpr f32 f = 1.0f / 255.0f;
428:                 D3DCOLORVALUE v;
429: 
430:                 v.r = col.getRed() * f;
431:                 v.g = col.getGreen() * f;
432:                 v.b = col.getBlue() * f;
433:                 v.a = col.getAlpha() * f;
434:                 return v;
435:             }
436: 
437:             E_RENDER_MODE         m_CurrentRenderMode;
438:             D3DPRESENT_PARAMETERS m_present;
439: 
440:             SMaterial      m_Material, m_LastMaterial;
441:             bool           m_ResetRenderStates;
442:             bool           m_Transformation3DChanged;
443:             const ITexture *m_CurrentTexture[MATERIAL_MAX_TEXTURES];
444:             bool           m_LastTextureMipMapsAvailable[MATERIAL_MAX_TEXTURES];
445:             core::matrix4  m_Matrices[ETS_COUNT];
446: 
447:             HINSTANCE        m_D3DLibrary;
448:             IDirect3D9       *m_pID3D;
449:             IDirect3DDevice9 *m_pID3DDevice;
450: 
451:             IDirect3DSurface9      *m_PrevRenderTarget;
452:             core::dimension2d<u32> m_CurrentRendertargetSize;
453: 
454:             HWND            m_WindowId;
455:             core::rect<s32> *m_SceneSourceRect;
456: 
457:             D3DCAPS9 m_Caps;
458: 
459:             SIrrlichtCreationParameters m_Params;

(Showing lines 410-459 of 498. Use offset=460 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 7.6s)

_Thinking:_

Now I understand the issue. Looking at CD3D9Driver::createMaterialRenderers():
- It uses `addAndDropMaterialRenderer` which calls `addMaterialRenderer` and then `drop()`
- This means the ref count is balanced properly

But in CD3D11Driver::createMaterialRenderers():
- It just calls `new CD3D11MaterialRenderer(...)` directly, without using `addAndDropMaterialRenderer`
- The CD3D11MaterialRenderer constructor itself calls `addMaterialRenderer(this)` which does a `grab()`
- So after construction, the ref count is 1 (from addMaterialRenderer) + 1 (from IReferenceCounted default initialization) = 2


- When deleteMaterialRenders() is called later, it only calls drop() once, leaving a ref count of 1 and preventing deletion

The fix requires either adopting CD3D9Driver's approach with addAndDropMaterialRenderer, or if storing pointers for manual deletion, calling drop() on each one after deleteMaterialRenders() completes.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.h",
  "limit": 60,
  "offset": 420
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h</path>
<type>file</type>
<content>
420: 
421:             void setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel);
422: 
423:             void setRenderStatesStencilFillMode(bool alpha);
424: 
425:             void setRenderStatesStencilShadowMode(bool zfail, u32 debugDataVisible);
426: 
427:             bool setActiveTexture(u32 stage, const video::ITexture *texture);
428: 
429:             bool reset();
430: 
431:             virtual video::ITexture* createDeviceDependentTexture(IImage *surface, const io::path &name, void *mipmapData = 0);
432: 
433:             virtual const core::dimension2d<u32>&getCurrentRenderTargetSize() const;
434: 
435:             void checkDepthBuffer(ITexture *tex);
436: 
437:             s32 addShaderMaterial(const c8 *vertexShaderProgram, const c8 *pixelShaderProgram,
438:                                   IShaderConstantSetCallBack *callback,
439:                                   E_MATERIAL_TYPE baseMaterial, s32 userData);
440: 
441:             virtual s32 addHighLevelShaderMaterial(
442:                 const c8 *vertexShaderProgram,
443:                 const c8 *vertexShaderEntryPointName = "main",
444:                 E_VERTEX_SHADER_TYPE vsCompileTarget = EVST_VS_4_0,
445:                 const c8 *pixelShaderProgram = 0,
446:                 const c8 *pixelShaderEntryPointName = "main",
447:                 E_PIXEL_SHADER_TYPE psCompileTarget = EPST_PS_4_0,
448:                 const c8 *geometryShaderProgram = 0,
449:                 const c8 *geometryShaderEntryPointName = "main",
450:                 E_GEOMETRY_SHADER_TYPE gsCompileTarget = EGST_GS_4_0,
451:                 scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
452:                 scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
453:                 u32 verticesOut = 0,
454:                 IShaderConstantSetCallBack *callback = 0,
455:                 E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
456:                 s32 userData = 0,
457:                 E_GPU_SHADING_LANGUAGE shadingLang = EGSL_DEFAULT);
458: 
459:             bool createBuiltInVertexShader(E_VERTEX_TYPE type);
460:             bool createBuiltInPixelShader(E_VERTEX_TYPE type);
461:             bool createInputLayout(E_VERTEX_TYPE type, ID3DBlob *shaderBlob);
462:             void updateMatrixConstantBuffer();
463:             void setRenderStates(E_RENDER_MODE mode, bool alpha);
464:             bool createDefaultStates();
465: 
466:             core::array<SD3D11DepthStencilView*>    m_DepthBuffers;
467: 
468:             void removeDepthSurface(SD3D11DepthStencilView *depth);
469: 
470:             core::array<CD3D11Shader*>      m_ShaderPool;
471:             DXGI_MODE_DESC                  m_SwapChainBufferDesc;
472:             DXGI_SWAP_CHAIN_DESC            m_SwapChainDesc;
473: 
474:             SMaterial           m_Material, m_LastMaterial;
475:             bool                m_ResetRenderStates;
476:             bool                m_Transformation3DChanged;
477:             const ITexture      *m_CurrentTexture[MATERIAL_MAX_TEXTURES];
478:             bool                m_LastTextureMipMapsAvailable[MATERIAL_MAX_TEXTURES];
479:             core::matrix4       m_Matrices[ETS_COUNT];

(Showing lines 420-479 of 537. Use offset=480 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 3.9s)

_Thinking:_

Now I need to see the rest of the header to understand the full structure and find a good place to add the material renderer array.

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.h",
  "limit": 60,
  "offset": 479
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h</path>
<type>file</type>
<content>
479:             core::matrix4       m_Matrices[ETS_COUNT];
480: 
481:             HMODULE                         m_D3D11Library;
482:             IDXGIFactory1                   *m_DXGIFactory;
483:             IDXGIAdapter1                   *m_Adapter;
484:             core::dimension2d<u32>          m_CurrentRendertargetSize;
485:             D3D11_VIEWPORT                  m_Viewport;
486:             HWND                            m_WindowId;
487:             core::rect<s32>                 m_ViewPort;
488:             core::rect<s32>                 *m_SceneSourceRect;
489:             UINT                            m_Caps;
490:             SIrrlichtCreationParameters     m_Params;
491:             E_VERTEX_TYPE                   m_LastVertexType;
492:             ID3D11InputLayout               *m_InputLayout[EVT_TANGENTS + 1];
493:             ID3D11VertexShader              *m_BuiltInVertexShader[EVT_TANGENTS + 1];
494:             ID3D11PixelShader               *m_BuiltInPixelShader[EVT_TANGENTS + 1];
495:             bool                            m_BuiltInShadersInitialized;
496: 
497:             SColorf             m_AmbientLight;
498:             core::stringc       m_VendorName;
499:             u16                 m_VendorID;
500: 
501:             u32     m_MaxTextureUnits;
502:             u32     m_MaxUserClipPlanes;
503:             u32     m_MaxMRTs;
504:             u32     m_NumSetMRTs;
505:             f32     m_MaxLightDistance;
506:             s32     m_LastSetLight;
507: 
508:             ECOLOR_FORMAT       m_ColorFormat;
509:             DXGI_FORMAT         m_DXGIFormat;
510:             bool                m_DeviceRemoved;
511:             bool                m_DriverWasReset;
512:             bool                m_OcclusionQuerySupport;
513:             bool                m_AlphaToCoverageSupport;
514: 
515:             ID3D11Buffer            *m_TempVertexBuffer;
516:             ID3D11Buffer            *m_TempIndexBuffer;
517:             ID3D11Buffer            *m_MatrixConstantBuffer;
518:             u32                     m_TempVertexBufferSize;
519:             u32                     m_TempIndexBufferSize;
520:             E_INDEX_TYPE            m_TempIndexType;
521: 
522:             D3D11_VIEWPORT                      m_DefaultViewport;
523:             D3D11_RECT                          m_DefaultScissorRect;
524:             ID3D11RasterizerState1              *m_RasterizerState;
525:             ID3D11DepthStencilState             *m_DepthStencilState;
526:             ID3D11BlendState1                   *m_BlendState;
527:             CSampler                            *m_DefaultSampler;
528: 
529:             E_RENDER_MODE    m_CurrentRenderMode;
530:         };
531: 
532:         IVideoDriver* createDirectX11Driver(const SIrrlichtCreationParameters &params,
533:                                             io::IFileSystem *io, HWND hwnd);
534:     } // end namespace video
535: } // end namespace irr
536: #endif // _IRR_COMPILE_WITH_DIRECT3D_11_
537: #endif // __C_VIDEO_DIRECTX_11_H_INCLUDED__

(End of file - total 537 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 17.2s)

_Thinking:_

Now I understand the issue. Looking at CD3D11Driver::createMaterialRenderers(), it creates material renderers with `new` but doesn't store them anywhere. When they're added to CNullDriver::MaterialRenderers via `addMaterialRenderer()`, the refcount goes from 1 to 2. Then when `deleteMaterialRenders()` is called, it only does `drop()` (refcount goes from 2 to 1), but the objects are never deleted.

The solution is similar to CD3D9Driver - use a container to store pointers to the material renderers created by CD3D11Driver, so they can be properly deleted.

Looking at CD3D9Driver.h, I don't see a specific container for material renderers - let me check more carefully. Actually, looking at CD3D9Driver::createMaterialRenderers(), it uses `addAndDropMaterialRenderer()` which handles the dropping properly. And for the normal/parallax map renderers, it explicitly calls `renderer->drop()` after adding them.

For CD3D11Driver, the issue is that:
1. It creates objects with `new` (refcount = 1)
2. Adds them via `addMaterialRenderer()` which calls `grab()` (refcount = 2)
3. But never does the second `drop()` to balance it

So the fix should be to store the created material renderer pointers and then delete them in the destructor.

I need to:
1. Add a member variable to store the created material renderers (like `core::array<IMaterialRenderer*> m_MaterialRenderers`)
2. In createMaterialRenderers(), store each created renderer in this array before adding to CNullDriver
3. In the destructor, after deleteMaterialRenders(), iterate through this array and delete each renderer


4. Or alternatively, modify createMaterialRenderers to use the same pattern as CD3D9Driver with addAndDropMaterialRenderer and explicit drop() calls

Let me implement option 1 since it's cleaner and follows the same pattern as the depth buffers and shader pool.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.h",
  "limit": 10,
  "offset": 465
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h</path>
<type>file</type>
<content>
465: 
466:             core::array<SD3D11DepthStencilView*>    m_DepthBuffers;
467: 
468:             void removeDepthSurface(SD3D11DepthStencilView *depth);
469: 
470:             core::array<CD3D11Shader*>      m_ShaderPool;
471:             DXGI_MODE_DESC                  m_SwapChainBufferDesc;
472:             DXGI_SWAP_CHAIN_DESC            m_SwapChainDesc;
473: 
474:             SMaterial           m_Material, m_LastMaterial;

(Showing lines 465-474 of 537. Use offset=475 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 7.6s)

_Thinking:_

Now I understand the issue fully. Let me implement the fix:

1. Add a `core::array<IMaterialRenderer*>` to store created material renderers in CD3D11Driver
2. Store each created material renderer in this array when created (before addMaterialRenderer which grabs and increases ref count)
3. In destructor, after deleteMaterialRenders(), iterate and drop each renderer in the array

The pattern used by CD3D9Driver is cleaner - it uses `addAndDropMaterialRenderer` which adds and immediately drops, keeping ref count balanced. But for CD3D11Driver, they create objects and add them without dropping.

Let me implement the fix properly.



Now I understand the issue. Let me implement the fix:


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.h",
  "newString": "core::array<SD3D11DepthStencilView*>    m_DepthBuffers;\n            core::array<IMaterialRenderer*>      m_MaterialRenderers;\n\n            void removeDepthSurface(SD3D11DepthStencilView *depth);",
  "oldString": "core::array<SD3D11DepthStencilView*>    m_DepthBuffers;\n\n            void removeDepthSurface(SD3D11DepthStencilView *depth);"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 13.0s)

_Thinking:_

Now update createMaterialRenderers to store the created renderers in m_MaterialRenderers and use addAndDropMaterialRenderer or drop them properly:


**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "newString": "void CD3D11Driver::createMaterialRenderers()\n        {\n            s32    matType = -1;\n\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"solid\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"solid_lightmap\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"solid_2_layer\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"translucent\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"translucent_2_layer\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"translucent_add_color\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"translucent_vertex_alpha\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"translucent_alpha_channel\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"translucent_alpha_channel_ref\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"one_texture_blend\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"lightmap_blend\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"detail_map\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"sphere_map\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"reflection_2_layer\"));\n            m_MaterialRenderers.push_back(new CD3D11MaterialRenderer(this, matType, \"transparent_reflection_2_layer\"));\n\n            if (queryFeature(video::EVDF_PIXEL_SHADER_1_1) && queryFeature(video::EVDF_VERTEX_SHADER_1_1))\n            {\n                m_MaterialRenderers.push_back(new CD3D11NormalMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID)));\n                m_MaterialRenderers.push_back(new CD3D11ParallaxMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID)));\n            }\n        }",
  "oldString": "void CD3D11Driver::createMaterialRenderers()\n        {\n            s32    matType = -1;\n\n            new CD3D11MaterialRenderer(this, matType, \"solid\");\n            new CD3D11MaterialRenderer(this, matType, \"solid_lightmap\");\n            new CD3D11MaterialRenderer(this, matType, \"solid_2_layer\");\n            new CD3D11MaterialRenderer(this, matType, \"translucent\");\n            new CD3D11MaterialRenderer(this, matType, \"translucent_2_layer\");\n            new CD3D11MaterialRenderer(this, matType, \"translucent_add_color\");\n            new CD3D11MaterialRenderer(this, matType, \"translucent_vertex_alpha\");\n            new CD3D11MaterialRenderer(this, matType, \"translucent_alpha_channel\");\n            new CD3D11MaterialRenderer(this, matType, \"translucent_alpha_channel_ref\");\n            new CD3D11MaterialRenderer(this, matType, \"one_texture_blend\");\n            new CD3D11MaterialRenderer(this, matType, \"lightmap_blend\");\n            new CD3D11MaterialRenderer(this, matType, \"detail_map\");\n            new CD3D11MaterialRenderer(this, matType, \"sphere_map\");\n            new CD3D11MaterialRenderer(this, matType, \"reflection_2_layer\");\n            new CD3D11MaterialRenderer(this, matType, \"transparent_reflection_2_layer\");\n\n            if (queryFeature(video::EVDF_PIXEL_SHADER_1_1) && queryFeature(video::EVDF_VERTEX_SHADER_1_1))\n            {\n                new CD3D11NormalMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));\n                new CD3D11ParallaxMapRenderer(m_pID3DDevice, m_pID3DDeviceContext, this, matType, getMaterialRenderer(EMT_SOLID));\n            }\n        }"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 12.0s)

_Thinking:_

Now I need to update the destructor to first call deleteMaterialRenders(), then release the material renderers in m_MaterialRenderers.

**Tool: edit**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "newString": "CD3D11Driver::~CD3D11Driver()\n        {\n            deleteMaterialRenders();\n\n            for (u32 i = 0; i < m_MaterialRenderers.size(); ++i)\n            {\n                m_MaterialRenderers[i]->drop();\n            }\n            m_MaterialRenderers.clear();\n\n            deleteAllTextures();",
  "oldString": "CD3D11Driver::~CD3D11Driver()\n        {\n            deleteMaterialRenders();\n            deleteAllTextures();"
}
```

**Output:**
```
Edit applied successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 7.5s)

_Thinking:_

Done. I've made the following changes:

1. Added `core::array<IMaterialRenderer*> m_MaterialRenderers;` to CD3D11Driver.h to store MaterialRenderer pointers

2. Modified CD3D11Driver::createMaterialRenderers() to push all created MaterialRenderer objects into m_MaterialRenderers instead of just letting them leak

3. Modified CD3D11Driver::~CD3D11Driver() to call drop() on all MaterialRenderers in m_MaterialRenderers after deleteMaterialRenders() is called

This ensures:
- ref count is 1 after new
- ref count becomes 2 after addMaterialRenderer
- deleteMaterialRenders reduces ref count to 1
- destructor loop reduces ref count to 0 and calls delete



修改完成：

1. **CD3D11Driver.h** - 添加了 `core::array<IMaterialRenderer*> m_MaterialRenderers;` 用于存储创建的 MaterialRenderer 指针

2. **CD3D11Driver.cpp createMaterialRenderers()** - 将所有 `new CD3D11MaterialRenderer/...` 改为 `m_MaterialRenderers.push_back(new ...)` 

3. **CD3D11Driver.cpp ~CD3D11Driver()** - 在 `deleteMaterialRenders()` 后添加循环，对 `m_MaterialRenderers` 中的每个 renderer 调用 `drop()`，使其引用计数从 1 减到 0 并触发 delete

---

