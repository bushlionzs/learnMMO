#include "OgreHeader.h"
#include <platform_file.h>
#include "VulkanRenderSystem.h"
#include "OgreRenderable.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreMaterial.h"
#include "OgreRoot.h"
#include "OgreResourceManager.h"
#include "glslUtil.h"
#include "VulkanRenderTarget.h"
#include "VulkanMappings.h"
#include <VulkanPipelineCache.h>
#include <VulkanHandles.h>
#include <VulkanPlatform.h>
#include <VulkanPipelineLayoutCache.h>
#include "VulkanTexture.h"
#include "VulkanTools.h"
#include "VulkanHelper.h"
#include "VulkanWindow.h"
#include "VulkanLayoutCache.h"
#include "hlslUtil.h"


VulkanRenderSystem::VulkanRenderSystem(HWND wnd)
{
    bluevk::initialize();

    mRenderSystemName = "Vulkan";
    mRenderType = EngineType_Vulkan;
        
}

VulkanRenderSystem::~VulkanRenderSystem()
{

}

bool VulkanRenderSystem::engineInit(bool raytracing)
{
    VulkanRenderSystemBase::engineInit(raytracing);
    

    return true;
}


void VulkanRenderSystem::addAccelerationStructure(
    const AccelerationStructureDesc* pDesc,
    AccelerationStructure** ppAccelerationStructure)
{
    size_t memSize = sizeof(VulkanAccelerationStructure);
    if (ACCELERATION_STRUCTURE_TYPE_BOTTOM == pDesc->mType)
    {
        memSize += pDesc->mBottom.mDescCount * sizeof(VkAccelerationStructureGeometryKHR);
        memSize += pDesc->mBottom.mDescCount * sizeof(uint32_t);
    }
    else
    {
        memSize += 1 * sizeof(VkAccelerationStructureGeometryKHR);
        memSize += 1 * sizeof(uint32_t);
    }

    VulkanAccelerationStructure* pAS = (VulkanAccelerationStructure*)malloc(memSize);
   

    pAS->mFlags = VulkanMappings::ToVkBuildASFlags(pDesc->mFlags);
    pAS->mType = VulkanMappings::ToVkASType(pDesc->mType);
    pAS->pGeometryDescs = (VkAccelerationStructureGeometryKHR*)(pAS + 1);
    pAS->mPrimitiveCount = 0;
    // TODO these would ideally be freed as soon as the AS is built. 
    // Right now they stay alive until the AS is removed.
    pAS->pGeometryDescs = (VkAccelerationStructureGeometryKHR*)(pAS + 1);
    pAS->pMaxPrimitiveCountPerGeometry = 
        (uint32_t*)(pAS->pGeometryDescs + pDesc->mBottom.mDescCount);

    VkDeviceSize scratchBufferSize = 0;

    if (ACCELERATION_STRUCTURE_TYPE_BOTTOM == pDesc->mType)
    {
        pAS->mDescCount = pDesc->mBottom.mDescCount;

        for (uint32_t j = 0; j < pAS->mDescCount; ++j)
        {
            AccelerationStructureGeometryDesc* pGeom = &pDesc->mBottom.pGeometryDescs[j];
            VkAccelerationStructureGeometryKHR* pGeometry = &pAS->pGeometryDescs[j];
            *pGeometry = VkAccelerationStructureGeometryKHR({});
            pGeometry->sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
            pGeometry->flags = VulkanMappings::util_to_vk_geometry_flags(pGeom->mFlags);
            pGeometry->geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
            pGeometry->geometry.triangles = VkAccelerationStructureGeometryTrianglesDataKHR{};
            pGeometry->geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
            
            uint32_t primitiveCount = 0;

            if (pGeom->mIndexCount)
            {
 
                VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress = {};
                indexBufferDeviceAddress.deviceAddress = 
                    getBufferDeviceAddress(getVkBuffer(pGeom->indexBufferHandle)) + pGeom->mIndexOffset;
                VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};
                transformBufferDeviceAddress.deviceAddress =
                    getBufferDeviceAddress(pGeom->transformBufferHandle) + sizeof(VkTransformMatrixKHR) * j;
                pGeometry->geometry.triangles.indexData = indexBufferDeviceAddress;
                pGeometry->geometry.triangles.indexType =
                    (INDEX_TYPE_UINT16 == pGeom->mIndexType) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

                primitiveCount = pGeom->mIndexCount / 3;

                pGeometry->geometry.triangles.transformData = transformBufferDeviceAddress;
            }
            else
            {
                pGeometry->geometry.triangles.indexData = VkDeviceOrHostAddressConstKHR{};
                pGeometry->geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

                // read 3 vertices at a time.
                primitiveCount = pGeom->mVertexCount / 3;
            }


            VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress = {};
            vertexBufferDeviceAddress.deviceAddress = 
                getBufferDeviceAddress(getVkBuffer(pGeom->vertexBufferHandle)) + pGeom->mVertexOffset;

            pGeometry->geometry.triangles.vertexData = vertexBufferDeviceAddress;
            pGeometry->geometry.triangles.maxVertex = pGeom->mVertexCount;
            pGeometry->geometry.triangles.vertexStride = pGeom->mVertexStride;
            pGeometry->geometry.triangles.vertexFormat = VulkanMappings::getVertexFormat(pGeom->mVertexElementType);

            pAS->mPrimitiveCount += primitiveCount;
            pAS->pMaxPrimitiveCountPerGeometry[j] = primitiveCount;
        }

        // Get size info
        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = {};
        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationStructureBuildGeometryInfo.flags = pAS->mFlags;
        accelerationStructureBuildGeometryInfo.geometryCount = pAS->mDescCount;
        accelerationStructureBuildGeometryInfo.pGeometries = pAS->pGeometryDescs;

        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo = {};
        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(mVulkanPlatform->getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &accelerationStructureBuildGeometryInfo, pAS->pMaxPrimitiveCountPerGeometry,
            &accelerationStructureBuildSizesInfo);

        BufferDesc desc{};
        desc.mBindingType = BufferObjectBinding_AccelerationStructure;
        desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        desc.bufferCreationFlags = BUFFER_CREATION_FLAG_OWN_MEMORY_BIT | BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS |
            BUFFER_CREATION_FLAG_NO_DESCRIPTOR_VIEW_CREATION;
        desc.mSize = accelerationStructureBuildSizesInfo.accelerationStructureSize;

        pAS->asBufferHandle = this->createBufferObject(desc);
        VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info = {};
        accelerationStructureCreate_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        accelerationStructureCreate_info.buffer = getVkBuffer(pAS->asBufferHandle);
        accelerationStructureCreate_info.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        accelerationStructureCreate_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        vkCreateAccelerationStructureKHR(mVulkanPlatform->getDevice(), &accelerationStructureCreate_info,
            nullptr, &pAS->mAccelerationStructure);

        VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo = {};
        accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        accelerationDeviceAddressInfo.accelerationStructure = pAS->mAccelerationStructure;
        pAS->mASDeviceAddress = vkGetAccelerationStructureDeviceAddressKHR(
            mVulkanPlatform->getDevice(), &accelerationDeviceAddressInfo);

        scratchBufferSize = accelerationStructureBuildSizesInfo.buildScratchSize;
    }
    else
    {
        // This naming is a bit confusing now, since on the TLAS, the blases are the primitives.
        pAS->mDescCount = 1;
        pAS->mPrimitiveCount = pDesc->mTop.mDescCount;
        pAS->pMaxPrimitiveCountPerGeometry[0] = pAS->mPrimitiveCount;
        /************************************************************************/
        /*  Construct buffer with instances descriptions                        */
        /************************************************************************/
        VkAccelerationStructureInstanceKHR* instanceDescs = NULL;
        instanceDescs = new VkAccelerationStructureInstanceKHR[pDesc->mTop.mDescCount];
        // Silence PVS
        if (!instanceDescs)
        {
            assert_invariant(false);
            return;
        }

        for (uint32_t i = 0; i < pDesc->mTop.mDescCount; ++i)
        {
            AccelerationStructureInstanceDesc* pInst = &pDesc->mTop.pInstanceDescs[i];
            VulkanAccelerationStructure* vulkanAS = (VulkanAccelerationStructure*)pInst->pBottomAS;
            instanceDescs[i].accelerationStructureReference = vulkanAS->mASDeviceAddress;
            instanceDescs[i].flags = VulkanMappings::util_to_vk_instance_flags(pInst->mFlags);
            instanceDescs[i].instanceShaderBindingTableRecordOffset =
                pInst->mInstanceContributionToHitGroupIndex; // NOTE(Alex): Not sure about this...
            instanceDescs[i].instanceCustomIndex = pInst->mInstanceID;
            instanceDescs[i].mask = pInst->mInstanceMask;
            memcpy(&instanceDescs[i].transform.matrix, pInst->mTransform, sizeof(float[12]));
        }


        uint32_t instanceSize = pDesc->mTop.mDescCount * sizeof(instanceDescs[0]);
        BufferDesc desc{};
        desc.mBindingType = BufferObjectBinding_Storge;
        desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        desc.bufferCreationFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT | 
            BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS |
            BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT;
        desc.mSize = instanceSize;
        pAS->instanceDescBuffer =  createBufferObject(desc);
        this->updateBufferObject(pAS->instanceDescBuffer, (const char*)instanceDescs, instanceSize, 0);
        VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress = {};
        instanceDataDeviceAddress.deviceAddress = getBufferDeviceAddress(getVkBuffer(pAS->instanceDescBuffer));

        VkAccelerationStructureGeometryKHR accelerationStructureGeometry = {};
        accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
        accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;

        // Get size info
        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = {};
        accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        accelerationStructureBuildGeometryInfo.flags = pAS->mFlags;
        accelerationStructureBuildGeometryInfo.geometryCount = 1;
        accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo = {};
        accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(mVulkanPlatform->getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &accelerationStructureBuildGeometryInfo, &pAS->mPrimitiveCount,
            &accelerationStructureBuildSizesInfo);

        desc.mBindingType = BufferObjectBinding_AccelerationStructure;
        desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        desc.bufferCreationFlags = BUFFER_CREATION_FLAG_OWN_MEMORY_BIT | BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS |
            BUFFER_CREATION_FLAG_NO_DESCRIPTOR_VIEW_CREATION;
        desc.mSize = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        pAS->instanceDescBuffer = createBufferObject(desc);
        pAS->asBufferHandle = createBufferObject(desc);

        VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info = {};
        accelerationStructureCreate_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        accelerationStructureCreate_info.buffer = getVkBuffer(pAS->asBufferHandle);
        accelerationStructureCreate_info.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        accelerationStructureCreate_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        vkCreateAccelerationStructureKHR(mVulkanPlatform->getDevice(), &accelerationStructureCreate_info,
            nullptr,
            &pAS->mAccelerationStructure);

        VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo = {};
        accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
        accelerationDeviceAddressInfo.accelerationStructure = pAS->mAccelerationStructure;
        pAS->mASDeviceAddress = vkGetAccelerationStructureDeviceAddressKHR(mVulkanPlatform->getDevice(), &accelerationDeviceAddressInfo);

        memcpy(pAS->pGeometryDescs, &accelerationStructureGeometry, sizeof(VkAccelerationStructureGeometryKHR));

        scratchBufferSize = accelerationStructureBuildSizesInfo.buildScratchSize;
    }

    // Create scratch buffer
    BufferDesc desc{};
    desc.mBindingType = BufferObjectBinding_Storge;
    desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    desc.bufferCreationFlags = BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS | BUFFER_CREATION_FLAG_OWN_MEMORY_BIT;
    desc.mSize = scratchBufferSize;
    pAS->scratchBufferHandle = createBufferObject(desc);
    // Buffer device address
    pAS->mScratchBufferDeviceAddress = getBufferDeviceAddress(getVkBuffer(pAS->scratchBufferHandle));

    *ppAccelerationStructure = pAS;
}

void VulkanRenderSystem::buildAccelerationStructure(RaytracingBuildASDesc* pDesc)
{
    VulkanAccelerationStructure* as = (VulkanAccelerationStructure*)pDesc->pAccelerationStructure;

    VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo = {};
    accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    accelerationBuildGeometryInfo.type = as->mType;
    accelerationBuildGeometryInfo.flags = as->mFlags;
    accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    accelerationBuildGeometryInfo.dstAccelerationStructure = as->mAccelerationStructure;
    accelerationBuildGeometryInfo.geometryCount = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR == as->mType ? as->mDescCount : 1;
    accelerationBuildGeometryInfo.pGeometries = as->pGeometryDescs;
    accelerationBuildGeometryInfo.scratchData.deviceAddress = as->mScratchBufferDeviceAddress;

    VkAccelerationStructureBuildRangeInfoKHR* accelerationStructureBuildRangeInfos =
        (VkAccelerationStructureBuildRangeInfoKHR*)alloca(as->mDescCount * sizeof(VkAccelerationStructureBuildRangeInfoKHR));
    for (uint32_t i = 0; i < as->mDescCount; i++)
    {
        VkAccelerationStructureBuildRangeInfoKHR& accelerationStructureBuildRangeInfo = accelerationStructureBuildRangeInfos[i];
        accelerationStructureBuildRangeInfo = {};
        accelerationStructureBuildRangeInfo.primitiveCount = as->pMaxPrimitiveCountPerGeometry[i];
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;
    }

    VkAccelerationStructureBuildRangeInfoKHR* accelerationBuildStructureRangeInfos[] = { accelerationStructureBuildRangeInfos };

    vkCmdBuildAccelerationStructuresKHR(mCommandBuffer, 1, &accelerationBuildGeometryInfo, accelerationBuildStructureRangeInfos);

    if (pDesc->mIssueRWBarrier)
    {
        BufferBarrier barrier = { as->asBufferHandle, RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE, RESOURCE_STATE_ACCELERATION_STRUCTURE_READ };
        resourceBarrier(1, &barrier, 0, nullptr, 0, nullptr);
    }
}

void VulkanRenderSystem::removeAccelerationStructureScratch(
    AccelerationStructure* pAccelerationStructure)
{
    return;
    VulkanAccelerationStructure* as = (VulkanAccelerationStructure*)pAccelerationStructure;
    destroyBufferObject(as->scratchBufferHandle);
    as->scratchBufferHandle.clear();
}

uint64_t VulkanRenderSystem::getBufferDeviceAddress(VkBuffer vkBuffer)
{
    VkBufferDeviceAddressInfoKHR bufferDeviceAI = {};
    bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = vkBuffer;
    return vkGetBufferDeviceAddressKHR(mVulkanPlatform->getDevice(), &bufferDeviceAI);
}

VkBuffer VulkanRenderSystem::getVkBuffer(Handle<HwBufferObject> bufferHandle)
{
    VulkanBufferObject* bo = mResourceAllocator.handle_cast<VulkanBufferObject*>(bufferHandle);
    return bo->buffer.getGpuBuffer();
}

void VulkanRenderSystem::updateDescriptorSetAccelerationStructure(
    Handle<HwDescriptorSet> dsh,
    backend::descriptor_binding_t binding,
    AccelerationStructure* accStructure)
{
    VulkanDescriptorSet* set = mResourceAllocator.handle_cast<VulkanDescriptorSet*>(dsh);

    VkWriteDescriptorSetAccelerationStructureKHR writeSetKHR = {};
    VkAccelerationStructureKHR                   currUpdateData = {};

    VkWriteDescriptorSet  descriptorWrite{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    descriptorWrite.dstBinding = binding;
    descriptorWrite.dstSet = set->vkSet;
    descriptorWrite.pNext = &writeSetKHR;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    writeSetKHR.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    writeSetKHR.pNext = NULL;
    writeSetKHR.accelerationStructureCount = 1;
    VulkanAccelerationStructure* as = (VulkanAccelerationStructure*)accStructure;
    currUpdateData = as->mAccelerationStructure;
    writeSetKHR.pAccelerationStructures = &currUpdateData;
    bluevk::vkUpdateDescriptorSets(
        mVulkanPlatform->getDevice(),
        1,
        &descriptorWrite, 0, nullptr);
}

Handle<HwRaytracingProgram> VulkanRenderSystem::createRaytracingProgram(
    const RaytracingShaderInfo& shaderInfo)
{
    Handle<HwRaytracingProgram> program = mResourceAllocator.allocHandle<VulkanRaytracingProgram>();

    VulkanRaytracingProgram* vulkanProgram = 
        mResourceAllocator.construct<VulkanRaytracingProgram>(program, shaderInfo.rayGenShaderName);

    VkShaderModuleInfo shaderModuleInfo;
    std::string content;
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};

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

    uint32_t missCount = 0;
    uint32_t hitCount = 0;

    const std::string& rayGenShaderName = shaderInfo.rayGenShaderName;
    const std::string& rayMissShaderName = shaderInfo.rayMissShaderName;
    const std::string& rayShadowShaderName = shaderInfo.rayShadowShaderName;
    const std::string& rayClosethitShaderName = shaderInfo.rayClosethitShaderName;
    const std::string& rayAnyHitShaderName = shaderInfo.rayAnyHitShaderName;

    
    // Ray generation group
    {
        shaderModuleInfo.shaderType = Ogre::ShaderType::RayGenShader;
        ResourceInfo* resInfo = ResourceManager::getSingleton().getResource(rayGenShaderName);
        assert_invariant(resInfo != nullptr);
        get_file_content(resInfo->_fullname.c_str(), content);
        glslCompileShader(resInfo->_fullname, content, shaderInfo.rayGenEntryName, 
            shaderInfo.shaderMacros, &shaderInfo.args, shaderModuleInfo);
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        shaderStage.module = shaderModuleInfo.shaderModule;
        shaderStage.pName = shaderInfo.rayGenEntryName.c_str();
        shaderStages.push_back(shaderStage);

        auto results = vks::tools::getProgramBindings(shaderModuleInfo.spv, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        for (auto& pair : results)
        {
            for (auto& layoutBingding : pair.second)
            {
                bindingMap[pair.first].push_back(layoutBingding);
            }
        }
        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);
    }
    // Miss group
    {
        shaderModuleInfo.shaderType = Ogre::ShaderType::MissShader;
        ResourceInfo* resInfo = ResourceManager::getSingleton().getResource(rayMissShaderName);
        assert_invariant(resInfo != nullptr);
        get_file_content(resInfo->_fullname.c_str(), content);
        glslCompileShader(resInfo->_fullname, content, shaderInfo.rayMissEntryName, 
            shaderInfo.shaderMacros, &shaderInfo.args, shaderModuleInfo);
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
        shaderStage.module = shaderModuleInfo.shaderModule;
        shaderStage.pName = shaderInfo.rayMissEntryName.c_str();
        shaderStages.push_back(shaderStage);

        auto results = vks::tools::getProgramBindings(shaderModuleInfo.spv, VK_SHADER_STAGE_MISS_BIT_KHR);
        bingingUpdate(bindingMap, results, VK_SHADER_STAGE_MISS_BIT_KHR);

        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);
        missCount++;
        // Second shader for shadows

        resInfo = ResourceManager::getSingleton().getResource(rayShadowShaderName);
        if (resInfo && !shaderInfo.rayShadowEntryName.empty())
        {
            get_file_content(resInfo->_fullname.c_str(), content);
            glslCompileShader(resInfo->_fullname, content, shaderInfo.rayShadowEntryName, 
                shaderInfo.shaderMacros, &shaderInfo.args, shaderModuleInfo);
            shaderStage = {};
            shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
            shaderStage.module = shaderModuleInfo.shaderModule;
            shaderStage.pName = shaderInfo.rayShadowEntryName.c_str();
            shaderStages.push_back(shaderStage);
            shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
            shaderGroups.push_back(shaderGroup);
            missCount++;
        }
    }
    // Closest hit group for doing texture lookups
    {
        shaderModuleInfo.shaderType = Ogre::ShaderType::ClosestHitShader;
        ResourceInfo* resInfo = ResourceManager::getSingleton().getResource(rayClosethitShaderName);
        assert_invariant(resInfo != nullptr);
        get_file_content(resInfo->_fullname.c_str(), content);
        glslCompileShader(resInfo->_fullname, content, shaderInfo.rayClosethitEntryName, 
            shaderInfo.shaderMacros, &shaderInfo.args, shaderModuleInfo);
        VkPipelineShaderStageCreateInfo shaderStage = {};
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        shaderStage.module = shaderModuleInfo.shaderModule;
        shaderStage.pName = shaderInfo.rayClosethitEntryName.c_str();
        shaderStages.push_back(shaderStage);

        auto results = vks::tools::getProgramBindings(shaderModuleInfo.spv, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        bingingUpdate(bindingMap, results, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);
        hitCount++;
        resInfo = ResourceManager::getSingleton().getResource(rayAnyHitShaderName);
        if (resInfo && !shaderInfo.rayAnyHitEntryName.empty())
        {
            shaderModuleInfo.shaderType = Ogre::ShaderType::AnyHitShader;
            get_file_content(resInfo->_fullname.c_str(), content);
            glslCompileShader(resInfo->_fullname, content, shaderInfo.rayAnyHitEntryName, 
                shaderInfo.shaderMacros, &shaderInfo.args, shaderModuleInfo);
            results = vks::tools::getProgramBindings(shaderModuleInfo.spv, VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
            bingingUpdate(bindingMap, results, VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
            shaderStage = {};
            shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStage.stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
            shaderStage.module = shaderModuleInfo.shaderModule;
            shaderStage.pName = shaderInfo.rayAnyHitEntryName.c_str();
            shaderStages.push_back(shaderStage);
            shaderGroup.anyHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
            shaderGroups.push_back(shaderGroup);
            hitCount++;
        }
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
            for (auto& layoutBingding : itor->second)
            {
                toBind[bindIndex].binding = layoutBingding.layoutBinding.binding;
                toBind[bindIndex].descriptorType = layoutBingding.layoutBinding.descriptorType;
                toBind[bindIndex].descriptorCount = layoutBingding.layoutBinding.descriptorCount;
                toBind[bindIndex].stageFlags = layoutBingding.layoutBinding.stageFlags;
                toBind[bindIndex].pImmutableSamplers = nullptr;
                bindIndex++;
            }

            
            VulkanDescriptorSetLayout::VulkanDescriptorSetLayoutInfo info;

            for (auto i = 0; i < bindIndex; i++)
            {
                switch (toBind[i].descriptorType)
                {
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    info.combinedImage+= toBind[i].descriptorCount;
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
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                    info.storeImage++;
                    break;
                case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
                    info.accelerationStructure++;
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
        else
        {
            keys.setLayout[set] = pEmptyDescriptorSetLayout;
        }
    }

    keys.pushConstant->size = 0;
    keys.pushConstant->stage = 0;
    VkPipelineLayout pipelineLayout = mPipelineLayoutCache->getLayout(keys);
    
    VkPipeline pipeline;
    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
    rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
    rayTracingPipelineCI.pStages = shaderStages.data();
    rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups.size());
    rayTracingPipelineCI.pGroups = shaderGroups.data();
    rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;
    rayTracingPipelineCI.layout = pipelineLayout;
    VK_CHECK_RESULT(vkCreateRayTracingPipelinesKHR(
        mVulkanPlatform->getDevice(), VK_NULL_HANDLE, VK_NULL_HANDLE, 
        1, &rayTracingPipelineCI, nullptr, &pipeline));

    vulkanProgram->updatePipelineLayout(pipelineLayout);
    vulkanProgram->updatePipeline(pipeline);
    
    //create shader binding table
    auto& rayTracingPipelineProperties = 
        mVulkanPlatform->getRayTracingPipelineProperties();

    const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
    const uint32_t handleSizeAligned = 
        vks::tools::alignedSize(
            rayTracingPipelineProperties.shaderGroupHandleSize, 
            rayTracingPipelineProperties.shaderGroupHandleAlignment);
    const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
    const uint32_t sbtSize = groupCount * handleSizeAligned;
    std::vector<uint8_t> shaderHandleStorage(sbtSize);
    VK_CHECK_RESULT(vkGetRayTracingShaderGroupHandlesKHR(
        mVulkanPlatform->getDevice(), pipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()));
    
    ShaderBindingTables* shaderBindingTables = new ShaderBindingTables;
    vulkanProgram->updateShaderBindingTables(shaderBindingTables);

    createShaderBindingTable(shaderBindingTables->raygen, 1);
    
    createShaderBindingTable(shaderBindingTables->miss, missCount);

    createShaderBindingTable(shaderBindingTables->hit, hitCount);
    // Copy handles
    void* raygenData = shaderBindingTables->raygen.mapped;
    memcpy(raygenData, shaderHandleStorage.data(), handleSize);
    // We are using two miss shaders, so we need to get two handles for the miss shader binding table
    void* missData = shaderBindingTables->miss.mapped;
    memcpy(missData, shaderHandleStorage.data() + handleSizeAligned, handleSize * missCount);
    void* hitData = shaderBindingTables->hit.mapped;
    memcpy(hitData, shaderHandleStorage.data() + handleSizeAligned * (1+ missCount), handleSize * hitCount);
    return program;
}

Handle<HwDescriptorSet> VulkanRenderSystem::createDescriptorSet(
    Handle<HwRaytracingProgram> programHandle,
    uint32_t set)
{
    VulkanRaytracingProgram* program = mResourceAllocator.handle_cast<VulkanRaytracingProgram*>(programHandle);

    auto layoutHandle =  program->getLayout(set);

    Handle<HwDescriptorSet> dsh = mResourceAllocator.allocHandle<VulkanDescriptorSet>();
    VulkanDescriptorSetLayout* layout = mResourceAllocator.handle_cast<VulkanDescriptorSetLayout*>(layoutHandle);
    VkDescriptorSet vkSet = mDescriptorInfinitePool->obtainSet(layout);
    VulkanDescriptorSet* vulkanDescSet = mResourceAllocator.construct<VulkanDescriptorSet>(dsh, &mResourceAllocator, vkSet, set);
    vulkanDescSet->updateVulkanProgram(program);
    return dsh;
}

void VulkanRenderSystem::bindPipeline(
    Handle<HwRaytracingProgram> programHandle,
    const Handle<HwDescriptorSet>* descSets,
    uint32_t setCount
)
{
    VulkanRaytracingProgram* program = mResourceAllocator.handle_cast<VulkanRaytracingProgram*>(programHandle);
    auto pipeline = program->getPipeline();
    auto pipelineLayout = program->getPipelineLayout();
    vkCmdBindPipeline(mCommandBuffer,
        VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);

    
    for (auto i = 0; i < setCount; i++)
    {
        VulkanDescriptorSet* set = mResourceAllocator.handle_cast<VulkanDescriptorSet*>(descSets[i]);
        vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
            pipelineLayout, set->mSet, 1, &set->vkSet, 0, nullptr);
    }
    
}

void VulkanRenderSystem::traceRay(Handle<HwRaytracingProgram> programHandle)
{
    auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
    VulkanRaytracingProgram* program = 
        mResourceAllocator.handle_cast<VulkanRaytracingProgram*>(programHandle);
    ShaderBindingTables* shaderBindingTables = program->getShaderBindingTables();
    VkStridedDeviceAddressRegionKHR emptySbtEntry = {};
    vkCmdTraceRaysKHR(
        mCommandBuffer,
        &shaderBindingTables->raygen.stridedDeviceAddressRegion,
        &shaderBindingTables->miss.stridedDeviceAddressRegion,
        &shaderBindingTables->hit.stridedDeviceAddressRegion,
        &emptySbtEntry,
        ogreConfig.width,
        ogreConfig.height,
        1);
}

void VulkanRenderSystem::copyImage(Ogre::RenderTarget* dst, Ogre::RenderTarget* src)
{
    auto width = src->getWidth();
    auto height = src->getHeight();
    VkImageCopy copyRegion{};
    copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    copyRegion.srcOffset = { 0, 0, 0 };
    copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    copyRegion.dstOffset = { 0, 0, 0 };
    copyRegion.extent = { width, height, 1 };

    VulkanTexture* srcImage = (VulkanTexture*)src->getTarget();
    VulkanTexture* dstImage = (VulkanTexture*)dst->getTarget();
    vkCmdCopyImage(mCommandBuffer, srcImage->getVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
        dstImage->getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        1, &copyRegion);
}

uint64_t VulkanRenderSystem::getBufferDeviceAddress(Handle<HwBufferObject> bufHandle)
{
    VulkanBufferObject* bo = mResourceAllocator.handle_cast<VulkanBufferObject*>(bufHandle);
    return getBufferDeviceAddress(bo->buffer.getGpuBuffer());
}

VkStridedDeviceAddressRegionKHR VulkanRenderSystem::getSbtEntryStridedDeviceAddressRegion(
    VkBuffer buffer, uint32_t handleCount)
{
    auto& rayTracingPipelineProperties =
        mVulkanPlatform->getRayTracingPipelineProperties();
    const uint32_t handleSizeAligned = vks::tools::alignedSize(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
    VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegionKHR{};
    stridedDeviceAddressRegionKHR.deviceAddress = getBufferDeviceAddress(buffer);
    stridedDeviceAddressRegionKHR.stride = handleSizeAligned;
    stridedDeviceAddressRegionKHR.size = handleCount * handleSizeAligned;
    return stridedDeviceAddressRegionKHR;
}

void VulkanRenderSystem::createShaderBindingTable(
    ShaderBindingTable& shaderBindingTable, uint32_t handleCount)
{
    auto& rayTracingPipelineProperties =
        mVulkanPlatform->getRayTracingPipelineProperties();
    // Create buffer to hold all shader handles for the SBT
   VK_CHECK_RESULT(VulkanHelper::getSingleton().createBuffer(
        VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &shaderBindingTable,
        rayTracingPipelineProperties.shaderGroupHandleSize * handleCount));
    /*auto* buffer = new VulkanBuffer(mAllocator, *mStagePool, 
        VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        rayTracingPipelineProperties.shaderGroupHandleSize * handleCount);*/
    // Get the strided address to be used when dispatching the rays
    shaderBindingTable.stridedDeviceAddressRegion = 
        getSbtEntryStridedDeviceAddressRegion(shaderBindingTable.buffer, handleCount);
    shaderBindingTable.map();
}












