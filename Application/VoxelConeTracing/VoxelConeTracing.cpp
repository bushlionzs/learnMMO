#include "OgreHeader.h"
#include "VoxelConeTracing.h"
#include "OgreParticleSystem.h"
#include "myutils.h"
#include "OgreResourceManager.h"
#include "OgreMaterialManager.h"
#include "OgreMeshManager.h"
#include "OgreSceneManager.h"
#include "OgreEntity.h"
#include "OgreCamera.h"
#include "OgreRenderWindow.h"
#include "renderSystem.h"
#include "OgreRoot.h"
#include "OgreRenderable.h"
#include "OgreVertexData.h"
#include "OgreTextureUnit.h"
#include "VoxelDef.h"
#include "VoxelizationPass.h"
#include "WrapBorderPass.h"
#include "GIPass.h"
#include "renderUtil.h"

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
	ogreConfig.reverseDepth = true;
	mGameCamera   = gameCamera;
	mSceneManager = sceneManager;
	mRenderWindow = renderWindow;
	std::string meshname = "Sponza.gltf";
	std::shared_ptr<Mesh> mesh = MeshManager::getSingletonPtr()->load(meshname);
	auto rootNode = sceneManager->getRoot();
	Entity* sponza = sceneManager->createEntity(meshname, meshname);
	SceneNode* sponzaNode = rootNode->createChildSceneNode(meshname);
	sponzaNode->attachObject(sponza);
	auto h = 20.0f;
	auto camPos = Ogre::Vector3(0, h, 0.0f);
	auto targetPos = Ogre::Vector3(-1, h, 0.0f);
	gameCamera->lookAt(camPos, targetPos);
	gameCamera->setMoveSpeed(200);
	

	float aspectInverse = ogreConfig.height / (float)ogreConfig.width;
	Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrixReverseZ(
		Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 5000.f);
	gameCamera->getCamera()->updateProjectMatrix(m);
	mRenderSystem = rs;

	auto* cam = gameCamera->getCamera();
	auto* light = sceneManager->createLight("light");
	if(0)
	{
		RenderPassInput input;
		input.color = renderWindow->getColorTarget();
		input.depth = renderWindow->getDepthTarget();
		input.cam = cam;
		input.sceneMgr = sceneManager;
		auto mainPass = createStandardRenderPass(input);
		renderPipeline->addRenderPass(mainPass);
		return;
	}
	
	
	VertexData* vertexData = mesh->getVertexData();
	VertexDeclaration* vertexDecl = vertexData->getVertexDeclaration();
	auto numFrame = ogreConfig.swapBufferCount;
	mFrameData.resize(numFrame);
	//SceneGeometryPass
	if(1)
	{
		mVoxelizationContext.albedoTarget = rs->createRenderTarget("albedoTarget",
			ogreConfig.width, ogreConfig.height, Ogre::PixelFormat::PF_A8R8G8B8, Ogre::TextureUsage::COLOR_ATTACHMENT);
		mVoxelizationContext.normalTarget = rs->createRenderTarget("normalTarget",
			ogreConfig.width, ogreConfig.height, Ogre::PixelFormat::PF_A8R8G8B8, Ogre::TextureUsage::COLOR_ATTACHMENT);
		mVoxelizationContext.metalRoughnessTarget = rs->createRenderTarget("metalRoughnessTarget",
			ogreConfig.width, ogreConfig.height, Ogre::PixelFormat::PF_A8R8G8B8, Ogre::TextureUsage::COLOR_ATTACHMENT);
		mVoxelizationContext.emissionTarget = rs->createRenderTarget("emissionTarget",
			ogreConfig.width, ogreConfig.height, Ogre::PixelFormat::PF_A8R8G8B8, Ogre::TextureUsage::COLOR_ATTACHMENT);
		mVoxelizationContext.depthTarget = mRenderSystem->createRenderTarget(
			"depthTarget", 2048, 2048, Ogre::PF_DEPTH32F,
			Ogre::TextureUsage::DEPTH_ATTACHMENT);

		ShaderInfo shaderInfo;
		shaderInfo.shaderName = "sceneGeometryPass";
		shaderInfo.shaderMacros.push_back(std::pair<std::string, std::string>("PBR", "1"));
		auto programHandle = rs->createShaderProgram(shaderInfo, nullptr);
		backend::RasterState rasterState{};
		auto targetCount = 4;
		rasterState.depthFunc = SamplerCompareFunc::GE;
		rasterState.renderTargetCount = targetCount;
		rasterState.depthWrite = true;
		rasterState.depthTest = true;
		rasterState.pixelFormat = Ogre::PixelFormat::PF_A8R8G8B8;
		auto pipelineHandle = rs->createPipeline(rasterState, programHandle);

		VoxelizationContext* context = &mVoxelizationContext;
		RenderPassCallback sceneGeometryPassCallback = [=, this](RenderPassInfo& info) {
			info.renderTargetCount = targetCount;
			info.renderTargets[0].renderTarget = mVoxelizationContext.albedoTarget;
			info.renderTargets[0].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
			info.renderTargets[1].renderTarget = mVoxelizationContext.normalTarget;
			info.renderTargets[1].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
			info.renderTargets[2].renderTarget = mVoxelizationContext.metalRoughnessTarget;
			info.renderTargets[2].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
			info.renderTargets[3].renderTarget = mVoxelizationContext.emissionTarget;
			info.renderTargets[3].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
			info.depthTarget.depthStencil = mVoxelizationContext.depthTarget;
			
			info.depthTarget.clearValue = { 0.0f, 0.0f };

			rs->beginRenderPass(info);
			renderSceneManager(*context);
			rs->endRenderPass(info);
			};
		UpdatePassCallback updateCallback = [=, this](float delta) {
			};

		
		auto sceneGeometryPass = createUserDefineRenderPass(
			sceneGeometryPassCallback, updateCallback);
		renderPipeline->addRenderPass(sceneGeometryPass);
	}

	//VoxelizationPass

	mVoxelizationContext.renderPipeline = renderPipeline;

	VoxelizationPass* voxelizationPass = new VoxelizationPass(mVoxelizationContext);
	voxelizationPass->init(16.f);

	WrapBorderPass* wrapBorderPass = new WrapBorderPass;

	GIPass* giPass = new GIPass;
}

void VoxelConeTracingApp::update(float delta)
{
}





