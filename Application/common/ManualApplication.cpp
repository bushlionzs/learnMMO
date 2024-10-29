#include "OgreHeader.h"
#include "ManualApplication.h"
#include "application_window.h"
#include "InputManager.h"
#include "renderSystem.h"
#include "OgreResourceManager.h"
#include "OgreMaterialManager.h"
#include "OgreCamera.h"
#include "OgreSceneManager.h"
#include "OgreRoot.h"
#include "OgreRenderWindow.h"
#include "OgreViewport.h"
#include "GameTableManager.h"
#include "CEGUIManager.h"
#include "OgreTextureManager.h"
#include <ResourceParserManager.h>
#include "pass.h"



ManualApplication::ManualApplication()
{
}

ManualApplication::~ManualApplication()
{

}

bool ManualApplication::frameStarted(const FrameEvent& evt)
{
	InputManager::getSingletonPtr()->captureInput();
	mAppInfo->update(evt.timeSinceLastFrame);
	mGameCamera->update(evt.timeSinceLastFrame);
	for (auto pass : mPassList)
	{
		pass->update(evt.timeSinceLastFrame);
	}
	return true;
}


bool ManualApplication::appInit()
{
	mApplicationWindow = new ApplicationWindow();

	new Ogre::Root();
	Ogre::Root::getSingleton()._initialise();

	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	ogreConfig.width = 1440;
	ogreConfig.height = 900;
	ogreConfig.enableRaytracing = mAppInfo->enableRayTracing;
	mApplicationWindow->createWindow(ogreConfig.width, ogreConfig.height);
	
	HWND wnd = mApplicationWindow->getWnd();
	
	
	


	if (!InputManager::getSingletonPtr())
	{
		new InputManager();
	}
	InputManager::getSingletonPtr()->createInput((size_t)wnd);

	mRenderSystem = Ogre::Root::getSingleton().createRenderEngine(wnd, EngineType_Vulkan);
	if (!mRenderSystem)
	{
		return false;
	}
	Ogre::ColourValue color(0.678431f, 0.847058f, 0.901960f, 1.000000000f);
	Ogre::NameValuePairList params;
	params["externalWindowHandle"] = Ogre::StringConverter::toString((uint64_t)wnd);
	params["backGroundColor"] = Ogre::StringConverter::toString(color);
	params["srgb"] = "1";
	mRenderWindow = mRenderSystem->createRenderWindow("", ogreConfig.width, ogreConfig.height, &params);

	ResourceParserManager::getSingleton()._initialise();
	ResourceManager::getSingletonPtr()->addDirectory(std::string("..\\..\\resources"), "sujian", true);
	ResourceManager::getSingletonPtr()->loadAllResource();
	
	mSceneManager = Ogre::Root::getSingleton().createSceneManger(MAIN_SCENE_MANAGER);

	mCamera = mSceneManager->createCamera(MAIN_CAMERA);

	mCamera->setNearClipDistance(1.0f);
	mGameCamera = new GameCamera(mCamera, mSceneManager);

	InputManager::getSingletonPtr()->addListener(mGameCamera);

	Ogre::Root::getSingleton().addFrameListener(this);

	if (isUseCEGUI())
	{
		/*ShowCursor(FALSE);
		SetCursor(NULL);*/
		new CEGUIManager;
		CEGUIManager::getSingleton()._initialise(mRenderWindow);
	}

	TextureManager::getSingleton().load("white1x1.dds", nullptr);
	return true;
}



void ManualApplication::run(AppInfo& info)
{
	mAppInfo = &info;
	appInit();
	info.setup(mRenderSystem, mRenderWindow, mSceneManager, mGameCamera);
	MSG msg;
	mRenderSystem->ready();
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			if (msg.message == WM_SIZE)
			{
				int kk = 0;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
			
			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			render();
			ShowFrameFrequency();
		}
	}
}

void ManualApplication::render()
{
	mRenderSystem->frameStart();
	Ogre::Root::getSingleton()._fireFrameStarted();

	for (auto pass : mPassList)
	{
		pass->execute(mRenderSystem);
	}


	mRenderSystem->present();
	mRenderSystem->frameEnd();
}

void ManualApplication::ShowFrameFrequency()
{
	if (mLastFPS != Ogre::Root::getSingletonPtr()->getCurrentFPS())
	{
		mLastFPS = Ogre::Root::getSingletonPtr()->getCurrentFPS();


		char buffer[1024];
		std::string str = mGameCamera->getCameraString();
		snprintf(buffer, sizeof(buffer), "render:%s, fps:%lld, triangle:%d,batch:%d, %s", 
			mRenderSystem->getRenderSystemName().c_str(),
			mLastFPS, mRenderSystem->getTriangleCount(), 
			mRenderSystem->getBatchCount(),
			str.c_str());

		
		::SetWindowText(mApplicationWindow->getWnd(), buffer);
	}
}

void ManualApplication::OnSize(uint32_t width, uint32_t height)
{
	if (mRenderWindow)
	{
		auto w = mRenderWindow->getWidth();
		auto h = mRenderWindow->getHeight();
		if (w != width || h != height)
		{
			mRenderWindow->resize(width, height);
		}
	}
}

void ManualApplication::addRenderPass(PassBase* pass)
{
	mPassList.push_back(pass);
}
