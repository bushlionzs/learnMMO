#pragma once
#include <string>
#include <unordered_map>
#include "engine_struct.h"
#include "OgreColourValue.h"
#include "OgreCommon.h"

class RenderSystem;

namespace Ogre {
    class RaySceneQuery;
    class SkyBoxRenderer;
    class SceneManager
    {
    public:
        enum QueryTypeMask : uint32_t
        {
            /// Query type mask which will be used for world geometry @see SceneQuery
            WORLD_GEOMETRY_TYPE_MASK = 0x80000000,
            /// Query type mask which will be used for entities @see SceneQuery
            ENTITY_TYPE_MASK = 0x40000000,
            /// Query type mask which will be used for effects like billboardsets / particle systems @see SceneQuery
            FX_TYPE_MASK = 0x20000000,
            /// Query type mask which will be used for StaticGeometry  @see SceneQuery
            STATICGEOMETRY_TYPE_MASK = 0x10000000,
            /// Query type mask which will be used for lights  @see SceneQuery
            LIGHT_TYPE_MASK = 0x08000000,
            /// Query type mask which will be used for frusta and cameras @see SceneQuery
            FRUSTUM_TYPE_MASK = 0x04000000,
            /// User type mask limit
            USER_TYPE_MASK_LIMIT = FRUSTUM_TYPE_MASK
        };
    public:
        SceneManager();
        ~SceneManager();

        Camera* createCamera(const std::string& name);
        Camera* getCamera(const std::string& name);

        void getSceneRenderList(
            ICamera* camera, 
            EngineRenderList& renderList,
            bool shadow);
        void update(float timeSinceLastFrame);
        Entity* createEntity(const std::string& name, const std::string& meshName, bool suppressSkeletalAnimation = true);
        SceneNode* getRoot();
        SceneNode* createSceneNode(const String& name);
        
        SceneNode* getSceneNode(const String& name);
        Ogre::ParticleSystem* createParticleSystem(const String& name,
            const String& templateName);
        MoveObject* createMovableObject(const String& name,
            const String& typeName, const NameValuePairList* params);
        bool hasMovableObject(const String& name,
            const String& typeName);
        virtual void destroySceneNode(SceneNode* sn);


        SceneNode* createSceneNode();
        virtual RaySceneQuery*
            createRayQuery(const Ray& ray, uint32 mask = 0xFFFFFFFF);

        void destroyMovableObject(const String& name, const String& typeName);
        void destroyMovableObject(MoveObject* m);

        RibbonTrail* createRibbonTrail(const String& name);

        void destroyRibbonTrail(const String& name);

        Light* createLight(const String& name);
        void destroyLight(const String& name);
        const std::vector<Light*>& getLightList();
        const ColourValue& getAmbientLight();
        void setAmbientLight(const ColourValue& colour);

        void destroyBillboardSet(MoveObject* set);
        void destroyBillboardSet(const String& name);

        BillboardSet* createBillboardSet(const String& name, unsigned int poolSize = 20);


        FogMode getFogMode(void) const;
        const ColourValue& getFogColour(void) const;
        Real getFogStart(void) const;
        Real getFogEnd(void) const;

        void destroyEntity(MoveObject* ent);

        /// @overload
        void destroyEntity(const String& name);

        virtual void destroyCamera(Camera* cam);

        /** Removes a camera from the scene.
            @remarks
                This method removes an camera from the scene based on the
                camera's name rather than a pointer.
        */
        virtual void destroyCamera(const String& name);

        void removeBillboardSet(const String& name);

        void destroyStaticGeometry(const String& name);

        void destroyParticleSystem(MoveObject* obj);
        /// @overload
        void destroyParticleSystem(const String& name);

        void setSkyBox(
            bool enable, const String& materialName, Real distance = 5000,
            bool drawFirst = true, const Quaternion& orientation = Quaternion::IDENTITY);

        /// @overload
        void _setSkyBox(
            bool enable, const String& materialName, Real distance = 5000,
            const Quaternion& orientation = Quaternion::IDENTITY);

        void setSkyLayer(int layerIndex, const String& meshName, bool drawFirst = false,
            const Vector3& offset = Vector3(0.f, 0.f, 0.f),
            const Vector3& scale = Vector3(1.0f, 1.0f, 1.0f),
            const Quaternion& orientation = Quaternion::IDENTITY,
            float uScroll = 0.f,
            float vScroll = 0.f,
            bool keepDistance = true,
            bool keepHeight = true,
            bool keepOrient = false,
            bool affectByFog = false);

        void clearScene();
        void destroyAllCameras();
        void destroyAllEntities();
        Ogre::Viewport* getCurrentViewport()
        {
            return mCurrentViewport;
        }

        std::unordered_map<std::string, MoveObject*>::iterator getMoveObjectIterator()
        {
            return mMoveObjectMap.begin();
        }

        std::unordered_map<std::string, MoveObject*>::iterator getMoveObjectIteratorEnd()
        {
            return mMoveObjectMap.end();
        }

    private:
        std::unordered_map<std::string, Camera*> mCameraMap;
        std::unordered_map<String, SceneNode*> mNamedNodes;
        std::unordered_map<std::string, MoveObject*> mMoveObjectMap;
        RenderSystem* mRenderSystem = nullptr;
        SceneNode* mRoot = nullptr;
        EngineRenderList mEngineRenderList;
        std::shared_ptr<SkyBoxRenderer> mSkyBox;

        //light
        std::unordered_map<String, Light*> mLightMap;
        std::vector<Light*> mLightList;
        ColourValue mAmbientLight;
        /// Current Viewport
        Viewport* mCurrentViewport = nullptr;
    };
}