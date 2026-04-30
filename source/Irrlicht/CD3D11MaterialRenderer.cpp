// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D11MaterialRenderer.h"
#include "CD3D11Driver.h"

namespace irr
{
    namespace video
    {
        CD3D11MaterialRenderer::CD3D11MaterialRenderer(CD3D11Driver *driver, s32 &materialType,
                                                       const c8 *name)
            : Driver(driver), MaterialType(materialType)
        {}


        CD3D11MaterialRenderer::~CD3D11MaterialRenderer()
        {}


        void CD3D11MaterialRenderer::OnSetMaterial(const SMaterial &material)
        {}


        bool CD3D11MaterialRenderer::OnSetTexture(u32 textureIndex, ITexture *texture)
        {
            return true;
        }


        void CD3D11MaterialRenderer::OnSetConstants(IMaterialRendererServices *services, s32 userData)
        {}


        void CD3D11MaterialRenderer::PostRender()
        {}
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_