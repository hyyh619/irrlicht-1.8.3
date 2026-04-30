// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_DIRECTX11_TEXTURE_H_INCLUDED__
#define __C_DIRECTX11_TEXTURE_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#include "ITexture.h"
#include "IImage.h"
#if defined(__BORLANDC__) || defined (__BCPLUSPLUS__)
#include "irrMath.h"
#endif
#include <d3d11.h>
#include <windef.h>
#include <dxgitype.h>
#include <dxgi1_2.h>

namespace irr
{
    namespace video
    {
        class CD3D11Driver;
        struct SD3D11DepthStencilView;

        class CD3D11Texture : public ITexture
        {
public:

            CD3D11Texture(IImage *image, CD3D11Driver *driver,
                          u32 flags, const io::path &name, void *mipmapData = 0);

            CD3D11Texture(CD3D11Driver *driver, const core::dimension2d<u32> &size, const io::path &name,
                          const ECOLOR_FORMAT format = ECOLOR_FORMAT::ECF_UNKNOWN);

            virtual ~CD3D11Texture();

            virtual void* lock(E_TEXTURE_LOCK_MODE mode = ETLM_READ_WRITE, u32 mipmapLevel = 0);

            virtual void unlock();

            virtual const core::dimension2d<u32>&getOriginalSize() const;

            virtual const core::dimension2d<u32>&getSize() const;

            virtual E_DRIVER_TYPE getDriverType() const;

            virtual ECOLOR_FORMAT getColorFormat() const;

            virtual u32 getPitch() const;

            ID3D11Texture2D* getD3D11Texture() const;

            bool hasMipMaps() const;

            virtual void regenerateMipMapLevels(void *mipmapData = 0);

            virtual bool isRenderTarget() const;

            ID3D11RenderTargetView* getRenderTargetView();

            ID3D11ShaderResourceView* getShaderResourceView() const;

private:
            friend class CD3D11Driver;

            void createRenderTarget(const ECOLOR_FORMAT format = ECOLOR_FORMAT::ECF_UNKNOWN);

            bool createTexture(u32 flags, IImage *image);

            bool copyTexture(IImage *image);

            bool createMipMaps(u32 level = 1);

            void copy16BitMipMap(char *src, char *tgt,
                                 s32 width, s32 height,  s32 pitchsrc, s32 pitchtgt) const;

            void copy32BitMipMap(char *src, char *tgt,
                                 s32 width, s32 height,  s32 pitchsrc, s32 pitchtgt) const;

            void setPitch(DXGI_FORMAT dxgiFormat);

            ID3D11Device                *Device;
            ID3D11Texture2D             *Texture;
            ID3D11ShaderResourceView    *ShaderResourceView;
            ID3D11RenderTargetView      *RenderTargetView;
            CD3D11Driver                *Driver;
            SD3D11DepthStencilView        *DepthSurface;
            core::dimension2d<u32>      TextureSize;
            core::dimension2d<u32>      ImageSize;
            s32                         Pitch;
            u32                         MipLevelLocked;
            ECOLOR_FORMAT               ColorFormat;
            DXGI_FORMAT                 DXGIFormat;

            bool    HasMipMaps;
            bool    HardwareMipMaps;
            bool    IsRenderTarget;
        };
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_
#endif // __C_DIRECTX11_TEXTURE_H_INCLUDED__