#include "OgreHeader.h"
#include <string_util.h>
#include "dx12RenderSystem.h"
#include "OgreMoveObject.h"
#include "OgreMaterial.h"
#include "dx12RayTracingShader.h"
#include "dx12Handles.h"
#include "dx12Texture.h"
#include "OgreRoot.h"
#include "dx12RenderTarget.h"
#include "dx12RenderWindow.h"
#include "dx12Helper.h"
#include "dx12Commands.h"
#include "memoryAllocator.h"
#include "D3D12Mappings.h"



Dx12RenderSystem::Dx12RenderSystem(HWND wnd)
{
	mRenderSystemName = "Directx12";
}


Dx12RenderSystem::~Dx12RenderSystem()
{

}

bool Dx12RenderSystem::engineInit(bool raytracing)
{
	Dx12RenderSystemBase::engineInit();

    mDevice->QueryInterface(IID_PPV_ARGS(&prDevice));
	return true;
}

OgreTexture* Dx12RenderSystem::createTextureFromFile(const std::string& name, TextureProperty* texProperty)
{
	Dx12Texture* tex = new Dx12Texture(
		name, texProperty, mCommands, true);

	if (!tex->load(nullptr))
	{
		delete tex;
		return nullptr;
	}

	return tex;
}

void Dx12RenderSystem::traceRay(Handle<HwRaytracingProgram> programHandle)
{
    DX12RayTracingProgram* program = mResourceAllocator.handle_cast<DX12RayTracingProgram*>(programHandle);
    DX12RayTracingProgramImpl* impl = program->getProgramImpl();

    D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
    ID3D12GraphicsCommandList* cl = mCommands->get();
    
    cl->QueryInterface(IID_PPV_ARGS(&m_dxrCommandList));
    
    
    ID3D12Resource* hitGroupShaderTable = impl->getHitGroupShaderTable();
    ID3D12Resource* missShaderTable = impl->getMissShaderTable();
    ID3D12Resource* rayGenShaderTable = impl->getRayGenShaderTable();
    ID3D12StateObject* stateObject = impl->getDxrStateObject();
    dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable->GetGPUVirtualAddress();
    dispatchDesc.HitGroupTable.SizeInBytes = hitGroupShaderTable->GetDesc().Width;
    dispatchDesc.HitGroupTable.StrideInBytes = dispatchDesc.HitGroupTable.SizeInBytes;
    dispatchDesc.MissShaderTable.StartAddress = missShaderTable->GetGPUVirtualAddress();
    dispatchDesc.MissShaderTable.SizeInBytes = missShaderTable->GetDesc().Width;
    dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
    dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable->GetGPUVirtualAddress();
    dispatchDesc.RayGenerationShaderRecord.SizeInBytes = rayGenShaderTable->GetDesc().Width;

    auto width = mRenderWindow->getWidth();
    auto height = mRenderWindow->getHeight();
    dispatchDesc.Width = width;
    dispatchDesc.Height = height;
    dispatchDesc.Depth = 1;
    m_dxrCommandList->SetPipelineState1(stateObject);
    m_dxrCommandList->DispatchRays(&dispatchDesc);
}

void Dx12RenderSystem::bindPipeline(
    Handle<HwRaytracingProgram> programHandle,
    const Handle<HwDescriptorSet>* descSets,
    uint32_t setCount
)
{
    ID3D12GraphicsCommandList* cl = mCommands->get();
    DX12RayTracingProgram* program = mResourceAllocator.handle_cast<DX12RayTracingProgram*>(programHandle);
    DX12RayTracingProgramImpl* impl = program->getProgramImpl();
    ID3D12RootSignature* rootSignature = impl->getRootSignature();
    cl->SetComputeRootSignature(rootSignature);

    for (uint32_t i = 0; i < setCount; i++)
    {
        if (!descSets[i])
            continue;
        DX12DescriptorSet* dset = mResourceAllocator.handle_cast<DX12DescriptorSet*>(descSets[i]);
        std::vector<const DescriptorInfo*> descriptorInfos = dset->getDescriptorInfos();
        auto cbvSrvUavHandle = dset->getCbvSrvUavHandle();
        auto samplerHandle = dset->getSamplerHandle();
        for (auto descriptorInfo : descriptorInfos)
        {
            if (descriptorInfo->mType == D3D_SIT_SAMPLER)
            {
                auto gpuHandle = descriptor_id_to_gpu_handle(
                    mDescriptorHeapContext->pSamplerHeaps[0], samplerHandle + descriptorInfo->mSetIndex);
                cl->SetComputeRootDescriptorTable(descriptorInfo->mRootIndex, gpuHandle);
            }
            else
            {
              
                auto gpuHandle = descriptor_id_to_gpu_handle(
                    mDescriptorHeapContext->mCbvSrvUavHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], cbvSrvUavHandle + descriptorInfo->mSetIndex);
                cl->SetComputeRootDescriptorTable(descriptorInfo->mRootIndex, gpuHandle);
                int kk = 0;
            }
        }
    }
}

Handle<HwRaytracingProgram> Dx12RenderSystem::createRaytracingProgram(
	const RaytracingShaderInfo& shaderInfo)
{
	Handle<HwRaytracingProgram> programHandle = mResourceAllocator.allocHandle<DX12RayTracingProgram>();

	DX12RayTracingProgram* program = mResourceAllocator.construct<DX12RayTracingProgram>(programHandle,
		shaderInfo);
    DX12RayTracingProgramImpl* impl = program->getProgramImpl();
    
    impl->createRaytracingPipelineStateObject(prDevice, shaderInfo);
    impl->buildShaderTables(shaderInfo);
	return programHandle;
}

Handle<HwDescriptorSet> Dx12RenderSystem::createDescriptorSet(
	Handle<HwRaytracingProgram> programHandle,
	uint32_t set)
{
	Handle<HwDescriptorSet> dsh = mResourceAllocator.allocHandle<DX12DescriptorSet>();
	DX12RayTracingProgram* program = mResourceAllocator.handle_cast<DX12RayTracingProgram*>(programHandle);

	DX12RayTracingProgramImpl* progIml = program->getProgramImpl();
	DX12DescriptorSet* dx12DescSet = mResourceAllocator.construct<DX12DescriptorSet>(
		dsh, (DX12ProgramBase*)progIml, set);
	uint32_t cbvSrvUavDescCount = progIml->getCbvSrvUavDescCount(set);

	DxDescriptorID cbvSrvUavHandle = consume_descriptor_handles(mDescriptorHeapContext->mCbvSrvUavHeaps[0], cbvSrvUavDescCount);
	dx12DescSet->updateCbvSrvUavHandle(cbvSrvUavHandle, cbvSrvUavDescCount);

	uint32_t samplerCount = progIml->getSamplerCount(set);

	if (samplerCount > 0)
	{
		DxDescriptorID samplerHandle = consume_descriptor_handles(mDescriptorHeapContext->pSamplerHeaps[0], samplerCount);
		dx12DescSet->updateSamplerHandle(samplerHandle, samplerCount);
	}
	return dsh;
}

void Dx12RenderSystem::addAccelerationStructure(
	const AccelerationStructureDesc* pDesc,
	AccelerationStructure** ppAccelerationStructure)
{
	size_t memSize = sizeof(DX12AccelerationStructure);
	if (ACCELERATION_STRUCTURE_TYPE_BOTTOM == pDesc->mType)
	{
		memSize += pDesc->mBottom.mDescCount * sizeof(D3D12_RAYTRACING_GEOMETRY_DESC);
	}

	DX12AccelerationStructure* pAS = (DX12AccelerationStructure*)malloc(memSize);
    memset(pAS, 0, memSize);
	pAS->mFlags = D3D12Mappings::util_to_dx_acceleration_structure_build_flags(pDesc->mFlags);
	pAS->mType = D3D12Mappings::ToDXRASType(pDesc->mType);

    uint32_t scratchBufferSize = 0;

    if (ACCELERATION_STRUCTURE_TYPE_BOTTOM == pDesc->mType)
    {
        pAS->mDescCount = pDesc->mBottom.mDescCount;
        pAS->pGeometryDescs = (D3D12_RAYTRACING_GEOMETRY_DESC*)(pAS + 1); //-V1027
        for (uint32_t j = 0; j < pAS->mDescCount; ++j)
        {
            AccelerationStructureGeometryDesc* pGeom = &pDesc->mBottom.pGeometryDescs[j];
            D3D12_RAYTRACING_GEOMETRY_DESC* pGeomD3D12 = &pAS->pGeometryDescs[j];

            pGeomD3D12->Flags = D3D12Mappings::util_to_dx_geometry_flags(pGeom->mFlags);
            pGeomD3D12->Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
            if (pGeom->mIndexCount)
            {
                ASSERT(pGeom->indexBufferHandle);
                pGeomD3D12->Triangles.IndexBuffer = getBufferDeviceAddress(pGeom->indexBufferHandle) + pGeom->mIndexOffset;
                pGeomD3D12->Triangles.IndexCount = pGeom->mIndexCount;
                pGeomD3D12->Triangles.IndexFormat = (pGeom->mIndexType == INDEX_TYPE_UINT16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
            }

            ASSERT(pGeom->vertexBufferHandle);
            ASSERT(pGeom->mVertexCount);

            pGeomD3D12->Triangles.VertexBuffer.StartAddress = getBufferDeviceAddress(pGeom->vertexBufferHandle) + pGeom->mVertexOffset;
            pGeomD3D12->Triangles.VertexBuffer.StrideInBytes = pGeom->mVertexStride;
            pGeomD3D12->Triangles.VertexCount = pGeom->mVertexCount;
            pGeomD3D12->Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
            pGeomD3D12->Triangles.Transform3x4 = 0;
        }
        /************************************************************************/
        // Get the size requirement for the Acceleration Structures
        /************************************************************************/
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc = {};
        prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        prebuildDesc.Flags = pAS->mFlags;
        prebuildDesc.NumDescs = pAS->mDescCount;
        prebuildDesc.pGeometryDescs = pAS->pGeometryDescs;
        prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
        prDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &info);

        /************************************************************************/
        // Allocate Acceleration Structure Buffer
        /************************************************************************/
        BufferDesc bufferDesc = {};
        bufferDesc.mBindingType = BufferObjectBinding_AccelerationStructure;
        bufferDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        bufferDesc.bufferCreationFlags = BUFFER_CREATION_FLAG_OWN_MEMORY_BIT | 
            BUFFER_CREATION_FLAG_NO_DESCRIPTOR_VIEW_CREATION;
        bufferDesc.mStructStride = 0;
        bufferDesc.mFirstElement = 0;
        bufferDesc.mElementCount = (uint32_t)(info.ResultDataMaxSizeInBytes / sizeof(UINT32));
        bufferDesc.mSize = info.ResultDataMaxSizeInBytes;
        bufferDesc.mStartState = RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE;
        bufferDesc.raw = true;
        pAS->asBufferHandle = this->createBufferObject(bufferDesc);
        /************************************************************************/
        // Store the scratch buffer size so user can create the scratch buffer accordingly
        /************************************************************************/
        scratchBufferSize = (UINT)info.ScratchDataSizeInBytes > scratchBufferSize ? (UINT)info.ScratchDataSizeInBytes : scratchBufferSize;
    }
    else
    {
        pAS->mDescCount = pDesc->mTop.mDescCount;
        /************************************************************************/
        // Get the size requirement for the Acceleration Structures
        /************************************************************************/
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc = {};
        prebuildDesc.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        prebuildDesc.Flags = D3D12Mappings::util_to_dx_acceleration_structure_build_flags(pDesc->mFlags);
        prebuildDesc.NumDescs = pDesc->mTop.mDescCount;
        prebuildDesc.pGeometryDescs = NULL;
        prebuildDesc.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
        prDevice->GetRaytracingAccelerationStructurePrebuildInfo(&prebuildDesc, &info);

        /************************************************************************/
        /*  Construct buffer with instances descriptions                        */
        /************************************************************************/

        std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs;
        instanceDescs.resize(pDesc->mTop.mDescCount);
        for (uint32_t i = 0; i < pDesc->mTop.mDescCount; ++i)
        {
            AccelerationStructureInstanceDesc* pInst = &pDesc->mTop.pInstanceDescs[i];
            ASSERT(pInst->pBottomAS);
            DX12AccelerationStructure* bottomAS = (DX12AccelerationStructure*)pInst->pBottomAS;
            
            instanceDescs[i].AccelerationStructure = getBufferDeviceAddress(bottomAS->asBufferHandle);
            instanceDescs[i].Flags = D3D12Mappings::util_to_dx_instance_flags(pInst->mFlags);
            instanceDescs[i].InstanceContributionToHitGroupIndex = pInst->mInstanceContributionToHitGroupIndex;
            instanceDescs[i].InstanceID = pInst->mInstanceID;
            instanceDescs[i].InstanceMask = pInst->mInstanceMask;

            memcpy(instanceDescs[i].Transform, pInst->mTransform, sizeof(float[12])); //-V595
        }
        uint32_t instanceSize = pDesc->mTop.mDescCount * sizeof(instanceDescs[0]);
        BufferDesc instanceDesc = {};
        instanceDesc.mBindingType = BufferObjectBinding_Storge;
        instanceDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        instanceDesc.bufferCreationFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
        instanceDesc.mSize = instanceSize;
        instanceDesc.raw = true;
        pAS->instanceDescBuffer = createBufferObject(instanceDesc);
        updateBufferObject(pAS->instanceDescBuffer, (const char*)instanceDescs.data(), instanceSize);
        
        /************************************************************************/
        // Allocate Acceleration Structure Buffer
        /************************************************************************/
        BufferDesc bufferDesc = {};
        bufferDesc.mBindingType = BufferObjectBinding_AccelerationStructure;
        bufferDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        bufferDesc.bufferCreationFlags = BUFFER_CREATION_FLAG_OWN_MEMORY_BIT;
        bufferDesc.mStructStride = 0;
        bufferDesc.mFirstElement = 0;
        bufferDesc.mElementCount = (uint32_t)(info.ResultDataMaxSizeInBytes / sizeof(UINT32));
        bufferDesc.mSize = info.ResultDataMaxSizeInBytes;
        bufferDesc.mStartState = RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE;
        bufferDesc.raw = true;
        pAS->asBufferHandle = createBufferObject(bufferDesc);

        scratchBufferSize = (UINT)info.ScratchDataSizeInBytes;
    }

    // Create scratch buffer
    BufferDesc scratchBufferDesc = {};
    scratchBufferDesc.mBindingType = BufferObjectBinding_Storge;
    scratchBufferDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    scratchBufferDesc.mStartState = RESOURCE_STATE_COMMON;
    scratchBufferDesc.bufferCreationFlags = BUFFER_CREATION_FLAG_NO_DESCRIPTOR_VIEW_CREATION;
    scratchBufferDesc.mSize = scratchBufferSize;
    scratchBufferDesc.raw = true;
    pAS->scratchBufferHandle = createBufferObject(scratchBufferDesc);
    *ppAccelerationStructure = pAS;
}

void Dx12RenderSystem::buildAccelerationStructure(RaytracingBuildASDesc* pDesc)
{
    DX12AccelerationStructure* as = (DX12AccelerationStructure*)pDesc->pAccelerationStructure;

    const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE type = as->mType;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
    buildDesc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    buildDesc.Inputs.Type = type;
    buildDesc.DestAccelerationStructureData = getBufferDeviceAddress(as->asBufferHandle);
    buildDesc.Inputs.Flags = as->mFlags;
    buildDesc.Inputs.pGeometryDescs = NULL;

    if (type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL)
    {
        buildDesc.Inputs.pGeometryDescs = as->pGeometryDescs;
    }
    else if (type == D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL)
    {
        buildDesc.Inputs.InstanceDescs = getBufferDeviceAddress(as->instanceDescBuffer);
    }

    buildDesc.Inputs.NumDescs = as->mDescCount;
    buildDesc.ScratchAccelerationStructureData = getBufferDeviceAddress(as->scratchBufferHandle);

    ID3D12GraphicsCommandList* cl = (ID3D12GraphicsCommandList*)mCommands->get();

    ID3D12GraphicsCommandList4* dxrCmd = NULL;
    cl->QueryInterface(IID_PPV_ARGS(&dxrCmd));
    ASSERT(dxrCmd);

    dxrCmd->BuildRaytracingAccelerationStructure(&buildDesc, 0, NULL);

    if (pDesc->mIssueRWBarrier)
    {
        BufferBarrier barrier = { as->asBufferHandle, RESOURCE_STATE_ACCELERATION_STRUCTURE_WRITE, RESOURCE_STATE_ACCELERATION_STRUCTURE_READ };
        resourceBarrier(1, &barrier, 0, NULL, 0, NULL);
    }

    dxrCmd->Release();
}

void Dx12RenderSystem::removeAccelerationStructureScratch(
	AccelerationStructure* pAccelerationStructure)
{

}

uint64_t Dx12RenderSystem::getBufferDeviceAddress(Handle<HwBufferObject> bufHandle)
{
    DX12BufferObject* bo = mResourceAllocator.handle_cast<DX12BufferObject*>(bufHandle);
    return bo->getGPUVirtualAddress();
}

DxDescriptorID Dx12RenderSystem::getBufferDescriptorID(Handle<HwBufferObject> bufHandle)
{
    DX12BufferObject* bo = mResourceAllocator.handle_cast<DX12BufferObject*>(bufHandle);
    return bo->getGPUVirtualAddress();
}

