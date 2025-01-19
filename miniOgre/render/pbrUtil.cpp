#include <OgreHeader.h>
#include "pbrUtil.h"
#include "OgreTexture.h"
#include "OgreRoot.h"
#include "renderSystem.h"
#include "OgreTextureManager.h"
#include "OgreMeshManager.h"
#include "OgreVertexData.h"
#include "OgreRenderTarget.h"

namespace Ogre
{
    Ogre::RenderTarget* generateCubeMap(
        const std::string& name,
        Ogre::OgreTexture* environmentCube,
        Ogre::PixelFormat format,
        int32_t dim,
        CubeType type)
    {
        Ogre::TextureProperty texProperty;
        texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
        texProperty._texType = TEX_TYPE_CUBE_MAP;

        RenderSystem* rs = Root::getSingleton().getRenderSystem();
        texProperty._width = dim;
        texProperty._height = dim;
        texProperty._tex_format = format;
        Ogre::RenderTarget* rt = rs->createRenderTarget(name, texProperty);
        OgreTexture* cubeTexture = rt->getTarget();
        struct PushBlockIrradiance {
            Ogre::Matrix4 mvp;
            float deltaPhi = (2.0f * float(Ogre::Math::PI)) / 180.0f;
            float deltaTheta = (0.5f * float(Ogre::Math::PI)) / 64.0f;
        } pushBlockIrradiance;

        struct PushBlockPrefilterEnv {
            Ogre::Matrix4 mvp;
            float roughness;
            uint32_t numSamples = 32u;
        } pushBlockPrefilterEnv;

        ShaderInfo shaderInfo;

        uint32 pushBlockSize;
        if (type == CubeType_Irradiance)
        {
            pushBlockSize = sizeof(PushBlockIrradiance);
            shaderInfo.shaderName = "irradianceMap";
        }
        else
        {
            pushBlockSize = sizeof(PushBlockPrefilterEnv);
            shaderInfo.shaderName = "prefilteredMap";
        }
        std::string meshName = "box.mesh";
        auto mesh = MeshManager::getSingleton().createBox(meshName, 256, "SkyLan");
        auto* subMesh = mesh->getSubMesh(0);
        IndexData* indexData = subMesh->getIndexData();
        VertexData* vertexData = subMesh->getVertexData();
        VertexDeclaration* decl = vertexData->getVertexDeclaration();
        Handle<HwProgram> programHandle = rs->createShaderProgram(shaderInfo, decl);

        
        backend::RasterState rasterState{};
        rasterState.colorWrite = true;
        rasterState.renderTargetCount = 1;
        rasterState.depthWrite = false;
        rasterState.depthTest = false;
        rasterState.pixelFormat[0] = format;

        Handle<HwPipeline> pipelineHandle = rs->createPipeline(rasterState, programHandle);
        BufferDesc desc{};
        desc.mBindingType = BufferObjectBinding_Uniform;
        desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        desc.bufferCreationFlags = 0;
        desc.mSize = pushBlockSize;
        Handle<HwBufferObject> pushBlockHandle = rs->createBufferObject(desc);
        auto zeroDescSet = rs->createDescriptorSet(programHandle, 0);
        DescriptorData descriptorData[2];

        descriptorData[0].pName = "samplerEnv";
        descriptorData[0].mCount = 1;
        descriptorData[0].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
        descriptorData[0].ppTextures = (const OgreTexture**) & environmentCube;

        descriptorData[1].pName = "pushConsts";
        descriptorData[1].mCount = 1;
        descriptorData[1].descriptorType = DESCRIPTOR_TYPE_BUFFER;
        descriptorData[1].ppBuffers = &pushBlockHandle;

        rs->updateDescriptorSet(zeroDescSet, 2, descriptorData);
        
        std::vector<Ogre::Matrix4> matrices = {
            Ogre::Math::makeRotateMatrix(Ogre::Math::makeRotateMatrix(Ogre::Matrix4::IDENTITY, 90.0f, Ogre::Vector3(0.0f, 1.0f, 0.0f)), 180.0f, Ogre::Vector3(1.0f, 0.0f, 0.0f)),
            Ogre::Math::makeRotateMatrix(Ogre::Math::makeRotateMatrix(Ogre::Matrix4::IDENTITY, -90.0f, Ogre::Vector3(0.0f, 1.0f, 0.0f)), 180.0f, Ogre::Vector3(1.0f, 0.0f, 0.0f)),
            Ogre::Math::makeRotateMatrix(Ogre::Matrix4::IDENTITY, -90.0f, Ogre::Vector3(1.0f, 0.0f, 0.0f)),
            Ogre::Math::makeRotateMatrix(Ogre::Matrix4::IDENTITY, 90.0f, Ogre::Vector3(1.0f, 0.0f, 0.0f)),
            Ogre::Math::makeRotateMatrix(Ogre::Matrix4::IDENTITY, 180.0f, Ogre::Vector3(1.0f, 0.0f, 0.0f)),
            Ogre::Math::makeRotateMatrix(Ogre::Matrix4::IDENTITY, 180.0f, Ogre::Vector3(0.0f, 0.0f, 1.0f)),
        };
       
        texProperty._width = dim;
        texProperty._height = dim;
        texProperty._tex_usage = Ogre::TextureUsage::COLOR_ATTACHMENT;
        texProperty._tex_format = format;
        texProperty._texType = TEX_TYPE_2D;
        auto outPutTarget = rs->createRenderTarget("outputTarget", texProperty);

        RenderPassInfo renderPassInfo;
        renderPassInfo.renderTargetCount = 1;
        renderPassInfo.renderTargets[0].renderTarget = outPutTarget;
        renderPassInfo.depthTarget.depthStencil = nullptr;
        renderPassInfo.renderTargets[0].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
        renderPassInfo.viewport = false;
        
        const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

        RenderTargetBarrier uavBarriers[] = {
               {
               rt,
               RESOURCE_STATE_GENERIC_READ,
               RESOURCE_STATE_COPY_DEST},
        };

        rs->beginCmd();
        rs->resourceBarrier(0, nullptr, 0, nullptr, 1, uavBarriers);
        rs->flushCmd(true);

        for (uint32_t m = 0; m < numMips; m++) 
        {
            for (uint32_t f = 0; f < 6; f++)
            {
                rs->beginCmd();
                uint32_t width = static_cast<float>(dim * std::pow(0.5f, m));
                uint32_t height = static_cast<float>(dim * std::pow(0.5f, m));

                auto perspective = Ogre::Math::makePerspectiveMatrixRH((float)(Ogre::Math::PI / 2.0), 1.0f, 0.1f, 512);
                if (type == CubeType_Irradiance)
                {
                    pushBlockIrradiance.mvp = perspective * matrices[f];
                    pushBlockIrradiance.mvp = pushBlockIrradiance.mvp.transpose();

                    rs->updateBufferObject(
                        pushBlockHandle,
                        (const char*)&pushBlockIrradiance,
                        sizeof(pushBlockIrradiance));

                }
                else
                {
                    pushBlockPrefilterEnv.mvp = perspective * matrices[f];
                    pushBlockPrefilterEnv.mvp = pushBlockPrefilterEnv.mvp.transpose();
                    pushBlockPrefilterEnv.roughness = (float)m / (float)(numMips - 1);

                    rs->updateBufferObject(
                        pushBlockHandle,
                        (const char*)&pushBlockPrefilterEnv,
                        sizeof(pushBlockPrefilterEnv));
                }
                rs->setViewport(0, 0, width, height, 0.0f, 1.0f);
                rs->setScissor(0, 0, width, height);
                rs->beginRenderPass(renderPassInfo);

                // Pass parameters for current pass using a push constant block
                
                rs->bindPipeline(pipelineHandle, &zeroDescSet, 1);

                vertexData->bind(nullptr);

                if (indexData)
                {
                    indexData->bind();
                    IndexDataView* view = subMesh->getIndexView();
                    rs->drawIndexed(view->mIndexCount, 1,
                        view->mIndexLocation, view->mBaseVertexLocation, 0);
                }

                rs->endRenderPass(renderPassInfo);

                RenderTargetBarrier rtBarriers[] =
                {
                    {
                        outPutTarget,
                        RESOURCE_STATE_RENDER_TARGET,
                        RESOURCE_STATE_GENERIC_READ
                    }
                };
                rs->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);

                ImageCopyDesc copyRegion;
                
                copyRegion.srcSubresource.aspectMask = 0;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;

                copyRegion.dstSubresource.aspectMask = 0;
                copyRegion.dstSubresource.baseArrayLayer = f;
                copyRegion.dstSubresource.mipLevel = m;
                copyRegion.dstSubresource.layerCount = 1;

                copyRegion.extent.width = width;
                copyRegion.extent.height = height;
                copyRegion.extent.depth = 1;
                rs->copyImage(rt, outPutTarget, copyRegion);

                rtBarriers[0] =
                {
                    outPutTarget,
                    RESOURCE_STATE_GENERIC_READ,
                    RESOURCE_STATE_RENDER_TARGET
                };
                rs->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);
                rs->flushCmd(true);                
            }
        }

        uavBarriers[0] = 
        {
               rt,
               RESOURCE_STATE_COPY_DEST,
               RESOURCE_STATE_SHADER_RESOURCE
        };

        rs->beginCmd();
        rs->resourceBarrier(0, nullptr, 0, nullptr, 1, uavBarriers);
        rs->flushCmd(true);

        return rt;
    }

    Ogre::RenderTarget* generateBRDFLUT(const std::string& name)
    {
        auto dim = 512;
        const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

        Ogre::TextureProperty texProperty;
        
        RenderSystem* rs = Root::getSingleton().getRenderSystem();

        texProperty._width = dim;
        texProperty._height = dim;
        texProperty._tex_usage = Ogre::TextureUsage::COLOR_ATTACHMENT;
        texProperty._tex_format = PF_FLOAT16_GR;
        texProperty._samplerParams.filterMag = backend::SamplerFilterType::LINEAR;
        texProperty._samplerParams.filterMin = backend::SamplerFilterType::LINEAR;
        texProperty._samplerParams.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR;
        texProperty._samplerParams.wrapS = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        texProperty._samplerParams.wrapT = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        texProperty._samplerParams.wrapR = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        texProperty._samplerParams.anisotropyLog2 = 0;
        texProperty._need_mipmap = false;
        Ogre::RenderTarget* rt = rs->createRenderTarget(name, texProperty);
        ShaderInfo shaderInfo;
        shaderInfo.shaderName = "generateBRDFLUT";
        Handle<HwProgram> programHandle = rs->createShaderProgram(shaderInfo, nullptr);

       
        backend::RasterState rasterState;
        rasterState.colorWrite = true;
        rasterState.renderTargetCount = 1;
        rasterState.depthWrite = false;
        rasterState.depthTest = false;
        rasterState.pixelFormat[0] = PF_FLOAT16_GR;
        Handle<HwPipeline> pipelineHandle = rs->createPipeline(rasterState, programHandle);
        RenderPassInfo renderPassInfo;
        renderPassInfo.renderTargetCount = 1;
        renderPassInfo.renderTargets[0].renderTarget = rt;
        renderPassInfo.depthTarget.depthStencil = nullptr;
        renderPassInfo.renderTargets[0].clearColour = { 0.0f, 0.0f, 0.0f, 1.000000000f };
        rs->beginCmd();

        RenderTargetBarrier uavBarriers[] = {
               {
               rt,
               RESOURCE_STATE_UNDEFINED,
               RESOURCE_STATE_RENDER_TARGET},
        };
        rs->resourceBarrier(0, nullptr, 0, nullptr, 1, uavBarriers);

        rs->beginRenderPass(renderPassInfo);
        rs->bindPipeline(pipelineHandle, nullptr, 0);
        rs->draw(3, 0);
        rs->endRenderPass(renderPassInfo);

        uavBarriers[0] = {
               rt,
               RESOURCE_STATE_RENDER_TARGET,
               RESOURCE_STATE_SHADER_RESOURCE
        };
        rs->resourceBarrier(0, nullptr, 0, nullptr, 1, uavBarriers);
        rs->flushCmd(true);

        return rt;
    }
}