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
	base2();
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
	name = "vulkanscene_shadow.gltf";
	auto mesh = MeshManager::getSingletonPtr()->load(name);

	SceneNode* root = mSceneManager->getRoot()->createChildSceneNode("root");

	Entity* sphere = mSceneManager->createEntity("sphere", name);
	SceneNode* spherenode = root->createChildSceneNode("sphere");

	sphere->setMaterialName("myrect");

	spherenode->attachObject(sphere);

	mGameCamera->lookAt(
		Ogre::Vector3(0.0f, 3.0f, 15.0f),
		Ogre::Vector3(0.0f, 0.0f, 0.0f));
	mGameCamera->setMoveSpeed(200);
	mGameCamera->setCameraType(CameraMoveType_LookAt);
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	float aspectInverse = ogreConfig.height / (float)ogreConfig.width;

	Ogre::Matrix4 m;

	if (ogreConfig.reverseDepth)
	{
		float aspectInverse = ogreConfig.height / (float)ogreConfig.width;
		m = Ogre::Math::makePerspectiveMatrixReverseZ(
			Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 2000);
	}
	else
	{
		float aspect = ogreConfig.width / (float)ogreConfig.height;
		m = Ogre::Math::makePerspectiveMatrixRH(
			Ogre::Math::PI / 2.0f, aspect, 0.1, 2000);
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

	std::string meshName = "box";
	auto mesh = MeshManager::getSingleton().createBox(meshName, 1, "mybox");

	{
		auto entity = mSceneManager->createEntity("box1", meshName);
		SceneNode* node = root->createChildSceneNode("box1");
		node->attachObject(entity);
		node->setPosition(0.0f, 0.0f, -5.0f);
	}

	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	float aspectInverse = ogreConfig.height / (float)ogreConfig.width;

	mGameCamera->setCameraType(CameraMoveType_LookAt);
	mGameCamera->lookAt(
		Ogre::Vector3(0.5f, 0.0f, -7),
		Ogre::Vector3(0.0f, 0.0f, 0.0f));
	mGameCamera->setMoveSpeed(5);

	Ogre::Matrix4 m;

	if (ogreConfig.reverseDepth)
	{
		float aspectInverse = ogreConfig.height / (float)ogreConfig.width;
		m = Ogre::Math::makePerspectiveMatrixReverseZ(
			Ogre::Math::PI / 4.0f, aspectInverse, 0.1, 10000.f);
	}
	else
	{
		float aspect = ogreConfig.width / (float)ogreConfig.height;
		m = Ogre::Math::makePerspectiveMatrix(
			Ogre::Math::PI / 4.0f, aspect, 0.1, 10000.f);
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
	std::string meshname = "bunny.fbx";
	meshname = "sphere_big.fbx";
	auto mesh = MeshManager::getSingletonPtr()->load(meshname);

	SceneNode* root = mSceneManager->getRoot()->createChildSceneNode("root");

	Entity* gltf = mSceneManager->createEntity("fbx", meshname);
	SceneNode* gltfnode = root->createChildSceneNode("fbx");
	gltfnode->updatechildren();
	gltfnode->attachObject(gltf);

	mGameCamera->lookAt(
		Ogre::Vector3(0.0f, 0.0, 20),
		Ogre::Vector3(0.0f, 0.0f, 0.0f));
	mGameCamera->setMoveSpeed(25);

	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	float aspectInverse = ogreConfig.height / (float)ogreConfig.width;

	Ogre::Matrix4 m;

	if (ogreConfig.reverseDepth)
	{
		float aspectInverse = ogreConfig.height / (float)ogreConfig.width;
		m = Ogre::Math::makePerspectiveMatrixReverseZ(
			Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 1000);
	}
	else
	{
		float aspect = ogreConfig.width / (float)ogreConfig.height;
		m = Ogre::Math::makePerspectiveMatrix(
			Ogre::Math::PI / 2.0f, aspect, 0.1, 1000);
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

void BasicApplication::updateFrameData(ICamera* camera, FrameConstantBuffer& frameBuffer)
{
	RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
	const Ogre::Matrix4& view = camera->getViewMatrix();
	const Ogre::Matrix4& proj = camera->getProjectMatrix();
	const Ogre::Vector3& camepos = camera->getDerivedPosition();
	Ogre::Matrix4 invView = view.inverse();
	Ogre::Matrix4 viewProj = proj * view;
	Ogre::Matrix4 invProj = proj.inverse();
	Ogre::Matrix4 invViewProj = viewProj.inverse();

	frameBuffer.View = view.transpose();
	frameBuffer.InvView = invView.transpose();
	frameBuffer.Proj = proj.transpose();
	frameBuffer.InvProj = invProj.transpose();
	frameBuffer.ViewProj = viewProj.transpose();
	frameBuffer.InvViewProj = invViewProj.transpose();

	frameBuffer.EyePosW = camepos;

	
	frameBuffer.Shadow = 0;
	frameBuffer.directionLights[0].Direction = Ogre::Vector3(0.739942074, 0.642787576, 0.198266909);
	frameBuffer.directionLights[0].Direction.normalise();



	frameBuffer.TotalTime += Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
	frameBuffer.DeltaTime = Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;

	auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
}

void BasicApplication::base6()
{
	std::string name = "Â¥À¼ÕÊÅñ04.mesh";
	auto mesh = MeshManager::getSingletonPtr()->load(name);

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
			Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 2000);
	}
	else
	{
		float aspect = ogreConfig.width / (float)ogreConfig.height;
		m = Ogre::Math::makePerspectiveMatrix(
			Ogre::Math::PI / 2.0f, aspect, 0.1, 2000);
	}
	mGameCamera->getCamera()->updateProjectMatrix(m);

	ShaderInfo shaderInfo;
	shaderInfo.shaderName = "basic2";
	auto presentHandle = mRenderSystem->createShaderProgram(shaderInfo, nullptr);

	backend::RasterState rasterState{};
	rasterState.depthWrite = false;
	rasterState.depthTest = false;
	rasterState.depthFunc = SamplerCompareFunc::A;
	rasterState.colorWrite = true;
	rasterState.renderTargetCount = 1;
	rasterState.pixelFormat[0] = Ogre::PixelFormat::PF_A8R8G8B8;
	auto pipelineHandle = mRenderSystem->createPipeline(rasterState, presentHandle);

	SubMesh* subMesh = mesh->getSubMesh(0);

	VertexData* vertexData = subMesh->getVertexData();
	Handle<HwBufferObject> vertexDataHandle = vertexData->getBuffer(0);
	IndexData* indexData = subMesh->getIndexData();
	Handle<HwBufferObject> indexDataHandle = indexData->getHandle();
	mFrameData.resize(2);
	DescriptorData descriptorData[16];

	

	std::shared_ptr<Material>& mat = subMesh->getMaterial();
	mat->load(nullptr);

	OgreTexture* tex = mat->getTexture(0);

	for (auto i = 0; i < 2; i++)
	{
		auto zeroSet = mRenderSystem->createDescriptorSet(presentHandle, 0);

		mFrameData[i].zeroSet = zeroSet;
		BufferDesc desc{};
		desc.mBindingType = BufferObjectBinding_Uniform;
		desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
		desc.bufferCreationFlags = 0;
		desc.mElementCount = 0;
		desc.mStructStride = 0;
		desc.mSize = sizeof(FrameConstantBuffer);
		desc.pName = "FrameConstantBuffer";
		auto passUniformBuffer = mRenderSystem->createBufferObject(desc);
		mFrameData[i].passUniformBuffer = passUniformBuffer;

		descriptorData[0].pName = "passUniformBlock";
		descriptorData[0].mCount = 1;
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[0].ppBuffers = &mFrameData[i].passUniformBuffer;

		descriptorData[1].pName = "vertexDataBuffer";
		descriptorData[1].mCount = 1;
		descriptorData[1].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[1].ppBuffers = &vertexDataHandle;

		descriptorData[2].pName = "first";
		descriptorData[2].mCount = 1;
		descriptorData[2].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
		descriptorData[2].ppTextures = (const Ogre::OgreTexture**)& tex;

		descriptorData[3].pName = "firstSampler";
		descriptorData[3].mCount = 1;
		descriptorData[3].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
		descriptorData[3].ppTextures = (const Ogre::OgreTexture**)&tex;

		mRenderSystem->updateDescriptorSet(zeroSet, 4, descriptorData);
	}

	uint32_t indexCount = indexData->getIndexCount();
	uint32_t indexSize = indexData->getIndexSize();
	RenderPassCallback presentCallback = [=, this](RenderPassInfo& info) {
		info.renderTargetCount = 1;
		info.renderTargets[0].renderTarget = mRenderWindow->getColorTarget();
		info.renderTargets[0].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
		info.depthTarget.depthStencil = nullptr;
		info.depthTarget.clearValue = { 0.0f, 0.0f };
		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		mRenderSystem->bindIndexBuffer(indexDataHandle, indexSize);
		mRenderSystem->pushGroupMarker("presentPass");
		mRenderSystem->beginRenderPass(info);
		mRenderSystem->bindPipeline(pipelineHandle,
			&mFrameData[frameIndex].zeroSet, 1);
		mRenderSystem->drawIndexed(indexCount, 1, 0, 0, 0);
		mRenderSystem->endRenderPass(info);
		mRenderSystem->popGroupMarker();
		};
	Ogre::Camera* cam = mGameCamera->getCamera();
	UpdatePassCallback updateCallback = [=, this](float delta) {
		FrameConstantBuffer frameBuffer;
		this->updateFrameData(cam, frameBuffer);
		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		mRenderSystem->updateBufferObject(mFrameData[frameIndex].passUniformBuffer,
			(const char*)&frameBuffer, sizeof(frameBuffer));
		};
	auto presentPass = createUserDefineRenderPass(presentCallback, updateCallback);
	mRenderPipeline->addRenderPass(presentPass);
}