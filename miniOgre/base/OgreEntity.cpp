#include "OgreHeader.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreSubMesh.h"
#include "OgreMesh.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreHardwareBufferManager.h"
#include "renderSystem.h"
#include "OgreSkeletonInstance.h"
#include "OgreMaterial.h"
#include "OgreMeshManager.h"
#include "OgreAnimationState.h"
#include "OgreRoot.h"
#include "OgreMaterialManager.h"
#include "OgreVertexDeclaration.h"

namespace Ogre {
    Entity::Entity()
        :MoveObject(std::string(""))
    {
        mSharedSkeletonEntities = nullptr;
        mAnimationState = nullptr;
    }
    Entity::Entity(const std::string& name, const std::shared_ptr<Mesh>& mesh)
        :MoveObject(name)
    {
        mMesh = mesh;
        mSharedSkeletonEntities = nullptr;
        mAnimationState = nullptr;
        mAABB.setNull();
        if (mMesh)
            buildSubEntityList(mMesh, &mSubEntityList);
    }

    Entity::~Entity()
    {

    }

    const std::vector<Renderable*>& Entity::getRenderableList()
    {
        /*static std::vector<Renderable*> aa;
        aa.clear();
        for (int32_t i = 0; i < mSubEntityList.size(); i++)
        {
            if ( i == 7)
            aa.push_back(mSubEntityList[i]);
        }*/
        return mSubEntityList;
    }

    bool Entity::isTransparent() const
    {
        return mTransparent;
    }

    SubEntity* Entity::createSubEntity(SubMesh* sub)
    {
        return new SubEntity(this, sub);
    }

    bool Entity::setMaterial(uint32_t index, std::shared_ptr<Material>& mat)
    {
        if (index >= mSubEntityList.size())
        {
            return false;
        }

        mat->load(nullptr);
        mSubEntityList[index]->setMaterial(mat);

        return true;
    }

    void Entity::setTransparent(float alpha)
    {
        mTransparent = true;
    }

    void Entity::setMirror(bool mirror)
    {
        mMirror = mirror;
    }

    void Entity::update(float delta)
    {
        if (mSkeletonInstance)
        {
            mSkeletonInstance->setAnimationState(*mAnimationState);
        }
    }

    RawData* Entity::getSkinnedData(uint32_t dummy)
    {
        if (!mSkeletonInstance)
        {
            return nullptr;
        }

        return mSkeletonInstance->getSkinnedData(dummy);
    }

    VertexData* Entity::getVertexData()
    {
        return mMesh->getVertexData();
    }

    IndexData* Entity::getIndexData()
    {
        return mMesh->getIndexData();
    }

    Ogre::Vector3 Entity::getPosition()
    {
        return Ogre::Vector3(0.0f, 0.0f, 0.0f);
    }

    AnimationStateSet* Entity::getAllAnimationStates(void) const
    {
        return mAnimationState;
    }

    AnimationState* Entity::getAnimationState(const String& name)
    {
        if (!mAnimationState)
        {
            return nullptr;
        }
        return mAnimationState->getAnimationState(name);
    }

    const AxisAlignedBox& Entity::getBoundingBox(void) const
    {
        if (mMesh)
        {
            mAABB = mMesh->getBounds();
        }
        return mAABB;
    }


    TagPoint* Entity::attachObjectToBone(const String& boneName,
        MoveObject* pMovable,
        const Quaternion& offsetOrientation,
        const Vector3& offsetPosition)
    {
        return nullptr;
    }

    int32_t Entity::getNumSubEntities() const
    {
        return mSubEntityList.size();
    }

    SubEntity* Entity::getSubEntity(int32_t index) const
    {
        return (SubEntity*)mSubEntityList[index];
    }

    bool Entity::hasSkeleton() const
    {
        return !!mSkeletonInstance;
    }

    SkeletonInstance* Entity::getSkeleton(void) const
    {
        return mSkeletonInstance.get();
    }

    Real Entity::getBoundingRadius(void) const
    {
        return 0.0f;
    }


    void Entity::setMaterialName(const std::string& name, bool copy)
    {
        auto mat = MaterialManager::getSingleton().getByName(name);
        
        for (auto r : mSubEntityList)
        {
            if (copy)
            {
                std::shared_ptr<Material> copy = mat->clone(name);
                r->setMaterial(copy);
            }
            else
            {
                r->setMaterial(mat);
            }
        }
    }

    const Entity::ChildObjectList& Entity::getAttachedObjects() const
    {
        static Entity::ChildObjectList aa;
        return aa;
    }

    void Entity::shareSkeletonInstanceWith(Entity* entity)
    {
        if (mSharedSkeletonEntities != nullptr)
        {
            entity->shareSkeletonInstanceWith(this);
        }
        else
        {
            if (mAnimationState)
            {
                delete mAnimationState;
            }
            mSkeletonInstance = entity->mSkeletonInstance;

            if (entity->mSharedSkeletonEntities == nullptr)
            {
                entity->mSharedSkeletonEntities = OGRE_NEW_T(EntitySet, MEMCATEGORY_ANIMATION)();
                entity->mSharedSkeletonEntities->insert(entity);
            }
            mSharedSkeletonEntities = entity->mSharedSkeletonEntities;
            mSharedSkeletonEntities->insert(this);

            mAnimationState = entity->mAnimationState;
        }
    }

    void Entity::refreshAvailableAnimationState(void)
    {
        mMesh->_refreshAnimationState(mAnimationState);
    }

    backend::ElementType translateElementType(VertexElementType type)
    {
        switch (type)
        {
        case VET_FLOAT3:
            return backend::ElementType::FLOAT3;
        case VET_FLOAT2:
            return backend::ElementType::FLOAT2;
        }
        assert(false);

        return backend::ElementType::BYTE;
    }
    void Entity::buildSubEntityList(
        std::shared_ptr<Mesh>& mesh, 
        std::vector<Renderable*>* sublist)
    {
        int32_t numSubMeshes = mesh->getSubMeshCount();
        std::string newMatName;
        for (int32_t i = 0; i < numSubMeshes; ++i)
        {
            SubMesh* subMesh = mesh->getSubMesh(i);

            std::shared_ptr<Material>& mat = subMesh->getMaterial();
            newMatName = mName + "_" + mat->getName() + std::to_string(i);
            std::shared_ptr<Material> newMat = mat->clone(newMatName);

            SubEntity* subEnt = createSubEntity(subMesh);

            const Ogre::Vector3& position = subMesh->getPosition();
            const Ogre::Vector3& scale = subMesh->getScale();
            const Ogre::Quaternion& q = subMesh->getRotate();
            subEnt->setLocalMatrix(position, scale, q);
            subEnt->setMaterial(newMat);
            sublist->push_back(subEnt);
        }

        //skeleton

        if (mesh->hasSkeleton())
        {
            std::shared_ptr<Skeleton> skeleton = mesh->getSkeleton();
            mSkeletonInstance = std::make_shared<SkeletonInstance>(skeleton);

            mAnimationState = OGRE_NEW AnimationStateSet();
            mesh->_initAnimationState(mAnimationState);
        }
        else
        {
            int kk = 0;
        }
    }


    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String EntityFactory::FACTORY_TYPE_NAME = "Entity";
    //-----------------------------------------------------------------------
    const String& EntityFactory::getType(void) const
    {
        return FACTORY_TYPE_NAME;
    }
    //-----------------------------------------------------------------------
    MoveObject* EntityFactory::createInstanceImpl(const String& name,
        const NameValuePairList* params)
    {
        // must have mesh parameter
        MeshPtr pMesh;
        if (params != 0)
        {
            NameValuePairList::const_iterator ni;

            ni = params->find("mesh");
            if (ni != params->end())
            {
                // Get mesh (load if required)
                pMesh = MeshManager::getSingleton().load(ni->second);
            }

        }
        if (!pMesh)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "'mesh' parameter required when constructing an Entity.",
                "EntityFactory::createInstance");
        }

        return OGRE_NEW Entity(name, pMesh);

    }
}
