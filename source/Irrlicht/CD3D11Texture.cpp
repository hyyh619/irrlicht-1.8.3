// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_11_

#define _IRR_DONT_DO_MEMORY_DEBUGGING_HERE
#include "CD3D11Texture.h"
#include "CD3D11Driver.h"
#include "CD3D11ObjectTracker.h"
#include "SColor.h"
#include "os.h"

namespace irr
{
    namespace video
    {
        CD3D11Texture::CD3D11Texture(CD3D11Driver *driver, const core::dimension2d<u32> &size,
                                     const io::path &name, const ECOLOR_FORMAT format)
            : ITexture(name), m_Texture(0), m_ShaderResourceView(0), m_RenderTargetView(0),
            m_Driver(driver), m_DepthSurface(0),
            m_TextureSize(size), m_ImageSize(size), m_Pitch(0), m_ColorFormat(ECOLOR_FORMAT::ECF_UNKNOWN),
            m_DXGIFormat(DXGI_FORMAT_UNKNOWN),
            m_HasMipMaps(false), m_HardwareMipMaps(false), m_IsRenderTarget(true)
        {
#ifdef _DEBUG
            setDebugName("CD3D11Texture");
#endif

            m_Device = driver->m_pID3DDevice;
            if (m_Device)
            {
                IRR_D3D11_DEVICE_ADDREF(m_Device, "CD3D11Texture_Device");
                m_Device->AddRef();
            }

            createRenderTarget(format);
        }


        CD3D11Texture::CD3D11Texture(IImage *image, CD3D11Driver *driver,
                                     u32 flags, const io::path &name, void *mipmapData)
            : ITexture(name), m_Texture(0), m_ShaderResourceView(0), m_RenderTargetView(0),
            m_Driver(driver), m_DepthSurface(0),
            m_TextureSize(0, 0), m_ImageSize(0, 0), m_Pitch(0), m_ColorFormat(ECOLOR_FORMAT::ECF_UNKNOWN),
            m_DXGIFormat(DXGI_FORMAT_UNKNOWN),
            m_HasMipMaps(false), m_HardwareMipMaps(false), m_IsRenderTarget(false)
        {
#ifdef _DEBUG
            setDebugName("CD3D11Texture");
#endif

            m_HasMipMaps = m_Driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);

            m_Device = driver->m_pID3DDevice;
            if (m_Device)
            {
                IRR_D3D11_DEVICE_ADDREF(m_Device, "CD3D11Texture_Device");
                m_Device->AddRef();
            }

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
            if (m_Texture)
            {
                IRR_D3D11_TEXTURE2D_RELEASE(m_Texture, "RenderTargetTexture");
                m_Texture->Release();
            }

            if (m_RenderTargetView)
            {
                IRR_D3D11_RTV_RELEASE(m_RenderTargetView, "RenderTargetView");
                m_RenderTargetView->Release();
            }

            if (m_ShaderResourceView)
            {
                IRR_D3D11_SRV_RELEASE(m_ShaderResourceView, "ShaderResourceView");
                m_ShaderResourceView->Release();
            }

            if (m_DepthSurface)
            {
                if (m_DepthSurface->drop())
                    m_Driver->removeDepthSurface(reinterpret_cast<SD3D11DepthStencilView*>(m_DepthSurface));
            }

            if (m_Device)
            {
                IRR_D3D11_DEVICE_RELEASE(m_Device, "CD3D11Texture_Device");
                m_Device->Release();
            }
        }


        void* CD3D11Texture::lock(E_TEXTURE_LOCK_MODE mode, u32 mipmapLevel)
        {
            return 0;
        }


        void CD3D11Texture::unlock()
        {}


        const core::dimension2d<u32>&CD3D11Texture::getOriginalSize() const
        {
            return m_ImageSize;
        }


        const core::dimension2d<u32>&CD3D11Texture::getSize() const
        {
            return m_TextureSize;
        }


        E_DRIVER_TYPE CD3D11Texture::getDriverType() const
        {
            return EDT_DIRECT3D11;
        }


        ECOLOR_FORMAT CD3D11Texture::getColorFormat() const
        {
            return m_ColorFormat;
        }


        u32 CD3D11Texture::getPitch() const
        {
            return m_Pitch;
        }


        ID3D11Texture2D* CD3D11Texture::getD3D11Texture() const
        {
            return m_Texture;
        }


        bool CD3D11Texture::hasMipMaps() const
        {
            return m_HasMipMaps;
        }


        void CD3D11Texture::regenerateMipMapLevels(void *mipmapData)
        {
            if (!m_Texture || !m_Driver->m_pID3DDeviceContext)
                return;

            if (m_HasMipMaps)
            {
                m_Driver->m_pID3DDeviceContext->GenerateMips(m_ShaderResourceView);
            }
        }


        bool CD3D11Texture::isRenderTarget() const
        {
            return m_IsRenderTarget;
        }


        ID3D11RenderTargetView* CD3D11Texture::getRenderTargetView()
        {
            return m_RenderTargetView;
        }


        ID3D11ShaderResourceView* CD3D11Texture::getShaderResourceView() const
        {
            return m_ShaderResourceView;
        }


        void CD3D11Texture::createRenderTarget(const ECOLOR_FORMAT format)
        {
            if (!m_Device)
                return;

            ECOLOR_FORMAT    colorFormat = format;
            if (colorFormat == ECOLOR_FORMAT::ECF_UNKNOWN)
                colorFormat = ECOLOR_FORMAT::ECF_A8R8G8B8;

            m_DXGIFormat = m_Driver->getDXGIFormatFromColorFormat(colorFormat);

            D3D11_TEXTURE2D_DESC    desc;
            desc.Width              = m_TextureSize.Width;
            desc.Height             = m_TextureSize.Height;
            desc.MipLevels          = 1;
            desc.ArraySize          = 1;
            desc.Format             = m_DXGIFormat;
            desc.SampleDesc.Count   = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage              = D3D11_USAGE_DEFAULT;
            desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            desc.CPUAccessFlags     = 0;
            desc.MiscFlags          = 0;

            HRESULT    hr = m_Device->CreateTexture2D(&desc, 0, &m_Texture);
            IRR_D3D11_TEXTURE2D_CREATE(m_Texture, "RenderTargetTexture");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create render target texture.", ELL_WARNING);
                return;
            }

            D3D11_RENDER_TARGET_VIEW_DESC    rtvDesc;
            rtvDesc.Format              = m_DXGIFormat;
            rtvDesc.ViewDimension       = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice  = 0;

            hr = m_Device->CreateRenderTargetView(m_Texture, &rtvDesc, &m_RenderTargetView);
            IRR_D3D11_RTV_CREATE(m_RenderTargetView, "RenderTargetView");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create render target view.", ELL_WARNING);
                return;
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC    srvDesc;
            srvDesc.Format                      = m_DXGIFormat;
            srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip   = 0;
            srvDesc.Texture2D.MipLevels         = 1;

            hr = m_Device->CreateShaderResourceView(m_Texture, &srvDesc, &m_ShaderResourceView);
            IRR_D3D11_SRV_CREATE(m_ShaderResourceView, "ShaderResourceView");
            if (FAILED(hr))
            {
                os::Printer::log("Could not create shader resource view.", ELL_WARNING);
                return;
            }

            m_ColorFormat       = colorFormat;
            m_IsRenderTarget    = true;
        }


        bool CD3D11Texture::createTexture(u32 flags, IImage *image)
        {
            if (!m_Device || !image)
                return false;

            m_ImageSize = image->getDimension();

            core::dimension2d<u32>    optSize = m_ImageSize.getOptimalSize(
                !m_Driver->queryFeature(EVDF_TEXTURE_NPOT),
                !m_Driver->queryFeature(EVDF_TEXTURE_NSQUARE),
                true, 0);

            ECOLOR_FORMAT    format = image->getColorFormat();

            switch (getTextureFormatFromFlags(flags))
            {
                case ETCF_ALWAYS_16_BIT:
                    format = ECOLOR_FORMAT::ECF_A1R5G5B5;
                    break;

                case ETCF_ALWAYS_32_BIT:
                    format = ECOLOR_FORMAT::ECF_A8R8G8B8;
                    break;

                case ETCF_OPTIMIZED_FOR_QUALITY:
                {
                    switch (image->getColorFormat())
                    {
                        case ECOLOR_FORMAT::ECF_R8G8B8:
                        case ECOLOR_FORMAT::ECF_A8R8G8B8:
                            format = ECOLOR_FORMAT::ECF_A8R8G8B8;
                            break;

                        case ECOLOR_FORMAT::ECF_A1R5G5B5:
                        case ECOLOR_FORMAT::ECF_R5G6B5:
                            format = ECOLOR_FORMAT::ECF_A1R5G5B5;
                            break;
                    }
                }
                break;

                case ETCF_OPTIMIZED_FOR_SPEED:
                    format = ECOLOR_FORMAT::ECF_A1R5G5B5;
                    break;

                default:
                    break;
            }

            if (m_Driver->getTextureCreationFlag(video::ETCF_NO_ALPHA_CHANNEL))
            {
                if (format == ECOLOR_FORMAT::ECF_A8R8G8B8)
                    format = ECOLOR_FORMAT::ECF_R8G8B8;
                else if (format == ECOLOR_FORMAT::ECF_A1R5G5B5)
                    format = ECOLOR_FORMAT::ECF_R5G6B5;
            }

            m_DXGIFormat = m_Driver->getDXGIFormatFromColorFormat(format);
            if (m_DXGIFormat == DXGI_FORMAT_UNKNOWN)
                m_DXGIFormat = DXGI_FORMAT_B8G8R8A8_UNORM;

            const bool      mipmaps     = m_Driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);
            const u32       mipLevels   = mipmaps ? 0 : 1;

            D3D11_TEXTURE2D_DESC    desc;
            desc.Width              = optSize.Width;
            desc.Height             = optSize.Height;
            desc.MipLevels          = mipLevels;
            desc.ArraySize          = 1;
            desc.Format             = m_DXGIFormat;
            desc.SampleDesc.Count   = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage              = D3D11_USAGE_DEFAULT;
            desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
            if (mipmaps)
                desc.BindFlags |= D3D11_BIND_RENDER_TARGET;

            desc.CPUAccessFlags = 0;
            desc.MiscFlags      = mipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

            HRESULT    hr = m_Device->CreateTexture2D(&desc, 0, &m_Texture);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create DIRECT3D11 Texture.", ELL_WARNING);
                return false;
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC    srvDesc;
            srvDesc.Format                      = m_DXGIFormat;
            srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip   = 0;
            srvDesc.Texture2D.MipLevels         = mipmaps ? -1 : 1;

            hr = m_Device->CreateShaderResourceView(m_Texture, &srvDesc, &m_ShaderResourceView);
            if (FAILED(hr))
            {
                os::Printer::log("Could not create shader resource view.", ELL_WARNING);
                m_Texture->Release();
                m_Texture = 0;
                return false;
            }

            m_TextureSize   = optSize;
            m_ColorFormat   = format;
            m_HasMipMaps    = mipmaps;
            setPitch(m_DXGIFormat);

            return true;
        }


        bool CD3D11Texture::copyTexture(IImage *image)
        {
            if (!m_Texture || !image)
                return false;

            ID3D11DeviceContext    *context = m_Driver->m_pID3DDeviceContext;

            D3D11_BOX    destBox;
            destBox.left    = 0;
            destBox.top     = 0;
            destBox.front   = 0;
            destBox.right   = image->getDimension().Width;
            destBox.bottom  = image->getDimension().Height;
            destBox.back    = 1;

            void    *imageData  = image->lock();

            if (image->getColorFormat() != m_ColorFormat)
            {
                IImage    *tmpImage = m_Driver->createImage(m_ColorFormat, image->getDimension());
                if (!tmpImage)
                {
                    image->unlock();
                    return false;
                }

                image->copyToScaling(tmpImage);

                u32     tmpPitch    = tmpImage->getPitch();
                void    *tmpData    = tmpImage->lock();
                context->UpdateSubresource(m_Texture, 0, &destBox, tmpData, tmpPitch, 0);
                tmpImage->unlock();
                tmpImage->drop();
            }
            else
            {
                u32    imagePitch  = image->getPitch();
                context->UpdateSubresource(m_Texture, 0, &destBox, imageData, imagePitch, 0);
            }

            image->unlock();

            return true;
        }


        bool CD3D11Texture::createMipMaps(u32 level)
        {
            if (!m_Texture || !m_Driver->m_pID3DDeviceContext)
                return false;

            if (m_HardwareMipMaps)
            {
                m_Driver->m_pID3DDeviceContext->GenerateMips(m_ShaderResourceView);
                return true;
            }

            if (level == 0)
                return true;

            ID3D11DeviceContext    *context = m_Driver->m_pID3DDeviceContext;

            const u32       width   = m_TextureSize.Width >> level;
            const u32       height  = m_TextureSize.Height >> level;

            D3D11_MAPPED_SUBRESOURCE    upperRes;
            D3D11_MAPPED_SUBRESOURCE    lowerRes;

            HRESULT    hr = context->Map(m_Texture, level - 1, D3D11_MAP_READ, 0, &upperRes);
            if (FAILED(hr))
            {
                os::Printer::log("Could not map upper texture for mip map generation", ELL_WARNING);
                return false;
            }

            hr = context->Map(m_Texture, level, D3D11_MAP_READ, 0, &lowerRes);
            if (FAILED(hr))
            {
                context->Unmap(m_Texture, level - 1);
                os::Printer::log("Could not map lower texture for mip map generation", ELL_WARNING);
                return false;
            }

            if (m_DXGIFormat == DXGI_FORMAT_B5G6R5_UNORM || m_DXGIFormat == DXGI_FORMAT_B5G5R5A1_UNORM)
                copy16BitMipMap((char*)upperRes.pData, (char*)lowerRes.pData,
                                width, height, upperRes.RowPitch, lowerRes.RowPitch);
            else if (m_DXGIFormat == DXGI_FORMAT_B8G8R8A8_UNORM || m_DXGIFormat == DXGI_FORMAT_B8G8R8X8_UNORM)
                copy32BitMipMap((char*)upperRes.pData, (char*)lowerRes.pData,
                                width, height, upperRes.RowPitch, lowerRes.RowPitch);
            else
                os::Printer::log("Unsupported mipmap format, cannot copy.", ELL_WARNING);

            context->Unmap(m_Texture, level - 1);
            context->Unmap(m_Texture, level);

            return createMipMaps(level + 1);
        }


        void CD3D11Texture::copy16BitMipMap(char *src, char *tgt,
                                            s32 width, s32 height, s32 pitchsrc, s32 pitchtgt) const
        {
            for (s32 y = 0; y < height; ++y)
            {
                for (s32 x = 0; x < width; ++x)
                {
                    u32    a = 0, r = 0, g = 0, b = 0;

                    for (s32 dy = 0; dy < 2; ++dy)
                    {
                        const s32    tgy = (y * 2) + dy;

                        for (s32 dx = 0; dx < 2; ++dx)
                        {
                            const s32    tgx = (x * 2) + dx;

                            SColor    c;
                            if (m_ColorFormat == ECOLOR_FORMAT::ECF_A1R5G5B5)
                                c = A1R5G5B5toA8R8G8B8(*(u16*)(&src[(tgx * 2) + (tgy * pitchsrc)]));
                            else
                                c = R5G6B5toA8R8G8B8(*(u16*)(&src[(tgx * 2) + (tgy * pitchsrc)]));

                            a   += c.getAlpha();
                            r   += c.getRed();
                            g   += c.getGreen();
                            b   += c.getBlue();
                        }
                    }

                    a   /= 4;
                    r   /= 4;
                    g   /= 4;
                    b   /= 4;

                    u16    c;
                    if (m_ColorFormat == ECOLOR_FORMAT::ECF_A1R5G5B5)
                        c = RGBA16(r, g, b, a);
                    else
                        c = A8R8G8B8toR5G6B5(SColor(a, r, g, b).color);

                    *(u16*)(&tgt[(x * 2) + (y * pitchtgt)]) = c;
                }
            }
        }


        void CD3D11Texture::copy32BitMipMap(char *src, char *tgt,
                                            s32 width, s32 height, s32 pitchsrc, s32 pitchtgt) const
        {
            for (s32 y = 0; y < height; ++y)
            {
                for (s32 x = 0; x < width; ++x)
                {
                    u32         a = 0, r = 0, g = 0, b = 0;
                    SColor      c;

                    for (s32 dy = 0; dy < 2; ++dy)
                    {
                        const s32    tgy = (y * 2) + dy;

                        for (s32 dx = 0; dx < 2; ++dx)
                        {
                            const s32    tgx = (x * 2) + dx;

                            c = *(u32*)(&src[(tgx * 4) + (tgy * pitchsrc)]);

                            a   += c.getAlpha();
                            r   += c.getRed();
                            g   += c.getGreen();
                            b   += c.getBlue();
                        }
                    }

                    a   /= 4;
                    r   /= 4;
                    g   /= 4;
                    b   /= 4;

                    c.set(a, r, g, b);
                    *(u32*)(&tgt[(x * 4) + (y * pitchtgt)]) = c.color;
                }
            }
        }


        void CD3D11Texture::setPitch(DXGI_FORMAT dxgiFormat)
        {
            m_Pitch = m_TextureSize.Width * 4;
        }
    } // end namespace video
} // end namespace irr
#endif // _IRR_COMPILE_WITH_DIRECT3D_11_