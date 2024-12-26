#pragma once

#include "renderSystem.h"
#include "d3dutil.h"
#include "UploadBuffer.h"
#include "dx12Common.h"
#include "dx12ResourceAllocator.h"
#include "dx12PipelineCache.h"


class Dx12GraphicsCommandList;
class Dx12RenderTarget;
class Dx12Texture;
class Dx12RenderWindow;
class DX12Commands;
class DX12SwapChain;
class DxMemoryAllocator;

class Dx12RenderSystemBase : public RenderSystem
{
public:
    Dx12RenderSystemBase();
    ~Dx12RenderSystemBase();

    struct DescriptorHeap** getCPUDescriptorHeaps()
    {
        return mDescriptorHeapContext.mCPUDescriptorHeaps;
    }

    virtual bool engineInit();
    virtual void ready();
    virtual Ogre::RenderWindow* createRenderWindow(
        const CreateWindowDesc& desc) override;
    virtual Ogre::RenderTarget* createRenderTarget(
        const String& name,
        TextureProperty& texProperty) override;
    virtual void frameStart();
    virtual void frameEnd();
    virtual void present();
    virtual void copyImage(
        Ogre::RenderTarget* dst,
        Ogre::RenderTarget* src,
        ImageCopyDesc& desc);
    void copyImage(
        Dx12Texture* dst,
        Dx12Texture* src,
        ImageCopyDesc& desc
    );

    virtual void setViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
    virtual void setScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    virtual void beginRenderPass(
        RenderPassInfo& renderPassInfo);
    virtual void endRenderPass(RenderPassInfo& renderPassInfo);

    virtual void bindPipeline(
        Handle<HwProgram> programHandle,
        Handle<HwPipeline> pipelineHandle,
        const Handle<HwDescriptorSet>* descSets,
        uint32_t setCount);

    virtual void drawIndexed(
        uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        uint32_t vertexOffset,
        uint32_t firstInstance);

    virtual void draw(uint32_t vertexCount, uint32_t firstVertex);
    virtual void drawIndexedIndirect(
        Handle<HwBufferObject> drawBuffer,
        uint32_t offset,
        uint32_t drawCount,
        uint32_t stride
    );
    virtual void beginComputePass(
        ComputePassInfo& computePassInfo);
    virtual void endComputePass();

    virtual void dispatchComputeShader();

    virtual void pushGroupMarker(const char* maker);
    virtual void popGroupMarker();

    virtual void bindVertexBuffer(
        Handle<HwBufferObject> bufHandle, 
        uint32_t binding,
        uint32_t vertexSize);
    virtual void bindIndexBuffer(Handle<HwBufferObject> bufHandle, uint32_t indexSize);
    virtual void* lockBuffer(Handle<HwBufferObject> bufHandle, uint32_t offset, uint32_t numBytes);
    virtual void unlockBuffer(Handle<HwBufferObject> bufHandle);
    virtual Handle<HwBufferObject> createBufferObject(
        BufferObjectBinding bindingType,
        ResourceMemoryUsage memoryUsage,
        uint32_t bufferCreationFlags,
        uint32_t byteCount,
        const char* debugName = nullptr);
    virtual void updateBufferObject(
        Handle<HwBufferObject> boh,
        const char* data,
        uint32_t size);
    virtual Handle<HwDescriptorSetLayout> getDescriptorSetLayout(Handle<HwProgram> programHandle, uint32_t set);
    virtual Handle<HwDescriptorSet> createDescriptorSet(
        Handle<HwProgram> programHandle,
        uint32_t set);
    virtual Handle<HwPipelineLayout> createPipelineLayout(std::array<Handle<HwDescriptorSetLayout>, 4>& layouts);
    virtual Handle<HwProgram> createShaderProgram(const ShaderInfo& mShaderInfo, VertexDeclaration* decl);
    virtual Handle<HwDescriptorSetLayout> getDescriptorSetLayout(
        Handle<HwComputeProgram> programHandle, uint32_t set);
    virtual Handle<HwDescriptorSetLayout> getDescriptorSetLayout(
        Handle<HwRaytracingProgram> programHandle, uint32_t set);
    virtual void updatePushConstants(
        Handle<HwProgram> program,
        uint32_t offset,
        const char* data,
        uint32_t size) {}
    virtual Handle<HwSampler> createTextureSampler(filament::backend::SamplerParams& samplerParams);
    virtual Handle<HwComputeProgram> createComputeProgram(const ShaderInfo& shaderInfo);
    virtual Handle<HwPipeline> createPipeline(
        backend::RasterState& rasterState,
        Handle<HwProgram>& program
    );

    virtual void updateDescriptorSet(
        Handle<HwDescriptorSet> dsh,
        uint32_t count,
        const DescriptorData* pParams
    );
    virtual void resourceBarrier(
        uint32_t numBufferBarriers,
        BufferBarrier* pBufferBarriers,
        uint32_t textureBarrierCount,
        TextureBarrier* pTextureBarriers,
        uint32_t numRtBarriers,
        RenderTargetBarrier* pRtBarriers
    );


    virtual void beginCmd();
    virtual void flushCmd(bool waitCmd);
protected:
    Dx12ResourceAllocator mResourceAllocator;
    DX12Commands* mCommands;

    DX12PipelineCache mDX12PipelineCache;

    Dx12RenderWindow* mRenderWindow;
    DX12SwapChain* mSwapChain;
    ID3D12Device* mDevice;

    DxMemoryAllocator* mMemoryAllocator;

    DescriptorHeapContext mDescriptorHeapContext;

    bool mSetDescriptorHeaps;
};