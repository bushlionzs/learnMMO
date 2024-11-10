/***********************************************************************
    created:    Tue Feb 17 2009
    author:     Paul D Turner
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2011 Paul D Turner & The CEGUI Development Team
 *
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/
#include <OgreHeader.h>
#include "CEGUI/RendererModules/Ogre/Texture.h"
#include "CEGUI/Exceptions.h"
#include "CEGUI/System.h"
#include "CEGUI/RendererModules/Ogre/ImageCodec.h"
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRoot.h>
#include <OgreImage.h>

// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
// helper function to return byte size of image of given size in given format
static size_t calculateDataSize(const Sizef size, Texture::PixelFormat fmt)
{
    switch (fmt)
    {
    case Texture::PF_RGBA:
        return size.d_width * size.d_height * 4;

    case Texture::PF_RGB:
        return size.d_width * size.d_height * 3;

    case Texture::PF_RGB_565:
    case Texture::PF_RGBA_4444:
        return size.d_width * size.d_height * 2;

    case Texture::PF_PVRTC2:
        return (static_cast<size_t>(size.d_width * size.d_height) * 2 + 7) / 8;

    case Texture::PF_PVRTC4:
        return (static_cast<size_t>(size.d_width * size.d_height) * 4 + 7) / 8;

    case Texture::PF_RGBA_DXT1:
        return std::ceil(size.d_width / 4) * std::ceil(size.d_height / 4) * 8;

    case Texture::PF_RGBA_DXT3:
    case Texture::PF_RGBA_DXT5:
        return std::ceil(size.d_width / 4) * std::ceil(size.d_height / 4) * 16;

    default:
        return 0;
    }
}

//----------------------------------------------------------------------------//
uint32 OgreTexture::d_textureNumber = 0;

//----------------------------------------------------------------------------//
OgreTexture::OgreTexture(const String& name) :
    d_isLinked(false),
    d_size(0, 0),
    d_dataSize(0, 0),
    d_texelScaling(0, 0),
    d_name(name)
{
    d_tex_name = "white1x1.dds";
}

//----------------------------------------------------------------------------//
OgreTexture::OgreTexture(const String& name, const String& filename,
                         const String& resourceGroup) :
    d_isLinked(false),
    d_size(0, 0),
    d_dataSize(0, 0),
    d_texelScaling(0, 0),
    d_name(name)
{
    d_tex_name = filename;
    ImageInfo imageInfo;

    TextureManager::getSingleton().getImageInfo(std::string(filename.c_str()), imageInfo, false);

    d_size.d_width = imageInfo.width;
    d_size.d_height = imageInfo.height;
    d_dataSize = d_size;

    updateCachedScaleValues();
}

//----------------------------------------------------------------------------//
OgreTexture::OgreTexture(const String& name, const Sizef& sz) :
    d_isLinked(false),
    d_size(0, 0),
    d_dataSize(0, 0),
    d_texelScaling(0, 0),
    d_name(name)
{
    std::string strName = name.c_str();
    d_tex_name = name;
    TextureProperty texProperty;
    texProperty._width = sz.d_width;
    texProperty._height = sz.d_height;
    texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
    TexturePtr tex = TextureManager::getSingleton().createManual(strName, texProperty);

    d_size.d_width = sz.d_width;
    d_size.d_height = sz.d_height;
    d_dataSize = sz;
    updateCachedScaleValues();
}

//----------------------------------------------------------------------------//
OgreTexture::OgreTexture(const String& name, const String& filename,
                         bool take_ownership) :
    d_isLinked(false),
    d_size(0, 0),
    d_dataSize(0, 0),
    d_texelScaling(0, 0),
    d_name(name)
{
    d_tex_name = filename;

    ImageInfo imageInfo;
    bool b = Ogre::TextureManager::getSingleton().getImageInfo(filename.c_str(), imageInfo);
    assert(b);

    d_size.d_width = imageInfo.width;
    d_size.d_height = imageInfo.height;
    d_dataSize = d_size;

    updateCachedScaleValues();

}

//----------------------------------------------------------------------------//
OgreTexture::~OgreTexture()
{
    freeOgreTexture();
}

//----------------------------------------------------------------------------//
const String& OgreTexture::getName() const
{
    return d_name;
}

//----------------------------------------------------------------------------//
const Sizef& OgreTexture::getSize() const
{
    return d_size;
}

//----------------------------------------------------------------------------//
const Sizef& OgreTexture::getOriginalDataSize() const
{
    return d_dataSize;
}

//----------------------------------------------------------------------------//
const Vector2f& OgreTexture::getTexelScaling() const
{
    return d_texelScaling;
}

//----------------------------------------------------------------------------//
void OgreTexture::loadFromFile(const String& filename,
                               const String& resourceGroup)
{
    // get and check existence of CEGUI::System object
    System* sys = System::getSingletonPtr();
    if (!sys)
        CEGUI_THROW(RendererException(
            "CEGUI::System object has not been created!"));

    d_texture = TextureManager::getSingleton().load(filename.c_str(), nullptr);
    d_size.d_width = d_texture->getWidth();
    d_size.d_height = d_texture->getHeight();
    d_dataSize = d_size;

    updateCachedScaleValues();
}

//----------------------------------------------------------------------------//
void OgreTexture::loadFromMemory(const void* buffer, const Sizef& buffer_size,
                                 PixelFormat pixel_format)
{
    using namespace Ogre;

    if (!isPixelFormatSupported(pixel_format))
        CEGUI_THROW(InvalidRequestException(
            "Data was supplied in an unsupported pixel format."));

    const size_t byte_size = calculateDataSize(buffer_size, pixel_format);

    //char* bufferCopy = new char[byte_size];
    //memcpy(bufferCopy, buffer, byte_size);

    Ogre::PixelBox pixelBox(buffer_size.d_width, buffer_size.d_height,
                                                       1, toOgrePixelFormat(pixel_format), (void*)buffer);

    std::string name = d_tex_name.c_str();

    auto tex = TextureManager::getSingleton().getByName(name);
    if (tex)
    {
        auto pTex = tex.get();
        if(pTex->getWidth() != buffer_size.d_width ||
            pTex->getHeight() != buffer_size.d_height)
            {
                pTex->freeInternalResources();
                pTex->setWidth(buffer_size.d_width);
                pTex->setHeight(buffer_size.d_height);
                pTex->createInternalResources();
                
            }
        pTex->getBuffer(0, 0)->blitFromMemory(pixelBox);
        pTex->postLoad();
        d_size.d_width = buffer_size.d_width;
        d_size.d_height = buffer_size.d_height;
        d_dataSize = buffer_size;
        updateCachedScaleValues();
    }

    //// throw exception if no texture was able to be created
    //if (!tex)
    //    CEGUI_THROW(RendererException(
    //        "Failed to blit to Texture from memory."));

    
}

//----------------------------------------------------------------------------//
void OgreTexture::blitFromMemory(const void* sourceData, const Rectf& area)
{
    if (!d_texture) // TODO: exception?
        return;

    // NOTE: const_cast because Ogre takes pointer to non-const here. Rather
    // than allow that to dictate poor choices in our own APIs, we choose to
    // address the issue as close to the source of the problem as possible.
    Ogre::PixelBox pb(area.getWidth(), area.getHeight(),
                      1, Ogre::PF_A8R8G8B8, const_cast<void*>(sourceData));
    Ogre::Box box(area.left(), area.top(), area.right(), area.bottom());
    d_texture->getBuffer()->blitFromMemory(pb, box);
}

//----------------------------------------------------------------------------//
void OgreTexture::blitToMemory(void* targetData)
{
    if (!d_texture) // TODO: exception?
        return;

    Ogre::PixelBox pb(d_size.d_width, d_size.d_height,
                      1, Ogre::PF_A8R8G8B8, targetData);
    d_texture->getBuffer()->blitToMemory(pb);
}

//----------------------------------------------------------------------------//
void OgreTexture::freeOgreTexture()
{
    if (d_texture && !d_isLinked)
        Ogre::TextureManager::getSingleton().remove(d_texture->getName());

    d_texture.reset();
}

//----------------------------------------------------------------------------//
Ogre::String OgreTexture::getUniqueName()
{
        std::stringstream strstream;
        strstream << "_cegui_ogre_" << d_textureNumber++;

        return strstream.str();
}

//----------------------------------------------------------------------------//
void OgreTexture::updateCachedScaleValues()
{
    //
    // calculate what to use for x scale
    //
    const float orgW = d_dataSize.d_width;
    const float texW = d_size.d_width;

    // if texture and original data width are the same, scale is based
    // on the original size.
    // if texture is wider (and source data was not stretched), scale
    // is based on the size of the resulting texture.
    d_texelScaling.d_x = 1.0f / ((orgW == texW) ? orgW : texW);

    //
    // calculate what to use for y scale
    //
    const float orgH = d_dataSize.d_height;
    const float texH = d_size.d_height;

    // if texture and original data height are the same, scale is based
    // on the original size.
    // if texture is taller (and source data was not stretched), scale
    // is based on the size of the resulting texture.
    d_texelScaling.d_y = 1.0f / ((orgH == texH) ? orgH : texH);
}

//----------------------------------------------------------------------------//
void OgreTexture::setOgreTexture(Ogre::TexturePtr texture, bool take_ownership)
{
    freeOgreTexture();

    d_texture = texture;
    d_isLinked = !take_ownership;

    if (d_texture)
    {
        d_size.d_width = d_texture->getWidth();
        d_size.d_height= d_texture->getHeight();
        d_dataSize = d_size;
    }
    else
        d_size = d_dataSize = Sizef(0, 0);

    updateCachedScaleValues();
}

//----------------------------------------------------------------------------//
Ogre::TexturePtr OgreTexture::getOgreTexture() const
{
    return d_texture;
}

//----------------------------------------------------------------------------//
bool OgreTexture::isPixelFormatSupported(const PixelFormat fmt) const
{
    return true;//zhousha
}

//----------------------------------------------------------------------------//
Ogre::PixelFormat OgreTexture::toOgrePixelFormat(const Texture::PixelFormat fmt)
{
    switch (fmt)
    {
        case Texture::PF_RGBA:       return Ogre::PF_A8B8G8R8;
        case Texture::PF_RGB:        return Ogre::PF_B8G8R8;
        case Texture::PF_RGB_565:    return Ogre::PF_R5G6B5;
        case Texture::PF_RGBA_4444:  return Ogre::PF_A4R4G4B4;
        case Texture::PF_PVRTC2:     return Ogre::PF_PVRTC_RGBA2;
        case Texture::PF_PVRTC4:     return Ogre::PF_PVRTC_RGBA4;
        case Texture::PF_RGBA_DXT1:  return Ogre::PF_DXT1;
        case Texture::PF_RGBA_DXT3:  return Ogre::PF_DXT3;
        case Texture::PF_RGBA_DXT5:  return Ogre::PF_DXT5;
        
        default:
            CEGUI_THROW(InvalidRequestException(
                "Invalid pixel format translation."));
    }
}

//----------------------------------------------------------------------------//
Texture::PixelFormat OgreTexture::fromOgrePixelFormat(
                                            const Ogre::PixelFormat fmt)
{
    switch (fmt)
    {
        case Ogre::PF_A8R8G8B8:     return Texture::PF_RGBA;
        case Ogre::PF_A8B8G8R8:     return Texture::PF_RGBA;
        case Ogre::PF_R8G8B8:       return Texture::PF_RGB;
        case Ogre::PF_B8G8R8:       return Texture::PF_RGB;
        case Ogre::PF_R5G6B5:       return Texture::PF_RGB_565;
        case Ogre::PF_A4R4G4B4:     return Texture::PF_RGBA_4444;
        case Ogre::PF_PVRTC_RGBA2:  return Texture::PF_PVRTC2;
        case Ogre::PF_PVRTC_RGBA4:  return Texture::PF_PVRTC4;
        case Ogre::PF_DXT1:         return Texture::PF_RGBA_DXT1;
        case Ogre::PF_DXT3:         return Texture::PF_RGBA_DXT3;
        case Ogre::PF_DXT5:         return Texture::PF_RGBA_DXT5;
        
        default:
            CEGUI_THROW(InvalidRequestException(
                "Invalid pixel format translation."));
    }
}



//----------------------------------------------------------------------------//

} // End of  CEGUI namespace section
