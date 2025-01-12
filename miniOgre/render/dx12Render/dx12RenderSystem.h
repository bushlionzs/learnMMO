#pragma once

#include "renderSystem.h"
#include "d3dutil.h"
#include "dx12Common.h"
#include "dx12RenderSystemBase.h"
class Dx12GraphicsCommandList;
class Dx12RenderTarget;
class Dx12TextureHandleManager;
class Dx12ShadowMap;
class Dx12RenderWindow;

class Dx12RenderSystem : public Dx12RenderSystemBase
{
public:
    Dx12RenderSystem(HWND wnd);
    ~Dx12RenderSystem();
    virtual bool engineInit(bool raytracing);

    virtual Ogre::OgreTexture* createTextureFromFile(const std::string& name, Ogre::TextureProperty* texProperty) override;

    virtual void traceRay(Handle<HwRaytracingProgram> programHandle);

    virtual void bindPipeline(
        Handle<HwRaytracingProgram> programHandle,
        const Handle<HwDescriptorSet>* descSets,
        uint32_t setCount
    );
    virtual Handle<HwRaytracingProgram> createRaytracingProgram(
        const RaytracingShaderInfo& mShaderInfo) override;

    virtual Handle<HwDescriptorSet> createDescriptorSet(
        Handle<HwRaytracingProgram> programHandle,
        uint32_t set) override;

    virtual void addAccelerationStructure(
        const AccelerationStructureDesc* pDesc,
        AccelerationStructure** ppAccelerationStructure) override;

    virtual void buildAccelerationStructure(RaytracingBuildASDesc* pDesc)  override;

    virtual void removeAccelerationStructureScratch(
        AccelerationStructure* pAccelerationStructure)  override;

    virtual uint64_t getBufferDeviceAddress(Handle<HwBufferObject> bufHandle) override;
private:
    DxDescriptorID getBufferDescriptorID(Handle<HwBufferObject> bufHandle);
private:
    ID3D12Device5* prDevice = nullptr;
    ID3D12GraphicsCommandList5* m_dxrCommandList = nullptr;
};