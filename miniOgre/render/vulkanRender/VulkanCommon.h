#pragma once
//#include <vulkan/vulkan.h>
//#include <vulkan/vulkan_beta.h>
//#include <vulkan/vulkan_win32.h>
#include <optional>
#include "OgreHardwareVertexBuffer.h"
#include "engine_struct.h"
#include <bluevk/BlueVK.h>
#include "VulkanInitializers.hpp"
#include <OgreCommon.h>
#include <DriverEnums.h>
#include <VulkanUtility.h>
#include <VulkanBuffer.h>
#include <vk_mem_alloc.h>
#include <tsl/robin_map.h>
#include <utils/FixedCapacityVector.h>
#include <SPIRV_Cross/spirv_glsl.hpp>

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaPool)
using namespace bluevk;
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


struct VulkanDepthStencil
{
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
};

struct VulkanImage
{
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format;
};

struct GeometryNode {
    VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress;
    VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress;
    VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress;
    int32_t textureIndex[3];
};

struct MultisampleTarget 
{
    struct {
        VkImage image;
        VkImageView view;
        VkDeviceMemory memory;
    } color;
    struct {
        VkImage image;
        VkImageView view;
        VkDeviceMemory memory;
    } depth;
};

struct SwapChainBuffer
{
    VkImage _image;
    VkImageView _view;
};

struct VulkanSettings {
    bool validation = false;
    bool fullscreen = false;
    bool vsync = false;
    bool multiSampling = true;
    bool mGeometryShaderSupported = true;
    bool mTessellationSupported = true;
    bool mRayPipelineSupported = false;
    bool mDebugUtilsExtension = true;
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_4_BIT;
    VulkanSettings()
    {
        multiSampling = false;

        if (!multiSampling)
        {
            sampleCount = VK_SAMPLE_COUNT_1_BIT;
        }
    }
};

struct GlslInputDesc
{
    std::string _name;
    uint32_t _index;
    uint32_t _location;
    uint32_t _size;
    spirv_cross::SPIRType _type;
    uint32_t _offset;

};

struct VkShaderModuleInfo
{
    Ogre::ShaderType shaderType;
    VkShaderModule shaderModule;
    uint64_t uboMask;
    uint64_t samplerMask;
    std::string spv;
    std::vector<GlslInputDesc> inputDesc;
};

struct VKDescriptorInfo
{
    char name[64];
    VkDescriptorSetLayoutBinding layoutBinding;
};

// Holds data for a ray tracing scratch buffer that is used as a temporary storage
struct RayTracingScratchBuffer
{
    uint64_t deviceAddress = 0;
    VkBuffer handle = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};



#define VULKAN_FRAME_RESOURCE_COUNT 3
#define VULKAN_TEXTURE_COUNT 6
#define VULKAN_COMMAND_THREAD 4

using namespace filament::backend;

VkFormat getVKFormatFromType(VertexElementType type);

VkFormat getVKFormatFromType(spirv_cross::SPIRType type);

int32_t getTypeSize(spirv_cross::SPIRType type);