#pragma once

#include "OgreSingleton.h"
#include "OgreTimer.h"
#include "OgreCommon.h"
#include "engine_struct.h"
#include "OgreFrameListener.h"
#include <utils/JobSystem.h>
class RenderSystem;
#define MAIN_SCENE_MANAGER "__main__"
#define MAIN_CAMERA "__main_camera__"


namespace Ogre {
	class MovableObjectFactory;
	struct EngineConfig
	{
		uint32_t width = 1280;
		uint32_t height = 720;
		uint32_t frameConstantBufferSize = sizeof(FrameConstantBuffer);
		uint32_t swapBufferCount = 2;
		bool     enableRaytracing = false;
	};

	class Root : public Ogre::Singleton<Root>
	{
	public:
		typedef std::map<String, MovableObjectFactory*> MovableObjectFactoryMap;
	public:
		Root();
		~Root();

		void _initialise();
		RenderSystem* createRenderEngine(
			HWND wnd,
			EngineType et);
		RenderSystem* getRenderSystem();
		void addMovableObjectFactory(MovableObjectFactory* fact,
			bool overrideExisting = false);
		void removeMovableObjectFactory(MovableObjectFactory* fact);
		MovableObjectFactory* getMovableObjectFactory(const String& typeName);

		SceneManager* createSceneManger(const std::string& name);
		void destroySceneManager(SceneManager* sceneMgr);

		SceneManager* getSceneManager(const std::string& name);
		uint64_t getCurrentFrame();
		uint64_t getCurrentFrameIndex();
		uint64_t getCurrentFPS();
		
		bool _fireFrameStarted();
		void update(float delta);
		bool _fireFrameEnded();
		void addFrameListener(FrameListener* newListener);
		void removeFrameListener(FrameListener* oldListener);
		void _syncAddedRemovedFrameListeners();

		bool renderOneFrame(void);

		Timer* getTimer()
		{
			return &mTimer;
		}

		Ogre::FrameEvent& getFrameEvent()
		{
			return mEvt;
		}

		EngineConfig& getEngineConfig()
		{
			return mEngineConfig;
		}
	private:
		uint32_t _allocateNextMovableObjectTypeFlag(void);
	private:
		MovableObjectFactoryMap mMovableObjectFactoryMap;
		uint32 mNextMovableObjectTypeFlag;

		uint64_t mCurrentFrame;
		uint64_t mCurrentFPS;

		uint64_t mLastFrame;
		double mLast;
		double mAccumulation;
		Timer mTimer;
		std::shared_ptr<RenderSystem> mRenderSystem;
		std::unordered_map<String, SceneManager*> mSceneManagerMap;

		/** Set of registered frame listeners */
		std::set<FrameListener*> mFrameListeners;

		/** Set of frame listeners marked for removal and addition*/
		std::set<FrameListener*> mRemovedFrameListeners;
		std::set<FrameListener*> mAddedFrameListeners;

		uint64_t mFrameCurrent, mFrameLast;

		Ogre::FrameEvent mEvt;
		
		EngineConfig mEngineConfig;
	};
}