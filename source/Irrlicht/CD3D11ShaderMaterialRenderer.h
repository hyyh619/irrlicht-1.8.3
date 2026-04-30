// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_D3D11_SHADER_MATERIAL_RENDERER_H_INCLUDED__
#define __C_D3D11_SHADER_MATERIAL_RENDERER_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_WINDOWS_

#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_
#if defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#include "irrMath.h"
#endif
#include <d3d11.h>
#include <d3dcompiler.h>

#include "IMaterialRenderer.h"

namespace irr
{
    namespace video
    {
        class IVideoDriver;
        class IShaderConstantSetCallBack;
        class IMaterialRenderer;

        class CD3D11ShaderMaterialRenderer : public IMaterialRenderer
        {
public:

            CD3D11ShaderMaterialRenderer(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext,
                video::IVideoDriver *driver, s32 &outMaterialTypeNr,
                const c8 *vertexShaderProgram, const c8 *pixelShaderProgram,
                IShaderConstantSetCallBack *callback, IMaterialRenderer *baseMaterial, s32 userData);

            ~CD3D11ShaderMaterialRenderer();

            virtual void OnSetMaterial(const video::SMaterial &material, const video::SMaterial &lastMaterial,
                bool resetAllRenderstates, video::IMaterialRendererServices *services);

            virtual void OnUnsetMaterial();

            virtual bool OnRender(IMaterialRendererServices *service, E_VERTEX_TYPE vtxtype);

            virtual bool isTransparent() const;

protected:

            CD3D11ShaderMaterialRenderer(ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dContext,
                video::IVideoDriver *driver, IShaderConstantSetCallBack *callback,
                IMaterialRenderer *baseMaterial, s32 userData = 0);

            void init(s32 &outMaterialTypeNr, const c8 *vertexShaderProgram, const c8 *pixelShaderProgram);
            bool createPixelShader(const c8 *pxsh);
            bool createVertexShader(const char *vtxsh);

            ID3D11Device                *pID3DDevice;
            ID3D11DeviceContext         *pID3DDeviceContext;
            video::IVideoDriver         *Driver;
            IShaderConstantSetCallBack  *CallBack;
            IMaterialRenderer           *BaseMaterial;

            ID3D11VertexShader          *VertexShader;
            ID3D11VertexShader          *OldVertexShader;
            ID3D11PixelShader           *PixelShader;
            ID3D11InputLayout           *InputLayout;
            s32                         UserData;
        };
    } // end namespace video
} // end namespace irr
#endif
#endif
#endif