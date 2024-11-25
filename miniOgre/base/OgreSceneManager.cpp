#include "OgreHeader.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "renderSystem.h"
#include "OgremeshManager.h"
#include "OgreEntity.h"
#include "OgreMaterialManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleSystem.h"
#include "OgreRoot.h"
#include "OgreMoveObject.h"
#include "OgreSky.h"
#include "OgreRenderable.h"
#include "myutils.h"
#include "OgreViewport.h"
#include "OgreLight.h"
#include "OgreSceneQuery.h"

namespace Ogre {

    class  DefaultRaySceneQuery : public RaySceneQuery
    {
    public:
        DefaultRaySceneQuery(SceneManager* creator):RaySceneQuery(creator)
        {

        }
        ~DefaultRaySceneQuery()
        {

        }

        /** See RayScenQuery. */
        void execute(RaySceneQueryListener* listener)
        {
            auto itor = mParentSceneMgr->getMoveObjectIterator();

            auto end = mParentSceneMgr->getMoveObjectIteratorEnd();

            for (; itor != end; itor++)
            {
                Ogre::MoveObject* a = itor->second;
                const std::string& name = a->getName();

                auto pos = name.find("ам");
                if (pos != std::string::npos)
                {
                    int kk = 0;
                }
                if (!(a->getTypeFlags() & mQueryTypeMask))
                    break;

                if ((a->getQueryFlags() & mQueryMask) &&
                    a->isVisible())
                {
                    

                    // Do ray / box test
                    std::pair<bool, Real> result = mRay.intersects(a->getWorldBoundingBox());

                    if (result.first)
                    {
                        if (!listener->queryResult(a, result.second)) return;
                    }
                }
            }
          
        }
    };

    SceneManager::SceneManager()
    {
        mRenderSystem = Ogre::Root::getSingleton().getRenderSystem();

        mRoot = new SceneNode(this, std::string("__root"));

        mAmbientLight = ColourValue(0.45f, 0.45f, 0.45f, 1.0f);
    }

    SceneManager::~SceneManager()
    {

    }

    Camera* SceneManager::createCamera(const std::string& name)
    {
        auto itor = mCameraMap.find(name);
        if (itor != mCameraMap.end())
        {
            return nullptr;
        }

        Camera* camera = new Camera(name, this);
        mCameraMap[name] = camera;
        return camera;
    }

    Camera* SceneManager::getCamera(const std::string& name)
    {
        auto itor = mCameraMap.find(name);
        if (itor != mCameraMap.end())
        {
            return itor->second;
        }

        return nullptr;
    }
 
    void SceneManager::getSceneRenderList(
        ICamera* camera, 
        EngineRenderList& renderList,
        bool shadow)
    {
        renderList.mOpaqueList.clear();
        renderList.mTransparentList.clear();


        mRoot->traverse(renderList, camera, shadow);

        std::sort(mEngineRenderList.mOpaqueList.begin(), mEngineRenderList.mOpaqueList.end(),
            [](Ogre::Renderable* a, Ogre::Renderable* b)
            {
                return a->getSortValue() < b->getSortValue();
            }
        );

    }

    void SceneManager::update(float timeSinceLastFrame)
    {
        mRoot->update(timeSinceLastFrame);
    }

    Entity* SceneManager::createEntity(const std::string& name, const std::string& meshName, bool suppressSkeletalAnimation)
    {
        NameValuePairList params;
        params["mesh"] = meshName;
        if (suppressSkeletalAnimation)
            params["suppressSkeletalAnimation"] = "true";

        Entity* entity = static_cast<Entity*>(
            createMovableObject(name, EntityFactory::FACTORY_TYPE_NAME,
                &params));

        return entity;
    }

    MoveObject* SceneManager::createMovableObject(const String& name,
        const String& typeName, const NameValuePairList* params)
    {

        Ogre::MovableObjectFactory* factory =
            Ogre::Root::getSingleton().getMovableObjectFactory(typeName);
        // Check for duplicate names

        auto itor = mMoveObjectMap.find(name);
        if (itor != mMoveObjectMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "duplicate name");
        }
        MoveObject* newObj = factory->createInstance(name, this, params);
        if (factory->getType() == EntityFactory::FACTORY_TYPE_NAME)
        {
            mMoveObjectMap[name] = newObj;
        }
        
        return newObj;
    }

    SceneNode* SceneManager::getRoot()
    {
        return mRoot;
    }

    SceneNode* SceneManager::createSceneNode(const String& name)
    {
        auto itor = mNamedNodes.find(name);
        if (itor != mNamedNodes.end())
        {
            assert(false);
        }
        auto node = OGRE_NEW SceneNode(this, name);
        mNamedNodes[name] = node;
        return node;
    }

    SceneNode* SceneManager::getSceneNode(const String& name)
    {
        auto itor = mNamedNodes.find(name);
        if (itor != mNamedNodes.end())
        {
            return itor->second;
        }
        return nullptr;
    }

    Ogre::ParticleSystem* SceneManager::createParticleSystem(const String& name,
        const String& templateName)
    {
        NameValuePairList params;
        params["templateName"] = templateName;

        return static_cast<Ogre::ParticleSystem*>(
            createMovableObject(name, Ogre::ParticleSystemFactory::FACTORY_TYPE_NAME,
                &params));
    }

    

    bool SceneManager::hasMovableObject(const String& name,
        const String& typeName)
    {
        auto itor = mMoveObjectMap.find(name);
        if (itor != mMoveObjectMap.end())
        {
            return true;
        }
        return false;
    }

    void SceneManager::destroySceneNode(SceneNode* sn)
    {
        String& name = sn->getName();

        auto itor = mNamedNodes.find(name);
        if (itor != mNamedNodes.end())
        {
            delete sn;
            mNamedNodes.erase(itor);
        }
        else
        {
            delete sn;
        }
    }

    SceneNode* SceneManager::createSceneNode()
    {
        return nullptr;
    }
    RaySceneQuery*
        SceneManager::createRayQuery(const Ray& ray, uint32 mask)
    {
        DefaultRaySceneQuery* q = OGRE_NEW DefaultRaySceneQuery(this);
        q->setRay(ray);
        q->setQueryMask(mask);
        return q;
    }

    void SceneManager::destroyMovableObject(const String& name, const String& typeName)
    {

    }
    void SceneManager::destroyMovableObject(MoveObject* m)
    {

    }

    RibbonTrail* SceneManager::createRibbonTrail(const String& name)
    {
        return nullptr;
    }

    void SceneManager::destroyRibbonTrail(const String& name)
    {

    }

    Light* SceneManager::createLight(const String& name)
    {
        auto itor = mLightMap.find(name);
        if (itor != mLightMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "light alreay exist");
            WARNING_LOG("light %s alreay exist", name.c_str());
        }

        Light* l = new Light;
        mLightMap[name] = l;

        mLightList.clear();

        for (auto& pair : mLightMap)
        {
            mLightList.push_back(pair.second);
        }

        std::sort(mLightList.begin(), mLightList.end(),
            [](Light* a, Light* b)
        {
            return a->getLightNumber() < b->getLightNumber();
        });
        return l;
    }

    void SceneManager::destroyLight(const String& name)
    {
        
    }

    void SceneManager::setAmbientLight(const ColourValue& colour)
    {
        mAmbientLight = colour;
    }

    const std::vector<Light*>& SceneManager::getLightList()
    {
        return mLightList;
    }

    const ColourValue& SceneManager::getAmbientLight()
    {
        return mAmbientLight;
    }

    void SceneManager::destroyBillboardSet(MoveObject* set)
    {

    }

    void SceneManager::destroyBillboardSet(const String& name)
    {

    }

    BillboardSet* SceneManager::createBillboardSet(const String& name, unsigned int poolSize)
    {
        return nullptr;
    }


    FogMode SceneManager::getFogMode(void) const
    {
        return FOG_NONE;
    }

    const ColourValue& SceneManager::getFogColour(void) const
    {
        return Ogre::ColourValue::Black;
    }
    Real SceneManager::getFogStart(void) const
    {
        return 0.0f;
    }
    Real SceneManager::getFogEnd(void) const
    {
        return 10000.f;
    }

    void SceneManager::destroyEntity(MoveObject* ent)
    {

    }

    /// @overload
    void SceneManager::destroyEntity(const String& name)
    {

    }

    void SceneManager::destroyCamera(Camera* cam)
    {

    }

 
    void SceneManager::destroyCamera(const String& name)
    {

    }

    void SceneManager::removeBillboardSet(const String& name)
    {

    }

    void SceneManager::destroyStaticGeometry(const String& name)
    {

    }

    void SceneManager::destroyParticleSystem(MoveObject* obj)
    {

    }
    /// @overload
    void SceneManager::destroyParticleSystem(const String& name) 
    {

    }

    void SceneManager::setSkyBox(
        bool enable, const String& materialName, Real distance,
        bool drawFirst, const Quaternion& orientation)
    {
        _setSkyBox(enable, materialName, distance,orientation);
    }

    /// @overload
    void SceneManager::_setSkyBox(
        bool enable, const String& materialName, Real distance,
        const Quaternion& orientation)
    {
        mSkyBox = std::make_shared<SkyBoxRenderer>(this);
        mSkyBox->setSkyBox(enable, materialName, distance, 0, orientation, BLANKSTRING);
    }

    void SceneManager::setSkyLayer(int layerIndex, const String& meshName, 
        bool drawFirst,
        const Vector3& offset,
        const Vector3& scale,
        const Quaternion& orientation,
        float uScroll,
        float vScroll,
        bool keepDistance,
        bool keepHeight,
        bool keepOrient,
        bool affectByFog)
    {

    }

    void SceneManager::clearScene()
    {

    }

    void SceneManager::destroyAllCameras()
    {

    }

    void SceneManager::destroyAllEntities()
    {

    }
}
