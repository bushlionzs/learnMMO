#pragma once

#include "VulkanRenderSystemBase.h"
#include "rayTracing.h"
#include "FVulkanBuffer.h"



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

    virtual Handle<HwRaytracingProgram> createRaytracingProgram(
        const ShaderInfo& mShaderInfo);
    virtual Handle<HwDescriptorSet> createDescriptorSet(
        Handle<HwRaytracingProgram> programHandle,
        uint32_t set) override;

    virtual void bindPipeline(
        Handle<HwRaytracingProgram> programHandle,
        Handle<HwDescriptorSet>* descSets,
        uint32_t setCount
    );

    virtual void traceRay(Handle<HwRaytracingProgram> programHandle);
    virtual void copyImage(Ogre::RenderTarget* dst, Ogre::RenderTarget* src);
    virtual uint64_t getBufferDeviceAddress(Handle<HwBufferObject> bufHandle);
private:
    uint64_t getBufferDeviceAddress(VkBuffer vkBuffer);

    VkBuffer getVkBuffer(Handle<HwBufferObject> bufferHandle);

    VkStridedDeviceAddressRegionKHR getSbtEntryStridedDeviceAddressRegion(
        VkBuffer buffer, uint32_t handleCount);
    void createShaderBindingTable(
        ShaderBindingTable& shaderBindingTable, uint32_t handleCount);
private:
    
    
};