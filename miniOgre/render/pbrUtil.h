#pragma once
#include "OgreCommon.h"
#include "OgrePixelFormat.h"

namespace Ogre
{
    Ogre::RenderTarget* generateCubeMap(
        const std::string& name,
        Ogre::OgreTexture* environmentCube,
        Ogre::PixelFormat format,
        int32_t dim,
        CubeType type);
   Ogre::RenderTarget* generateBRDFLUT(const std::string& name);
}