#pragma once

#include <memory>
#include "OgreCommon.h"
#include "OgreHardwareBuffer.h"
#include "OgreColourValue.h"



class VertexElement
{
public:
    VertexElement(int16_t source, int16_t index, int32_t offset, VertexElementType type, VertexElementSemantic semantic)
    {
        mSource         = source;
        mIndex          = index;
        mOffset         = offset;
        mType           = type;
        mSemantic       = semantic;
    }

    VertexElementSemantic  getSemantic() const
    {
        return mSemantic;
    }

    int16_t getSource() const
    {
        return mSource;
    }

    int16_t getIndex() const
    {
        return mIndex;
    }

    int16_t getOffset() const
    {
        return mOffset;
    }

    VertexElementType getType() const
    {
        return mType;
    }

    uint32_t getSize(void) const;

    inline void baseVertexPointerToElement(void* pBase, float** pElem) const
    {
        *pElem = static_cast<float*>(
            static_cast<void*>(
                static_cast<unsigned char*>(pBase) + mOffset));
    }

    template<typename T>
    void baseVertexPointerToElement(void* pBase, T** pElem) const
    {
        // The only way we can do this is to cast to char* in order to use byte offset
        // then cast back to T*.
        *pElem = reinterpret_cast<T*>(static_cast<char*>(pBase) + mOffset);
    }

    uint32_t getLocation() const;

    static uint32_t getTypeSize(VertexElementType etype);

    static uint32_t convertColourValue(const Ogre::ColourValue& src,
        VertexElementType dst);
private:
    int32_t mOffset;
    int16_t mSource;
    int16_t mIndex;
    VertexElementType mType;
    VertexElementSemantic mSemantic;
};


namespace Ogre {

    class HardwareVertexBuffer : public HardwareBuffer
    {
    public:
        HardwareVertexBuffer(size_t vertexSize, size_t numVerts,
            HardwareBuffer* delegate);
        ~HardwareVertexBuffer();


        virtual void* lockimpl(size_t offset, size_t length, LockOptions options);
        virtual void unlock();
        virtual void bind(int32_t slot, void* cb);

        

        int32_t getVertexSize()
        {
            return mVertexSize;
        }

        HardwareBuffer* getHardwareBuffer()
        {
            return mDelegate.get();
        }
    private:
        std::unique_ptr<HardwareBuffer> mDelegate;
        
    };
}