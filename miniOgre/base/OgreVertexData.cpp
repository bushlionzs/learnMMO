#include "OgreHeader.h"
#include "OgreVertexData.h"
#include "OgreHardwareBufferManager.h"
#include "renderSystem.h"
#include "OgreVertexDeclaration.h"
#include "OgreRoot.h"
#include "OgreMemoryBuffer.h"



void VertexSlotInfo::createBuffer(uint32_t vertexSize, uint32_t vertexCount)
{
    mVertexSize  = vertexSize;
    mVertexCount = vertexCount;
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();

    uint32_t bufferCreationFlags = 0;

    if (Ogre::Root::getSingleton().getEngineConfig().enableRaytracing)
    {
        bufferCreationFlags = BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT | 
            BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS;
    }
    mVertexBufferHandle = rs->createBufferObject(
        BufferObjectBinding::BufferObjectBinding_Storge | BufferObjectBinding::BufferObjectBinding_Vertex,
        bufferCreationFlags,
        vertexSize * vertexCount,
        "VertexBuffer");
}

void VertexSlotInfo::writeData(const char* data, uint32_t size)
{
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();

    rs->updateBufferObject(mVertexBufferHandle, data, size);
}

BufferHandleLockGuard::BufferHandleLockGuard(Handle<HwBufferObject> bufferHandle)
{
    mBufferHandle = bufferHandle;
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();

    mBufferData = rs->lockBuffer(mBufferHandle, 0, UINT32_MAX);
}

void* BufferHandleLockGuard::data()
{
    return mBufferData;
}

BufferHandleLockGuard::~BufferHandleLockGuard()
{
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();
    rs->unlockBuffer(mBufferHandle);
}

VertexData::VertexData()
{
    vertexDeclaration = new VertexDeclaration;
}

VertexData::~VertexData()
{

}

void VertexData::bind(void* cb)
{
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();

    for (auto i = 0; i < vertexSlotInfo.size(); i++)
    {
        auto& slot = vertexSlotInfo[i];
        if (slot.mVertexSize > 0)
        {
            rs->bindVertexBuffer(slot.mVertexBufferHandle, i);
        }
    }
}

Handle<HwBufferObject> VertexData::getBuffer(int32_t index)
{
    VertexSlotInfo* slot = vertexSlotInfo.data();
    if (slot[index].mVertexSize > 0)
    {
        return slot[index].getHandle();
    }
    assert(false);
    return Handle<HwBufferObject>();
}

uint32_t VertexData::getBufferCount()
{
    uint32_t count = 0;

    for (auto& slot : vertexSlotInfo)
    {
        if (slot.mVertexSize > 0)
        {
            count++;
        }
    }
    return count;
}

void VertexData::setVertexCount(uint32_t vertexCount)
{
    mVertexCount = vertexCount;
}

uint32_t VertexData::getVertexCount() const
{
    return mVertexCount;
}

uint32_t VertexData::getVertexSize(uint32_t binding) const
{
    return vertexSlotInfo[binding].mVertexSize;
}

void VertexData::removeAllElements()
{
    vertexDeclaration->removeAllElements();
}

const VertexElement& VertexData::addElement(
    int16_t source,
    int16_t index,
    int32_t offset,
    VertexElementType theType,
    VertexElementSemantic semantic)
{
    return vertexDeclaration->addElement(source, index, offset, theType, semantic);
}

int32_t VertexData::addBindBuffer(uint32_t vertexSize, uint32_t vertexCount)
{
    auto binding = getUnusedBinding();
    assert(binding >= 0);
    addBindBuffer(binding, vertexSize, vertexCount);
    return binding;
}

void VertexData::addBindBuffer(uint32_t binding, uint32_t vertexSize, uint32_t vertexCount)
{
    vertexSlotInfo[binding].createBuffer(vertexSize, vertexCount);
}

void VertexData::updateBindBuffer(uint32_t binding, uint32_t vertexCount)
{
    vertexSlotInfo[binding].createBuffer(binding, vertexCount);
}

void VertexData::addBoneInfo(std::vector<VertexBoneAssignment>& assignInfoList)
{
    uint32_t binding = 0;

    for (auto i = 0; i < vertexSlotInfo.size(); i++)
    {
        if (vertexSlotInfo[i].mVertexSize == 0)
        {
            binding = i;
            break;
        }
    }

    auto& slot = vertexSlotInfo[binding];

    slot.mVertexSize = sizeof(SkinnedData);
    slot.createBuffer(sizeof(SkinnedData), mVertexCount);

    vertexSlotInfo[binding].createBuffer(sizeof(SkinnedData), mVertexCount);

    BufferHandleLockGuard lockGuard(slot.getHandle());
    SkinnedData* skin = (SkinnedData*)lockGuard.data();
    std::vector<uint32_t> vertexIndex(mVertexCount);

    SkinnedData tmp;
    tmp.boneWeight[0] = 0.0f;
    tmp.boneWeight[1] = 0.0f;
    tmp.boneWeight[2] = 0.0f;
    tmp.boneWeight[3] = 0.0f;
    tmp.boneIndices[0] = 0;
    tmp.boneIndices[1] = 0;
    tmp.boneIndices[2] = 0;
    tmp.boneIndices[3] = 0;
    for (int32_t i = 0; i < mVertexCount; i++)
    {
        memcpy(&skin[i], &tmp, sizeof(SkinnedData));
    }

    for (auto& assign : assignInfoList)
    {
        int32_t i = assign.vertexIndex;
        int32_t wix = vertexIndex[i];
        skin[i].boneWeight[wix] = assign.weight;
        skin[i].boneIndices[wix] = assign.boneIndex;
        vertexIndex[i]++;
    }

    vertexDeclaration->addElement(binding, 0, 0, VET_FLOAT4, VES_BLEND_WEIGHTS);
    vertexDeclaration->addElement(binding, 0, 16, VET_UINT4, VES_BLEND_INDICES);

}

void VertexData::writeBindBufferData(uint32_t slot, const char* data, uint32_t size, bool writeGPU)
{
    vertexSlotInfo[slot].writeData(data, size);
}

int32_t VertexData::getUnusedBinding()
{
    uint32_t binding = 0;
    for (uint32_t i = 0; i < vertexSlotInfo.size(); i++)
    {
        if (vertexSlotInfo[i].mVertexSize == 0)
        {
            return i;
        }
    }
    return -1;
}



