#pragma once

#include "OgreSubMesh.h"
#include "OgreResource.h"
class HardwareBufferManager;
class VertexData;
class IndexData;
namespace Ogre {
    class Mesh: public Resource
    {
    public:
        Mesh(const std::string& name);
        ~Mesh();
        SubMesh* addSubMesh(bool shareVertices, bool shareIndex);

        int32_t getSubMeshCount();

        SubMesh* getSubMesh(int32_t index);

        bool hasSkeleton();
        std::shared_ptr<Skeleton>& getSkeleton();
        void applySkeleton(std::shared_ptr<Skeleton>& skeleton);
        void setSkeletonName(const std::string& skelName);
        void _initAnimationState(AnimationStateSet* animSet);
        void _refreshAnimationState(AnimationStateSet* animSet);
        VertexData* getVertexData();
        IndexData* getIndexData();

        void addBoneAssignment(const VertexBoneAssignment& vertBoneAssign);

        void prepare();
        const std::string& getName()
        {
            return mName;
        }

        virtual void loadImpl(void) {}
        
        virtual void unloadImpl(void) {}

        void _setBounds(const AxisAlignedBox& bounds, bool pad = true);
        const AxisAlignedBox& getBounds(void) const;

        const std::string& getSkeletonName() const;

        const SkeletonPtr& getSkeleton(void) const { return mSkeleton; }

    private:
        std::vector<SubMesh*> mSubMeshList;
        SkeletonPtr mSkeleton;
        std::string mSkelName;

        VertexData* mVertexData;
        IndexData* mIndexData;


        AxisAlignedBox mAABB;

        std::vector<VertexBoneAssignment> mBoneAssignments;
    };
}