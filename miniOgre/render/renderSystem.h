#pragma once
#include <functional>
#include "shader.h"
#include "engine_struct.h"
#include "OgreTexture.h"
#include <DriverEnums.h>
#include <utils/Allocator.h>
#include <utils/JobSystem.h>
#include <filament/Handle.h>
#include <filament/DriverBase.h>
#include <filament/DescriptorSetOffsetArray.h>
#include <fg/FrameGraphId.h>
#include <fg/FrameGraphTexture.h>
#include <fg/Allocators.h>
#include <fg/ResourceAllocator.h>
#include "rayTracing.h"

class GraphicsCommandList;
class RenderableData;
class VertexDeclaration;

using namespace filament::backend;

namespace Ogre
{
    class RenderWindow;
}

using FrameGraphPassCallback = std::function< FrameGraphId<FrameGraphTexture>(FrameGraph& fg)>;

class RenderSystem
{
public:
    RenderSystem();
    ~RenderSystem();
    virtual bool engineInit(bool raytracing = false);
    virtual void frameStart() = 0;
    virtual void frameEnd() = 0;
    virtual void render(FrameGraphPassCallback cb);
    virtual Ogre::OgreTexture* createTextureFromFile(
        const std::string& name,
        Ogre::TextureProperty* texProperty);
    virtual Ogre::OgreTexture* createTexture(Ogre::TextureProperty* texProperty);
    virtual Ogre::RenderWindow* createRenderWindow(
        const String& name, unsigned int width, unsigned int height,
        const NameValuePairList* miscParams) = 0;

    virtual Ogre::RenderTarget* createRenderTarget(
        const String& name, 
        uint32_t width, 
        uint32_t height, 
        Ogre::PixelFormat format, 
        uint32_t textureUsage);

    virtual void ready() {}

    virtual const String& getRenderSystemName()
    {
        return mRenderSystemName;
    }

    void incrTriangleCount(uint32_t count)
    {
        mTriangleCount += count;
    }
    uint32_t getTriangleCount()
    {
        return mTriangleCount;
    }

    void incrBatchCount(uint32_t count)
    {
        mBatchCount += count;
    }

    uint32_t getBatchCount()
    {
        return mBatchCount;
    }

    utils::JobSystem& getJobSystem()
    {
        return mJobSystem;
    }

    virtual void beginRenderPass(
        RenderPassInfo& renderPassInfo);
    virtual void endRenderPass(RenderPassInfo& renderPassInfo);

    virtual void bindPipeline(
        Handle<HwProgram> programHandle,
        Handle<HwPipeline> pipelineHandle,
        Handle<HwDescriptorSet>* descSets,
        uint32_t setCount) 
    {
    }

    virtual void bindPipeline(
        Handle<HwRaytracingProgram> programHandle,
        Handle<HwDescriptorSet>* descSets,
        uint32_t setCount
    ) {}

    virtual void traceRay(Handle<HwRaytracingProgram> programHandle) {}

    virtual void copyImage(Ogre::RenderTarget* dst, Ogre::RenderTarget* src) {}
    virtual void drawIndexed(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        uint32_t vertexOffset,
        uint32_t firstInstance)
    {
    }

    virtual void draw(uint32_t vertexCount, uint32_t firstVertex)
    {

    }
    virtual void drawIndexedIndirect(
        Handle<HwBufferObject> drawBuffer,
        uint32_t offset,
        uint32_t drawCount,
        uint32_t stride
    ) 
    {
    }

    virtual void beginComputePass(
        ComputePassInfo& computePassInfo);
    virtual void endComputePass();

    virtual void dispatchComputeShader();
    virtual void present();

    virtual void pushGroupMarker(const char* maker) {}
    virtual void popGroupMarker() {}

    virtual Ogre::OgreTexture* generateCubeMap(
        const std::string& name,
        Ogre::OgreTexture* environmentCube,
        Ogre::PixelFormat format,
        int32_t dim,
        CubeType type)
    {
        return nullptr;
    }

    virtual Ogre::OgreTexture* generateBRDFLUT(const std::string& name)
    {
        return nullptr;
    }

    virtual FrameConstantBuffer* getFrameConstantBuffer()
    {
        return nullptr;
    }

    virtual void bindVertexBuffer(Handle<HwBufferObject> bufHandle, uint32_t binding) {}
    virtual void bindIndexBuffer(Handle<HwBufferObject> bufHandle, uint32_t indexSize) {}
    virtual void* lockBuffer(Handle<HwBufferObject> bufHandle, uint32_t offset, uint32_t numBytes) { return nullptr; }
    virtual void unlockBuffer(Handle<HwBufferObject> bufHandle) {}
    virtual Handle<HwBufferObject> createBufferObject(
        uint32_t bindingType,
        uint32_t bufferCreationFlags,
        uint32_t byteCount,
        const char* debugName = nullptr);
    virtual void updateBufferObject(
        Handle<HwBufferObject> boh, 
        const char* data, 
        uint32_t size);
    virtual Handle<HwDescriptorSetLayout> createDescriptorSetLayout(DescriptorSetLayout& info);
    virtual Handle<HwDescriptorSetLayout> getDescriptorSetLayout(Handle<HwProgram> programHandle, uint32_t set);
    virtual Handle<HwDescriptorSet> createDescriptorSet(Handle<HwDescriptorSetLayout> dslh);
    virtual Handle<HwPipelineLayout> createPipelineLayout(std::array<Handle<HwDescriptorSetLayout>, 4>& layouts);
    virtual Handle<HwProgram> createShaderProgram(const ShaderInfo& mShaderInfo, VertexDeclaration* decl);
    virtual Handle<HwDescriptorSetLayout> getDescriptorSetLayout(
        Handle<HwComputeProgram> programHandle, uint32_t set);
    virtual Handle<HwRaytracingProgram> createRaytracingProgram(const ShaderInfo& mShaderInfo);
    virtual Handle<HwDescriptorSetLayout> getDescriptorSetLayout(
        Handle<HwRaytracingProgram> programHandle, uint32_t set);
    virtual Handle<HwSampler> createTextureSampler(filament::backend::SamplerParams& samplerParams);
    virtual Handle<HwComputeProgram> createComputeProgram(const ShaderInfo& shaderInfo);
    virtual Handle<HwPipeline> createPipeline(
        backend::RasterState& rasterState,
        Handle<HwProgram>& program);

    virtual void bindDescriptorSet(
        Handle<HwDescriptorSet> dsh,
        uint8_t setIndex,
        backend::DescriptorSetOffsetArray&& offsets);
    virtual void updateDescriptorSetBuffer(
        Handle<HwDescriptorSet> dsh,
        backend::descriptor_binding_t binding,
        backend::BufferObjectHandle* boh,
        uint32_t handleCount);
    virtual void updateDescriptorSetTexture(
        Handle<HwDescriptorSet> dsh,
        backend::descriptor_binding_t binding,
        OgreTexture** tex,
        uint32_t count,
        TextureBindType type = TextureBindType_Image) {}

    virtual void updateDescriptorSetSampler(
        Handle<HwDescriptorSet> dsh,
        backend::descriptor_binding_t binding,
        Handle<HwSampler> samplerHandle) {}

    virtual void updateDescriptorSetSampler(
        Handle<HwDescriptorSet> dsh,
        backend::descriptor_binding_t binding,
        OgreTexture* tex) {}
    virtual void updateDescriptorSetAccelerationStructure(
        Handle<HwDescriptorSet> dsh,
        backend::descriptor_binding_t binding,
        AccelerationStructure* accStructure) {}
    virtual void resourceBarrier(
        uint32_t numBufferBarriers, 
        BufferBarrier* pBufferBarriers,
        uint32_t textureBarrierCount, 
        TextureBarrier* pTextureBarriers,
        uint32_t numRtBarriers, 
        RenderTargetBarrier* pRtBarriers
    ) {}


    virtual void beginCmd() {}
    virtual void flushCmd(bool waitCmd) {}
    //raytracing

    virtual void addAccelerationStructure(
        const AccelerationStructureDesc* pDesc,
        AccelerationStructure** ppAccelerationStructure) {}

    virtual void buildAccelerationStructure(RaytracingBuildASDesc* pDesc) {}

    virtual void removeAccelerationStructureScratch(
        AccelerationStructure* pAccelerationStructure) {}

    //destroy
    virtual void destroyBufferObject(Handle<HwBufferObject> bufHandle) {}
private:
    void renderJob(ArenaScope& arena, FrameGraphPassCallback cb);

protected:
	
    uint32_t mBatchCount = 0;
    uint32_t mTriangleCount = 0;
    uint32_t mLoadResCount = 0;


    String mRenderSystemName;
    uint32_t mRenderType;
    
    utils::JobSystem mJobSystem;
    filament::LinearAllocatorArena mPerRenderPassArena;
};
