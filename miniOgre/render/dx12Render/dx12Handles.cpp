#include <OgreHeader.h>
#include <OgreResourceManager.h>
#include "dx12Handles.h"
#include "dx12Helper.h"
#include "dx12Shader.h"
#include "memoryAllocator.h"
#include "glslUtil.h"
#include "myutils.h"
#include "shaderManager.h"

DX12BufferObject::DX12BufferObject(
    DescriptorHeapContext* context,
    BufferObjectBinding bufferObjectBinding,
    ResourceMemoryUsage memoryUsage,
    uint32_t bufferCreationFlags,
    uint32_t byteCount,
    DxDescriptorID id
   )
{
    mDescriptorHeapContext = context;
    mBufferObjectBinding = bufferObjectBinding;
    mMemoryUsage = memoryUsage;
    mDescriptorID = id;
    mByteCount = d3dUtil::CalcConstantBufferByteSize(byteCount);
    ID3D12Device* dx12Device = DX12Helper::getSingleton().getDevice();
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(mByteCount);
    ThrowIfFailed(dx12Device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(BufferGPU.GetAddressOf())));

    auto cpuHandle = descriptor_id_to_cpu_handle(
        context->mCPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], id);


    if (mByteCount < 65536)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = BufferGPU->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (UINT)mByteCount;
        dx12Device->CreateConstantBufferView(&cbvDesc, cpuHandle);
    }
    else
    {
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements = mByteCount;
        srvDesc.Buffer.StructureByteStride = 1;
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        dx12Device->CreateShaderResourceView(BufferGPU.Get(), &srvDesc, cpuHandle);
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

    auto dstBarrier = CD3DX12_RESOURCE_BARRIER::Transition(BufferGPU.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    cmdList->ResourceBarrier(1, &dstBarrier);
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
    cmdList->CopyBufferRegion(
        BufferGPU.Get(), 0, BufferUploader.Get(), mRange.Begin, size);
 
}

DX12Program::DX12Program(const ShaderInfo& info, VertexDeclaration* decl)
{
    mProgramImpl = new DX12ProgramImpl(info, decl);
}

DX12ComputeProgram::DX12ComputeProgram(const ShaderInfo& info)
{
    mProgramImpl = new DX12ProgramImpl(info, nullptr);
}

DX12ComputeProgram::~DX12ComputeProgram()
{

}


DX12DescriptorSet::DX12DescriptorSet(DX12ProgramImpl* program, uint32_t set)
    :mProgram(program),
    mSet(set)
{

}

DX12DescriptorSet::~DX12DescriptorSet()
{

}