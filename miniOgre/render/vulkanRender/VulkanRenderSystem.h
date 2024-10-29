#pragma once

#include "VulkanRenderSystemBase.h"
#include "rayTracing.h"


class VulkanRenderSystem : public VulkanRenderSystemBase
{
public:
    VulkanRenderSystem(HWND wnd);
	~VulkanRenderSystem();

    virtual bool engineInit(bool raytracing) override;
    
    virtual void addAccelerationStructure(
        const AccelerationStructureDesc* pDesc,
        AccelerationStructure** ppAccelerationStructure) override;

    virtual void buildAccelerationStructure(RaytracingBuildASDesc* pDesc) override;

    virtual void removeAccelerationStructureScratch(
        AccelerationStructure* pAccelerationStructure) override;

    virtual void updateDescriptorSetAccelerationStructure(
        Handle<HwDescriptorSet> dsh,
        backend::descriptor_binding_t binding,
        AccelerationStructure* accStructure) override;
    /// 
private:
    uint64_t getBufferDeviceAddress(VkBuffer vkBuffer);

    VkBuffer getVkBuffer(Handle<HwBufferObject> bufferHandle);
private:
    
    
};