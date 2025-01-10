#include <OgreHeader.h>
#include <OgreResourceManager.h>
#include "dx12Handles.h"
#include "dx12Helper.h"
#include "dx12Shader.h"
#include "memoryAllocator.h"
#include "glslUtil.h"
#include "myutils.h"
#include "d3dutil.h"
#include "shaderManager.h"

DX12BufferObject::DX12BufferObject(
    DescriptorHeapContext* context,
    BufferDesc& desc,
    DxDescriptorID id
   )
{
    mDescriptorHeapContext = context;
    mBufferObjectBinding = desc.mBindingType;
    mMemoryUsage = desc.mMemoryUsage;
    mDescriptorID = id;
    mByteCount = desc.mSize;

    if (BufferObjectBinding_Uniform == mBufferObjectBinding)
    {
        mByteCount = d3dUtil::CalcConstantBufferByteSize(mByteCount);
    }
    
    ID3D12Device* dx12Device = DX12Helper::getSingleton().getDevice();
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(mByteCount);

    D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;

    if (BufferObjectBinding_Storge == mBufferObjectBinding)
    {
        state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
    ThrowIfFailed(dx12Device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        state,
        nullptr,
        IID_PPV_ARGS(BufferGPU.GetAddressOf())));

    auto cpuHandle = descriptor_id_to_cpu_handle(
        context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], id);

    switch (mBufferObjectBinding)
    {
    case BufferObjectBinding_Vertex:
    case BufferObjectBinding_Index:
    case BufferObjectBinding_Buffer:
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = desc.mElementCount;
        srvDesc.Buffer.StructureByteStride = desc.mStructStride;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        if (desc.raw)
        {
            srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            srvDesc.Buffer.NumElements = mByteCount / 4;
            srvDesc.Buffer.StructureByteStride = 0;
            srvDesc.Buffer.Flags |= D3D12_BUFFER_SRV_FLAG_RAW;
        }
        dx12Device->CreateShaderResourceView(BufferGPU.Get(), &srvDesc, cpuHandle);
    }
    break;
    case BufferObjectBinding_Storge:
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.NumElements = desc.mElementCount;
        uavDesc.Buffer.StructureByteStride = desc.mStructStride;
        uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
        if (desc.raw)
        {
            uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            uavDesc.Buffer.NumElements = mByteCount / 4;
            uavDesc.Buffer.StructureByteStride = 0;
            uavDesc.Buffer.Flags |= D3D12_BUFFER_UAV_FLAG_RAW;
        }
        dx12Device->CreateUnorderedAccessView(BufferGPU.Get(), nullptr, &uavDesc, cpuHandle);
        int kk = 0;
    }
        break;
    case BufferObjectBinding_Uniform:
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = BufferGPU->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (UINT)mByteCount;
        dx12Device->CreateConstantBufferView(&cbvDesc, cpuHandle);
    }
    break;
    case BufferObjectBinding_AccelerationStructure:
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.RaytracingAccelerationStructure.Location = BufferGPU->GetGPUVirtualAddress();
        dx12Device->CreateShaderResourceView(BufferGPU.Get(), &srvDesc, cpuHandle);
    }
    break;
    default:
        assert(false);
        break;
    }

    heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(mByteCount);
    ThrowIfFailed(dx12Device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(BufferUploader.GetAddressOf())));
}

void DX12BufferObject::copyData(
    ID3D12GraphicsCommandList* cmdList,
    const char* data, 
    uint32_t size)
{
    void* mapData = lock(0, size);
    memcpy(mapData, data, size);
    unlock(cmdList);

    
}

D3D12_GPU_VIRTUAL_ADDRESS DX12BufferObject::getGPUVirtualAddress()
{    
    return  BufferGPU->GetGPUVirtualAddress();
}

void* DX12BufferObject::lock(uint32_t offset, uint32_t numBytes)
{
    BYTE* mappedData = nullptr;
    mRange.Begin = offset;
    mRange.End = offset + numBytes;

    if (mRange.End > mByteCount)
        mRange.End = mByteCount;
    BufferUploader->Map(0, &mRange, reinterpret_cast<void**>(&mappedData));
    return mappedData;
}

void DX12BufferObject::unlock(ID3D12GraphicsCommandList* cmdList)
{
    BufferUploader->Unmap(0, &mRange);
    uint32_t size = mRange.End - mRange.Begin;
    auto dstBarrier = CD3DX12_RESOURCE_BARRIER::Transition(BufferGPU.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1, &dstBarrier);

    cmdList->CopyBufferRegion(
        BufferGPU.Get(), 0, BufferUploader.Get(), mRange.Begin, size);
    dstBarrier = CD3DX12_RESOURCE_BARRIER::Transition(BufferGPU.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    cmdList->ResourceBarrier(1, &dstBarrier);
}

DX12Program::DX12Program(const ShaderInfo& info, VertexDeclaration* decl)
{
    mProgramImpl = new DX12ProgramImpl(info, decl);
}

DX12RayTracingProgram::DX12RayTracingProgram(const RaytracingShaderInfo& shaderInfo)
{

}

DX12ComputeProgram::DX12ComputeProgram(const ShaderInfo& info)
{
    mProgramImpl = new DX12ProgramImpl(info, nullptr);
}

DX12ComputeProgram::~DX12ComputeProgram()
{

}


DX12DescriptorSet::DX12DescriptorSet(DX12ProgramBase* program, uint32_t set)
    :mProgram(program),
    mSet(set)
{

}

DX12DescriptorSet::~DX12DescriptorSet()
{

}