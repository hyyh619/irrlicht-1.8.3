// Copyright (C) 2012 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#if defined(_IRR_COMPILE_WITH_DIRECT3D_11_) && defined(_IRR_COMPILE_WITH_CG_)

#include "CD3D11CgMaterialRenderer.h"
#include "CD3D11Driver.h"
#include "CD3D11Texture.h"

namespace irr
{
    namespace video
    {
        CD3D11CgUniformSampler2D::CD3D11CgUniformSampler2D(const CGparameter &parameter, bool global)
            : CCgUniform(parameter, global)
        {
            Type = CG_SAMPLER2D;
        }

        void CD3D11CgUniformSampler2D::update(const void *data, const SMaterial &material) const
        {
            s32 *Data = (s32*)data;
            s32 LayerID = *Data;

            if (material.TextureLayer[LayerID].Texture)
            {
                ID3D11ShaderResourceView *textureView =
                    reinterpret_cast<irr::video::CD3D11Texture*>(material.TextureLayer[LayerID].Texture)->getShaderResourceView();

                if (textureView)
                    cgD3D11SetShaderResource(Parameter, textureView);
            }
        }

        CD3D11CgMaterialRenderer::CD3D11CgMaterialRenderer(CD3D11Driver *driver, s32 &materialType,
            const c8 *vertexProgram, const c8 *vertexEntry, E_VERTEX_SHADER_TYPE vertexProfile,
            const c8 *fragmentProgram, const c8 *fragmentEntry, E_PIXEL_SHADER_TYPE fragmentProfile,
            const c8 *geometryProgram, const c8 *geometryEntry, E_GEOMETRY_SHADER_TYPE geometryProfile,
            scene::E_PRIMITIVE_TYPE inType, scene::E_PRIMITIVE_TYPE outType, u32 vertices,
            IShaderConstantSetCallBack *callback, IMaterialRenderer *baseMaterial, s32 userData)
            : m_Driver(driver), CCgMaterialRenderer(callback, baseMaterial, userData)
        {
#ifdef _DEBUG
            setDebugName("CD3D11CgMaterialRenderer");
#endif

            init(materialType, vertexProgram, vertexEntry, vertexProfile, fragmentProgram, fragmentEntry, fragmentProfile,
                geometryProgram, geometryEntry, geometryProfile, inType, outType, vertices);
        }

        CD3D11CgMaterialRenderer::~CD3D11CgMaterialRenderer()
        {
            if (VertexProgram)
            {
                cgD3D11UnloadProgram(VertexProgram);
                cgDestroyProgram(VertexProgram);
            }

            if (FragmentProgram)
            {
                cgD3D11UnloadProgram(FragmentProgram);
                cgDestroyProgram(FragmentProgram);
            }

            if (GeometryProgram)
            {
                cgD3D11UnloadProgram(GeometryProgram);
                cgDestroyProgram(GeometryProgram);
            }
        }

        void CD3D11CgMaterialRenderer::OnSetMaterial(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates, IMaterialRendererServices *services)
        {
            Material = material;

            if (material.MaterialType != lastMaterial.MaterialType || resetAllRenderstates)
            {
                if (VertexProgram)
                    cgD3D11BindProgram(VertexProgram);

                if (FragmentProgram)
                    cgD3D11BindProgram(FragmentProgram);

                if (GeometryProgram)
                    cgD3D11BindProgram(GeometryProgram);

                if (BaseMaterial)
                    BaseMaterial->OnSetMaterial(material, material, true, this);
            }

            if (CallBack)
                CallBack->OnSetMaterial(material);

            Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }

        bool CD3D11CgMaterialRenderer::OnRender(IMaterialRendererServices *services, E_VERTEX_TYPE vtxtype)
        {
            if (CallBack && (VertexProgram || FragmentProgram || GeometryProgram))
                CallBack->OnSetConstants(this, UserData);

            return true;
        }

        void CD3D11CgMaterialRenderer::OnUnsetMaterial()
        {
            if (VertexProgram)
                cgD3D11UnbindProgram(VertexProgram);

            if (FragmentProgram)
                cgD3D11UnbindProgram(FragmentProgram);

            if (GeometryProgram)
                cgD3D11UnbindProgram(GeometryProgram);

            if (BaseMaterial)
                BaseMaterial->OnUnsetMaterial();

            Material = IdentityMaterial;
        }

        void CD3D11CgMaterialRenderer::setBasicRenderStates(const SMaterial &material,
            const SMaterial &lastMaterial, bool resetAllRenderstates)
        {
            m_Driver->setBasicRenderStates(material, lastMaterial, resetAllRenderstates);
        }

        IVideoDriver* CD3D11CgMaterialRenderer::getVideoDriver()
        {
            return m_Driver;
        }

        void CD3D11CgMaterialRenderer::init(s32 &materialType,
            const c8 *vertexProgram, const c8 *vertexEntry, E_VERTEX_SHADER_TYPE vertexProfile,
            const c8 *fragmentProgram, const c8 *fragmentEntry, E_PIXEL_SHADER_TYPE fragmentProfile,
            const c8 *geometryProgram, const c8 *geometryEntry, E_GEOMETRY_SHADER_TYPE geometryProfile,
            scene::E_PRIMITIVE_TYPE inType, scene::E_PRIMITIVE_TYPE outType, u32 vertices)
        {
            bool Status = true;
            CGerror Error = CG_NO_ERROR;

            materialType = -1;

            if (vertexProgram)
            {
                VertexProfile = cgD3D11GetLatestVertexProfile();

                if (VertexProfile)
                    VertexProgram = cgCreateProgram(m_Driver->getCgContext(), CG_SOURCE, vertexProgram,
                        VertexProfile, vertexEntry, 0);

                if (!VertexProgram)
                {
                    Error = cgGetError();
                    os::Printer::log("Cg vertex program failed to compile:", ELL_ERROR);
                    os::Printer::log(cgGetLastListing(m_Driver->getCgContext()), ELL_ERROR);
                    Status = false;
                }
                else
                    cgD3D11LoadProgram(VertexProgram, 0, 0);
            }

            if (fragmentProgram)
            {
                FragmentProfile = cgD3D11GetLatestPixelProfile();

                if (FragmentProfile)
                    FragmentProgram = cgCreateProgram(m_Driver->getCgContext(), CG_SOURCE, fragmentProgram,
                        FragmentProfile, fragmentEntry, 0);

                if (!FragmentProgram)
                {
                    Error = cgGetError();
                    os::Printer::log("Cg fragment program failed to compile:", ELL_ERROR);
                    os::Printer::log(cgGetLastListing(m_Driver->getCgContext()), ELL_ERROR);
                    Status = false;
                }
                else
                    cgD3D11LoadProgram(FragmentProgram, 0, 0);
            }

            if (geometryProgram)
            {
                GeometryProfile = cgD3D11GetLatestGeometryProfile();

                if (GeometryProfile)
                    GeometryProgram = cgCreateProgram(m_Driver->getCgContext(), CG_SOURCE, geometryProgram,
                        GeometryProfile, geometryEntry, 0);

                if (!GeometryProgram)
                {
                    Error = cgGetError();
                    os::Printer::log("Cg geometry program failed to compile:", ELL_ERROR);
                    os::Printer::log(cgGetLastListing(m_Driver->getCgContext()), ELL_ERROR);
                    Status = false;
                }
                else
                    cgD3D11LoadProgram(GeometryProgram, 0, 0);
            }

            getUniformList();

            for (unsigned int i = 0; i < UniformInfo.size(); ++i)
            {
                if (UniformInfo[i]->getType() == CG_SAMPLER2D)
                {
                    bool IsGlobal = true;
                    if (UniformInfo[i]->getSpace() == CG_PROGRAM)
                        IsGlobal = false;

                    CCgUniform *Uniform = new CD3D11CgUniformSampler2D(UniformInfo[i]->getParameter(), IsGlobal);
                    delete UniformInfo[i];
                    UniformInfo[i] = Uniform;
                }
            }

            if (Status)
                materialType = m_Driver->addMaterialRenderer(this);
        }
    }
}
#endif