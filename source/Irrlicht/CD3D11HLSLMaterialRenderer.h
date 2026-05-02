// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_D3D11_HLSL_MATERIAL_RENDERER_H_INCLUDED__
#define __C_D3D11_HLSL_MATERIAL_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_WINDOWS_

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include "CD3D11ShaderMaterialRenderer.h"
#include "IGPUProgrammingServices.h"
#include "irrMap.h"

namespace irr
{
    namespace video
    {
        class IVideoDriver;
        class IShaderConstantSetCallBack;
        class IMaterialRenderer;

        class CD3D11HLSLMaterialRenderer : public CD3D11ShaderMaterialRenderer
        {
public:

            CD3D11HLSLMaterialRenderer(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext,
                video::IVideoDriver *driver, s32 &outMaterialTypeNr,
                const c8 *vertexShaderProgram,
                const c8 *vertexShaderEntryPointName,
                E_VERTEX_SHADER_TYPE vsCompileTarget,
                const c8 *pixelShaderProgram,
                const c8 *pixelShaderEntryPointName,
                E_PIXEL_SHADER_TYPE psCompileTarget,
                IShaderConstantSetCallBack *callback,
                IMaterialRenderer *baseMaterial,
                s32 userData);

            ~CD3D11HLSLMaterialRenderer();

            virtual bool OnRender(IMaterialRendererServices *service, E_VERTEX_TYPE vtxtype);

protected:

            bool createHLSLVertexShader(const char *vertexShaderProgram,
                const char *shaderEntryPointName,
                const char *shaderTargetName);

            bool createHLSLPixelShader(const char *pixelShaderProgram,
                const char *shaderEntryPointName,
                const char *shaderTargetName);

            void printHLSLVariables(ID3DBlob *constantTable);

            core::map<core::stringc, D3D11_SHADER_VARIABLE_DESC> m_VSVariables;
            core::map<core::stringc, D3D11_SHADER_VARIABLE_DESC> m_PSVariables;
        };
    } // end namespace video
} // end namespace irr
#endif
#endif
#endif