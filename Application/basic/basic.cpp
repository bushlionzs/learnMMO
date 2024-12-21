#include "stdafx.h"
#include "basic.h"
#include "engine_manager.h"
#include "myutils.h"
#include "OgreResourceManager.h"
#include "OgreMaterialManager.h"
#include "OgreAnimationState.h"
#include "renderSystem.h"
#include "OgreCamera.h"
#include "OgreRenderTarget.h"
#include "OgreRenderWindow.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreMeshManager.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreRoot.h"

BasicApplication::BasicApplication()
{

}

BasicApplication::~BasicApplication()
{

}


void BasicApplication::setup(
	RenderPipeline* renderPipeline,
	RenderSystem* renderSystem,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	ogreConfig.reverseDepth = false;
	mSceneManager = sceneManager;
	mGameCamera = gameCamera;
	mRenderWindow = renderWindow;
	mRenderSystem = renderSystem;
	base1();

	RenderPassInput input;
	input.color = renderWindow->getColorTarget();
	input.depth = renderWindow->getDepthTarget();
	input.cam = gameCamera->getCamera();
	input.sceneMgr = sceneManager;
	auto mainPass = createStandardRenderPass(input);
	renderPipeline->addRenderPass(mainPass);
}

void BasicApplication::update(float delta)
{
	if (mAnimationState)
	{
		mAnimationState->addTime(delta);
	}
}

void BasicApplication::addCustomDirectory()
{
	//ResourceManager::getSingletonPtr()->addDirectory(std::string("D:\\wow3.3.5\\Data"), "wow", true);
}

void BasicApplication::base1()
{
	SceneNode* root = mSceneManager->getRoot()->createChildSceneNode("root");
	float aa = 1.0f;
	Ogre::Vector3 leftop = Ogre::Vector3(-aa, aa, 0.0f);
	Ogre::Vector3 leftbottom = Ogre::Vector3(-aa, -aa, 0.0f);
	Ogre::Vector3 righttop = Ogre::Vector3(aa, aa, 0.0f);
	Ogre::Vector3 rightbottom = Ogre::Vector3(aa, -aa, 0.0f);
	Ogre::Vector3 normal = Ogre::Vector3(0.0f, 0.0f, 1.0f);

	std::string meshName = "rect";
	
	auto mesh = MeshManager::getSingletonPtr()->createRect(
		nullptr,
		meshName,
		leftop, leftbottom, righttop, rightbottom, normal);

	Entity* rect = mSceneManager->createEntity("rect", meshName);
	SceneNode* rectnode = root->createChildSceneNode("rect");
	rectnode->attachObject(rect);

	SubEntity* subEntry = rect->getSubEntity(0);
	auto& mat = subEntry->getMaterial();
	ShaderInfo& info = mat->getShaderInfo();
	//info.shaderName = "testShader";
	//mSceneManager->setSkyBox(true, "SkyLan", 1000.0f);
	mGameCamera->lookAt(Ogre::Vector3(0, 0.0f, 3.f), Ogre::Vector3::ZERO);
	mGameCamera->setCameraType(CameraMoveType_FirstPerson);
	mGameCamera->setMoveSpeed(5);
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	Ogre::Matrix4 m;
	if (ogreConfig.reverseDepth)
	{
		float aspectInverse = ogreConfig.height / (float)ogreConfig.width;
		m = Ogre::Math::makePerspectiveMatrixReverseZ(
			Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 6000);
	}
	else
	{
		float aspect = ogreConfig.width / (float)ogreConfig.height;
		m = Ogre::Math::makePerspectiveMatrix(
			Ogre::Math::PI / 2.0f, aspect, 0.1, 6000);
	}
	mGameCamera->getCamera()->updateProjectMatrix(m);
}

void BasicApplication::base2()
{
	std::string name = "Êé_·ðÉ½_·¿ÎÝ_13.mesh";
	name = "Â¥À¼ÕÊÅñ04.mesh";
	auto mesh = MeshManager::getSingletonPtr()->load(name);

	SceneNode* root = mSceneManager->getRoot()->createChildSceneNode("root");

	Entity* sphere = mSceneManager->createEntity("sphere", name);
	SceneNode* spherenode = root->createChildSceneNode("sphere");

	spherenode->attachObject(sphere);

	mSceneManager->setSkyBox(true, "SkyLan", 5000);
	
	mGameCamera->lookAt(
		Ogre::Vector3(2000, 0.0, 0.0f), 
		Ogre::Vector3(0.0f, 0.0f, 0.0f));
	mGameCamera->setMoveSpeed(200);

	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	float aspectInverse = ogreConfig.height / (float)ogreConfig.width;

	Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrixLHReverseZ(
		Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 10000.f);

	mGameCamera->getCamera()->updateProjectMatrix(m);
}

void BasicApplication::base3()
{
	SceneNode* root = mSceneManager->getRoot()->createChildSceneNode("root");
	auto mesh = MeshManager::getSingleton().createBox("box.mesh", 1, "mybox");

	auto& meshName = mesh->getName();
	{
		auto entity = mSceneManager->createEntity("box1", meshName);
		SceneNode* node = root->createChildSceneNode("box1");
		node->attachObject(entity);
		node->setPosition(0.0f, 0.0f, -5.0f);
	}

	{
		auto entity = mSceneManager->createEntity("box1", meshName);
		SceneNode* node = root->createChildSceneNode("box1");
		node->attachObject(entity);
		node->setPosition(0.0f, 0.0f, 5.0f);
	}

	{
		auto entity = mSceneManager->createEntity("box1", meshName);
		SceneNode* node = root->createChildSceneNode("box1");
		node->attachObject(entity);
		node->setPosition(-5.0f, 0.0f, 0.0f);
	}

	{
		auto entity = mSceneManager->createEntity("box1", meshName);
		SceneNode* node = root->createChildSceneNode("box1");
		node->attachObject(entity);
		node->setPosition(5.0f, 0.0f, 0.0f);
	}
	

	mGameCamera->setDistance(2.0f);

	mGameCamera->setMoveSpeed(25.0f);
}

void BasicApplication::base4()
{
	std::string meshname = "ÃÉ¹Å¹ó×åÅ®_03.mesh";
	auto mesh = MeshManager::getSingletonPtr()->load(meshname);

	SceneNode* root = mSceneManager->getRoot()->createChildSceneNode("root");

	Entity* gltf = mSceneManager->createEntity("gltf", meshname);
	SceneNode* gltfnode = root->createChildSceneNode("gltf");
	gltfnode->updatechildren();
	gltfnode->attachObject(gltf);

	mAnimationState = gltf->getAnimationState(std::string("ÐÝÏÐ04_02"));
	if (mAnimationState)
	{
		mAnimationState->setEnabled(true);
		mAnimationState->setLoop(true);
	}

	mGameCamera->setHeight(300.0f);
	mGameCamera->setDistance(500);
	mGameCamera->setMoveSpeed(25.0f);
}

void BasicApplication::base5()
{
	
}