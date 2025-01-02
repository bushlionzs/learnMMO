#include "OgreHeader.h"
#include "OgreRenderable.h"
#include "renderSystem.h"
#include "OgreRoot.h"
#include <OgreCamera.h>
#include <OgreVertexData.h>
#include <OgreIndexData.h>
#include <OgreMaterial.h>
#include "OgreTextureUnit.h"
#include <OgreTextureManager.h>

namespace Ogre {
    Renderable::Renderable():
        mObjectType(ObjectType_Static)
    {
        mModel = Ogre::Matrix4::IDENTITY;

        mSortValue = 1000000;

        mColor = Ogre::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
        mFlags = 0;
    }

    Renderable::~Renderable()
    {
    }

    void Renderable::setMaterial(std::shared_ptr<Material> mat)
    {
        mMaterial = mat;

        VertexData* vd = getVertexData();
        VertexDeclaration* decl = vd->getVertexDeclaration();
        mMaterial->updateVertexDeclaration(decl);
    }

    uint64_t Renderable::getSortValue()
    {
        return 0;
    }

    const Ogre::Matrix4& Renderable::getModelMatrix()
    {
        return mModel;
    }

    void Renderable::updateModelMatrix(const Ogre::Matrix4& m)
    {
        mModel = m;
    }

    Ogre::Vector3 getPosition()
    {
        return Ogre::Vector3(0.0f, 0.0f, 0.0f);
    }

    Ogre::OperationType Renderable::getPrimitiveTopology()
    {
        return OT_TRIANGLE_LIST;
    }

    bool Renderable::updateFrameResource(
        uint32_t frameIndex,
        void* frameData)
    {
        if (mFrameResourceInfoList.empty())
        {
            auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
            mFrameResourceInfoList.resize(ogreConfig.swapBufferCount);
        }
        
        assert(frameIndex < mFrameResourceInfoList.size());
        mFrameResourceInfoList[frameIndex] = frameData;
        return true;
    }

    void* Renderable::getFrameResourceInfo(uint32_t frameIndex)
    {
        if (frameIndex >= mFrameResourceInfoList.size())
        {
            return nullptr;
        }
        return mFrameResourceInfoList[frameIndex];
    }

    void Renderable::setObjectType(ObjectType type)
    {
        mObjectType = type;
    }

    void Renderable::setColor(const Ogre::Vector4& color)
    {
        mColor = color;
    }

    const Ogre::Vector4& Renderable::getColor() const 
    {
        return mColor;
    }

    bool Renderable::hasFlag(uint32_t bit) const
    {
        return mFlags.test(bit);
    }

    void Renderable::setFlag(uint32_t bit, bool val)
    {
        mFlags.set(bit, val);
    }
}