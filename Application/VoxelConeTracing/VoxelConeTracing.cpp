#include "OgreHeader.h"
#include "VoxelConeTracing.h"
#include "engine_manager.h"
#include "OgreParticleSystem.h"
#include "OGImpact.h"
#include "OGImpactManager.h"
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
	mGameCamera   = gameCamera;
	mSceneManager = sceneManager;
	mRenderWindow = renderWindow;
	std::string meshname = "sponza.gltf";
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
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();

	float aspectInverse = ogreConfig.height / (float)ogreConfig.width;
	Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrixLHReverseZ(
		Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 5000.f);
	gameCamera->getCamera()->updateProjectMatrix(m);
	mRenderSystem = rs;
	
	auto width = ogreConfig.width;
	auto height = ogreConfig.height;
	mFrameConstantBuffer.RenderTargetSize =
		Ogre::Vector2((float)width,
			(float)height);
	mFrameConstantBuffer.InvRenderTargetSize =
		Ogre::Vector2(1.0f / width, 1.0f / height);
	mFrameConstantBuffer.NearZ = 0.1f;
	mFrameConstantBuffer.FarZ = 10000.0f;
	mFrameBufferObjectList.resize(ogreConfig.swapBufferCount);


	for (auto i = 0; i < ogreConfig.swapBufferCount; i++)
	{
		mFrameBufferObjectList[i] =
			rs->createBufferObject(BufferObjectBinding::BufferObjectBinding_Uniform, BufferUsage::DYNAMIC, sizeof(mFrameConstantBuffer));
	}

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
		auto* albedoTarget = rs->createRenderTarget("albedoTarget",
			width, height, Ogre::PixelFormat::PF_A8R8G8B8, Ogre::TextureUsage::COLOR_ATTACHMENT);
		auto* normalTarget = rs->createRenderTarget("normalTarget",
			width, height, Ogre::PixelFormat::PF_A8R8G8B8, Ogre::TextureUsage::COLOR_ATTACHMENT);
		auto* metalRoughnessTarget = rs->createRenderTarget("metalRoughnessTarget",
			width, height, Ogre::PixelFormat::PF_A8R8G8B8, Ogre::TextureUsage::COLOR_ATTACHMENT);
		auto* emissionTarget = rs->createRenderTarget("emissionTarget",
			width, height, Ogre::PixelFormat::PF_A8R8G8B8, Ogre::TextureUsage::COLOR_ATTACHMENT);
		auto* depthTarget = mRenderSystem->createRenderTarget(
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
		RenderPassCallback sceneGeometryPassCallback = [=, this](RenderPassInfo& info) {
			info.renderTargetCount = targetCount;
			info.renderTargets[0].renderTarget = albedoTarget;
			info.renderTargets[0].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
			info.renderTargets[1].renderTarget = normalTarget;
			info.renderTargets[1].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
			info.renderTargets[2].renderTarget = metalRoughnessTarget;
			info.renderTargets[2].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
			info.renderTargets[3].renderTarget = emissionTarget;
			info.renderTargets[3].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
			info.depthTarget.depthStencil = depthTarget;
			
			info.depthTarget.clearValue = { 0.0f, 0.0f };
			info.cam = cam;
			static EngineRenderList engineRenerList;
			sceneManager->getSceneRenderList(cam, engineRenerList, false);
			for (auto r : engineRenerList.mOpaqueList)
			{
				updateObject(r, pipelineHandle);
			}

			updateFrameData(cam, nullptr);
			rs->beginRenderPass(info);
			for (auto r : engineRenerList.mOpaqueList)
			{
				renderObject(r, programHandle, pipelineHandle);
			}
			rs->endRenderPass(info);
			};
		UpdatePassCallback updateCallback = [=, this](float delta) {
			};

		
		auto sceneGeometryPass = createUserDefineRenderPass(
			sceneGeometryPassCallback, updateCallback);
		renderPipeline->addRenderPass(sceneGeometryPass);
	}

	//VoxelizationPass
	VoxelizationPass::VoxelizationPassInfo voxelizaitonPassInfo;
	voxelizaitonPassInfo.renderPipeline = renderPipeline;

	VoxelizationPass* voxelizationPass = new VoxelizationPass(voxelizaitonPassInfo);
	voxelizationPass->init(16.f);
}

void VoxelConeTracingApp::update(float delta)
{
}

void VoxelConeTracingApp::renderObject(
	Ogre::Renderable* r, 
	Handle<HwProgram> programHandle,
	Handle<HwPipeline> pipelineHandle)
{
	Ogre::Material* mat = r->getMaterial().get();
	auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
	FrameResourceInfo* resourceInfo = r->getFrameResourceInfo(frameIndex);
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
}

void VoxelConeTracingApp::updateObject(Ogre::Renderable* r, Handle<HwPipeline> pipelineHandle)
{
	auto& ogreConfig = ::Root::getSingleton().getEngineConfig();
	auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
	Ogre::Material* mat = r->getMaterial().get();
	
	if (!mat->isLoaded())
	{
		mat->load(nullptr);
		r->createFrameResource();
		for (auto i = 0; i < ogreConfig.swapBufferCount; i++)
		{
			FrameResourceInfo* resourceInfo = r->getFrameResourceInfo(i);
			auto frameHandle = mFrameBufferObjectList[i];
			mRenderSystem->updateDescriptorSetBuffer(resourceInfo->zeroSet, 1, &frameHandle, 1);
			mRenderSystem->updateDescriptorSetBuffer(resourceInfo->zeroShadowSet, 1, &frameHandle, 1);
		}
	}
	r->updateFrameResource(frameIndex);
}

void VoxelConeTracingApp::updateFrameData(ICamera* camera, ICamera* light)
{
	RenderSystem* rs = mRenderSystem;
	const Ogre::Matrix4& view = camera->getViewMatrix();
	const Ogre::Matrix4& proj = camera->getProjectMatrix();
	const Ogre::Vector3& camepos = mGameCamera->getPosition();

	Ogre::Matrix4 invView = view.inverse();
	Ogre::Matrix4 viewProj = proj * view;
	Ogre::Matrix4 invProj = proj.inverse();
	Ogre::Matrix4 invViewProj = viewProj.inverse();

	mFrameConstantBuffer.View = view.transpose();
	mFrameConstantBuffer.InvView = invView.transpose();
	mFrameConstantBuffer.Proj = proj.transpose();
	mFrameConstantBuffer.InvProj = invProj.transpose();
	mFrameConstantBuffer.ViewProj = viewProj.transpose();
	mFrameConstantBuffer.InvViewProj = invViewProj.transpose();

	mFrameConstantBuffer.EyePosW = camepos;

	if (light)
	{
		mFrameConstantBuffer.ShadowTransform = 
			(light->getProjectMatrix() * light->getViewMatrix()).transpose();
		mFrameConstantBuffer.Shadow = 1;

		mFrameConstantBuffer.numDirLights = 1;
		mFrameConstantBuffer.directionLights[0].lightViewProject =
			(light->getProjectMatrix() * light->getViewMatrix()).transpose();
		mFrameConstantBuffer.directionLights[0].Direction = Ogre::Vector3(-100.0f, -100.0f, 0.0f);
		mFrameConstantBuffer.directionLights[0].Direction.normalise();
	}
	else
	{
		mFrameConstantBuffer.Shadow = 0;
	}


	mFrameConstantBuffer.TotalTime += Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;
	mFrameConstantBuffer.DeltaTime = Ogre::Root::getSingleton().getFrameEvent().timeSinceLastFrame;

	auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();

	rs->updateBufferObject(
		mFrameBufferObjectList[frameIndex], (const char*)&mFrameConstantBuffer, sizeof(mFrameConstantBuffer));

}