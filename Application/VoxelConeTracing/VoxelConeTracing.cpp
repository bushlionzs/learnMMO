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

	auto h = 7.0f;
	auto camPos = Ogre::Vector3(0.0f, h, 33.0f);
	auto targetPos = Ogre::Vector3(0.0f, h, 0.0f);
	gameCamera->lookAt(camPos, targetPos);
	gameCamera->setMoveSpeed(20);


	float aspect = ogreConfig.width / (float)ogreConfig.height;
	Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrix(
		Ogre::Math::PI / 3.0f, aspect, 0.1, 1000.0f);

	gameCamera->getCamera()->updateProjectMatrix(m);

	initScene();
	
	mRenderSystem = rs;

	auto* cam = gameCamera->getCamera();
	auto* light = sceneManager->createLight("light");

	if(1)
	{
		sceneGeometryPass();
	}

	if (1)
	{
		shadowPass();
	}

	if (1)
	{
		voxelizationPass();
	}

	if (1)
	{
		computePass();
	}
}

void VoxelConeTracingApp::update(float delta)
{
}

void VoxelConeTracingApp::sceneGeometryPass()
{
	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(mFrameConstantBuffer);
	Handle<HwBufferObject> zeroFrameBufferHandle = mRenderSystem->createBufferObject(desc);
	Handle<HwBufferObject> firstFrameBufferHandle = mRenderSystem->createBufferObject(desc);

	VoxelizationContext* context = &mVoxelizationContext;
	static UserDefineShader userDefineShader;
	userDefineShader.initCallback = std::bind(
		&VoxelConeTracingApp::initFrameResource, this, std::placeholders::_1, std::placeholders::_2);
	RenderableBindCallback bindCallback = [=, this](uint32_t frameIndex, Renderable* r) {
		if (r->hasFlag(sceneGeometryPassBit + frameIndex))
		{
			return;
		}
		r->setFlag(sceneGeometryPassBit + frameIndex, true);
		DescriptorData descriptorData[2];
		Handle<HwBufferObject> tmp[2];
		tmp[0] = zeroFrameBufferHandle;
		tmp[1] = firstFrameBufferHandle;
		
		VctFrameResourceInfo* resourceInfo = (VctFrameResourceInfo*)r->getFrameResourceInfo(frameIndex);
		descriptorData[0].mCount = 1;
		descriptorData[0].pName = "cbPass";
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[0].ppBuffers = &zeroFrameBufferHandle;
		
		mRenderSystem->updateDescriptorSet(resourceInfo->zeroSet, 1, descriptorData);

		};
	userDefineShader.bindCallback = bindCallback;

	RenderableDrawCallback drawCallback = [=, this](uint32_t frameIndex, Renderable* r) {
		void* frameData = r->getFrameResourceInfo(frameIndex);
		VctFrameResourceInfo* resourceInfo = (VctFrameResourceInfo*)frameData;
		Ogre::Material* mat = r->getMaterial().get();

		Handle<HwDescriptorSet> descriptorSet[2];
		descriptorSet[0] = resourceInfo->zeroSet;
		descriptorSet[1] = resourceInfo->firstSet;
		mRenderSystem->bindPipeline(mSceneGeometryPipelineHandle, descriptorSet, 2);


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
		
		mRenderSystem->pushGroupMarker("sceneGeometryPass");
		renderScene(mGameCamera->getCamera(), mSceneManager, 
			info, pUserDefineShader);
		mRenderSystem->popGroupMarker();
		};

	FrameConstantBuffer* pFrameBuffer = &mFrameConstantBuffer;
	UpdatePassCallback updateCallback = [=, this](float delta) {
		uint32_t frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		updateFrameData(mGameCamera->getCamera(), nullptr, *pFrameBuffer);
		mRenderSystem->updateBufferObject(
			frameIndex? firstFrameBufferHandle:zeroFrameBufferHandle, 
			(const char*)pFrameBuffer, sizeof(mFrameConstantBuffer));
		EngineRenderList renderList;
		mSceneManager->getSceneRenderList(mGameCamera->getCamera(), renderList, false);
		for (auto r : renderList.mOpaqueList)
		{
			updateFrameResource(frameIndex, r);
		}
		};


	auto sceneGeometryPass = createUserDefineRenderPass(
		sceneGeometryPassCallback, updateCallback);
	mRenderPipeline->addRenderPass(sceneGeometryPass);
}

void VoxelConeTracingApp::shadowPass()
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	

	

	VoxelizationContext* context = &mVoxelizationContext;

	static Ogre::Camera mShadowCamera("", nullptr);

	mLightView = Ogre::Math::makeLookAtRH(
		Ogre::Vector3(0.0f, 0.0f, 0.0f), Ogre::Vector3(-0.191f, -1.0f, -0.574f), Ogre::Vector3::UNIT_Y);

	mShadowCamera.updateViewMatrix(mLightView);

	
	uint32_t size = 256;
	Real left = -256 / 2.0f;
	Real right = 256 / 2.0f;
	Real top = 256 / 2.0f;
	Real bottom = -256 / 2.0f;

	mLightProject = Ogre::Math::makeOrthoRH(left, right, bottom, top, -256, 256);
	mShadowCamera.updateProjectMatrix(mLightProject);

	Ogre::Camera* lightCam = &mShadowCamera;

	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(mFrameConstantBuffer);
	Handle<HwBufferObject> zeroFrameBufferHandle = mRenderSystem->createBufferObject(desc);
	Handle<HwBufferObject> firstFrameBufferHandle = mRenderSystem->createBufferObject(desc);

	static UserDefineShader userDefineShader;
	userDefineShader.initCallback = std::bind(
		&VoxelConeTracingApp::initFrameResource, this, std::placeholders::_1, std::placeholders::_2);
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
		VctFrameResourceInfo* resourceInfo = (VctFrameResourceInfo*)frameData;
		Ogre::Material* mat = r->getMaterial().get();

		mRenderSystem->bindPipeline(mShadowPipelineHandle, &resourceInfo->zeroShadowSet, 1);


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
		info.renderTargetCount = 0;
		info.depthTarget.depthStencil = mVoxelizationContext.depthTarget;
		info.depthTarget.clearValue = { 1.0f, 0.0f };
		info.passName = "vctShadowPass";
		mRenderSystem->pushGroupMarker("vctShadowPass");
		renderScene(lightCam, mSceneManager,
			info, pUserDefineShader);
		mRenderSystem->popGroupMarker();
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

void VoxelConeTracingApp::voxelizationPass()
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	
	VoxelizationContext* context = &mVoxelizationContext;

	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(VoxelizationBlock);
	Handle<HwBufferObject> zeroVoxelizationBlockHandle = mRenderSystem->createBufferObject(desc);
	Handle<HwBufferObject> firstVoxelizationBlockHandle = mRenderSystem->createBufferObject(desc);

	static UserDefineShader userDefineShader;
	userDefineShader.initCallback = std::bind(
		&VoxelConeTracingApp::initFrameResource, this, std::placeholders::_1, std::placeholders::_2);
	RenderableBindCallback bindCallback = [=, this](uint32_t frameIndex, Renderable* r) {
		if (r->hasFlag(voxelizationPassBit + frameIndex))
		{
			return;
		}
		r->setFlag(voxelizationPassBit + frameIndex, true);
		DescriptorData descriptorData;
		Handle<HwBufferObject> tmp[2];
		tmp[0] = zeroVoxelizationBlockHandle;
		tmp[1] = firstVoxelizationBlockHandle;

		descriptorData.mCount = 1;
		descriptorData.pName = "VoxelizationCB";
		descriptorData.ppBuffers = &tmp[frameIndex];
		VctFrameResourceInfo* resourceInfo = (VctFrameResourceInfo*)r->getFrameResourceInfo(frameIndex);
		mRenderSystem->updateDescriptorSet(resourceInfo->zeroSetOfVoxelization, 1, &descriptorData);
		};
	userDefineShader.bindCallback = bindCallback;
	RenderableDrawCallback drawCallback = [=, this](uint32_t frameIndex, Renderable* r) {
		void* frameData = r->getFrameResourceInfo(frameIndex);
		VctFrameResourceInfo* resourceInfo = (VctFrameResourceInfo*)frameData;
		Ogre::Material* mat = r->getMaterial().get();

		mRenderSystem->bindPipeline(mVoxellizationPipelineHandle, 
			&resourceInfo->zeroSetOfVoxelization, 1);


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
	RenderPassCallback voxelizationPassCallback = [=, this](RenderPassInfo& info) {
		VoxelizationContext* context = &this->mVoxelizationContext;
		info.renderTargetCount = 0;
		info.depthTarget.depthStencil = nullptr;
		info.depthTarget.clearValue = { 1.0f, 0.0f };
		info.passName = "VoxelizationPass";
		info.extent[0] = VCT_SCENE_VOLUME_SIZE;
		info.extent[1] = VCT_SCENE_VOLUME_SIZE;
		RenderTargetBarrier rtBarriers[] =
		{
			{
				 context->depthTarget,
				RESOURCE_STATE_DEPTH_WRITE,
				RESOURCE_STATE_SHADER_RESOURCE
			}
		};
		mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);
		mRenderSystem->pushGroupMarker("VoxelizationPass");
		mRenderSystem->clearRenderTarget(context->voxelizationTarget, Ogre::Vector4::ZERO);
		rtBarriers[0] = {
			context->voxelizationTarget,
			RESOURCE_STATE_UNORDERED_ACCESS,
			RESOURCE_STATE_UNORDERED_ACCESS
		};
		mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);
		renderScene(mGameCamera->getCamera(), mSceneManager,
			info, pUserDefineShader);
		mRenderSystem->popGroupMarker();
		rtBarriers[0] = {
			context->depthTarget,
				RESOURCE_STATE_SHADER_RESOURCE,
				RESOURCE_STATE_DEPTH_WRITE
		};
		mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);
		};
	mVoxelizationBlock.worldVoxelScale = 128.0f;
	const Ogre::Matrix4& view = mGameCamera->getCamera()->getViewMatrix();
	const Ogre::Matrix4& proj = mGameCamera->getCamera()->getProjectMatrix();
	mVoxelizationBlock.viewProjection = (proj * view).transpose();

	auto translate = Ogre::Math::makeTranslateMatrix(
		Ogre::Vector3(-VCT_SCENE_VOLUME_SIZE * 0.25f, 
			-VCT_SCENE_VOLUME_SIZE * 0.25f, 
			-VCT_SCENE_VOLUME_SIZE * 0.25f));
	auto scale = Ogre::Math::makeScaleMatrix(Ogre::Vector3(1.0f, -1.0f, 1.0f));
	mVoxelizationBlock.worldVoxelCube = (scale * translate).transpose();
	mVoxelizationBlock.shadowViewProjection = (mLightProject * mLightView).transpose();
	mRenderSystem->updateBufferObject(
		 zeroVoxelizationBlockHandle, (const char*)&mVoxelizationBlock, sizeof(VoxelizationBlock));
	mRenderSystem->updateBufferObject(
		firstVoxelizationBlockHandle, (const char*)&mVoxelizationBlock, sizeof(VoxelizationBlock));
	UpdatePassCallback updateCallback = [=, this](float delta) {
		uint32_t frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		};

	auto voxelizationPass = createUserDefineRenderPass(
		voxelizationPassCallback, updateCallback);
	mRenderPipeline->addRenderPass(voxelizationPass);
}

void VoxelConeTracingApp::computePass()
{
	ComputePassCallback computeCallback = [=, this](ComputePassInfo& info) {
		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		VctFrameData* frameData = &this->mComputeFrameData[0];

		{
			RenderTargetBarrier barriers[] = {
			{
				mVoxelizationContext.voxelizationTarget,
				RESOURCE_STATE_UNORDERED_ACCESS,
				RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
			},
			{
				mVoxelizationContext.albedoTarget,
				RESOURCE_STATE_RENDER_TARGET,
				RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
			},
			{
				mVoxelizationContext.normalTarget,
				RESOURCE_STATE_RENDER_TARGET,
				RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
			},
			{
				mVoxelizationContext.worldPosTarget,
				RESOURCE_STATE_RENDER_TARGET,
				RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
			},
			};

			mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 4, barriers);
		}
		

		if (1)
		{
			info.programHandle = mMipmapPrepareHandle;
			info.computeGroup = Ogre::Vector3i(16, 16, 16);
			info.descSets.clear();
			info.descSets.push_back(frameData->mipmapPrepareZeroSet);
			mRenderSystem->pushGroupMarker("MipmapPrepare");
			mRenderSystem->beginComputePass(info);
			mRenderSystem->endComputePass();
			mRenderSystem->popGroupMarker();

			
		}
		
		if (1)
		{
			mRenderSystem->pushGroupMarker("VCT Mipmapping main CS");
			int mipDimension = VCT_SCENE_VOLUME_SIZE >> 1;
			info.programHandle = mMipmapMainHandle;
			for (uint32_t i = 0; i < VCT_MIPS; i++)
			{
				info.descSets.clear();
				info.descSets.push_back(frameData->mipmapResultZeroSet[i]);
				uint32_t count = std::max(1, mipDimension / 8);
				info.computeGroup = Ogre::Vector3i(count, count, count);
				
				mRenderSystem->beginComputePass(info);
				mRenderSystem->endComputePass();
				mipDimension >>= 1;
			}
			mRenderSystem->popGroupMarker();

			{
				RenderTargetBarrier barriers[] = {
				{
					mVoxelizationContext.posxResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_UNORDERED_ACCESS
				},
				{
					mVoxelizationContext.negxResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_UNORDERED_ACCESS
				},
				{
					mVoxelizationContext.posyResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_UNORDERED_ACCESS
				},
				{
					mVoxelizationContext.negyResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_UNORDERED_ACCESS
				},
				{
					mVoxelizationContext.poszResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_UNORDERED_ACCESS
				},
			   {
					mVoxelizationContext.negzResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_UNORDERED_ACCESS
				}
				};

				mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 6, barriers);
			}
		}
		
		if (1)
		{
			{
				RenderTargetBarrier barriers[] = {
				{
					mVoxelizationContext.posxResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
				},
				{
					mVoxelizationContext.negxResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
				},
				{
					mVoxelizationContext.posyResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
				},
				{
					mVoxelizationContext.negyResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
				},
				{
					mVoxelizationContext.poszResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
				},
			   {
					mVoxelizationContext.negzResultTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
				}
				};

				mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 6, barriers);
			}
			mRenderSystem->pushGroupMarker("VCT TracingCone CS");
			info.descSets.clear();
			info.descSets.push_back(frameData->tracingConeZeroSet);
			info.programHandle = mTracingConeHandle;
			info.computeGroup = Ogre::Vector3i(mVoxelizationContext.tracingResultTarget->getWidth() / 8,
				Ogre::Math::ICeil(mVoxelizationContext.tracingResultTarget->getHeight()/8.0f), 1);
			mRenderSystem->beginComputePass(info);
			mRenderSystem->endComputePass();
			mRenderSystem->popGroupMarker();

			{
				RenderTargetBarrier barriers[] = {
				{
					mVoxelizationContext.posxResultTarget,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
					RESOURCE_STATE_UNORDERED_ACCESS
				},
				{
					mVoxelizationContext.negxResultTarget,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
					RESOURCE_STATE_UNORDERED_ACCESS
				},
				{
					mVoxelizationContext.posyResultTarget,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
					RESOURCE_STATE_UNORDERED_ACCESS
				},
				{
					mVoxelizationContext.negyResultTarget,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
					RESOURCE_STATE_UNORDERED_ACCESS
				},
				{
					mVoxelizationContext.poszResultTarget,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
					RESOURCE_STATE_UNORDERED_ACCESS
				},
			   {
					mVoxelizationContext.negzResultTarget,
					RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
					RESOURCE_STATE_UNORDERED_ACCESS
				}
				};

				mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 6, barriers);
			}
		}

		{
			RenderTargetBarrier barriers[] = {
			{
				mVoxelizationContext.voxelizationTarget,
				RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				RESOURCE_STATE_UNORDERED_ACCESS
			},
			{
				mVoxelizationContext.albedoTarget,
				RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				RESOURCE_STATE_RENDER_TARGET
			},
			{
				mVoxelizationContext.normalTarget,
				RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				RESOURCE_STATE_RENDER_TARGET
			},
			{
				mVoxelizationContext.worldPosTarget,
				RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
				RESOURCE_STATE_RENDER_TARGET
			}
			};

			mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 4, barriers);
		}
		};
		auto* cam = mGameCamera->getCamera();
	UpdatePassCallback updateCallback = [=, this](float delta) {
		uint32_t frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		const Ogre::Matrix4& view = cam->getViewMatrix();
		const Ogre::Matrix4& proj = cam->getProjectMatrix();
		mVoxelizationBlock.viewProjection = (proj * view).transpose();

		mRenderSystem->updateBufferObject(tracingVoxelizationBlockHandle,
			(const char*)&this->mVoxelizationBlock, sizeof(mVoxelizationBlock));
		mVctMainBlock.CameraPos = cam->getDerivedPosition();
		mRenderSystem->updateBufferObject(tracingMainBlockHandle,
			(const char*)&this->mVctMainBlock, sizeof(VCTMainBlock));
		};
	
	mVctMainBlock.CameraPos = cam->getDerivedPosition();
	mVctMainBlock.UpsampleRatio = Ogre::Vector2(2.0f, 2.0f);
	mVctMainBlock.IndirectDiffuseStrength = 1.0f;
	mVctMainBlock.IndirectSpecularStrength = 1.0f;
	mVctMainBlock.MaxConeTraceDistance = 100.0f;
	mVctMainBlock.AOFalloff = 2.0f;
	mVctMainBlock.SamplingFactor = 0.5f;
	mVctMainBlock.VoxelSampleOffset = 0.0f;
	auto computePass = createComputePass(computeCallback, updateCallback);
	mRenderPipeline->addRenderPass(computePass);
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
	m = m * Ogre::Math::makeRotateMatrix(1.099557f / Ogre::Math::PI * 180, Ogre::Vector3::UNIT_X);
	addEntry("torus", "torus.fbx", Ogre::Vector3(21.0f, 4.0f, -9.6f), m, Ogre::Vector4(0.329f, 0.26f, 0.8f, 0.8f));
	addEntry("sphere_big", "sphere_big.fbx", Ogre::Vector3(-17.25f, -1.15f, -24.15f), 
		Ogre::Matrix4::IDENTITY, Ogre::Vector4(0.692f, 0.215f, 0.0f, 0.6f));
	addEntry("sphere_medium", "sphere_medium.fbx", Ogre::Vector3(-21.0f, -0.95f, -13.20f), 
		Ogre::Matrix4::IDENTITY, Ogre::Vector4(0.005, 0.8, 0.426, 0.7f));
	addEntry("sphere_small", "sphere_small.fbx", Ogre::Vector3(-11.25f, -0.45f, -16.20f), 
		Ogre::Matrix4::IDENTITY, Ogre::Vector4(0.01, 0.0, 0.8, 0.75f));
	m = Ogre::Math::makeRotateMatrix(-90, Ogre::Vector3::UNIT_X);
	addEntry("block", "block.fbx", Ogre::Vector3(3.0f, 8.0f, -30.0f), m, Ogre::Vector4(0.9, 0.15, 1.0, 0.0));
	m =  Ogre::Math::makeRotateMatrix(-0.907571f / Ogre::Math::PI * 180, Ogre::Vector3::UNIT_Y) * m;
	addEntry("cube", "cube.fbx", Ogre::Vector3(21.0f, 5.0f, -19.0f), m, Ogre::Vector4(0.1, 0.75, 0.8, 0.0));

	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	TextureProperty texProperty;
	texProperty._width = ogreConfig.width;
	texProperty._height = ogreConfig.height;
	texProperty._tex_format = Ogre::PixelFormat::PF_A8B8G8R8;
	texProperty._tex_usage = Ogre::TextureUsage::COLOR_ATTACHMENT;
	texProperty._need_mipmap = false;
	mVoxelizationContext.albedoTarget = mRenderSystem->createRenderTarget("albedoTarget", texProperty);
	texProperty._tex_format = Ogre::PixelFormat::PF_RGBA16_SNORM;
	mVoxelizationContext.normalTarget = mRenderSystem->createRenderTarget("normalTarget", texProperty);
	texProperty._tex_format = Ogre::PixelFormat::PF_FLOAT32_RGBA;
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
	mSceneGeometryProgramHandle = mRenderSystem->createShaderProgram(shaderInfo, &decl);
	backend::RasterState rasterState{};
	rasterState.depthBiasConstantFactor = 0.0f;
	rasterState.depthBiasSlopeFactor = 0.0f;
	auto targetCount = 4;
	rasterState.depthFunc = SamplerCompareFunc::LE;
	rasterState.renderTargetCount = targetCount;
	rasterState.depthWrite = true;
	rasterState.depthTest = true;
	rasterState.pixelFormat[0] = Ogre::PixelFormat::PF_A8B8G8R8;
	rasterState.pixelFormat[1] = Ogre::PixelFormat::PF_RGBA16_SNORM;
	rasterState.pixelFormat[2] = Ogre::PixelFormat::PF_FLOAT32_RGBA;
	rasterState.pixelFormat[3] = Ogre::PixelFormat::PF_A8R8G8B8;
	mSceneGeometryPipelineHandle = mRenderSystem->createPipeline(rasterState, mSceneGeometryProgramHandle);

	//
	shaderInfo.shaderName = "vctShadowPass";

	mShadowProgramHandle = mRenderSystem->createShaderProgram(shaderInfo, &decl);

	rasterState.depthFunc = SamplerCompareFunc::LE;
	rasterState.renderTargetCount = 0;
	rasterState.depthWrite = true;
	rasterState.depthTest = true;
	rasterState.depthBiasSlopeFactor = 10.0f;
	rasterState.pixelFormat[0] = Ogre::PixelFormat::PF_A8R8G8B8;
	mShadowPipelineHandle = mRenderSystem->createPipeline(rasterState, mShadowProgramHandle);
	rasterState.depthBiasSlopeFactor = 0.0f;
	//
	texProperty._texType = TEX_TYPE_3D;
	texProperty._width = 256;
	texProperty._height = 256;
	texProperty._depth = 256;
	texProperty._tex_format = Ogre::PixelFormat::PF_A8B8G8R8;
	texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
	texProperty._need_mipmap = false;
	mVoxelizationContext.voxelizationTarget = mRenderSystem->createRenderTarget("voxelizationTarget", texProperty);

	shaderInfo.shaderName = "voxelizationPass";
	mVoxellizationProgramHandle = mRenderSystem->createShaderProgram(shaderInfo, &decl);
	rasterState.depthFunc = SamplerCompareFunc::A;
	rasterState.renderTargetCount = 0;
	rasterState.depthWrite = false;
	rasterState.depthTest = false;
	rasterState.pixelFormat[0] = Ogre::PixelFormat::PF_A8R8G8B8;
	mVoxellizationPipelineHandle = mRenderSystem->createPipeline(rasterState, mVoxellizationProgramHandle);

	backend::SamplerParams params;
	params.filterMag = backend::SamplerFilterType::LINEAR;
	params.filterMin = backend::SamplerFilterType::LINEAR;
	params.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST;
	params.wrapS = backend::SamplerWrapMode::CLAMP_TO_BODY;
	params.wrapT = backend::SamplerWrapMode::CLAMP_TO_BODY;
	params.wrapR = backend::SamplerWrapMode::REPEAT;
	params.compareMode = backend::SamplerCompareMode::COMPARE_TO_TEXTURE;
	params.compareFunc = backend::SamplerCompareFunc::LE;
	params.anisotropyLog2 = 4;
	params.padding0 = 0;
	params.padding1 = 0;
	params.padding2 = 0;
	Handle<HwSampler>samplerHandle = mRenderSystem->createTextureSampler(params);
	mVoxelizationContext.voxelizationSampler = samplerHandle;
	//
	shaderInfo.shaderName = "mipmapPreparePass";
	mMipmapPrepareHandle = mRenderSystem->createComputeProgram(shaderInfo);

	texProperty._texType = TEX_TYPE_3D;
	texProperty._width = 128;
	texProperty._height = 128;
	texProperty._depth = 128;
	texProperty._tex_format = Ogre::PixelFormat::PF_A8B8G8R8;
	texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
	texProperty._need_mipmap = false;

	mVoxelizationContext.posxTarget = mRenderSystem->createRenderTarget("posxTarget", texProperty);
	mVoxelizationContext.negxTarget = mRenderSystem->createRenderTarget("negxTarget", texProperty);
	mVoxelizationContext.posyTarget = mRenderSystem->createRenderTarget("posyTarget", texProperty);
	mVoxelizationContext.negyTarget = mRenderSystem->createRenderTarget("negyTarget", texProperty);
	mVoxelizationContext.poszTarget = mRenderSystem->createRenderTarget("poszTarget", texProperty);
	mVoxelizationContext.negzTarget = mRenderSystem->createRenderTarget("negzTarget", texProperty);

	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(ObjectConstantBuffer);
	mVoxelizationContext.mipmapBlockHandle = mRenderSystem->createBufferObject(desc);
	MipmapBlock mipmapBlock;
	mipmapBlock.MipDimension = VCT_SCENE_VOLUME_SIZE >> 1;
	mipmapBlock.MipLevel = 0;
	mRenderSystem->updateBufferObject(mVoxelizationContext.mipmapBlockHandle,
		(const char*)&mipmapBlock, sizeof(mipmapBlock));
	mComputeFrameData.resize(ogreConfig.swapBufferCount);

	DescriptorData descriptorData[16];
	for (uint32_t i = 0; i < ogreConfig.swapBufferCount; i++)
	{
		VctFrameData* frameData = &mComputeFrameData[i];
		frameData->mipmapPrepareZeroSet = mRenderSystem->createDescriptorSet(mMipmapPrepareHandle, 0);

		descriptorData[0].pName = "MipmapCB";
		descriptorData[0].mCount = 1;
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[0].ppBuffers = &mVoxelizationContext.mipmapBlockHandle;

		OgreTexture* posxTexture = mVoxelizationContext.posxTarget->getTarget();
		descriptorData[1].pName = "voxelTextureResultPosX";
		descriptorData[1].mCount = 1;
		descriptorData[1].mLevel = 0;
		descriptorData[1].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[1].ppTextures = (const OgreTexture **)&posxTexture;

		OgreTexture* negxTexture = mVoxelizationContext.negxTarget->getTarget();
		descriptorData[2].pName = "voxelTextureResultNegX";
		descriptorData[2].mCount = 1;
		descriptorData[2].mLevel = 0;
		descriptorData[2].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[2].ppTextures = (const OgreTexture**)&negxTexture;

		OgreTexture* posyTexture = mVoxelizationContext.posyTarget->getTarget();
		descriptorData[3].pName = "voxelTextureResultPosY";
		descriptorData[3].mCount = 1;
		descriptorData[3].mLevel = 0;
		descriptorData[3].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[3].ppTextures = (const OgreTexture**)&posyTexture;

		OgreTexture* negyTexture = mVoxelizationContext.negyTarget->getTarget();
		descriptorData[4].pName = "voxelTextureResultNegY";
		descriptorData[4].mCount = 1;
		descriptorData[4].mLevel = 0;
		descriptorData[4].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[4].ppTextures = (const OgreTexture**)&negyTexture;

		OgreTexture* poszTexture = mVoxelizationContext.poszTarget->getTarget();
		descriptorData[5].pName = "voxelTextureResultPosZ";
		descriptorData[5].mCount = 1;
		descriptorData[5].mLevel = 0;
		descriptorData[5].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[5].ppTextures = (const OgreTexture**)&poszTexture;

		OgreTexture* negzTexture = mVoxelizationContext.negzTarget->getTarget();
		descriptorData[6].pName = "voxelTextureResultNegZ";
		descriptorData[6].mCount = 1;
		descriptorData[6].mLevel = 0;
		descriptorData[6].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[6].ppTextures = (const OgreTexture**)&negzTexture;

		OgreTexture* voxelTexture = mVoxelizationContext.voxelizationTarget->getTarget();
		descriptorData[7].pName = "voxelTexture";
		descriptorData[7].mCount = 1;
		descriptorData[7].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
		descriptorData[7].ppTextures = (const OgreTexture**)&voxelTexture;

		mRenderSystem->updateDescriptorSet(frameData->mipmapPrepareZeroSet, 8, descriptorData);
	}
	//

	shaderInfo.shaderName = "mipmapMainPass";
	mMipmapMainHandle = mRenderSystem->createComputeProgram(shaderInfo);

	texProperty._texType = TEX_TYPE_3D;
	texProperty._width = 128;
	texProperty._height = 128;
	texProperty._depth = 128;
	texProperty._tex_format = Ogre::PixelFormat::PF_A8B8G8R8;
	texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
	texProperty._need_mipmap = true;
	texProperty._maxMipLevel = 6;
	mVoxelizationContext.posxResultTarget = mRenderSystem->createRenderTarget("voxelTextureResultPosX", texProperty);
	mVoxelizationContext.negxResultTarget = mRenderSystem->createRenderTarget("voxelTextureResultNegX", texProperty);
	mVoxelizationContext.posyResultTarget = mRenderSystem->createRenderTarget("voxelTextureResultPosY", texProperty);
	mVoxelizationContext.negyResultTarget = mRenderSystem->createRenderTarget("voxelTextureResultNegY", texProperty);
	mVoxelizationContext.poszResultTarget = mRenderSystem->createRenderTarget("voxelTextureResultPosZ", texProperty);
	mVoxelizationContext.negzResultTarget = mRenderSystem->createRenderTarget("voxelTextureResultNegZ", texProperty);
	texProperty._maxMipLevel = INT_MAX;
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(ObjectConstantBuffer);
	

	
	mRenderSystem->updateBufferObject(mVoxelizationContext.mipmapBlockHandle,
		(const char*)&mipmapBlock, sizeof(mipmapBlock));
	
	uint32_t mipDimension = VCT_SCENE_VOLUME_SIZE >> 1;
	for (uint32_t i = 0; i < VCT_MIPS; i++)
	{
		VctFrameData* frameData = &mComputeFrameData[0];

		frameData->mipmapBlockHandle[i] = mRenderSystem->createBufferObject(desc);
		mipmapBlock.MipDimension = mipDimension;
		mipmapBlock.MipLevel = i;
		mipDimension >>= 1;
		mRenderSystem->updateBufferObject(frameData->mipmapBlockHandle[i], (const char*)&mipmapBlock, sizeof(mipmapBlock));
		frameData->mipmapResultZeroSet[i] = mRenderSystem->createDescriptorSet(mMipmapMainHandle, 0);

		descriptorData[0].pName = "MipmapCB";
		descriptorData[0].mCount = 1;
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[0].ppBuffers = &frameData->mipmapBlockHandle[i];

		{
			descriptorData[1].pName = "voxelTextureSrcPosX";
			descriptorData[1].mCount = 1;
			descriptorData[1].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
			OgreTexture* posxTexture = nullptr;
			if (i == 0)
			{
				posxTexture = mVoxelizationContext.posxTarget->getTarget();
				descriptorData[1].mLevel = 0;
			}
			else
			{
				posxTexture = mVoxelizationContext.posxResultTarget->getTarget();
				descriptorData[1].mLevel = i - 1;
			}

			descriptorData[1].ppTextures = (const OgreTexture**)&posxTexture;
			//
			descriptorData[2].pName = "voxelTextureSrcNegX";
			descriptorData[2].mCount = 1;
			descriptorData[2].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
			OgreTexture* negxTexture = nullptr;
			if (i == 0)
			{
				negxTexture = mVoxelizationContext.negxTarget->getTarget();
				descriptorData[2].mLevel = 0;
			}
			else
			{
				negxTexture = mVoxelizationContext.negxResultTarget->getTarget();
				descriptorData[2].mLevel = i - 1;
			}

			descriptorData[2].ppTextures = (const OgreTexture**)&negxTexture;
			
			//
			descriptorData[3].pName = "voxelTextureSrcPosY";
			descriptorData[3].mCount = 1;
			descriptorData[3].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
			OgreTexture* posyTexture = nullptr;
			if (i == 0)
			{
				posyTexture = mVoxelizationContext.posyTarget->getTarget();
				descriptorData[3].mLevel = 0;
			}
			else
			{
				posyTexture = mVoxelizationContext.posyResultTarget->getTarget();
				descriptorData[3].mLevel = i - 1;
			}

			descriptorData[3].ppTextures = (const OgreTexture**)&posyTexture;
			//
			descriptorData[4].pName = "voxelTextureSrcNegY";
			descriptorData[4].mCount = 1;
			descriptorData[4].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
			OgreTexture* negyTexture = nullptr;
			if (i == 0)
			{
				negyTexture = mVoxelizationContext.negyTarget->getTarget();
				descriptorData[4].mLevel = 0;
			}
			else
			{
				negyTexture = mVoxelizationContext.negyResultTarget->getTarget();
				descriptorData[4].mLevel = i - 1;
			}

			descriptorData[4].ppTextures = (const OgreTexture**)&negyTexture;
			//
			descriptorData[5].pName = "voxelTextureSrcPosZ";
			descriptorData[5].mCount = 1;
			descriptorData[5].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
			OgreTexture* poszTexture = nullptr;
			if (i == 0)
			{
				poszTexture = mVoxelizationContext.poszTarget->getTarget();
				descriptorData[5].mLevel = 0;
			}
			else
			{
				poszTexture = mVoxelizationContext.poszResultTarget->getTarget();
				descriptorData[5].mLevel = i - 1;
			}

			descriptorData[5].ppTextures = (const OgreTexture**)&poszTexture;

			//
			descriptorData[6].pName = "voxelTextureSrcNegZ";
			descriptorData[6].mCount = 1;
			descriptorData[6].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
			OgreTexture* negzTexture = nullptr;
			if (i == 0)
			{
				negzTexture = mVoxelizationContext.negzTarget->getTarget();
				descriptorData[6].mLevel = 0;
			}
			else
			{
				negzTexture = mVoxelizationContext.negzResultTarget->getTarget();
				descriptorData[6].mLevel = i - 1;
			}

			descriptorData[6].ppTextures = (const OgreTexture**)&negzTexture;
		}
		

		OgreTexture* posxTexture = mVoxelizationContext.posxResultTarget->getTarget();
		descriptorData[7].pName = "voxelTextureResultPosX";
		descriptorData[7].mCount = 1;
		descriptorData[7].mLevel = i;
		descriptorData[7].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[7].ppTextures = (const OgreTexture**)&posxTexture;

		OgreTexture* negxTexture = mVoxelizationContext.negxResultTarget->getTarget();
		descriptorData[8].pName = "voxelTextureResultNegX";
		descriptorData[8].mCount = 1;
		descriptorData[8].mLevel = i;
		descriptorData[8].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[8].ppTextures = (const OgreTexture**)&negxTexture;

		OgreTexture* posyTexture = mVoxelizationContext.posyResultTarget->getTarget();
		descriptorData[9].pName = "voxelTextureResultPosY";
		descriptorData[9].mCount = 1;
		descriptorData[9].mLevel = i;
		descriptorData[9].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[9].ppTextures = (const OgreTexture**)&posyTexture;

		OgreTexture* negyTexture = mVoxelizationContext.negyResultTarget->getTarget();
		descriptorData[10].pName = "voxelTextureResultNegY";
		descriptorData[10].mCount = 1;
		descriptorData[10].mLevel = i;
		descriptorData[10].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[10].ppTextures = (const OgreTexture**)&negyTexture;

		OgreTexture* poszTexture = mVoxelizationContext.poszResultTarget->getTarget();
		descriptorData[11].pName = "voxelTextureResultPosZ";
		descriptorData[11].mCount = 1;
		descriptorData[11].mLevel = i;
		descriptorData[11].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[11].ppTextures = (const OgreTexture**)&poszTexture;

		OgreTexture* negzTexture = mVoxelizationContext.negzResultTarget->getTarget();
		descriptorData[12].pName = "voxelTextureResultNegZ";
		descriptorData[12].mCount = 1;
		descriptorData[12].mLevel = i;
		descriptorData[12].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[12].ppTextures = (const OgreTexture**)&negzTexture;


		mRenderSystem->updateDescriptorSet(frameData->mipmapResultZeroSet[i], 
			13, descriptorData);
	}

	//
	shaderInfo.shaderName = "tracingConePass";
	mTracingConeHandle = mRenderSystem->createComputeProgram(shaderInfo);

	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(VCTMainBlock);
	tracingMainBlockHandle = mRenderSystem->createBufferObject(desc);
	desc.mSize = sizeof(VoxelizationBlock);
	tracingVoxelizationBlockHandle = mRenderSystem->createBufferObject(desc);
	
	float ratio = 0.8f;

	texProperty._texType = TEX_TYPE_2D;
	texProperty._width = 960.0f;
	texProperty._height = 540.0f;
	texProperty._depth = 1;
	texProperty._tex_format = Ogre::PixelFormat::PF_A8R8G8B8;
	texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
	texProperty._need_mipmap = false;
	mVoxelizationContext.tracingResultTarget = mRenderSystem->createRenderTarget("tracingResult", texProperty);

	params.filterMag = backend::SamplerFilterType::LINEAR;
	params.filterMin = backend::SamplerFilterType::LINEAR;
	params.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR;
	params.wrapS = backend::SamplerWrapMode::CLAMP_TO_EDGE;
	params.wrapT = backend::SamplerWrapMode::CLAMP_TO_EDGE;
	params.wrapR = backend::SamplerWrapMode::CLAMP_TO_EDGE;
	params.compareMode = backend::SamplerCompareMode::COMPARE_TO_TEXTURE;
	params.compareFunc = backend::SamplerCompareFunc::LE;
	params.anisotropyLog2 = 4;
	params.padding0 = 0;
	params.padding1 = 0;
	params.padding2 = 0;
	mVoxelizationContext.tracingSampler = mRenderSystem->createTextureSampler(params);

	texProperty._texType = TEX_TYPE_3D;
	texProperty._width = 128;
	texProperty._height = 128;
	texProperty._depth = 128;
	texProperty._tex_format = Ogre::PixelFormat::PF_A8R8G8B8;
	texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
	texProperty._need_mipmap = true;

	VctFrameData* frameData = &mComputeFrameData[0];
	frameData->tracingConeZeroSet = mRenderSystem->createDescriptorSet(mTracingConeHandle, 0);

	OgreTexture* albedoTexture = mVoxelizationContext.albedoTarget->getTarget();
	descriptorData[0].pName = "albedoBuffer";
	descriptorData[0].mCount = 1;
	descriptorData[0].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[0].ppTextures = (const OgreTexture**)&albedoTexture;

	OgreTexture* normalTexture = mVoxelizationContext.normalTarget->getTarget();
	descriptorData[1].pName = "normalBuffer";
	descriptorData[1].mCount = 1;
	descriptorData[1].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[1].ppTextures = (const OgreTexture**)&normalTexture;

	OgreTexture* worldPosTexture = mVoxelizationContext.worldPosTarget->getTarget();
	descriptorData[2].pName = "worldPosBuffer";
	descriptorData[2].mCount = 1;
	descriptorData[2].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[2].ppTextures = (const OgreTexture**)&worldPosTexture;

	OgreTexture* posxTexture = mVoxelizationContext.posxResultTarget->getTarget();
	descriptorData[3].pName = "voxelTexturePosX";
	descriptorData[3].mCount = 1;
	descriptorData[3].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[3].ppTextures = (const OgreTexture**)&posxTexture;

	OgreTexture* negxTexture = mVoxelizationContext.negxResultTarget->getTarget();
	descriptorData[4].pName = "voxelTextureNegX";
	descriptorData[4].mCount = 1;
	descriptorData[4].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[4].ppTextures = (const OgreTexture**)&negxTexture;

	OgreTexture* posyTexture = mVoxelizationContext.posyResultTarget->getTarget();
	descriptorData[5].pName = "voxelTexturePosY";
	descriptorData[5].mCount = 1;
	descriptorData[5].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[5].ppTextures = (const OgreTexture**)&posyTexture;

	OgreTexture* negyTexture = mVoxelizationContext.negyResultTarget->getTarget();
	descriptorData[6].pName = "voxelTextureNegY";
	descriptorData[6].mCount = 1;
	descriptorData[6].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[6].ppTextures = (const OgreTexture**)&negyTexture;

	OgreTexture* poszTexture = mVoxelizationContext.poszResultTarget->getTarget();
	descriptorData[7].pName = "voxelTexturePosZ";
	descriptorData[7].mCount = 1;
	descriptorData[7].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[7].ppTextures = (const OgreTexture**)&poszTexture;

	OgreTexture* negzTexture = mVoxelizationContext.negzResultTarget->getTarget();
	descriptorData[8].pName = "voxelTextureNegZ";
	descriptorData[8].mCount = 1;
	descriptorData[8].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[8].ppTextures = (const OgreTexture**)&negzTexture;

	OgreTexture* voxelizationTexture = mVoxelizationContext.voxelizationTarget->getTarget();
	descriptorData[9].pName = "voxelTexture";
	descriptorData[9].mCount = 1;
	descriptorData[9].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
	descriptorData[9].ppTextures = (const OgreTexture**)&voxelizationTexture;

	OgreTexture* resultTexture = mVoxelizationContext.tracingResultTarget->getTarget();
	descriptorData[10].pName = "result";
	descriptorData[10].mCount = 1;
	descriptorData[10].mLevel = 0;
	descriptorData[10].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
	descriptorData[10].ppTextures = (const OgreTexture**)&resultTexture;

	descriptorData[11].pName = "LinearSampler";
	descriptorData[11].mCount = 1;
	descriptorData[11].descriptorType = DESCRIPTOR_TYPE_SAMPLER;
	descriptorData[11].ppSamplers = &mVoxelizationContext.tracingSampler;

	descriptorData[12].pName = "VoxelizationCB";
	descriptorData[12].mCount = 1;
	descriptorData[12].descriptorType = DESCRIPTOR_TYPE_BUFFER;
	descriptorData[12].ppBuffers = &tracingVoxelizationBlockHandle;

	descriptorData[13].pName = "VCTMainCB";
	descriptorData[13].mCount = 1;
	descriptorData[13].descriptorType = DESCRIPTOR_TYPE_BUFFER;
	descriptorData[13].ppBuffers = &tracingMainBlockHandle;

	mRenderSystem->updateDescriptorSet(frameData->tracingConeZeroSet, 14, descriptorData);
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

void VoxelConeTracingApp::initFrameResource(uint32_t frameIndex, Renderable* r)
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	auto* rs = Ogre::Root::getSingleton().getRenderSystem();
	if (1)
	{
		VctFrameResourceInfo* resourceInfo = new VctFrameResourceInfo;
		resourceInfo->update = false;

		r->updateFrameResource(frameIndex, (void*)resourceInfo);

		BufferDesc desc{};
		desc.mBindingType = BufferObjectBinding_Uniform;
		desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
		desc.bufferCreationFlags = 0;
		desc.mSize = sizeof(ObjectConstantBuffer);
		Handle<HwBufferObject> objectBufferHandle =
			rs->createBufferObject(desc);
		resourceInfo->modelObjectHandle = objectBufferHandle;

		Ogre::Material* mat = r->getMaterial().get();
		Handle<HwBufferObject> matBufferHandle;
		if (mat->isPbr())
		{
			desc.mBindingType = BufferObjectBinding_Uniform;
			desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
			desc.bufferCreationFlags = 0;
			desc.mSize = sizeof(PbrMaterialConstanceBuffer);
			matBufferHandle = rs->createBufferObject(desc);
		}
		else
		{
			desc.mBindingType = BufferObjectBinding_Uniform;
			desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
			desc.bufferCreationFlags = 0;
			desc.mSize = sizeof(GeneralMaterialConstantBuffer);
			matBufferHandle = rs->createBufferObject(desc);
		}

		resourceInfo->matObjectHandle = matBufferHandle;

		
		resourceInfo->zeroSet = rs->createDescriptorSet(mSceneGeometryProgramHandle, 0);
		resourceInfo->firstSet = rs->createDescriptorSet(mSceneGeometryProgramHandle, 1);
		resourceInfo->zeroShadowSet = rs->createDescriptorSet(mShadowProgramHandle, 0);
		resourceInfo->zeroSetOfVoxelization = rs->createDescriptorSet(mVoxellizationProgramHandle, 0);
		DescriptorData descriptorData[256];
		uint32_t descriptorCount = 0;
		descriptorData[descriptorCount].pName = "cbPerObject";
		descriptorData[descriptorCount].mCount = 1;
		descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[descriptorCount].ppBuffers = &objectBufferHandle;
		descriptorCount++;
		/*const char* materialName = mat->isPbr() ? "pbrMaterial" : "cbMaterial";
		descriptorData[descriptorCount].pName = materialName;
		descriptorData[descriptorCount].mCount = 1;
		descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[descriptorCount].ppBuffers = &matBufferHandle;
		descriptorCount++;


		RawData* rawData = r->getSkinnedData();
		if (rawData)
		{
			desc.mBindingType = BufferObjectBinding_Uniform;
			desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
			desc.bufferCreationFlags = 0;
			desc.mSize = sizeof(SkinnedConstantBuffer);
			resourceInfo->skinObjectHandle = rs->createBufferObject(desc);
			descriptorData[descriptorCount].pName = "cbSkinned";
			descriptorData[descriptorCount].mCount = 1;
			descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_BUFFER;
			descriptorData[descriptorCount].ppBuffers = &resourceInfo->skinObjectHandle;
			descriptorCount++;
		}*/

		rs->updateDescriptorSet(resourceInfo->zeroSet, descriptorCount, descriptorData);
		rs->updateDescriptorSet(resourceInfo->zeroShadowSet, descriptorCount, descriptorData);

		//
		descriptorData[0].pName = "cbPerObject";
		descriptorData[0].mCount = 1;
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[0].ppBuffers = &objectBufferHandle;

		Ogre::OgreTexture* voxelizationTexture = mVoxelizationContext.voxelizationTarget->getTarget();

		descriptorData[1].pName = "outputTexture";
		descriptorData[1].mCount = 1;
		descriptorData[1].mLevel = 0;
		descriptorData[1].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[1].ppTextures = (const OgreTexture**)& voxelizationTexture;

		Ogre::OgreTexture* shadowTexture = mVoxelizationContext.depthTarget->getTarget();

		descriptorData[2].pName = "shadowBuffer";
		descriptorData[2].mCount = 1;
		descriptorData[2].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
		descriptorData[2].ppTextures = (const OgreTexture**)& shadowTexture;

		descriptorData[3].pName = "PcfShadowMapSampler";
		descriptorData[3].mCount = 1;
		descriptorData[3].descriptorType = DESCRIPTOR_TYPE_SAMPLER;
		descriptorData[3].ppSamplers = &mVoxelizationContext.voxelizationSampler;

		rs->updateDescriptorSet(resourceInfo->zeroSetOfVoxelization, 4, descriptorData);

		//update texture
		uint32_t index = 0;
		descriptorCount = 0;

		auto& texs = mat->getAllTexureUnit();
		if (mat->isPbr())
		{
			OgreTexture* texArray[9];
			for (int32_t i = 0; i < texs.size(); i++)
			{
				const char* pName = "";
				const char* samplerName = "";
				int32_t texIndex = -1;
				switch (texs[i]->getTextureProperty()->_pbrType)
				{
				case TextureTypePbr_Albedo:
					texIndex = 0;
					pName = "albedo_pbr";
					samplerName = "albedoSampler";
					break;
				case TextureTypePbr_MetalRoughness:
					texIndex = 4;
					pName = "metal_roughness_pbr";
					samplerName = "metalRoughnessSampler";
					break;
				case TextureTypePbr_NormalMap:
					texIndex = 2;
					pName = "normal_pbr";
					samplerName = "normalSampler";
					break;
				case TextureTypePbr_Emissive:
					texIndex = 3;
					pName = "emissive_pbr";
					samplerName = "emissiveSampler";
					break;
				case TextureTypePbr_AmbientOcclusion:
					texIndex = 1;
					pName = "ao_pbr";
					samplerName = "aoSampler";
					break;
				case TextureTypePbr_Roughness:
					texIndex = 5;
					pName = "roughness_pbr";
					samplerName = "roughnessSampler";
					break;
				case TextureTypePbr_BRDF_LUT:
					texIndex = 6;
					pName = "brdflut_pbr";
					samplerName = "brdflutSampler";
					break;
				case TextureTypePbr_IBL_Diffuse:
					texIndex = 7;
					pName = "irradianceCube";
					samplerName = "irradianceSampler";
					break;
				case TextureTypePbr_IBL_Specular:
					texIndex = 8;
					pName = "prefilteredCube";
					samplerName = "prefilteredSampler";
					break;
				}
				assert(texIndex >= 0);
				OgreTexture* tex = texs[i]->getRaw();
				texArray[i] = tex;
				descriptorData[descriptorCount].pName = pName;
				descriptorData[descriptorCount].mCount = 1;
				descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
				descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[i];
				descriptorCount++;

				descriptorData[descriptorCount].pName = samplerName;
				descriptorData[descriptorCount].mCount = 1;
				descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
				descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[i];
				descriptorCount++;
			}
			rs->updateDescriptorSet(resourceInfo->firstSet, descriptorCount, descriptorData);
		}
		else
		{
			OgreTexture* texArray[4];
			int32_t texIndex = -1;
			for (int32_t i = 0; i < texs.size(); i++)
			{
				if (texs[i]->getTextureProperty()->_texType == TEX_TYPE_CUBE_MAP)
					continue;
				texArray[++texIndex] = texs[i]->getRaw();
			}

			if (texIndex >= 0)
			{
				descriptorData[descriptorCount].pName = "first";
				descriptorData[descriptorCount].mCount = 1;
				descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
				descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[0];
				descriptorCount++;

				descriptorData[descriptorCount].pName = "firstSampler";
				descriptorData[descriptorCount].mCount = 1;
				descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
				descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[0];
				descriptorCount++;
			}

			if (texIndex >= 1)
			{
				descriptorData[descriptorCount].pName = "second";
				descriptorData[descriptorCount].mCount = 1;
				descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
				descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[1];
				descriptorCount++;

				descriptorData[descriptorCount].pName = "secondSampler";
				descriptorData[descriptorCount].mCount = 1;
				descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
				descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[1];
				descriptorCount++;
			}

			if (texIndex >= 2)
			{
				descriptorData[descriptorCount].pName = "third";
				descriptorData[descriptorCount].mCount = 1;
				descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
				descriptorData[descriptorCount].ppTextures = (const OgreTexture**)texArray[2];
				descriptorCount++;

				descriptorData[descriptorCount].pName = "thirdSampler";
				descriptorData[descriptorCount].mCount = 1;
				descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
				descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[2];
				descriptorCount++;
			}

			for (int32_t i = 0; i < texs.size(); i++)
			{
				if (texs[i]->getTextureProperty()->_texType == TEX_TYPE_CUBE_MAP)
				{
					texArray[3] = texs[i]->getRaw();
					descriptorData[descriptorCount].pName = "cubeMap";
					descriptorData[descriptorCount].mCount = 1;
					descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
					descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[3];
					descriptorCount++;

					descriptorData[descriptorCount].pName = "cubeSampler";
					descriptorData[descriptorCount].mCount = 1;
					descriptorData[descriptorCount].descriptorType = DESCRIPTOR_TYPE_TEXTURE_SAMPLER;
					descriptorData[descriptorCount].ppTextures = (const OgreTexture**)&texArray[3];
					descriptorCount++;
					break;
				}
			}
			if (descriptorCount)
			{
				rs->updateDescriptorSet(resourceInfo->firstSet, descriptorCount, descriptorData);
			}
		}
	}
}




