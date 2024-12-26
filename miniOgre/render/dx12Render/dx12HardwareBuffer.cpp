#include "OgreHeader.h"
#include "dx12HardwareBuffer.h"
#include "dx12RenderSystem.h"
#include "d3dutil.h"
#include "UploadBuffer.h"
#include "dx12Helper.h"

Dx12HardwareBuffer::Dx12HardwareBuffer(BufferType btype, size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage)
{
    mBufferType = btype;
    mVertexSize = vertexSize;
    mNumVerts = numVerts;
    
    size_t byteSize = vertexSize * numVerts;

    mMemBuffer.resize(byteSize);
    ID3D12Device* dx12Device = DX12Helper::getSingleton().getDevice();

    if (btype == INDEX_BUFFER)
    {
        if (vertexSize == 4)
        {
            mIndexFormat = DXGI_FORMAT_R32_UINT;
        }
        else
        {
            mIndexFormat = DXGI_FORMAT_R16_UINT;
        }
    }
    
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    ThrowIfFailed(dx12Device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(BufferGPU.GetAddressOf())));
    heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    ThrowIfFailed(dx12Device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(BufferUploader.GetAddressOf())));
}

Dx12HardwareBuffer::~Dx12HardwareBuffer()
{

}

void* Dx12HardwareBuffer::lockimpl(size_t offset, size_t length, LockOptions options)
{
    return mMemBuffer.data();
}

void Dx12HardwareBuffer::unlock()
{
    assert(false);
}

void Dx12HardwareBuffer::_copydataimpl(
    ID3D12GraphicsCommandList* cmdList, void* data, uint32_t size)
{
    D3D12_SUBRESOURCE_DATA subResourceData = {};
    subResourceData.pData = data;
    subResourceData.RowPitch = size;
    subResourceData.SlicePitch = subResourceData.RowPitch;

    // Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
    // will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
    // the intermediate upload heap data will be copied to mBuffer.
    auto dstBarrier = CD3DX12_RESOURCE_BARRIER::Transition(BufferGPU.Get(),
        D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
    cmdList->ResourceBarrier(1, &dstBarrier);
    UpdateSubresources<1>(cmdList, BufferGPU.Get(), BufferUploader.Get(), 0, 0, 1, &subResourceData);
    dstBarrier = CD3DX12_RESOURCE_BARRIER::Transition(BufferGPU.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
    cmdList->ResourceBarrier(1, &dstBarrier);
}

void Dx12HardwareBuffer::bind(int32_t slot, void* cb) const
{
    assert(false);
    /*ID3D12GraphicsCommandList* cmdList = DX12Helper::getSingleton().getCurrentCommandList();

    if (mBufferType == VERTEX_BUFFER)
    {
        D3D12_VERTEX_BUFFER_VIEW vbv;
        vbv.BufferLocation = BufferGPU->GetGPUVirtualAddress();
        vbv.StrideInBytes = mVertexSize;
        vbv.SizeInBytes = mVertexSize * mNumVerts;
        cmdList->IASetVertexBuffers(slot, 1, &vbv);
    }
    else if (mBufferType == INDEX_BUFFER)
    {
        D3D12_INDEX_BUFFER_VIEW ibv;
        ibv.BufferLocation = BufferGPU->GetGPUVirtualAddress();
        ibv.Format = mIndexFormat;
        ibv.SizeInBytes = mVertexSize * mNumVerts;
        cmdList->IASetIndexBuffer(&ibv);
    }*/

}

void Dx12HardwareBuffer::bind(ID3D12GraphicsCommandList* cmdList, int32_t slot)
{
    D3D12_VERTEX_BUFFER_VIEW vbv;
    vbv.BufferLocation = BufferGPU->GetGPUVirtualAddress();
    vbv.StrideInBytes = mVertexSize;
    vbv.SizeInBytes = mVertexSize * mNumVerts;
    cmdList->IASetVertexBuffers(slot, 1, &vbv);
}