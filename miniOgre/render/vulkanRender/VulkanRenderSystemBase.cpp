#include <OgreHeader.h>
#include "OgreRoot.h"
#include "OgreNode.h"
#include "OgreResourceManager.h"
#include "OgreMeshManager.h"
#include "OgreSceneManager.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreVertexDeclaration.h"
#include "VulkanRenderSystemBase.h"
#include "VulkanWindow.h"
#include "VulkanTexture.h"
#include "VulkanHelper.h"
#include "VulkanMappings.h"
#include "VulkanTools.h"
#include "VulkanPipelineLayoutCache.h"
#include "VulkanPipelineCache.h"
#include "VulkanBuffer.h"
#include "VulkanLayoutCache.h"
#include "shaderManager.h"
#include "glslUtil.h"
#include <vk_mem_alloc.h>
#define MAX_HANDLE_COUNT 256
static VmaAllocator createAllocator(VkInstance instance, VkPhysicalDevice physicalDevice,
    VkDevice device) {
    VmaAllocator allocator;
    VmaVulkanFunctions const funcs{
#if VMA_DYNAMIC_VULKAN_FUNCTIONS
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
#else
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vkAllocateMemory,
        .vkFreeMemory = vkFreeMemory,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory = vkBindBufferMemory,
        .vkBindImageMemory = vkBindImageMemory,
        .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
        .vkCreateBuffer = vkCreateBuffer,
        .vkDestroyBuffer = vkDestroyBuffer,
        .vkCreateImage = vkCreateImage,
        .vkDestroyImage = vkDestroyImage,
        .vkCmdCopyBuffer = vkCmdCopyBuffer,
        .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
        .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR
#endif
    };
    VmaAllocatorCreateInfo  allocatorInfo{
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &funcs,
        .instance = instance,
    };
    VulkanSettings& settings = VulkanHelper::getSingleton().getVulkanSettings();
    if (settings.mRayPipelineSupported)
    {
        allocatorInfo.flags |= 
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    }
    
    vmaCreateAllocator(&allocatorInfo, &allocator);
    return allocator;
}

VulkanRenderSystemBase::VulkanRenderSystemBase()
    :
    mResourceAllocator(83886080, false)
{
    new VulkanHelper(this);
}

VulkanRenderSystemBase::~VulkanRenderSystemBase()
{

}

bool VulkanRenderSystemBase::engineInit(bool raytracing)
{
    mVulkanPlatform = new VulkanPlatform();


    VulkanHelper& helper = VulkanHelper::getSingleton();

    helper.getVulkanSettings().mRayPipelineSupported = raytracing;

    helper._initialise(mVulkanPlatform);

    auto device = mVulkanPlatform->getDevice();
    mAllocator = createAllocator(
        mVulkanPlatform->getInstance(), mVulkanPlatform->getPhysicalDevice(), device);
    
    auto queue = mVulkanPlatform->getGraphicsQueue();
    auto queueIndex = mVulkanPlatform->getGraphicsQueueIndex();

    mCommands = new VulkanCommands(device, queue, queueIndex, &mVulkanContext, &mResourceAllocator);
    mStagePool = new VulkanStagePool(mAllocator, mCommands);

    mPipelineCache = helper.getPipelineCache();
    mPipelineLayoutCache = helper.getPipelineLayoutCache();

    mVulkanSettings = &helper.getVulkanSettings();
    mVulkanLayoutCache = new VulkanLayoutCache(device, &mResourceAllocator);
    mDescriptorInfinitePool = new DescriptorInfinitePool(device);

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkCreateDescriptorSetLayout(device, &layoutCreateInfo,
        nullptr,
        &pEmptyDescriptorSetLayout);
    VkDescriptorPoolSize descriptorPoolSizes[1] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1 } };
    VkDescriptorPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.pNext = NULL;
    poolCreateInfo.poolSizeCount = 1;
    poolCreateInfo.pPoolSizes = descriptorPoolSizes;
    poolCreateInfo.flags = 0;
    poolCreateInfo.maxSets = 1;
    vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &pEmptyDescriptorPool);

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.descriptorPool = pEmptyDescriptorPool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &pEmptyDescriptorSetLayout;
    vkAllocateDescriptorSets(device, &alloc_info, &pEmptyDescriptorSet);
    return true;
}

OgreTexture* VulkanRenderSystemBase::createTextureFromFile(
    const std::string& name,
    TextureProperty* texProperty)
{
    auto tex = new VulkanTexture(name, mVulkanPlatform, mCommands, texProperty);
    return tex;
}

Ogre::OgreTexture* VulkanRenderSystemBase::createTexture(Ogre::TextureProperty* texProperty)
{
    auto tex = new VulkanTexture("", mVulkanPlatform, mCommands, nullptr, texProperty);
    tex->load(nullptr);
    return tex;
}

void VulkanRenderSystemBase::ready()
{
    VulkanHelper::getSingleton().loadDefaultResources();
}

Ogre::RenderWindow* VulkanRenderSystemBase::createRenderWindow(
    const String& name, unsigned int width, unsigned int height,
    const NameValuePairList* miscParams)
{
    mRenderWindow = new VulkanWindow();

    auto itor = miscParams->find("externalWindowHandle");
    if (itor == miscParams->end())
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "externalWindowHandle should be provided");
    }

    HWND wnd = (HWND)StringConverter::parseSizeT(itor->second);

    bool srgb = false;

    itor = miscParams->find("srgb");

    if (itor != miscParams->end())
    {
        srgb = true;
    }
    
    
    VkExtent2D extent;
    extent.width = 0;
    extent.height = 0;

    uint32_t flags = 0;

    if (srgb)
    {
        flags = backend::SWAP_CHAIN_CONFIG_SRGB_COLORSPACE;
    }
    //flags |= SWAP_CHAIN_CONFIG_HAS_STENCIL_BUFFER;
    mSwapChain = new VulkanSwapChain(
        mVulkanPlatform, 
        mVulkanContext, 
        mAllocator, mCommands, &mResourceAllocator, *mStagePool, (void*)wnd, flags, extent);

    mRenderWindow->create(mSwapChain);

    return mRenderWindow;
}

Ogre::RenderTarget* VulkanRenderSystemBase::createRenderTarget(
    const String& name, 
    TextureProperty& texProperty)
{
    if (texProperty._tex_usage & (uint32_t)Ogre::TextureUsage::DEPTH_ATTACHMENT)
    {
        texProperty._samplerParams.wrapS = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
        texProperty._samplerParams.wrapT = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
        texProperty._samplerParams.wrapR = filament::backend::SamplerWrapMode::CLAMP_TO_EDGE;
    }
    Ogre::VulkanRenderTarget* renderTarget = new Ogre::VulkanRenderTarget(
        name, mVulkanPlatform, mCommands, texProperty);
    return renderTarget;
}

void VulkanRenderSystemBase::frameStart()
{
    mTriangleCount = 0;
    mBatchCount = 0;
    mLastPipeline = VK_NULL_HANDLE;
    if (mCommandBuffer == nullptr)
    {
        mCommandBuffer = mCommands->get().buffer();
    }
    
    bool resized = false;
    mSwapChain->acquire(resized);
}



void VulkanRenderSystemBase::frameEnd()
{
    mStagePool->gc();
    mCommandBuffer = nullptr;
}

void VulkanRenderSystemBase::setViewport(
    float x, float y, float width, float height, float minDepth, float maxDepth)
{
    VkViewport viewport{};
    viewport.x = 0.0;
    viewport.y = height;
    viewport.width = width;
    viewport.height = -(float)height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    bluevk::vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
}

void VulkanRenderSystemBase::setScissor(
    uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);

    bluevk::vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
}

void VulkanRenderSystemBase::beginRenderPass(
    RenderPassInfo& renderPassInfo)
{
    VkCommandBuffer cmdBuffer = mCommands->get().buffer();

    VkRenderingAttachmentInfo colorAttachments[MAX_RENDER_TARGET_ATTACHMENTS] = {};
    VkRenderingAttachmentInfo depthAttachment = {};

    for (auto i = 0; i < renderPassInfo.renderTargetCount; i++)
    {
        colorAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorAttachments[i].pNext = NULL;
        colorAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachments[i].loadOp = VulkanMappings::getVkAttachmentLoadOp(renderPassInfo.renderLoadAction);
        colorAttachments[i].storeOp = VulkanMappings::getVkAttachmentStoreOp(renderPassInfo.renderStoreAction);
        Ogre::VulkanRenderTarget* rt = (Ogre::VulkanRenderTarget*)renderPassInfo.renderTargets[i].renderTarget;

        vks::tools::insertImageMemoryBarrier(
            cmdBuffer,
            rt->getImage(),
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

        colorAttachments[i].imageView = rt->getImageView();
        const ClearValue* clearValue = &renderPassInfo.renderTargets[i].clearColour;
        colorAttachments[i].clearValue.color = { { clearValue->r, clearValue->g, clearValue->b, clearValue->a } };
    }

    bool hasDepth = renderPassInfo.depthTarget.depthStencil != nullptr;

    if (hasDepth)
    {
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VulkanMappings::getVkAttachmentLoadOp(renderPassInfo.depthLoadAction);
        depthAttachment.storeOp = VulkanMappings::getVkAttachmentStoreOp(renderPassInfo.depthStoreAction);

        Ogre::VulkanRenderTarget* rt = (Ogre::VulkanRenderTarget*)renderPassInfo.depthTarget.depthStencil;
        depthAttachment.imageView = rt->getImageView();

        vks::tools::insertImageMemoryBarrier(
            cmdBuffer,
            rt->getImage(),
            0,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT , 0, 1, 0, 1 });

        const ClearValue* clearValue = &renderPassInfo.depthTarget.clearValue;
        depthAttachment.clearValue.depthStencil = { clearValue->depth, clearValue->stencil };

    }

    VkRect2D renderArea = {};
    renderArea.offset.x = 0;
    renderArea.offset.y = 0;
    uint32_t layerCount = 0;
    if (renderPassInfo.renderTargetCount)
    {
        renderArea.extent.width = renderPassInfo.renderTargets[0].renderTarget->getWidth();
        renderArea.extent.height = renderPassInfo.renderTargets[0].renderTarget->getHeight();
        layerCount = 1;
    }
    else if (hasDepth)
    {
        renderArea.extent.width = renderPassInfo.depthTarget.depthStencil->getWidth();
        renderArea.extent.height = renderPassInfo.depthTarget.depthStencil->getHeight();
        layerCount = 1;
    }
    else
    {
        assert(false);
    }

    VkRenderingInfoKHR renderingInfo = {};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    renderingInfo.pColorAttachments = colorAttachments;
    renderingInfo.colorAttachmentCount = renderPassInfo.renderTargetCount;
    renderingInfo.pDepthAttachment = hasDepth ? &depthAttachment : nullptr;
    renderingInfo.pStencilAttachment = nullptr;
    renderingInfo.renderArea = renderArea;
    renderingInfo.layerCount = layerCount;


    bluevk::vkCmdBeginRenderingKHR(cmdBuffer, &renderingInfo);

    if (renderPassInfo.viewport)
    {
        auto width = renderArea.extent.width;
        auto height = renderArea.extent.height;
        VkViewport viewport{};
        viewport.x = 0.0;
        viewport.y = height;
        viewport.width = width;
        viewport.height = -(float)height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor = vks::initializers::rect2D(width, height, 0, 0);

        bluevk::vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
        bluevk::vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    }
}

void VulkanRenderSystemBase::endRenderPass(RenderPassInfo& renderPassInfo)
{
    vkCmdEndRenderingKHR(mCommandBuffer);
}


void VulkanRenderSystemBase::bindPipeline(
    Handle<HwProgram> programHandle,
    Handle<HwPipeline> pipelineHandle,
    Handle<HwDescriptorSet>* descSets,
    uint32_t setCount)
{
    VulkanShaderProgram* vulkanProgram = mResourceAllocator.handle_cast<VulkanShaderProgram*>(programHandle);
    VkCommandBuffer commandBuffer = mCommands->get().buffer();
    VulkanPipeline* vulkanPipeline = mResourceAllocator.handle_cast<VulkanPipeline*>(pipelineHandle);

    auto pipeline = vulkanPipeline->getPipeline();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    auto pipelineLayout = vulkanProgram->getVulkanPipelineLayout();


    if (setCount > 0)
    {
        VkDescriptorSet descriptorSet[4];

        for (auto i = 0; i < setCount; i++)
        {
            if (descSets[i])
            {
                VulkanDescriptorSet* set = mResourceAllocator.handle_cast<VulkanDescriptorSet*>(descSets[i]);
                descriptorSet[i] = set->vkSet;
            }
            else
            {
                descriptorSet[i] = pEmptyDescriptorSet;
            }
        }

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 0, setCount, &descriptorSet[0], 0, nullptr);
    }
    
}

void VulkanRenderSystemBase::draw(uint32_t vertexCount, uint32_t firstVertex)
{
    vkCmdDraw(mCommandBuffer, vertexCount, 1, firstVertex, 0);
    incrTriangleCount(vertexCount / 3);
    incrBatchCount(1);
}

void VulkanRenderSystemBase::drawIndexed(
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    uint32_t vertexOffset,
    uint32_t firstInstance)
{
    vkCmdDrawIndexed(mCommandBuffer, indexCount, instanceCount,
        firstIndex, vertexOffset, firstInstance);

    incrTriangleCount(indexCount / 3);

    incrBatchCount(1);
}

void VulkanRenderSystemBase::drawIndexedIndirect(
    Handle<HwBufferObject> drawBuffer,
    uint32_t offset,
    uint32_t drawCount,
    uint32_t stride
)
{
    auto cmdBuffer = mCommands->get().buffer();
    VulkanBufferObject* vulkanBufferObject = mResourceAllocator.handle_cast<VulkanBufferObject*>(drawBuffer);
    VkBuffer vkBuf = vulkanBufferObject->buffer.getGpuBuffer();
    vkCmdDrawIndexedIndirect(cmdBuffer, vkBuf, offset, drawCount, stride);
}

void VulkanRenderSystemBase::beginComputePass(ComputePassInfo& computePassInfo)
{
    VulkanComputeProgram* program = mResourceAllocator.handle_cast<VulkanComputeProgram*>(computePassInfo.programHandle);

    auto pipeline = program->getPipeline();
    auto pipelineLayout = program->getPipelineLayout();
    VkCommandBuffer commandBuffer = mCommands->get().buffer();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    VkDescriptorSet descriptorSet[4];
    uint32_t index = 0;
    for (auto& ds : computePassInfo.descSets)
    {
        VulkanDescriptorSet* set = mResourceAllocator.handle_cast<VulkanDescriptorSet*>(ds);
        descriptorSet[index] = set->vkSet;
        index++;
    }

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        pipelineLayout, 0, index, &descriptorSet[0], 0, nullptr);



    vkCmdDispatch(commandBuffer,
        computePassInfo.computeGroup.x, computePassInfo.computeGroup.y, computePassInfo.computeGroup.z);

}

void VulkanRenderSystemBase::endComputePass()
{

}

void VulkanRenderSystemBase::present()
{
    mSwapChain->present();
}

void VulkanRenderSystemBase::copyImage(
    Ogre::RenderTarget* dst,
    Ogre::RenderTarget* src,
    ImageCopyDesc& desc)
{
    VkImageCopy copyRegion{};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.baseArrayLayer = desc.srcSubresource.baseArrayLayer;
    copyRegion.srcSubresource.layerCount = desc.srcSubresource.layerCount;
    copyRegion.srcSubresource.mipLevel = desc.srcSubresource.mipLevel;

    copyRegion.srcOffset = { 0, 0, 0 };
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.baseArrayLayer = desc.dstSubresource.baseArrayLayer;
    copyRegion.dstSubresource.layerCount = desc.dstSubresource.layerCount;
    copyRegion.dstSubresource.mipLevel = desc.dstSubresource.mipLevel;
    copyRegion.dstOffset = { 0, 0, 0 };
    copyRegion.extent.width = desc.extent.width;
    copyRegion.extent.height = desc.extent.height;
    copyRegion.extent.depth = desc.extent.depth;
    VulkanTexture* srcImage = (VulkanTexture*)src->getTarget();
    VulkanTexture* dstImage = (VulkanTexture*)dst->getTarget();
    vkCmdCopyImage(mCommandBuffer, srcImage->getVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImage->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &copyRegion);
}

void VulkanRenderSystemBase::pushGroupMarker(const char* maker)
{
    if (mVulkanSettings->mDebugUtilsExtension)
    {
        VkDebugUtilsLabelEXT markerInfo = {};
        markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        markerInfo.color[0] = 1.0f;
        markerInfo.color[1] = 1.0f;
        markerInfo.color[2] = 0.0f;
        markerInfo.color[3] = 1.0f;
        markerInfo.pLabelName = maker;
        vkCmdBeginDebugUtilsLabelEXT(mCommands->get().buffer(), &markerInfo);
    }
    
}

void VulkanRenderSystemBase::popGroupMarker()
{
    if (mVulkanSettings->mDebugUtilsExtension)
    {
        vkCmdEndDebugUtilsLabelEXT(mCommands->get().buffer());
    }
}

void* VulkanRenderSystemBase::lockBuffer(Handle<HwBufferObject> bufHandle, uint32_t offset, uint32_t numBytes)
{
    VulkanBufferObject* vulkanBufferObject = mResourceAllocator.handle_cast<VulkanBufferObject*>(bufHandle);
    return vulkanBufferObject->buffer.lock(offset, numBytes);
}

void VulkanRenderSystemBase::unlockBuffer(Handle<HwBufferObject> bufHandle)
{
    VulkanBufferObject* vulkanBufferObject = mResourceAllocator.handle_cast<VulkanBufferObject*>(bufHandle);
    vulkanBufferObject->buffer.unlock(mCommands->get().buffer());
}

void VulkanRenderSystemBase::bindVertexBuffer(
    Handle<HwBufferObject> bufferHandle, 
    uint32_t binding,
    uint32_t vertexSize)
{
    VulkanBufferObject* vulkanBufferObject = mResourceAllocator.handle_cast<VulkanBufferObject*>(bufferHandle);

    VkDeviceSize offsets[1] = { 0 };
    auto cmdBuffer = mCommands->get().buffer();

    VkBuffer vkBuffer = vulkanBufferObject->buffer.getGpuBuffer();
    vkCmdBindVertexBuffers(cmdBuffer, binding, 1, &vkBuffer, offsets);

}

void VulkanRenderSystemBase::bindIndexBuffer(Handle<HwBufferObject> bufferHandle, uint32_t indexSize)
{
    VulkanBufferObject* vulkanBufferObject = mResourceAllocator.handle_cast<VulkanBufferObject*>(bufferHandle);
    auto cmdBuffer = mCommands->get().buffer();

    VkBuffer vkBuffer = vulkanBufferObject->buffer.getGpuBuffer();

    vkCmdBindIndexBuffer(cmdBuffer, vkBuffer, 0, indexSize==2? VK_INDEX_TYPE_UINT16: VK_INDEX_TYPE_UINT32);
}

Handle<HwBufferObject> VulkanRenderSystemBase::createBufferObject(
    BufferObjectBinding bufferObjectBinding,
    ResourceMemoryUsage memoryUsage,
    uint32_t bufferCreationFlags,
    uint32_t byteCount,
    const char* debugName)
{
    Handle<HwBufferObject> boh =  mResourceAllocator.allocHandle<VulkanBufferObject>();

    VulkanBufferObject* bufferObject = mResourceAllocator.construct<VulkanBufferObject>(boh, mAllocator,
        *mStagePool, byteCount, bufferObjectBinding, bufferCreationFlags);

    if (mVulkanSettings->mDebugUtilsExtension)
    {
        if (debugName)
        {
            bufferObject->buffer.setBufferName(mVulkanPlatform->getDevice(), debugName);
        }
    }
    
    
    return boh;
}

void VulkanRenderSystemBase::updateBufferObject(
    Handle<HwBufferObject> boh,
    const char* data,
    uint32_t size)
{
    VulkanCommandBuffer& commands = mCommands->get();
    VulkanBufferObject* bo = mResourceAllocator.handle_cast<VulkanBufferObject*>(boh);
    bo->buffer.loadFromCpu(commands.buffer(), data, 0, size);
}

Handle<HwDescriptorSet> VulkanRenderSystemBase::createDescriptorSet(
    Handle<HwProgram> programHandle,
    uint32_t set)
{
    Handle<HwDescriptorSet> dsh = mResourceAllocator.allocHandle<VulkanDescriptorSet>();
    VulkanShaderProgram* vulkanProgram = mResourceAllocator.handle_cast<VulkanShaderProgram*>(programHandle);
    Handle<HwDescriptorSetLayout> layoutHandle = vulkanProgram->getLayout(set);
    VulkanDescriptorSetLayout* layout = mResourceAllocator.handle_cast<VulkanDescriptorSetLayout*>(layoutHandle);
    VkDescriptorSet vkSet = mDescriptorInfinitePool->obtainSet(layout);
    VulkanDescriptorSet* vulkanDescSet = mResourceAllocator.construct<VulkanDescriptorSet>(dsh, &mResourceAllocator, vkSet, set);
    vulkanDescSet->updateVulkanProgram(vulkanProgram);
    return dsh;
}

Handle<HwDescriptorSet> VulkanRenderSystemBase::createDescriptorSet(
    Handle<HwComputeProgram> programHandle,
    uint32_t set)
{
    VulkanComputeProgram* program = mResourceAllocator.handle_cast<VulkanComputeProgram*>(programHandle);
    auto layoutHandle =  program->getSetLayoutHandle(set);

    Handle<HwDescriptorSet> dsh = mResourceAllocator.allocHandle<VulkanDescriptorSet>();
    VulkanDescriptorSetLayout* layout = mResourceAllocator.handle_cast<VulkanDescriptorSetLayout*>(layoutHandle);
    VkDescriptorSet vkSet = mDescriptorInfinitePool->obtainSet(layout);
    VulkanDescriptorSet* vulkanDescSet = mResourceAllocator.construct<VulkanDescriptorSet>(dsh, &mResourceAllocator, vkSet, set);
    vulkanDescSet->updateVulkanProgram(program);
    return dsh;
}

Handle<HwPipelineLayout> VulkanRenderSystemBase::createPipelineLayout(std::array<Handle<HwDescriptorSetLayout>, 4>& layouts)
{
    Handle<HwPipelineLayout> plo = mResourceAllocator.allocHandle<VulkanPipelineLayout>();
    uint32_t index = 0;

    VulkanPipelineLayoutCache::PipelineLayoutKey key;
    for (auto& layoutHandle : layouts)
    {
        auto layout = mResourceAllocator.handle_cast<VulkanDescriptorSetLayout*>(layoutHandle);
        VkDescriptorSetLayout vkLayout = layout->getVkLayout();
        key.setLayout[index] = vkLayout;
        index++;
    }
    VkPipelineLayout vulkanPipelineLayout = mPipelineLayoutCache->getLayout(key);
    VulkanPipelineLayout* pipeLayout = mResourceAllocator.construct<VulkanPipelineLayout>(plo, vulkanPipelineLayout);
    return plo;
}

Handle<HwProgram> VulkanRenderSystemBase::createShaderProgram(const ShaderInfo& shaderInfo, VertexDeclaration* decl)
{
    Handle<HwProgram> program = mResourceAllocator.allocHandle<VulkanShaderProgram>();
    VulkanShaderProgram* vulkanProgram = mResourceAllocator.construct<VulkanShaderProgram>(program, shaderInfo.shaderName);

    Ogre::ShaderPrivateInfo* privateInfo =
        ShaderManager::getSingleton().getShader(shaderInfo.shaderName, EngineType_Vulkan);

    auto res = ResourceManager::getSingleton().getResource(privateInfo->vertexShaderName);

    vks::tools::BingdingInfo bindingMap;

    auto bingingUpdate = [](
        vks::tools::BingdingInfo& bindingMap,
        vks::tools::BingdingInfo& results,
        VkShaderStageFlagBits flagBits
        )
        {
            auto findLayout = [](
                std::vector<VKDescriptorInfo>& bindingList,
                VKDescriptorInfo binding)
                {
                    for (auto i = 0; i < bindingList.size(); i++)
                    {
                        if (bindingList.at(i).layoutBinding.binding == binding.layoutBinding.binding)
                        {
                            return i;
                        }
                    }
                    return -1;
                };
            for (auto& pair : results)
            {
                auto& bingdingList = bindingMap[pair.first];
                for (auto& layoutBingding : pair.second)
                {
                    auto i = findLayout(bingdingList, layoutBingding);
                    if (i >= 0)
                    {
                        bingdingList[i].layoutBinding.stageFlags |= flagBits;
                    }
                    else
                    {
                        bingdingList.push_back(layoutBingding);
                    }
                }
            }
        };

    String* vertexContent = ShaderManager::getSingleton().getShaderContent(privateInfo->vertexShaderName);
    VkShaderModuleInfo moduleInfo;
    moduleInfo.shaderType = Ogre::VertexShader;
    glslCompileShader(
        res->_fullname,
        *vertexContent,
        privateInfo->vertexShaderEntryPoint,
        shaderInfo.shaderMacros,
        moduleInfo);
    
    vulkanProgram->updateVertexShader(moduleInfo.shaderModule);
    std::vector<VkVertexInputAttributeDescription>& attributeDescriptions =
        vulkanProgram->getAttributeDescriptions();
    this->parseAttributeDescriptions(decl, moduleInfo.inputDesc, attributeDescriptions);
    std::vector<VkVertexInputBindingDescription>& vertexInputBindings =
        vulkanProgram->getVertexInputBindings();
    this->parseInputBindingDescription(decl, moduleInfo.inputDesc, vertexInputBindings);

    std::vector<vks::tools::PushConstants> constantsList;
    auto results = vks::tools::getProgramBindings(
        moduleInfo.spv, 
        VK_SHADER_STAGE_VERTEX_BIT,
        &constantsList);
    for (auto& pair : results)
    {
        for (auto& layoutBingding : pair.second)
        {
            bindingMap[pair.first].push_back(layoutBingding);
        }
    }

    res = ResourceManager::getSingleton().getResource(privateInfo->geometryShaderName);
    if (res)
    {
        String* content = ShaderManager::getSingleton().getShaderContent(privateInfo->geometryShaderName);
        VkShaderModuleInfo moduleInfo;
        moduleInfo.shaderType = Ogre::GeometryShader;
        glslCompileShader(
            res->_fullname,
            *content,
            privateInfo->geometryShaderEntryPoint,
            shaderInfo.shaderMacros,
            moduleInfo);
        vulkanProgram->updateGeometryShader(moduleInfo.shaderModule);

        auto results = vks::tools::getProgramBindings(moduleInfo.spv, VK_SHADER_STAGE_GEOMETRY_BIT);
        bingingUpdate(bindingMap, results, VK_SHADER_STAGE_GEOMETRY_BIT);
    }
    res = ResourceManager::getSingleton().getResource(privateInfo->fragShaderName);

    if (res)
    {
        String* content = ShaderManager::getSingleton().getShaderContent(privateInfo->fragShaderName);
        VkShaderModuleInfo moduleInfo;
        moduleInfo.shaderType = Ogre::PixelShader;

        glslCompileShader(
            res->_fullname,
            *content,
            privateInfo->fragShaderEntryPoint,
            shaderInfo.shaderMacros,
            moduleInfo);
        vulkanProgram->updateFragmentShader(moduleInfo.shaderModule);
        constantsList.clear();
        auto results = vks::tools::getProgramBindings(
            moduleInfo.spv, 
            VK_SHADER_STAGE_FRAGMENT_BIT,
            &constantsList);
        bingingUpdate(bindingMap, results, VK_SHADER_STAGE_FRAGMENT_BIT);
    }
    
    vulkanProgram->updateDescriptorInfo(bindingMap);

    VkDescriptorSetLayoutBinding toBind[VulkanDescriptorSetLayout::MAX_BINDINGS];
    uint32_t bindIndex = 0;
    VulkanPipelineLayoutCache::PipelineLayoutKey keys;
    for (auto set = 0; set < VulkanDescriptorSetLayout::MAX_BINDING_SET; set++)
    {
        bindIndex = 0;
        
        auto itor = bindingMap.find(set);

        if (itor != bindingMap.end())
        {
            for (auto& obj : itor->second)
            {
                toBind[bindIndex] = obj.layoutBinding;
                bindIndex++;
            }
        }
        
        if(bindIndex == 0)
        {
            keys.setLayout[set] = pEmptyDescriptorSetLayout;
        }
        else
        {
            VulkanDescriptorSetLayout::VulkanDescriptorSetLayoutInfo info;

            for (auto i = 0; i < bindIndex; i++)
            {
                switch (toBind[i].descriptorType)
                {
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    info.combinedImage++;
                    break;
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    info.inputAttachmentCount++;
                    break;
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                    info.samplerCount++;
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                    info.storeCount++;
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    info.uboCount++;
                    break;
                default:
                    assert(false);
                    break;
                }
            }
            Handle<HwDescriptorSetLayout> layoutHandle = 
                mResourceAllocator.allocHandle<VulkanDescriptorSetLayout>();
            VulkanDescriptorSetLayout* vulkanLayout = 
                mResourceAllocator.construct<VulkanDescriptorSetLayout>(layoutHandle, info);
            
            
            VkDescriptorSetLayout vkLayout = mVulkanLayoutCache->getLayout(&toBind[0], bindIndex);
            vulkanLayout->setVkLayout(vkLayout);

            keys.setLayout[set] = vkLayout;

            vulkanProgram->updateLayout(set, layoutHandle);
        }
    }

    keys.pushConstant->size = 0;
    keys.pushConstant->stage = 0;
   
    VkPipelineLayout vulkanPipelineLayout = mPipelineLayoutCache->getLayout(keys);
    vulkanProgram->updateVulkanPipelineLayout(vulkanPipelineLayout);
    
    return program;
}

void VulkanRenderSystemBase::updatePushConstants(
    Handle<HwProgram> programHandle,
    uint32_t offset,
    const char* data,
    uint32_t size)
{
    VulkanShaderProgram* program =
        mResourceAllocator.handle_cast<VulkanShaderProgram*>(programHandle);
    auto pipelineLayout = program->getVulkanPipelineLayout();
    vkCmdPushConstants(
        mCommandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
        offset, size, data);
}

Handle<HwSampler> VulkanRenderSystemBase::createTextureSampler(filament::backend::SamplerParams& samplerParams)
{
    Handle<HwSampler> samplerHandle = mResourceAllocator.allocHandle<HwSampler>();

    VulkanTextureSampler* sampler = mResourceAllocator.construct<VulkanTextureSampler>(samplerHandle, samplerParams);

    return samplerHandle;
}

Handle<HwComputeProgram> VulkanRenderSystemBase::createComputeProgram(const ShaderInfo& shaderInfo)
{
    Handle<HwComputeProgram> program = mResourceAllocator.allocHandle<VulkanComputeProgram>();
    VulkanComputeProgram* vulkanProgram = mResourceAllocator.construct<VulkanComputeProgram>(program, shaderInfo.shaderName);


    Ogre::ShaderPrivateInfo* privateInfo =
        ShaderManager::getSingleton().getShader(shaderInfo.shaderName, EngineType_Vulkan);

    auto res = ResourceManager::getSingleton().getResource(privateInfo->computeShaderName);
    
    String* vertexContent = ShaderManager::getSingleton().getShaderContent(privateInfo->computeShaderName);
    VkShaderModuleInfo moduleInfo;
    moduleInfo.shaderType = Ogre::ComputeShader;
    glslCompileShader(
        res->_fullname,
        *vertexContent,
        privateInfo->vertexShaderEntryPoint,
        shaderInfo.shaderMacros,
        moduleInfo);
    vulkanProgram->updateComputeShader(moduleInfo.shaderModule);

    auto results = vks::tools::getProgramBindings(moduleInfo.spv, VK_SHADER_STAGE_COMPUTE_BIT);
    vulkanProgram->updateDescriptorInfo(results);
   
    assert_invariant(!results.empty() && "Need at least one binding for descriptor set layout.");

    std::array<VkDescriptorSetLayout, 4> layoutlist;

    for (auto set = 0; set < 4; set++)
    {
        auto itor = results.find(set);
        if (itor == results.end())
        {
            layoutlist[set] = pEmptyDescriptorSetLayout;
            continue;
        }

        VkDescriptorSetLayoutBinding binding[VulkanDescriptorSetLayout::MAX_BINDINGS];
        uint32_t size = itor->second.size();
        for (uint32_t i = 0; i < size; i++)
        {
            binding[i] = itor->second[i].layoutBinding;
        }
        VkDescriptorSetLayoutCreateInfo dlinfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = (uint32_t)itor->second.size(),
            .pBindings = binding,
        };

        //todo: use layout cache
        VkDescriptorSetLayout vkLayout;
        vkCreateDescriptorSetLayout(mVulkanPlatform->getDevice(), &dlinfo, VKALLOC, &vkLayout);
        layoutlist[set] = vkLayout;
        vulkanProgram->updateSetLayout(vkLayout);
        DescriptorSetLayout layoutInfo;
        Handle<HwDescriptorSetLayout> dslh = mResourceAllocator.allocHandle<VulkanDescriptorSetLayout>();
        VulkanDescriptorSetLayout::VulkanDescriptorSetLayoutInfo info;
        for (auto& binding : itor->second)
        {
            switch (binding.layoutBinding.descriptorType)
            {
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                info.combinedImage++;
                break;
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                info.inputAttachmentCount++;
                break;
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                info.samplerCount++;
                break;
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                info.storeCount++;
                break;
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                info.uboCount++;
                break;
            }
        }
        VulkanDescriptorSetLayout* uboLayout = mResourceAllocator.construct<VulkanDescriptorSetLayout>(dslh, info);
        uboLayout->setVkLayout(vkLayout);
        vulkanProgram->updateSetLayoutHandle(set, dslh);
    }
    
    VkPipelineLayout pipelineLayout;
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
        vks::initializers::pipelineLayoutCreateInfo(
            layoutlist.data(),
            results.size());


    if (vkCreatePipelineLayout(mVulkanPlatform->getDevice(), &pPipelineLayoutCreateInfo,
        nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        assert_invariant(!results.empty() && "vkCreatePipelineLayout failed.");
    }

    vulkanProgram->updatePipelineLayout(pipelineLayout);

    VkPipelineShaderStageCreateInfo stage{};
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.pNext = NULL;
    stage.flags = 0;
    stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage.module = moduleInfo.shaderModule;
    stage.pName = "main";
    stage.pSpecializationInfo = nullptr;

    VkComputePipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.stage = stage;
    create_info.layout = pipelineLayout;
    create_info.basePipelineHandle = 0;
    create_info.basePipelineIndex = 0;
    VkPipeline pipeline;
    vkCreateComputePipelines(mVulkanPlatform->getDevice(), NULL, 1, &create_info,
        nullptr, &pipeline);

    vulkanProgram->updatePipeline(pipeline);
    return program;
}

Handle<HwPipeline> VulkanRenderSystemBase::createPipeline(
    backend::RasterState& rasterState,
    Handle<HwProgram>& program)
{
    Handle<HwPipeline> pipelineHandle = mResourceAllocator.allocHandle<VulkanPipeline>();
    
    VulkanShaderProgram* vulkanProgram = mResourceAllocator.handle_cast<VulkanShaderProgram*>(program);
    VkPipelineLayout pipelineLayout = vulkanProgram->getVulkanPipelineLayout();
    VulkanPipelineCache::RasterState vulkanRasterState;
    vulkanRasterState.cullMode = getCullMode(rasterState.culling);
    vulkanRasterState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vulkanRasterState.depthBiasEnable = VK_FALSE;


    vulkanRasterState.blendEnable = rasterState.hasBlending();
    if (vulkanRasterState.blendEnable)
    {
        int kk = 0;
    }
    vulkanRasterState.depthWriteEnable = rasterState.depthWrite;
    vulkanRasterState.depthTestEnable = rasterState.depthTest;

    vulkanRasterState.srcColorBlendFactor = getBlendFactor(rasterState.blendFunctionSrcRGB);
    vulkanRasterState.dstColorBlendFactor = getBlendFactor(rasterState.blendFunctionDstRGB);
    vulkanRasterState.srcAlphaBlendFactor = getBlendFactor(rasterState.blendFunctionSrcAlpha);
    vulkanRasterState.dstAlphaBlendFactor = getBlendFactor(rasterState.blendFunctionDstAlpha);
    vulkanRasterState.colorBlendOp = rasterState.blendEquationRGB;
    vulkanRasterState.alphaBlendOp = rasterState.blendEquationAlpha;

    vulkanRasterState.colorWriteMask = 0xf;
    vulkanRasterState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    vulkanRasterState.colorTargetCount = rasterState.renderTargetCount;
    vulkanRasterState.depthCompareOp = rasterState.depthFunc;
    vulkanRasterState.depthBiasConstantFactor = 0.0f;
    vulkanRasterState.depthBiasSlopeFactor = 0.0f;

    std::vector<VkVertexInputBindingDescription>& vertexInputBindings =
        vulkanProgram->getVertexInputBindings();
    std::vector<VkVertexInputAttributeDescription>& attributeDescriptions =
        vulkanProgram->getAttributeDescriptions();
    PixelFormat format = (PixelFormat)rasterState.pixelFormat;
    if (format == PF_UNKNOWN)
    {
        format = mRenderWindow->getColorFormat();
    }
    

    mPipelineCache->bindFormat(VulkanMappings::_getPF(format), VK_FORMAT_D32_SFLOAT);
    mPipelineCache->bindProgram(
        vulkanProgram->getVertexShader(), 
        vulkanProgram->getGeometryShader(), 
        vulkanProgram->getFragmentShader());
    mPipelineCache->bindRasterState(vulkanRasterState);
    mPipelineCache->bindPrimitiveTopology(VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    mPipelineCache->bindLayout(pipelineLayout);
    mPipelineCache->bindVertexArray(
        attributeDescriptions.data(),
        attributeDescriptions.size(),
        vertexInputBindings.data(),
        vertexInputBindings.size());

    VkPipeline pipeline = mPipelineCache->getPipeline();

    mPipelineCache->bindProgram(vulkanProgram->getVertexShader(), nullptr, nullptr);

    VkPipeline pipelineShadow = mPipelineCache->getPipeline();
    VulkanPipeline* vulkanPipeline = mResourceAllocator.construct<VulkanPipeline>(
        pipelineHandle, pipeline, pipelineShadow);
    return pipelineHandle;
}

void VulkanRenderSystemBase::updateDescriptorSetBuffer(
    Handle<HwDescriptorSet> dsh,
    backend::descriptor_binding_t binding,
    backend::BufferObjectHandle* boh,
    uint32_t handleCount)
{
    VulkanDescriptorSet* set = mResourceAllocator.handle_cast<VulkanDescriptorSet*>(dsh);

    assert(handleCount < MAX_HANDLE_COUNT);
    VulkanBufferObject* bufferObjects[MAX_HANDLE_COUNT];
    VkDescriptorBufferInfo bufferInfos[MAX_HANDLE_COUNT];
    for (auto i = 0; i < handleCount; i++)
    {
        bufferObjects[i] = mResourceAllocator.handle_cast<VulkanBufferObject*>(boh[i]);

        bufferInfos[i].offset = 0;
        bufferInfos[i].range = VK_WHOLE_SIZE;
        bufferInfos[i].buffer = bufferObjects[i]->buffer.getGpuBuffer();
    }

    VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    if (bufferObjects[0]->bindingType & BufferObjectBinding::BufferObjectBinding_Storge)
    {
        type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
    VkWriteDescriptorSet const descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set->vkSet,
            .dstBinding = binding,
            .descriptorCount = handleCount,
            .descriptorType = type,
            .pBufferInfo = &bufferInfos[0],
    };
    vkUpdateDescriptorSets(mVulkanPlatform->getDevice(), 1, &descriptorWrite, 0, nullptr);
}

void VulkanRenderSystemBase::updateDescriptorSetTexture(
    Handle<HwDescriptorSet> dsh,
    backend::descriptor_binding_t binding,
    OgreTexture** tex,
    uint32_t count,
    TextureBindType bindType)
{
    VulkanDescriptorSet* set = mResourceAllocator.handle_cast<VulkanDescriptorSet*>(dsh);

    
    VkDescriptorImageInfo infos[256];
    VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    if (bindType == TextureBindType_RW_Image)
    {
        layout = VK_IMAGE_LAYOUT_GENERAL;
    }
    for (auto i = 0; i < count; i++)
    {
        VulkanTexture* vulkanTexture = (VulkanTexture*)tex[i];
        infos[i].imageLayout = layout;
        infos[i].imageView = vulkanTexture->getVkImageView();
        if (bindType == TextureBindType_Combined_Image_Sampler)
        {
            infos[i].sampler = vulkanTexture->getSampler();
        }
    }
    
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    if (bindType == TextureBindType_Combined_Image_Sampler)
    {
        type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
    else if (bindType == TextureBindType_RW_Image)
    {
        type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    }
    VkWriteDescriptorSet const descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set->vkSet,
            .dstBinding = binding,
            .descriptorCount = count,
            .descriptorType = type,
            .pImageInfo = &infos[0],
        };

    vkUpdateDescriptorSets(
        mVulkanPlatform->getDevice(),
        1,
        &descriptorWrite, 0, nullptr);
}

void VulkanRenderSystemBase::updateDescriptorSet(
    Handle<HwDescriptorSet> dsh,
    uint32_t count,
    const DescriptorData* pParams
)
{
    VulkanDescriptorSet* set = mResourceAllocator.handle_cast<VulkanDescriptorSet*>(dsh);
    VulkanProgram* vulkanProgram = set->getVulkanProgram();
    VkDescriptorImageInfo imageInfos[MAX_HANDLE_COUNT * 4];
    VkDescriptorImageInfo samplerInfos[MAX_HANDLE_COUNT];
    VkDescriptorBufferInfo bufferInfos[MAX_HANDLE_COUNT];
    uint32_t imageCount = 0;
    uint32_t bufferCount = 0;
    uint32_t samplerCount = 0;
    VkWriteDescriptorSet  descriptorWrite[32];

    for (uint32_t i = 0; i < count; i++)
    {
        const DescriptorData* pParam = pParams + i;
        const VKDescriptorInfo* descriptroInfo = vulkanProgram->getDescriptor(pParam->pName);
        assert(descriptroInfo);
        const uint32_t       arrayCount = std::max(1U, pParam->mCount);

        
        switch (descriptroInfo->layoutBinding.descriptorType)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        {
            VkDescriptorType type = descriptroInfo->layoutBinding.descriptorType;
            VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            if (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
            {
                layout = VK_IMAGE_LAYOUT_GENERAL;
            }
            for (uint32_t arr = 0; arr < arrayCount; ++arr)
            {
                uint32_t index = arr + imageCount;
                VulkanTexture* vulkanTexture = (VulkanTexture*)pParam->ppTextures[arr];
                imageInfos[index].imageLayout = layout;
                imageInfos[index].imageView = vulkanTexture->getVkImageView();
                if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                {
                    imageInfos[index].sampler = vulkanTexture->getSampler();
                }
            }
            descriptorWrite[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[i].pNext = nullptr;
            descriptorWrite[i].dstSet = set->vkSet;
            descriptorWrite[i].dstBinding = descriptroInfo->layoutBinding.binding;
            descriptorWrite[i].descriptorCount = arrayCount;
            descriptorWrite[i].descriptorType = type;
            descriptorWrite[i].dstArrayElement = 0;
            descriptorWrite[i].pImageInfo = &imageInfos[imageCount];
            imageCount += arrayCount;
        }

        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        {
            VkDescriptorType type = descriptroInfo->layoutBinding.descriptorType;

            for (uint32_t arr = 0; arr < arrayCount; ++arr)
            {
                uint32_t index = arr + bufferCount;
                VulkanBufferObject* vbo = mResourceAllocator.handle_cast<VulkanBufferObject*>(pParam->ppBuffers[arr]);
                bufferInfos[index].offset = 0;
                bufferInfos[index].range = VK_WHOLE_SIZE;
                bufferInfos[index].buffer = vbo->buffer.getGpuBuffer();
            }

            descriptorWrite[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[i].pNext = nullptr;
            descriptorWrite[i].dstSet = set->vkSet;
            descriptorWrite[i].dstBinding = descriptroInfo->layoutBinding.binding;
            descriptorWrite[i].descriptorCount = arrayCount;
            descriptorWrite[i].descriptorType = type;
            descriptorWrite[i].dstArrayElement = 0;
            descriptorWrite[i].pBufferInfo = &bufferInfos[bufferCount];
            bufferCount += arrayCount;
        }
        break;
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        {
            VkDescriptorType type = descriptroInfo->layoutBinding.descriptorType;
            for (uint32_t arr = 0; arr < arrayCount; ++arr)
            {
                uint32_t index = arr + samplerCount;

                VkSampler sampler = VK_NULL_HANDLE;

                if (pParam->descriptorType == DESCRIPTOR_TYPE_SAMPLER)
                {
                    VulkanTextureSampler* vulkanSampler =
                        mResourceAllocator.handle_cast<VulkanTextureSampler*>(pParam->ppSamplers[arr]);
                    sampler = vulkanSampler->getSampler();
                }
                else
                {
                    VulkanTexture* vulkanTexture = (VulkanTexture*)pParam->ppTextures[arr];

                    sampler = vulkanTexture->getSampler();
                }
                

                samplerInfos[index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                samplerInfos[index].imageView = VK_NULL_HANDLE;
                samplerInfos[index].sampler = sampler;
            }
            descriptorWrite[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[i].pNext = nullptr;
            descriptorWrite[i].dstSet = set->vkSet;
            descriptorWrite[i].dstBinding = descriptroInfo->layoutBinding.binding;
            descriptorWrite[i].descriptorCount = arrayCount;
            descriptorWrite[i].descriptorType = type;
            descriptorWrite[i].dstArrayElement = 0;
            descriptorWrite[i].pImageInfo = &samplerInfos[samplerCount];
            samplerCount += arrayCount;
        }
        break;
        default:
            assert(false);
         break;
        }
    }

    bluevk::vkUpdateDescriptorSets(
        mVulkanPlatform->getDevice(),
        count,
        descriptorWrite, 0, nullptr);
    
}

void VulkanRenderSystemBase::updateDescriptorSetSampler(
    Handle<HwDescriptorSet> dsh,
    backend::descriptor_binding_t binding,
    Handle<HwSampler> samplerHandle)
{
    VulkanDescriptorSet* set = mResourceAllocator.handle_cast<VulkanDescriptorSet*>(dsh);

    VulkanTextureSampler* vulkanSampler = mResourceAllocator.handle_cast<VulkanTextureSampler*>(samplerHandle);
    VkDescriptorImageInfo info{};

    info.imageView = VK_NULL_HANDLE;
    info.sampler = vulkanSampler->getSampler();
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet const descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set->vkSet,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = &info,
    };

    bluevk::vkUpdateDescriptorSets(
        mVulkanPlatform->getDevice(),
        1,
        &descriptorWrite, 0, nullptr);
}

void VulkanRenderSystemBase::updateDescriptorSetSampler(
    Handle<HwDescriptorSet> dsh,
    backend::descriptor_binding_t binding,
    OgreTexture* tex)
{
    VulkanDescriptorSet* set = mResourceAllocator.handle_cast<VulkanDescriptorSet*>(dsh);
    VulkanTexture* vulkanTexture = (VulkanTexture*)tex;
    VkDescriptorImageInfo info{};

    info.imageView = VK_NULL_HANDLE;
    info.sampler = vulkanTexture->getSampler();
    info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet const descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = set->vkSet,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = &info,
    };

    bluevk::vkUpdateDescriptorSets(
        mVulkanPlatform->getDevice(),
        1,
        &descriptorWrite, 0, nullptr);
}



void VulkanRenderSystemBase::resourceBarrier(
    uint32_t numBufferBarriers,
    BufferBarrier* pBufferBarriers,
    uint32_t numTextureBarriers,
    TextureBarrier* pTextureBarriers,
    uint32_t numRtBarriers,
    RenderTargetBarrier* pRtBarriers
)
{
    vks::tools::resourceBarrier(
        numBufferBarriers, pBufferBarriers,
        numTextureBarriers, pTextureBarriers,
        numRtBarriers, pRtBarriers,
        QUEUE_TYPE_GRAPHICS,
        mVulkanPlatform->getGraphicsQueueFamilyIndex(),
        mCommandBuffer
    );
}



void VulkanRenderSystemBase::beginCmd()
{
    mCommandBuffer = mCommands->get().buffer();
}

void VulkanRenderSystemBase::flushCmd(bool waitCmd)
{
    mCommands->flush(waitCmd);
    mCommandBuffer = nullptr;
}


void VulkanRenderSystemBase::destroyBufferObject(Handle<HwBufferObject> bufHandle)
{
    VulkanBufferObject* bo = mResourceAllocator.handle_cast<VulkanBufferObject*>(bufHandle);

    mResourceAllocator.destruct<VulkanBufferObject>(bufHandle);
}

void VulkanRenderSystemBase::parseInputBindingDescription(
    VertexDeclaration* decl,
    std::vector<GlslInputDesc>& inputDesc,
    std::vector<VkVertexInputBindingDescription>& vertexInputBindings)
{
    vertexInputBindings.clear();
    if (decl)
    {
        for (uint32_t binding = 0; binding < 10; binding++)
        {
            int32_t stride = decl->getVertexSize(binding);

            if (stride <= 0)
                continue;
            vertexInputBindings.emplace_back();
            vertexInputBindings.back().binding = binding;
            vertexInputBindings.back().stride = stride;
            vertexInputBindings.back().inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }
    }
    else
    {
        int32_t stride = 0;
        for (auto i = 0; i < inputDesc.size(); i++)
        {
            auto& input = inputDesc[i];

            stride += getTypeSize(input._type);
        }

       
        vertexInputBindings.emplace_back();
        vertexInputBindings.back().binding = 0;
        vertexInputBindings.back().stride = stride;
        vertexInputBindings.back().inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
    }
}

void VulkanRenderSystemBase::parseAttributeDescriptions(
    VertexDeclaration* decl,
    std::vector<GlslInputDesc>& inputDesc,
    std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
{

    attributeDescriptions.resize(inputDesc.size());
    if (decl == nullptr)
    {
        uint32_t offset = 0;
        for (auto i = 0; i < inputDesc.size(); i++)
        {
            auto& input = inputDesc[i];
            attributeDescriptions[i].binding = 0;
            attributeDescriptions[i].location = input._location;
            attributeDescriptions[i].format = getVKFormatFromType(input._type);
            attributeDescriptions[i].offset = offset;
            offset += getTypeSize(input._type);
        }
        return;
    }
    
    int32_t i = 0;

    for (GlslInputDesc& input : inputDesc)
    {
        bool find = false;

        auto& elementlist = decl->getElementList();
        for (auto elem : elementlist)
        {
            if (input._name == VulkanMappings::getSemanticName(elem.getSemantic()) &&
                input._index == elem.getIndex())
            {
                attributeDescriptions[i].binding = elem.getSource();
                attributeDescriptions[i].location = input._location;
                attributeDescriptions[i].format = getVKFormatFromType(elem.getType());
                attributeDescriptions[i].offset = elem.getOffset();
                i++;
                find = true;
                break;
            }
        }


        if (!find)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "can not find input param");
        }
    }
}