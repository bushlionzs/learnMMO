#pragma once


//#include "engine_struct.h"
#include <utils/RangeMap.h>
#include "VulkanCommon.h"
#include "OgreTexture.h"
#include "VulkanCommands.h"
#include "VulkanImageUtility.h"
#include "VulkanPlatform.h"

namespace filament::backend {
    struct VulkanContext;
    class VulkanCommands;
    class VulkanStagePool;
}
class VulkanRenderSystemBase;


class VulkanTexture :public Ogre::OgreTexture, public HwTexture
{
public:
    VulkanTexture(
        const std::string& name,
        VulkanPlatform* platform,
        VulkanCommands* commands,
        Ogre::TextureProperty* texProperty
        );

    VulkanTexture(
        const std::string& name,
        VulkanPlatform* platform,
        VulkanCommands* commands,
        VkImage image,
        Ogre::TextureProperty* texProperty
    );

    void updateTextureName(const char* name, uint32_t size)
    {
        mName.assign(name, size);
    }

    ~VulkanTexture();

    VkImageView getVkImageView()
    {
        return mTextureImageView;
    }

    VkImage getVkImage()
    {
        return mTextureImage;
    }

    VkFormat getVkFormat()
    {
        return mVulkanFormat;
    }

    VkSampler getSampler()
    {
        return mTextureSampler;
    }

    VkImageAspectFlags getAspectFlag()
    {
        return mAspectFlags;
    }

    uint32_t getMipLevels()
    {
        return mMipLevels;
    }

    void setSidecar(VulkanTexture* sidecar) {
        mSidecarMSAA.reset(sidecar);
    }

    VulkanTexture* getSidecar() const {
        return mSidecarMSAA.get();
    }
    VulkanLayout getLayout(uint32_t layer, uint32_t level) const;
    void setLayout(const VkImageSubresourceRange& range, VulkanLayout newLayout);
    bool transitionReady();
    void transitionLayout(
        VkCommandBuffer cmdbuf,
        const VkImageSubresourceRange& range, 
        VulkanLayout newLayout);

    

    void updateTextureData();
    void* getVulkanBuffer(uint32_t offset);


    int32_t getSlot()
    {
        return mSlot;
    }

    void setSlot(int32_t slot)
    {
        mSlot = slot;
    }
private:
    bool need_midmap();
    void _createSurfaceList(void);
    virtual void createInternalResourcesImpl(void);
    virtual void freeInternalResourcesImpl(void);
    virtual void postLoad();
private:
    void createImage(
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory);
    VkImageView createImageView(VkImage image, VkFormat format);
    void createTextureSampler();

    
private:
    VulkanPlatform* mPlatform;
    VkBuffer mStagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory mStagingBufferMemory = VK_NULL_HANDLE;
    char* mMappedMemory;

    VkImage mTextureImage = VK_NULL_HANDLE;
    VkDeviceMemory mTextureImageMemory;

    VkImageView mTextureImageView;
    VkSampler mTextureSampler;

    VkFormat mVulkanFormat = VK_FORMAT_UNDEFINED;
    bool mNeedMipmaps = false;
    uint32_t mMipLevels = 1;

    // The texture with the sidecar owns the sidecar.
    std::unique_ptr<VulkanTexture> mSidecarMSAA;
    // Track the image layout of each subresource using a sparse range map.
    utils::RangeMap<uint32_t, filament::backend::VulkanLayout> mSubresourceLayouts;

    VkImageAspectFlags mAspectFlags;

    VmaAllocator mAllocator;
    VulkanCommands* mCommands;
    VulkanStagePool* mStagePool = nullptr;

    int32_t mSlot = -1;
};