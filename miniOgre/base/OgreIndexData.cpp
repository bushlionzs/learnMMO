#include "OgreHeader.h"
#include "OgreIndexData.h"
#include "OgreHardwareBuffer.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareIndexBuffer.h"
#include "renderSystem.h"
#include "OgreRoot.h"
#include "OgreMemoryBuffer.h"

IndexData::IndexData()
{

}

IndexData::~IndexData()
{

}

void IndexData::bind()
{
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();
    rs->bindIndexBuffer(mIndexBufferHandle, mIndexSize);
}

void IndexData::createBuffer(uint32_t indexSize, uint32_t indexCount)
{
    mIndexSize = indexSize;
    mIndexCount = indexCount;
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();

    uint32_t bufferCreationFlags = 0;

    if (Ogre::Root::getSingleton().getEngineConfig().enableRaytracing)
    {
        bufferCreationFlags = BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT |
            BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS;
    }

    mIndexBufferHandle = rs->createBufferObject(
        backend::BufferObjectBinding_Storge | backend::BufferObjectBinding_Index,
        bufferCreationFlags,
        indexSize * indexCount,
        "IndexBuffer");
    
}

void IndexData::writeData(const char* data, uint32_t size)
{
    auto* rs = Ogre::Root::getSingleton().getRenderSystem();
    rs->updateBufferObject(mIndexBufferHandle, data, size);
}
