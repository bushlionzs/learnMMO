/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_BACKEND_VULKANHANDLES_H
#define TNT_FILAMENT_BACKEND_VULKANHANDLES_H

// This needs to be at the top
#include "DriverBase.h"

#include "VulkanBuffer.h"
#include "VulkanResources.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include "VulkanUtility.h"
#include "FVulkanBuffer.h"
#include <SamplerGroup.h>
#include <Program.h>

#include <utils/bitset.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Mutex.h>
#include <utils/StructureOfArrays.h>

namespace filament::backend {

namespace {
// Counts the total number of descriptors for both vertex and fragment stages.
template<typename Bitmask>
inline uint8_t collapsedCount(Bitmask const& mask) {
    static_assert(sizeof(mask) <= 64);
    constexpr uint64_t VERTEX_MASK = (1ULL << getFragmentStageShift<Bitmask>()) - 1ULL;
    constexpr uint64_t FRAGMENT_MASK = (VERTEX_MASK << getFragmentStageShift<Bitmask>());
    uint64_t val = mask.getValue();
    val = ((val & VERTEX_MASK) >> getVertexStageShift<Bitmask>()) |
        ((val & FRAGMENT_MASK) >> getFragmentStageShift<Bitmask>());
    return (uint8_t) Bitmask(val).count();
}



} // anonymous namespace

class VulkanTimestamps;
struct VulkanBufferObject;

struct VulkanDescriptorSetLayout : public VulkanResource, HwDescriptorSetLayout {
    static constexpr uint8_t UNIQUE_DESCRIPTOR_SET_COUNT = 4;
    static constexpr uint8_t MAX_BINDING_SET = 4;
    static constexpr uint8_t MAX_BINDINGS = 25;

    using VulkanLayoutKey = std::array<uint32_t, MAX_BINDINGS>;
    struct VulkanDescriptorSetLayoutInfo
    {
        uint32_t uboCount = 0;
        uint32_t dynamicUboCount = 0;
        uint32_t storeCount = 0;
        uint32_t samplerCount = 0;
        uint32_t combinedImage = 0;
        uint32_t storeImage = 0;
        uint32_t accelerationStructure = 0;
        uint32_t inputAttachmentCount = 0;
    };
    VulkanDescriptorSetLayout(const VulkanDescriptorSetLayoutInfo& info);

    ~VulkanDescriptorSetLayout() = default;

    VkDescriptorSetLayout getVkLayout() const 
    { 
        return mVkLayout; 
    }
    void setVkLayout(VkDescriptorSetLayout vklayout) 
    { 
        mVkLayout = vklayout;
    }

    bool hasUbo()
    {
        return mVulkanDescriptorSetLayoutInfo.uboCount > 0;
    }

    uint32_t getUboCount()
    {
        return mVulkanDescriptorSetLayoutInfo.uboCount;
    }

    bool hasDynamicUbo()
    {
        return mVulkanDescriptorSetLayoutInfo.dynamicUboCount > 0;
    }

    uint32_t getDynamicUboCount()
    {
        return mVulkanDescriptorSetLayoutInfo.dynamicUboCount;
    }

    bool hasStore()
    {
        return mVulkanDescriptorSetLayoutInfo.storeCount > 0;
    }

    uint32_t getStoreCount()
    {
        return mVulkanDescriptorSetLayoutInfo.storeCount;
    }

    bool hasCombinedImage()
    {
        return mVulkanDescriptorSetLayoutInfo.combinedImage > 0;
    }

    uint32_t getCombinedImageCount()
    {
        return mVulkanDescriptorSetLayoutInfo.combinedImage;
    }

    bool hasInputAttachment()
    {
        return mVulkanDescriptorSetLayoutInfo.inputAttachmentCount > 0;
    }

    uint32_t getInputAttachmentCount()
    {
        return mVulkanDescriptorSetLayoutInfo.inputAttachmentCount;
    }

    bool hasStoreImage()
    {
        return mVulkanDescriptorSetLayoutInfo.storeImage > 0;
    }

    uint32_t getStoreImageCount()
    {
        return mVulkanDescriptorSetLayoutInfo.storeImage;
    }

    bool hasAccelerationStructure()
    {
        return mVulkanDescriptorSetLayoutInfo.accelerationStructure > 0;
    }

    uint32_t getAccelerationStructureCount()
    {
        return mVulkanDescriptorSetLayoutInfo.accelerationStructure;
    }
private:
    VkDescriptorSetLayout mVkLayout = VK_NULL_HANDLE;
    VulkanDescriptorSetLayoutInfo mVulkanDescriptorSetLayoutInfo;
};

struct VulkanDescriptorSet : public VulkanResource, HwDescriptorSet {
public:
    // Because we need to recycle descriptor sets not used, we allow for a callback that the "Pool"
    // can use to repackage the vk handle.
    using OnRecycle = std::function<void(VulkanDescriptorSet*)>;

    VulkanDescriptorSet(VulkanResourceAllocator* allocator, VkDescriptorSet rawSet)
        : VulkanResource(VulkanResourceType::DESCRIPTOR_SET),
          vkSet(rawSet),
          mResources(allocator) {}

    ~VulkanDescriptorSet() {
        if (mOnRecycleFn) {
            mOnRecycleFn(this);
        }
    }

    void acquire(VulkanTexture* texture);

    void acquire(VulkanBufferObject* texture);

    // TODO: maybe change to fixed size for performance.
    VkDescriptorSet const vkSet;

private:
    VulkanAcquireOnlyResourceManager mResources;
    OnRecycle mOnRecycleFn;
};

struct VulkanPipelineLayout : public VulkanResource, HwPipelineLayout {
    VulkanPipelineLayout(VkPipelineLayout layout):
        mPipelineLayout(layout),
        VulkanResource(VulkanResourceType::END_TYPE)
    {
    }

    VkPipelineLayout getLayout()
    {
        return mPipelineLayout;
    }
private:
    VkPipelineLayout mPipelineLayout;
};

using PushConstantNameArray = utils::FixedCapacityVector<char const*>;
using PushConstantNameByStage = std::array<PushConstantNameArray, Program::SHADER_TYPE_COUNT>;

struct PushConstantDescription {

    explicit PushConstantDescription(backend::Program const& program) noexcept;

    VkPushConstantRange const* getVkRanges() const noexcept { return mRanges; }

    uint32_t getVkRangeCount() const noexcept { return mRangeCount; }

    void write(VulkanCommands* commands, VkPipelineLayout layout, backend::ShaderStage stage,
            uint8_t index, backend::PushConstantVariant const& value);

private:
    static constexpr uint32_t ENTRY_SIZE = sizeof(uint32_t);

    utils::FixedCapacityVector<backend::ConstantType> mTypes[Program::SHADER_TYPE_COUNT];
    VkPushConstantRange mRanges[Program::SHADER_TYPE_COUNT];
    uint32_t mRangeCount;
};

struct VulkanProgram : public HwProgram, VulkanResource {
    VulkanProgram(const std::string& name) noexcept;

    ~VulkanProgram();

    inline VkShaderModule getVertexShader() const {
        return mShaders[0];
    }

    inline VkShaderModule getGeometryShader() const
    {
        return mShaders[1];
    }

    inline VkShaderModule getFragmentShader() const 
    { 
        return mShaders[2];
    }

    
    void updateVertexShader(VkShaderModule shaderModule)
    {
        mShaders[0] = shaderModule;
    }

    void updateGeometryShader(VkShaderModule shaderModule)
    {
        mShaders[1] = shaderModule;
    }

    void updateFragmentShader(VkShaderModule shaderModule)
    {
        mShaders[2] = shaderModule;
    }

    void updateVulkanPipelineLayout(VkPipelineLayout layout)
    {
        mPipelineLayout = layout;
    }

    VkPipelineLayout getVulkanPipelineLayout()
    {
        return mPipelineLayout;
    }

    void updateLayout(uint32_t index, Handle<HwDescriptorSetLayout> h)
    {
        mLayouts[index] = h;
    }

    Handle<HwDescriptorSetLayout> getLayout(uint32_t index)
    {
        return mLayouts[index];
    }

    std::vector<VkVertexInputBindingDescription>& getVertexInputBindings()
    {
        return mVertexInputBindings;
    }

    std::vector<VkVertexInputAttributeDescription>& getAttributeDescriptions()
    {
        return mAttributeDescriptions;
    }

    void updatePushConstantsSize(uint32_t size)
    {
        mPushConstantsSize = size;
    }
    static constexpr uint8_t const MAX_SHADER_MODULES = 3;

private:
    VkShaderModule mShaders[MAX_SHADER_MODULES];
    VkPipelineLayout mPipelineLayout;
    Handle<HwDescriptorSetLayout> mLayouts[4];
    std::vector<VkVertexInputBindingDescription> mVertexInputBindings;
    std::vector<VkVertexInputAttributeDescription> mAttributeDescriptions;
    uint32_t mPushConstantsSize;
};

struct VulkanTextureSampler : public HwSampler, VulkanResource {
    VulkanTextureSampler(backend::SamplerParams& samplerParams);
    ~VulkanTextureSampler();

    VkSampler getSampler()
    {
        return mVkSampler;
    }
private:
    VkSampler mVkSampler;
};

struct VulkanRaytracingProgram : public HwRaytracingProgram, VulkanResource {
    VulkanRaytracingProgram(const std::string& name) noexcept;
    ~VulkanRaytracingProgram();
    void updatePipelineLayout(VkPipelineLayout pipelineLayout)
    {
        mVkPipelineLayout = pipelineLayout;
    }
    void updatePipeline(VkPipeline pipeline)
    {
        mVkPipeline = pipeline;
    }

    VkPipelineLayout getPipelineLayout()
    {
        return mVkPipelineLayout;
    }

    VkPipeline getPipeline()
    {
        return mVkPipeline;
    }

    void updateLayout(uint32_t set, Handle<HwDescriptorSetLayout> layoutHandle)
    {
        mLayouts[set] = layoutHandle;
    }

    Handle<HwDescriptorSetLayout> getLayout(uint32_t set)
    {
        return mLayouts[set];
    }

    void updateShaderBindingTables(ShaderBindingTables* tables)
    {
        mShaderBindingTables = tables;
    }

    ShaderBindingTables* getShaderBindingTables()
    {
        return mShaderBindingTables;
    }
private:
    VkPipelineLayout mVkPipelineLayout;
    VkPipeline mVkPipeline;
    Handle<HwDescriptorSetLayout> mLayouts[VulkanDescriptorSetLayout::MAX_BINDING_SET];

    ShaderBindingTables* mShaderBindingTables;
};
struct VulkanComputeProgram : public HwComputeProgram, VulkanResource {
    VulkanComputeProgram(const std::string& name) noexcept;

    ~VulkanComputeProgram();
    void updateComputeShader(VkShaderModule shaderModule)
    {
        mShader = shaderModule;
    }

    void updateSetLayout(VkDescriptorSetLayout layout)
    {
        mLayout = layout;
    }

    void updateSetLayoutHandle(uint32_t set, Handle<HwDescriptorSetLayout> layoutHandle)
    {
        mLayoutHandleArray[set] = layoutHandle;
    }

    Handle<HwDescriptorSetLayout> getSetLayoutHandle(uint32_t set)
    {
        return mLayoutHandleArray[set];
    }

    void updatePipelineLayout(VkPipelineLayout layout)
    {
        mPipelineLayout = layout;
    }

    VkPipelineLayout getPipelineLayout()
    {
        return mPipelineLayout;
    }

    void updatePipeline(VkPipeline pipeline)
    {
        mPipeline = pipeline;
    }
    VkPipeline getPipeline()
    {
        return mPipeline;
    }
private:
    VkShaderModule mShader;
    VkPipelineLayout mPipelineLayout;
    VkPipeline       mPipeline;
    VkDescriptorSetLayout mLayout;
    Handle<HwDescriptorSetLayout> mLayoutHandle;

    std::array<Handle<HwDescriptorSetLayout>, 4> mLayoutHandleArray;
};

struct VulkanPipeline : private HwPipeline, VulkanResource
{
    VulkanPipeline(VkPipeline pipeline, VkPipeline pipelineShadow);

    VkPipeline getPipeline()
    {
        return mPipeline;
    }

    VkPipeline getPipelineShadow()
    {
        return mPipelineShadow;
    }
private:
    VkPipeline mPipeline;
    VkPipeline mPipelineShadow;
};
// The render target bundles together a set of attachments, each of which can have one of the
// following ownership semantics:
//
// - The attachment's VkImage is shared and the owner is VulkanSwapChain (mOffscreen = false).
// - The attachment's VkImage is shared and the owner is VulkanTexture   (mOffscreen = true).
//
// We use private inheritance to shield clients from the width / height fields in HwRenderTarget,
// which are not representative when this is the default render target.
struct VulkanRenderTarget : private HwRenderTarget, VulkanResource {
    // Creates an offscreen render target.
    VulkanRenderTarget(VkDevice device, VkPhysicalDevice physicalDevice,
            VulkanContext const& context, VmaAllocator allocator,
            VulkanCommands* commands,
            VulkanResourceAllocator* handleAllocator,
            uint32_t width, uint32_t height,
            uint8_t samples, VulkanAttachment color[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT],
            VulkanAttachment depthStencil[2], VulkanStagePool& stagePool, uint8_t layerCount);

    // Creates a special "default" render target (i.e. associated with the swap chain)
    explicit VulkanRenderTarget();

    void transformClientRectToPlatform(VkRect2D* bounds) const;
    void transformClientRectToPlatform(VkViewport* bounds) const;
    VkExtent2D getExtent() const;
    // We return references in the following methods to avoid a copy.
    VulkanAttachment& getColor(int target);
    VulkanAttachment& getMsaaColor(int target);
    VulkanAttachment& getDepth();
    VulkanAttachment& getMsaaDepth();
    uint8_t getColorTargetCount(const VulkanRenderPass& pass) const;
    uint8_t getSamples() const { return mSamples; }
    uint8_t getLayerCount() const { return mLayerCount; }
    bool hasDepth() const { return mDepth.texture; }
    bool isSwapChain() const { return !mOffscreen; }
    void bindToSwapChain(VulkanSwapChain& surf);

private:
    VulkanAttachment mColor[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VulkanAttachment mDepth = {};
    VulkanAttachment mMsaaAttachments[MRT::MAX_SUPPORTED_RENDER_TARGET_COUNT] = {};
    VulkanAttachment mMsaaDepthAttachment = {};
    const bool mOffscreen : 1;
    uint8_t mSamples : 7;
    uint8_t mLayerCount = 1;
};

struct VulkanBufferObject;

struct VulkanVertexBufferInfo : public HwVertexBufferInfo, VulkanResource {
    VulkanVertexBufferInfo(uint8_t bufferCount, uint8_t attributeCount,
            AttributeArray const& attributes);

    inline VkVertexInputAttributeDescription const* getAttribDescriptions() const {
        return mInfo.mSoa.data<PipelineInfo::ATTRIBUTE_DESCRIPTION>();
    }

    inline VkVertexInputBindingDescription const* getBufferDescriptions() const {
        return mInfo.mSoa.data<PipelineInfo::BUFFER_DESCRIPTION>();
    }

    inline int8_t const* getAttributeToBuffer() const {
        return mInfo.mSoa.data<PipelineInfo::ATTRIBUTE_TO_BUFFER_INDEX>();
    }

    inline VkDeviceSize const* getOffsets() const {
        return mInfo.mSoa.data<PipelineInfo::OFFSETS>();
    }

    size_t getAttributeCount() const noexcept {
        return mInfo.mSoa.size();
    }

private:
    struct PipelineInfo {
        PipelineInfo(size_t size) : mSoa(size /* capacity */) {
            mSoa.resize(size);
        }

        // These correspond to the index of the element in the SoA
        static constexpr uint8_t ATTRIBUTE_DESCRIPTION = 0;
        static constexpr uint8_t BUFFER_DESCRIPTION = 1;
        static constexpr uint8_t OFFSETS = 2;
        static constexpr uint8_t ATTRIBUTE_TO_BUFFER_INDEX = 3;

        utils::StructureOfArrays<
            VkVertexInputAttributeDescription,
            VkVertexInputBindingDescription,
            VkDeviceSize,
            int8_t
        > mSoa;
    };

    PipelineInfo mInfo;
};

struct VulkanVertexBuffer : public HwVertexBuffer, VulkanResource {
    VulkanVertexBuffer(VulkanContext& context, VulkanStagePool& stagePool,
            VulkanResourceAllocator* allocator,
            uint32_t vertexCount, Handle<HwVertexBufferInfo> vbih);

    void setBuffer(VulkanResourceAllocator const& allocator,
            VulkanBufferObject* bufferObject, uint32_t index);

    inline VkBuffer const* getVkBuffers() const {
        return mBuffers.data();
    }

    inline VkBuffer* getVkBuffers() {
        return mBuffers.data();
    }

    Handle<HwVertexBufferInfo> vbih;
private:
    utils::FixedCapacityVector<VkBuffer> mBuffers;
    FixedSizeVulkanResourceManager<MAX_VERTEX_BUFFER_COUNT> mResources;
};

struct VulkanIndexBuffer : public HwIndexBuffer, VulkanResource {
    VulkanIndexBuffer(VmaAllocator allocator, VulkanStagePool& stagePool, uint8_t elementSize,
            uint32_t indexCount)
        : HwIndexBuffer(elementSize, indexCount),
          VulkanResource(VulkanResourceType::INDEX_BUFFER),
          buffer(allocator, stagePool, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, elementSize * indexCount),
          indexType(elementSize == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32) {}

    VulkanBuffer buffer;
    const VkIndexType indexType;
};

struct VulkanBufferObject : public HwBufferObject, VulkanResource {
    VulkanBufferObject(
        VmaAllocator allocator, 
        VulkanStagePool& stagePool, 
        uint32_t byteCount,
        BufferObjectBinding bindingType,
        uint32_t bufferCreationFlags);

    VulkanBuffer buffer;
    const BufferObjectBinding bindingType;
};

struct VulkanSamplerGroup : public HwSamplerGroup, VulkanResource {
    // NOTE: we have to use out-of-line allocation here because the size of a Handle<> is limited
    std::unique_ptr<SamplerGroup> sb;// FIXME: this shouldn't depend on filament::SamplerGroup
    explicit VulkanSamplerGroup(size_t size) noexcept
        : VulkanResource(VulkanResourceType::SAMPLER_GROUP),
          sb(new SamplerGroup(size)) {}
};

struct VulkanRenderPrimitive : public HwRenderPrimitive, VulkanResource {
    VulkanRenderPrimitive(VulkanResourceAllocator* resourceAllocator,
            PrimitiveType pt, Handle<HwVertexBuffer> vbh, Handle<HwIndexBuffer> ibh);

    ~VulkanRenderPrimitive() {
        mResources.clear();
    }

    VulkanVertexBuffer* vertexBuffer = nullptr;
    VulkanIndexBuffer* indexBuffer = nullptr;

private:
    // Keep references to the vertex buffer and the index buffer.
    FixedSizeVulkanResourceManager<2> mResources;
};

struct VulkanFence : public HwFence, VulkanResource {
    VulkanFence()
        : VulkanResource(VulkanResourceType::FENCE) {}

    explicit VulkanFence(std::shared_ptr<VulkanCmdFence> fence)
        : VulkanResource(VulkanResourceType::FENCE),
          fence(fence) {}

    std::shared_ptr<VulkanCmdFence> fence;
};

struct VulkanTimerQuery : public HwTimerQuery, VulkanThreadSafeResource {
    explicit VulkanTimerQuery(std::tuple<uint32_t, uint32_t> indices);
    ~VulkanTimerQuery();

    void setFence(std::shared_ptr<VulkanCmdFence> fence) noexcept;

    bool isCompleted() noexcept;

    uint32_t getStartingQueryIndex() const {
        return mStartingQueryIndex;
    }

    uint32_t getStoppingQueryIndex() const {
        return mStoppingQueryIndex;
    }

private:
    uint32_t mStartingQueryIndex;
    uint32_t mStoppingQueryIndex;

    std::shared_ptr<VulkanCmdFence> mFence;
    utils::Mutex mFenceMutex;
};


inline  VkBufferUsageFlags getBufferObjectUsage(
        uint32_t bindingType,
        uint32_t creationFlags) noexcept
{
    VkBufferUsageFlags flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (bindingType & BufferObjectBinding::BufferObjectBinding_Vertex)
    {
        flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }

    if (bindingType & BufferObjectBinding::BufferObjectBinding_Index)
    {
        flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (bindingType & BufferObjectBinding::BufferObjectBinding_Uniform)
    {
        flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }

    if (bindingType & BufferObjectBinding::BufferObjectBinding_Storge)
    {
        flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }

    if (bindingType & BufferObjectBinding::BufferObjectBinding_InDirectBuffer)
    {
        flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
    }

    if (bindingType & BufferObjectBinding::BufferObjectBinding_AccelerationStructure)
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
    }

    if (creationFlags & BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT)
    {
        flags |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    }
    if (creationFlags & BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS)
    {
        flags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    }
    if (creationFlags & BUFFER_CREATION_FLAG_SHADER_BINDING_TABLE)
    {
        flags |= VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
    }
    return flags;
}

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANHANDLES_H
