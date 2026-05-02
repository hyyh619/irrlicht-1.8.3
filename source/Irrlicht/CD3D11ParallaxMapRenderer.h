// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_D3D11_PARALLAX_MAPMATERIAL_RENDERER_H_INCLUDED__
#define __C_D3D11_PARALLAX_MAPMATERIAL_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_WINDOWS_

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
#if defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#include "irrMath.h"
#endif
#include <d3d11.h>

#include "CD3D11ShaderMaterialRenderer.h"
#include "IShaderConstantSetCallBack.h"

namespace irr
{
    namespace video
    {
        class CD3D11ParallaxMapRenderer : public CD3D11ShaderMaterialRenderer, IShaderConstantSetCallBack
        {
public:

            CD3D11ParallaxMapRenderer(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext,
                video::IVideoDriver *driver, s32 &outMaterialTypeNr, IMaterialRenderer *baseMaterial);

            ~CD3D11ParallaxMapRenderer();

            virtual void OnSetConstants(IMaterialRendererServices *services, s32 userData);

            virtual bool OnRender(IMaterialRendererServices *service, E_VERTEX_TYPE vtxtype);

            virtual s32 getRenderCapability() const;

            virtual void OnSetMaterial(const SMaterial &material);

            virtual void OnSetMaterial(const video::SMaterial &material,
                const video::SMaterial &lastMaterial,
                bool resetAllRenderstates, video::IMaterialRendererServices *services);

private:

            f32 m_CurrentScale;
        };
    } // end namespace video
} // end namespace irr
#endif
#endif
#endif