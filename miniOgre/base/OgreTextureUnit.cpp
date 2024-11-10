#include "OgreHeader.h"
#include "OgreTextureUnit.h"
#include "OgreTextureManager.h"
#include "OgreControllerManager.h"
#include "OgreMaterial.h"
#include "OgreRoot.h"

void TextureAnimationControllerValue::setValue(Ogre::Real value)
{
    mTexUnit->addTime(value);
}

TextureUnit::TextureUnit(Ogre::Material* owner):
    mRotate(0),
    mCurrentRotate(0),
    mUMod(0.0f),
    mVMod(0.0f),
    mU(0.0f),
    mV(0.0f),
    mUScale(1.0f),
    mVScale(1.0f)
{
    mOwner = owner;
}


TextureUnit::~TextureUnit()
{
    Ogre::ControllerManager& controllerManager = Ogre::ControllerManager::getSingleton();
    if(mAnimController)
        controllerManager.destroyController(mAnimController);
}

void TextureUnit::setTexture(std::shared_ptr<OgreTexture> tex)
{
    if (mTextures.empty())
    {
        mTextures.push_back(tex);
    }
    else
    {
        mTextures[0] = tex;
    }
}

void TextureUnit::setTexture(uint32_t index, std::shared_ptr<OgreTexture> tex)
{
    if (index >= mTextures.size())
    {
        mTextures.push_back(tex);
    }
    else
    {
        mTextures[index] = tex;
    }
}

bool TextureUnit::updateTexture(uint32_t index, const std::string& texName)
{
    if (texName == "simhei12_auto_glyph_images_32")
    {
        int kk = 0;
    }
    if (mNameList[index] != texName)
    {
        mNameList[index] = texName;

        if (mLoad)
        {
            mTextures[index] = TextureManager::getSingleton().load(texName, nullptr);
        }
       
    }
    else
    {
        int kk = 0;
    }

    return true;
    
}

void TextureUnit::setTexture(const std::string& name, Ogre::TextureProperty* texProperty)
{
    if (name == "simhei12_auto_glyph_images_32")
    {
        int kk = 0;
    }
    mNameList.clear();
    mNameList.push_back(name);
    if (texProperty)
    {
        mTextureProperty = *texProperty;
    }
}

void TextureUnit::setAnimtexture(const std::vector<String>& namelist, float duration)
{
    mNameList = namelist;
    mAnimDuration = duration;
    mUseAnimation = true;
}

void TextureUnit::setTextureUScale(float u)
{
    mUScale = u;
    mRecalcTexMatrix = true;
}
void TextureUnit::setTextureVScale(float v)
{
    mVScale = v;
    mRecalcTexMatrix = true;
}

void TextureUnit::setTextureScale(float u, float v)
{
    mUScale = u;
    mVScale = v;
    mRecalcTexMatrix = true;
}

void TextureUnit::setTextureUScroll(float u)
{
    mUMod = u;
    mRecalcTexMatrix = true;
    mUseScroll = true;
}

void TextureUnit::setTextureVScroll(float v)
{
    mVMod = v;
    mRecalcTexMatrix = true;
    mUseScroll = true;
}

void TextureUnit::setTextureScroll(float u, float v)
{
    mUMod = u;
    mVMod = v;
    mRecalcTexMatrix = true;
    mUseScroll = true;
}

void TextureUnit::setTextureRotate(const Ogre::Radian& angle)
{
    mRotate = angle;
    mRecalcTexMatrix = true;
}

void TextureUnit::preLoad()
{
    
}
void TextureUnit::_load(utils::JobSystem::Job* job)
{
    if (job)
    {
        assert(false);
       
    }
    else
    {
        mTextures.reserve(mNameList.size());
        for (auto& name : mNameList)
        {
            auto tex = TextureManager::getSingletonPtr()->load(name, &mTextureProperty, false);
            mTextures.push_back(tex);
        }


        for (auto& tex : mTextures)
        {
            tex->load(job);
        }

        if (mUseAnimation || mUseScroll || mRotate != Ogre::Radian(0))
        {
            if (!mControllerOwner)
                mControllerOwner = std::make_shared<TextureAnimationControllerValue>(this);
            Ogre::ControllerManager& controllerManager = Ogre::ControllerManager::getSingleton();
            mAnimController = controllerManager.createFrameTimePassthroughController(
                mControllerOwner);
        }

        mLoad = true;
    }
    
}

void TextureUnit::_unload()
{
    if (mAnimController)
    {
        ControllerManager::getSingleton().destroyController(mAnimController);
        mAnimController = nullptr;
    }

    for (auto itor = mTextures.begin(); itor != mTextures.end(); itor++)
    {
        String TexName;
        if (!(*itor))
            TexName = (*itor)->getName();
        (*itor).reset();
        if (Ogre::TextureManager::getSingletonPtr()->unloadUnreferencedResource(TexName))
        {
            Ogre::TextureManager::getSingletonPtr()->remove(TexName);
        }
                
    }
}

void TextureUnit::updateResourceState()
{
    
    
    
}

std::shared_ptr<OgreTexture> TextureUnit::getTexture()
{
    return mTextures[mTextureIndex];
}


OgreTexture* TextureUnit::getRaw()
{
    return mTextures[mTextureIndex].get();
}

TextureProperty* TextureUnit::getTextureProperty()
{
    return &mTextureProperty;
}

std::shared_ptr<TextureUnit> TextureUnit::clone(Ogre::Material* owner)
{
    auto& name = owner->getName();
    if (name == "SkyLan")
    {
        int kk = 0;
    }
    std::shared_ptr<TextureUnit> tu = std::make_shared<TextureUnit>(owner);

    tu->mNameList = mNameList;
    tu->mTextureProperty = mTextureProperty;
    tu->mAnimDuration = mAnimDuration;
    tu->mRotate = mRotate;
    tu->mUseScroll = mUseScroll;
    tu->mU = mU;
    tu->mV = mV;
    tu->mUScale = mUScale;
    tu->mVScale = mVScale;
    return tu;
}

void TextureUnit::setColourOperationEx(
    LayerBlendOperationEx op,
    LayerBlendSource source1 ,
    LayerBlendSource source2 ,

    const ColourValue& arg1 ,
    const ColourValue& arg2 ,

    Real manualBlend)
{

}

void TextureUnit::setProjectiveTexturing(bool enable,
    const Frustum* projectionSettings)
{
    if (enable)
    {
        TextureEffect eff;
        eff.type = ET_PROJECTIVE_TEXTURE;
        eff.frustum = projectionSettings;
        addEffect(eff);
    }
    else
    {
        removeEffect(ET_PROJECTIVE_TEXTURE);
    }
}

void TextureUnit::addEffect(TextureEffect& effect)
{
    // Ensure controller pointer is null
    effect.controller = 0;

    if (effect.type == ET_ENVIRONMENT_MAP
        || effect.type == ET_UVSCROLL
        || effect.type == ET_USCROLL
        || effect.type == ET_VSCROLL
        || effect.type == ET_ROTATE
        || effect.type == ET_PROJECTIVE_TEXTURE)
    {
        // Replace - must be unique
        // Search for existing effect of this type
        auto i = mEffects.find(effect.type);
        if (i != mEffects.end())
        {
            // Destroy old effect controller if exist
            if (i->second.controller)
            {
                ControllerManager::getSingleton().destroyController(i->second.controller);
            }

            mEffects.erase(i);
        }
    }

    if (isLoaded())
    {
        // Create controller
        createEffectController(effect);
    }

    // Record new effect
    mEffects.insert(EffectMap::value_type(effect.type, effect));

}

//-----------------------------------------------------------------------
void TextureUnit::createEffectController(TextureEffect& effect)
{
    assert(effect.controller == 0);
    ControllerManager& cMgr = ControllerManager::getSingleton();
    switch (effect.type)
    {
    case ET_UVSCROLL:

        break;
    case ET_USCROLL:
  
        break;
    case ET_VSCROLL:

        break;
    case ET_ROTATE:

        break;
    case ET_TRANSFORM:

        break;
    case ET_ENVIRONMENT_MAP:
        break;
    default:
        break;
    }
}

void TextureUnit::removeEffect(const TextureEffectType type)
{
    // Get range of items matching this effect
    std::pair< EffectMap::iterator, EffectMap::iterator > remPair =
        mEffects.equal_range(type);
    // Remove controllers
    for (EffectMap::iterator i = remPair.first; i != remPair.second; ++i)
    {
        if (i->second.controller)
        {
            ControllerManager::getSingleton().destroyController(i->second.controller);
        }
    }
    // Erase         
    mEffects.erase(remPair.first, remPair.second);
}

int32_t TextureUnit::getNumFrames()
{
    return 1;
}

unsigned int TextureUnit::getCurrentFrame(void) const
{
    return 0;
}

void TextureUnit::setCurrentFrame(unsigned int frameNumber)
{

}



void TextureUnit::addTime(float delta)
{
    if (mTextures.size() > 1)
    {
        float fragment = mAnimDuration / mTextures.size();

        mAnimAccumulate += delta;

        float exist = (mTextureIndex + 1) * fragment;

        if (mAnimAccumulate >= exist)
        {
            mTextureIndex++;
            if (mTextureIndex >= mTextures.size())
            {
                mTextureIndex = 0;
            }

            if (mAnimAccumulate > mAnimDuration)
            {
                mAnimAccumulate -= mAnimDuration;
            }

            assert(false);
        }

    }

    if (mUseScroll)
    {
        mU += mUMod * delta;
        mV += mVMod * delta;
        
    }

    mRecalcTexMatrix = true;

    mCurrentRotate += mRotate * delta;
}

float TextureUnit::getAnimationDuration()
{
    return mAnimDuration;
}

String TextureUnit::getFrameTextureName(uint32_t index)
{
    return mNameList[index];
}

void TextureUnit::setAnimatedTextureName(
    const String& name,
    unsigned int numFrames,
    Real duration)
{
    String ext;
    String baseName;

    size_t pos = name.find_last_of(".");
    baseName = name.substr(0, pos);
    ext = name.substr(pos);

    mNameList.resize(numFrames);
    mTextures.resize(numFrames);

    mAnimDuration = duration;
    mTextureIndex = 0;

    for (unsigned int i = 0; i < numFrames; ++i)
    {
        std::ostringstream str;
        str << baseName << "_" << i << ext;
        mNameList[i] = str.str();
        mTextures[i].reset();
    }

    // Load immediately if Material loaded
    if (isLoaded())
    {
        _load(nullptr);
    }
}

bool TextureUnit::isLoaded()
{
    return mResourceState == ResourceState::READY;
}

const Matrix4& TextureUnit::getTextureTransform() const
{
    if (mRecalcTexMatrix)
        recalcTextureMatrix();
    return mTexModMatrix;
}

void TextureUnit::recalcTextureMatrix() const
{
    mTexModMatrix = Ogre::Matrix4::IDENTITY;
    if (mUScale != 1 || mVScale != 1)
    {
        // Offset to center of texture
        mTexModMatrix[0][0] = 1 / mUScale;
        mTexModMatrix[1][1] = 1 / mVScale;
        // Skip matrix concat since first matrix update
        mTexModMatrix[0][3] = (-0.5f * mTexModMatrix[0][0]) + 0.5f;
        mTexModMatrix[1][3] = (-0.5f * mTexModMatrix[1][1]) + 0.5f;
    }

    if (mUseScroll)
    {
        mTexModMatrix.setTrans(Ogre::Vector3(mU, mV, 0));
    }

    if (mRotate != Radian(0))
    {
        Ogre::Matrix4 rot = Ogre::Matrix4::IDENTITY;
        Radian theta(mCurrentRotate);
        Real cosTheta = Math::Cos(theta);
        Real sinTheta = Math::Sin(theta);

        rot[0][0] = cosTheta;
        rot[0][1] = -sinTheta;
        rot[1][0] = sinTheta;
        rot[1][1] = cosTheta;
        // Offset center of rotation to center of texture
        rot[0][3] = 0.5f + ((-0.5f * cosTheta) - (-0.5f * sinTheta));
        rot[1][3] = 0.5f + ((-0.5f * sinTheta) + (-0.5f * cosTheta));

        mTexModMatrix = rot * mTexModMatrix;
    }

    mRecalcTexMatrix = false;

}