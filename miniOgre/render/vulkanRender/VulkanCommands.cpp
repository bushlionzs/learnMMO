/*
 * Copyright (C) 2021 The Android Open Source Project
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

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26812) // Unscoped enums
#endif
#include "OgreHeader.h"
#include "VulkanCommands.h"

#include "VulkanConstants.h"
#include "VulkanContext.h"

#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/debug.h>

using namespace bluevk;
using namespace utils;

namespace filament::backend {

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    using Timestamp = VulkanGroupMarkers::Timestamp;
#endif

    VulkanCmdFence::VulkanCmdFence(VkFence ifence)
        : fence(ifence) {
        // Internally we use the VK_INCOMPLETE status to mean "not yet submitted". When this fence gets
        // submitted, its status changes to VK_NOT_READY. Finally, when the GPU actually finishes
        // executing the command buffer, the status changes to VK_SUCCESS.
        status.store(VK_INCOMPLETE);
    }

    VulkanCommandBuffer::VulkanCommandBuffer(VulkanResourceAllocator* allocator, VkDevice device,
        VkCommandPool pool)
        : mResourceManager(allocator),
        mPipeline(VK_NULL_HANDLE) {
        // Create the low-level command buffer.
        const VkCommandBufferAllocateInfo allocateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
        };

        // The buffer allocated here will be implicitly reset when vkBeginCommandBuffer is called.
        // We don't need to deallocate since destroying the pool will free all of the buffers.
        vkAllocateCommandBuffers(device, &allocateInfo, &mBuffer);
    }

    CommandBufferObserver::~CommandBufferObserver() {}

    static VkCommandPool createPool(VkDevice device, uint32_t queueFamilyIndex) {
        VkCommandPoolCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
                         | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                .queueFamilyIndex = queueFamilyIndex,
        };
        VkCommandPool pool;
        vkCreateCommandPool(device, &createInfo, VKALLOC, &pool);
        return pool;
    }

#if FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
    void VulkanGroupMarkers::push(std::string const& marker, Timestamp start) noexcept {
        mMarkers.push_back(marker);

#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
        mTimestamps.push_back(start.time_since_epoch().count() > 0.0
            ? start
            : std::chrono::high_resolution_clock::now());
#endif
    }

    std::pair<std::string, Timestamp> VulkanGroupMarkers::pop() noexcept {
        auto const marker = mMarkers.back();
        mMarkers.pop_back();

#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
        auto const timestamp = mTimestamps.back();
        mTimestamps.pop_back();
        return std::make_pair(marker, timestamp);
#else
        return std::make_pair(marker, Timestamp{});
#endif
    }

    std::pair<std::string, Timestamp> VulkanGroupMarkers::pop_bottom() noexcept {
        auto const marker = mMarkers.front();
        mMarkers.pop_front();

#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
        auto const timestamp = mTimestamps.front();
        mTimestamps.pop_front();
        return std::make_pair(marker, timestamp);
#else
        return std::make_pair(marker, Timestamp{});
#endif
    }

    std::pair<std::string, Timestamp> VulkanGroupMarkers::top() const {
        assert_invariant(!empty());
        auto const marker = mMarkers.back();
#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
        auto const topTimestamp = mTimestamps.front();
        return std::make_pair(marker, topTimestamp);
#else
        return std::make_pair(marker, Timestamp{});
#endif
    }

    bool VulkanGroupMarkers::empty() const noexcept {
        return mMarkers.empty();
    }

#endif // FVK_DEBUG_GROUP_MARKERS

    VulkanCommands::VulkanCommands(VkDevice device, VkQueue queue, uint32_t queueFamilyIndex,
        VulkanContext* context, VulkanResourceAllocator* allocator)
        : mDevice(device),
        mQueue(queue),
        mPool(createPool(mDevice, queueFamilyIndex)),
        mContext(context),
        mStorage(CAPACITY) {
        VkSemaphoreCreateInfo sci{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        for (auto& semaphore : mSubmissionSignals) {
            vkCreateSemaphore(mDevice, &sci, nullptr, &semaphore);
        }

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (auto& fence : mFences) {
            vkCreateFence(device, &fenceCreateInfo, VKALLOC, &fence);
        }

        for (size_t i = 0; i < CAPACITY; ++i) {
            mStorage[i] = std::make_unique<VulkanCommandBuffer>(allocator, mDevice, mPool);
        }

#if !FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS)
        (void)mContext;
#endif
    }

    void VulkanCommands::terminate() {
        vkDestroyCommandPool(mDevice, mPool, VKALLOC);
        for (VkSemaphore sema : mSubmissionSignals) {
            vkDestroySemaphore(mDevice, sema, VKALLOC);
        }
        for (VkFence fence : mFences) {
            vkDestroyFence(mDevice, fence, VKALLOC);
        }
    }

    VulkanCommandBuffer& VulkanCommands::get() {
        if (mCurrentCommandBufferIndex >= 0) {
            return *mStorage[mCurrentCommandBufferIndex].get();
        }

        int8_t nextIndex = (mLastCommandBufferIndex + 1) % CAPACITY;

        auto fence = mFences[nextIndex];

        VkResult result = vkWaitForFences(mDevice, 1, &fence, VK_TRUE, UINT64_MAX);

        vkResetFences(mDevice, 1, &fence);

        VulkanCommandBuffer* currentbuf = mStorage[nextIndex].get();
        
        mCurrentCommandBufferIndex = nextIndex;
        assert_invariant(currentbuf);

        // Note that the fence wrapper uses shared_ptr because a DriverAPI fence can also have ownership
        // over it.  The destruction of the low-level fence occurs either in VulkanCommands::gc(), or in
        // VulkanDriver::destroyFence(), both of which are safe spots.
        currentbuf->fence = std::make_shared<VulkanCmdFence>(mFences[nextIndex]);

        // Begin writing into the command buffer.
        const VkCommandBufferBeginInfo binfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        vkBeginCommandBuffer(currentbuf->buffer(), &binfo);

        // Notify the observer that a new command buffer has been activated.
        if (mObserver) {
            mObserver->onCommandBuffer(*currentbuf);
        }

        return *currentbuf;
    }

    bool VulkanCommands::flush(bool waitCmd) {
        // It's perfectly fine to call flush when no commands have been written.
        if (mCurrentCommandBufferIndex < 0) {
            return false;
        }

        int8_t const index = mCurrentCommandBufferIndex;
        VulkanCommandBuffer const* currentbuf = mStorage[index].get();
        VkSemaphore const renderingFinished = mSubmissionSignals[index];

        vkEndCommandBuffer(currentbuf->buffer());

        // If the injected semaphore is an "image available" semaphore that has not yet been signaled,
        // it is sometimes fine to start executing commands anyway, as along as we stall the GPU at the
        // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage. However we need to assume the worst
        // here and use VK_PIPELINE_STAGE_ALL_COMMANDS_BIT. This is a more aggressive stall, but it is
        // the only safe option because the previously submitted command buffer might have set up some
        // state that the new command buffer depends on.
        VkPipelineStageFlags waitDestStageMasks[2] = {
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        };

        VkSemaphore signals[2] = {
                VK_NULL_HANDLE,
                VK_NULL_HANDLE,
        };
        uint32_t waitSemaphoreCount = 0;
        if (mSubmissionSignal) {
            signals[waitSemaphoreCount++] = mSubmissionSignal;
        }
        if (mInjectedSignal) {
            signals[waitSemaphoreCount++] = mInjectedSignal;
        }
        VkCommandBuffer const cmdbuffer = currentbuf->buffer();
        VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = waitSemaphoreCount,
                .pWaitSemaphores = waitSemaphoreCount > 0 ? signals: nullptr,
                .pWaitDstStageMask = waitDestStageMasks,
                .commandBufferCount = 1,
                .pCommandBuffers = &cmdbuffer,
                .signalSemaphoreCount = 1u,
                .pSignalSemaphores = &renderingFinished,
        };

        auto& cmdfence = currentbuf->fence;

        UTILS_UNUSED_IN_RELEASE VkResult result = vkQueueSubmit(mQueue, 1, &submitInfo, cmdfence->getFence());

        assert_invariant(result == VK_SUCCESS);

        mSubmissionSignal = renderingFinished;
        mInjectedSignal = VK_NULL_HANDLE;
        mLastCommandBufferIndex = mCurrentCommandBufferIndex;
        mCurrentCommandBufferIndex = -1;

        if (waitCmd)
        {
            auto& fence = cmdfence->getFence();
            VkResult result = vkWaitForFences(mDevice, 1, &fence, VK_TRUE, UINT64_MAX);
            assert_invariant(result == VK_SUCCESS);
        }
        return true;
    }

    VkSemaphore VulkanCommands::acquireFinishedSignal() {
        VkSemaphore semaphore = mSubmissionSignal;
        mSubmissionSignal = VK_NULL_HANDLE;
        return semaphore;
    }

    void VulkanCommands::injectDependency(VkSemaphore next) {
        assert_invariant(mInjectedSignal == VK_NULL_HANDLE);
        mInjectedSignal = next;
    }

    

} // namespace filament::backend

#if defined(_MSC_VER)
#pragma warning( pop )
#endif
