#pragma once

#include <bitset>
#include "engine_struct.h"
#include "shader.h"
#include <filament/Handle.h>

class RenderSystem;
class VertexData;
class IndexData;
class IndexDataView;
class RenderableData;


using namespace filament;

namespace Ogre {
    class Renderable
    {
    public:
        Renderable();
        ~Renderable();
        virtual bool buildRenderable(RenderSystem* engine, SceneNode* node) { return false; }
        virtual void render(RenderSystem* engine) {}
        virtual void addTexture(OgreTexture* tex) {}
        void setMaterial(std::shared_ptr<Material> mat);

        virtual const std::shared_ptr<Material>& getMaterial()
        {
            return mMaterial;
        }

        virtual VertexData* getVertexData()
        {
            return nullptr;
        }

        virtual IndexData* getIndexData()
        {
            return nullptr;
        }

        virtual IndexDataView* getIndexView()
        {
            return nullptr;
        }

        virtual RawDataView* getRawDataView()
        {
            return nullptr;
        }

        virtual RawData* getSkinnedData(uint32_t dummy = 0)
        {
            return nullptr;
        }

        virtual const Ogre::Matrix4& getProjectorMatrix()
        {
            return Ogre::Matrix4::IDENTITY;
        }

        virtual uint64_t getSortValue();

        virtual const Ogre::Matrix4& getModelMatrix();
        void updateModelMatrix(const Ogre::Matrix4& m);

        Ogre::OperationType getPrimitiveTopology();

        bool updateFrameResource(
            uint32_t frameIndex, 
            void* frameData);
        void* getFrameResourceInfo(uint32_t frameIndex);

        void setObjectType(ObjectType type);
        ObjectType getObjectType()
        {
            return mObjectType;
        }

        void setColor(const Ogre::Vector4& color);
        const Ogre::Vector4& getColor() const;

        bool hasFlag(uint32_t pos) const;
        void setFlag(uint32_t pos, bool val);
    protected:
        std::shared_ptr<Material> mMaterial;
        Ogre::Matrix4 mModel;
        uint64_t mSortValue;
        ObjectType mObjectType;
        Ogre::Vector4 mColor;
        std::vector<void*> mFrameResourceInfoList;
        std::bitset<64>  mFlags;
    };
}