#include <OgreHeader.h>
#include <OgreRoot.h>
#include <OgreMaterial.h>
#include <OgreTexture.h>
#include <OgreTextureUnit.h>
#include <OgreSceneManager.h>
#include <OgreVertexData.h>
#include <OgreIndexData.h>
#include <renderSystem.h>
#include "renderUtil.h"
#include "game_camera.h"

void initFrameResource(uint32_t frameIndex, Renderable* r)
{
    auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();
    if(1)
    {
        FrameResourceInfo* resourceInfo = new FrameResourceInfo;
        resourceInfo->update = false;

        r->updateFrameResource(frameIndex, (void*)resourceInfo);

        BufferDesc desc{};
        desc.mBindingType = BufferObjectBinding_Uniform;
        desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        desc.bufferCreationFlags = 0;
        desc.mSize = sizeof(ObjectConstantBuffer);
        Handle<HwBufferObject> objectBufferHandle =
            rs->createBufferObject(desc);
        resourceInfo->modelObjectHandle = objectBufferHandle;

        Ogre::Material* mat = r->getMaterial().get();
        Handle<HwBufferObject> matBufferHandle;
        if (mat->isPbr())
        {
            desc.mBindingType = BufferObjectBinding_Uniform;
            desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
            desc.bufferCreationFlags = 0;
            desc.mSize = sizeof(PbrMaterialConstanceBuffer);
            matBufferHandle = rs->createBufferObject(desc);
        }
        else
        {
            desc.mBindingType = BufferObjectBinding_Uniform;
            desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
            desc.bufferCreationFlags = 0;
            desc.mSize = sizeof(GeneralMaterialConstantBuffer);
            matBufferHandle = rs->createBufferObject(desc);
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
        const char* materialName = mat->isPbr() ? "pbrMaterial" : "cbMaterial";
        descriptorData[descriptorCount].pName = materialName;
        descriptorData[descriptorCount].mCount = 1;
        descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_BUFFER;
        descriptorData[descriptorCount].ppBuffers = &matBufferHandle;
        descriptorCount++;


        RawData* rawData = r->getSkinnedData();
        if (rawData)
        {
            desc.mBindingType = BufferObjectBinding_Uniform;
            desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
            desc.bufferCreationFlags = 0;
            desc.mSize = sizeof(SkinnedConstantBuffer);
            resourceInfo->skinObjectHandle = rs->createBufferObject(desc);
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
            OgreTexture* texArray[9];
            for (int32_t i = 0; i < texs.size(); i++)
            {
                const char* pName = "";
                const char* samplerName = "";
                int32_t texIndex = -1;
                switch (texs[i]->getTextureProperty()->_pbrType)
                {
                case TextureTypePbr_Albedo:
                    texIndex = 0;
                    pName = "albedo_pbr";
                    samplerName = "albedoSampler";
                    break;
                case TextureTypePbr_MetalRoughness:
                    texIndex = 4;
                    pName = "metal_roughness_pbr";
                    samplerName = "metalRoughnessSampler";
                    break;
                case TextureTypePbr_NormalMap:
                    texIndex = 2;
                    pName = "normal_pbr";
                    samplerName = "normalSampler";
                    break;
                case TextureTypePbr_Emissive:
                    texIndex = 3;
                    pName = "emissive_pbr";
                    samplerName = "emissiveSampler";
                    break;
                case TextureTypePbr_AmbientOcclusion:
                    texIndex = 1;
                    pName = "ao_pbr";
                    samplerName = "aoSampler";
                    break;
                case TextureTypePbr_Roughness:
                    texIndex = 5;
                    pName = "roughness_pbr";
                    samplerName = "roughnessSampler";
                    break;
                case TextureTypePbr_BRDF_LUT:
                    texIndex = 6;
                    pName = "brdflut_pbr";
                    samplerName = "brdflutSampler";
                    break;
                case TextureTypePbr_IBL_Diffuse:
                    texIndex = 7;
                    pName = "irradianceCube";
                    samplerName = "irradianceSampler";
                    break;
                case TextureTypePbr_IBL_Specular:
                    texIndex = 8;
                    pName = "prefilteredCube";
                    samplerName = "prefilteredSampler";
                    break;
                }
                assert(texIndex >= 0);
                OgreTexture* tex = texs[i]->getRaw();
                texArray[i] = tex;
                descriptorData[descriptorCount].pName = pName;
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[i];
                descriptorCount++;

                descriptorData[descriptorCount].pName = samplerName;
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[i];
                descriptorCount++;
            }
            rs->updateDescriptorSet(resourceInfo->firstSet, descriptorCount, descriptorData);
        }
        else
        {
            OgreTexture* texArray[4];
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
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[0];
                descriptorCount++;

                descriptorData[descriptorCount].pName = "firstSampler";
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[0];
                descriptorCount++;
            }

            if (texIndex >= 1)
            {
                descriptorData[descriptorCount].pName = "second";
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[1];
                descriptorCount++;

                descriptorData[descriptorCount].pName = "secondSampler";
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[1];
                descriptorCount++;
            }

            if (texIndex >= 2)
            {
                descriptorData[descriptorCount].pName = "third";
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)texArray[2];
                descriptorCount++;

                descriptorData[descriptorCount].pName = "thirdSampler";
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[2];
                descriptorCount++;
            }

            for (int32_t i = 0; i < texs.size(); i++)
            {
                if (texs[i]->getTextureProperty()->_texType == TEX_TYPE_CUBE_MAP)
                {
                    texArray[3] = texs[i]->getRaw();
                    descriptorData[descriptorCount].pName = "cubeMap";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[3];
                    descriptorCount++;

                    descriptorData[descriptorCount].pName = "cubeSampler";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[3];
                    descriptorCount++;
                    break;
                }
            }
            rs->updateDescriptorSet(resourceInfo->firstSet, descriptorCount, descriptorData);
        }
    }
}

void updateFrameResource(uint32_t frameIndex, Renderable* r)
{
    void* frameData= r->getFrameResourceInfo(frameIndex);
    FrameResourceInfo* resourceInfo = (FrameResourceInfo*)frameData;
    if (resourceInfo == nullptr)
        return;
    if (resourceInfo->update)
    {
        auto objectType = r->getObjectType();
        if (objectType == ObjectType_Static)
            return;
    }
    else
    {
        //resourceInfo.update = true;
    }

    auto* rs = Ogre::Root::getSingleton().getRenderSystem();
    Ogre::Material* mat = r->getMaterial().get();
    ObjectConstantBuffer objectBuffer;
    const auto& modelMatrix = r->getModelMatrix();

    objectBuffer.world = modelMatrix.transpose();
    objectBuffer.diffuseColor = r->getColor();
    rs->updateBufferObject(resourceInfo->modelObjectHandle,
        (const char*)&objectBuffer, sizeof(objectBuffer));

    RawData* rawData = r->getSkinnedData();
    if (rawData)
    {
        rs->updateBufferObject(resourceInfo->skinObjectHandle, rawData->mData, rawData->mDataSize);
    }

    if (mat->isPbr())
    {
        auto& matBuffer = mat->getPbrMatInfo();
        rs->updateBufferObject(resourceInfo->matObjectHandle,
            (const char*)&matBuffer, sizeof(matBuffer));
    }
    else
    {
        auto& matBuffer = mat->getMatInfo();
        rs->updateBufferObject(resourceInfo->matObjectHandle,
            (const char*)&matBuffer, sizeof(matBuffer));
    }
}

void updateMaterialInfo(Renderable* r, bool updateTexture)
{
    Material* mat = r->getMaterial().get();

    auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();
    for (auto i = 0; i < ogreConfig.swapBufferCount; i++)
    {
        void* frameData = r->getFrameResourceInfo(i);
        FrameResourceInfo* resourceInfo = (FrameResourceInfo*)frameData;
        resourceInfo->update = false;
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
        void* frameData = r->getFrameResourceInfo(i);
        FrameResourceInfo* resourceInfo = (FrameResourceInfo*)frameData;
        if (mat->isPbr())
        {
            OgreTexture* texArray[9];
            for (int32_t i = 0; i < texs.size(); i++)
            {
                const char* pName = "";
                const char* samplerName = "";
                int32_t texIndex = -1;
                switch (texs[i]->getTextureProperty()->_pbrType)
                {
                case TextureTypePbr_Albedo:
                    texIndex = 0;
                    pName = "albedo_pbr";
                    samplerName = "albedoSampler";
                    break;
                case TextureTypePbr_MetalRoughness:
                    texIndex = 4;
                    pName = "metal_roughness_pbr";
                    samplerName = "metalRoughnessSampler";
                    break;
                case TextureTypePbr_NormalMap:
                    texIndex = 2;
                    pName = "normal_pbr";
                    samplerName = "normalSampler";
                    break;
                case TextureTypePbr_Emissive:
                    texIndex = 3;
                    pName = "emissive_pbr";
                    samplerName = "emissiveSampler";
                    break;
                case TextureTypePbr_AmbientOcclusion:
                    texIndex = 1;
                    pName = "ao_pbr";
                    samplerName = "aoSampler";
                    break;
                case TextureTypePbr_Roughness:
                    texIndex = 5;
                    pName = "roughness_pbr";
                    samplerName = "roughnessSampler";
                    break;
                case TextureTypePbr_BRDF_LUT:
                    texIndex = 6;
                    pName = "brdflut_pbr";
                    samplerName = "brdflutSampler";
                    break;
                case TextureTypePbr_IBL_Diffuse:
                    texIndex = 7;
                    pName = "irradianceCube";
                    samplerName = "irradianceSampler";
                    break;
                case TextureTypePbr_IBL_Specular:
                    texIndex = 8;
                    pName = "prefilteredCube";
                    samplerName = "prefilteredSampler";
                    break;
                }
                assert(texIndex >= 0);
                OgreTexture* tex = texs[i]->getRaw();
                texArray[i] = tex;
                descriptorData[descriptorCount].pName = pName;
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[i];
                descriptorCount++;

                descriptorData[descriptorCount].pName = samplerName;
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[i];
                descriptorCount++;
            }
            rs->updateDescriptorSet(resourceInfo->firstSet, descriptorCount, descriptorData);

        }
        else
        {
            std::array<OgreTexture*, 4> texArray{};
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
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[0];
                descriptorCount++;
            }

            if (texIndex >= 1)
            {
                descriptorData[descriptorCount].pName = "second";
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[1];
                descriptorCount++;
            }

            if (texIndex >= 2)
            {
                descriptorData[descriptorCount].pName = "third";
                descriptorData[descriptorCount].mCount = 1;
                descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[2];
                descriptorCount++;
            }

            for (int32_t i = 0; i < texs.size(); i++)
            {
                if (texs[i]->getTextureProperty()->_texType == TEX_TYPE_CUBE_MAP)
                {
                    texArray[3] = texs[i]->getRaw();
                    descriptorData[descriptorCount].pName = "gCubeMap";
                    descriptorData[descriptorCount].mCount = 1;
                    descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
                    descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[3];
                    descriptorCount++;
                    break;
                }
            }

            rs->updateDescriptorSet(resourceInfo->firstSet, descriptorCount, descriptorData);
        }
    }
}

void renderScene(
    ICamera* cam,
    SceneManager* sceneManager,
    RenderPassInfo& renderPassInfo,
    UserDefineShader* userDefineShader)
{
    EngineRenderList renderList;
    sceneManager->getSceneRenderList(cam, renderList, false);
    renderScene(cam, renderList.mOpaqueList, renderPassInfo, userDefineShader);
}

void renderScene(
    ICamera* cam,
    const std::vector<Renderable*>& renderList,
    RenderPassInfo& renderPassInfo,
    UserDefineShader* userDefineShader)
{
    auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
    auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();
    uint32_t index = 0;
    for (auto r : renderList)
    {
        Ogre::Material* mat = r->getMaterial().get();
        if (!mat->isLoaded())
        {
            mat->load(nullptr);
        }

        if (!r->hasFlag(frameIndex))
        {
            userDefineShader->initCallback(frameIndex, r);
            r->setFlag(frameIndex, true);
        }

        userDefineShader->bindCallback(frameIndex, r);
    }

    rs->beginRenderPass(renderPassInfo);
    for (auto r : renderList)
    {
        Ogre::Material* mat = r->getMaterial().get();
        auto flags = mat->getMaterialFlags();
        if (flags & MATERIAL_FLAG_ALPHA_TESTED)
        {
            continue;
        }
        userDefineShader->drawCallback(frameIndex, r);
    }

    for (auto r : renderList)
    {
        break;
        Ogre::Material* mat = r->getMaterial().get();
        auto flags = mat->getMaterialFlags();
        if (flags & MATERIAL_FLAG_ALPHA_TESTED)
        {
            userDefineShader->drawCallback(frameIndex, r);
        }
    }

    rs->endRenderPass(renderPassInfo);
}

//void drawRenderable(
//    uint32_t frameIndex, 
//    Renderable* r, 
//    UserDefineShader* userDefineShader)
//{
//    auto* rs = Ogre::Root::getSingleton().getRenderSystem();
//    void* frameData = r->getFrameResourceInfo(frameIndex);
//    FrameResourceInfo* resourceInfo = (FrameResourceInfo*)frameData;
//    Ogre::Material* mat = r->getMaterial().get();
//    if (userDefineShader->useDefineShader)
//    {
//        UserDefineShaderData* dummy = &userDefineShader->userDefineShaderData;
//        rs->bindPipeline(dummy->programHandle, dummy->pipelineHandle,
//            dummy->descSetData[frameIndex].descSets,
//            dummy->descSetData[frameIndex].descSetCount);
//    }
//    else
//    {
//        auto programHandle = mat->getProgram();
//        auto piplineHandle = mat->getPipeline();
//        Handle<HwDescriptorSet> descriptorSet[2];
//        descriptorSet[0] = resourceInfo->zeroSet;
//        descriptorSet[1] = resourceInfo->firstSet;
//        rs->bindPipeline(programHandle, piplineHandle, descriptorSet, 2);
//    }
//
//
//    VertexData* vertexData = r->getVertexData();
//    IndexData* indexData = r->getIndexData();
//    vertexData->bind(nullptr);
//    indexData->bind();
//    IndexDataView* view = r->getIndexView();
//    rs->drawIndexed(view->mIndexCount, 1,
//        view->mIndexLocation, view->mBaseVertexLocation, 0);
//}

void updateFrameData(
    ICamera* camera, 
    ICamera* light, 
    FrameConstantBuffer& frameConstantBuffer)
{
    RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
    const Ogre::Matrix4& view = camera->getViewMatrix();
    const Ogre::Matrix4& proj = camera->getProjectMatrix();
    const Ogre::Vector3& camepos = camera->getDerivedPosition();
    Ogre::Matrix4 invView = view.inverse();
    Ogre::Matrix4 viewProj = proj * view;
    Ogre::Matrix4 invProj = proj.inverse();
    Ogre::Matrix4 invViewProj = viewProj.inverse();

    frameConstantBuffer.View = view.transpose();
    frameConstantBuffer.InvView = invView.transpose();
    frameConstantBuffer.Proj = proj.transpose();
    frameConstantBuffer.InvProj = invProj.transpose();
    frameConstantBuffer.ViewProj = viewProj.transpose();
    frameConstantBuffer.InvViewProj = invViewProj.transpose();

    frameConstantBuffer.EyePosW = camepos;

    if (light)
    {
        frameConstantBuffer.ShadowTransform = (light->getProjectMatrix() * light->getViewMatrix()).transpose();
        frameConstantBuffer.Shadow = 1;

        frameConstantBuffer.numDirLights = 1;
        /*mFrameConstantBuffer.directionLights[0].lightViewProject =
            (light->getProjectMatrix() * light->getViewMatrix()).transpose();*/
        frameConstantBuffer.directionLights[0].Direction = Ogre::Vector3(-100.0f, -100.0f, 0.0f);
    }
    else
    {
        frameConstantBuffer.Shadow = 0;
        frameConstantBuffer.directionLights[0].Direction = Ogre::Vector3(0.739942074, 0.642787576, 0.198266909);
        frameConstantBuffer.directionLights[0].Direction.normalise();
    }


    frameConstantBuffer.TotalTime += Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
    frameConstantBuffer.DeltaTime = Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
}