#pragma once

#include "OgreCommon.h"
#include "OgreSceneNode.h"
#include "OgreIndexData.h"
class VertexData;
class IndexData;
class VertexDeclaration;


namespace Ogre {
    class SubMesh
    {
        friend class Mesh;
    public:
        SubMesh(
            Mesh* mesh,
            bool sharedVertices = false,
            bool sharedIndex = false);
        ~SubMesh();

        VertexData* getVertexData();
        IndexData* getIndexData();
        Ogre::OperationType getOperationType();
        void setOperationType(Ogre::OperationType ot);

        void addIndexs(
            uint32_t drawCount, 
            uint32_t indexLocation, 
            uint32_t baseVertexLocation);

        IndexDataView* getIndexView();

        void setMaterial(std::shared_ptr<Material>& mat);
        std::shared_ptr<Material>& getMaterial();

        void setMaterialName(std::string& name);
        const std::string& getMaterialName();

        VertexDeclaration* getVertexDeclaration()
        {
            return mVetexDeclaration;
        }

        void addBoneAssignment(const VertexBoneAssignment& vertBoneAssign);
        void prepare();

        void setPosition(Ogre::Vector3& pos);
        void setScale(Ogre::Vector3& scale);
        void setRotate(Ogre::Quaternion& q);
        const Ogre::Vector3& getPosition();
        const Ogre::Vector3& getScale();
        const Ogre::Quaternion& getRotate();

        bool useSharedVertices()
        {
            return mSharedVertices;
        }

        Mesh* getParent()
        {
            return mParent;
        }

        void updateBuffer(VertexBuffer* vb, IndexBuffer* ib)
        {
            mVertexBuffer = vb;
            mIndexBuffer = ib;
        }

        VertexBuffer* getVertexBuffer()
        {
            return mVertexBuffer;
        }

        IndexBuffer* getIndexBuffer()
        {
            return mIndexBuffer;
        }

    private:
        VertexDeclaration* mVetexDeclaration;
        VertexData* mVertexData;
        IndexData* mIndexData;
        Mesh* mParent = nullptr;

        IndexDataView mIndexView;

        std::shared_ptr<Material> mMaterial;
        std::string mMatName;

        OperationType mOperationType = OT_TRIANGLE_LIST;

        Ogre::Vector3 mPosition;
        Ogre::Vector3 mScale;
        Ogre::Quaternion mQuaternion;

        bool mSharedVertices = false;
        bool mSharedIndex = false;

        VertexBuffer* mVertexBuffer = nullptr;
        IndexBuffer* mIndexBuffer = nullptr;

        std::vector<VertexBoneAssignment> mBoneAssignments;
    };
}