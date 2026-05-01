// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D11MaterialRenderer.h"
#include "CD3D11Driver.h"
#include "IMaterialRendererServices.h"

namespace irr
{
    namespace video
    {
        CD3D11MaterialRenderer::CD3D11MaterialRenderer(CD3D11Driver *driver, s32 &materialType,
                                                       const c8 *name)
            : Driver(driver), MaterialType(materialType)
        {
#ifdef _DEBUG
            setDebugName("CD3D11MaterialRenderer");
#endif
            MaterialType = driver->addMaterialRenderer(this);
        }


        CD3D11MaterialRenderer::~CD3D11MaterialRenderer()
        {}


        void CD3D11MaterialRenderer::OnSetMaterial(const SMaterial &material)
        {
            Driver->setBasicRenderStates(material, LastMaterial, true);
            LastMaterial = material;
        }


        bool CD3D11MaterialRenderer::OnSetTexture(u32 textureIndex, ITexture *texture)
        {
            if (texture)
            {
                CD3D11Texture* tex = (CD3D11Texture*)texture;
                Driver->pID3DDeviceContext->PSSetShaderResources(textureIndex, 1, &tex->ShaderResourceView);
            }
            return true;
        }


        void CD3D11MaterialRenderer::OnSetConstants(IMaterialRendererServices *services, s32 userData)
        {}


        void CD3D11MaterialRenderer::PostRender()
        {}


        CD3D11MaterialRenderer_SOLID::CD3D11MaterialRenderer_SOLID(CD3D11Driver *p, video::IVideoDriver *d)
            : CD3D11MaterialRenderer(p, -1, "solid") {}


        CD3D11MaterialRenderer_SOLID_2_LAYER::CD3D11MaterialRenderer_SOLID_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d)
            : CD3D11MaterialRenderer(p, -1, "solid_2_layer") {}


        CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR::CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR(CD3D11Driver *p, video::IVideoDriver *d)
            : CD3D11MaterialRenderer(p, -1, "transparent_add_color") {}


        CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA::CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA(CD3D11Driver *p, video::IVideoDriver *d)
            : CD3D11MaterialRenderer(p, -1, "transparent_vertex_alpha") {}


        CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL(CD3D11Driver *p, video::IVideoDriver *d)
            : CD3D11MaterialRenderer(p, -1, "transparent_alpha_channel") {}


        CD3D11MaterialRenderer_ONETEXTURE_BLEND::CD3D11MaterialRenderer_ONETEXTURE_BLEND(CD3D11Driver *p, video::IVideoDriver *d)
            : CD3D11MaterialRenderer(p, -1, "one_texture_blend") { transparent = false; }


        CD3D11MaterialRenderer_LIGHTMAP::CD3D11MaterialRenderer_LIGHTMAP(CD3D11Driver *p, video::IVideoDriver *d)
            : CD3D11MaterialRenderer(p, -1, "lightmap") {}


        CD3D11MaterialRenderer_DETAIL_MAP::CD3D11MaterialRenderer_DETAIL_MAP(CD3D11Driver *p, video::IVideoDriver *d)
            : CD3D11MaterialRenderer(p, -1, "detail_map") {}


        CD3D11MaterialRenderer_SPHERE_MAP::CD3D11MaterialRenderer_SPHERE_MAP(CD3D11Driver *p, video::IVideoDriver *d)
            : CD3D11MaterialRenderer(p, -1, "sphere_map") {}


        CD3D11MaterialRenderer_REFLECTION_2_LAYER::CD3D11MaterialRenderer_REFLECTION_2_LAYER(CD3D11Driver *p, video::IVideoDriver *d)
            : CD3D11MaterialRenderer(p, -1, "reflection_2_layer") {}


        void CD3D11MaterialRenderer_SOLID::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        void CD3D11MaterialRenderer_SOLID_2_LAYER::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        void CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        bool CD3D11MaterialRenderer_TRANSPARENT_ADD_COLOR::isTransparent() const
        {
            return true;
        }


        void CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        bool CD3D11MaterialRenderer_TRANSPARENT_VERTEX_ALPHA::isTransparent() const
        {
            return true;
        }


        void CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        void CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::OnUnsetMaterial()
        {
        }


        bool CD3D11MaterialRenderer_TRANSPARENT_ALPHA_CHANNEL::isTransparent() const
        {
            return true;
        }


        void CD3D11MaterialRenderer_ONETEXTURE_BLEND::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);

            E_BLEND_FACTOR srcFact, dstFact;
            E_MODULATE_FUNC modulate;
            u32 alphaSource;
            unpack_textureBlendFunc(srcFact, dstFact, modulate, alphaSource, material.MaterialTypeParam);

            transparent = (srcFact != EBF_SRC_COLOR || dstFact != EBF_ZERO);

            if (srcFact == EBF_SRC_COLOR && dstFact == EBF_ZERO)
            {
            }
            else
            {
            }

            if (textureBlendFunc_hasAlpha(srcFact) || textureBlendFunc_hasAlpha(dstFact))
            {
            }
        }


        bool CD3D11MaterialRenderer_ONETEXTURE_BLEND::isTransparent() const
        {
            return transparent;
        }


        u32 CD3D11MaterialRenderer_ONETEXTURE_BLEND::getD3D11Blend(E_BLEND_FACTOR factor) const
        {
            switch (factor)
            {
                case EBF_ZERO:                return D3D11_BLEND_ZERO;
                case EBF_ONE:                 return D3D11_BLEND_ONE;
                case EBF_DST_COLOR:           return D3D11_BLEND_DEST_COLOR;
                case EBF_ONE_MINUS_DST_COLOR: return D3D11_BLEND_INV_DEST_COLOR;
                case EBF_SRC_COLOR:           return D3D11_BLEND_SRC_COLOR;
                case EBF_ONE_MINUS_SRC_COLOR: return D3D11_BLEND_INV_SRC_COLOR;
                case EBF_SRC_ALPHA:           return D3D11_BLEND_SRC_ALPHA;
                case EBF_ONE_MINUS_SRC_ALPHA: return D3D11_BLEND_INV_SRC_ALPHA;
                case EBF_DST_ALPHA:           return D3D11_BLEND_DEST_ALPHA;
                case EBF_ONE_MINUS_DST_ALPHA: return D3D11_BLEND_INV_DEST_ALPHA;
                case EBF_SRC_ALPHA_SATURATE:  return D3D11_BLEND_SRC_ALPHA_SAT;
            }
            return D3D11_BLEND_ONE;
        }


        u32 CD3D11MaterialRenderer_ONETEXTURE_BLEND::getD3D11Modulate(E_MODULATE_FUNC func) const
        {
            switch (func)
            {
                case EMFN_MODULATE_1X: return 1;
                case EMFN_MODULATE_2X: return 2;
                case EMFN_MODULATE_4X: return 4;
            }
            return 1;
        }


        void CD3D11MaterialRenderer_LIGHTMAP::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        void CD3D11MaterialRenderer_DETAIL_MAP::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        void CD3D11MaterialRenderer_SPHERE_MAP::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        void CD3D11MaterialRenderer_SPHERE_MAP::OnUnsetMaterial()
        {
        }


        void CD3D11MaterialRenderer_REFLECTION_2_LAYER::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            services->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }


        void CD3D11MaterialRenderer_REFLECTION_2_LAYER::OnUnsetMaterial()
        {
        }
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_