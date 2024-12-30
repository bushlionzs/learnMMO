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
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgreVertexDeclaration.h"

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
	mRenderPipeline = renderPipeline;
	base4();	
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
	float aa = 1;
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
	mGameCamera->lookAt(Ogre::Vector3(0, 0.0f, -5.f), Ogre::Vector3::ZERO);
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

	RenderPassInput input;
	input.color = mRenderWindow->getColorTarget();
	input.depth = mRenderWindow->getDepthTarget();
	input.cam = mGameCamera->getCamera();
	input.sceneMgr = mSceneManager;
	auto mainPass = createStandardRenderPass(input);
	mRenderPipeline->addRenderPass(mainPass);
}

void BasicApplication::base2()
{
	std::string name = "Â¥À¼ÕÊÅñ04.mesh";
	auto mesh = MeshManager::getSingletonPtr()->load(name);

	SceneNode* root = mSceneManager->getRoot()->createChildSceneNode("root");

	Entity* sphere = mSceneManager->createEntity("sphere", name);
	SceneNode* spherenode = root->createChildSceneNode("sphere");

	spherenode->attachObject(sphere);

	mGameCamera->lookAt(
		Ogre::Vector3(1000, 0.0, 0.0f), 
		Ogre::Vector3(0.0f, 0.0f, 0.0f));
	mGameCamera->setMoveSpeed(200);

	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	float aspectInverse = ogreConfig.height / (float)ogreConfig.width;

	Ogre::Matrix4 m;

	if (ogreConfig.reverseDepth)
	{
		float aspectInverse = ogreConfig.height / (float)ogreConfig.width;
		m = Ogre::Math::makePerspectiveMatrixReverseZ(
			Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 10000.f);
	}
	else
	{
		float aspect = ogreConfig.width / (float)ogreConfig.height;
		m = Ogre::Math::makePerspectiveMatrix(
			Ogre::Math::PI / 2.0f, aspect, 0.1, 10000.f);
	}
	mGameCamera->getCamera()->updateProjectMatrix(m);

	RenderPassInput input;
	input.color = mRenderWindow->getColorTarget();
	input.depth = mRenderWindow->getDepthTarget();
	input.cam = mGameCamera->getCamera();
	input.sceneMgr = mSceneManager;
	auto mainPass = createStandardRenderPass(input);
	mRenderPipeline->addRenderPass(mainPass);
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

	/*mGameCamera->setHeight(300.0f);
	mGameCamera->setDistance(500);
	mGameCamera->setMoveSpeed(25.0f);*/

	mGameCamera->lookAt(
		Ogre::Vector3(0.0f, 0.0f, 500.0f),
		Ogre::Vector3(0.0f, 0.0f, 0.0f));
	mGameCamera->setMoveSpeed(25);

	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	float aspectInverse = ogreConfig.height / (float)ogreConfig.width;

	Ogre::Matrix4 m;

	if (ogreConfig.reverseDepth)
	{
		float aspectInverse = ogreConfig.height / (float)ogreConfig.width;
		m = Ogre::Math::makePerspectiveMatrixReverseZ(
			Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 10000.f);
	}
	else
	{
		float aspect = ogreConfig.width / (float)ogreConfig.height;
		m = Ogre::Math::makePerspectiveMatrix(
			Ogre::Math::PI / 2.0f, aspect, 0.1, 10000.f);
	}
	mGameCamera->getCamera()->updateProjectMatrix(m);

	RenderPassInput input;
	input.color = mRenderWindow->getColorTarget();
	input.depth = mRenderWindow->getDepthTarget();
	input.cam = mGameCamera->getCamera();
	input.sceneMgr = mSceneManager;
	auto mainPass = createStandardRenderPass(input);
	mRenderPipeline->addRenderPass(mainPass);
}

void BasicApplication::base5()
{
	auto* rs = mRenderSystem;
	ShaderInfo shaderInfo;
	shaderInfo.shaderName = "mipmap";
	VertexDeclaration decl;
	decl.addElement(0, 0, 0, VET_FLOAT3, VES_POSITION);
	decl.addElement(0, 0, 12, VET_FLOAT3, VES_NORMAL);
	decl.addElement(0, 0, 24, VET_FLOAT2, VES_TEXTURE_COORDINATES);
	auto mipmapHandle = rs->createShaderProgram(shaderInfo, &decl);
	backend::RasterState rasterState{};
	rasterState.colorWrite = true;
	rasterState.renderTargetCount = 1;
	rasterState.depthWrite = false;
	rasterState.depthTest = false;
	rasterState.pixelFormat = PF_A8R8G8B8;
	auto mipmapPipelineHandle = rs->createPipeline(rasterState, mipmapHandle);

	auto mipmapDescSet = rs->createDescriptorSet(mipmapHandle, 0);
	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding::BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(Ogre::Matrix4);
	auto mipmapBlockHandle = rs->createBufferObject(desc);
	const char* name = "garden_nx.dds";
	name = "studio_garden_px.jpg";
	auto tex = rs->createTextureFromFile(name, nullptr);

	DescriptorData descriptorData[3];

	descriptorData[0].pName = "cbPerObject";
	descriptorData[0].mCount = 1;
	descriptorData[0].descriptorType = DESCRIPTOR_TYPE_BUFFER;
	descriptorData[0].ppBuffers = &mipmapBlockHandle;
		
	descriptorData[1].pName = "first";
	descriptorData[1].mCount = 1;
	descriptorData[1].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[1].ppTextures = (const OgreTexture**)&tex;

	descriptorData[2].pName = "firstSampler";
	descriptorData[2].mCount = 1;
	descriptorData[2].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[2].ppTextures = (const OgreTexture**)&tex;

	rs->updateDescriptorSet(mipmapDescSet, 3, descriptorData);

	auto width = mRenderWindow->getWidth();
	auto height = mRenderWindow->getHeight();
	
	auto project = Ogre::Math::makeOrthoLH(0.0f, width, height, 0.0f, 0.1, 1000.0f);

	/*float aspect = width / height;
	project = Ogre::Math::makePerspectiveMatrix(
		Ogre::Math::PI / 2.0f, aspect, 0.1, 6000);*/

	Ogre::Vector3 eyePos = Ogre::Vector3(0, 0, -10);
	Ogre::Matrix4 view = Ogre::Math::makeLookAtLH(eyePos, Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_Y);

	Ogre::Matrix4 viewProj = (project * view).transpose();

	rs->updateBufferObject(mipmapBlockHandle, (const char*)&viewProj, sizeof(viewProj));
	float aa = 1024;
	Ogre::Vector3 leftop = Ogre::Vector3(-aa, aa, 0.0f);
	Ogre::Vector3 leftbottom = Ogre::Vector3(-aa, -aa, 0.0f);
	Ogre::Vector3 righttop = Ogre::Vector3(aa, aa, 0.0f);
	Ogre::Vector3 rightbottom = Ogre::Vector3(aa, -aa, 0.0f);
	Ogre::Vector3 normal = Ogre::Vector3(0.0f, 0.0f, 1.0f);

	leftop = Ogre::Vector3(0.0f, 0.0f, 0.0f);
	leftbottom = Ogre::Vector3(0.0f, aa, 0.0f);
	righttop = Ogre::Vector3(aa, 0.0f, 0.0f);
	rightbottom = Ogre::Vector3(aa, aa, 0.0f);
	normal = Ogre::Vector3(0.0f, 0.0f, 1.0f);

	std::string meshName = "rect";

	auto mesh = MeshManager::getSingletonPtr()->createRect(
		nullptr,
		meshName,
		leftop, leftbottom, righttop, rightbottom, normal);

	
	auto* subMesh = mesh->getSubMesh(0);
	VertexData* vertexData = mesh->getVertexData();
	IndexData* indexData = mesh->getIndexData();

	RenderPassCallback renderCallback = [=, this](RenderPassInfo& renderPassInfo) {
		renderPassInfo.renderTargetCount = 1;
		renderPassInfo.renderTargets[0].renderTarget = mRenderWindow->getColorTarget();;
		renderPassInfo.depthTarget.depthStencil = nullptr;
		renderPassInfo.renderTargets[0].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
		renderPassInfo.viewport = false;

		float v = 512;
		rs->setViewport(0, 0, v, v, 0.0f, 1.0f);
		rs->setScissor(0, 0, v, v);
		rs->beginRenderPass(renderPassInfo);
		
		rs->bindPipeline(mipmapHandle, mipmapPipelineHandle, &mipmapDescSet, 1);
		vertexData->bind(nullptr);
		indexData->bind();
		IndexDataView* indexView = subMesh->getIndexView();
		rs->drawIndexed(indexView->mIndexCount, 1,
			indexView->mIndexLocation, indexView->mBaseVertexLocation, 0);
		rs->endRenderPass(renderPassInfo);
		};

	UpdatePassCallback updateCallback = [=, this](float delta) {
		};

	auto myPass = createUserDefineRenderPass(renderCallback, updateCallback);
	mRenderPipeline->addRenderPass(myPass);
}