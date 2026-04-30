// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D11Texture.h"
#include "CD3D11Driver.h"
#include "SColor.h"
#include "os.h"

namespace irr
{
    namespace video
    {
        CD3D11Texture::CD3D11Texture(CD3D11Driver *driver, const core::dimension2d<u32> &size,
                                     const io::path &name, const ECOLOR_FORMAT format)
            : ITexture(name), Texture(0), ShaderResourceView(0), RenderTargetView(0),
            Driver(driver), DepthSurface(0),
            TextureSize(size), ImageSize(size), Pitch(0), ColorFormat(ECOLOR_FORMAT::ECF_UNKNOWN),
            DXGIFormat(DXGI_FORMAT_UNKNOWN),
            HasMipMaps(false), HardwareMipMaps(false), IsRenderTarget(true)
        {
#ifdef _DEBUG
            setDebugName("CD3D11Texture");
#endif

            Device = driver->pID3DDevice;
            if (Device)
                Device->AddRef();

            createRenderTarget(format);
        }


        CD3D11Texture::CD3D11Texture(IImage *image, CD3D11Driver *driver,
                                     u32 flags, const io::path &name, void *mipmapData)
            : ITexture(name), Texture(0), ShaderResourceView(0), RenderTargetView(0),
            Driver(driver), DepthSurface(0),
            TextureSize(0, 0), ImageSize(0, 0), Pitch(0), ColorFormat(ECOLOR_FORMAT::ECF_UNKNOWN),
            DXGIFormat(DXGI_FORMAT_UNKNOWN),
            HasMipMaps(false), HardwareMipMaps(false), IsRenderTarget(false)
        {
#ifdef _DEBUG
            setDebugName("CD3D11Texture");
#endif

            HasMipMaps = Driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);

            Device = driver->pID3DDevice;
            if (Device)
                Device->AddRef();

            if (image)
            {
                if (createTexture(flags, image))
                {
                    if (copyTexture(image))
                    {
                        regenerateMipMapLevels(mipmapData);
                    }
                }
                else
                    os::Printer::log("Could not create DIRECT3D11 Texture.", ELL_WARNING);
            }
        }


        CD3D11Texture::~CD3D11Texture()
        {
            if (Texture)
                Texture->Release();

            if (RenderTargetView)
                RenderTargetView->Release();

            if (ShaderResourceView)
                ShaderResourceView->Release();

            if (DepthSurface)
            {
                if (DepthSurface->drop())
                    Driver->removeDepthSurface(reinterpret_cast<SD3D11DepthStencilView*>(DepthSurface));
            }

            if (Device)
                Device->Release();
        }


        void* CD3D11Texture::lock(E_TEXTURE_LOCK_MODE mode, u32 mipmapLevel)
        {
            return 0;
        }


        void CD3D11Texture::unlock()
        {}


        const core::dimension2d<u32>&CD3D11Texture::getOriginalSize() const
        {
            return ImageSize;
        }


        const core::dimension2d<u32>&CD3D11Texture::getSize() const
        {
            return TextureSize;
        }


        E_DRIVER_TYPE CD3D11Texture::getDriverType() const
        {
            return EDT_DIRECT3D11;
        }


        ECOLOR_FORMAT CD3D11Texture::getColorFormat() const
        {
            return ColorFormat;
        }


        u32 CD3D11Texture::getPitch() const
        {
            return Pitch;
        }


        ID3D11Texture2D* CD3D11Texture::getD3D11Texture() const
        {
            return Texture;
        }


        bool CD3D11Texture::hasMipMaps() const
        {
            return HasMipMaps;
        }


        void CD3D11Texture::regenerateMipMapLevels(void *mipmapData)
        {}


        bool CD3D11Texture::isRenderTarget() const
        {
            return IsRenderTarget;
        }


        ID3D11RenderTargetView* CD3D11Texture::getRenderTargetView()
        {
            return RenderTargetView;
        }


        void CD3D11Texture::createRenderTarget(const ECOLOR_FORMAT format)
        {
            if (!Device)
                return;

            ECOLOR_FORMAT    colorFormat = format;
            if (colorFormat == ECOLOR_FORMAT::ECF_UNKNOWN)
                colorFormat = ECOLOR_FORMAT::ECF_A8R8G8B8;

            DXGIFormat = Driver->getDXGIFormatFromColorFormat(colorFormat);

            D3D11_TEXTURE2D_DESC    desc;
            desc.Width              = TextureSize.Width;
            desc.Height             = TextureSize.Height;
            desc.MipLevels          = 1;
            desc.ArraySize          = 1;
            desc.Format             = DXGIFormat;
            desc.SampleDesc.Count   = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage              = D3D11_USAGE_DEFAULT;
            desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags     = 0;
            desc.MiscFlags          = 0;

            HRESULT    hr = Device->CreateTexture2D(&desc, 0, &Texture);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create render target texture.", ELL_WARNING);
                return;
            }

            D3D11_RENDER_TARGET_VIEW_DESC    rtvDesc;
            rtvDesc.Format              = DXGIFormat;
            rtvDesc.ViewDimension       = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice  = 0;

            hr = Device->CreateRenderTargetView(Texture, &rtvDesc, &RenderTargetView);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create render target view.", ELL_WARNING);
                return;
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC    srvDesc;
            srvDesc.Format                      = DXGIFormat;
            srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip   = 0;
            srvDesc.Texture2D.MipLevels         = 1;

            hr = Device->CreateShaderResourceView(Texture, &srvDesc, &ShaderResourceView);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create shader resource view.", ELL_WARNING);
                return;
            }

            ColorFormat     = colorFormat;
            IsRenderTarget  = true;
        }


        bool CD3D11Texture::createTexture(u32 flags, IImage *image)
        {
            return false;
        }


        bool CD3D11Texture::copyTexture(IImage *image)
        {
            return false;
        }


        bool CD3D11Texture::createMipMaps(u32 level)
        {
            return false;
        }


        void CD3D11Texture::copy16BitMipMap(char *src, char *tgt,
                                            s32 width, s32 height, s32 pitchsrc, s32 pitchtgt) const
        {}


        void CD3D11Texture::copy32BitMipMap(char *src, char *tgt,
                                            s32 width, s32 height, s32 pitchsrc, s32 pitchtgt) const
        {}


        void CD3D11Texture::setPitch(DXGI_FORMAT dxgiFormat)
        {
            Pitch = TextureSize.Width * 4;
        }
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_