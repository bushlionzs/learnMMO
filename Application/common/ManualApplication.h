#pragma once

#include "ApplicationBase.h"
#include "OgreMeshManager.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"
#include "game_camera.h"
#include "OgreFrameListener.h"
#include "pass.h"

class ApplicationWindow;
class RenderSystem;
class Ogre::RenderTarget;
class Ogre::Camera;



class ManualApplication: public Ogre::FrameListener, public RenderPipeline
{
public:
	ManualApplication();
	~ManualApplication();

	void run(AppInfo& info);
	
	virtual bool isUseCEGUI()
	{
		return mUseCEGUI;
	}

	virtual void OnSize(uint32_t width, uint32_t height);

	virtual void addRenderPass(PassBase* pass);
private:
	bool appInit();
	void render();
	virtual bool frameStarted(const FrameEvent& evt);
	void ShowFrameFrequency();
protected:
	ApplicationWindow* mApplicationWindow = nullptr;
	RenderSystem* mRenderSystem;

	uint64_t mLastFPS = 0;

	SceneManager* mSceneManager = nullptr;
	Ogre::Camera* mCamera = nullptr;
	GameCamera* mGameCamera = nullptr;
	Ogre::RenderWindow* mRenderWindow = nullptr;
	std::vector<PassBase*> mPassList;
	RenderPassInfo mPassInfo;
	bool mUseCEGUI;
	AppInfo* mAppInfo;
};