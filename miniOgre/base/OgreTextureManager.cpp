#include "OgreHeader.h"
#include "OgreTextureManager.h"
#include "renderSystem.h"
#include "OgreRoot.h"
#include "OgreResourceManager.h"
#include <utils/JobSystem.h>


namespace Ogre {
    template<> TextureManager* Ogre::Singleton<TextureManager>::msSingleton = 0;
    TextureManager::TextureManager()
    {
       
    }

    TextureManager::~TextureManager()
    {

    }

    
    std::shared_ptr<OgreTexture> TextureManager::load(
        const std::string& name, 
        TextureProperty* texProperty,
        bool read)
    {
        auto it = mTexMap.find(name);
        if (it != mTexMap.end())
        {
            return it->second;
        }

        OgreTexture* tmp = Ogre::Root::getSingleton().getRenderSystem()->createTextureFromFile(name, texProperty);

        if (tmp == nullptr)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "fail to create texture");
        }

        if (read)
        {
            tmp->load(nullptr);
        }
        std::shared_ptr<OgreTexture> tex(tmp);

        mTexMap[name] = tex;

        return tex;
    }

    std::shared_ptr<OgreTexture> TextureManager::loadImage(
        const String& name,
        const CImage& img,
        TextureType texType,
        int numMipmaps,
        Real gamma,
        bool isAlpha,
        PixelFormat desiredFormat,
        bool hwGammaCorrection)
    {
        OgreTexture* tmp = Ogre::Root::getSingleton().getRenderSystem()->createTextureFromFile(name, nullptr);

        tmp->setNumMipmaps(1000);

        tmp->loadImage(img);

        return std::shared_ptr<OgreTexture>(tmp);
    }

    bool TextureManager::addTexture(const String& name, OgreTexture* tex)
    {
        auto it = mTexMap.find(name);
        if (it != mTexMap.end())
        {
            return false;
        }

        mTexMap[name] = std::shared_ptr<OgreTexture>(tex);
        return true;
    }
    

    std::shared_ptr<OgreTexture> TextureManager::getByName(
        const String& name,
        const String& group) const
    {
        auto it = mTexMap.find(name);
        if (it != mTexMap.end())
        {
            return it->second;
        }

        return std::shared_ptr<OgreTexture>();
    }

    bool TextureManager::getImageInfo(const String& name, ImageInfo& info, bool cube)
    {
        auto it = mTexMap.find(name);
        if (it != mTexMap.end())
        {
            info.width = it->second->getWidth();
            info.height = it->second->getHeight();
            return true;
        }
        return CImage::loadImageInfo(name, info, cube);
    }

    void TextureManager::remove(const std::string& name)
    {

    }

    TexturePtr TextureManager::createManual(const String& name, TextureProperty& texProperty)
    {
        auto it = mTexMap.find(name);
        if (it != mTexMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "duplicated texture name");
        }

        OgreTexture* tmp = Ogre::Root::getSingleton().getRenderSystem()->createTextureFromFile(name, &texProperty);

        if (tmp == nullptr)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "fail to create texture");
        }

        tmp->load(nullptr);
        std::shared_ptr<OgreTexture> tex(tmp);

        mTexMap[name] = tex;

        return tex;
    }

    TexturePtr TextureManager::loadRawData(const String& name, DataStreamPtr& stream, TextureProperty& texProperty)
    {
        auto it = mTexMap.find(name);
        if (it != mTexMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "duplicated texture name");
        }

        OgreTexture* tmp = Ogre::Root::getSingleton().getRenderSystem()->createTextureFromFile(name, &texProperty);

        if (tmp == nullptr)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "fail to create texture");
        }

        tmp->loadRawData(stream, texProperty._width, texProperty._height, texProperty._tex_format);
        std::shared_ptr<OgreTexture> tex(tmp);

        mTexMap[name] = tex;

        return tex;
    }

    bool TextureManager::isFormatSupported(Ogre::TextureType ttype, Ogre::PixelFormat format, int usage)
    {
        return true;
    }

    bool TextureManager::unloadUnreferencedResource(const String& name)
    {
        assert(false);
        return true;
    }

    void TextureManager::updateTextures()
    {
        
        
    }
}