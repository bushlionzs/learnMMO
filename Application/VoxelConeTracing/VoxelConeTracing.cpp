#include "OgreHeader.h"
#include "VoxelConeTracing.h"
#include "OgreParticleSystem.h"
#include "myutils.h"
#include "OgreResourceManager.h"
#include "OgreMaterialManager.h"
#include "OgreMeshManager.h"
#include "OgreSceneManager.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreCamera.h"
#include "OgreRenderWindow.h"
#include "renderSystem.h"
#include "OgreRoot.h"
#include "OgreRenderable.h"
#include "OgreVertexData.h"
#include "OgreTextureUnit.h"
#include "OgreVertexDeclaration.h"
#include "renderUtil.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

VoxelConeTracingApp::VoxelConeTracingApp()
{

}

VoxelConeTracingApp::~VoxelConeTracingApp()
{

}

void VoxelConeTracingApp::setup(
	RenderPipeline* renderPipeline,
	RenderSystem* rs,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	mRenderPipeline = renderPipeline;
	mGameCamera   = gameCamera;
	mSceneManager = sceneManager;
	mRenderWindow = renderWindow;
	mRenderSystem = rs;
	/*std::string meshname = "Sponza.gltf";
	std::shared_ptr<Mesh> mesh = MeshManager::getSingletonPtr()->load(meshname);
	auto rootNode = sceneManager->getRoot();
	Entity* sponza = sceneManager->createEntity(meshname, meshname);
	SceneNode* sponzaNode = rootNode->createChildSceneNode(meshname);
	sponzaNode->attachObject(sponza);*/

	initScene();
	auto h = 7.0f;
	auto camPos = Ogre::Vector3(0.0f, h, 33.0f);
	auto targetPos = Ogre::Vector3(0.0f, h, 0.0f);
	gameCamera->lookAt(camPos, targetPos);
	gameCamera->setMoveSpeed(20);
	

	float aspect = ogreConfig.width / (float)ogreConfig.height;
	Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrix(
		Ogre::Math::PI / 3.0f, aspect, 0.1, 1000.0f);
	gameCamera->getCamera()->updateProjectMatrix(m);
	mRenderSystem = rs;

	auto* cam = gameCamera->getCamera();
	auto* light = sceneManager->createLight("light");

	//SceneGeometryPass
	if(1)
	{
		sceneGeometryPass();
	}

	//shadow pass
	if (1)
	{
		shadowPass();
	}
}

void VoxelConeTracingApp::update(float delta)
{
}

void VoxelConeTracingApp::sceneGeometryPass()
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	TextureProperty texProperty;
	texProperty._width = ogreConfig.width;
	texProperty._height = ogreConfig.height;
	texProperty._tex_format = Ogre::PixelFormat::PF_A8R8G8B8;
	texProperty._tex_usage = Ogre::TextureUsage::COLOR_ATTACHMENT;
	mVoxelizationContext.albedoTarget = mRenderSystem->createRenderTarget("albedoTarget", texProperty);
	mVoxelizationContext.normalTarget = mRenderSystem->createRenderTarget("normalTarget", texProperty);
	mVoxelizationContext.worldPosTarget = mRenderSystem->createRenderTarget("worldPos", texProperty);
	texProperty._width = 2048;
	texProperty._height = 2048;
	texProperty._tex_format = Ogre::PixelFormat::PF_DEPTH32F;
	texProperty._tex_usage = Ogre::TextureUsage::DEPTH_ATTACHMENT;
	mVoxelizationContext.depthTarget = mRenderSystem->createRenderTarget(
		"depthTarget", texProperty);

	ShaderInfo shaderInfo;
	shaderInfo.shaderName = "sceneGeometryPass";
	//shaderInfo.shaderMacros.push_back(std::pair<std::string, std::string>("PBR", "1"));
	VertexDeclaration decl;
	decl.addElement(0, 0, 0, VET_FLOAT3, VES_POSITION);
	decl.addElement(0, 0, 12, VET_FLOAT3, VES_NORMAL);
	decl.addElement(0, 0, 24, VET_FLOAT2, VES_TEXTURE_COORDINATES);
	auto programHandle = mRenderSystem->createShaderProgram(shaderInfo, &decl);
	backend::RasterState rasterState{};
	auto targetCount = 4;
	rasterState.depthFunc = SamplerCompareFunc::LE;
	rasterState.renderTargetCount = targetCount;
	rasterState.depthWrite = true;
	rasterState.depthTest = true;
	rasterState.pixelFormat = Ogre::PixelFormat::PF_A8R8G8B8;
	auto pipelineHandle = mRenderSystem->createPipeline(rasterState, programHandle);
	
	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(mFrameConstantBuffer);
	Handle<HwBufferObject> zeroFrameBufferHandle = mRenderSystem->createBufferObject(desc);
	Handle<HwBufferObject> firstFrameBufferHandle = mRenderSystem->createBufferObject(desc);

	VoxelizationContext* context = &mVoxelizationContext;
	static UserDefineShader userDefineShader;
	userDefineShader.initCallback = initFrameResource;
	RenderableBindCallback bindCallback = [=, this](uint32_t frameIndex, Renderable* r) {
		if (r->hasFlag(sceneGeometryPassBit + frameIndex))
		{
			return;
		}
		r->setFlag(sceneGeometryPassBit + frameIndex, true);
		DescriptorData descriptorData;
		Handle<HwBufferObject> tmp[2];
		tmp[0] = zeroFrameBufferHandle;
		tmp[1] = firstFrameBufferHandle;
		
		descriptorData.mCount = 1;
		descriptorData.pName = "cbPass";
		descriptorData.ppBuffers = &tmp[frameIndex];
		FrameResourceInfo* resourceInfo = (FrameResourceInfo*)r->getFrameResourceInfo(frameIndex);
		mRenderSystem->updateDescriptorSet(resourceInfo->zeroSet, 1, &descriptorData);

		};
	userDefineShader.bindCallback = bindCallback;

	RenderableDrawCallback drawCallback = [=, this](uint32_t frameIndex, Renderable* r) {
		void* frameData = r->getFrameResourceInfo(frameIndex);
		FrameResourceInfo* resourceInfo = (FrameResourceInfo*)frameData;
		Ogre::Material* mat = r->getMaterial().get();

		Handle<HwDescriptorSet> descriptorSet[2];
		descriptorSet[0] = resourceInfo->zeroSet;
		descriptorSet[1] = resourceInfo->firstSet;
		mRenderSystem->bindPipeline(programHandle, pipelineHandle, descriptorSet, 2);


		VertexData* vertexData = r->getVertexData();
		IndexData* indexData = r->getIndexData();
		vertexData->bind(nullptr);
		indexData->bind();
		IndexDataView* view = r->getIndexView();
		mRenderSystem->drawIndexed(view->mIndexCount, 1,
			view->mIndexLocation, view->mBaseVertexLocation, 0);
		};
	userDefineShader.drawCallback = drawCallback;
	UserDefineShader* pUserDefineShader = &userDefineShader;
	RenderPassCallback sceneGeometryPassCallback = [=, this](RenderPassInfo& info) {
		info.renderTargetCount = 4;
		info.renderTargets[0].renderTarget = mVoxelizationContext.albedoTarget;
		info.renderTargets[0].clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
		info.renderTargets[1].renderTarget = mVoxelizationContext.normalTarget;
		info.renderTargets[1].clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
		info.renderTargets[2].renderTarget = mVoxelizationContext.worldPosTarget;
		info.renderTargets[2].clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
		info.renderTargets[3].renderTarget = mRenderWindow->getColorTarget();
		info.renderTargets[3].clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
		info.depthTarget.depthStencil = mVoxelizationContext.depthTarget;

		info.depthTarget.clearValue = { 1.0f, 0.0f };
		info.passName = "sceneGeometryPass";
		
		
		renderScene(mGameCamera->getCamera(), mSceneManager, 
			info, pUserDefineShader);
		};

	FrameConstantBuffer* pFrameBuffer = &mFrameConstantBuffer;
	UpdatePassCallback updateCallback = [=, this](float delta) {
		uint32_t frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		updateFrameData(mGameCamera->getCamera(), nullptr, *pFrameBuffer);
		mRenderSystem->updateBufferObject(
			frameIndex? firstFrameBufferHandle:zeroFrameBufferHandle, 
			(const char*)pFrameBuffer, sizeof(mFrameConstantBuffer));
		};


	auto sceneGeometryPass = createUserDefineRenderPass(
		sceneGeometryPassCallback, updateCallback);
	mRenderPipeline->addRenderPass(sceneGeometryPass);
}

void VoxelConeTracingApp::shadowPass()
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	TextureProperty texProperty;
	texProperty._width = ogreConfig.width;
	texProperty._height = ogreConfig.height;
	texProperty._tex_format = Ogre::PixelFormat::PF_A8R8G8B8;
	texProperty._tex_usage = Ogre::TextureUsage::COLOR_ATTACHMENT;
	mVoxelizationContext.albedoTarget = mRenderSystem->createRenderTarget("albedoTarget", texProperty);
	mVoxelizationContext.normalTarget = mRenderSystem->createRenderTarget("normalTarget", texProperty);
	mVoxelizationContext.worldPosTarget = mRenderSystem->createRenderTarget("worldPos", texProperty);
	texProperty._width = 2048;
	texProperty._height = 2048;
	texProperty._tex_format = Ogre::PixelFormat::PF_DEPTH32F;
	texProperty._tex_usage = Ogre::TextureUsage::DEPTH_ATTACHMENT;
	mVoxelizationContext.depthTarget = mRenderSystem->createRenderTarget(
		"depthTarget", texProperty);

	ShaderInfo shaderInfo;
	shaderInfo.shaderName = "vctShadowPass";
	//shaderInfo.shaderMacros.push_back(std::pair<std::string, std::string>("PBR", "1"));
	VertexDeclaration decl;
	decl.addElement(0, 0, 0, VET_FLOAT3, VES_POSITION);
	decl.addElement(0, 0, 12, VET_FLOAT3, VES_NORMAL);
	decl.addElement(0, 0, 24, VET_FLOAT2, VES_TEXTURE_COORDINATES);
	auto programHandle = mRenderSystem->createShaderProgram(shaderInfo, &decl);
	backend::RasterState rasterState{};
	auto targetCount = 0;
	rasterState.depthFunc = SamplerCompareFunc::LE;
	rasterState.renderTargetCount = targetCount;
	rasterState.depthWrite = true;
	rasterState.depthTest = true;
	rasterState.pixelFormat = Ogre::PixelFormat::PF_A8R8G8B8;
	auto pipelineHandle = mRenderSystem->createPipeline(rasterState, programHandle);

	VoxelizationContext* context = &mVoxelizationContext;

	static Ogre::Camera mShadowCamera("", nullptr);

	Ogre::Vector3 eyePos(0.0f, 0.0f, 0.0f);
	Ogre::Vector3 eyeTarget(-0.191f, -1.0f, -0.574f);

	Ogre::Matrix4 view;

	view = Ogre::Math::makeLookAtRH(eyePos, eyeTarget, Ogre::Vector3::UNIT_Y);

	mShadowCamera.updateViewMatrix(view);

	
	uint32_t size = 256;
	Real left = -256 / 2.0f;
	Real right = 256 / 2.0f;
	Real top = 256 / 2.0f;
	Real bottom = -256 / 2.0f;

	Ogre::Matrix4 project = Ogre::Math::makeOrthoRH(left, right, bottom, top, -256, 256);
	mShadowCamera.updateProjectMatrix(project);

	auto viewproject = project * view;
	Ogre::Camera* lightCam = &mShadowCamera;

	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(mFrameConstantBuffer);
	Handle<HwBufferObject> zeroFrameBufferHandle = mRenderSystem->createBufferObject(desc);
	Handle<HwBufferObject> firstFrameBufferHandle = mRenderSystem->createBufferObject(desc);

	static UserDefineShader userDefineShader;
	userDefineShader.initCallback = initFrameResource;
	RenderableBindCallback bindCallback = [=, this](uint32_t frameIndex, Renderable* r) {
			if (r->hasFlag(shadowPassBit + frameIndex))
			{
				return;
			}
			r->setFlag(shadowPassBit + frameIndex, true);
			DescriptorData descriptorData;
			Handle<HwBufferObject> tmp[2];
			tmp[0] = zeroFrameBufferHandle;
			tmp[1] = firstFrameBufferHandle;

			descriptorData.mCount = 1;
			descriptorData.pName = "cbPass";
			descriptorData.ppBuffers = &tmp[frameIndex];
			FrameResourceInfo* resourceInfo = (FrameResourceInfo*)r->getFrameResourceInfo(frameIndex);
			mRenderSystem->updateDescriptorSet(resourceInfo->zeroShadowSet, 1, &descriptorData);
		};
	userDefineShader.bindCallback = bindCallback;
	RenderableDrawCallback drawCallback = [=, this](uint32_t frameIndex, Renderable* r) {
		void* frameData = r->getFrameResourceInfo(frameIndex);
		FrameResourceInfo* resourceInfo = (FrameResourceInfo*)frameData;
		Ogre::Material* mat = r->getMaterial().get();

		mRenderSystem->bindPipeline(programHandle, pipelineHandle, &resourceInfo->zeroShadowSet, 1);


		VertexData* vertexData = r->getVertexData();
		IndexData* indexData = r->getIndexData();
		vertexData->bind(nullptr);
		indexData->bind();
		IndexDataView* view = r->getIndexView();
		mRenderSystem->drawIndexed(view->mIndexCount, 1,
			view->mIndexLocation, view->mBaseVertexLocation, 0);
		};
	userDefineShader.drawCallback = drawCallback;

	UserDefineShader* pUserDefineShader = &userDefineShader;
	RenderPassCallback shadowPassCallback = [=, this](RenderPassInfo& info) {
		info.renderTargetCount = targetCount;
		info.depthTarget.depthStencil = mVoxelizationContext.depthTarget;
		info.depthTarget.clearValue = { 1.0f, 0.0f };
		info.passName = "vctShadowPass";
		renderScene(lightCam, mSceneManager,
			info, pUserDefineShader);
		};

	FrameConstantBuffer* pFrameBuffer = &mFrameConstantBuffer;
	UpdatePassCallback updateCallback = [=, this](float delta) {
		uint32_t frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		updateFrameData(lightCam, nullptr, *pFrameBuffer);
		mRenderSystem->updateBufferObject(
			frameIndex? firstFrameBufferHandle: zeroFrameBufferHandle,
			(const char*)pFrameBuffer, sizeof(mFrameConstantBuffer));
		};

	auto shadowPass = createUserDefineRenderPass(
		shadowPassCallback, updateCallback);
	mRenderPipeline->addRenderPass(shadowPass);
}

void VoxelConeTracingApp::initScene()
{
	Ogre::Matrix4 m;
	Ogre::Vector3 pos;

	m = Ogre::Math::makeRotateMatrix(-90, Ogre::Vector3::UNIT_X);
	addEntry("room", "room.fbx", Ogre::Vector3::ZERO, m, Ogre::Vector4(0.7, 0.7, 0.7, 0.0));
	addEntry("dragon", "dragon.fbx", Ogre::Vector3(1.5f, 0.0f, -7.0f), 
		Ogre::Matrix4::IDENTITY, Ogre::Vector4(0.044f, 0.627f, 0, 0.0));
	m = Ogre::Math::makeRotateMatrix(-0.3752457f / Ogre::Math::PI * 180, Ogre::Vector3::UNIT_Y);
	addEntry("bunny", "bunny.fbx", Ogre::Vector3(21.0f, 13.9f, -19.0f),
		m, Ogre::Vector4(0.8f, 0.71f, 0, 0.0));
	m = Ogre::Math::makeRotateMatrix(-90, Ogre::Vector3::UNIT_X);
	m = m * Ogre::Math::makeRotateMatrix(-1.099557f / Ogre::Math::PI * 180, Ogre::Vector3::UNIT_X);
	addEntry("torus", "torus.fbx", Ogre::Vector3(21.0f, 4.0f, -9.6f), m, Ogre::Vector4(0.329f, 0.26f, 0.8f, 0.8f));
	addEntry("sphere_big", "sphere_big.fbx", Ogre::Vector3(-17.25f, -1.15f, -24.15f), 
		Ogre::Matrix4::IDENTITY, Ogre::Vector4(0.692f, 0.215f, 0.0f, 0.6f));
	addEntry("sphere_medium", "sphere_medium.fbx", Ogre::Vector3(-21.0f, -0.95f, -13.20f), 
		Ogre::Matrix4::IDENTITY, Ogre::Vector4(0.005, 0.8, 0.426, 0.7f));
	addEntry("sphere_small", "sphere_small.fbx", Ogre::Vector3(-11.25f, -0.45f, -16.20f), 
		Ogre::Matrix4::IDENTITY, Ogre::Vector4(0.01, 0.0, 0.8, 0.75f));
	m = Ogre::Math::makeRotateMatrix(-90, Ogre::Vector3::UNIT_X);
	addEntry("block", "block.fbx", Ogre::Vector3(3.0f, 8.0f, -30.0f), m, Ogre::Vector4(0.9, 0.15, 1.0, 0.0));
	addEntry("cube", "cube.fbx", Ogre::Vector3(21.0f, 5.0f, -19.0f), m, Ogre::Vector4(0.1, 0.75, 0.8, 0.0));
}

void VoxelConeTracingApp::addEntry(
	const std::string& entryName,
	const std::string& meshName,
	const Ogre::Vector3& position,
	const Ogre::Matrix4& rotate,
	const Ogre::Vector4& color
)
{
	auto mesh = MeshManager::getSingletonPtr()->load(meshName);

	SceneNode* root = mSceneManager->getRoot()->createChildSceneNode("root");

	Entity* entry = mSceneManager->createEntity(entryName, meshName);
	SceneNode* node = root->createChildSceneNode(entryName);

	node->setPosition(position);
	Ogre::Quaternion q = rotate.extractQuaternion();
	
	node->setOrientation(q);
	entry->setColor(color);
	node->attachObject(entry);

	SubEntity* subEntity = entry->getSubEntity(0);

	const auto& modelMatrix  = subEntity->getModelMatrix();

	int kk = 0;
}




