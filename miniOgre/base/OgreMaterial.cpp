#include "OgreHeader.h"
#include "OgreMaterial.h"
#include "OgreTextureUnit.h"
#include "OgreMaterialManager.h"
#include "VideoManager.h"
#include "shaderManager.h"
#include "OgreResourceManager.h"
#include "glslUtil.h"
#include <OgreRoot.h>
#include <renderSystem.h>
#include <filament/DriverEnums.h>
#include <filament/DriverBase.h>

namespace Ogre {

    Material::Material(const std::string& name, bool pbr)
    {
        mMaterialName = name;
        mPbr = pbr;
        mShaderInfo.uboVertexMask = 1 | 2 | 8;
        mShaderInfo.uboFragMask = 1 | 2 | 4;
        mShaderInfo.samplerFragMask = 1 | 2 | 4 | 8 | 16;

        mRasterState.depthWrite = true;
        mRasterState.depthTest = true;
        mRasterState.depthFunc = SamplerCompareFunc::GE;
        mRasterState.colorWrite = true;
        mRasterState.pixelFormat = Ogre::PixelFormat::PF_A8R8G8B8_SRGB;
        mRasterState.renderTargetCount = 1;
    }


    Material::~Material()
    {

    }

    uint32_t Material::addTexture(const std::string& name, Ogre::TextureProperty* texProperty)
    {
        std::shared_ptr<TextureUnit> tu(new TextureUnit(this));

        tu->setTexture(name, texProperty);

        mTextureUnits.push_back(tu);
        return mTextureUnits.size() - 1;
    }

    uint32_t Material::addAnimTexture(const std::vector<String>& namelist, float duration)
    {
        std::shared_ptr<TextureUnit> tu(new TextureUnit(this));

        tu->setAnimtexture(namelist, duration);

        mTextureUnits.push_back(tu);

        return mTextureUnits.size() - 1;
    }

    uint32_t Material::addTexture(const Ogre::TexturePtr& tex)
    {
        std::shared_ptr<TextureUnit> tu(new TextureUnit(this));
        tu->setTexture(tex);
        mTextureUnits.push_back(tu);
        return mTextureUnits.size() - 1;
    }

    void Material::preLoad()
    {
        if (mLoad)
        {
            return;
        }
        for (auto& it : mTextureUnits)
        {
            it->preLoad();
        }
    }

    void Material::load(utils::JobSystem::Job* job)
    {
        if (mLoad)
        {
            return;
        }

        for (auto& it : mTextureUnits)
        {
            if (!it->isLoaded())
            {
                it->_load(job);
            }
        }

        createFrameResourceInfo();

        mLoad = true;

    }

    void Material::updateResourceState()
    {
        if (mState == ResourceState::LOADING)
        {
            bool ready = true;
            for (auto& it : mTextureUnits)
            {
                it->updateResourceState();
                if (it->getResourceState() != ResourceState::READY)
                {
                    ready = false;
                }
            }

            if (ready)
            {
                mState = ResourceState::READY;
            }
        }
    }

    void Material::createFrameResourceInfo()
    {
        auto* rs = Ogre::Root::getSingleton().getRenderSystem();

        mProgramHandle = rs->createShaderProgram(mShaderInfo, mVertexDeclaration);
        mPipelineHandle = rs->createPipeline(mRasterState, mProgramHandle);

        mUboLayoutHandle = rs->getDescriptorSetLayout(mProgramHandle, 0);
        mSamplerLayoutHandle = rs->getDescriptorSetLayout(mProgramHandle, 1);
        
        if (mPbr)
        {
            for (int32_t i = 0; i < mTextureUnits.size(); i++)
            {
                switch (mTextureUnits[i]->getTextureProperty()->_pbrType)
                {
                case TextureTypePbr_MetalRoughness:
                    mPbrMatInfo.hasMetalRoughNessMap = 1;
                    break;
                case TextureTypePbr_NormalMap:
                    mPbrMatInfo.hasNormalMap = 1;
                    break;
                case TextureTypePbr_Emissive:
                    mPbrMatInfo.hasEmissiveMap = 1;
                    break;
                }
            }
        } 
    }

    bool Material::isLoaded()
    {
        return mLoad;
    }

    std::shared_ptr<Material> Material::clone(const String& name)
    {
        Material* mat = new Material(name);
        *mat = *this;

        return std::shared_ptr<Material>(mat);
    }

    std::shared_ptr<TextureUnit>& Material::getTextureUnit(uint32_t index)
    {
        return mTextureUnits[index];
    }

    std::shared_ptr<TextureUnit>& Material::getTextureUnit(const String& name)
    {
        assert(false);
        return mTextureUnits[0];
    }

    int32_t Material::getTextureUnitCount()
    {
        return mTextureUnits.size();
    }

    std::vector<std::shared_ptr<TextureUnit>>& Material::getAllTexureUnit()
    {
        return mTextureUnits;
    }

    void Material::addShader(ShaderInfo& sinfo)
    {
        mShaderInfo = sinfo;

        mShaderInfo.uboVertexMask = 1 | 2 | 8;
        mShaderInfo.uboFragMask = 1 | 2 | 4;
        mShaderInfo.samplerFragMask = 1 | 2 | 4 | 8 | 16;
    }

    ShaderInfo& Material::getShaderInfo()
    {
        return mShaderInfo;
    }

    void Material::scale(Real u, Real v)
    {
        mMatInfo.TexScale = Ogre::Math::makeScaleMatrix(Ogre::Vector3(u, v, 0.0f));
    }

    void Material::animation(Real u, Real v)
    {
        mAnimation = true;
        mUFactor = u;
        mVFactor = v;
    }

    PbrMaterialConstanceBuffer& Material::getPbrMatInfo()
    {
        return mPbrMatInfo;
    }

    GeneralMaterialConstantBuffer& Material::getMatInfo()
    {
        return mMatInfo;
    }

    void Material::update(Real delta)
    {
        if (!mAnimation)
            return;
        mUOffset += mUFactor * delta;
        mVOffset += mVFactor * delta;

        if (mUOffset >= 1.0f)
        {
            mUOffset -= 1.0f;
        }

        if (mVOffset >= 1.0f)
        {
            mVOffset -= 1.0f;
        }

        mMatInfo.TexTransform = Ogre::Math::makeTranslateMatrix(Ogre::Vector3(mUOffset, mVOffset, 0.0f)).transpose();
    }

    void Material::setFresnelR0(Ogre::Vector3& fresnelR0)
    {

    }

    void Material::setRoughness(Real roughness)
    {

    }

    void Material::setDiffuseAlbedo(Ogre::Vector4& diffuseAlbedo)
    {

    }

    bool Material::hasAnimation()
    {
        return mAnimation;
    }

    Ogre::Vector2 Material::getTexAnimationOffset()
    {
        return Ogre::Vector2(mUOffset, mVOffset);
    }

    const Ogre::ColourBlendState& Material::getBlendState() const
    {
        return mBlendState;
    }

    void Material::setBlendState(Ogre::ColourBlendState& state)
    {
        mBlendState = state;
    }

    void Material::setCullMode(backend::CullingMode mode)
    {
        
        mRasterState.culling = mode;
        
    }

    backend::CullingMode Material::getCullMode()
    {
        return mRasterState.culling;
    }

    Material& Material::operator=(const Material& rhs)
    {
        mAnimation = rhs.mAnimation;
        mUOffset = rhs.mUOffset;
        mVOffset = rhs.mVOffset;
        mUFactor = rhs.mUFactor;
        mVFactor = rhs.mVFactor;
        mBlendState = rhs.mBlendState;
        mShaderInfo = rhs.mShaderInfo;
        mPbrMatInfo = rhs.mPbrMatInfo;
        mMatInfo = rhs.mMatInfo;
        mPbr = rhs.mPbr;
        mLoad = false;
        for (auto tu : rhs.mTextureUnits)
        {
            mTextureUnits.push_back(tu->clone(this));
        }

        mRasterState = rhs.mRasterState;

        if (mRasterState.culling == backend::CullingMode::BACK)
        {
            int kk = 0;
        }
        return *this;
    }

    bool Material::isDepthTest()
    {
        return mRasterState.depthFunc != SamplerCompareFunc::A;
    }

    void Material::setDepthTest(bool test)
    {
        if (test)
        {
            mRasterState.depthFunc = SamplerCompareFunc::LE;
        }
        else
        {
            mRasterState.depthFunc = SamplerCompareFunc::A;
        }
    }

    bool Material::isWriteDepth()
    {
        return mRasterState.depthWrite;
    }
    void Material::setWriteDepth(bool bWrite)
    {
        mRasterState.depthWrite = bWrite;
    }

    bool Material::isTransparent()
    {
        return false;
    }
}