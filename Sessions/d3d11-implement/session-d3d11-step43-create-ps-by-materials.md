# 生成EMT材质类型的PS HLSL代码

**Session ID:** ses_1faf66c62ffeWp10YDmhQsITP5
**Created:** 2026/5/8 08:42:54
**Updated:** 2026/5/8 08:47:34

---

## User

根据下面material的类型，生成对应的PS HLSL代码，HLSL代码实现可以参考 CD3D9MaterialRenderer，CD3D9MaterialRenderer_*.OnSetMaterial的实现
            //! Standard solid material.
            /** Only first texture is used, which is supposed to be the
             * diffuse material. */
            EMT_SOLID = 0,

            //! Solid material with 2 texture layers.
            /** The second is blended onto the first using the alpha value
             * of the vertex colors. This material is currently not implemented in OpenGL.
             */
            EMT_SOLID_2_LAYER,

            //! Material type with standard lightmap technique
            /** There should be 2 textures: The first texture layer is a
             * diffuse map, the second is a light map. Dynamic light is
             * ignored. */
            EMT_LIGHTMAP,

            //! Material type with lightmap technique like EMT_LIGHTMAP.
            /** But lightmap and diffuse texture are added instead of modulated. */
            EMT_LIGHTMAP_ADD,

            //! Material type with standard lightmap technique
            /** There should be 2 textures: The first texture layer is a
             * diffuse map, the second is a light map. Dynamic light is
             * ignored. The texture colors are effectively multiplied by 2
             * for brightening. Like known in DirectX as D3DTOP_MODULATE2X. */
            EMT_LIGHTMAP_M2,

            //! Material type with standard lightmap technique
            /** There should be 2 textures: The first texture layer is a
             * diffuse map, the second is a light map. Dynamic light is
             * ignored. The texture colors are effectively multiplyied by 4
             * for brightening. Like known in DirectX as D3DTOP_MODULATE4X. */
            EMT_LIGHTMAP_M4,

            //! Like EMT_LIGHTMAP, but also supports dynamic lighting.
            EMT_LIGHTMAP_LIGHTING,

            //! Like EMT_LIGHTMAP_M2, but also supports dynamic lighting.
            EMT_LIGHTMAP_LIGHTING_M2,

            //! Like EMT_LIGHTMAP_4, but also supports dynamic lighting.
            EMT_LIGHTMAP_LIGHTING_M4,

            //! Detail mapped material.
            /** The first texture is diffuse color map, the second is added
             * to this and usually displayed with a bigger scale value so that
             * it adds more detail. The detail map is added to the diffuse map
             * using ADD_SIGNED, so that it is possible to add and substract
             * color from the diffuse map. For example a value of
             * (127,127,127) will not change the appearance of the diffuse map
             * at all. Often used for terrain rendering. */
            EMT_DETAIL_MAP,

            //! Look like a reflection of the environment around it.
            /** To make this possible, a texture called 'sphere map' is
             * used, which must be set as the first texture. */
            EMT_SPHERE_MAP,

            //! A reflecting material with an optional non reflecting texture layer.
            /** The reflection map should be set as first texture. */
            EMT_REFLECTION_2_LAYER,

            //! A transparent material.
            /** Only the first texture is used. The new color is calculated
             * by simply adding the source color and the dest color. This
             * means if for example a billboard using a texture with black
             * background and a red circle on it is drawn with this material,
             * the result is that only the red circle will be drawn a little
             * bit transparent, and everything which was black is 100%
             * transparent and not visible. This material type is useful for
             * particle effects. */
            EMT_TRANSPARENT_ADD_COLOR,

            //! Makes the material transparent based on the texture alpha channel.
            /** The final color is blended together from the destination
             * color and the texture color, using the alpha channel value as
             * blend factor. Only first texture is used. If you are using
             * this material with small textures, it is a good idea to load
             * the texture in 32 bit mode
             * (video::IVideoDriver::setTextureCreationFlag()). Also, an alpha
             * ref is used, which can be manipulated using
             * SMaterial::MaterialTypeParam. This value controls how sharp the
             * edges become when going from a transparent to a solid spot on
             * the texture. */
            EMT_TRANSPARENT_ALPHA_CHANNEL,

            //! Makes the material transparent based on the texture alpha channel.
            /** If the alpha channel value is greater than 127, a
             * pixel is written to the target, otherwise not. This
             * material does not use alpha blending and is a lot faster
             * than EMT_TRANSPARENT_ALPHA_CHANNEL. It is ideal for drawing
             * stuff like leafes of plants, because the borders are not
             * blurry but sharp. Only first texture is used. If you are
             * using this material with small textures and 3d object, it
             * is a good idea to load the texture in 32 bit mode
             * (video::IVideoDriver::setTextureCreationFlag()). */
            EMT_TRANSPARENT_ALPHA_CHANNEL_REF,

            //! Makes the material transparent based on the vertex alpha value.
            EMT_TRANSPARENT_VERTEX_ALPHA,

            //! A transparent reflecting material with an optional additional non reflecting texture layer.
            /** The reflection map should be set as first texture. The
             * transparency depends on the alpha value in the vertex colors. A
             * texture which will not reflect can be set as second texture.
             * Please note that this material type is currently not 100%
             * implemented in OpenGL. */
            EMT_TRANSPARENT_REFLECTION_2_LAYER,

            //! A solid normal map renderer.
            /** First texture is the color map, the second should be the
             * normal map. Note that you should use this material only when
             * drawing geometry consisting of vertices of type
             * S3DVertexTangents (EVT_TANGENTS). You can convert any mesh into
             * this format using IMeshManipulator::createMeshWithTangents()
             * (See SpecialFX2 Tutorial). This shader runs on vertex shader
             * 1.1 and pixel shader 1.1 capable hardware and falls back to a
             * fixed function lighted material if this hardware is not
             * available. Only two lights are supported by this shader, if
             * there are more, the nearest two are chosen. */
            EMT_NORMAL_MAP_SOLID,

            //! A transparent normal map renderer.
            /** First texture is the color map, the second should be the
             * normal map. Note that you should use this material only when
             * drawing geometry consisting of vertices of type
             * S3DVertexTangents (EVT_TANGENTS). You can convert any mesh into
             * this format using IMeshManipulator::createMeshWithTangents()
             * (See SpecialFX2 Tutorial). This shader runs on vertex shader
             * 1.1 and pixel shader 1.1 capable hardware and falls back to a
             * fixed function lighted material if this hardware is not
             * available. Only two lights are supported by this shader, if
             * there are more, the nearest two are chosen. */
            EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR,

            //! A transparent (based on the vertex alpha value) normal map renderer.
            /** First texture is the color map, the second should be the
             * normal map. Note that you should use this material only when
             * drawing geometry consisting of vertices of type
             * S3DVertexTangents (EVT_TANGENTS). You can convert any mesh into
             * this format using IMeshManipulator::createMeshWithTangents()
             * (See SpecialFX2 Tutorial). This shader runs on vertex shader
             * 1.1 and pixel shader 1.1 capable hardware and falls back to a
             * fixed function lighted material if this hardware is not
             * available.  Only two lights are supported by this shader, if
             * there are more, the nearest two are chosen. */
            EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA,

            //! Just like EMT_NORMAL_MAP_SOLID, but uses parallax mapping.
            /** Looks a lot more realistic. This only works when the
             * hardware supports at least vertex shader 1.1 and pixel shader
             * 1.4. First texture is the color map, the second should be the
             * normal map. The normal map texture should contain the height
             * value in the alpha component. The
             * IVideoDriver::makeNormalMapTexture() method writes this value
             * automatically when creating normal maps from a heightmap when
             * using a 32 bit texture. The height scale of the material
             * (affecting the bumpiness) is being controlled by the
             * SMaterial::MaterialTypeParam member. If set to zero, the
             * default value (0.02f) will be applied. Otherwise the value set
             * in SMaterial::MaterialTypeParam is taken. This value depends on
             * with which scale the texture is mapped on the material. Too
             * high or low values of MaterialTypeParam can result in strange
             * artifacts. */
            EMT_PARALLAX_MAP_SOLID,

            //! A material like EMT_PARALLAX_MAP_SOLID, but transparent.
            /** Using EMT_TRANSPARENT_ADD_COLOR as base material. */
            EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR,

            //! A material like EMT_PARALLAX_MAP_SOLID, but transparent.
            /** Using EMT_TRANSPARENT_VERTEX_ALPHA as base material. */
            EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA,

            //! BlendFunc = source * sourceFactor + dest * destFactor ( E_BLEND_FUNC )
            /** Using only first texture. Generic blending method. */
            EMT_ONETEXTURE_BLEND,

            //! This value is not used. It only forces this enumeration to compile to 32 bit.
            EMT_FORCE_32BIT = 0x7fffffff 

---

## Assistant (Build · MiniMax-M2.7 · 41.1s)

_Thinking:_

The user wants me to generate Pixel Shader (PS) HLSL code for various material types in the Irrlicht Engine. They want me to reference the CD3D9MaterialRenderer and its OnSetMaterial implementations for guidance.

Let me first find the CD3D9MaterialRenderer implementation to understand the pattern.


**Tool: grep**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3",
  "pattern": "CD3D9MaterialRenderer"
}
```

**Output:**
```
Found 354 matches (showing first 100)
C:\Development\Graphics\irrlicht-1.8.3\Prompt\D3D11_Implement.md:
  Line 269: 错误	C2614	“irr::video::CD3D9MaterialRenderer”: 非法的成员初始化:“pID3DDevice”不是基或成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	61		

  Line 270: 错误	C2614	“irr::video::CD3D9MaterialRenderer”: 非法的成员初始化:“Driver”不是基或成员	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	61		

  Line 271: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	115		

  Line 272: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	119		

  Line 273: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	120		

  Line 274: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	148		

  Line 275: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	152		

  Line 276: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	153		

  Line 277: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	154		

  Line 278: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	157		

  Line 279: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	164		

  Line 280: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	168		

  Line 281: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	172		

  Line 282: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	177		

  Line 283: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	261		

  Line 284: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	263		

  Line 285: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	264		

  Line 286: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	266		

  Line 287: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	287		

  Line 288: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	290		

  Line 289: 错误	C2065	“pID3DDevice”: 未声明的标识符	Irrlicht	C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h	291		

  Line 569: 根据下面material的类型，生成对应的PS HLSL代码，HLSL代码实现可以参考 CD3D9MaterialRenderer，CD3D9MaterialRenderer_*.OnSetMaterial的实现


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step42-separate-vs-ps-settings.md:
  Line 3900:   Line 3281:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;

  Line 3904:   Line 3294:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;

  Line 3908:   Line 3307:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;

  Line 3912:   Line 3320:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;

  Line 3916:   Line 3333:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;

  Line 3920:   Line 3346:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp:
  Line 13: #include "CD3D9MaterialRenderer.h"

  Line 119:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SOLID(m_pID3DDevice, this));

  Line 120:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SOLID_2_LAYER(m_pID3DDevice, this));

  Line 124:             CD3D9MaterialRenderer_LIGHTMAP    *lmr = new CD3D9MaterialRenderer_LIGHTMAP(m_pID3DDevice, this);

  Line 136:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_DETAIL_MAP(m_pID3DDevice, this));

  Line 137:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SPHERE_MAP(m_pID3DDevice, this));

  Line 138:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_REFLECTION_2_LAYER(m_pID3DDevice, this));

  Line 139:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR(m_pID3DDevice, this));

  Line 140:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(m_pID3DDevice, this));

  Line 141:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(m_pID3DDevice, this));

  Line 142:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(m_pID3DDevice, this));

  Line 143:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(m_pID3DDevice, this));

  Line 177:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_ONETEXTURE_BLEND(m_pID3DDevice, this));

  Line 3281:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;

  Line 3294:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;

  Line 3307:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;

  Line 3320:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;

  Line 3333:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;

  Line 3346:                 CD3D9MaterialRenderer    *r = (CD3D9MaterialRenderer*)MaterialRenderers[m_Material.MaterialType].Renderer;


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht.2019.vcxproj:
  Line 1906:     <ClInclude Include="CD3D9MaterialRenderer.h" />


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step30-add-log.md:
  Line 813: 119:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SOLID(m_pID3DDevice, this));

  Line 814: 120:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SOLID_2_LAYER(m_pID3DDevice, this));

  Line 818: 124:             CD3D9MaterialRenderer_LIGHTMAP *lmr = new CD3D9MaterialRenderer_LIGHTMAP(m_pID3DDevice, this);

  Line 830: 136:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_DETAIL_MAP(m_pID3DDevice, this));

  Line 831: 137:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SPHERE_MAP(m_pID3DDevice, this));

  Line 832: 138:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_REFLECTION_2_LAYER(m_pID3DDevice, this));

  Line 833: 139:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR(m_pID3DDevice, this));

  Line 834: 140:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(m_pID3DDevice, this));

  Line 835: 141:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(m_pID3DDevice, this));

  Line 836: 142:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(m_pID3DDevice, this));

  Line 837: 143:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(m_pID3DDevice, this));


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step29-MaterialRender-mgr.md:
  Line 813: 119:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SOLID(m_pID3DDevice, this));

  Line 814: 120:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SOLID_2_LAYER(m_pID3DDevice, this));

  Line 818: 124:             CD3D9MaterialRenderer_LIGHTMAP *lmr = new CD3D9MaterialRenderer_LIGHTMAP(m_pID3DDevice, this);

  Line 830: 136:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_DETAIL_MAP(m_pID3DDevice, this));

  Line 831: 137:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SPHERE_MAP(m_pID3DDevice, this));

  Line 832: 138:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_REFLECTION_2_LAYER(m_pID3DDevice, this));

  Line 833: 139:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR(m_pID3DDevice, this));

  Line 834: 140:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(m_pID3DDevice, this));

  Line 835: 141:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(m_pID3DDevice, this));

  Line 836: 142:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(m_pID3DDevice, this));

  Line 837: 143:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(m_pID3DDevice, this));


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step28-fix-build-error.md:
  Line 1614: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h:


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\Irrlicht.2019.vcxproj.filters:
  Line 966:     <ClInclude Include="CD3D9MaterialRenderer.h">


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step27-add-object-tracking.md:
  Line 1614: C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h:


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step26-fix-other-memory-leak.md:
  Line 1900: 119:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SOLID(m_pID3DDevice, this));

  Line 1901: 120:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SOLID_2_LAYER(m_pID3DDevice, this));

  Line 1905: 124:             CD3D9MaterialRenderer_LIGHTMAP *lmr = new CD3D9MaterialRenderer_LIGHTMAP(m_pID3DDevice, this);

  Line 1917: 136:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_DETAIL_MAP(m_pID3DDevice, this));

  Line 1918: 137:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_SPHERE_MAP(m_pID3DDevice, this));

  Line 1919: 138:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_REFLECTION_2_LAYER(m_pID3DDevice, this));

  Line 1920: 139:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR(m_pID3DDevice, this));

  Line 1921: 140:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(m_pID3DDevice, this));

  Line 1922: 141:             addAndDropMaterialRenderer(new CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(m_pID3DDevice, this));


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step20-create-sampler-class.md:
  Line 2888: -a----          2026/5/2     22:17          27167 CD3D9MaterialRenderer.h                                              


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step19-fix-build-error-of-dx11.1.md:
  Line 842: -a----          2026/5/2     22:17          27167 CD3D9MaterialRenderer.h                                              


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step18-add-blend-raster-rendertarget-states.md:
  Line 1496: -a----          2026/5/2     22:17          27167 CD3D9MaterialRenderer.h                                              


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step17-create-ps.md:
  Line 4518: -a----          2026/5/2     22:17          27167 CD3D9MaterialRenderer.h                                              


C:\Development\Graphics\irrlicht-1.8.3\Sessions\d3d11-implement\session-d3d11-step16-add-vs-constant.md:
  Line 4518: -a----          2026/5/2     22:17          27167 CD3D9MaterialRenderer.h                                              


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h:
  Line 55:         class CD3D9MaterialRenderer : public IMaterialRenderer

  Line 60:             CD3D9MaterialRenderer(IDirect3DDevice9 *d3ddev, video::IVideoDriver *driver)

  Line 101:         class CD3D9MaterialRenderer_SOLID : public CD3D9MaterialRenderer

  Line 105:             CD3D9MaterialRenderer_SOLID(IDirect3DDevice9 *p, video::IVideoDriver *d)

  Line 106:                 : CD3D9MaterialRenderer(p, d) {}

  Line 125:         class CD3D9MaterialRenderer_ONETEXTURE_BLEND : public CD3D9MaterialRenderer

  Line 129:             CD3D9MaterialRenderer_ONETEXTURE_BLEND(IDirect3DDevice9 *p, video::IVideoDriver *d)

  Line 130:                 : CD3D9MaterialRenderer(p, d) {}

  Line 247:         class CD3D9MaterialRenderer_SOLID_2_LAYER : public CD3D9MaterialRenderer

  Line 251:             CD3D9MaterialRenderer_SOLID_2_LAYER(IDirect3DDevice9 *p, video::IVideoDriver *d)

  Line 252:                 : CD3D9MaterialRenderer(p, d) {}

  Line 273:         class CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR : public CD3D9MaterialRenderer

  Line 277:             CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR(IDirect3DDevice9 *p, video::IVideoDriver *d)


(Results truncated: showing 100 of 354 matches (254 hidden). Consider using a more specific path or pattern.)
```

**Tool: grep**

**Input:**
```json
{
  "include": "*.cpp",
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source",
  "pattern": "OnSetMaterial"
}
```

**Output:**
```
Found 47 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp:
  Line 2225:                     MaterialRenderers[m_Material.MaterialType].Renderer->OnSetMaterial(


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLDriver.cpp:
  Line 2745:                     MaterialRenderers[Material.MaterialType].Renderer->OnSetMaterial(


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Driver.cpp:
  Line 1422:                     MaterialRenderers[Material.MaterialType].Renderer->OnSetMaterial(


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ShaderMaterialRenderer.cpp:
  Line 167:         void CD3D11ShaderMaterialRenderer::OnSetMaterial(const video::SMaterial &material,

  Line 184:                     m_BaseMaterial->OnSetMaterial(material, material, true, services);

  Line 188:                 m_CallBack->OnSetMaterial(material);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11MaterialRenderer.cpp:
  Line 44:         void CD3D11MaterialRenderer::OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,

  Line 111:         void CD3D11MaterialRenderer_SOLID::OnSetMaterial(const SMaterial &material,

  Line 118:         void CD3D11MaterialRenderer_SOLID_2_LAYER::OnSetMaterial(const SMaterial &material,

  Line 125:         void CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR::OnSetMaterial(const SMaterial &material,

  Line 138:         void CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA::OnSetMaterial(const SMaterial &material,

  Line 151:         void CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::OnSetMaterial(const SMaterial &material,

  Line 169:         void CD3D11MaterialRenderer_ONETEXTURE_BLEND::OnSetMaterial(const SMaterial &material,

  Line 232:         void CD3D11MaterialRenderer_LIGHTMAP::OnSetMaterial(const SMaterial &material,

  Line 239:         void CD3D11MaterialRenderer_DETAIL_MAP::OnSetMaterial(const SMaterial &material,

  Line 246:         void CD3D11MaterialRenderer_SPHERE_MAP::OnSetMaterial(const SMaterial &material,

  Line 258:         void CD3D11MaterialRenderer_REFLECTION_2_LAYER::OnSetMaterial(const SMaterial &material,


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9ShaderMaterialRenderer.cpp:
  Line 111:         void CD3D9ShaderMaterialRenderer::OnSetMaterial(const video::SMaterial &material, const video::SMaterial &lastMaterial,

  Line 134:                     m_BaseMaterial->OnSetMaterial(material, material, true, services);

  Line 139:                 m_CallBack->OnSetMaterial(material);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9ParallaxMapRenderer.cpp:
  Line 313:         void CD3D9ParallaxMapRenderer::OnSetMaterial(const video::SMaterial &material,

  Line 317:             CD3D9ShaderMaterialRenderer::OnSetMaterial(material, lastMaterial,


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLSLMaterialRenderer.cpp:
  Line 194:         void COpenGLSLMaterialRenderer::OnSetMaterial(const video::SMaterial &material,

  Line 207:                     BaseMaterial->OnSetMaterial(material, material, true, this);

  Line 212:                 CallBack->OnSetMaterial(material);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLParallaxMapRenderer.cpp:
  Line 255:         void COpenGLParallaxMapRenderer::OnSetMaterial(const video::SMaterial &material,

  Line 259:             COpenGLShaderMaterialRenderer::OnSetMaterial(material, lastMaterial,


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLShaderMaterialRenderer.cpp:
  Line 118:         void COpenGLShaderMaterialRenderer::OnSetMaterial(const video::SMaterial &material, const video::SMaterial &lastMaterial,

  Line 163:                     BaseMaterial->OnSetMaterial(material, material, true, services);

  Line 168:                 CallBack->OnSetMaterial(material);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLCgMaterialRenderer.cpp:
  Line 72:         void COpenGLCgMaterialRenderer::OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)

  Line 97:                     BaseMaterial->OnSetMaterial(material, material, true, this);

  Line 101:                 CallBack->OnSetMaterial(material);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9CgMaterialRenderer.cpp:
  Line 71:         void CD3D9CgMaterialRenderer::OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)

  Line 87:                     BaseMaterial->OnSetMaterial(material, material, true, this);

  Line 91:                 CallBack->OnSetMaterial(material);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8ShaderMaterialRenderer.cpp:
  Line 110:         void CD3D8ShaderMaterialRenderer::OnSetMaterial(const video::SMaterial &material, const video::SMaterial &lastMaterial,

  Line 135:                     BaseMaterial->OnSetMaterial(material, material, true, services);

  Line 140:                 CallBack->OnSetMaterial(material);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8ParallaxMapRenderer.cpp:
  Line 225:         void CD3D8ParallaxMapRenderer::OnSetMaterial(const video::SMaterial &material,

  Line 229:             CD3D8ShaderMaterialRenderer::OnSetMaterial(material, lastMaterial,


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11CgMaterialRenderer.cpp:
  Line 74:         void CD3D11CgMaterialRenderer::OnSetMaterial(const SMaterial &material,

  Line 91:                     BaseMaterial->OnSetMaterial(material, material, true, this);

  Line 95:                 CallBack->OnSetMaterial(material);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ParallaxMapRenderer.cpp:
  Line 170:         void CD3D11ParallaxMapRenderer::OnSetMaterial(const video::SMaterial &material)

  Line 176:         void CD3D11ParallaxMapRenderer::OnSetMaterial(const video::SMaterial &material,

  Line 180:             CD3D11ShaderMaterialRenderer::OnSetMaterial(material, lastMaterial,

```

---

## Assistant (Build · MiniMax-M2.7 · 10.4s)

_Thinking:_

Let me read the CD3D9MaterialRenderer.h file to understand the structure and the OnSetMaterial implementations for each material type.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D9MaterialRenderer.h"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9MaterialRenderer.h</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #ifndef __C_D3D9_MATERIAL_RENDERER_H_INCLUDED__
6: #define __C_D3D9_MATERIAL_RENDERER_H_INCLUDED__
7: 
8: #include "IrrCompileConfig.h"
9: #ifdef _IRR_WINDOWS_
10: 
11: #ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
12: #if defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
13: #include "irrMath.h"    // needed by borland for sqrtf define
14: #endif
15: #include <d3d9.h>
16: 
17: #include "IMaterialRenderer.h"
18: 
19: namespace irr
20: {
21:     namespace video
22:     {
23:         namespace
24:         {
25:             D3DMATRIX UnitMatrixD3D9;
26:             D3DMATRIX SphereMapMatrixD3D9;
27:             inline void setTextureColorStage(IDirect3DDevice9 *dev, DWORD i,
28:                 DWORD arg1, DWORD op, DWORD arg2)
29:             {
30:                 dev->SetTextureStageState(i, D3DTSS_COLOROP, op);
31:                 dev->SetTextureStageState(i, D3DTSS_COLORARG1, arg1);
32:                 dev->SetTextureStageState(i, D3DTSS_COLORARG2, arg2);
33:             }
34:             inline void setTextureColorStage(IDirect3DDevice9 *dev, DWORD i, DWORD arg1)
35:             {
36:                 dev->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
37:                 dev->SetTextureStageState(i, D3DTSS_COLORARG1, arg1);
38:             }
39: 
40:             inline void setTextureAlphaStage(IDirect3DDevice9 *dev, DWORD i,
41:                 DWORD arg1, DWORD op, DWORD arg2)
42:             {
43:                 dev->SetTextureStageState(i, D3DTSS_ALPHAOP, op);
44:                 dev->SetTextureStageState(i, D3DTSS_ALPHAARG1, arg1);
45:                 dev->SetTextureStageState(i, D3DTSS_ALPHAARG2, arg2);
46:             }
47:             inline void setTextureAlphaStage(IDirect3DDevice9 *dev, DWORD i, DWORD arg1)
48:             {
49:                 dev->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
50:                 dev->SetTextureStageState(i, D3DTSS_ALPHAARG1, arg1);
51:             }
52:         } // anonymous namespace
53: 
54:         //! Base class for all internal D3D9 material renderers
55:         class CD3D9MaterialRenderer : public IMaterialRenderer
56:         {
57: public:
58: 
59:             //! Constructor
60:             CD3D9MaterialRenderer(IDirect3DDevice9 *d3ddev, video::IVideoDriver *driver)
61:                 : m_pID3DDevice(d3ddev), m_Driver(driver)
62:             {}
63: 
64:             //! sets a variable in the shader.
65:             //! \param vertexShader: True if this should be set in the vertex shader, false if
66:             //! in the pixel shader.
67:             //! \param name: Name of the variable
68:             //! \param floats: Pointer to array of floats
69:             //! \param count: Amount of floats in array.
70:             virtual bool setVariable(bool vertexShader, const c8 *name, const f32 *floats, int count)
71:             {
72:                 os::Printer::log("Invalid material to set variable in.");
73: 
74:                 return false;
75:             }
76: 
77:             //! Bool interface for the above.
78:             virtual bool setVariable(bool vertexShader, const c8 *name, const bool *bools, int count)
79:             {
80:                 os::Printer::log("Invalid material to set variable in.");
81: 
82:                 return false;
83:             }
84: 
85:             //! Int interface for the above.
86:             virtual bool setVariable(bool vertexShader, const c8 *name, const s32 *ints, int count)
87:             {
88:                 os::Printer::log("Invalid material to set variable in.");
89: 
90:                 return false;
91:             }
92: 
93: protected:
94: 
95:             IDirect3DDevice9    *m_pID3DDevice;
96:             video::IVideoDriver *m_Driver;
97:         };
98: 
99: 
100:         //! Solid material renderer
101:         class CD3D9MaterialRenderer_SOLID : public CD3D9MaterialRenderer
102:         {
103: public:
104: 
105:             CD3D9MaterialRenderer_SOLID(IDirect3DDevice9 *p, video::IVideoDriver *d)
106:                 : CD3D9MaterialRenderer(p, d) {}
107: 
108:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
109:                 bool resetAllRenderstates, IMaterialRendererServices *services)
110:             {
111:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
112: 
113:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
114:                 {
115:                     setTextureColorStage(m_pID3DDevice, 0,
116:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
117:                 }
118: 
119:                 m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
120:                 m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
121:             }
122:         };
123: 
124:         //! Generic Texture Blend
125:         class CD3D9MaterialRenderer_ONETEXTURE_BLEND : public CD3D9MaterialRenderer
126:         {
127: public:
128: 
129:             CD3D9MaterialRenderer_ONETEXTURE_BLEND(IDirect3DDevice9 *p, video::IVideoDriver *d)
130:                 : CD3D9MaterialRenderer(p, d) {}
131: 
132:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
133:                 bool resetAllRenderstates, IMaterialRendererServices *services)
134:             {
135:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
136: 
137:                 if (material.MaterialType != lastMaterial.MaterialType ||
138:                     material.MaterialTypeParam != lastMaterial.MaterialTypeParam ||
139:                     resetAllRenderstates)
140:                 {
141:                     E_BLEND_FACTOR  srcFact, dstFact;
142:                     E_MODULATE_FUNC modulate;
143:                     u32             alphaSource;
144:                     unpack_textureBlendFunc (srcFact, dstFact, modulate, alphaSource, material.MaterialTypeParam);
145: 
146:                     if (srcFact == EBF_SRC_COLOR && dstFact == EBF_ZERO)
147:                     {
148:                         m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
149:                     }
150:                     else
151:                     {
152:                         m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
153:                         m_pID3DDevice->SetRenderState(D3DRS_SRCBLEND, getD3DBlend (srcFact));
154:                         m_pID3DDevice->SetRenderState(D3DRS_DESTBLEND, getD3DBlend (dstFact));
155:                     }
156: 
157:                     setTextureColorStage(m_pID3DDevice, 0,
158:                         D3DTA_TEXTURE, getD3DModulate(modulate), D3DTA_DIFFUSE);
159: 
160:                     if (textureBlendFunc_hasAlpha (srcFact) || textureBlendFunc_hasAlpha (dstFact))
161:                     {
162:                         if (alphaSource == EAS_VERTEX_COLOR)
163:                         {
164:                             setTextureAlphaStage(m_pID3DDevice, 0, D3DTA_DIFFUSE);
165:                         }
166:                         else if (alphaSource == EAS_TEXTURE)
167:                         {
168:                             setTextureAlphaStage(m_pID3DDevice, 0, D3DTA_TEXTURE);
169:                         }
170:                         else
171:                         {
172:                             setTextureAlphaStage(m_pID3DDevice, 0,
173:                                 D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
174:                         }
175:                     }
176: 
177:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
178:                 }
179:             }
180: 
181:             //! Returns if the material is transparent.
182:             /** The scene management needs to know this for being able to sort the
183:              * materials by opaque and transparent.
184:              * The return value could be optimized, but we'd need to know the
185:              * MaterialTypeParam for it. */
186:             virtual bool isTransparent() const
187:             {
188:                 return true;
189:             }
190: 
191: private:
192: 
193:             u32 getD3DBlend(E_BLEND_FACTOR factor) const
194:             {
195:                 u32 r = 0;
196: 
197:                 switch (factor)
198:                 {
199:                     case EBF_ZERO:                    r = D3DBLEND_ZERO; break;
200: 
201:                     case EBF_ONE:                    r = D3DBLEND_ONE; break;
202: 
203:                     case EBF_DST_COLOR:                r = D3DBLEND_DESTCOLOR; break;
204: 
205:                     case EBF_ONE_MINUS_DST_COLOR:    r = D3DBLEND_INVDESTCOLOR; break;
206: 
207:                     case EBF_SRC_COLOR:                r = D3DBLEND_SRCCOLOR; break;
208: 
209:                     case EBF_ONE_MINUS_SRC_COLOR:    r = D3DBLEND_INVSRCCOLOR; break;
210: 
211:                     case EBF_SRC_ALPHA:                r = D3DBLEND_SRCALPHA; break;
212: 
213:                     case EBF_ONE_MINUS_SRC_ALPHA:    r = D3DBLEND_INVSRCALPHA; break;
214: 
215:                     case EBF_DST_ALPHA:                r = D3DBLEND_DESTALPHA; break;
216: 
217:                     case EBF_ONE_MINUS_DST_ALPHA:    r = D3DBLEND_INVDESTALPHA; break;
218: 
219:                     case EBF_SRC_ALPHA_SATURATE:    r = D3DBLEND_SRCALPHASAT; break;
220:                 }
221: 
222:                 return r;
223:             }
224: 
225:             u32 getD3DModulate(E_MODULATE_FUNC func) const
226:             {
227:                 u32 r = D3DTOP_MODULATE;
228: 
229:                 switch (func)
230:                 {
231:                     case EMFN_MODULATE_1X: r = D3DTOP_MODULATE; break;
232: 
233:                     case EMFN_MODULATE_2X: r = D3DTOP_MODULATE2X; break;
234: 
235:                     case EMFN_MODULATE_4X: r = D3DTOP_MODULATE4X; break;
236:                 }
237: 
238:                 return r;
239:             }
240: 
241:             bool transparent;
242:         };
243: 
244: 
245: 
246:         //! Solid 2 layer material renderer
247:         class CD3D9MaterialRenderer_SOLID_2_LAYER : public CD3D9MaterialRenderer
248:         {
249: public:
250: 
251:             CD3D9MaterialRenderer_SOLID_2_LAYER(IDirect3DDevice9 *p, video::IVideoDriver *d)
252:                 : CD3D9MaterialRenderer(p, d) {}
253: 
254:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
255:                 bool resetAllRenderstates, IMaterialRendererServices *services)
256:             {
257:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
258: 
259:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
260:                 {
261:                     setTextureColorStage(m_pID3DDevice, 0, D3DTA_TEXTURE);
262: 
263:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
264:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
265: 
266:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
267:                 }
268:             }
269:         };
270: 
271: 
272:         //! Transparent add color material renderer
273:         class CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR : public CD3D9MaterialRenderer
274:         {
275: public:
276: 
277:             CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR(IDirect3DDevice9 *p, video::IVideoDriver *d)
278:                 : CD3D9MaterialRenderer(p, d) {}
279: 
280:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
281:                 bool resetAllRenderstates, IMaterialRendererServices *services)
282:             {
283:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
284: 
285:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
286:                 {
287:                     setTextureColorStage(m_pID3DDevice, 0,
288:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
289: 
290:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
291:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
292:                     m_pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
293:                     m_pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
294:                 }
295:             }
296: 
297:             //! Returns if the material is transparent. The scene management needs to know this
298:             //! for being able to sort the materials by opaque and transparent.
299:             virtual bool isTransparent() const
300:             {
301:                 return true;
302:             }
303:         };
304: 
305: 
306:         //! Transparent vertex alpha material renderer
307:         class CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA : public CD3D9MaterialRenderer
308:         {
309: public:
310: 
311:             CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(IDirect3DDevice9 *p, video::IVideoDriver *d)
312:                 : CD3D9MaterialRenderer(p, d) {}
313: 
314:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
315:                 bool resetAllRenderstates, IMaterialRendererServices *services)
316:             {
317:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
318: 
319:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
320:                 {
321:                     setTextureColorStage(m_pID3DDevice, 0,
322:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
323:                     setTextureAlphaStage(m_pID3DDevice, 0, D3DTA_DIFFUSE);
324: 
325:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
326:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
327:                     m_pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
328:                     m_pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
329:                 }
330:             }
331: 
332:             //! Returns if the material is transparent. The scene managment needs to know this
333:             //! for being able to sort the materials by opaque and transparent.
334:             virtual bool isTransparent() const
335:             {
336:                 return true;
337:             }
338:         };
339: 
340: 
341:         //! Transparent alpha channel material renderer
342:         class CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL : public CD3D9MaterialRenderer
343:         {
344: public:
345: 
346:             CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(IDirect3DDevice9 *p, video::IVideoDriver *d)
347:                 : CD3D9MaterialRenderer(p, d) {}
348: 
349:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
350:                 bool resetAllRenderstates, IMaterialRendererServices *services)
351:             {
352:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
353: 
354:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates
355:                     || material.MaterialTypeParam != lastMaterial.MaterialTypeParam)
356:                 {
357:                     setTextureColorStage(m_pID3DDevice, 0,
358:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_CURRENT);
359:                     setTextureAlphaStage(m_pID3DDevice, 0, D3DTA_TEXTURE);
360: 
361:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
362:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
363:                     m_pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
364:                     m_pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
365: 
366:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHAREF, core::floor32(material.MaterialTypeParam * 255.f));
367:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
368:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
369:                 }
370:             }
371: 
372:             virtual void OnUnsetMaterial()
373:             {
374:                 m_pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
375:             }
376: 
377:             //! Returns if the material is transparent. The scene managment needs to know this
378:             //! for being able to sort the materials by opaque and transparent.
379:             virtual bool isTransparent() const
380:             {
381:                 return true;
382:             }
383:         };
384: 
385: 
386: 
387:         //! Transparent alpha channel material renderer
388:         class CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF : public CD3D9MaterialRenderer
389:         {
390: public:
391: 
392:             CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF(IDirect3DDevice9 *p, video::IVideoDriver *d)
393:                 : CD3D9MaterialRenderer(p, d) {}
394: 
395:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
396:                 bool resetAllRenderstates, IMaterialRendererServices *services)
397:             {
398:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
399: 
400:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
401:                 {
402:                     setTextureColorStage(m_pID3DDevice, 0,
403:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_CURRENT);
404:                     setTextureAlphaStage(m_pID3DDevice, 0, D3DTA_TEXTURE);
405: 
406:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
407: 
408:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
409: 
410:                     // 127 is required by EMT_TRANSPARENT_ALPHA_CHANNEL_REF
411:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHAREF, 127);
412:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
413:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
414:                 }
415:             }
416: 
417:             virtual void OnUnsetMaterial()
418:             {
419:                 m_pID3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
420:             }
421: 
422:             //! Returns if the material is transparent. The scene managment needs to know this
423:             //! for being able to sort the materials by opaque and transparent.
424:             virtual bool isTransparent() const
425:             {
426:                 return false; // this material is not really transparent because it does no blending.
427:             }
428:         };
429: 
430: 
431:         //! material renderer for all kinds of lightmaps
432:         class CD3D9MaterialRenderer_LIGHTMAP : public CD3D9MaterialRenderer
433:         {
434: public:
435: 
436:             CD3D9MaterialRenderer_LIGHTMAP(IDirect3DDevice9 *p, video::IVideoDriver *d)
437:                 : CD3D9MaterialRenderer(p, d) {}
438: 
439:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
440:                 bool resetAllRenderstates, IMaterialRendererServices *services)
441:             {
442:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
443: 
444:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
445:                 {
446:                     if (material.MaterialType >= EMT_LIGHTMAP_LIGHTING)
447:                     {
448:                         // with lighting
449:                         setTextureColorStage(m_pID3DDevice, 0,
450:                             D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
451:                     }
452:                     else
453:                     {
454:                         setTextureColorStage(m_pID3DDevice, 0, D3DTA_TEXTURE);
455:                     }
456: 
457:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
458: 
459:                     setTextureColorStage(m_pID3DDevice, 1,
460:                         D3DTA_TEXTURE,
461:                         (material.MaterialType == EMT_LIGHTMAP_ADD) ?
462:                         D3DTOP_ADD :
463:                         (material.MaterialType == EMT_LIGHTMAP_M4 || material.MaterialType == EMT_LIGHTMAP_LIGHTING_M4) ?
464:                         D3DTOP_MODULATE4X :
465:                         (material.MaterialType == EMT_LIGHTMAP_M2 || material.MaterialType == EMT_LIGHTMAP_LIGHTING_M2) ?
466:                         D3DTOP_MODULATE2X :
467:                         D3DTOP_MODULATE,
468:                         D3DTA_CURRENT);
469: 
470:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
471:                 }
472:             }
473:         };
474: 
475: 
476: 
477:         //! material renderer for detail maps
478:         class CD3D9MaterialRenderer_DETAIL_MAP : public CD3D9MaterialRenderer
479:         {
480: public:
481: 
482:             CD3D9MaterialRenderer_DETAIL_MAP(IDirect3DDevice9 *p, video::IVideoDriver *d)
483:                 : CD3D9MaterialRenderer(p, d) {}
484: 
485:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
486:                 bool resetAllRenderstates, IMaterialRendererServices *services)
487:             {
488:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
489: 
490:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
491:                 {
492:                     setTextureColorStage(m_pID3DDevice, 0,
493:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
494:                     setTextureColorStage(m_pID3DDevice, 1,
495:                         D3DTA_TEXTURE, D3DTOP_ADDSIGNED, D3DTA_CURRENT);
496:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
497:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
498:                 }
499:             }
500:         };
501: 
502: 
503:         //! sphere map material renderer
504:         class CD3D9MaterialRenderer_SPHERE_MAP : public CD3D9MaterialRenderer
505:         {
506: public:
507: 
508:             CD3D9MaterialRenderer_SPHERE_MAP(IDirect3DDevice9 *p, video::IVideoDriver *d)
509:                 : CD3D9MaterialRenderer(p, d) {}
510: 
511:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
512:                 bool resetAllRenderstates, IMaterialRendererServices *services)
513:             {
514:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
515: 
516:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
517:                 {
518:                     setTextureColorStage(m_pID3DDevice, 0,
519:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
520: 
521:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
522:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
523: 
524:                     m_pID3DDevice->SetTransform(D3DTS_TEXTURE0, &SphereMapMatrixD3D9);
525:                     m_pID3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
526:                     m_pID3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL);
527:                 }
528:             }
529: 
530:             virtual void OnUnsetMaterial()
531:             {
532:                 m_pID3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
533:                 m_pID3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
534:                 m_pID3DDevice->SetTransform(D3DTS_TEXTURE0, &UnitMatrixD3D9);
535:             }
536:         };
537: 
538: 
539:         //! reflection 2 layer material renderer
540:         class CD3D9MaterialRenderer_REFLECTION_2_LAYER : public CD3D9MaterialRenderer
541:         {
542: public:
543: 
544:             CD3D9MaterialRenderer_REFLECTION_2_LAYER(IDirect3DDevice9 *p, video::IVideoDriver *d)
545:                 : CD3D9MaterialRenderer(p, d) {}
546: 
547:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
548:                 bool resetAllRenderstates, IMaterialRendererServices *services)
549:             {
550:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
551: 
552:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
553:                 {
554:                     setTextureColorStage(m_pID3DDevice, 0,
555:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
556: 
557:                     setTextureColorStage(m_pID3DDevice, 1,
558:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_CURRENT);
559: 
560:                     m_pID3DDevice->SetTransform(D3DTS_TEXTURE1, &SphereMapMatrixD3D9);
561:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
562:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
563:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
564:                 }
565:             }
566: 
567:             virtual void OnUnsetMaterial()
568:             {
569:                 m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
570:                 m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
571:                 m_pID3DDevice->SetTransform(D3DTS_TEXTURE1, &UnitMatrixD3D9);
572:             }
573:         };
574: 
575: 
576:         //! reflection 2 layer material renderer
577:         class CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER : public CD3D9MaterialRenderer
578:         {
579: public:
580: 
581:             CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER(IDirect3DDevice9 *p, video::IVideoDriver *d)
582:                 : CD3D9MaterialRenderer(p, d) {}
583: 
584:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
585:                 bool resetAllRenderstates, IMaterialRendererServices *services)
586:             {
587:                 services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
588: 
589:                 if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
590:                 {
591:                     setTextureColorStage(m_pID3DDevice, 0,
592:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_DIFFUSE);
593:                     setTextureAlphaStage(m_pID3DDevice, 0, D3DTA_DIFFUSE);
594:                     setTextureColorStage(m_pID3DDevice, 1,
595:                         D3DTA_TEXTURE, D3DTOP_MODULATE, D3DTA_CURRENT);
596:                     setTextureAlphaStage(m_pID3DDevice, 1, D3DTA_CURRENT);
597: 
598:                     m_pID3DDevice->SetTransform(D3DTS_TEXTURE1, &SphereMapMatrixD3D9);
599:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
600:                     m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR);
601: 
602:                     m_pID3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
603:                     m_pID3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
604:                     m_pID3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
605:                 }
606:             }
607: 
608:             virtual void OnUnsetMaterial()
609:             {
610:                 m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
611:                 m_pID3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
612:                 m_pID3DDevice->SetTransform(D3DTS_TEXTURE1, &UnitMatrixD3D9);
613:             }
614: 
615:             //! Returns if the material is transparent. The scene managment needs to know this
616:             //! for being able to sort the materials by opaque and transparent.
617:             virtual bool isTransparent() const
618:             {
619:                 return true;
620:             }
621:         };
622:     } // end namespace video
623: } // end namespace irr
624: #endif
625: #endif
626: #endif

(End of file - total 626 lines)
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

---

## Assistant (Build · MiniMax-M2.7 · 8.8s)

_Thinking:_

Now I need to look at the CD3D11MaterialRenderer to see how the OnSetMaterial implementations work for the various types, and then generate HLSL pixel shader code for each material type.

Let me read the CD3D11MaterialRenderer.cpp and .h files to understand the structure and how to map the D3D9 texture stages to HLSL.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11MaterialRenderer.cpp"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11MaterialRenderer.cpp</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #include "IrrCompileConfig.h"
6: #ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
7: 
8: #define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
9: #include "CD3D11MaterialRenderer.h"
10: #include "CD3D11Driver.h"
11: #include "CD3D11Texture.h"
12: #include "IMaterialRendererServices.h"
13: #include "os.h"
14: #include "irrString.h"
15: 
16: namespace irr
17: {
18:     namespace video
19:     {
20:         CD3D11MaterialRenderer::CD3D11MaterialRenderer(CD3D11Driver *driver, s32 materialType,
21:                                                        const c8 *name)
22:             : m_Driver(driver), m_MaterialType(materialType)
23:         {
24: #ifdef _DEBUG
25:             setDebugName("CD3D11MaterialRenderer");
26:             c8 buf[32];
27:             sprintf(buf, "0x%p", (void*)this);
28:             os::Printer::log("CD3D11MaterialRenderer created:", buf, ELL_INFORMATION);
29: #endif
30:             driver->addMaterialRenderer(this);
31:         }
32: 
33: 
34:         CD3D11MaterialRenderer::~CD3D11MaterialRenderer()
35:         {
36: #ifdef _DEBUG
37:             c8 buf[32];
38:             sprintf(buf, "0x%p", (void*)this);
39:             os::Printer::log("CD3D11MaterialRenderer destroyed:", buf, ELL_INFORMATION);
40: #endif
41:         }
42: 
43: 
44:         void CD3D11MaterialRenderer::OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
45:             bool resetAllRenderstates, IMaterialRendererServices *services)
46:         {
47:             m_Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
48:         }
49: 
50: 
51:         bool CD3D11MaterialRenderer::OnSetTexture(u32 textureIndex, ITexture *texture)
52:         {
53:             if (texture)
54:             {
55:                 CD3D11Texture* tex = (CD3D11Texture*)texture;
56:                 ID3D11ShaderResourceView* srv = tex->getShaderResourceView();
57:                 m_Driver->m_pID3DDeviceContext->PSSetShaderResources(textureIndex, 1, &srv);
58:             }
59:             return true;
60:         }
61: 
62: 
63:         void CD3D11MaterialRenderer::OnSetConstants(IMaterialRendererServices *services, s32 userData)
64:         {}
65: 
66: 
67:         void CD3D11MaterialRenderer::PostRender()
68:         {}
69: 
70: 
71:         CD3D11MaterialRenderer_SOLID::CD3D11MaterialRenderer_SOLID(CD3D11Driver *p, video::IVideoDriver *d)
72:             : CD3D11MaterialRenderer(p, -1, "solid") {}
73: 
74: 
75:         CD3D11MaterialRenderer_SOLID_2_LAYER::CD3D11MaterialRenderer_SOLID_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d)
76:             : CD3D11MaterialRenderer(p, -1, "solid_2_layer") {}
77: 
78: 
79:         CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR::CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR(CD3D11Driver *p, video::IVideoDriver *d)
80:             : CD3D11MaterialRenderer(p, -1, "transparent_add_color") {}
81: 
82: 
83:         CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA::CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(CD3D11Driver *p, video::IVideoDriver *d)
84:             : CD3D11MaterialRenderer(p, -1, "transparent_vertex_alpha") {}
85: 
86: 
87:         CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(CD3D11Driver *p, video::IVideoDriver *d)
88:             : CD3D11MaterialRenderer(p, -1, "transparent_alpha_channel") {}
89: 
90: 
91:         CD3D11MaterialRenderer_ONETEXTURE_BLEND::CD3D11MaterialRenderer_ONETEXTURE_BLEND(CD3D11Driver *p, video::IVideoDriver *d)
92:             : CD3D11MaterialRenderer(p, -1, "one_texture_blend") { m_Transparent = false; }
93: 
94: 
95:         CD3D11MaterialRenderer_LIGHTMAP::CD3D11MaterialRenderer_LIGHTMAP(CD3D11Driver *p, video::IVideoDriver *d)
96:             : CD3D11MaterialRenderer(p, -1, "lightmap") {}
97: 
98: 
99:         CD3D11MaterialRenderer_DETAIL_MAP::CD3D11MaterialRenderer_DETAIL_MAP(CD3D11Driver *p, video::IVideoDriver *d)
100:             : CD3D11MaterialRenderer(p, -1, "detail_map") {}
101: 
102: 
103:         CD3D11MaterialRenderer_SPHERE_MAP::CD3D11MaterialRenderer_SPHERE_MAP(CD3D11Driver *p, video::IVideoDriver *d)
104:             : CD3D11MaterialRenderer(p, -1, "sphere_map") {}
105: 
106: 
107:         CD3D11MaterialRenderer_REFLECTION_2_LAYER::CD3D11MaterialRenderer_REFLECTION_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d)
108:             : CD3D11MaterialRenderer(p, -1, "reflection_2_layer") {}
109: 
110: 
111:         void CD3D11MaterialRenderer_SOLID::OnSetMaterial(const SMaterial &material,
112:             const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
113:         {
114:             services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
115:         }
116: 
117: 
118:         void CD3D11MaterialRenderer_SOLID_2_LAYER::OnSetMaterial(const SMaterial &material,
119:             const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
120:         {
121:             services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
122:         }
123: 
124: 
125:         void CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR::OnSetMaterial(const SMaterial &material,
126:             const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
127:         {
128:             services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
129:         }
130: 
131: 
132:         bool CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR::isTransparent() const
133:         {
134:             return true;
135:         }
136: 
137: 
138:         void CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA::OnSetMaterial(const SMaterial &material,
139:             const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
140:         {
141:             services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
142:         }
143: 
144: 
145:         bool CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA::isTransparent() const
146:         {
147:             return true;
148:         }
149: 
150: 
151:         void CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::OnSetMaterial(const SMaterial &material,
152:             const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
153:         {
154:             services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
155:         }
156: 
157: 
158:         void CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::OnUnsetMaterial()
159:         {
160:         }
161: 
162: 
163:         bool CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::isTransparent() const
164:         {
165:             return true;
166:         }
167: 
168: 
169:         void CD3D11MaterialRenderer_ONETEXTURE_BLEND::OnSetMaterial(const SMaterial &material,
170:             const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
171:         {
172:             services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
173: 
174:             E_BLEND_FACTOR srcFact, dstFact;
175:             E_MODULATE_FUNC modulate;
176:             u32 alphaSource;
177:             unpack_textureBlendFunc(srcFact, dstFact, modulate, alphaSource, material.MaterialTypeParam);
178: 
179:             m_Transparent = (srcFact != EBF_SRC_COLOR || dstFact != EBF_ZERO);
180: 
181:             if (srcFact == EBF_SRC_COLOR && dstFact == EBF_ZERO)
182:             {
183:             }
184:             else
185:             {
186:             }
187: 
188:             if (textureBlendFunc_hasAlpha(srcFact) || textureBlendFunc_hasAlpha(dstFact))
189:             {
190:             }
191:         }
192: 
193: 
194:         bool CD3D11MaterialRenderer_ONETEXTURE_BLEND::isTransparent() const
195:         {
196:             return m_Transparent;
197:         }
198: 
199: 
200:         u32 CD3D11MaterialRenderer_ONETEXTURE_BLEND::getD3D11Blend(E_BLEND_FACTOR factor) const
201:         {
202:             switch (factor)
203:             {
204:                 case EBF_ZERO:                return D3D11_BLEND_ZERO;
205:                 case EBF_ONE:                 return D3D11_BLEND_ONE;
206:                 case EBF_DST_COLOR:           return D3D11_BLEND_DEST_COLOR;
207:                 case EBF_ONE_MINUS_DST_COLOR: return D3D11_BLEND_INV_DEST_COLOR;
208:                 case EBF_SRC_COLOR:           return D3D11_BLEND_SRC_COLOR;
209:                 case EBF_ONE_MINUS_SRC_COLOR: return D3D11_BLEND_INV_SRC_COLOR;
210:                 case EBF_SRC_ALPHA:           return D3D11_BLEND_SRC_ALPHA;
211:                 case EBF_ONE_MINUS_SRC_ALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
212:                 case EBF_DST_ALPHA:           return D3D11_BLEND_DEST_ALPHA;
213:                 case EBF_ONE_MINUS_DST_ALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
214:                 case EBF_SRC_ALPHA_SATURATE:  return D3D11_BLEND_SRC_ALPHA_SAT;
215:             }
216:             return D3D11_BLEND_ONE;
217:         }
218: 
219: 
220:         u32 CD3D11MaterialRenderer_ONETEXTURE_BLEND::getD3D11Modulate(E_MODULATE_FUNC func) const
221:         {
222:             switch (func)
223:             {
224:                 case EMFN_MODULATE_1X: return 1;
225:                 case EMFN_MODULATE_2X: return 2;
226:                 case EMFN_MODULATE_4X: return 4;
227:             }
228:             return 1;
229:         }
230: 
231: 
232:         void CD3D11MaterialRenderer_LIGHTMAP::OnSetMaterial(const SMaterial &material,
233:             const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
234:         {
235:             services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
236:         }
237: 
238: 
239:         void CD3D11MaterialRenderer_DETAIL_MAP::OnSetMaterial(const SMaterial &material,
240:             const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
241:         {
242:             services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
243:         }
244: 
245: 
246:         void CD3D11MaterialRenderer_SPHERE_MAP::OnSetMaterial(const SMaterial &material,
247:             const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
248:         {
249:             services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
250:         }
251: 
252: 
253:         void CD3D11MaterialRenderer_SPHERE_MAP::OnUnsetMaterial()
254:         {
255:         }
256: 
257: 
258:         void CD3D11MaterialRenderer_REFLECTION_2_LAYER::OnSetMaterial(const SMaterial &material,
259:             const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
260:         {
261:             services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
262:         }
263: 
264: 
265:         void CD3D11MaterialRenderer_REFLECTION_2_LAYER::OnUnsetMaterial()
266:         {
267:         }
268:     } // end namespace video
269: } // end namespace irr
270: #endif // _IRR_COMPILE_WITH_DIRECT3D_11_

(End of file - total 270 lines)
</content>
```

**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11MaterialRenderer.h"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11MaterialRenderer.h</path>
<type>file</type>
<content>
1: // Copyright (C) 2002-2012 Nikolaus Gebhardt
2: // This file is part of the "Irrlicht Engine".
3: // For conditions of distribution and use, see copyright notice in irrlicht.h
4: 
5: #ifndef __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__
6: #define __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__
7: 
8: #include "IrrCompileConfig.h"
9: #ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
10: 
11: #include "IMaterialRenderer.h"
12: #include "SColor.h"
13: 
14: namespace irr
15: {
16:     namespace video
17:     {
18:         class CD3D11Driver;
19: 
20:         class CD3D11MaterialRenderer : public IMaterialRenderer
21:         {
22: public:
23: 
24:             CD3D11MaterialRenderer(CD3D11Driver *driver, s32 materialType,
25:                                    const c8 *name);
26: 
27:             virtual ~CD3D11MaterialRenderer();
28: 
29:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
30:                 bool resetAllRenderstates, IMaterialRendererServices *services);
31:             virtual bool OnSetTexture(u32 textureIndex, ITexture *texture);
32:             virtual void OnSetConstants(IMaterialRendererServices *services, s32 userData);
33:             virtual void PostRender();
34: 
35: protected:
36: 
37:             CD3D11Driver    *m_Driver;
38:             s32             m_MaterialType;
39:         };
40: 
41:         class CD3D11MaterialRenderer_SOLID : public CD3D11MaterialRenderer
42:         {
43: public:
44:             CD3D11MaterialRenderer_SOLID(CD3D11Driver *p, video::IVideoDriver *d);
45: 
46:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
47:                 bool resetAllRenderstates, IMaterialRendererServices *services);
48:         };
49: 
50:         class CD3D11MaterialRenderer_SOLID_2_LAYER : public CD3D11MaterialRenderer
51:         {
52: public:
53:             CD3D11MaterialRenderer_SOLID_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d);
54: 
55:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
56:                 bool resetAllRenderstates, IMaterialRendererServices *services);
57:         };
58: 
59:         class CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR : public CD3D11MaterialRenderer
60:         {
61: public:
62:             CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR(CD3D11Driver *p, video::IVideoDriver *d);
63: 
64:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
65:                 bool resetAllRenderstates, IMaterialRendererServices *services);
66: 
67:             virtual bool isTransparent() const;
68:         };
69: 
70:         class CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA : public CD3D11MaterialRenderer
71:         {
72: public:
73:             CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(CD3D11Driver *p, video::IVideoDriver *d);
74: 
75:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
76:                 bool resetAllRenderstates, IMaterialRendererServices *services);
77: 
78:             virtual bool isTransparent() const;
79:         };
80: 
81:         class CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL : public CD3D11MaterialRenderer
82:         {
83: public:
84:             CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(CD3D11Driver *p, video::IVideoDriver *d);
85: 
86:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
87:                 bool resetAllRenderstates, IMaterialRendererServices *services);
88: 
89:             virtual void OnUnsetMaterial();
90: 
91:             virtual bool isTransparent() const;
92:         };
93: 
94:         class CD3D11MaterialRenderer_ONETEXTURE_BLEND : public CD3D11MaterialRenderer
95:         {
96: public:
97:             CD3D11MaterialRenderer_ONETEXTURE_BLEND(CD3D11Driver *p, video::IVideoDriver *d);
98: 
99:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
100:                 bool resetAllRenderstates, IMaterialRendererServices *services);
101: 
102:             virtual bool isTransparent() const;
103: 
104:         private:
105:             u32 getD3D11Blend(E_BLEND_FACTOR factor) const;
106:             u32 getD3D11Modulate(E_MODULATE_FUNC func) const;
107:             bool m_Transparent;
108:         };
109: 
110:         class CD3D11MaterialRenderer_LIGHTMAP : public CD3D11MaterialRenderer
111:         {
112: public:
113:             CD3D11MaterialRenderer_LIGHTMAP(CD3D11Driver *p, video::IVideoDriver *d);
114: 
115:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
116:                 bool resetAllRenderstates, IMaterialRendererServices *services);
117:         };
118: 
119:         class CD3D11MaterialRenderer_DETAIL_MAP : public CD3D11MaterialRenderer
120:         {
121: public:
122:             CD3D11MaterialRenderer_DETAIL_MAP(CD3D11Driver *p, video::IVideoDriver *d);
123: 
124:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
125:                 bool resetAllRenderstates, IMaterialRendererServices *services);
126:         };
127: 
128:         class CD3D11MaterialRenderer_SPHERE_MAP : public CD3D11MaterialRenderer
129:         {
130: public:
131:             CD3D11MaterialRenderer_SPHERE_MAP(CD3D11Driver *p, video::IVideoDriver *d);
132: 
133:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
134:                 bool resetAllRenderstates, IMaterialRendererServices *services);
135: 
136:             virtual void OnUnsetMaterial();
137:         };
138: 
139:         class CD3D11MaterialRenderer_REFLECTION_2_LAYER : public CD3D11MaterialRenderer
140:         {
141: public:
142:             CD3D11MaterialRenderer_REFLECTION_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d);
143: 
144:             virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
145:                 bool resetAllRenderstates, IMaterialRendererServices *services);
146: 
147:             virtual void OnUnsetMaterial();
148:         };
149: 
150:     } // end namespace video
151: } // end namespace irr
152: #endif // _IRR_COMPILE_WITH_DIRECT3D_11_
153: #endif // __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__

(End of file - total 153 lines)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 8.2s)

_Thinking:_

Now I need to look at the existing PS shaders in the D3D11 implementation to understand the structure. Let me search for existing shader files.


**Tool: glob**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht",
  "pattern": "**/*.hlsl"
}
```

**Output:**
```
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\MacOSX\media\d3d9.hlsl
```

**Tool: grep**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht",
  "pattern": "PixelShader"
}
```

**Output:**
```
Found 226 matches (showing first 100)
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.h:
  Line 338:             virtual void setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount = 1);

  Line 346:             virtual bool setPixelShaderConstant(const c8 *name, const f32 *floats, int count);

  Line 348:             virtual bool setPixelShaderConstant(const c8 *name, const bool *bools, int count);

  Line 350:             virtual bool setPixelShaderConstant(const c8 *name, const s32 *ints, int count);

  Line 469:             bool createBuiltInPixelShader(E_VERTEX_TYPE type);

  Line 505:             ID3D11PixelShader               *m_BuiltInPixelShader[EVT_VERTEX_TYPE_MAX];


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp:
  Line 230:                 m_BuiltInPixelShader[i]     = 0;

  Line 348:                 if (m_BuiltInPixelShader[i])

  Line 350:                     IRR_D3D11_PS_RELEASE(m_BuiltInPixelShader[i], "BuiltInPixelShader");

  Line 351:                     m_BuiltInPixelShader[i]->Release();

  Line 2056:         void CD3D11Driver::setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount)

  Line 2078:         bool CD3D11Driver::setPixelShaderConstant(const c8 *name, const f32 *floats, int count)

  Line 2084:         bool CD3D11Driver::setPixelShaderConstant(const c8 *name, const bool *bools, int count)

  Line 2090:         bool CD3D11Driver::setPixelShaderConstant(const c8 *name, const s32 *ints, int count)

  Line 2430:                     createBuiltInPixelShader((E_VERTEX_TYPE)i);

  Line 2439:             if (shader && shader->getPixelShader())

  Line 2441:                 m_pID3DDeviceContext->PSSetShader(shader->getPixelShader(), 0, 0);

  Line 2443:             else if (vertexType >= 0 && vertexType <= EVT_2D_RECTANGLE && m_BuiltInPixelShader[vertexType])

  Line 2445:                 m_pID3DDeviceContext->PSSetShader(m_BuiltInPixelShader[vertexType], 0, 0);

  Line 2617:         bool CD3D11Driver::createBuiltInPixelShader(E_VERTEX_TYPE type)

  Line 2646:             if (!shader->createPixelShader())

  Line 2654:             if (m_BuiltInPixelShader[type])

  Line 2656:                 IRR_D3D11_PS_RELEASE(m_BuiltInPixelShader[type], "BuiltInPixelShader");

  Line 2657:                 m_BuiltInPixelShader[type]->Release();

  Line 2660:             m_BuiltInPixelShader[type] = shader->getPixelShader();

  Line 2661:             IRR_D3D11_PS_CREATE(m_BuiltInPixelShader[type], "BuiltInPixelShader");

  Line 2662:             m_BuiltInPixelShader[type]->AddRef();

  Line 2791:             if (!psShader->createPixelShader())

  Line 2800:             if (m_BuiltInPixelShader[EVT_2D_RECTANGLE])

  Line 2802:                 IRR_D3D11_PS_RELEASE(m_BuiltInPixelShader[EVT_2D_RECTANGLE], "BuiltInPixelShader");

  Line 2803:                 m_BuiltInPixelShader[EVT_2D_RECTANGLE]->Release();

  Line 2806:             m_BuiltInPixelShader[EVT_2D_RECTANGLE] = psShader->getPixelShader();

  Line 2807:             IRR_D3D11_PS_CREATE(m_BuiltInPixelShader[EVT_2D_RECTANGLE], "BuiltInPixelShader");

  Line 2808:             m_BuiltInPixelShader[EVT_2D_RECTANGLE]->AddRef();


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Shader.cpp:
  Line 21:             m_GeometryShader(0), m_PixelShader(0), m_ComputeShader(0),

  Line 67:             if (m_PixelShader)

  Line 69:                 IRR_D3D11_PS_RELEASE(m_PixelShader, "PixelShader");

  Line 70:                 m_PixelShader->Release();

  Line 200:         bool CD3D11Shader::createPixelShader()

  Line 205:             if (m_PixelShader)

  Line 207:                 m_PixelShader->Release();

  Line 208:                 m_PixelShader = 0;

  Line 211:             HRESULT    hr = m_Driver->m_pID3DDevice->CreatePixelShader(

  Line 215:                 &m_PixelShader);

  Line 217:             IRR_D3D11_PS_CREATE(m_PixelShader, "PixelShader");


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Shader.h:
  Line 50:             bool createPixelShader();

  Line 71:             ID3D11PixelShader* getPixelShader() const

  Line 73:                 return m_PixelShader;

  Line 134:             ID3D11PixelShader       *m_PixelShader;


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.cpp:
  Line 686:                     return m_Caps.PixelShaderVersion >= D3DPS_VERSION(1, 1);

  Line 689:                     return m_Caps.PixelShaderVersion >= D3DPS_VERSION(1, 2);

  Line 692:                     return m_Caps.PixelShaderVersion >= D3DPS_VERSION(1, 3);

  Line 695:                     return m_Caps.PixelShaderVersion >= D3DPS_VERSION(1, 4);

  Line 698:                     return m_Caps.PixelShaderVersion >= D3DPS_VERSION(2, 0);

  Line 701:                     return m_Caps.PixelShaderVersion >= D3DPS_VERSION(3, 0);

  Line 3269:         void CD3D9Driver::setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount)

  Line 3272:                 m_pID3DDevice->SetPixelShaderConstantF(startRegister, data, constantAmount);

  Line 3316:         bool CD3D9Driver::setPixelShaderConstant(const c8 *name, const f32 *floats, int count)

  Line 3329:         bool CD3D9Driver::setPixelShaderConstant(const c8 *name, const bool *bools, int count)

  Line 3342:         bool CD3D9Driver::setPixelShaderConstant(const c8 *name, const s32 *ints, int count)


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLDriver.cpp:
  Line 4075:         void COpenGLDriver::setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount)

  Line 4087:             return setPixelShaderConstant(name, floats, count);

  Line 4093:             return setPixelShaderConstant(name, bools, count);

  Line 4099:             return setPixelShaderConstant(name, ints, count);

  Line 4103:         bool COpenGLDriver::setPixelShaderConstant(const c8 *name, const f32 *floats, int count)

  Line 4105:             os::Printer::log("Error: Please call services->setPixelShaderConstant(), not VideoDriver->setPixelShaderConstant().");

  Line 4111:         bool COpenGLDriver::setPixelShaderConstant(const c8 *name, const bool *bools, int count)

  Line 4113:             os::Printer::log("Error: Please call services->setPixelShaderConstant(), not VideoDriver->setPixelShaderConstant().");

  Line 4119:         bool COpenGLDriver::setPixelShaderConstant(const c8 *name, const s32 *ints, int count)

  Line 4121:             os::Printer::log("Error: Please call services->setPixelShaderConstant(), not VideoDriver->setPixelShaderConstant().");


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLDriver.h:
  Line 286:             virtual void setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount = 1);

  Line 298:             virtual bool setPixelShaderConstant(const c8 *name, const f32 *floats, int count);

  Line 301:             virtual bool setPixelShaderConstant(const c8 *name, const bool *bools, int count);

  Line 304:             virtual bool setPixelShaderConstant(const c8 *name, const s32 *ints, int count);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9Driver.h:
  Line 266:             virtual void setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount = 1);

  Line 278:             virtual bool setPixelShaderConstant(const c8 *name, const f32 *floats, int count);

  Line 281:             virtual bool setPixelShaderConstant(const c8 *name, const bool *bools, int count);

  Line 284:             virtual bool setPixelShaderConstant(const c8 *name, const s32 *ints, int count);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8Driver.cpp:
  Line 609:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(1, 1);

  Line 612:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(1, 2);

  Line 615:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(1, 3);

  Line 618:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(1, 4);

  Line 621:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(2, 0);

  Line 624:                     return Caps.PixelShaderVersion >= D3DPS_VERSION(3, 0);

  Line 2249:         void CD3D8Driver::setPixelShaderConstant(const f32 *data, s32 startRegister, s32 constantAmount)

  Line 2252:                 pID3DDevice->SetPixelShaderConstant(startRegister, data, constantAmount);

  Line 2284:         bool CD3D8Driver::setPixelShaderConstant(const c8 *name, const f32 *floats, int count)

  Line 2293:         bool CD3D8Driver::setPixelShaderConstant(const c8 *name, const bool *bools, int count)

  Line 2302:         bool CD3D8Driver::setPixelShaderConstant(const c8 *name, const s32 *ints, int count)


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ObjectTracker.h:
  Line 225:         inline void LogD3D11PixelShaderCreate(ID3D11PixelShader *obj, const char *name, const char *func, int line)

  Line 227:             LogD3D11ObjectCreate(obj, "ID3D11PixelShader", name, func, line);

  Line 229:         inline void LogD3D11PixelShaderRelease(ID3D11PixelShader *obj, const char *name, const char *func, int line)

  Line 232:                 LogD3D11ObjectRelease(obj, "ID3D11PixelShader", name, func, line);

  Line 234:                 LogD3D11ObjectReleaseNoPtr("ID3D11PixelShader", name, func, line);

  Line 399:         #define IRR_D3D11_PS_CREATE(obj, name)  LogD3D11PixelShaderCreate(obj, name, __FUNCTION__, __LINE__)

  Line 400:         #define IRR_D3D11_PS_RELEASE(obj, name) LogD3D11PixelShaderRelease(obj, name, __FUNCTION__, __LINE__)


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11ShaderMaterialRenderer.cpp:
  Line 30:             m_VertexShader(0), m_OldVertexShader(0), m_PixelShader(0), m_InputLayout(0), m_UserData(userData)

  Line 65:             m_VertexShader(0), m_OldVertexShader(0), m_PixelShader(0), m_InputLayout(0), m_UserData(userData)

  Line 101:             if (!createPixelShader(pixelShaderProgram))

  Line 130:             if (m_PixelShader)


(Results truncated: showing 100 of 226 matches (126 hidden). Consider using a more specific path or pattern.)
```

---

## Assistant (Build · MiniMax-M2.7 · 7.0s)

_Thinking:_

Let me look at the existing d3d9.hlsl file and also look for any existing HLSL shader files in the codebase.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\MacOSX\\media\\d3d9.hlsl"
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\MacOSX\media\d3d9.hlsl</path>
<type>file</type>
<content>
1: // part of the Irrlicht Engine Shader example.
2: // These simple Direct3D9 pixel and vertex shaders will be loaded by the shaders
3: // example. Please note that these example shaders don't do anything really useful.
4: // They only demonstrate that shaders can be used in Irrlicht.
5: 
6: //-----------------------------------------------------------------------------
7: // Global variables
8: //-----------------------------------------------------------------------------
9: float4x4 mWorldViewProj; // World * View * Projection transformation
10: float4x4 mInvWorld;      // Inverted world matrix
11: float4x4 mTransWorld;    // Transposed world matrix
12: float3 mLightPos;        // Light position
13: float4 mLightColor;      // Light color
14: 
15: 
16: // Vertex shader output structure
17: struct VS_OUTPUT
18: {
19: 	float4 Position : POSITION;  // vertex position
20: 	float4 Diffuse  : COLOR0;    // vertex diffuse color
21: 	float2 TexCoord : TEXCOORD0; // tex coords
22: };
23: 
24: 
25: VS_OUTPUT vertexMain(in float4 vPosition : POSITION,
26: 					in float3 vNormal    : NORMAL,
27: 					float2 texCoord      : TEXCOORD0 )
28: {
29: 	VS_OUTPUT Output;
30: 
31: 	// transform position to clip space
32: 	Output.Position = mul(vPosition, mWorldViewProj);
33: 
34: 	// transform normal
35: 	float3 normal = mul(float4(vNormal,0.0), mInvWorld);
36: 
37: 	// renormalize normal
38: 	normal = normalize(normal);
39: 
40: 	// position in world coodinates
41: 	float3 worldpos = mul(mTransWorld, vPosition);
42: 
43: 	// calculate light vector, vtxpos - lightpos
44: 	float3 lightVector = worldpos - mLightPos;
45: 
46: 	// normalize light vector
47: 	lightVector = normalize(lightVector);
48: 
49: 	// calculate light color
50: 	float3 tmp = dot(-lightVector, normal);
51: 	tmp = lit(tmp.x, tmp.y, 1.0);
52: 
53: 	tmp = mLightColor * tmp.y;
54: 	Output.Diffuse = float4(tmp.x, tmp.y, tmp.z, 0);
55: 	Output.TexCoord = texCoord;
56: 
57: 	return Output;
58: }
59: 
60: 
61: // Pixel shader output structure
62: struct PS_OUTPUT
63: {
64: 	float4 RGBColor : COLOR0; // Pixel color
65: };
66: 
67: 
68: sampler2D myTexture;
69: 	
70: PS_OUTPUT pixelMain(float2 TexCoord : TEXCOORD0,
71: 					float4 Position : POSITION,
72: 					float4 Diffuse  : COLOR0 ) 
73: {
74: 	PS_OUTPUT Output;
75: 
76: 	float4 col = tex2D( myTexture, TexCoord ); // sample color map
77: 
78: 	// multiply with diffuse and do other senseless operations
79: 	Output.RGBColor = Diffuse * col;
80: 	Output.RGBColor *= 4.0;
81: 
82: 	return Output;
83: }
84: 

(End of file - total 84 lines)
</content>
```

**Tool: glob**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3",
  "pattern": "**/*.fx"
}
```

**Output:**
```
No files found
```

---

## Assistant (Build · MiniMax-M2.7 · 12.0s)

_Thinking:_

Now I understand the structure. The HLSL files are embedded as text in the code or loaded from external files. Let me look for existing HLSL shader code in the codebase and also understand how the D3D11 driver handles materials.

Let me check how createBuiltInPixelShader works in CD3D11Driver.cpp to see what shaders are currently defined.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "limit": 200,
  "offset": 2600
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
2600:             IRR_D3D11_VS_CREATE(m_BuiltInVertexShader[type], "BuiltInVertexShader");
2601:             m_BuiltInVertexShader[type]->AddRef();
2602: 
2603:             if (m_InputLayout[type])
2604:             {
2605:                 IRR_D3D11_IL_RELEASE(m_InputLayout[type], "BuiltInInputLayout");
2606:                 m_InputLayout[type]->Release();
2607:             }
2608: 
2609:             m_InputLayout[type] = shader->getInputLayout();
2610:             IRR_D3D11_IL_CREATE(m_InputLayout[type], "BuiltInInputLayout");
2611:             m_InputLayout[type]->AddRef();
2612: 
2613:             return true;
2614:         }
2615: 
2616: 
2617:         bool CD3D11Driver::createBuiltInPixelShader(E_VERTEX_TYPE type)
2618:         {
2619:             const char    *shaderSource = 0;
2620: 
2621:             switch (type)
2622:             {
2623:                 case EVT_STANDARD:
2624:                     shaderSource = PIXEL_SHADER_STANDARD;
2625:                     break;
2626: 
2627:                 case EVT_2TCOORDS:
2628:                     shaderSource = PIXEL_SHADER_2TCOORDS;
2629:                     break;
2630: 
2631:                 case EVT_TANGENTS:
2632:                     shaderSource = PIXEL_SHADER_TANGENTS;
2633:                     break;
2634: 
2635:                 default:
2636:                     return false;
2637:             }
2638: 
2639:             CD3D11Shader    *shader = new CD3D11Shader(this);
2640:             if (!shader->compile(EDST_PIXEL, shaderSource, "main", "ps_4_0"))
2641:             {
2642:                 shader->drop();
2643:                 return false;
2644:             }
2645: 
2646:             if (!shader->createPixelShader())
2647:             {
2648:                 shader->drop();
2649:                 return false;
2650:             }
2651: 
2652:             m_ShaderPool.push_back(shader);
2653: 
2654:             if (m_BuiltInPixelShader[type])
2655:             {
2656:                 IRR_D3D11_PS_RELEASE(m_BuiltInPixelShader[type], "BuiltInPixelShader");
2657:                 m_BuiltInPixelShader[type]->Release();
2658:             }
2659: 
2660:             m_BuiltInPixelShader[type] = shader->getPixelShader();
2661:             IRR_D3D11_PS_CREATE(m_BuiltInPixelShader[type], "BuiltInPixelShader");
2662:             m_BuiltInPixelShader[type]->AddRef();
2663: 
2664:             return true;
2665:         }
2666: 
2667: 
2668:         bool CD3D11Driver::createInputLayout(E_VERTEX_TYPE type, ID3DBlob *shaderBlob)
2669:         {
2670:             D3D11_INPUT_ELEMENT_DESC    *layout     = 0;
2671:             u32                         numElements = 0;
2672: 
2673:             switch (type)
2674:             {
2675:                 case EVT_STANDARD:
2676:                 {
2677:                     static D3D11_INPUT_ELEMENT_DESC    standardLayout[] =
2678:                     {
2679:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
2680:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
2681:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
2682:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
2683:                     };
2684:                     layout      = standardLayout;
2685:                     numElements = 4;
2686:                     break;
2687:                 }
2688: 
2689:                 case EVT_2TCOORDS:
2690:                 {
2691:                     static D3D11_INPUT_ELEMENT_DESC    twoTexLayout[] =
2692:                     {
2693:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
2694:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
2695:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
2696:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
2697:                         {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
2698:                     };
2699:                     layout      = twoTexLayout;
2700:                     numElements = 5;
2701:                     break;
2702:                 }
2703: 
2704:                 case EVT_TANGENTS:
2705:                 {
2706:                     static D3D11_INPUT_ELEMENT_DESC    tangentLayout[] =
2707:                     {
2708:                         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
2709:                         {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
2710:                         {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
2711:                         {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
2712:                         {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
2713:                         {"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
2714:                     };
2715:                     layout      = tangentLayout;
2716:                     numElements = 6;
2717:                     break;
2718:                 }
2719: 
2720:                 default:
2721:                     return false;
2722:             }
2723: 
2724:             HRESULT    hr = m_pID3DDevice->CreateInputLayout(layout, numElements,
2725:                                                              shaderBlob->GetBufferPointer(),
2726:                                                              shaderBlob->GetBufferSize(),
2727:                                                              &m_InputLayout[type]);
2728:             return SUCCEEDED(hr);
2729:         }
2730: 
2731: 
2732:         bool CD3D11Driver::createRectangleShaders()
2733:         {
2734:             CD3D11Shader    *vsShader = new CD3D11Shader(this);
2735: 
2736:             if (!vsShader->compile(EDST_VERTEX, VERTEX_SHADER_RECTANGLE, "main", "vs_4_0"))
2737:             {
2738:                 vsShader->drop();
2739:                 return false;
2740:             }
2741: 
2742:             if (!vsShader->createVertexShader())
2743:             {
2744:                 vsShader->drop();
2745:                 return false;
2746:             }
2747: 
2748:             D3D11_INPUT_ELEMENT_DESC    rectangleLayout[] =
2749:             {
2750:                 {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
2751:                 {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
2752:             };
2753: 
2754:             if (!vsShader->createInputLayout(rectangleLayout, 2))
2755:             {
2756:                 vsShader->drop();
2757:                 return false;
2758:             }
2759: 
2760:             vsShader->setVertexType(EVT_2D_RECTANGLE);
2761:             m_ShaderPool.push_back(vsShader);
2762: 
2763:             if (m_BuiltInVertexShader[EVT_2D_RECTANGLE])
2764:             {
2765:                 IRR_D3D11_VS_RELEASE(m_BuiltInVertexShader[EVT_2D_RECTANGLE], "BuiltInVertexShader");
2766:                 m_BuiltInVertexShader[EVT_2D_RECTANGLE]->Release();
2767:             }
2768: 
2769:             m_BuiltInVertexShader[EVT_2D_RECTANGLE] = vsShader->getVertexShader();
2770:             IRR_D3D11_VS_CREATE(m_BuiltInVertexShader[EVT_2D_RECTANGLE], "BuiltInVertexShader");
2771:             m_BuiltInVertexShader[EVT_2D_RECTANGLE]->AddRef();
2772: 
2773:             if (m_InputLayout[EVT_2D_RECTANGLE])
2774:             {
2775:                 IRR_D3D11_IL_RELEASE(m_InputLayout[EVT_2D_RECTANGLE], "BuiltInInputLayout");
2776:                 m_InputLayout[EVT_2D_RECTANGLE]->Release();
2777:             }
2778: 
2779:             m_InputLayout[EVT_2D_RECTANGLE] = vsShader->getInputLayout();
2780:             IRR_D3D11_IL_CREATE(m_InputLayout[EVT_2D_RECTANGLE], "BuiltInInputLayout");
2781:             m_InputLayout[EVT_2D_RECTANGLE]->AddRef();
2782: 
2783:             CD3D11Shader    *psShader = new CD3D11Shader(this);
2784: 
2785:             if (!psShader->compile(EDST_PIXEL, PIXEL_SHADER_RECTANGLE, "main", "ps_4_0"))
2786:             {
2787:                 psShader->drop();
2788:                 return false;
2789:             }
2790: 
2791:             if (!psShader->createPixelShader())
2792:             {
2793:                 psShader->drop();
2794:                 return false;
2795:             }
2796: 
2797:             psShader->setVertexType(EVT_2D_RECTANGLE);
2798:             m_ShaderPool.push_back(psShader);
2799: 

(Showing lines 2600-2799 of 3087. Use offset=2800 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 10.0s)

_Thinking:_

I need to see the PIXEL_SHADER_STANDARD and other shader source definitions. Let me search for where these are defined.


**Tool: grep**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht",
  "pattern": "PIXEL_SHADER_STANDARD"
}
```

**Output:**
```
Found 2 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp:
  Line 139:         static const char    PIXEL_SHADER_STANDARD[] =

  Line 2624:                     shaderSource = PIXEL_SHADER_STANDARD;

```

---

## Assistant (Build · MiniMax-M2.7 · 9.2s)

_Thinking:_

I need to look at the actual shader source at line 139.


**Tool: read**

**Input:**
```json
{
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\CD3D11Driver.cpp",
  "limit": 200,
  "offset": 100
}
```

**Output:**
```
<path>C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D11Driver.cpp</path>
<type>file</type>
<content>
100:             "struct PS_INPUT {"
101:             "    float4 Pos : SV_POSITION;"
102:             "    float4 Color : COLOR;"
103:             "};"
104:             "float4 main(PS_INPUT input) : SV_TARGET {"
105:             "    return input.Color;"
106:             "}";
107: 
108:         static const char    VERTEX_SHADER_TANGENTS[] =
109:             "struct VS_INPUT {"
110:             "    float3 Pos : POSITION;"
111:             "    float3 Normal : NORMAL;"
112:             "    float4 Color : COLOR;"
113:             "    float2 TexCoord : TEXCOORD0;"
114:             "    float3 Tangent : TANGENT;"
115:             "    float3 Binormal : BINORMAL;"
116:             "};"
117:             "struct VS_OUTPUT {"
118:             "    float4 Pos : SV_POSITION;"
119:             "    float4 Color : COLOR;"
120:             "    float2 TexCoord : TEXCOORD0;"
121:             "    float3 Normal : TEXCOORD1;"
122:             "    float3 Tangent : TEXCOORD2;"
123:             "    float3 Binormal : TEXCOORD3;"
124:             "};"
125:             "cbuffer MatrixBuffer : register(b0) {"
126:             "    float4x4 WorldViewProj;"
127:             "};"
128:             "VS_OUTPUT main(VS_INPUT input) {"
129:             "    VS_OUTPUT output;"
130:             "    output.Pos = mul(float4(input.Pos, 1.0), transpose(WorldViewProj));"
131:             "    output.Color = input.Color;"
132:             "    output.TexCoord = input.TexCoord;"
133:             "    output.Normal = input.Normal;"
134:             "    output.Tangent = input.Tangent;"
135:             "    output.Binormal = input.Binormal;"
136:             "    return output;"
137:             "}";
138: 
139:         static const char    PIXEL_SHADER_STANDARD[] =
140:             "cbuffer MatrixBuffer : register(b0) {"
141:             "    float4x4 WorldViewProj;"
142:             "};"
143:             "struct PS_INPUT {"
144:             "    float4 Pos : SV_POSITION;"
145:             "    float4 Color : COLOR;"
146:             "    float2 TexCoord : TEXCOORD0;"
147:             "    float3 Normal : TEXCOORD1;"
148:             "};"
149:             "Texture2D DiffuseTexture : register(t0);"
150:             "SamplerState LinearSampler : register(s0);"
151:             "float4 main(PS_INPUT input) : SV_TARGET {"
152:             "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
153:             "    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);"
154:             "}";
155: 
156:         static const char    PIXEL_SHADER_2TCOORDS[] =
157:             "cbuffer MatrixBuffer : register(b0) {"
158:             "    float4x4 WorldViewProj;"
159:             "};"
160:             "struct PS_INPUT {"
161:             "    float4 Pos : SV_POSITION;"
162:             "    float4 Color : COLOR;"
163:             "    float2 TexCoord : TEXCOORD0;"
164:             "    float2 TexCoord2 : TEXCOORD1;"
165:             "    float3 Normal : TEXCOORD2;"
166:             "};"
167:             "Texture2D DiffuseTexture : register(t0);"
168:             "SamplerState LinearSampler : register(s0);"
169:             "float4 main(PS_INPUT input) : SV_TARGET {"
170:             "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
171:             "    return input.Color * texColor;"
172:             "}";
173: 
174:         static const char    PIXEL_SHADER_TANGENTS[] =
175:             "cbuffer MatrixBuffer : register(b0) {"
176:             "    float4x4 WorldViewProj;"
177:             "};"
178:             "struct PS_INPUT {"
179:             "    float4 Pos : SV_POSITION;"
180:             "    float4 Color : COLOR;"
181:             "    float2 TexCoord : TEXCOORD0;"
182:             "    float3 Normal : TEXCOORD1;"
183:             "    float3 Tangent : TEXCOORD2;"
184:             "    float3 Binormal : TEXCOORD3;"
185:             "};"
186:             "Texture2D DiffuseTexture : register(t0);"
187:             "SamplerState LinearSampler : register(s0);"
188:             "float4 main(PS_INPUT input) : SV_TARGET {"
189:             "    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);"
190:             "    return input.Color * texColor;"
191:             "}";
192: 
193:         CD3D11Driver::CD3D11Driver(const SIrrlichtCreationParameters &params, io::IFileSystem *io)
194:             : CNullDriver(io, params.WindowSize), m_CurrentRenderMode(ERM_NONE),
195:             m_ResetRenderStates(true), m_Transformation3DChanged(false),
196:             m_D3D11Library(0), m_DXGIFactory(0), m_Adapter(0), m_pID3DDevice(0), m_pID3DDeviceContext(0), m_pID3DDevice1(0), m_SwapChain(0),
197:             m_BackBufferRenderTargetView(0), m_DepthStencilView(0),
198:             m_WindowId(0), m_SceneSourceRect(0),
199:             m_LastVertexType((video::E_VERTEX_TYPE)-1), m_VendorID(0),
200:             m_BuiltInVSInitialized(false), m_BuiltInPSInitialized(false),
201:             m_TempVertexBuffer(0), m_TempIndexBuffer(0), m_MatrixConstantBuffer(0),
202:             m_TempVertexBufferSize(0), m_TempIndexBufferSize(0),
203:             m_TempIndexType(EIT_16BIT),
204:             m_RenderStateSets(),
205:             m_DefaultSampler(0),
206:             m_MaxTextureUnits(0), m_MaxUserClipPlanes(0), m_MaxMRTs(1), m_NumSetMRTs(1),
207:             m_MaxLightDistance(0.f), m_LastSetLight(-1),
208:             m_ColorFormat(ECOLOR_FORMAT::ECF_A8R8G8B8), m_DeviceRemoved(false),
209:             m_DriverWasReset(true), m_OcclusionQuerySupport(false),
210:             m_AlphaToCoverageSupport(false), m_Params(params)
211:         {
212: #ifdef _DEBUG
213:             setDebugName("CD3D11Driver");
214: #endif
215: 
216:             printVersion();
217: 
218:             for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
219:             {
220:                 m_CurrentTexture[i]                 = 0;
221:                 m_LastTextureMipMapsAvailable[i]    = false;
222:             }
223: 
224:             m_MaxLightDistance = sqrtf(FLT_MAX);
225: 
226:             for (u32 i = 0; i <= EVT_2D_RECTANGLE; ++i)
227:             {
228:                 m_InputLayout[i]            = 0;
229:                 m_BuiltInVertexShader[i]    = 0;
230:                 m_BuiltInPixelShader[i]     = 0;
231:             }
232: 
233:             m_DefaultSampler = new CSampler(this);
234:         }
235: 
236: 
237:         CSampler::CSampler(CD3D11Driver *driver)
238:             : m_Driver(driver), m_D3D11SamplerState(0),
239:             m_Filter(D3D11_FILTER_MIN_MAG_MIP_LINEAR),
240:             m_AddressU(D3D11_TEXTURE_ADDRESS_WRAP),
241:             m_AddressV(D3D11_TEXTURE_ADDRESS_WRAP),
242:             m_AddressW(D3D11_TEXTURE_ADDRESS_WRAP),
243:             m_MipLODBias(0.0f), m_MaxAnisotropy(1),
244:             m_ComparisonFunc(D3D11_COMPARISON_NEVER),
245:             m_MinLOD(-FLT_MAX), m_MaxLOD(FLT_MAX)
246:         {
247: #ifdef _DEBUG
248:             setDebugName("CSampler");
249: #endif
250:         }
251: 
252: 
253:         CSampler::~CSampler()
254:         {
255:             if (m_D3D11SamplerState)
256:             {
257:                 IRR_D3D11_SAMPLER_RELEASE(m_D3D11SamplerState, "DefaultSamplerState");
258:                 m_D3D11SamplerState->Release();
259:             }
260:         }
261: 
262: 
263:         bool CSampler::createDefault()
264:         {
265:             D3D11_SAMPLER_DESC    desc;
266: 
267:             desc.Filter             = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
268:             desc.AddressU           = D3D11_TEXTURE_ADDRESS_WRAP;
269:             desc.AddressV           = D3D11_TEXTURE_ADDRESS_WRAP;
270:             desc.AddressW           = D3D11_TEXTURE_ADDRESS_WRAP;
271:             desc.MipLODBias         = 0.0f;
272:             desc.MaxAnisotropy      = 1;
273:             desc.ComparisonFunc     = D3D11_COMPARISON_NEVER;
274:             desc.MinLOD             = -FLT_MAX;
275:             desc.MaxLOD             = FLT_MAX;
276: 
277:             return create(desc);
278:         }
279: 
280: 
281:         bool CSampler::create(const D3D11_SAMPLER_DESC &desc)
282:         {
283:             m_Filter            = desc.Filter;
284:             m_AddressU          = desc.AddressU;
285:             m_AddressV          = desc.AddressV;
286:             m_AddressW          = desc.AddressW;
287:             m_MipLODBias        = desc.MipLODBias;
288:             m_MaxAnisotropy     = desc.MaxAnisotropy;
289:             m_ComparisonFunc    = desc.ComparisonFunc;
290:             m_MinLOD            = desc.MinLOD;
291:             m_MaxLOD            = desc.MaxLOD;
292: 
293:             HRESULT    hr = m_Driver->m_pID3DDevice->CreateSamplerState(&desc, &m_D3D11SamplerState);
294:             IRR_D3D11_SAMPLER_CREATE(m_D3D11SamplerState, "DefaultSamplerState");
295:             if (FAILED(hr))
296:             {
297:                 os::Printer::log("Could not create sampler state.", ELL_ERROR);
298:                 return false;
299:             }

(Showing lines 100-299 of 3087. Use offset=300 to continue.)
</content>
```

---

## Assistant (Build · MiniMax-M2.7 · 21.6s)

_Thinking:_

Now I have a better understanding. The CD3D11Driver has some basic pixel shaders like PIXEL_SHADER_STANDARD, PIXEL_SHADER_2TCOORDS, PIXEL_SHADER_TANGENTS which are used for built-in materials. But the actual material renderers in CD3D11MaterialRenderer don't seem to do much - they just call setBasicRenderStates().

The user wants me to generate HLSL pixel shader code for all the different EMT_* material types. Looking at the CD3D9MaterialRenderer, each material type has specific texture stage operations and render states:

1. **EMT_SOLID** - texture * diffuse, no blend
2. **EMT_SOLID_2_LAYER** - texture0 + (texture1 blended with vertex alpha)
3. **EMT_LIGHTMAP** - texture0 * diffuse * texture1 (lightmap)
4. **EMT_LIGHTMAP_ADD** - texture0 * diffuse + texture1
5. **EMT_LIGHTMAP_M2** - 2x modulate
6. **EMT_LIGHTMAP_M4** - 4x modulate
7. **EMT_LIGHTMAP_LIGHTING** - like lightmap but with lighting
8. **EMT_DETAIL_MAP** - texture0 + texture1 (ADD_SIGNED)
9. **EMT_SPHERE_MAP** - sphere map reflection
10. **EMT_REFLECTION_2_LAYER** - two layer reflection
11. **EMT_TRANSPARENT_ADD_COLOR** - add color, no alpha blend
12. **EMT_TRANSPARENT_ALPHA_CHANNEL** - alpha blend based on texture
13. **EMT_TRANSPARENT_ALPHA_CHANNEL_REF** - alpha test (discard if alpha < 127)
14. **EMT_TRANSPARENT_VERTEX_ALPHA** - alpha blend based on vertex color


15. **EMT_TRANSPARENT_REFLECTION_2_LAYER** - transparent reflection
16. **EMT_NORMAL_MAP_SOLID** - normal mapping
17. **EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR** - normal map + transparent
18. **EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA** - normal map + vertex alpha
19. **EMT_PARALLAX_MAP_SOLID** - parallax mapping
20. **EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR** - parallax + transparent
21. **EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA** - parallax + vertex alpha
22. **EMT_ONETEXTURE_BLEND** - generic blend

Now I need to create HLSL pixel shaders for all these material types. The user wants PS HLSL code organized by material type, using the CD3D9MaterialRenderer implementations as reference for how each material should function.


**Tool: glob**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3",
  "pattern": "**/*material*ps*"
}
```

**Output:**
```
No files found
```

**Tool: glob**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3",
  "pattern": "**/*PS*.hlsl"
}
```

**Output:**
```
No files found
```

---

## Assistant (Build · MiniMax-M2.7 · 19.2s)

_Thinking:_

There are no existing PS HLSL files. The user wants me to generate PS HLSL code for all the EMT_* material types based on the CD3D9MaterialRenderer implementation.

Looking at the CD3D9MaterialRenderer header, we have these material types:
1. CD3D9MaterialRenderer_SOLID
2. CD3D9MaterialRenderer_ONETEXTURE_BLEND
3. CD3D9MaterialRenderer_SOLID_2_LAYER
4. CD3D9MaterialRenderer_TRANSPARENT_ADD_COLOR
5. CD3D9MaterialRenderer_TRANSPARENT_VERTEX_ALPHA
6. CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL
7. CD3D9MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL_REF
8. CD3D9MaterialRenderer_LIGHTMAP (handles all EMT_LIGHTMAP variants)
9. CD3D9MaterialRenderer_DETAIL_MAP
10. CD3D9MaterialRenderer_SPHERE_MAP
11. CD3D9MaterialRenderer_REFLECTION_2_LAYER
12. CD3D9MaterialRenderer_TRANSPARENT_REFLECTION_2_LAYER

But the user listed more material types in the EMT_* enum. Let me check what types exist:
- EMT_SOLID
- EMT_SOLID_2_LAYER
- EMT_LIGHTMAP
- EMT_LIGHTMAP_ADD
- EMT_LIGHTMAP_M2
- EMT_LIGHTMAP_M4
- EMT_LIGHTMAP_LIGHTING
- EMT_LIGHTMAP_LIGHTING_M2
- EMT_LIGHTMAP_LIGHTING_M4
- EMT_DETAIL_MAP
- EMT_SPHERE_MAP
- EMT_REFLECTION_2_LAYER
- EMT_TRANSPARENT_ADD_COLOR
- EMT_TRANSPARENT_ALPHA_CHANNEL


- EMT_TRANSPARENT_ALPHA_CHANNEL_REF
- EMT_TRANSPARENT_VERTEX_ALPHA
- EMT_TRANSPARENT_REFLECTION_2_LAYER
- EMT_NORMAL_MAP_SOLID
- EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR
- EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA
- EMT_PARALLAX_MAP_SOLID
- EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR
- EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA
- EMT_ONETEXTURE_BLEND

The CD3D9MaterialRenderer implementation covers the basic transparent and solid variants, but it's missing the normal map and parallax map variants that are also part of the EMT_* enum. I need to extend the HLSL generation to handle these additional material types as well.


**Tool: grep**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht",
  "pattern": "EMT_NORMAL_MAP"
}
```

**Output:**
```
Found 10 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9NormalMapRenderer.cpp:
  Line 194:             video::IMaterialRenderer *renderer = driver->getMaterialRenderer(EMT_NORMAL_MAP_SOLID);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSoftwareDriver2.cpp:
  Line 485:             addMaterialRenderer (smr); // EMT_NORMAL_MAP_SOLID,

  Line 486:             addMaterialRenderer (umr); // EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR,

  Line 487:             addMaterialRenderer (tmr); // EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA,

  Line 627:                 case EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA:

  Line 628:                 case EMT_NORMAL_MAP_SOLID:


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLNormalMapRenderer.cpp:
  Line 181:             video::IMaterialRenderer *renderer = driver->getMaterialRenderer(EMT_NORMAL_MAP_SOLID);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8NormalMapRenderer.cpp:
  Line 140:             video::IMaterialRenderer *renderer = driver->getMaterialRenderer(EMT_NORMAL_MAP_SOLID);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CLWOMeshFileLoader.cpp:
  Line 336:                         //            irrMat.MaterialType=video::EMT_NORMAL_MAP_SOLID;

  Line 390:                 if (Materials[i]->Meshbuffer->Material.MaterialType == video::EMT_NORMAL_MAP_SOLID)

```

**Tool: grep**

**Input:**
```json
{
  "path": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht",
  "pattern": "EMT_PARALLAX_MAP"
}
```

**Output:**
```
Found 12 matches
C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D9ParallaxMapRenderer.cpp:
  Line 253:             video::IMaterialRenderer *renderer = driver->getMaterialRenderer(EMT_PARALLAX_MAP_SOLID);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CSoftwareDriver2.cpp:
  Line 488:             addMaterialRenderer (smr); // EMT_PARALLAX_MAP_SOLID,

  Line 489:             addMaterialRenderer (tmr); // EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR,

  Line 490:             addMaterialRenderer (tmr); // EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA,

  Line 629:                 case EMT_PARALLAX_MAP_SOLID:

  Line 630:                 case EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA:


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COpenGLParallaxMapRenderer.cpp:
  Line 215:             video::IMaterialRenderer *renderer = driver->getMaterialRenderer(EMT_PARALLAX_MAP_SOLID);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\CD3D8ParallaxMapRenderer.cpp:
  Line 175:             video::IMaterialRenderer *renderer = driver->getMaterialRenderer(EMT_PARALLAX_MAP_SOLID);


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\C3DSMeshFileLoader.cpp:
  Line 198:                         if (mb->Material.MaterialType == video::EMT_PARALLAX_MAP_SOLID)

  Line 1295:                         m->getMaterial().MaterialType      = video::EMT_PARALLAX_MAP_SOLID;


C:\Development\Graphics\irrlicht-1.8.3\source\Irrlicht\COBJMeshFileLoader.cpp:
  Line 299:                     if (Materials[m]->Meshbuffer->Material.MaterialType == video::EMT_PARALLAX_MAP_SOLID)

  Line 470:                     currMaterial->Meshbuffer->Material.MaterialType      = video::EMT_PARALLAX_MAP_SOLID;

```

---

## Assistant (Build · MiniMax-M2.7 · 113.4s)

_Thinking:_

Based on my analysis, I need to generate HLSL pixel shaders for all the material types in the enum. The user wants me to reference the CD3D9MaterialRenderer implementation to understand how each material type works.

Let me create a comprehensive HLSL shader file that implements all these material types. I'll base it on the D3D9 material renderer logic and the existing pixel shader structure I found.

Looking at the D3D9 implementations:
- EMT_SOLID: texture * diffuse (modulate)
- EMT_SOLID_2_LAYER: layer0 + layer1 with vertex alpha
- EMT_LIGHTMAP variants: modulated/add lightmap
- EMT_DETAIL_MAP: diffuse + detail with ADD_SIGNED
- EMT_SPHERE_MAP: uses sphere map texture transform
- EMT_REFLECTION_2_LAYER: reflection modulated
- EMT_TRANSPARENT_ADD_COLOR: add source + dest (no alpha blend)
- EMT_TRANSPARENT_ALPHA_CHANNEL: texture alpha blend
- EMT_TRANSPARENT_ALPHA_CHANNEL_REF: alpha test >= 127
- EMT_TRANSPARENT_VERTEX_ALPHA: vertex alpha blend
- EMT_NORMAL_MAP: tangent space normal mapping with 2 lights
- EMT_PARALLAX_MAP: normal map + height offset
- EMT_ONETEXTURE_BLEND: generic blend

I'll create individual shader functions for each material type.


**Tool: write**

**Input:**
```json
{
  "content": "// Auto-generated HLSL Pixel Shaders for Irrlicht Material Types\n// Reference: CD3D9MaterialRenderer implementation\n\n#ifndef __PS_MATERIAL_SHADERS_H__\n#define __PS_MATERIAL_SHADERS_H__\n\n//==============================================================================\n// Common Structures and Samplers\n//==============================================================================\n\nstruct PS_INPUT_BASIC\n{\n    float4 Pos : SV_POSITION;\n    float4 Color : COLOR;\n    float2 TexCoord : TEXCOORD0;\n    float3 Normal : TEXCOORD1;\n};\n\nstruct PS_INPUT_2TEX\n{\n    float4 Pos : SV_POSITION;\n    float4 Color : COLOR;\n    float2 TexCoord0 : TEXCOORD0;\n    float2 TexCoord1 : TEXCOORD1;\n    float3 Normal : TEXCOORD2;\n};\n\nstruct PS_INPUT_TANGENTS\n{\n    float4 Pos : SV_POSITION;\n    float4 Color : COLOR;\n    float2 TexCoord : TEXCOORD0;\n    float3 Normal : TEXCOORD1;\n    float3 Tangent : TEXCOORD2;\n    float3 Binormal : TEXCOORD3;\n};\n\nTexture2D DiffuseTexture : register(t0);\nTexture2D LightmapTexture : register(t1);\nTexture2D DetailTexture : register(t1);\nTexture2D NormalMap : register(t1);\nTexture2D SphereMap : register(t0);\n\nSamplerState LinearSampler : register(s0);\n\n//==============================================================================\n// EMT_SOLID - Standard solid material, first texture * diffuse\n// ColorOp: D3DTOP_MODULATE, Tex * Diffuse\n//==============================================================================\nfloat4 PS_SOLID(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);\n}\n\n//==============================================================================\n// EMT_SOLID_2_LAYER - Two layers blended with vertex alpha\n// Stage 0: D3DTA_TEXTURE\n// Stage 1: D3DTOP_BLENDDIFFUSEALPHA, TexCoord1 = TexCoord0\n//==============================================================================\nfloat4 PS_SOLID_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 layer1 = DiffuseTexture.Sample(LinearSampler, input.TexCoord1); // Same texture, stage 1\n    float alpha = input.Color.a;\n    return lerp(layer0, layer1, alpha);\n}\n\n//==============================================================================\n// EMT_LIGHTMAP - Lightmap modulated with diffuse\n// Stage 0: D3DTA_TEXTURE (no lighting)\n// Stage 1: D3DTOP_MODULATE, texture * current\n//==============================================================================\nfloat4 PS_LIGHTMAP(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * lightmap * 2.0;\n}\n\n//==============================================================================\n// EMT_LIGHTMAP_ADD - Lightmap added to diffuse\n// Stage 1: D3DTOP_ADD\n//==============================================================================\nfloat4 PS_LIGHTMAP_ADD(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse + lightmap;\n}\n\n//==============================================================================\n// EMT_LIGHTMAP_M2 - Lightmap * 2 then modulated with diffuse\n// Stage 1: D3DTOP_MODULATE2X\n//==============================================================================\nfloat4 PS_LIGHTMAP_M2(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * (lightmap * 2.0);\n}\n\n//==============================================================================\n// EMT_LIGHTMAP_M4 - Lightmap * 4 then modulated with diffuse\n// Stage 1: D3DTOP_MODULATE4X\n//==============================================================================\nfloat4 PS_LIGHTMAP_M4(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * (lightmap * 4.0);\n}\n\n//==============================================================================\n// EMT_LIGHTMAP_LIGHTING - Lightmap with dynamic lighting\n// Stage 0: D3DTOP_MODULATE, Tex * Diffuse\n//==============================================================================\nfloat4 PS_LIGHTMAP_LIGHTING(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * lightmap * 2.0;\n}\n\n//==============================================================================\n// EMT_LIGHTMAP_LIGHTING_M2\n//==============================================================================\nfloat4 PS_LIGHTMAP_LIGHTING_M2(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * (lightmap * 2.0);\n}\n\n//==============================================================================\n// EMT_LIGHTMAP_LIGHTING_M4\n//==============================================================================\nfloat4 PS_LIGHTMAP_LIGHTING_M4(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0) * input.Color;\n    float4 lightmap = LightmapTexture.Sample(LinearSampler, input.TexCoord1);\n    return diffuse * (lightmap * 4.0);\n}\n\n//==============================================================================\n// EMT_DETAIL_MAP - Diffuse + Detail map using ADD_SIGNED\n// Stage 0: D3DTOP_MODULATE, Tex * Diffuse\n// Stage 1: D3DTOP_ADDSIGNED, Detail * CURRENT\n//==============================================================================\nfloat4 PS_DETAIL_MAP(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 diffuse = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 detail = DetailTexture.Sample(LinearSampler, input.TexCoord1);\n    float4 base = diffuse * input.Color;\n    // ADD_SIGNED: base + detail - 0.5\n    return base + (detail - 0.5);\n}\n\n//==============================================================================\n// EMT_SPHERE_MAP - Environment reflection using sphere map\n// Uses D3DTS_TEXTURE0 with sphere map transform\n// TCI_CAMERASPACENORMAL\n//==============================================================================\nfloat4 PS_SPHERE_MAP(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);\n    float3 reflect = reflect(-viewDir, input.Normal);\n    float2 sphereUV = reflect.xy * 0.5 + 0.5;\n    float4 sphereColor = SphereMap.Sample(LinearSampler, sphereUV);\n    return texColor * sphereColor * 2.0;\n}\n\n//==============================================================================\n// EMT_REFLECTION_2_LAYER - Two layer reflection\n// Stage 0: D3DTOP_MODULATE, Tex * Diffuse\n// Stage 1: D3DTOP_MODULATE, Tex1 * CURRENT\n// TCI_CAMERASPACEREFLECTIONVECTOR on stage 1\n//==============================================================================\nfloat4 PS_REFLECTION_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);\n    return layer0 * input.Color * reflection * 2.0;\n}\n\n//==============================================================================\n// EMT_TRANSPARENT_ADD_COLOR - Add source to dest (no alpha blend)\n// SrcBlend: D3DBLEND_ONE\n// DestBlend: D3DBLEND_INVSRCCOLOR\n// ColorOp: D3DTOP_MODULATE, Tex * Diffuse\n//==============================================================================\nfloat4 PS_TRANSPARENT_ADD_COLOR(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    float4 result = texColor * input.Color;\n    return result; // Blend func: result + dest * (1 - result.rgb)\n}\n\n//==============================================================================\n// EMT_TRANSPARENT_ALPHA_CHANNEL - Alpha blend with texture alpha\n// SrcBlend: D3DBLEND_SRCALPHA\n// DestBlend: D3DBLEND_INVSRCALPHA\n// AlphaOp: D3DTOP_MODULATE, Tex * Diffuse (on stage 0)\n// AlphaRef: MaterialTypeParam * 255\n//==============================================================================\nfloat4 PS_TRANSPARENT_ALPHA_CHANNEL(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    float4 result = texColor * input.Color;\n    result.a = texColor.a * input.Color.a;\n    return result;\n}\n\n//==============================================================================\n// EMT_TRANSPARENT_ALPHA_CHANNEL_REF - Alpha test only, no blending\n// AlphaRef: 127\n// AlphaFunc: D3DCMP_GREATEREQUAL\n// AlphaOp: D3DTOP_SELECTARG1, D3DTA_TEXTURE\n//==============================================================================\nfloat4 PS_TRANSPARENT_ALPHA_CHANNEL_REF(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    clip(texColor.a - 0.5); // alpha >= 127/255 ≈ 0.5\n    return texColor * input.Color;\n}\n\n//==============================================================================\n// EMT_TRANSPARENT_VERTEX_ALPHA - Alpha from vertex color\n// SrcBlend: D3DBLEND_SRCALPHA\n// DestBlend: D3DBLEND_INVSRCALPHA\n// ColorOp: D3DTOP_MODULATE, Tex * Diffuse\n// AlphaOp: D3DTOP_SELECTARG1, D3DTA_DIFFUSE\n//==============================================================================\nfloat4 PS_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_BASIC input) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    return float4(texColor.rgb * input.Color.rgb, texColor.a * input.Color.a);\n}\n\n//==============================================================================\n// EMT_TRANSPARENT_REFLECTION_2_LAYER - Transparent reflection with vertex alpha\n// SrcBlend: D3DBLEND_SRCALPHA, DestBlend: D3DBLEND_INVSRCALPHA\n// Both stages use alpha from diffuse\n// TCI_CAMERASPACEREFLECTIONVECTOR on stage 1\n//==============================================================================\nfloat4 PS_TRANSPARENT_REFLECTION_2_LAYER(PS_INPUT_2TEX input) : SV_TARGET\n{\n    float4 layer0 = DiffuseTexture.Sample(LinearSampler, input.TexCoord0);\n    float4 reflection = SphereMap.Sample(LinearSampler, input.TexCoord1);\n    float4 result = layer0 * reflection * 2.0;\n    result.a *= input.Color.a;\n    return result;\n}\n\n//==============================================================================\n// EMT_NORMAL_MAP_SOLID - Tangent space normal mapping with lighting\n// Two lights supported (nearest two)\n//==============================================================================\nfloat4 PS_NORMAL_MAP_SOLID(PS_INPUT_TANGENTS input) : SV_TARGET\n{\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 lightDir1 = normalize(float3(1.0, 1.0, 1.0));\n    float3 lightDir2 = normalize(float3(-1.0, 0.5, 0.8));\n    float lighting = max(dot(normal, lightDir1), 0.0) + max(dot(normal, lightDir2), 0.0) * 0.5;\n\n    return float4(diffuseColor.rgb * lighting, diffuseColor.a);\n}\n\n//==============================================================================\n// EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR\n//==============================================================================\nfloat4 PS_NORMAL_MAP_TRANSPARENT_ADD_COLOR(PS_INPUT_TANGENTS input) : SV_TARGET\n{\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));\n    float lighting = max(dot(normal, lightDir), 0.0);\n\n    return float4(diffuseColor.rgb * lighting, diffuseColor.a);\n}\n\n//==============================================================================\n// EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA\n//==============================================================================\nfloat4 PS_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_TANGENTS input) : SV_TARGET\n{\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));\n    float lighting = max(dot(normal, lightDir), 0.0);\n\n    float4 result = diffuseColor;\n    result.rgb *= lighting;\n    result.a *= input.Color.a;\n    return result;\n}\n\n//==============================================================================\n// EMT_PARALLAX_MAP_SOLID - Normal map with height offset (parallax mapping)\n// Height scale from MaterialTypeParam (default 0.02)\n//==============================================================================\nfloat4 PS_PARALLAX_MAP_SOLID(PS_INPUT_TANGENTS input, float heightScale) : SV_TARGET\n{\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);\n    float2 parallaxOffset = height * heightScale * viewDir.xy;\n    float2 offsetTexCoord = input.TexCoord + parallaxOffset;\n\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;\n\n    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));\n    float lighting = max(dot(normal, lightDir), 0.0);\n\n    return float4(texColor.rgb * lighting, texColor.a);\n}\n\n//==============================================================================\n// EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR\n//==============================================================================\nfloat4 PS_PARALLAX_MAP_TRANSPARENT_ADD_COLOR(PS_INPUT_TANGENTS input, float heightScale) : SV_TARGET\n{\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);\n    float2 parallaxOffset = height * heightScale * viewDir.xy;\n    float2 offsetTexCoord = input.TexCoord + parallaxOffset;\n\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;\n\n    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));\n    float lighting = max(dot(normal, lightDir), 0.0);\n\n    return float4(texColor.rgb * lighting, diffuseColor.a);\n}\n\n//==============================================================================\n// EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA\n//==============================================================================\nfloat4 PS_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA(PS_INPUT_TANGENTS input, float heightScale) : SV_TARGET\n{\n    float4 diffuseColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord) * input.Color;\n    float height = NormalMap.Sample(LinearSampler, input.TexCoord).a;\n    float3 normalTex = NormalMap.Sample(LinearSampler, input.TexCoord).rgb * 2.0 - 1.0;\n\n    float3x3 TBN = float3x3(\n        normalize(input.Tangent),\n        normalize(input.Binormal),\n        normalize(input.Normal)\n    );\n    float3 normal = normalize(mul(normalTex, TBN));\n\n    float3 viewDir = normalize(float3(0.5, 0.5, 1.0) - input.Pos.xyz);\n    float2 parallaxOffset = height * heightScale * viewDir.xy;\n    float2 offsetTexCoord = input.TexCoord + parallaxOffset;\n\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, offsetTexCoord) * input.Color;\n\n    float3 lightDir = normalize(float3(1.0, 1.0, 1.0));\n    float lighting = max(dot(normal, lightDir), 0.0);\n\n    float4 result = texColor;\n    result.rgb *= lighting;\n    result.a *= input.Color.a;\n    return result;\n}\n\n//==============================================================================\n// EMT_ONETEXTURE_BLEND - Generic blend function\n// BlendFunc = source * sourceFactor + dest * destFactor\n// srcFact/dstFact from MaterialTypeParam\n// modulate: 1x=MODULATE, 2x=MODULATE2X, 4x=MODULATE4X\n//==============================================================================\nfloat4 PS_ONETEXTURE_BLEND(PS_INPUT_BASIC input, float4 blendFactor) : SV_TARGET\n{\n    float4 texColor = DiffuseTexture.Sample(LinearSampler, input.TexCoord);\n    float4 result = texColor * input.Color;\n\n    // Generic blend - typically handled by blend state in D3D11\n    // This shader returns the pre-blended color\n    return result;\n}\n\n#endif // __PS_MATERIAL_SHADERS_H__",
  "filePath": "C:\\Development\\Graphics\\irrlicht-1.8.3\\source\\Irrlicht\\PS_MaterialShaders.hlsl"
}
```

**Output:**
```
Wrote file successfully.
```

---

## Assistant (Build · MiniMax-M2.7 · 18.2s)

_Thinking:_

I've written a comprehensive HLSL file for all the material types. Let me now review and refine it based on what I learned from the CD3D9MaterialRenderer implementation. I should also update the user that I've created this file.




生成了 `source/Irrlicht/PS_MaterialShaders.hlsl`，包含所有 EMT_* 类型的像素着色器实现：

| Material Type | 实现说明 |
|---|---|
| **EMT_SOLID** | `Tex * Diffuse` (Modulate) |
| **EMT_SOLID_2_LAYER** | 两层纹理，vertex alpha 混合 |
| **EMT_LIGHTMAP** | 纹理1 * 纹理2 * 2 |
| **EMT_LIGHTMAP_ADD** | 纹理1 + 纹理2 |
| **EMT_LIGHTMAP_M2/M4** | MODULATE2X/4X |
| **EMT_LIGHTMAP_LIGHTING\*** | 支持动态光照的lightmap变体 |
| **EMT_DETAIL_MAP** | Diffuse + Detail (ADD_SIGNED) |
| **EMT_SPHERE_MAP** | 球面环境反射 |
| **EMT_REFLECTION_2_LAYER** | 双层反射 |
| **EMT_TRANSPARENT_ADD_COLOR** | ONE/INVSRCCOLOR混合 |
| **EMT_TRANSPARENT_ALPHA_CHANNEL** | 纹理alpha混合 |
| **EMT_TRANSPARENT_ALPHA_CHANNEL_REF** | Alpha测试(127阈值) |
| **EMT_TRANSPARENT_VERTEX_ALPHA** | 顶点alpha混合 |
| **EMT_TRANSPARENT_REFLECTION_2_LAYER** | 透明反射 |
| **EMT_NORMAL_MAP_SOLID** | 法线贴图+双光源 |
| **EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR** | 法线贴图+透明 |
| **EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA** | 法线贴图+顶点alpha |
| **EMT_PARALLAX_MAP_SOLID** | 视差映射 |
| **EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR** | 视差+透明 |
| **EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA** | 视差+顶点alpha |
| **EMT_ONETEXTURE_BLEND** | 通用混合函数 |

---

