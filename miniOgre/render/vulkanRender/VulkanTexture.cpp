#include "OgreHeader.h"
#include "VulkanTexture.h"
#include "OgreImage.h"
#include "VulkanRenderSystem.h"
#include "OgreResourceManager.h"
#include "myutils.h"
#include "VulkanHelper.h"
#include "VulkanHardwarePixelBuffer.h"
#include "VulkanMappings.h"
#include "VulkanTools.h"



VulkanTexture::VulkanTexture(
    const std::string& name,
    VulkanPlatform* platform,
    VulkanCommands* commands,
    Ogre::TextureProperty* texProperty):
    OgreTexture(name, texProperty)
{
    mLoad = false;

    mName = name;
    mCommands = commands;
    mPlatform = platform;
}

VulkanTexture::VulkanTexture(
    const std::string& name,
    VulkanPlatform* platform,
    VulkanCommands* commands,
    VkImage image,
    Ogre::TextureProperty* texProperty
):OgreTexture(name, texProperty)
{
    mName = name;
    mCommands = commands;
    mPlatform = platform;
    mTextureImage = image;
    createInternalResourcesImpl();
}

VulkanTexture::~VulkanTexture()
{
    
}

bool VulkanTexture::need_midmap()
{
    if (!mTextureProperty._need_mipmap)
        return false;
    if (mTextureProperty._tex_usage & Ogre::TextureUsage::COLOR_ATTACHMENT)
    {
        return false;
    }
    if (mTextureProperty._tex_usage & Ogre::TextureUsage::DEPTH_ATTACHMENT)
    {
        return false;
    }
    return true;
}

void VulkanTexture::_createSurfaceList(void)
{
    if (mTextureProperty._tex_usage & Ogre::TextureUsage::COLOR_ATTACHMENT)
    {
        return;
    }
    if (mTextureProperty._tex_usage & Ogre::TextureUsage::DEPTH_ATTACHMENT)
    {
        return;
    }
    
    // Create new list of surfaces
    mSurfaceList.clear();
    size_t depth = mTextureProperty._depth;

    uint64_t bufferSizeAll = 0;
    for (size_t face = 0; face < mFace; ++face)
    {
        size_t width = mTextureProperty._width;
        size_t height = mTextureProperty._height;
        for (size_t mip = 0; mip <= mTextureProperty._numMipmaps; ++mip)
        {

            VulkanHardwarePixelBuffer* buffer;
            buffer = new VulkanHardwarePixelBuffer(
                this, // parentTexture
                mip,
                width,
                height,
                depth,
                face,
                bufferSizeAll,
                mFormat,
                (HardwareBuffer::Usage)mUsage
            );

            bufferSizeAll += buffer->getSizeInBytes();

            mSurfaceList.push_back(HardwarePixelBufferPtr(buffer));

            if (width > 1) width /= 2;
            if (height > 1) height /= 2;
        }
    }

    VulkanHelper::getSingleton()._createBuffer(
        bufferSizeAll,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mStagingBuffer,
        mStagingBufferMemory);

    VkResult result = vkMapMemory(mPlatform->getDevice(), mStagingBufferMemory, 0, bufferSizeAll, 0, (void**)&mMappedMemory);

    if (result != VK_SUCCESS)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "vkMapMemory error");
    }
}

void VulkanTexture::createInternalResourcesImpl(void)
{
    if (VK_FORMAT_UNDEFINED == mVulkanFormat)
    {
        mFormat = VulkanMappings::_getClosestSupportedPF(mFormat);

        mVulkanFormat = VulkanMappings::_getGammaFormat(VulkanMappings::_getPF(mFormat), false);
    }
    

    if (mTextureImage == VK_NULL_HANDLE)
    {
        createImage(
            mTextureProperty._width,
            mTextureProperty._height,
            mVulkanFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mTextureImage,
            mTextureImageMemory);
    }
    else
    {
        int kk = 0;
    }
    

    mTextureImageView = createImageView(mTextureImage, mVulkanFormat);
    createTextureSampler();

    _createSurfaceList();

    if (mTextureProperty._tex_usage & WRITEABLE)
    {
        VkCommandBuffer cmdBuf = VulkanHelper::getSingleton().beginTransferCommand();
        TextureBarrier uavBarriers[] = {
                {
                this,
                RESOURCE_STATE_UNDEFINED,
                RESOURCE_STATE_UNORDERED_ACCESS},
        };

        auto queueFamilyIndex = VulkanHelper::getSingleton().getTransferFamilyIndex();
        vks::tools::resourceBarrier(0, nullptr, 1, uavBarriers, 0, nullptr, QUEUE_TYPE_TRANSFER, 0, cmdBuf);
        VulkanHelper::getSingleton().endTransferCommand(cmdBuf);
    }
}


void VulkanTexture::freeInternalResourcesImpl(void)
{
    auto device = mPlatform->getDevice();
    if (mTextureImageView)
    {
        vkDestroyImageView(device, mTextureImageView, nullptr);
        mTextureImageView = VK_NULL_HANDLE;
    }

    if (mTextureImage)
    {
        vkDestroyImage(device, mTextureImage, nullptr);
        mTextureImage = VK_NULL_HANDLE;
    }

    if (mTextureImageMemory)
    {
        vkFreeMemory(device, mTextureImageMemory, nullptr);
    }
}

void VulkanTexture::postLoad()
{
    auto frameIndex = 0;

    VkCommandBuffer commandBuffer = VulkanHelper::getSingleton().beginTransferCommand();
    if (!mSurfaceList.empty())
    {
        vks::tools::copyBufferToImage(
            commandBuffer,
            mStagingBuffer,
            mTextureImage,
            this
        );
    }

    if (mNeedMipmaps)
    {
        //generate mipmap
        vks::tools::generateMipmaps(commandBuffer, this);
    }

    VulkanHelper::getSingleton().endTransferCommand(commandBuffer);
}

void VulkanTexture::createImage(
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& imageMemory)
{
    mMipLevels = mTextureProperty._numMipmaps + 1;
    if (need_midmap())
    {
        if (mTextureProperty._numMipmaps == 0)
        {
            auto current = static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1.0);

            if (current > mMipLevels)
            {
                mNeedMipmaps = true;
                mMipLevels = current;
            }
        }
    }
    
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    
    if (mTextureProperty._texType == TEX_TYPE_3D)
    {
        imageInfo.imageType = VK_IMAGE_TYPE_3D;
    }
    imageInfo.extent = { width, height, 1 };

    imageInfo.mipLevels = mMipLevels;
    imageInfo.arrayLayers = mFace;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    
    
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


    if (mUsage & Ogre::TextureUsage::COLOR_ATTACHMENT)
    {
        imageInfo.usage |=  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ;
    }

    if (mUsage & Ogre::TextureUsage::DEPTH_ATTACHMENT)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    
    if (mUsage & Ogre::TextureUsage::WRITEABLE)
    {
        imageInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }


    if (isCubeTexture())
    {
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    auto device = mPlatform->getDevice();
    VK_CHECK_RESULT(vkCreateImage(device, &imageInfo, nullptr, &image));

    VkDebugUtilsObjectNameInfoEXT nameInfo = {};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
    nameInfo.objectHandle = (uint64_t)image;
    nameInfo.pObjectName = mName.c_str();
    vkSetDebugUtilsObjectNameEXT(device, &nameInfo);

    VkMemoryRequirements memRequirements;
    bluevk::vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanHelper::getSingleton()._findMemoryType(memRequirements.memoryTypeBits, properties);

    VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory));

    VK_CHECK_RESULT(vkBindImageMemory(device, image, imageMemory, 0));

}

VkImageView VulkanTexture::createImageView(VkImage image, VkFormat format) 
{
    auto device = mPlatform->getDevice();
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    if (isCubeTexture())
    {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }
    else
    {
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        if (mTextureProperty._texType == TEX_TYPE_3D)
        {
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
        }
    }
    
    viewInfo.format = format;

    mAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
    if (this->mTextureProperty._tex_usage & Ogre::TextureUsage::DEPTH_ATTACHMENT)
    {
        mAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    viewInfo.subresourceRange = { mAspectFlags, 0, mMipLevels, 0, mFace };

    viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "failed to create texture image view!");
    }

    return imageView;
}

void VulkanTexture::createTextureSampler()
{
    mTextureSampler = VulkanHelper::getSingleton().getSampler(mTextureProperty._samplerParams);
}


void* VulkanTexture::getVulkanBuffer(uint32_t offset)
{
    return mMappedMemory + offset;
}

VulkanLayout VulkanTexture::getLayout(uint32_t layer, uint32_t level) const
{
    assert_invariant(level <= 0xffff && layer <= 0xffff);
    const uint32_t key = (layer << 16) | level;
    if (!mSubresourceLayouts.has(key)) {
        return VulkanLayout::UNDEFINED;
    }
    return mSubresourceLayouts.get(key);
}

void VulkanTexture::setLayout(const VkImageSubresourceRange& range, VulkanLayout newLayout)
{
    uint32_t const firstLayer = range.baseArrayLayer;
    uint32_t const lastLayer = firstLayer + range.layerCount;
    uint32_t const firstLevel = range.baseMipLevel;
    uint32_t const lastLevel = firstLevel + range.levelCount;

    assert_invariant(firstLevel <= 0xffff && lastLevel <= 0xffff);
    assert_invariant(firstLayer <= 0xffff && lastLayer <= 0xffff);

    if (newLayout == VulkanLayout::UNDEFINED) {
        for (uint32_t layer = firstLayer; layer < lastLayer; ++layer) {
            uint32_t const first = (layer << 16) | firstLevel;
            uint32_t const last = (layer << 16) | lastLevel;
            mSubresourceLayouts.clear(first, last);
        }
    }
    else {
        for (uint32_t layer = firstLayer; layer < lastLayer; ++layer) {
            uint32_t const first = (layer << 16) | firstLevel;
            uint32_t const last = (layer << 16) | lastLevel;
            mSubresourceLayouts.add(first, last, newLayout);
        }
    }
}

bool VulkanTexture::transitionReady()
{
    return true;
}

void VulkanTexture::transitionLayout(
    VkCommandBuffer cmdbuf,
    const VkImageSubresourceRange& range,
    VulkanLayout newLayout)
{
    VulkanLayout const oldLayout = getLayout(range.baseArrayLayer, range.baseMipLevel);

    uint32_t const firstLayer = range.baseArrayLayer;
    uint32_t const lastLayer = firstLayer + range.layerCount;
    uint32_t const firstLevel = range.baseMipLevel;
    uint32_t const lastLevel = firstLevel + range.levelCount;

    // If we are transitioning more than one layer/level (slice), we need to know whether they are
    // all of the same layer.  If not, we need to transition slice-by-slice. Otherwise it would
    // trigger the validation layer saying that the `oldLayout` provided is incorrect.
    // TODO: transition by multiple slices with more sophiscated range finding.
    bool transitionSliceBySlice = false;
    for (uint32_t i = firstLayer; i < lastLayer; ++i) {
        for (uint32_t j = firstLevel; j < lastLevel; ++j) {
            if (oldLayout != getLayout(i, j)) {
                transitionSliceBySlice = true;
                break;
            }
        }
    }

    if (transitionSliceBySlice) {
        for (uint32_t i = firstLayer; i < lastLayer; ++i) {
            for (uint32_t j = firstLevel; j < lastLevel; ++j) {
                VulkanLayout const layout = getLayout(i, j);
                imgutil::transitionLayout(cmdbuf, {
                        .image = mTextureImage,
                        .oldLayout = layout,
                        .newLayout = newLayout,
                        .subresources = {
                            .aspectMask = range.aspectMask,
                            .baseMipLevel = j,
                            .levelCount = 1,
                            .baseArrayLayer = i,
                            .layerCount = 1,
                        },
                    });
            }
        }
    }
    else {
        imgutil::transitionLayout(cmdbuf, {
            .image = mTextureImage,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .subresources = range,
            });
    }

    setLayout(range, newLayout);
}

void VulkanTexture::updateTextureData()
{
    VkCommandBuffer commandBuffer = mCommands->get().buffer();
    vks::tools::copyBufferToImage(
        commandBuffer,
        mStagingBuffer,
        mTextureImage,
        this
    );
}
