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

private:

            CD3D11Driver    *Driver;
            s32             &MaterialType;
        };
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_
#endif // __C_DIRECTX11_MATERIAL_RENDERER_H_INCLUDED__