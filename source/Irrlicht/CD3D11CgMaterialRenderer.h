// Copyright (C) 2012 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_DIRECT3D_11_CG_MATERIAL_RENDERER_H_INCLUDED__
#define __C_DIRECT3D_11_CG_MATERIAL_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"
#if defined(_IRR_COMPILE_WITH_DIRECT3D_11_) && defined(_IRR_COMPILE_WITH_CG_)

#include <windows.h>
#include <d3d11.h>
#include "CCgMaterialRenderer.h"
#include "Cg/cgD3D11.h"

#ifdef _MSC_VER
    #pragma comment(lib, "cgD3D11.lib")
#endif

namespace irr
{
    namespace video
    {
        class CD3D11Driver;

        class CD3D11CgUniformSampler2D : public CCgUniform
        {
public:
            CD3D11CgUniformSampler2D(const CGparameter &parameter, bool global);

            void update(const void *data, const SMaterial &material) const;
        };

        class CD3D11CgMaterialRenderer : public CCgMaterialRenderer
        {
public:
            CD3D11CgMaterialRenderer(CD3D11Driver *driver, s32 &materialType,
                const c8 *vertexProgram = 0, const c8 *vertexEntry = "main",
                E_VERTEX_SHADER_TYPE vertexProfile = video::EVST_VS_4_0,
                const c8 *fragmentProgram = 0, const c8 *fragmentEntry = "main",
                E_PIXEL_SHADER_TYPE fragmentProfile = video::EPST_PS_4_0,
                const c8 *geometryProgram = 0, const c8 *geometryEntry = "main",
                E_GEOMETRY_SHADER_TYPE geometryProfile = video::EGST_GS_4_0,
                scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
                scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
                u32 vertices = 0, IShaderConstantSetCallBack *callback = 0,
                IMaterialRenderer *baseMaterial = 0, s32 userData = 0);

            virtual ~CD3D11CgMaterialRenderer();

            virtual void OnSetMaterial(const SMaterial &material, const SMaterial &lastMaterial,
                bool resetAllRenderstates, IMaterialRendererServices *services);
            virtual bool OnRender(IMaterialRendererServices *services, E_VERTEX_TYPE vtxtype);
            virtual void OnUnsetMaterial();

            virtual void setBasicRenderStates(const SMaterial &material,
                const SMaterial &lastMaterial, bool resetAllRenderstates);
            virtual IVideoDriver* getVideoDriver();

protected:
            void init(s32 &materialType,
                const c8 *vertexProgram = 0, const c8 *vertexEntry = "main",
                E_VERTEX_SHADER_TYPE vertexProfile = video::EVST_VS_4_0,
                const c8 *fragmentProgram = 0, const c8 *fragmentEntry = "main",
                E_PIXEL_SHADER_TYPE fragmentProfile = video::EPST_PS_4_0,
                const c8 *geometryProgram = 0, const c8 *geometryEntry = "main",
                E_GEOMETRY_SHADER_TYPE geometryProfile = video::EGST_GS_4_0,
                scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
                scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
                u32 vertices = 0);

            CD3D11Driver *Driver;
        };
    }
}
#endif
#endif