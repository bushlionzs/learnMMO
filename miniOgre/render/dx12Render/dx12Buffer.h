#pragma once
#include <OgreHardwareBuffer.h>
#include "dx12Common.h"
#include "dx12Handles.h"

class Dx12RenderSystem;
class GraphicsCommandList;

class Dx12Buffer
{
public:
    Dx12Buffer(
        HardwareBuffer::BufferType btype,
        size_t vertexSize, 
        size_t numVerts, 
        uint32_t bufferCreationFlags);
    virtual ~Dx12Buffer();

    virtual void* lockimpl(size_t offset, size_t length, HardwareBuffer::LockOptions options);
    virtual void unlock();

    virtual void bind(int32_t slot, void* cb);

    void bind(ID3D12GraphicsCommandList* cmdList, int32_t slot);
private:
    int32_t mVertexSize;
    int32_t mNumVerts;
    HardwareBuffer::BufferType mBufferType;

    std::string mMemBuffer;
    DXGI_FORMAT mIndexFormat;
    
};