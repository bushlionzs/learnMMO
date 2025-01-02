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
#include "OgreRenderable.h"
#include "GameTableManager.h"
#include "CEGUIManager.h"
#include "OgreTextureManager.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include <ResourceParserManager.h>
#include "renderUtil.h"
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
	mGameCamera->update(evt.timeSinceLastFrame);
	mAppInfo->update(evt.timeSinceLastFrame);
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
	ogreConfig.width = 1600;
	ogreConfig.height = 900;
	ogreConfig.enableRaytracing = mAppInfo->enableRayTracing;
	mApplicationWindow->createWindow(ogreConfig.width, ogreConfig.height);
	
	HWND wnd = mApplicationWindow->getWnd();

	if (!InputManager::getSingletonPtr())
	{
		new InputManager();
	}
	InputManager::getSingletonPtr()->createInput((size_t)wnd);

	mRenderSystem = Ogre::Root::getSingleton().createRenderEngine(wnd, mAppInfo->engineType);
	if (!mRenderSystem)
	{
		return false;
	}
	Ogre::ColourValue color(0.678431f, 0.847058f, 0.901960f, 1.000000000f);
	
	CreateWindowDesc desc;
	desc.width = ogreConfig.width;
	desc.height = ogreConfig.height;
	desc.srgb = mAppInfo->useSRGB;
	std::string wndString = Ogre::StringConverter::toString((uint64_t)wnd);
	strncpy(desc.windowHandle, wndString.c_str(), sizeof(desc.windowHandle));
	mRenderWindow = mRenderSystem->createRenderWindow(desc);

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
	mUseCEGUI = mAppInfo->useCEGUI;
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

	{
		RenderTargetBarrier rtBarriers[] =
		{
			{
				mRenderWindow->getColorTarget(),
				RESOURCE_STATE_PRESENT,
				RESOURCE_STATE_RENDER_TARGET
			}
		};
		mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);
	}
	

	for (auto pass : mPassList)
	{
		pass->execute(mRenderSystem);
	}

	{
		RenderTargetBarrier rtBarriers[] =
		{
			{
				mRenderWindow->getColorTarget(),
				RESOURCE_STATE_RENDER_TARGET,
				RESOURCE_STATE_PRESENT
			}
		};
		mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);
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

void updateFrameData(
	ICamera* camera, 
	FrameConstantBuffer& frameConstantBuffer,
	Handle<HwBufferObject> frameHandle)
{
	RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
	const Ogre::Matrix4& view = camera->getViewMatrix();
	const Ogre::Matrix4& proj = camera->getProjectMatrix();
	const Ogre::Vector3& camepos = camera->getDerivedPosition();
	Ogre::Matrix4 invView = view.inverse();
	Ogre::Matrix4 viewProj = proj * view;
	Ogre::Matrix4 invProj = proj.inverse();
	Ogre::Matrix4 invViewProj = viewProj.inverse();

	frameConstantBuffer.View = view.transpose();
	frameConstantBuffer.InvView = invView.transpose();
	frameConstantBuffer.Proj = proj.transpose();
	frameConstantBuffer.InvProj = invProj.transpose();
	frameConstantBuffer.ViewProj = viewProj.transpose();
	frameConstantBuffer.InvViewProj = invViewProj.transpose();

	frameConstantBuffer.EyePosW = camepos;

	
	frameConstantBuffer.Shadow = 0;
	frameConstantBuffer.directionLights[0].Direction = Ogre::Vector3(0, -1, 0.0f);
	frameConstantBuffer.directionLights[0].Direction.normalise();
	


	frameConstantBuffer.TotalTime += Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
	frameConstantBuffer.DeltaTime = Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;

	
	rs->updateBufferObject(frameHandle,
		(const char*)&frameConstantBuffer, sizeof(frameConstantBuffer));
}
void ManualApplication::addUIPass()
{
	return;
	CEGUIManager* ceguiManager = CEGUIManager::getSingletonPtr();
	Ogre::Camera*  cam = ceguiManager->getCamera();
	Ogre::SceneManager* sceneManager = ceguiManager->getSceneManager();
	FrameConstantBuffer frameConstantBuffer;
	auto* rs = mRenderSystem;
	auto& ogreConfig = ::Root::getSingleton().getEngineConfig();
	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(frameConstantBuffer);
	Handle<HwBufferObject> frameHandle =
		rs->createBufferObject(desc);

	updateFrameData(cam, frameConstantBuffer, frameHandle);

	static UserDefineShader userDefineShader;
	userDefineShader.initCallback = initFrameResource;

	userDefineShader.initCallback = initFrameResource;

	RenderableBindCallback bindCallback = [=, this](uint32_t frameIndex, Renderable* r) {
		DescriptorData descriptorData;
		for (auto i = 0; i < ogreConfig.swapBufferCount; i++)
		{
			descriptorData.mCount = 1;
			descriptorData.pName = "cbPass";
			descriptorData.ppBuffers = &frameHandle;
			FrameResourceInfo* resourceInfo = (FrameResourceInfo*)r->getFrameResourceInfo(frameIndex);
			auto* rs = Ogre::Root::getSingleton().getRenderSystem();
			rs->updateDescriptorSet(resourceInfo->zeroSet, 1, &descriptorData);
		}
		};
	userDefineShader.bindCallback = bindCallback;

	RenderableDrawCallback drawCallback = [=, this](uint32_t frameIndex, Renderable* r) {
		void* frameData = r->getFrameResourceInfo(frameIndex);
		FrameResourceInfo* resourceInfo = (FrameResourceInfo*)frameData;
		Ogre::Material* mat = r->getMaterial().get();

		auto programHandle = mat->getProgram();
		auto piplineHandle = mat->getPipeline();
		Handle<HwDescriptorSet> descriptorSet[2];
		descriptorSet[0] = resourceInfo->zeroSet;
		descriptorSet[1] = resourceInfo->firstSet;
		rs->bindPipeline(programHandle, piplineHandle, descriptorSet, 2);


		VertexData* vertexData = r->getVertexData();
		IndexData* indexData = r->getIndexData();
		vertexData->bind(nullptr);
		indexData->bind();
		IndexDataView* view = r->getIndexView();
		rs->drawIndexed(view->mIndexCount, 1,
			view->mIndexLocation, view->mBaseVertexLocation, 0);
		};
	userDefineShader.drawCallback = drawCallback;
	
	RenderPassCallback guiCallback = [=, this](RenderPassInfo& info) {		
		
		info.renderTargetCount = 1;
		info.renderTargets[0].renderTarget = mRenderWindow->getColorTarget();
		info.renderLoadAction = LOAD_ACTION_LOAD;
		info.renderStoreAction = STORE_ACTION_STORE;
		info.renderTargets[0].clearColour = { 0.0f, 0.847058f, 0.901960f, 1.000000000f };
		info.depthTarget.depthStencil = nullptr;
		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		const std::vector<Renderable*>& renderList = ceguiManager->getRenderableList();
		
		renderScene(cam, renderList, info, &userDefineShader);
		};
	UpdatePassCallback updateCallback = [](float delta) {
		};
	auto guiPass = createUserDefineRenderPass(guiCallback, updateCallback);
	addRenderPass(guiPass);
}
