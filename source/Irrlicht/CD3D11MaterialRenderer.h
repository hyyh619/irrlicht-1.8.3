// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__
#define __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include "IMaterialRenderer.h"
#include "SColor.h"

namespace irr
{
    namespace video
    {
        class CD3D11Driver;

        class CD3D11MaterialRenderer : public IMaterialRenderer
        {
public:

            CD3D11MaterialRenderer(CD3D11Driver *driver, s32 &materialType,
                                   const c8 *name);

            virtual ~CD3D11MaterialRenderer();

            virtual void OnSetMaterial(const SMaterial &material);
            virtual bool OnSetTexture(u32 textureIndex, ITexture *texture);
            virtual void OnSetConstants(IMaterialRendererServices *services, s32 userData);
            virtual void PostRender();

protected:

            CD3D11Driver    *Driver;
            s32             &MaterialType;
        };

        class CD3D11MaterialRenderer_SOLID : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_SOLID(CD3D11Driver *p, video::IVideoDriver *d)
                : CD3D11MaterialRenderer(p, matType, "solid") { matType = -1; }

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);
        };

        class CD3D11MaterialRenderer_SOLID_2_LAYER : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_SOLID_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d)
                : CD3D11MaterialRenderer(p, matType, "solid_2_layer") { matType = -1; }

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);
        };

        class CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR(CD3D11Driver *p, video::IVideoDriver *d)
                : CD3D11MaterialRenderer(p, matType, "transparent_add_color") { matType = -1; }

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual bool isTransparent() const;
        };

        class CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(CD3D11Driver *p, video::IVideoDriver *d)
                : CD3D11MaterialRenderer(p, matType, "transparent_vertex_alpha") { matType = -1; }

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual bool isTransparent() const;
        };

        class CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(CD3D11Driver *p, video::IVideoDriver *d)
                : CD3D11MaterialRenderer(p, matType, "transparent_alpha_channel") { matType = -1; }

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual void OnUnsetMaterial();

            virtual bool isTransparent() const;
        };

        class CD3D11MaterialRenderer_ONETEXTURE_BLEND : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_ONETEXTURE_BLEND(CD3D11Driver *p, video::IVideoDriver *d)
                : CD3D11MaterialRenderer(p, matType, "one_texture_blend") { matType = -1; }

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual bool isTransparent() const;

        private:
            u32 getD3D11Blend(E_BLEND_FACTOR factor) const;
            u32 getD3D11Modulate(E_MODULATE_FUNC func) const;
            bool transparent;
        };

        class CD3D11MaterialRenderer_LIGHTMAP : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_LIGHTMAP(CD3D11Driver *p, video::IVideoDriver *d)
                : CD3D11MaterialRenderer(p, matType, "lightmap") { matType = -1; }

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);
        };

        class CD3D11MaterialRenderer_DETAIL_MAP : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_DETAIL_MAP(CD3D11Driver *p, video::IVideoDriver *d)
                : CD3D11MaterialRenderer(p, matType, "detail_map") { matType = -1; }

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);
        };

        class CD3D11MaterialRenderer_SPHERE_MAP : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_SPHERE_MAP(CD3D11Driver *p, video::IVideoDriver *d)
                : CD3D11MaterialRenderer(p, matType, "sphere_map") { matType = -1; }

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual void OnUnsetMaterial();
        };

        class CD3D11MaterialRenderer_REFLECTION_2_LAYER : public CD3D11MaterialRenderer
        {
public:
            CD3D11MaterialRenderer_REFLECTION_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d)
                : CD3D11MaterialRenderer(p, matType, "reflection_2_layer") { matType = -1; }

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);

            virtual void OnUnsetMaterial();
        };

    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_
#endif // __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__