#include "OgreHeader.h"
#include "OgreRoot.h"
#include "OgreMoveObject.h"
#include "OgreSceneManager.h"
#include "OgreTextureManager.h"
#include "OgreMeshManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreMaterialManager.h"
#include "OgreSkeletonManager.h"
#include "OgreResourceManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreControllerManager.h"
#include "OgreParticleFx.h"
#include "renderSystem.h"
#include "ResourceParserManager.h"
#include "OgreFrameListener.h"
#include "dx11RenderSystem.h"
#include "dx12RenderSystem.h"
#include "VulkanRenderSystem.h"
#include "shaderManager.h"
#include "OgreEntity.h"

namespace Ogre {

	template<> Root* Ogre::Singleton<Root>::msSingleton = 0;

	Root::Root()
	{
        new ResourceParserManager;
        new ResourceManager;
        new TextureManager;
        new MeshManager;
        new MaterialManager;
        new SkeletonManager;
        new Ogre::ParticleSystemManager;
        new Ogre::ControllerManager;
        new ShaderManager;
        mRenderSystem = nullptr;
        mCurrentFrame = 0;

        mLastFrame = 0;
        mCurrentFPS = 0;
        mNextMovableObjectTypeFlag = 0;
        mLast = 0.0f;
        mAccumulation = 0.0f;

        mFrameCurrent = 0;
        mFrameLast = mTimer.getMicrosecondsCPU();

        mEvt.timeSinceLastEvent = 0;
        mEvt.timeSinceLastFrame = 0;
	}

	Root::~Root()
	{

	}

    void Root::_initialise()
    {
        OgreParticleFx_init();
        ParticleSystemManager::getSingletonPtr()->_initialise();


        addMovableObjectFactory(new EntityFactory());

    }

    RenderSystem* Root::createRenderEngine(
        HWND wnd,
        EngineType et)
    {
        if (et == EngineType_Dx12)
        {
            Dx12RenderSystem* engine = new Dx12RenderSystem(wnd);
            std::shared_ptr<RenderSystem> ans(engine);

            mRenderSystem = ans;
        }
        else if (et == EngineType_Vulkan)
        {
            VulkanRenderSystem* engine = new VulkanRenderSystem(wnd);
            std::shared_ptr<RenderSystem> ans(engine);

            mRenderSystem = ans;

        }
        else if (et == EngineType_Dx11)
        {
            Dx11RenderSystem* engine = new Dx11RenderSystem(wnd);
            std::shared_ptr<RenderSystem> ans(engine);

            mRenderSystem = ans;

        }
        else
        {
            assert(false);
            return nullptr;
        }

        mRenderSystem->engineInit(mEngineConfig.enableRaytracing);
        return mRenderSystem.get();
    }

    RenderSystem* Root::getRenderSystem()
    {
        return mRenderSystem.get();
    }

	void Root::addMovableObjectFactory(MovableObjectFactory* fact,
		bool overrideExisting)
	{
        auto& type = fact->getType();
        MovableObjectFactoryMap::iterator facti = mMovableObjectFactoryMap.find(type);
        if (!overrideExisting && facti != mMovableObjectFactoryMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                "A factory of type '" + fact->getType() + "' already exists.",
                "Root::addMovableObjectFactory");
        }

        if (fact->requestTypeFlags())
        {
            if (facti != mMovableObjectFactoryMap.end() && facti->second->requestTypeFlags())
            {
                // Copy type flags from the factory we're replacing
                fact->_notifyTypeFlags(facti->second->getTypeFlags());
            }
            else
            {
                // Allocate new
                fact->_notifyTypeFlags(_allocateNextMovableObjectTypeFlag());
            }
        }

        // Save
        mMovableObjectFactoryMap[fact->getType()] = fact;
	}

    void Root::removeMovableObjectFactory(MovableObjectFactory* fact)
    {
        MovableObjectFactoryMap::iterator i = mMovableObjectFactoryMap.find(
            fact->getType());
        if (i != mMovableObjectFactoryMap.end())
        {
            mMovableObjectFactoryMap.erase(i);
        }

    }

    MovableObjectFactory* Root::getMovableObjectFactory(const String& typeName)
    {
        MovableObjectFactoryMap::iterator i =
            mMovableObjectFactoryMap.find(typeName);
        if (i == mMovableObjectFactoryMap.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND,
                "MovableObjectFactory of type " + typeName + " does not exist",
                "Root::getMovableObjectFactory");
        }
        return i->second;
    }

    SceneManager* Root::createSceneManger(const std::string& name)
    {
        auto itor = mSceneManagerMap.find(name);
        if (itor != mSceneManagerMap.end())
        {
            assert(false);
        }
        SceneManager* current = new SceneManager;
        mSceneManagerMap[name] = current;
        return current;
    }

    void Root::destroySceneManager(SceneManager* sceneMgr)
    {

    }


    SceneManager* Root::getSceneManager(const std::string& name)
    {
        auto itor = mSceneManagerMap.find(name);

        if (itor != mSceneManagerMap.end())
        {
            return itor->second;
        }

        return nullptr;
    }

    uint64_t Root::getCurrentFrame()
    {
        return mCurrentFrame;
    }

    uint64_t Root::getCurrentFrameIndex()
    {
        return mCurrentFrame % mEngineConfig.swapBufferCount;
    }

    uint64_t Root::getCurrentFPS()
    {
        return mCurrentFPS;
    }

    bool Root::_fireFrameStarted()
    {
        mFrameCurrent = mTimer.getMicrosecondsCPU();

        float delta = (mFrameCurrent - mFrameLast) / 1000000.0f;

        mFrameLast = mFrameCurrent;
        _syncAddedRemovedFrameListeners();
        mCurrentFrame++;
        
        

        mAccumulation += delta;
        if (mAccumulation >= mLast + 1.0f)
        {
            mCurrentFPS = (mCurrentFrame - mLastFrame);

            mLastFrame = mCurrentFrame;
            mLast = mAccumulation;
        }

        
        mEvt.timeSinceLastEvent = 0.0f;
        mEvt.timeSinceLastFrame = delta;

        for (std::set<FrameListener*>::iterator i = mFrameListeners.begin(); i != mFrameListeners.end(); ++i)
        {
            if (mRemovedFrameListeners.find(*i) != mRemovedFrameListeners.end())
                continue;

            if (!(*i)->frameStarted(mEvt))
                return false;
        }

        ControllerManager::getSingleton().updateAllControllers();

        update(delta);
        
        return true;
    }

    void Root::update(float delta)
    {
        for (auto& pair : mSceneManagerMap)
        {
            pair.second->update(delta);
        }
    }

    bool Root::_fireFrameEnded()
    {
        return true;
    }

    void Root::addFrameListener(FrameListener* newListener)
    {
        mRemovedFrameListeners.erase(newListener);
        mAddedFrameListeners.insert(newListener);
    }

    void Root::removeFrameListener(FrameListener* oldListener)
    {
        mRemovedFrameListeners.insert(oldListener);
    }

    void Root::_syncAddedRemovedFrameListeners()
    {
        for (std::set<FrameListener*>::iterator i = mRemovedFrameListeners.begin(); i != mRemovedFrameListeners.end(); ++i)
            mFrameListeners.erase(*i);
        mRemovedFrameListeners.clear();

        for (std::set<FrameListener*>::iterator i = mAddedFrameListeners.begin(); i != mAddedFrameListeners.end(); ++i)
            mFrameListeners.insert(*i);
        mAddedFrameListeners.clear();
    }

    bool Root::renderOneFrame(void)
    {
        mRenderSystem->frameStart();
        if (!_fireFrameStarted())
        {
            mRenderSystem->frameEnd();
            return false;
        }
           
        mRenderSystem->frameEnd();

        return _fireFrameEnded();
    }

    uint32_t Root::_allocateNextMovableObjectTypeFlag(void)
    {
        if (mNextMovableObjectTypeFlag == SceneManager::USER_TYPE_MASK_LIMIT)
        {
            OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
                "Cannot allocate a type flag since "
                "all the available flags have been used.",
                "Root::_allocateNextMovableObjectTypeFlag");

        }
        uint32 ret = mNextMovableObjectTypeFlag;
        mNextMovableObjectTypeFlag <<= 1;
        return ret;

    }
}