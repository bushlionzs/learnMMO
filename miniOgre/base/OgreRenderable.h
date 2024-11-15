#pragma once


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
    using RenderableFrameResourceCallback = std::function< bool(
        std::vector<FrameResourceInfo>& frameResourceInfoList, Material* mat)>;
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

        bool createFrameResource(RenderableFrameResourceCallback callback = nullptr);
        void updateFrameResource(uint32_t frameIndex);
        void updateMaterialInfo(bool updateTexture);
        FrameResourceInfo* getFrameResourceInfo(uint32_t frameIndex);

        void setObjectType(ObjectType type);
    protected:
        std::shared_ptr<Material> mMaterial;
        Ogre::Matrix4 mModel;
        uint64_t mSortValue;
        ObjectType mObjectType;
        std::vector<FrameResourceInfo> mFrameResourceInfoList;

        RenderableFrameResourceCallback mCallback = nullptr;
    };
}