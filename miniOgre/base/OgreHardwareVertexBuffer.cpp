#include "OgreHeader.h"
#include "OgreHardwareVertexBuffer.h"


    uint32_t VertexElement::getSize() const
    {
        return getTypeSize(mType);
    }

    uint32_t VertexElement::getLocation() const
    {
        switch (mSemantic)
        {
        case VES_POSITION:
            return 0;
        case VES_BLEND_WEIGHTS:
            return 6;
        case VES_BLEND_INDICES:
            return 5;
        case VES_TEXTURE_COORDINATES:
            return 3;
        case VES_DIFFUSE:
            return 2;
        case VES_NORMAL:
            return 1;
        default:
            assert(false);
            return 0;
        }
    }

    uint32_t VertexElement::getTypeSize(VertexElementType etype)
    {
        switch (etype)
        {
        case VET_FLOAT1:
            return sizeof(float);
        case VET_FLOAT2:
            return sizeof(float) * 2;
        case VET_FLOAT3:
            return sizeof(float) * 3;
        case VET_FLOAT4:
            return sizeof(float) * 4;
        case VET_DOUBLE1:
            return sizeof(double);
        case VET_DOUBLE2:
            return sizeof(double) * 2;
        case VET_DOUBLE3:
            return sizeof(double) * 3;
        case VET_DOUBLE4:
            return sizeof(double) * 4;
        case VET_SHORT1:
        case VET_USHORT1:
            return sizeof(short);
        case VET_SHORT2:
        case VET_SHORT2_NORM:
        case VET_USHORT2:
        case VET_USHORT2_NORM:
            return sizeof(short) * 2;
        case VET_SHORT3:
        case VET_USHORT3:
            return sizeof(short) * 3;
        case VET_SHORT4:
        case VET_SHORT4_NORM:
        case VET_USHORT4:
        case VET_USHORT4_NORM:
            return sizeof(short) * 4;
        case VET_INT1:
        case VET_UINT1:
            return sizeof(int);
        case VET_INT2:
        case VET_UINT2:
            return sizeof(int) * 2;
        case VET_INT3:
        case VET_UINT3:
            return sizeof(int) * 3;
        case VET_INT4:
        case VET_UINT4:
            return sizeof(int) * 4;
        case VET_BYTE4:
        case VET_BYTE4_NORM:
        case VET_UBYTE4:
        case VET_UBYTE4_NORM:
        case _DETAIL_SWAP_RB:
            return sizeof(char) * 4;
        case VET_INT_10_10_10_2_NORM:
            return 4;
        }
        return 0;
    }

    uint32_t VertexElement::convertColourValue(const Ogre::ColourValue& src,
        VertexElementType dst)
    {

        if (dst == VET_COLOUR_ABGR)
        {
            return src.getAsABGR();
        }
        else
        {
            return src.getAsARGB();
        }
    }
namespace Ogre {
    HardwareVertexBuffer::HardwareVertexBuffer(size_t vertexSize, size_t numVerts,
        HardwareBuffer* delegate)
    {
        mVertexSize = vertexSize;
        mNumVerts = numVerts;
        mSizeInBytes = vertexSize * numVerts;
        mDelegate.reset(delegate);
    }

    HardwareVertexBuffer::~HardwareVertexBuffer()
    {

    }

    void* HardwareVertexBuffer::lockimpl(size_t offset, size_t length, LockOptions options)
    {
        return mDelegate->lock(options);
    }
    void HardwareVertexBuffer::unlock()
    {
        mDelegate->unlock();
    }

    void HardwareVertexBuffer::bind(int32_t slot, void* cb)
    {
        mDelegate->bind(slot, cb);
    }
}