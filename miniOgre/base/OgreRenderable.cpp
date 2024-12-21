#include "OgreHeader.h"
#include "OgreRenderable.h"
#include "renderSystem.h"
#include "OgreRoot.h"
#include <OgreCamera.h>
#include <OgreVertexData.h>
#include <OgreIndexData.h>
#include <OgreMaterial.h>
#include "OgreTextureUnit.h"
#include <OgreTextureManager.h>

namespace Ogre {
    Renderable::Renderable():
        mObjectType(ObjectType_Static)
    {
        mModel = Ogre::Matrix4::IDENTITY;

        mSortValue = 1000000;
    }

    Renderable::~Renderable()
    {
    }

    void Renderable::setMaterial(std::shared_ptr<Material> mat)
    {
        mMaterial = mat;

        VertexData* vd = getVertexData();
        VertexDeclaration* decl = vd->getVertexDeclaration();
        mMaterial->updateVertexDeclaration(decl);
    }

    uint64_t Renderable::getSortValue()
    {
        return 0;
    }

    const Ogre::Matrix4& Renderable::getModelMatrix()
    {
        return mModel;
    }

    void Renderable::updateModelMatrix(const Ogre::Matrix4& m)
    {
        mModel = m;
    }

    Ogre::Vector3 getPosition()
    {
        return Ogre::Vector3(0.0f, 0.0f, 0.0f);
    }

    Ogre::OperationType Renderable::getPrimitiveTopology()
    {
        return OT_TRIANGLE_LIST;
    }

    bool Renderable::createFrameResource(RenderableFrameResourceCallback callback)
    {
        if (!mFrameResourceInfoList.empty())
            return false;
        auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
        mFrameResourceInfoList.resize(ogreConfig.swapBufferCount);
        Material* mat = mMaterial.get();
        if (callback)
        {
            return callback(mFrameResourceInfoList, mat);
        }
        
        auto* rs = Ogre::Root::getSingleton().getRenderSystem();
        
        for (auto i = 0; i < ogreConfig.swapBufferCount; i++)
        {
            FrameResourceInfo* resourceInfo = &mFrameResourceInfoList[i];
            resourceInfo->update = false;
            Handle<HwBufferObject> objectBufferHandle =
                rs->createBufferObject(
                    BufferObjectBinding::BufferObjectBinding_Uniform, 
                    RESOURCE_MEMORY_USAGE_CPU_TO_GPU,
                    0,
                    sizeof(ObjectConstantBuffer));
            resourceInfo->modelObjectHandle = objectBufferHandle;

            Handle<HwBufferObject> matBufferHandle;
            if (mat->isPbr())
            {
                matBufferHandle =
                    rs->createBufferObject(
                        BufferObjectBinding::BufferObjectBinding_Uniform, 
                        RESOURCE_MEMORY_USAGE_CPU_TO_GPU,
                        0,
                        sizeof(PbrMaterialConstanceBuffer));
            }
            else
            {
                matBufferHandle =
                    rs->createBufferObject(
                        BufferObjectBinding::BufferObjectBinding_Uniform,
                        RESOURCE_MEMORY_USAGE_CPU_TO_GPU,
                        0, sizeof(GeneralMaterialConstantBuffer));
            }

            resourceInfo->matObjectHandle = matBufferHandle;

            Handle<HwProgram> programHandle = mat->getProgram();
            resourceInfo->zeroSet = rs->createDescriptorSet(programHandle, 0);
            resourceInfo->firstSet = rs->createDescriptorSet(programHandle, 1);
            resourceInfo->zeroShadowSet = rs->createDescriptorSet(programHandle, 0);

            DescriptorData descriptorData[256];
            uint32_t descriptorCount = 0;
            descriptorData[descriptorCount].pName = "cbPerObject";
            descriptorData[descriptorCount].mCount = 1;
            descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_BUFFER;
            descriptorData[descriptorCount].ppBuffers = &objectBufferHandle;
            descriptorCount++;
            descriptorData[descriptorCount].pName = "cbMaterial";
            descriptorData[descriptorCount].mCount = 1;
            descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_BUFFER;
            descriptorData[descriptorCount].ppBuffers = &matBufferHandle;
            descriptorCount++;

            
            RawData* rawData = getSkinnedData();
            if (rawData)
            {
                resourceInfo->skinObjectHandle =
                    rs->createBufferObject(
                        BufferObjectBinding::BufferObjectBinding_Uniform,
                        RESOURCE_MEMORY_USAGE_GPU_ONLY,
                        0,
                        sizeof(SkinnedConstantBuffer));
                descriptorData[descriptorCount].pName = "cbSkinned";
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_BUFFER;
                descriptorData[descriptorCount].ppBuffers = &resourceInfo->skinObjectHandle;
                descriptorCount++;
            }

            rs->updateDescriptorSet(resourceInfo->zeroSet, descriptorCount, descriptorData);
            rs->updateDescriptorSet(resourceInfo->zeroShadowSet, descriptorCount, descriptorData);
            //update texture
            uint32_t index = 0;
            descriptorCount = 0;
            
            auto& texs = mat->getAllTexureUnit();
            if (mat->isPbr())
            {
                for (int32_t i = 0; i < texs.size(); i++)
                {
                    const char* pName = "";
                    int32_t texIndex = -1;
                    switch (texs[i]->getTextureProperty()->_pbrType)
                    {
                    case TextureTypePbr_Albedo:
                        texIndex = 0;
                        pName = "albedo_pbr";
                        break;
                    case TextureTypePbr_MetalRoughness:
                        texIndex = 4;
                        pName = "metal_roughness_pbr";
                        break;
                    case TextureTypePbr_NormalMap:
                        texIndex = 2;
                        pName = "normal_pbr";
                        break;
                    case TextureTypePbr_Emissive:
                        texIndex = 3;
                        pName = "emissive_pbr";
                        break;
                    case TextureTypePbr_AmbientOcclusion:
                        texIndex = 1;
                        pName = "ao_pbr";
                        break;
                    case TextureTypePbr_Roughness:
                        texIndex = 5;
                        pName = "roughness_pbr";
                        break;
                    case TextureTypePbr_BRDF_LUT:
                        texIndex = 6;
                        pName = "brdflut";
                        break;
                    case TextureTypePbr_IBL_Diffuse:
                        texIndex = 7;
                        pName = "irradianceCube";
                        break;
                    case TextureTypePbr_IBL_Specular:
                        texIndex = 8;
                        pName = "prefilteredCube";
                        break;
                    }
                    assert(texIndex >= 0);
                    OgreTexture* tex = texs[i]->getRaw();

                    descriptorData[descriptorCount].pName = "cbSkinned";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = tex;
                    descriptorCount++;
                }
                rs->updateDescriptorSet(resourceInfo->firstSet, descriptorCount, descriptorData);
            }
            else
            {
                std::array<OgreTexture*, 3> texArray{};
                int32_t texIndex = -1;
                for (int32_t i = 0; i < texs.size(); i++)
                {
                    if (texs[i]->getTextureProperty()->_texType == TEX_TYPE_CUBE_MAP)
                        continue;
                    texArray[++texIndex] = texs[i]->getRaw();
                }

                if (texIndex >= 0)
                {
                    descriptorData[descriptorCount].pName = "first";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = texArray[0];
                    descriptorCount++;
                }

                if (texIndex >= 1)
                {
                    descriptorData[descriptorCount].pName = "second";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = texArray[1];
                    descriptorCount++;
                }

                if (texIndex >= 2)
                {
                    descriptorData[descriptorCount].pName = "third";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = texArray[2];
                    descriptorCount++;
                }

                for (int32_t i = 0; i < texs.size(); i++)
                {
                    if (texs[i]->getTextureProperty()->_texType == TEX_TYPE_CUBE_MAP)
                    {
                        OgreTexture* tex = texs[i]->getRaw();
                        descriptorData[descriptorCount].pName = "gCubeMap";
                        descriptorData[descriptorCount].mCount = 1;
                        descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                        descriptorData[descriptorCount].ppTextures = tex;
                        descriptorCount++;
                        break;
                    }
                }

                rs->updateDescriptorSet(resourceInfo->firstSet, descriptorCount, descriptorData);
            }
        }
        return true;
    }

    void Renderable::updateFrameResource(uint32_t frameIndex)
    {
        FrameResourceInfo& resourceInfo = mFrameResourceInfoList[frameIndex];
        if (resourceInfo.update)
        {
            if(mObjectType == ObjectType_Static)
                return;
        }
        else
        {
            //resourceInfo.update = true;
        }
        
        auto* rs = Ogre::Root::getSingleton().getRenderSystem();
        Material* mat = mMaterial.get();
        static ObjectConstantBuffer objectBuffer;
        const auto& modelMatrix = this->getModelMatrix();

        objectBuffer.world = modelMatrix.transpose();
        rs->updateBufferObject(resourceInfo.modelObjectHandle,
            (const char*)&objectBuffer, sizeof(objectBuffer));

        if (mat->isPbr())
        {
            auto& matBuffer = mat->getPbrMatInfo();
            rs->updateBufferObject(resourceInfo.matObjectHandle,
                (const char*)&matBuffer, sizeof(matBuffer));
        }
        else
        {
            auto& matBuffer = mat->getMatInfo();
            rs->updateBufferObject(resourceInfo.matObjectHandle,
                (const char*)&matBuffer, sizeof(matBuffer));
        }
    }

    void Renderable::updateMaterialInfo(bool updateTexture)
    {
        Material* mat = mMaterial.get();
        auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
        auto* rs = Ogre::Root::getSingleton().getRenderSystem();
        for (auto i = 0; i < ogreConfig.swapBufferCount; i++)
        {
            auto& resourceInfo = mFrameResourceInfoList[i];
            resourceInfo.update = false;
        }
        
        if (!updateTexture)
            return;
        DescriptorData descriptorData[256];
        uint32_t descriptorCount = 0;
        uint32_t index = 0;
        mat->load(nullptr);
        auto& texs = mat->getAllTexureUnit();
        for (auto i = 0; i < ogreConfig.swapBufferCount; i++)
        {
            auto& resourceInfo = mFrameResourceInfoList[i];
            if (mat->isPbr())
            {
                for (int32_t i = 0; i < texs.size(); i++)
                {
                    const char* pName = "";
                    int32_t texIndex = -1;
                    switch (texs[i]->getTextureProperty()->_pbrType)
                    {
                    case TextureTypePbr_Albedo:
                        texIndex = 0;
                        pName = "albedo_pbr";
                        break;
                    case TextureTypePbr_MetalRoughness:
                        texIndex = 4;
                        pName = "metal_roughness_pbr";
                        break;
                    case TextureTypePbr_NormalMap:
                        texIndex = 2;
                        pName = "normal_pbr";
                        break;
                    case TextureTypePbr_Emissive:
                        texIndex = 3;
                        pName = "emissive_pbr";
                        break;
                    case TextureTypePbr_AmbientOcclusion:
                        texIndex = 1;
                        pName = "ao_pbr";
                        break;
                    case TextureTypePbr_Roughness:
                        texIndex = 5;
                        pName = "roughness_pbr";
                        break;
                    case TextureTypePbr_BRDF_LUT:
                        texIndex = 6;
                        pName = "brdflut";
                        break;
                    case TextureTypePbr_IBL_Diffuse:
                        texIndex = 7;
                        pName = "irradianceCube";
                        break;
                    case TextureTypePbr_IBL_Specular:
                        texIndex = 8;
                        pName = "prefilteredCube";
                        break;
                    }
                    assert(texIndex >= 0);
                    OgreTexture* tex = texs[i]->getRaw();

                    descriptorData[descriptorCount].pName = "cbSkinned";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = tex;
                    descriptorCount++;
                }
                rs->updateDescriptorSet(resourceInfo.firstSet, descriptorCount, descriptorData);

            }
            else
            {
                std::array<OgreTexture*, 3> texArray{};
                int32_t texIndex = -1;
                for (int32_t i = 0; i < texs.size(); i++)
                {
                    if (texs[i]->getTextureProperty()->_texType == TEX_TYPE_CUBE_MAP)
                        continue;
                    texArray[++texIndex] = texs[i]->getRaw();
                }

                if (texIndex >= 0)
                {
                    descriptorData[descriptorCount].pName = "first";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = texArray[0];
                    descriptorCount++;
                }

                if (texIndex >= 1)
                {
                    descriptorData[descriptorCount].pName = "second";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = texArray[1];
                    descriptorCount++;
                }

                if (texIndex >= 2)
                {
                    descriptorData[descriptorCount].pName = "third";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = texArray[2];
                    descriptorCount++;
                }

                for (int32_t i = 0; i < texs.size(); i++)
                {
                    if (texs[i]->getTextureProperty()->_texType == TEX_TYPE_CUBE_MAP)
                    {
                        OgreTexture* tex = texs[i]->getRaw();
                        descriptorData[descriptorCount].pName = "gCubeMap";
                        descriptorData[descriptorCount].mCount = 1;
                        descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                        descriptorData[descriptorCount].ppTextures = tex;
                        descriptorCount++;
                        break;
                    }
                }

                rs->updateDescriptorSet(resourceInfo.firstSet, descriptorCount, descriptorData);
            }
        }
        
    }

    FrameResourceInfo* Renderable::getFrameResourceInfo(uint32_t frameIndex)
    {
        return &mFrameResourceInfoList[frameIndex];
    }

    void Renderable::setObjectType(ObjectType type)
    {
        mObjectType = type;
    }
}