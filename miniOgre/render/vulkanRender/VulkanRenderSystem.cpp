#include "OgreHeader.h"
#include "VulkanRenderSystem.h"
#include "OgreRenderable.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreMaterial.h"
#include "OgreRoot.h"
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

struct AccelerationStructure
{
    Handle<HwBufferObject> asBufferHandle;
    uint64_t               mASDeviceAddress;
    Handle<HwBufferObject> scratchBufferHandle;
    uint64_t               mScratchBufferDeviceAddress;
    Handle<HwBufferObject> instanceDescBuffer;
    VkAccelerationStructureGeometryKHR* pGeometryDescs;
    uint32_t* pMaxPrimitiveCountPerGeometry;
    VkAccelerationStructureKHR           mAccelerationStructure;
    uint32_t                             mPrimitiveCount;
    uint32_t                             mDescCount;
    VkBuildAccelerationStructureFlagsKHR mFlags;
    VkAccelerationStructureTypeKHR       mType;
};


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
    size_t memSize = sizeof(AccelerationStructure);
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

    AccelerationStructure* pAS = (AccelerationStructure*)malloc(memSize);
   

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
                indexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(getVkBuffer(pGeom->indexBufferHandle)) + pGeom->mIndexOffset;

                pGeometry->geometry.triangles.indexData = indexBufferDeviceAddress;
                pGeometry->geometry.triangles.indexType =
                    (INDEX_TYPE_UINT16 == pGeom->mIndexType) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

                primitiveCount = pGeom->mIndexCount / 3;
            }
            else
            {
                pGeometry->geometry.triangles.indexData = VkDeviceOrHostAddressConstKHR{};
                pGeometry->geometry.triangles.indexType = VK_INDEX_TYPE_NONE_KHR;

                // read 3 vertices at a time.
                primitiveCount = pGeom->mVertexCount / 3;
            }


            VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress = {};
            vertexBufferDeviceAddress.deviceAddress = getBufferDeviceAddress(getVkBuffer(pGeom->vertexBufferHandle)) + pGeom->mVertexOffset;

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


        pAS->asBufferHandle = this->createBufferObject(
            BufferObjectBinding::BufferObjectBinding_AccelerationStructure,
            BUFFER_CREATION_FLAG_OWN_MEMORY_BIT | BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS |
            BUFFER_CREATION_FLAG_NO_DESCRIPTOR_VIEW_CREATION,
            accelerationStructureBuildSizesInfo.accelerationStructureSize,
            nullptr
        );
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

            instanceDescs[i].accelerationStructureReference = pInst->pBottomAS->mASDeviceAddress;
            instanceDescs[i].flags = VulkanMappings::util_to_vk_instance_flags(pInst->mFlags);
            instanceDescs[i].instanceShaderBindingTableRecordOffset =
                pInst->mInstanceContributionToHitGroupIndex; // NOTE(Alex): Not sure about this...
            instanceDescs[i].instanceCustomIndex = pInst->mInstanceID;
            instanceDescs[i].mask = pInst->mInstanceMask;
            memcpy(&instanceDescs[i].transform.matrix, pInst->mTransform, sizeof(float[12]));
        }


        uint32_t instanceSize = pDesc->mTop.mDescCount * sizeof(instanceDescs[0]);
        //todo
        pAS->instanceDescBuffer = 
            createBufferObject(
                BufferObjectBinding::BufferObjectBinding_Storge,
                BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT | BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS |
                BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT,
                instanceSize,
                nullptr);
        this->updateBufferObject(pAS->instanceDescBuffer, (const char*)instanceDescs, instanceSize);
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

        
        pAS->asBufferHandle = createBufferObject(
            BufferObjectBinding::BufferObjectBinding_AccelerationStructure,
            BUFFER_CREATION_FLAG_OWN_MEMORY_BIT | BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS |
            BUFFER_CREATION_FLAG_NO_DESCRIPTOR_VIEW_CREATION,
            accelerationStructureBuildSizesInfo.accelerationStructureSize,
            nullptr);

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

    pAS->scratchBufferHandle = createBufferObject(
        BufferObjectBinding::BufferObjectBinding_Storge,
        BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS | BUFFER_CREATION_FLAG_OWN_MEMORY_BIT,
        scratchBufferSize,
        nullptr);
    // Buffer device address
    pAS->mScratchBufferDeviceAddress = getBufferDeviceAddress(getVkBuffer(pAS->scratchBufferHandle));

    *ppAccelerationStructure = pAS;
}

void VulkanRenderSystem::buildAccelerationStructure(RaytracingBuildASDesc* pDesc)
{
    AccelerationStructure* as = pDesc->pAccelerationStructure;

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
    destroyBufferObject(pAccelerationStructure->scratchBufferHandle);
    pAccelerationStructure->scratchBufferHandle.clear();
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

    currUpdateData = accStructure->mAccelerationStructure;
    writeSetKHR.pAccelerationStructures = &currUpdateData;
    bluevk::vkUpdateDescriptorSets(
        mVulkanPlatform->getDevice(),
        1,
        &descriptorWrite, 0, nullptr);
}













