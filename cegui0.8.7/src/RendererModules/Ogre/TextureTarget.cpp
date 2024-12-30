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
#include "CEGUI/RendererModules/Ogre/TextureTarget.h"
#include "CEGUI/RendererModules/Ogre/Texture.h"
#include "CEGUI/PropertyHelper.h"

#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreRenderTexture.h>
#include <RenderSystem.h>
#include <OgreViewport.h>


// Start of CEGUI namespace section
namespace CEGUI
{
//----------------------------------------------------------------------------//
const float OgreTextureTarget::DEFAULT_SIZE = 128.0f;
uint OgreTextureTarget::s_textureNumber = 0;

//----------------------------------------------------------------------------//
OgreTextureTarget::OgreTextureTarget(OgreRenderer& owner,
                                     RenderSystem& rs) :
    OgreRenderTarget<TextureTarget>(owner, rs),
    d_CEGUITexture(0)
{
    d_CEGUITexture = static_cast<OgreTexture*>(
        &d_owner.createTexture(generateTextureName()));

    // setup area and cause the initial texture to be generated.
    declareRenderSize(Sizef(DEFAULT_SIZE, DEFAULT_SIZE));
}

//----------------------------------------------------------------------------//
OgreTextureTarget::~OgreTextureTarget()
{
    d_owner.destroyTexture(*d_CEGUITexture);
}

//----------------------------------------------------------------------------//
bool OgreTextureTarget::isImageryCache() const
{
    return true;
}

//----------------------------------------------------------------------------//
void OgreTextureTarget::clear()
{
    if (!d_viewportValid)
        updateViewport();
 

}

//----------------------------------------------------------------------------//
Texture& OgreTextureTarget::getTexture() const
{
    return *d_CEGUITexture;
}

//----------------------------------------------------------------------------//
void OgreTextureTarget::declareRenderSize(const Sizef& sz)
{
    // exit if current size is enough
    if ((d_area.getWidth() >= sz.d_width) && (d_area.getHeight() >=sz.d_height))
        return;

    TextureProperty tex;
    tex._tex_format = Ogre::PF_A8R8G8B8;
    tex._tex_usage = Ogre::TextureUsage::COLOR_ATTACHMENT;
    tex._width = sz.d_width;
    tex._height = sz.d_height;
    Ogre::TexturePtr rttTex = Ogre::TextureManager::getSingleton().createManual(
        OgreTexture::getUniqueName(),
        tex);

    //d_renderTarget = rttTex->getBuffer()->getRenderTarget();
    assert(false);
    Rectf init_area(
        Vector2f(0, 0),
        Sizef(d_renderTarget->getWidth(), d_renderTarget->getHeight())
    );

    setArea(init_area);

#ifdef CEGUI_USE_OGRE_COMPOSITOR2
    // Setting this should properly change everything
    d_renderTargetUpdated = true;
#else
    // delete viewport and reset ptr so a new one is generated.  This is
    // required because we have changed d_renderTarget so need a new VP also.
    OGRE_DELETE d_viewport;
    d_viewport = 0;
#endif    



    // because Texture takes ownership, the act of setting the new ogre texture
    // also ensures any previous ogre texture is released.
    d_CEGUITexture->setOgreTexture(rttTex, true);

    clear();
}

//----------------------------------------------------------------------------//
bool OgreTextureTarget::isRenderingInverted() const
{
    return false;
}

//----------------------------------------------------------------------------//
String OgreTextureTarget::generateTextureName()
{
    String tmp("_ogre_tt_tex_");
    tmp.append(PropertyHelper<uint>::toString(s_textureNumber++));

    return tmp;
}

//----------------------------------------------------------------------------//

} // End of  CEGUI namespace section

//----------------------------------------------------------------------------//
// Implementation of template base class
#include "./RenderTarget.inl"

