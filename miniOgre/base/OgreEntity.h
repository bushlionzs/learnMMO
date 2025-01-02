#pragma once

#include "OgreMoveObject.h"

class VertexData;
class IndexData;
class RawData;

namespace Ogre {
    class AnimationView
    {
    public:
        Animation* mCurrentAnimation;
    };
    class TagPoint;
    class Entity : public MoveObject
    {
    public:
        typedef std::vector<MoveObject*> ChildObjectList;
        typedef std::set<Entity*> EntitySet;
        Entity();
        Entity(const std::string& name, const std::shared_ptr<Mesh>& mesh);
        ~Entity();

        virtual const std::vector<Renderable*>& getRenderableList();
        virtual bool isTransparent() const;

        bool setMaterial(uint32_t index, std::shared_ptr<Material>& mat);
        void setTransparent(float alpha);
        void setMirror(bool mirror);

        virtual void update(float delta);
        RawData* getSkinnedData(uint32_t dummy);
        VertexData* getVertexData();
        IndexData* getIndexData();

        Ogre::Vector3 getPosition();

        AnimationStateSet* getAllAnimationStates(void) const;

        AnimationState* getAnimationState(const String& name);

        const AxisAlignedBox& getBoundingBox(void) const;

        void setNormaliseNormals(bool normalise)
        {
            mNormaliseNormals = normalise;
        }

        TagPoint* attachObjectToBone(const String& boneName,
            MoveObject* pMovable,
            const Quaternion& offsetOrientation = Quaternion::IDENTITY,
            const Vector3& offsetPosition = Vector3::ZERO);

        int32_t getNumSubEntities() const;

        SubEntity* getSubEntity(int32_t index) const;

        bool hasSkeleton() const;

        SkeletonInstance* getSkeleton(void) const;

        Real getBoundingRadius(void) const;
        const MeshPtr& getMesh(void) const
        {
            return mMesh;
        }

        void setMaterialName(const std::string& name, bool copy = true);

        void setColor(const Ogre::Vector4& color);

        const ChildObjectList& getAttachedObjects() const;

        void shareSkeletonInstanceWith(Entity* entity);

        void refreshAvailableAnimationState(void);
    private:
        SubEntity* createSubEntity(SubMesh* sub);
        virtual void buildSubEntityList(std::shared_ptr<Mesh>& mesh, std::vector<Renderable*>* sublist);
    protected:
        std::vector<Renderable*> mSubEntityList;
        std::shared_ptr<Mesh> mMesh;
        bool mTransparent = false;
        bool mMirror = false;

        SubEntity* mReflectTarget = nullptr;

        std::shared_ptr<SkeletonInstance> mSkeletonInstance;

        bool mNormaliseNormals;

        mutable AxisAlignedBox mAABB;

        EntitySet* mSharedSkeletonEntities;

        AnimationStateSet* mAnimationState;

        uint64_t mFrameLastUpdated = 0;
    };

    class  EntityFactory : public MovableObjectFactory
    {
    private:
        MoveObject* createInstanceImpl(const String& name, const NameValuePairList* params);
    public:
        EntityFactory() {}
        ~EntityFactory() {}

        static String FACTORY_TYPE_NAME;

        const String& getType(void) const;
    };
    /** @} */
    /** @} */

}