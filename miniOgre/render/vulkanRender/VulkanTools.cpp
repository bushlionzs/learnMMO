/*
* Assorted commonly used Vulkan helper functions
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include "OgreHeader.h"
#include "VulkanTools.h"
#include "VulkanTexture.h"
#include "VulkanHelper.h"
#include "VulkanBuffer.h"
#include "glslUtil.h"
#include <spirv.hpp>
#include <platform_file.h>
#include "VulkanMappings.h"
#include "VulkanRenderTarget.h"
#include <SPIRV_Cross/spirv_glsl.hpp>

namespace vks
{
	namespace tools
	{
		bool errorModeSilent = false;

		std::string errorString(VkResult errorCode)
		{
			switch (errorCode)
			{
#define STR(r) case VK_ ##r: return #r
				STR(NOT_READY);
				STR(TIMEOUT);
				STR(EVENT_SET);
				STR(EVENT_RESET);
				STR(INCOMPLETE);
				STR(ERROR_OUT_OF_HOST_MEMORY);
				STR(ERROR_OUT_OF_DEVICE_MEMORY);
				STR(ERROR_INITIALIZATION_FAILED);
				STR(ERROR_DEVICE_LOST);
				STR(ERROR_MEMORY_MAP_FAILED);
				STR(ERROR_LAYER_NOT_PRESENT);
				STR(ERROR_EXTENSION_NOT_PRESENT);
				STR(ERROR_FEATURE_NOT_PRESENT);
				STR(ERROR_INCOMPATIBLE_DRIVER);
				STR(ERROR_TOO_MANY_OBJECTS);
				STR(ERROR_FORMAT_NOT_SUPPORTED);
				STR(ERROR_SURFACE_LOST_KHR);
				STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
				STR(SUBOPTIMAL_KHR);
				STR(ERROR_OUT_OF_DATE_KHR);
				STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
				STR(ERROR_VALIDATION_FAILED_EXT);
				STR(ERROR_INVALID_SHADER_NV);
#undef STR
			default:
				return "UNKNOWN_ERROR";
			}
		}

		std::string physicalDeviceTypeString(VkPhysicalDeviceType type)
		{
			switch (type)
			{
#define STR(r) case VK_PHYSICAL_DEVICE_TYPE_ ##r: return #r
				STR(OTHER);
				STR(INTEGRATED_GPU);
				STR(DISCRETE_GPU);
				STR(VIRTUAL_GPU);
				STR(CPU);
#undef STR
			default: return "UNKNOWN_DEVICE_TYPE";
			}
		}

		VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat)
		{
			// Since all depth formats may be optional, we need to find a suitable depth format to use
			// Start with the highest precision packed format
			std::vector<VkFormat> depthFormats = {
				VK_FORMAT_D32_SFLOAT_S8_UINT,
				VK_FORMAT_D32_SFLOAT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D16_UNORM
			};

			for (auto& format : depthFormats)
			{
				VkFormatProperties formatProps;
				vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
				// Format must support depth stencil attachment for optimal tiling
				if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				{
					*depthFormat = format;
					return true;
				}
			}

			return false;
		}

		VkBool32 formatHasStencil(VkFormat format)
		{
			std::vector<VkFormat> stencilFormats = {
				VK_FORMAT_S8_UINT,
				VK_FORMAT_D16_UNORM_S8_UINT,
				VK_FORMAT_D24_UNORM_S8_UINT,
				VK_FORMAT_D32_SFLOAT_S8_UINT,
			};
			return std::find(stencilFormats.begin(), stencilFormats.end(), format) != std::end(stencilFormats);
		}

		// Returns if a given format support LINEAR filtering
		VkBool32 formatIsFilterable(VkPhysicalDevice physicalDevice, VkFormat format, VkImageTiling tiling)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

			if (tiling == VK_IMAGE_TILING_OPTIMAL)
				return formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

			if (tiling == VK_IMAGE_TILING_LINEAR)
				return formatProps.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

			return false;
		}

		// Create an image memory barrier for changing the layout of
		// an image and put it into an active command buffer
		// See chapter 11.4 "Image Layout" for details

		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage imagex,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask)
		{
			// Create an image barrier object
			VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = imagex;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			// Source layouts (old)
			// Source access mask controls actions that have to be finished on the old layout
			// before it will be transitioned to the new layout
			switch (oldImageLayout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				// Image layout is undefined (or does not matter)
				// Only valid as initial layout
				// No flags required, listed only for completeness
				imageMemoryBarrier.srcAccessMask = 0;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				// Image is preinitialized
				// Only valid as initial layout for linear images, preserves memory contents
				// Make sure host writes have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image is a color attachment
				// Make sure any writes to the color buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image is a depth/stencil attachment
				// Make sure any writes to the depth/stencil buffer have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image is a transfer source
				// Make sure any reads from the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image is a transfer destination
				// Make sure any writes to the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image is read by a shader
				// Make sure any shader reads from the image have been finished
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
			}

			// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
			switch (newImageLayout)
			{
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				// Image will be used as a transfer destination
				// Make sure any writes to the image have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				// Image will be used as a transfer source
				// Make sure any reads from the image have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				// Image will be used as a color attachment
				// Make sure any writes to the color buffer have been finished
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				// Image layout will be used as a depth/stencil attachment
				// Make sure any writes to depth/stencil buffer have been finished
				imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				// Image will be read in a shader (sampler, input attachment)
				// Make sure any writes to the image have been finished
				if (imageMemoryBarrier.srcAccessMask == 0)
				{
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				}
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			default:
				// Other source layouts aren't handled (yet)
				break;
			}

			// Put barrier inside setup command buffer
			vkCmdPipelineBarrier(
				cmdbuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}

		// Fixed sub resource on first mip level and layer
		void setImageLayout(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkImageAspectFlags aspectMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask)
		{
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = aspectMask;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.layerCount = 1;
			setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
		}

		void insertImageMemoryBarrier(
			VkCommandBuffer cmdbuffer,
			VkImage image,
			VkAccessFlags srcAccessMask,
			VkAccessFlags dstAccessMask,
			VkImageLayout oldImageLayout,
			VkImageLayout newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask,
			VkImageSubresourceRange subresourceRange)
		{
			VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
			imageMemoryBarrier.srcAccessMask = srcAccessMask;
			imageMemoryBarrier.dstAccessMask = dstAccessMask;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(
				cmdbuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}

		void exitFatal(const std::string& message, int32_t exitCode)
		{
#if defined(_WIN32)
			if (!errorModeSilent) {
				MessageBox(NULL, message.c_str(), NULL, MB_OK | MB_ICONERROR);
			}
#elif defined(__ANDROID__)
            LOGE("Fatal error: %s", message.c_str());
			vks::android::showAlert(message.c_str());
#endif
			std::cerr << message << "\n";
#if !defined(__ANDROID__)
			exit(exitCode);
#endif
		}

		void exitFatal(const std::string& message, VkResult resultCode)
		{
			exitFatal(message, (int32_t)resultCode);
		}


		VkShaderModule loadShaderBinary(const char *fileName, VkDevice device)
		{
			std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

			if (is.is_open())
			{
				size_t size = is.tellg();
				is.seekg(0, std::ios::beg);
				char* shaderCode = new char[size];
				is.read(shaderCode, size);
				is.close();

				assert(size > 0);

				VkShaderModule shaderModule;
				VkShaderModuleCreateInfo moduleCreateInfo{};
				moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				moduleCreateInfo.codeSize = size;
				moduleCreateInfo.pCode = (uint32_t*)shaderCode;

				VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

				delete[] shaderCode;

				return shaderModule;
			}
			else
			{
				std::cerr << "Error: Could not open shader file \"" << fileName << "\"" << "\n";
				return VK_NULL_HANDLE;
			}
		}

		VkShaderModule loadShaderAssic(const char* fileName, VkDevice device, Ogre::ShaderType type)
		{
			std::string content;
			get_file_content(fileName, content);

			std::string result;
			std::string strName(fileName);
			std::string entryPoint("main");
			std::vector<std::pair<std::string, std::string>> shaderMacros;
			shaderc_shader_kind kind = shaderc_glsl_vertex_shader;
			std::vector<GlslInputDesc> inputDesc;
			VkShaderModuleInfo shaderModuleInfo;
			switch (type)
			{
			case VertexShader:
				shaderModuleInfo.shaderType = Ogre::ShaderType::VertexShader;
				glslCompileShader(strName, content, entryPoint, shaderMacros, shaderModuleInfo);
				break;
			case PixelShader:
				shaderModuleInfo.shaderType = Ogre::ShaderType::PixelShader;
				glslCompileShader(strName, content, entryPoint, shaderMacros, shaderModuleInfo);
				break;
			default:
				assert(false);
				break;
			}
			return shaderModuleInfo.shaderModule;
		}

		VkShaderModule loadShaderMemory(const std::string& code, VkDevice device)
		{
			VkShaderModule shaderModule;
			VkShaderModuleCreateInfo moduleCreateInfo{};
			moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleCreateInfo.codeSize = code.size();
			moduleCreateInfo.pCode = (uint32_t*)code.data();

			VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

			return shaderModule;
		}

		VkPipelineShaderStageCreateInfo loadShader(
			const std::string& fileName,
			VkShaderStageFlagBits stage)
		{
			std::string content;
			get_file_content(fileName.c_str(), content);

			std::string result;
			std::string strName(fileName);
			std::string entryPoint("main");
			std::vector<std::pair<std::string, std::string>> shaderMacros;
			shaderc_shader_kind kind = shaderc_glsl_vertex_shader;
			std::vector<GlslInputDesc> inputDesc;
			VkShaderModuleInfo shaderModuleInfo;
			switch (stage)
			{
			case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
				shaderModuleInfo.shaderType = Ogre::ShaderType::RayGenShader;
				break;
			case VK_SHADER_STAGE_MISS_BIT_KHR:
				shaderModuleInfo.shaderType = Ogre::ShaderType::MissShader;
				break;
			case VK_SHADER_STAGE_ANY_HIT_BIT_KHR:
				shaderModuleInfo.shaderType = Ogre::ShaderType::AnyHitShader;
				break;
			case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
				shaderModuleInfo.shaderType = Ogre::ShaderType::ClosestHitShader;
				break;
			default:
				assert(false);
				break;
			}

			glslCompileShader(strName, content, entryPoint, shaderMacros, shaderModuleInfo);

			VkPipelineShaderStageCreateInfo shaderStage = {};
			shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStage.stage = stage;
			shaderStage.module = shaderModuleInfo.shaderModule;
			shaderStage.pName = "main";
			return shaderStage;
		}

		bool fileExists(const std::string &filename)
		{
			std::ifstream f(filename.c_str());
			return !f.fail();
		}

		uint32_t alignedSize(uint32_t value, uint32_t alignment)
        {
	        return (value + alignment - 1) & ~(alignment - 1);
        }

		uint64_t getBufferDeviceAddress(VkDevice device, VkBuffer buffer)
		{
			VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
			bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			bufferDeviceAI.buffer = buffer;
			return vkGetBufferDeviceAddressKHR(device, &bufferDeviceAI);
		}

		void copyBufferToImage(
			VkCommandBuffer commandBuffer,
			VkBuffer buffer,
			VkImage image,
			VulkanTexture* tex)
		{
			uint32_t mipLevels = tex->getSourceMipmaps() + 1;
			uint32_t face_count = tex->getFace();

			std::vector<VkBufferImageCopy> regions;
			regions.reserve(face_count * mipLevels);

			uint32_t offset = 0;
			for (uint32_t face = 0; face < face_count; face++)
			{
				uint32_t width = tex->getWidth();
				uint32_t height = tex->getHeight();
				for (uint32_t i = 0; i < mipLevels; i++)
				{
					VkBufferImageCopy bufferCopyRegion = {};
					bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					bufferCopyRegion.imageSubresource.mipLevel = i;
					bufferCopyRegion.imageSubresource.baseArrayLayer = face;
					bufferCopyRegion.imageSubresource.layerCount = 1;
					bufferCopyRegion.imageExtent.width = width;
					bufferCopyRegion.imageExtent.height = height;
					bufferCopyRegion.imageExtent.depth = 1;
					bufferCopyRegion.bufferOffset = offset;
					regions.push_back(bufferCopyRegion);

					if (width > 1)width /= 2;
					if (height > 1)height /= 2;
					offset += tex->getBuffer(face, i)->getSizeInBytes();
				}
			}


			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = mipLevels;
			subresourceRange.layerCount = face_count;

			vks::tools::setImageLayout(
				commandBuffer,
				image,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange);
			vkCmdCopyBufferToImage(commandBuffer, buffer, image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (uint32_t)regions.size(), regions.data());

			insertImageMemoryBarrier(
				commandBuffer,
				image,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_TRANSFER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				subresourceRange);
		}

		void resourceBarrier(
			uint32_t numBufferBarriers,
			BufferBarrier* pBufferBarriers,
			uint32_t numTextureBarriers,
			TextureBarrier* pTextureBarriers,
			uint32_t numRtBarriers,
			RenderTargetBarrier* pRtBarriers,
			QueueType queueType,
			uint32_t queueFamilyIndex,
			VkCommandBuffer cmdBuf
		)
		{
			VkMemoryBarrier memoryBarrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };

			VkAccessFlags srcAccessFlags = 0;
			VkAccessFlags dstAccessFlags = 0;

			for (uint32_t i = 0; i < numBufferBarriers; ++i)
			{
				BufferBarrier* pTrans = &pBufferBarriers[i];

				if (RESOURCE_STATE_UNORDERED_ACCESS == pTrans->mCurrentState && RESOURCE_STATE_UNORDERED_ACCESS == pTrans->mNewState)
				{
					memoryBarrier.srcAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
					memoryBarrier.dstAccessMask |= VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
				}
				else
				{
					memoryBarrier.srcAccessMask |= VulkanMappings::util_to_vk_access_flags(pTrans->mCurrentState);
					memoryBarrier.dstAccessMask |= VulkanMappings::util_to_vk_access_flags(pTrans->mNewState);
				}

				srcAccessFlags |= memoryBarrier.srcAccessMask;
				dstAccessFlags |= memoryBarrier.dstAccessMask;
			}

			VkImageMemoryBarrier imageBarriers[256];
			uint32_t imageBarrierCount = 0;
			for (uint32_t i = 0; i < numTextureBarriers; ++i)
			{
				TextureBarrier* pTrans = &pTextureBarriers[i];
				VulkanTexture* pTexture = (VulkanTexture*)pTrans->pTexture;
				VkImageMemoryBarrier* pImageBarrier = NULL;

				if (RESOURCE_STATE_UNORDERED_ACCESS == pTrans->mCurrentState && RESOURCE_STATE_UNORDERED_ACCESS == pTrans->mNewState)
				{
					pImageBarrier = &imageBarriers[imageBarrierCount++];           //-V522
					pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; //-V522
					pImageBarrier->pNext = NULL;

					pImageBarrier->srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
					pImageBarrier->dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
					pImageBarrier->oldLayout = VK_IMAGE_LAYOUT_GENERAL;
					pImageBarrier->newLayout = VK_IMAGE_LAYOUT_GENERAL;
				}
				else
				{
					pImageBarrier = &imageBarriers[imageBarrierCount++];
					pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					pImageBarrier->pNext = NULL;

					pImageBarrier->srcAccessMask = VulkanMappings::util_to_vk_access_flags(pTrans->mCurrentState);
					pImageBarrier->dstAccessMask = VulkanMappings::util_to_vk_access_flags(pTrans->mNewState);
					pImageBarrier->oldLayout = VulkanMappings::util_to_vk_image_layout(pTrans->mCurrentState);
					pImageBarrier->newLayout = VulkanMappings::util_to_vk_image_layout(pTrans->mNewState);
					assert_invariant(pImageBarrier->newLayout != VK_IMAGE_LAYOUT_UNDEFINED);
				}

				if (pImageBarrier)
				{
					pImageBarrier->image = pTexture->getVkImage();
					pImageBarrier->subresourceRange.aspectMask = (VkImageAspectFlags)pTexture->getAspectFlag();
					pImageBarrier->subresourceRange.baseMipLevel = pTrans->mSubresourceBarrier ? pTrans->mMipLevel : 0;
					pImageBarrier->subresourceRange.levelCount = pTrans->mSubresourceBarrier ? 1 : VK_REMAINING_MIP_LEVELS;
					pImageBarrier->subresourceRange.baseArrayLayer = pTrans->mSubresourceBarrier ? pTrans->mArrayLayer : 0;
					pImageBarrier->subresourceRange.layerCount = pTrans->mSubresourceBarrier ? 1 : VK_REMAINING_ARRAY_LAYERS;

					if (pTrans->mAcquire && pTrans->mCurrentState != RESOURCE_STATE_UNDEFINED)
					{
						pImageBarrier->srcQueueFamilyIndex = queueFamilyIndex;
						pImageBarrier->dstQueueFamilyIndex = queueFamilyIndex;
					}
					else if (pTrans->mRelease && pTrans->mCurrentState != RESOURCE_STATE_UNDEFINED)
					{
						pImageBarrier->srcQueueFamilyIndex = queueFamilyIndex;
						pImageBarrier->dstQueueFamilyIndex = queueFamilyIndex;
					}
					else
					{
						pImageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						pImageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					}

					srcAccessFlags |= pImageBarrier->srcAccessMask;
					dstAccessFlags |= pImageBarrier->dstAccessMask;
				}
			}

			for (uint32_t i = 0; i < numRtBarriers; ++i)
			{
				RenderTargetBarrier* pTrans = &pRtBarriers[i];
				Ogre::VulkanRenderTarget* vulkanRenderTarget = (Ogre::VulkanRenderTarget*)pTrans->pRenderTarget;

				VkImageMemoryBarrier* pImageBarrier = NULL;

				if (RESOURCE_STATE_UNORDERED_ACCESS == pTrans->mCurrentState && RESOURCE_STATE_UNORDERED_ACCESS == pTrans->mNewState)
				{
					pImageBarrier = &imageBarriers[imageBarrierCount++];
					pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					pImageBarrier->pNext = NULL;

					pImageBarrier->srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
					pImageBarrier->dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
					pImageBarrier->oldLayout = VK_IMAGE_LAYOUT_GENERAL;
					pImageBarrier->newLayout = VK_IMAGE_LAYOUT_GENERAL;
				}
				else
				{
					pImageBarrier = &imageBarriers[imageBarrierCount++];
					pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
					pImageBarrier->pNext = NULL;

					pImageBarrier->srcAccessMask = VulkanMappings::util_to_vk_access_flags(pTrans->mCurrentState);
					pImageBarrier->dstAccessMask = VulkanMappings::util_to_vk_access_flags(pTrans->mNewState);
					pImageBarrier->oldLayout = VulkanMappings::util_to_vk_image_layout(pTrans->mCurrentState);
					pImageBarrier->newLayout = VulkanMappings::util_to_vk_image_layout(pTrans->mNewState);
					assert_invariant(pImageBarrier->newLayout != VK_IMAGE_LAYOUT_UNDEFINED);
				}

				if (pImageBarrier)
				{
					pImageBarrier->image = vulkanRenderTarget->getImage();
					pImageBarrier->subresourceRange.aspectMask = vulkanRenderTarget->getAspectFlag();
					pImageBarrier->subresourceRange.baseMipLevel = pTrans->mSubresourceBarrier ? pTrans->mMipLevel : 0;
					pImageBarrier->subresourceRange.levelCount = pTrans->mSubresourceBarrier ? 1 : VK_REMAINING_MIP_LEVELS;
					pImageBarrier->subresourceRange.baseArrayLayer = pTrans->mSubresourceBarrier ? pTrans->mArrayLayer : 0;
					pImageBarrier->subresourceRange.layerCount = pTrans->mSubresourceBarrier ? 1 : VK_REMAINING_ARRAY_LAYERS;

					if (pTrans->mAcquire && pTrans->mCurrentState != RESOURCE_STATE_UNDEFINED)
					{
						pImageBarrier->srcQueueFamilyIndex =
							queueFamilyIndex;
						pImageBarrier->dstQueueFamilyIndex =
							queueFamilyIndex;
					}
					else if (pTrans->mRelease && pTrans->mCurrentState != RESOURCE_STATE_UNDEFINED)
					{
						pImageBarrier->srcQueueFamilyIndex =
							queueFamilyIndex;
						pImageBarrier->dstQueueFamilyIndex =
							queueFamilyIndex;
					}
					else
					{
						pImageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						pImageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					}

					srcAccessFlags |= pImageBarrier->srcAccessMask;
					dstAccessFlags |= pImageBarrier->dstAccessMask;
				}
			}

			auto& setting = VulkanHelper::getSingleton().getVulkanSettings();

			VkPipelineStageFlags srcStageMask = vks::tools::util_determine_pipeline_stage_flags(
				&setting, srcAccessFlags, queueType);
			VkPipelineStageFlags dstStageMask = vks::tools::util_determine_pipeline_stage_flags(
				&setting, dstAccessFlags, queueType);

			if (srcAccessFlags || dstAccessFlags)
			{
				vkCmdPipelineBarrier(
					cmdBuf,
					srcStageMask, dstStageMask, 0, memoryBarrier.srcAccessMask ? 1 : 0,
					memoryBarrier.srcAccessMask ? &memoryBarrier : NULL, 0, NULL,
					imageBarrierCount, imageBarriers);
			}
		}

		void generateMipmaps(VkCommandBuffer commandBuffer, VulkanTexture* tex)
		{
			auto mipLevels = tex->getMipLevels();
			uint32_t face_count = tex->getFace();
			

			VkImage image = tex->getVkImage();

			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.layerCount = face_count;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			insertImageMemoryBarrier(
				commandBuffer,
				image,
				VK_ACCESS_TRANSFER_READ_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				subresourceRange);

			for (auto face = 0; face < face_count; face++)
			{
				uint32_t width = tex->getTextureProperty()->_width;
				uint32_t height = tex->getTextureProperty()->_height;
				

				for (uint32_t i = 1; i < mipLevels; i++) {
					VkImageBlit imageBlit{};
					VkImageSubresourceRange mipSubRange = {};
					mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					mipSubRange.baseMipLevel = i;
					mipSubRange.levelCount = 1;
					mipSubRange.layerCount = 1;
					mipSubRange.baseArrayLayer = face;

					insertImageMemoryBarrier(
						commandBuffer,
						image,
						0,
						VK_ACCESS_TRANSFER_WRITE_BIT,
						VK_IMAGE_LAYOUT_UNDEFINED,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						VK_PIPELINE_STAGE_TRANSFER_BIT,
						VK_PIPELINE_STAGE_TRANSFER_BIT,
						mipSubRange);

					imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageBlit.srcSubresource.layerCount = 1;
					imageBlit.srcSubresource.mipLevel = i - 1;
					imageBlit.srcSubresource.baseArrayLayer = face;
					imageBlit.srcOffsets[1].x = int32_t(width >> (i - 1));
					imageBlit.srcOffsets[1].y = int32_t(height >> (i - 1));
					imageBlit.srcOffsets[1].z = 1;

					imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imageBlit.dstSubresource.layerCount = 1;
					imageBlit.dstSubresource.mipLevel = i;
					imageBlit.dstSubresource.baseArrayLayer = face;
					imageBlit.dstOffsets[1].x = int32_t(width >> i);
					imageBlit.dstOffsets[1].y = int32_t(height >> i);
					imageBlit.dstOffsets[1].z = 1;

					vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

					insertImageMemoryBarrier(
						commandBuffer,
						image,
						VK_ACCESS_TRANSFER_WRITE_BIT,
						VK_ACCESS_TRANSFER_READ_BIT,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						VK_PIPELINE_STAGE_TRANSFER_BIT,
						VK_PIPELINE_STAGE_TRANSFER_BIT,
						mipSubRange);
				}
			}

			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = mipLevels;
			subresourceRange.baseArrayLayer = 0;
			subresourceRange.layerCount = face_count;
			insertImageMemoryBarrier(
				commandBuffer,
				image,
				VK_ACCESS_TRANSFER_READ_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				subresourceRange);
		}

		BingdingInfo getProgramBindings(const std::string& blob, VkShaderStageFlags stageFlags)
		{
			
			BingdingInfo result;
			spirv_cross::CompilerGLSL  glsl((const uint32_t*)blob.data(), blob.size() / 4);

			auto uniforms = glsl.get_shader_resources().uniform_buffers;

			VkDescriptorSetLayoutBinding layout{};
			for (int32_t i = 0; i < uniforms.size(); i++)
			{
				auto& input = uniforms[i];
				const std::string& name = glsl.get_name(input.id);

				auto set = glsl.get_decoration(input.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(input.id, spv::DecorationBinding);
				spirv_cross::SPIRType type = glsl.get_type(input.type_id);

				if (type.array.size())
					layout.descriptorCount = type.array[0];
				else
					layout.descriptorCount = 1;

				layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				layout.binding = binding;
				layout.stageFlags = stageFlags;
				result[set].push_back(layout);
			}

			auto storage_buffers = glsl.get_shader_resources().storage_buffers;

			for (int32_t i = 0; i < storage_buffers.size(); i++)
			{
				auto& input = storage_buffers[i];
				const std::string& name = glsl.get_name(input.id);

				auto set = glsl.get_decoration(input.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(input.id, spv::DecorationBinding);
				
				spirv_cross::SPIRType type = glsl.get_type(input.type_id);

				if (type.array.size())
					layout.descriptorCount = type.array[0];
				else
					layout.descriptorCount = 1;

				layout.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

				layout.binding = binding;
				layout.stageFlags = stageFlags;


				result[set].push_back(layout);
			}

			auto separate_samplers = glsl.get_shader_resources().separate_samplers;

			for (int32_t i = 0; i < separate_samplers.size(); i++)
			{
				auto& input = separate_samplers[i];
				const std::string& name = glsl.get_name(input.id);

				auto set = glsl.get_decoration(input.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(input.id, spv::DecorationBinding);

				spirv_cross::SPIRType type = glsl.get_type(input.type_id);

				if (type.array.size())
					layout.descriptorCount = type.array[0];
				else
					layout.descriptorCount = 1;

				layout.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

				layout.binding = binding;
				layout.stageFlags = stageFlags;

				result[set].push_back(layout);
			}

			auto separate_images = glsl.get_shader_resources().separate_images;

			for (int32_t i = 0; i < separate_images.size(); i++)
			{
				auto& input = separate_images[i];
				const std::string& name = glsl.get_name(input.id);

				auto set = glsl.get_decoration(input.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(input.id, spv::DecorationBinding);

				spirv_cross::SPIRType type = glsl.get_type(input.type_id);

				if (type.array.size())
					layout.descriptorCount = type.array[0];
				else
					layout.descriptorCount = 1;

				layout.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

				layout.binding = binding;
				layout.stageFlags = stageFlags;

				result[set].push_back(layout);
			}

			auto storage_images = glsl.get_shader_resources().storage_images;

			for (int32_t i = 0; i < storage_images.size(); i++)
			{
				auto& input = storage_images[i];
				const std::string& name = glsl.get_name(input.id);

				auto set = glsl.get_decoration(input.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(input.id, spv::DecorationBinding);

				spirv_cross::SPIRType type = glsl.get_type(input.type_id);

				if (type.array.size())
					layout.descriptorCount = type.array[0];
				else
					layout.descriptorCount = 1;

				layout.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				layout.binding = binding;
				layout.stageFlags = stageFlags;

				result[set].push_back(layout);
			}

			auto sampled_images = glsl.get_shader_resources().sampled_images;

			for (int32_t i = 0; i < sampled_images.size(); i++)
			{
				auto& input = sampled_images[i];
				const std::string& name = glsl.get_name(input.id);

				auto set = glsl.get_decoration(input.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(input.id, spv::DecorationBinding);

				spirv_cross::SPIRType type = glsl.get_type(input.type_id);

				if (type.array.size())
					layout.descriptorCount = type.array[0];
				else
					layout.descriptorCount = 1;

				layout.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

				layout.binding = binding;
				layout.stageFlags = stageFlags;

				result[set].push_back(layout);
			}


			auto acceleration_structures = glsl.get_shader_resources().acceleration_structures;

			for (int32_t i = 0; i < acceleration_structures.size(); i++)
			{
				auto& input = acceleration_structures[i];
				const std::string& name = glsl.get_name(input.id);

				auto set = glsl.get_decoration(input.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(input.id, spv::DecorationBinding);

				spirv_cross::SPIRType type = glsl.get_type(input.type_id);

				if (type.array.size())
					layout.descriptorCount = type.array[0];
				else
					layout.descriptorCount = 1;

				layout.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

				layout.binding = binding;
				layout.stageFlags = stageFlags;

				result[set].push_back(layout);
			}

			return result;
		}

		VkPipelineStageFlags util_determine_pipeline_stage_flags(
			VulkanSettings* settings,
			VkAccessFlags accessFlags,
			QueueType queueType)
		{
			VkPipelineStageFlags flags = 0;

			if (settings->mRayPipelineSupported)
			{
				if (accessFlags & (VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR))
				{
					flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
				}
				if (accessFlags & (VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR))
				{
					flags |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
				}
			}

			switch (queueType)
			{
			case QUEUE_TYPE_GRAPHICS:
			{
				if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0)
					flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

				if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
				{
					flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
					flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					if (settings->mGeometryShaderSupported)
					{
						flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
					}
					if (settings->mTessellationSupported)
					{
						flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
						flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
					}
					flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

					if (settings->mRayPipelineSupported)
					{
						flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
					}
				}

				if ((accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0)
					flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

				if ((accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0)
					flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

				if ((accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
					flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

				break;
			}
			case QUEUE_TYPE_COMPUTE:
			{
				if ((accessFlags & (VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT)) != 0 ||
					(accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT) != 0 ||
					(accessFlags & (VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)) != 0 ||
					(accessFlags & (VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)) != 0)
					return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

				if ((accessFlags & (VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT)) != 0)
					flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

				break;
			}
			case QUEUE_TYPE_TRANSFER:
				return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			}

			// Compatible with both compute and graphics queues
			if ((accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT) != 0)
				flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

			if ((accessFlags & (VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

			if ((accessFlags & (VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT)) != 0)
				flags |= VK_PIPELINE_STAGE_HOST_BIT;

			if (flags == 0)
				flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

			return flags;
		}
	}

	
}
