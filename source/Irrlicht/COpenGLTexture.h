// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

/**
 * @file COpenGLTexture.h
 * @brief OpenGL texture implementation for the Irrlicht Engine.
 * This file contains the COpenGLTexture class which provides OpenGL-specific
 * texture handling functionality, including mipmap management, render targets,
 * and FrameBufferObject (FBO) support.
 */

#ifndef __C_OPEN_GL_TEXTURE_H_INCLUDED__
#define __C_OPEN_GL_TEXTURE_H_INCLUDED__

#include "ITexture.h"
#include "IImage.h"

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_OPENGL_

#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
    #define GL_GLEXT_LEGACY 1
#else
    #define GL_GLEXT_PROTOTYPES 1
#endif
#ifdef _IRR_WINDOWS_API_
// include windows headers for HWND
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <GL/gl.h>
#ifdef _MSC_VER
    #pragma comment(lib, "OpenGL32.lib")
#endif
#elif defined(_IRR_OSX_PLATFORM_)
    #include <OpenGL/gl.h>
#elif defined(_IRR_COMPILE_WITH_SDL_DEVICE_)
    #define NO_SDL_GLEXT
    #include <SDL/SDL_video.h>
    #include <SDL/SDL_opengl.h>
#else
    #if defined(_IRR_OSX_PLATFORM_)
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
    #endif
#endif


namespace irr
{
namespace video
{
class COpenGLDriver;

//! OpenGL texture.
/**
 * @brief COpenGLTexture implements ITexture for OpenGL rendering.
 * This class manages OpenGL texture objects, including creation from images,
 * mipmap generation, render target support, and hardware texture binding.
 */
class COpenGLTexture : public ITexture
{
public:

    /** @brief Constructor
     * @param surface The image surface to create the texture from
     * @param name The name/path of the texture
     * @param mipmapData Optional pointer to pre-generated mipmap data
     * @param driver Pointer to the OpenGL driver instance
     */
    COpenGLTexture(IImage *surface, const io::path &name, void *mipmapData = 0, COpenGLDriver *driver = 0);

    //! destructor
    virtual ~COpenGLTexture();

    /** @brief Lock the texture for CPU access
     * @param mode Lock mode - read, write, or both
     * @param mipmapLevel Which mip level to lock (0 = base)
     * @return Pointer to the locked texture data
     */
    virtual void* lock(E_TEXTURE_LOCK_MODE mode = ETLM_READ_WRITE, u32 mipmapLevel = 0);

    //! unlock function
    virtual void unlock();

    //! Returns original size of the texture (image).
    virtual const core::dimension2d<u32>&getOriginalSize() const;

    //! Returns size of the texture.
    virtual const core::dimension2d<u32>&getSize() const;

    //! returns driver type of texture (=the driver, that created it)
    virtual E_DRIVER_TYPE getDriverType() const;

    //! returns color format of texture
    virtual ECOLOR_FORMAT getColorFormat() const;

    //! returns pitch of texture (in bytes)
    virtual u32 getPitch() const;

    //! return open gl texture name
    GLuint getOpenGLTextureName() const;

    //! return whether this texture has mipmaps
    virtual bool hasMipMaps() const;

    /** @brief Regenerates the mip map levels of the texture.
     * Useful after locking and modifying the texture
     * @param mipmapData Pointer to raw mipmap data, including all necessary mip levels, 
     *        in the same format as the main texture image. If not set the mipmaps are 
     *        derived from the main image.
     */
    virtual void regenerateMipMapLevels(void *mipmapData = 0);

    //! Is it a render target?
    virtual bool isRenderTarget() const;

    //! Is it a FrameBufferObject?
    virtual bool isFrameBufferObject() const;

    //! Bind RenderTargetTexture
    virtual void bindRTT();

    //! Unbind RenderTargetTexture
    virtual void unbindRTT();

    /** @brief Sets whether this texture is intended to be used as a render target.
     * @param isTarget True to enable render target mode
     */
    void setIsRenderTarget(bool isTarget);

protected:

    /** @brief Protected constructor for derived classes
     * Creates the object with basic setup but does not create an OpenGL texture name.
     * @param name The texture name/path
     * @param driver Pointer to the OpenGL driver
     */
    COpenGLTexture(const io::path &name, COpenGLDriver *driver);

    /** @brief Get the desired color format based on texture creation flags
     * @param format The source color format
     * @return The optimal color format for OpenGL
     */
    ECOLOR_FORMAT getBestColorFormat(ECOLOR_FORMAT format);

    /** @brief Get OpenGL color format parameters from Irrlicht color format
     * @param format The Irrlicht color format
     * @param filtering Reference to store filtering mode
     * @param colorformat Reference to store OpenGL color format
     * @param type Reference to store OpenGL pixel type
     */
    GLint getOpenGLFormatAndParametersFromColorFormat(
        ECOLOR_FORMAT format, GLint &filtering, GLenum &colorformat, GLenum &type);

    /** @brief Get important properties from the image
     * @param image The source image to analyze
     */
    void getImageValues(IImage *image);

    /** @brief Upload texture data to OpenGL
     * @param newTexture True if called for newly created texture first time
     * @param mipmapData Pointer to raw mipmap data
     * @param mipLevel If non-zero, only that specific miplevel is updated
     */
    void uploadTexture(bool newTexture = false, void *mipmapData = 0, u32 mipLevel = 0);

    core::dimension2d<u32> ImageSize;
    core::dimension2d<u32> TextureSize;
    ECOLOR_FORMAT          ColorFormat;
    COpenGLDriver          *Driver;
    IImage                 *Image;
    IImage                 *MipImage;

    GLuint TextureName;
    GLint  InternalFormat;
    GLenum PixelFormat;
    GLenum PixelType;

    u8   MipLevelStored;
    bool HasMipMaps;
    bool MipmapLegacyMode;
    bool IsRenderTarget;
    bool AutomaticMipmapUpdate;
    bool ReadOnlyLock;
    bool KeepImage;
};

// ! OpenGL FBO texture.
class COpenGLFBOTexture : public COpenGLTexture
{
public:

    // ! FrameBufferObject constructor
    COpenGLFBOTexture(const core::dimension2d<u32> &size, const io::path &name,
                      COpenGLDriver *driver = 0, ECOLOR_FORMAT format = ECOLOR_FORMAT::ECF_UNKNOWN);

    // ! destructor
    virtual ~COpenGLFBOTexture();

    // ! Is it a FrameBufferObject?
    virtual bool isFrameBufferObject() const;

    // ! Bind RenderTargetTexture
    virtual void bindRTT();

    // ! Unbind RenderTargetTexture
    virtual void unbindRTT();

    ITexture *DepthTexture;
protected:
    GLuint ColorFrameBuffer;
};


// ! OpenGL FBO depth texture.
class COpenGLFBODepthTexture : public COpenGLTexture
{
public:
    // ! FrameBufferObject depth constructor
    COpenGLFBODepthTexture(const core::dimension2d<u32> &size, const io::path &name, COpenGLDriver *driver = 0, bool useStencil = false);

    // ! destructor
    virtual ~COpenGLFBODepthTexture();

    // ! Bind RenderTargetTexture
    virtual void bindRTT();

    // ! Unbind RenderTargetTexture
    virtual void unbindRTT();

    bool attach(ITexture*);

protected:
    GLuint DepthRenderBuffer;
    GLuint StencilRenderBuffer;
    bool   UseStencil;
};
}     // end namespace video
} // end namespace irr
#endif
#endif // _IRR_COMPILE_WITH_OPENGL_