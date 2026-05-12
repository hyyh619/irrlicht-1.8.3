// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_D3D11_SHADER_H_INCLUDED__
#define __C_D3D11_SHADER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_WINDOWS_

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include <d3d11.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include "IReferenceCounted.h"
#include "irrString.h"
#include "S3DVertex.h"
#include "EMaterialTypes.h"

namespace irr
{
    namespace video
    {
        class CD3D11Driver;

        enum E_D3D11_SHADER_TYPE
        {
            EDST_VERTEX = 0,
            EDST_HULL,
            EDST_DOMAIN,
            EDST_GEOMETRY,
            EDST_PIXEL,
            EDST_COMPUTE,
            EDST_COUNT
        };

        class CD3D11Shader : public IReferenceCounted
        {
public:
            CD3D11Shader(CD3D11Driver *driver);
            virtual ~CD3D11Shader();

            bool compile(E_D3D11_SHADER_TYPE type, const c8 *hlslSource, const c8 *entryPoint, const c8 *profile, const c8 *hlslSourcePart2 = 0);
            bool createVertexShader();
            bool createHullShader();
            bool createDomainShader();
            bool createGeometryShader();
            bool createPixelShader();
            bool createComputeShader();
            bool createInputLayout(const D3D11_INPUT_ELEMENT_DESC *layout, u32 elementCount);
            void drop();

            ID3D11VertexShader* getVertexShader() const
            {
                return m_VertexShader;
            }
            ID3D11HullShader* getHullShader() const
            {
                return m_HullShader;
            }
            ID3D11DomainShader* getDomainShader() const
            {
                return m_DomainShader;
            }
            ID3D11GeometryShader* getGeometryShader() const
            {
                return m_GeometryShader;
            }
            ID3D11PixelShader* getPixelShader() const
            {
                return m_PixelShader;
            }
            ID3D11ComputeShader* getComputeShader() const
            {
                return m_ComputeShader;
            }
            ID3D11InputLayout* getInputLayout() const
            {
                return m_InputLayout;
            }
            ID3DBlob* getShaderBlob() const
            {
                return m_ShaderBlob;
            }
            const core::stringc&getHLSLSource() const
            {
                return m_HLSLSource;
            }
            E_D3D11_SHADER_TYPE getShaderType() const
            {
                return m_ShaderType;
            }
            E_VERTEX_TYPE getVertexType() const
            {
                return m_VertexType;
            }
            void setVertexType(E_VERTEX_TYPE type)
            {
                m_VertexType = type;
            }
            void setMaterialType(E_MATERIAL_TYPE type)
            {
                m_MaterialType = type;
            }
            E_MATERIAL_TYPE getMaterialType() const
            {
                return m_MaterialType;
            }
            bool isCompiled() const
            {
                return m_Compiled;
            }

private:
            CD3D11Driver    *m_Driver;

            core::stringc       m_HLSLSource;
            core::stringc       m_EntryPoint;
            core::stringc       m_Profile;

            E_D3D11_SHADER_TYPE     m_ShaderType;
            E_VERTEX_TYPE           m_VertexType;
            E_MATERIAL_TYPE         m_MaterialType;
            bool                    m_Compiled;

            ID3DBlob    *m_ShaderBlob;

            ID3D11VertexShader      *m_VertexShader;
            ID3D11HullShader        *m_HullShader;
            ID3D11DomainShader      *m_DomainShader;
            ID3D11GeometryShader    *m_GeometryShader;
            ID3D11PixelShader       *m_PixelShader;
            ID3D11ComputeShader     *m_ComputeShader;

            ID3D11InputLayout           *m_InputLayout;
            D3D11_INPUT_ELEMENT_DESC    *m_InputLayoutDesc;
            u32                         m_InputLayoutElementCount;
        };

        extern const char    VERTEX_SHADER_STANDARD[];
        extern const char    VERTEX_SHADER_STANDARD_DIRECTIONAL[];
        extern const char    VERTEX_SHADER_STANDARD_POINT[];
        extern const char    VERTEX_SHADER_STANDARD_SPOT[];
        extern const char    VERTEX_SHADER_2TCOORDS_DIRECTIONAL[];
        extern const char    VERTEX_SHADER_2TCOORDS_POINT[];
        extern const char    VERTEX_SHADER_2TCOORDS_SPOT[];
        extern const char    VERTEX_SHADER_2TCOORDS[];
        extern const char    VERTEX_SHADER_RECTANGLE[];
        extern const char    PIXEL_SHADER_RECTANGLE[];
        extern const char    VERTEX_SHADER_TANGENTS[];
        extern const char    PIXEL_SHADER_STANDARD[];
        extern const char    PIXEL_SHADER_2TCOORDS[];
        extern const char    PIXEL_SHADER_TANGENTS[];
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_
#endif // _IRR_WINDOWS_
#endif // __C_D3D11_SHADER_H_INCLUDED__