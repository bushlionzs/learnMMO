#pragma once
#include "VulkanCommon.h"
#include "VulkanConstants.h"
#include "VulkanHandles.h"

class DescriptorPool {
public:
    DescriptorPool(VkDevice device, VulkanDescriptorSetLayout* layout, uint16_t capacity)
        : mDevice(device),
        mLayout(layout),
        mCapacity(capacity),
        mSize(0),
        mUnusedCount(0) {
        VkDescriptorPoolSize sizes[4];
        uint8_t npools = 0;
        if (layout->hasUbo()) {
            sizes[npools++] = {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = layout->getUboCount(),
            };
        }
        if (layout->hasDynamicUbo()) {
            sizes[npools++] = {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                .descriptorCount = layout->getDynamicUboCount(),
            };
        }
        if (layout->hasCombinedImage()) {
            sizes[npools++] = {
              .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
              .descriptorCount = layout->getCombinedImageCount(),
            };
        }
        if (layout->hasInputAttachment()) {
            sizes[npools++] = {
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = layout->getInputAttachmentCount(),
            };
        }

        if (layout->hasStoreImage())
        {
            sizes[npools++] = {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = layout->getStoreImageCount(),
            };
        }

        if (layout->hasAccelerationStructure())
        {
            sizes[npools++] = {
                .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
                .descriptorCount = layout->getAccelerationStructureCount(),
            };
        }
        VkDescriptorPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = capacity,
            .poolSizeCount = npools,
            .pPoolSizes = sizes,
        };
        vkCreateDescriptorPool(mDevice, &info, VKALLOC, &mPool);
    }

    DescriptorPool(DescriptorPool const&) = delete;
    DescriptorPool& operator=(DescriptorPool const&) = delete;

    ~DescriptorPool() {
        vkDestroyDescriptorPool(mDevice, mPool, VKALLOC);
    }

    uint16_t const& capacity() {
        return mCapacity;
    }

    // A convenience method for checking if this pool can allocate sets for a given layout.
    inline bool canAllocate(VulkanDescriptorSetLayout* layout) {
        return mLayout == layout;
    }

    VkDescriptorSet obtainSet(VkDescriptorSetLayout vklayout) {
        auto itr = findSets(vklayout);
        if (itr != mUnused.end()) {
            // If we don't have any unused, then just return an empty handle.
            if (itr->second.empty()) {
                return VK_NULL_HANDLE;
            }
            std::vector<VkDescriptorSet>& sets = itr->second;
            auto set = sets.back();
            sets.pop_back();
            mUnusedCount--;
            return set;
        }
        if (mSize + 1 > mCapacity) {
            return VK_NULL_HANDLE;
        }
        // Creating a new set
        VkDescriptorSetLayout layouts[1] = { vklayout };
        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = mPool,
            .descriptorSetCount = 1,
            .pSetLayouts = layouts,
        };
        VkDescriptorSet vkSet;
        UTILS_UNUSED VkResult result = vkAllocateDescriptorSets(mDevice, &allocInfo, &vkSet);
        FILAMENT_CHECK_POSTCONDITION(result == VK_SUCCESS)
            << "Failed to allocate descriptor set code=" << result << " size=" << mSize
            << " capacity=" << mCapacity << " count=" << mUnusedCount;
        mSize++;
        return vkSet;
    }

    void recycle(VkDescriptorSetLayout vklayout, VkDescriptorSet vkSet) {
        // We are recycling - release the set back into the pool. Note that the
        // vk handle has not changed, but we need to change the backend handle to allow
        // for proper refcounting of resources referenced in this set.
        auto itr = findSets(vklayout);
        if (itr != mUnused.end()) {
            itr->second.push_back(vkSet);
        }
        else {
            mUnused.push_back(std::make_pair(vklayout, std::vector<VkDescriptorSet> {vkSet}));
        }
        mUnusedCount++;
    }

private:
    using UnusedSets = std::pair<VkDescriptorSetLayout, std::vector<VkDescriptorSet>>;
    using UnusedSetMap = std::vector<UnusedSets>;

    inline UnusedSetMap::iterator findSets(VkDescriptorSetLayout vklayout) {
        return std::find_if(mUnused.begin(), mUnused.end(), [vklayout](auto const& value) {
            return value.first == vklayout;
            });
    }

    VkDevice mDevice;
    VkDescriptorPool mPool;
    VulkanDescriptorSetLayout* mLayout;
    uint16_t const mCapacity;

    // Tracks the number of allocated descriptor sets.
    uint16_t mSize;
    // Tracks  the number of in-use descriptor sets.
    uint16_t mUnusedCount;

    // This maps a layout to a list of descriptor sets allocated for that layout.
    UnusedSetMap mUnused;
};

// This is an ever-expanding pool of sets where it
//    1. Keeps a list of smaller pools of different layout-dimensions.
//    2. Will add a pool if existing pool are not compatible with the requested layout o runs out.
class DescriptorInfinitePool {
private:
    static constexpr uint16_t EXPECTED_SET_COUNT = 10;
    static constexpr float SET_COUNT_GROWTH_FACTOR = 1.5;

public:
    DescriptorInfinitePool(VkDevice device)
        : mDevice(device) {}

    VkDescriptorSet obtainSet(VulkanDescriptorSetLayout* layout) {
        auto const vklayout = layout->getVkLayout();
        DescriptorPool* sameTypePool = nullptr;
        for (auto& pool : mPools) {
            if (!pool->canAllocate(layout)) {
                continue;
            }
            if (auto set = pool->obtainSet(vklayout); set != VK_NULL_HANDLE) {
                return set;
            }
            if (!sameTypePool || sameTypePool->capacity() < pool->capacity()) {
                sameTypePool = pool.get();
            }
        }

        uint16_t capacity = EXPECTED_SET_COUNT;
        if (sameTypePool) {
            // Exponentially increase the size of the pool  to ensure we don't hit this too often.
            capacity = std::ceil(sameTypePool->capacity() * SET_COUNT_GROWTH_FACTOR);
        }

        // We need to increase the set of pools by one.
        mPools.push_back(std::make_unique<DescriptorPool>(mDevice,
            layout, capacity));
        auto& pool = mPools.back();
        auto ret = pool->obtainSet(vklayout);
        assert_invariant(ret != VK_NULL_HANDLE && "failed to obtain a set?");
        return ret;
    }

    void recycle(VulkanDescriptorSetLayout* layout,
        VkDescriptorSet vkSet) {
        for (auto& pool : mPools) {
            if (!pool->canAllocate(layout)) {
                continue;
            }
            pool->recycle(layout->getVkLayout(), vkSet);
            break;
        }
    }

private:
    VkDevice mDevice;
    std::vector<std::unique_ptr<DescriptorPool>> mPools;
};