#include <OgreHeader.h>
#include "dx12Handles.h"
#include "dx12Helper.h"

DX12BufferObject::DX12BufferObject(
    DxMemoryAllocator* allocator,
    BufferObjectBinding bufferObjectBinding,
    ResourceMemoryUsage memoryUsage,
    uint32_t bufferCreationFlags,
    uint32_t byteCount
   )
{
    mAllocator = allocator;
    mBufferObjectBinding = bufferObjectBinding;
    mMemoryUsage = memoryUsage;
    ID3D12Device* dx12Device = DX12Helper::getSingleton().getDevice();
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteCount);
    ThrowIfFailed(dx12Device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(BufferGPU.GetAddressOf())));
    heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteCount);
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
    /*auto dstBarrier = CD3DX12_RESOURCE_BARRIER::Transition(BufferGPU.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1, &dstBarrier);*/
    
    cmdList->CopyBufferRegion(BufferGPU.Get(), 0, BufferUploader.Get(), 0, size);
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
    BufferUploader->Map(0, &mRange, reinterpret_cast<void**>(&mappedData));
    return mappedData;
}

void DX12BufferObject::unlock(ID3D12GraphicsCommandList* cmdList)
{
    BufferUploader->Unmap(0, &mRange);

    cmdList->CopyBufferRegion(
        BufferGPU.Get(), 0, BufferUploader.Get(), mRange.Begin, mRange.End - mRange.End);
 
}
