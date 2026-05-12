// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __E_MATERIAL_TYPES_H_INCLUDED__
#define __E_MATERIAL_TYPES_H_INCLUDED__

namespace irr
{
    namespace video
    {
        //! Abstracted and easy to use fixed function/programmable pipeline material modes.
        enum E_MATERIAL_TYPE
        {
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

            //! A reflecting material with an optional non reflecting texture layer, with lighting.
            /** The reflection map should be set as first texture. */
            EMT_REFLECTION_2_LAYER_WITH_LIGHT,

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

            //! solid without texture, color only.
            EMT_SOLID_COLOR,

            //! 2tex coordinates but there is one texture.
            EMT_SOLID_1_LAYER,

            //! solid with lighting
            EMT_SOLID_WITH_LIGHT,

            //! Solid material with Gouraud shading for lighting interpolation.
            EMT_SOLID_LIGHTING_GOURAUD,

            //! Solid material with Flat shading (constant per face lighting).
            EMT_SOLID_LIGHTING_FLAT,

            //! Just make PS changing if using 2d rectangle shader
            EMT_2D_RECTANGLE,

            //! The max value of material.
            EMT_MATERIAL_MAX,

            //! This value is not used. It only forces this enumeration to compile to 32 bit.
            EMT_FORCE_32BIT = 0x7fffffff
        };

        //! Array holding the built in material type names
        const char* const sBuiltInMaterialTypeNames[] =
        {
            "solid",
            "solid_2layer",
            "lightmap",
            "lightmap_add",
            "lightmap_m2",
            "lightmap_m4",
            "lightmap_light",
            "lightmap_light_m2",
            "lightmap_light_m4",
            "detail_map",
            "sphere_map",
            "reflection_2layer",
            "trans_add",
            "trans_alphach",
            "trans_alphach_ref",
            "trans_vertex_alpha",
            "trans_reflection_2layer",
            "normalmap_solid",
            "normalmap_trans_add",
            "normalmap_trans_vertexalpha",
            "parallaxmap_solid",
            "parallaxmap_trans_add",
            "parallaxmap_trans_vertexalpha",
            "onetexture_blend",
            "solid_1layer",
            "solid_lighting_gouraud",
            "solid_lighting_flat",
            0
        };

        static inline const c8* getMaterialTypeName(E_MATERIAL_TYPE materialType)
        {
            switch (materialType)
            {
                case EMT_SOLID: return "EMT_SOLID";
                case EMT_SOLID_2_LAYER: return "EMT_SOLID_2_LAYER";
                case EMT_LIGHTMAP: return "EMT_LIGHTMAP";
                case EMT_LIGHTMAP_ADD: return "EMT_LIGHTMAP_ADD";
                case EMT_LIGHTMAP_M2: return "EMT_LIGHTMAP_M2";
                case EMT_LIGHTMAP_M4: return "EMT_LIGHTMAP_M4";
                case EMT_LIGHTMAP_LIGHTING: return "EMT_LIGHTMAP_LIGHTING";
                case EMT_LIGHTMAP_LIGHTING_M2: return "EMT_LIGHTMAP_LIGHTING_M2";
                case EMT_LIGHTMAP_LIGHTING_M4: return "EMT_LIGHTMAP_LIGHTING_M4";
                case EMT_DETAIL_MAP: return "EMT_DETAIL_MAP";
                case EMT_SPHERE_MAP: return "EMT_SPHERE_MAP";
                case EMT_REFLECTION_2_LAYER: return "EMT_REFLECTION_2_LAYER";
                case EMT_REFLECTION_2_LAYER_WITH_LIGHT: return "EMT_REFLECTION_2_LAYER_WITH_LIGHT";
                case EMT_TRANSPARENT_ADD_COLOR: return "EMT_TRANSPARENT_ADD_COLOR";
                case EMT_TRANSPARENT_ALPHA_CHANNEL: return "EMT_TRANSPARENT_ALPHA_CHANNEL";
                case EMT_TRANSPARENT_ALPHA_CHANNEL_REF: return "EMT_TRANSPARENT_ALPHA_CHANNEL_REF";
                case EMT_TRANSPARENT_VERTEX_ALPHA: return "EMT_TRANSPARENT_VERTEX_ALPHA";
                case EMT_TRANSPARENT_REFLECTION_2_LAYER: return "EMT_TRANSPARENT_REFLECTION_2_LAYER";
                case EMT_NORMAL_MAP_SOLID: return "EMT_NORMAL_MAP_SOLID";
                case EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR: return "EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR";
                case EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA: return "EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA";
                case EMT_PARALLAX_MAP_SOLID: return "EMT_PARALLAX_MAP_SOLID";
                case EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR: return "EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR";
                case EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA: return "EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA";
                case EMT_ONETEXTURE_BLEND: return "EMT_ONETEXTURE_BLEND";
                case EMT_SOLID_1_LAYER: return "EMT_SOLID_1_LAYER";
                case EMT_SOLID_COLOR: return "EMT_SOLID_COLOR";
                case EMT_SOLID_LIGHTING_GOURAUD: return "EMT_SOLID_LIGHTING_GOURAUD";
                case EMT_SOLID_LIGHTING_FLAT: return "EMT_SOLID_LIGHTING_FLAT";
                case EMT_2D_RECTANGLE: return "EMT_2D_RECTANGLE";
                case EMT_SOLID_WITH_LIGHT: return "EMT_SOLID_WITH_LIGHT";
                default: return "Unknown";
            }
        }
    } // end namespace video
} // end namespace irr
#endif // __E_MATERIAL_TYPES_H_INCLUDED__