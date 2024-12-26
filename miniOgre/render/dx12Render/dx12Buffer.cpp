#include "OgreHeader.h"
#include "dx12Buffer.h"
#include "dx12RenderSystem.h"
#include "d3dutil.h"
#include "UploadBuffer.h"
#include "dx12Helper.h"

Dx12Buffer::Dx12Buffer(
    HardwareBuffer::BufferType btype, 
    size_t vertexSize, 
    size_t numVerts, 
    uint32_t bufferCreationFlags)
{
    mBufferType = btype;
    mVertexSize = vertexSize;
    mNumVerts = numVerts;
    
    size_t byteSize = vertexSize * numVerts;

    mMemBuffer.resize(byteSize);
    
    if (btype == HardwareBuffer::INDEX_BUFFER)
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
}

Dx12Buffer::~Dx12Buffer()
{

}

void* Dx12Buffer::lockimpl(size_t offset, size_t length, HardwareBuffer::LockOptions options)
{
    return mMemBuffer.data();
}

void Dx12Buffer::unlock()
{
    
}

void Dx12Buffer::bind(int32_t slot, void* cb)
{
    assert(false);
    //ID3D12GraphicsCommandList* cmdList = DX12Helper::getSingleton().getCurrentCommandList();

    //if (mBufferType == HardwareBuffer::VERTEX_BUFFER)
    //{
    //    D3D12_VERTEX_BUFFER_VIEW vbv;
    //    //vbv.BufferLocation = mDX12BufferObject.getGPUVirtualAddress();
    //    vbv.StrideInBytes = mVertexSize;
    //    vbv.SizeInBytes = mVertexSize * mNumVerts;
    //    cmdList->IASetVertexBuffers(slot, 1, &vbv);
    //}
    //else if (mBufferType == HardwareBuffer::INDEX_BUFFER)
    //{
    //    D3D12_INDEX_BUFFER_VIEW ibv;
    //   // ibv.BufferLocation = mDX12BufferObject.getGPUVirtualAddress();
    //    ibv.Format = mIndexFormat;
    //    ibv.SizeInBytes = mVertexSize * mNumVerts;
    //    cmdList->IASetIndexBuffer(&ibv);
    //}

}

void Dx12Buffer::bind(ID3D12GraphicsCommandList* cmdList, int32_t slot)
{
    D3D12_VERTEX_BUFFER_VIEW vbv;
    //vbv.BufferLocation = mDX12BufferObject.getGPUVirtualAddress();
    vbv.StrideInBytes = mVertexSize;
    vbv.SizeInBytes = mVertexSize * mNumVerts;
    cmdList->IASetVertexBuffers(slot, 1, &vbv);
}