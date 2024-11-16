#pragma once

#include "renderSystem.h"
#include "d3dutil.h"
#include "UploadBuffer.h"
#include "dx12Common.h"
#include "dx12ResourceAllocator.h"
#include "dx12PipelineCache.h"

class Dx12GraphicsCommandList;
class Dx12RenderTarget;
class Dx12TextureHandleManager;
class Dx12ShadowMap;
class Dx12RenderWindow;
class DX12Commands;

class Dx12RenderSystemBase : public RenderSystem
{
public:
    Dx12RenderSystemBase();
    ~Dx12RenderSystemBase();

    Dx12TextureHandleManager* getTextureHandleManager()
    {
        return nullptr;
    }

    virtual bool engineInit();
    virtual void ready();
   
    virtual void beginRenderPass(
        RenderPassInfo& renderPassInfo);
    virtual void endRenderPass(RenderPassInfo& renderPassInfo);

    virtual void bindPipeline(
        Handle<HwProgram> programHandle,
        Handle<HwPipeline> pipelineHandle,
        Handle<HwDescriptorSet>* descSets,
        uint32_t setCount);

    virtual void copyImage(Ogre::RenderTarget* dst, Ogre::RenderTarget* src);
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
    virtual void present();

    virtual void pushGroupMarker(const char* maker);
    virtual void popGroupMarker();

    virtual Ogre::OgreTexture* generateCubeMap(
        const std::string& name,
        Ogre::OgreTexture* environmentCube,
        Ogre::PixelFormat format,
        int32_t dim,
        CubeType type);
    virtual Ogre::OgreTexture* generateBRDFLUT(const std::string& name);
    virtual void bindVertexBuffer(Handle<HwBufferObject> bufHandle, uint32_t binding);
    virtual void bindIndexBuffer(Handle<HwBufferObject> bufHandle, uint32_t indexSize);
    virtual void* lockBuffer(Handle<HwBufferObject> bufHandle, uint32_t offset, uint32_t numBytes);
    virtual void unlockBuffer(Handle<HwBufferObject> bufHandle);
    virtual Handle<HwBufferObject> createBufferObject(
        uint32_t bindingType,
        uint32_t bufferCreationFlags,
        uint32_t byteCount,
        const char* debugName = nullptr);
    virtual void updateBufferObject(
        Handle<HwBufferObject> boh,
        const char* data,
        uint32_t size);
    virtual Handle<HwDescriptorSetLayout> getDescriptorSetLayout(Handle<HwProgram> programHandle, uint32_t set);
    virtual Handle<HwDescriptorSet> createDescriptorSet(Handle<HwDescriptorSetLayout> dslh);
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
        TextureBindType type = TextureBindType_Image);

    virtual void updateDescriptorSetSampler(
        Handle<HwDescriptorSet> dsh,
        backend::descriptor_binding_t binding,
        Handle<HwSampler> samplerHandle);

    virtual void updateDescriptorSetSampler(
        Handle<HwDescriptorSet> dsh,
        backend::descriptor_binding_t binding,
        OgreTexture* tex);

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
    DX12Commands* mDX12Commands;

    DX12PipelineCache mDX12PipelineCache;

    Dx12RenderWindow* mRenderWindow;

    ID3D12Device* mDevice;
};