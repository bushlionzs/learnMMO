#include "OgreHeader.h"
#include "OgreTextureManager.h"
#include "OgreResourceManager.h"
#include "glslUtil.h"
#include "VulkanHelper.h"
#include "VulkanRenderSystem.h"
#include "VulkanTools.h"
#include "VulkanHardwareBufferManager.h"
#include "FVulkanBuffer.h"
#include "VulkanMappings.h"
#include "VulkanLayoutCache.h"
#include "VulkanPipelineCache.h"
#include "VulkanPipelineLayoutCache.h"
#include "shaderManager.h"


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
    VmaAllocatorCreateInfo const allocatorInfo{
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &funcs,
        .instance = instance,
    };
    vmaCreateAllocator(&allocatorInfo, &allocator);
    return allocator;
}


template<> VulkanHelper* Ogre::Singleton<VulkanHelper>::msSingleton = 0;

static const std::vector<const char*> validationLayers = 
{
    "VK_LAYER_KHRONOS_validation"
};



static std::vector<const char*> getRequiredExtensions()
{
    std::vector<const char*> instanceExtensions = {
        "VK_KHR_surface", 
        "VK_KHR_win32_surface",
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

    return instanceExtensions;
}

VulkanHelper::VulkanHelper(VulkanRenderSystemBase* rs)
    :mResourceAllocator(8388608, false)
{
    mVulkanRenderSystem = rs;

 
    new VulkanHardwareBufferManager();
}

VulkanHelper::~VulkanHelper()
{

}

static  std::vector<const char*> deviceExtensions =
{
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME
};

void VulkanHelper::_initialise(VulkanPlatform* platform)
{
    mPlatform = platform;

    if (mSettings.mRayPipelineSupported)
    {
        deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        deviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        deviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    }
   

    Platform::DriverConfig config;

    config.enableRayTracing = mSettings.mRayPipelineSupported;

    mPlatform->createDriver(nullptr, config);

    mVKInstance = mPlatform->getInstance();
    mPhysicalDevice = mPlatform->getPhysicalDevice();
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &mPhysicalDeviceProperties);
    vkGetPhysicalDeviceFeatures(mPhysicalDevice, &mDeviceFeatures);
    vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mPhysicalMemoryProperties);
    mVKDevice = mPlatform->getDevice();

    
    createCommandPool();
    createVulkanResourceCache();
}

void VulkanHelper::_createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(mVKDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(mVKDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        _findMemoryType(
            memRequirements.memoryTypeBits,
            properties);
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        allocInfo.pNext = &allocFlagsInfo;
    }
    if (vkAllocateMemory(mVKDevice, &allocInfo, 
        nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(mVKDevice, buffer, bufferMemory, 0);
}

VkResult VulkanHelper::createBuffer(
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags memoryPropertyFlags,
    FVulkanBuffer* buffer,
    VkDeviceSize size,
    void* data)
{
    buffer->device = mVKDevice;

    // Create the buffer handle
    VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
    VK_CHECK_RESULT(vkCreateBuffer(mVKDevice, &bufferCreateInfo, nullptr, &buffer->buffer));

    // Create the memory backing up the buffer handle
    VkMemoryRequirements memReqs;
    VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
    vkGetBufferMemoryRequirements(mVKDevice, buffer->buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    // Find a memory type index that fits the properties of the buffer
    memAlloc.memoryTypeIndex = _findMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
    // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
    VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
    if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
        allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
        memAlloc.pNext = &allocFlagsInfo;
    }
    VK_CHECK_RESULT(vkAllocateMemory(mVKDevice, &memAlloc, nullptr, &buffer->memory));

    buffer->alignment = memReqs.alignment;
    buffer->size = size;
    buffer->usageFlags = usageFlags;
    buffer->memoryPropertyFlags = memoryPropertyFlags;

    // If a pointer to the buffer data has been passed, map the buffer and copy over the data
    if (data != nullptr)
    {
        VK_CHECK_RESULT(buffer->map());
        memcpy(buffer->mapped, data, size);
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
            buffer->flush();

        buffer->unmap();
    }

    // Initialize a default descriptor that covers the whole buffer size
    buffer->setupDescriptor();

    // Attach the memory to the buffer object
    return buffer->bind();
}

VkPhysicalDeviceProperties& VulkanHelper::_getVkPhysicalDeviceProperties()
{
    return mPhysicalDeviceProperties;
}

void VulkanHelper::loadDefaultResources()
{
    mDefaultTexture = TextureManager::getSingleton().load("white1x1.dds", nullptr);
    mDefaultTexture->load(nullptr);
}

std::shared_ptr<OgreTexture>& VulkanHelper::getDefaultTexture()
{
    return mDefaultTexture;
}

VkCommandBuffer VulkanHelper::beginTransferCommand()
{
    if (mTransfercommandBuffer == VK_NULL_HANDLE)
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = mSingleCommandPool;

        // 
        allocInfo.commandBufferCount = 1;

        vkAllocateCommandBuffers(mVKDevice, &allocInfo, &mTransfercommandBuffer);
    }
    


    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    //beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(mTransfercommandBuffer, &beginInfo);
    return mTransfercommandBuffer;
}

void VulkanHelper::endTransferCommand(VkCommandBuffer commandBuffer)
{
    assert_invariant(commandBuffer == mTransfercommandBuffer);
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    auto queue = mPlatform->getTransferQueue();

    auto result = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

    if (result != VK_SUCCESS)
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "failed to vkQueueSubmit!");
    }
    vkQueueWaitIdle(queue);
}

uint32_t VulkanHelper::getTransferFamilyIndex()
{
    return mPlatform->getTransferQueueFamilyIndex();
}


void VulkanHelper::createCommandPool()
{
    VkCommandPoolCreateInfo cmdPoolInfo = {};


    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = mPlatform->getTransferQueueIndex();
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(mVKDevice, &cmdPoolInfo, nullptr, &mSingleCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanHelper::createVulkanResourceCache()
{
    mAllocator = createAllocator(mVKInstance, mPhysicalDevice, mVKDevice);

    mLayoutCache = new VulkanLayoutCache(mVKDevice, &mResourceAllocator);

    mPipelineCache = new VulkanPipelineCache(mVKDevice, mAllocator);

    mPipelineLayoutCache = new VulkanPipelineLayoutCache(mVKDevice, &mResourceAllocator);
}


VkSampler VulkanHelper::getSampler(const filament::backend::SamplerParams& params)
{
    auto iter = mSamplersCache.find(params);
    if (UTILS_LIKELY(iter != mSamplersCache.end())) {
        //return iter->second;
    }
    VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VulkanMappings::getFilter(params.filterMag),
            .minFilter = VulkanMappings::getFilter(params.filterMin),
            .mipmapMode = VulkanMappings::getMipmapMode(params.mipMapMode),
            .addressModeU = VulkanMappings::getWrapMode(params.wrapS),
            .addressModeV = VulkanMappings::getWrapMode(params.wrapT),
            .addressModeW = VulkanMappings::getWrapMode(params.wrapR),
            .anisotropyEnable = params.anisotropyLog2 == 0 ? VK_FALSE : VK_TRUE,
            .maxAnisotropy = params.anisotropyLog2 == 0?0.0f:(float)(1u << params.anisotropyLog2),
            .compareEnable = VulkanMappings::getCompareEnable(params.compareMode),
            .compareOp = VulkanMappings::getCompareOp(params.compareFunc),
            .minLod = 0.0f,
            .maxLod = VulkanMappings::getMaxLod(params.mipMapMode),
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
    };

    VkSampler sampler;
    VkResult error = vkCreateSampler(mVKDevice, &samplerInfo, VKALLOC, &sampler);
    mSamplersCache.insert({ params, sampler });
    return sampler;
}

int32_t VulkanHelper::_findMemoryType(
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < mPhysicalMemoryProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (mPhysicalMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    return -1;
}


