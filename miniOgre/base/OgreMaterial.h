#pragma once

#include <unordered_map>
#include "shader.h"
#include "engine_struct.h"
#include "OgreBlendMode.h"
#include "OgreCommon.h"
#include "OgreTexture.h"
#include <filament/Handle.h>
#include <filament/DriverEnums.h>
#include <filament/DriverBase.h>

class TextureUnit;
class VertexDeclaration;
class MaterialInfo
{
public:
    std::string mTexname;
    float mTexScale = 1.0f;
    bool mCube = false;
    std::string mVSname="basic.hlsl";
    std::string mPSname="basic.hlsl";
    GeneralMaterialConstantBuffer mMatInfo;
};

namespace Ogre {
    class Material
    {
    public:
        Material(const std::string& name, bool pbr = false);
        ~Material();

        uint32_t addTexture(const std::string& name, Ogre::TextureProperty* texProperty = nullptr);
        uint32_t addAnimTexture(const std::vector<String>& namelist, float duration);
        uint32_t addTexture(const Ogre::TexturePtr& tex);
        void preLoad();
        void load(utils::JobSystem::Job* job);
        bool isLoaded();

        std::shared_ptr<Material> clone(const String& name);
        std::shared_ptr<TextureUnit>& getTextureUnit(uint32_t index);
        std::shared_ptr<TextureUnit>& getTextureUnit(const String& name);
       
        int32_t getTextureUnitCount();
        std::vector<std::shared_ptr<TextureUnit>>& getAllTexureUnit();
        OgreTexture* getPbrTexture(TextureTypePbr type);
        
        void addShader(ShaderInfo& sinfo);

        ShaderInfo& getShaderInfo();

        void scale(Real u, Real v);
        void animation(Real u, Real v);

        PbrMaterialConstanceBuffer& getPbrMatInfo();
        GeneralMaterialConstantBuffer& getMatInfo();
        void update(Real delta);
        void setFresnelR0(Ogre::Vector3& fresnelR0);
        void setRoughness(Real roughness);
        void setDiffuseAlbedo(Ogre::Vector4& diffuseAlbedo);

        bool hasAnimation();
        Ogre::Vector2 getTexAnimationOffset();
        const Ogre::ColourBlendState& getBlendState() const;

        void setBlendState(Ogre::ColourBlendState& state);

        void setCullMode(backend::CullingMode mode);
        backend::CullingMode getCullMode();

        bool isPbr() const
        {
            return mPbr;
        }

        const String& getName() const
        {
            return mMaterialName;
        }

        void setMaterialFlags(uint32_t flags)
        {
            mMaterialFlags = flags;
        }

        uint32_t getMaterialFlags()
        {
            return mMaterialFlags;
        }

        bool isDepthTest();

        void setDepthTest(bool test);

        bool isWriteDepth();
        void setWriteDepth(bool bWrite);
        Material& operator=(const Material& rhs);

        bool isTransparent();

        ResourceState getResourceState()
        {
            return mState;
        }
        void updateResourceState();

        void setResourceState(ResourceState rs)
        {
            mState = rs;
        }

        RasterState& getRasterState()
        {
            return mRasterState;
        }

        void updateVertexDeclaration(VertexDeclaration* decl)
        {
            mVertexDeclaration = decl;
        }
        Handle<HwPipeline> getPipeline()
        {
            return mPipelineHandle;
        }
        Handle<HwProgram> getProgram()
        {
            return mProgramHandle;
        }

    private:
        void createFrameResourceInfo();
    private:
        std::string mMaterialName;
 
        uint32_t mMaterialFlags = 0;
        bool mAnimation = false;
        float mUOffset = 0.0f;
        float mVOffset = 0.0f;
        float mUFactor = 0.1f;
        float mVFactor = 0.02f;

        std::vector<std::shared_ptr<TextureUnit>> mTextureUnits;
        ShaderInfo mShaderInfo;
        PbrMaterialConstanceBuffer mPbrMatInfo;
        GeneralMaterialConstantBuffer mMatInfo;
        bool mPbr;
        bool mLoad = false;

        Ogre::ColourBlendState mBlendState;

        ResourceState mState = ResourceState::ResourceState_None;

        
        
        RasterState mRasterState;
        Handle<HwPipeline> mPipelineHandle;
        Handle<HwProgram> mProgramHandle;
        Handle<HwDescriptorSetLayout> mUboLayoutHandle;
        Handle<HwDescriptorSetLayout> mSamplerLayoutHandle;

        VertexDeclaration* mVertexDeclaration = nullptr;
    };
}