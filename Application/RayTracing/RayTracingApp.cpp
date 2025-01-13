#include "OgreHeader.h"
#include "RayTracingApp.h"
#include "myutils.h"
#include "OgreResourceManager.h"
#include "OgreMaterialManager.h"
#include "OgreCamera.h"
#include "OgreRoot.h"
#include "renderSystem.h"
#include "forgeCommon.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreMesh.h"
#include "OgreVertexData.h"
#include "OgreIndexData.h"
#include "OgrePixelFormat.h"
#include "OgreTextureUnit.h"
#include "OgreRenderTarget.h"
#include "OgreRenderWindow.h"
#include "game_camera.h"
#include "OgreMurmurHash3.h"
#include "OgreTextureManager.h"
#include "OgreMeshManager.h"
#include "renderUtil.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

RayTracingApp::RayTracingApp()
{

}

RayTracingApp::~RayTracingApp()
{

}

void RayTracingApp::setup(
	RenderPipeline* renderPipeline,
	RenderSystem* renderSystem,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	mRenderSystem = renderSystem;
	int type = 3;
	switch (type)
	{
	case 0:
		RayTracingBasic(renderPipeline, renderSystem, renderWindow, sceneManager, gameCamera);
		break;
	case 1:
		RayTracingBox(renderPipeline, renderSystem, renderWindow, sceneManager, gameCamera);
		break;
	case 2:
		RayTracingShadow(renderPipeline, renderSystem, renderWindow, sceneManager, gameCamera);
		break;
	case 3:
		RayTracingGltf(renderPipeline, renderSystem, renderWindow, sceneManager, gameCamera);
		break;
	case 4:
		RayQuery(renderPipeline, renderSystem, renderWindow, sceneManager, gameCamera);
		break;
	}
}
static float haltonSequence(uint32_t index, uint32_t base)
{
	float f = 1.f;
	float r = 0.f;

	while (index > 0)
	{
		f /= (float)base;
		r += f * (float)(index % base);
		index /= base;
	}

	return r;
}
#define TF_RAND_MAX     0x7FFFFFFF
#define TF_INITIAL_SEED 0x9747b28c

int32_t getRandomInt(void)
{
	static uint32_t       seed = TF_INITIAL_SEED;
	static const uint32_t messageHash[] = {
		0x8C2100D0, 0xEC843F56, 0xDD467E25, 0xC22461F6, 0xA1368AB0, 0xBBDA7B12, 0xA175F888, 0x6BD9BDA2,
		0x999AC54C, 0x7C043DD3, 0xD502088F, 0x1B5B4D72, 0x94BB5742, 0x2CDA891E, 0x88613640, 0x31A50479
	};

	int32_t result;
	MurmurHash3_x86_32(&messageHash, sizeof(messageHash), seed--, &result);
	return abs(result);
}

float randomFloat01() { return (float)getRandomInt() / (float)TF_RAND_MAX; }

void RayTracingApp::update(float delta)
{
	return;
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	Ogre::Camera*  cam = mGameCamera->getCamera();
	const Ogre::Matrix4& viewMat = cam->getViewMatrix();

	const float aspectInverse = (float)ogreConfig.height / (float)ogreConfig.width;
	const float horizontalFOV = Ogre::Math::PI / 2.0f;
	const float nearPlane = 0.1f;
	const float farPlane = 6000.f;
	Ogre::Matrix4        projMat = Ogre::Math::makePerspectiveMatrixLHReverseZ(horizontalFOV, aspectInverse, nearPlane, farPlane);
	Ogre::Matrix4        projectView = projMat * viewMat;

	mPathTracingData.mWorldToCamera = viewMat.transpose();
	mPathTracingData.mProjMat = projMat.transpose();
	mPathTracingData.mProjectView = projectView.transpose();
	mPathTracingData.mCameraToWorld = viewMat.inverse().transpose();
	mPathTracingData.mProjNear = nearPlane;
	mPathTracingData.mProjFarMinusNear = farPlane - nearPlane;
	mPathTracingData.mZ1PlaneSize = Ogre::Vector2(1.0f / projMat[0][0], 1.0f / projMat[1][1]);

	mPathTracingData.mRandomSeed = randomFloat01();
	if (true)
	{
		bool cameraMoved = memcmp(&mPathTracingData.mProjectView, 
			&mPathTracingData.mHistoryProjView, sizeof(Ogre::Matrix4)) != 0;
		bool lightMoved = memcmp(&mLightDirection, &mPathTracingData.mHistoryLightDirection, sizeof(Ogre::Vector3)) != 0; //-V1014


		if (cameraMoved || lightMoved)
		{
			mPathTracingData.mFrameIndex = 0;
			mPathTracingData.mHaltonIndex = 0;
		}

		if (cameraMoved)
		{
			mPathTracingData.mLastCameraMoveFrame = mPathTracingData.mFrameIndex;
		}

		ShadersConfigBlock cb;
		cb.mCameraToWorld = mPathTracingData.mCameraToWorld;
		cb.mProjNear = mPathTracingData.mProjNear;
		cb.mProjFarMinusNear = mPathTracingData.mProjFarMinusNear;
		cb.mZ1PlaneSize = mPathTracingData.mZ1PlaneSize;
		mLightDirection.normalise();
		cb.mLightDirection = mLightDirection;

		cb.mRandomSeed = mPathTracingData.mRandomSeed;

		// Loop through the first 16 items in the Halton sequence.
		// The Halton sequence takes one-based indices.
		cb.mSubpixelJitter =
			Ogre::Vector2(haltonSequence(mPathTracingData.mHaltonIndex + 1, 2), haltonSequence(mPathTracingData.mHaltonIndex + 1, 3));

		cb.mFrameIndex = mPathTracingData.mFrameIndex;
		

		cb.mFramesSinceCameraMove = mPathTracingData.mFrameIndex - mPathTracingData.mLastCameraMoveFrame;
		cb.mWidth = ogreConfig.width;
		cb.mHeight = ogreConfig.height;
		cb.mWorldToCamera = mPathTracingData.mWorldToCamera;
		cb.mCameraToProjection = mPathTracingData.mProjMat;
		cb.mWorldToProjectionPrevious = mPathTracingData.mHistoryProjView;
		cb.mRtInvSize = Ogre::Vector2(1.0f / ogreConfig.width, 1.0f / ogreConfig.height);

		cb.mWorldMatrix = sanMiguelProp.mWorldMatrix;

		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();

		FrameInfo* frameInfo = getFrameInfo(frameIndex);
		BufferHandleLockGuard lockGuard(frameInfo->genConfigBuffer);
		memcpy(lockGuard.data(), &cb, sizeof(cb));

		mPathTracingData.mHistoryProjView = mPathTracingData.mProjectView;
		mPathTracingData.mHistoryLightDirection = mLightDirection;
		mPathTracingData.mFrameIndex += 1;
		mPathTracingData.mHaltonIndex = (mPathTracingData.mHaltonIndex + 1) % 16;
	}
}

void RayTracingApp::RayQuery(RenderPipeline* renderPipeline,
	RenderSystem* renderSystem,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	mGameCamera = gameCamera;
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	auto rootNode = sceneManager->getRoot();
	RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
	ShaderInfo shaderInfo;
	shaderInfo.shaderName = "RayQuery";
	auto programHandle = rs->createComputeProgram(shaderInfo);

	std::string meshname = "SanMiguel.bin";
	std::shared_ptr<Mesh> mesh = loadSanMiguel(meshname);

	Entity* sanMiguel = sceneManager->createEntity(meshname, meshname);
	SceneNode* sanMiguelNode = rootNode->createChildSceneNode(meshname);
	sanMiguelNode->attachObject(sanMiguel);

	auto subMeshCount = mesh->getSubMeshCount();
	/************************************************************************/
			// Creation Acceleration Structure
   /************************************************************************/
	std::vector<TransformMatrix> transformMatrices;
	transformMatrices.reserve(subMeshCount);

	uint32_t transformSize = sizeof(TransformMatrix) * subMeshCount;
	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Storge;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT | BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS;
	desc.mSize = transformSize;
	desc.mElementCount = subMeshCount;
	desc.mStructStride = sizeof(TransformMatrix);
	Handle< HwBufferObject> transformBufferHandle = rs->createBufferObject(desc);

	AccelerationStructureDesc         asDesc = {};
	AccelerationStructureGeometryDesc geomDescs[512] = {};
	VertexData* vertexData = mesh->getVertexData();
	IndexData* indexData = mesh->getIndexData();
	for (uint32_t i = 0; i < subMeshCount; i++)
	{
		SubMesh* subMesh = mesh->getSubMesh(i);
		auto& mat = subMesh->getMaterial();

		auto materialFlag = mat->getMaterialFlags();

		SubEntity* subEntity = sanMiguel->getSubEntity(i);
		const Ogre::Matrix4& subMatrix = subEntity->getModelMatrix();
		TransformMatrix transformMatrix;

		Ogre::Matrix4 m = Ogre::Matrix4::IDENTITY;
		memcpy(&transformMatrix, (void*)&subMatrix, sizeof(transformMatrix));
		transformMatrices.push_back(transformMatrix);

		geomDescs[i].mFlags = (materialFlag & MATERIAL_FLAG_ALPHA_TESTED)
			? ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION
			: ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE;
		geomDescs[i].vertexBufferHandle = vertexData->getBuffer(0);
		geomDescs[i].mVertexCount = (uint32_t)vertexData->getVertexCount();
		geomDescs[i].mVertexStride = vertexData->getVertexSize(0);
		geomDescs[i].mVertexElementType = VertexElementType::VET_FLOAT3;
		geomDescs[i].indexBufferHandle = indexData->getHandle();

		IndexDataView* indexView = subMesh->getIndexView();
		geomDescs[i].mIndexCount = indexView->mIndexCount;
		geomDescs[i].mIndexOffset = indexView->mIndexLocation * sizeof(uint32_t);
		geomDescs[i].mIndexType = INDEX_TYPE_UINT32;

		geomDescs[i].transformBufferHandle = transformBufferHandle;
	}

	rs->updateBufferObject(transformBufferHandle,
		(const char*)transformMatrices.data(), transformSize);

	asDesc.mBottom.mDescCount = subMeshCount;
	asDesc.mBottom.pGeometryDescs = geomDescs;
	asDesc.mType = ACCELERATION_STRUCTURE_TYPE_BOTTOM;
	asDesc.mFlags = ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;


	AccelerationStructure* pSanMiguelBottomAS;
	AccelerationStructure* pSanMiguelAS = NULL;

	rs->addAccelerationStructure(&asDesc, &pSanMiguelBottomAS);

	sanMiguelProp.mWorldMatrix = Ogre::Math::makeScaleMatrix(Ogre::Vector3(10.0f, 10.0f, 10.0f));

	AccelerationStructureInstanceDesc instanceDesc = {};
	instanceDesc.mFlags = ACCELERATION_STRUCTURE_INSTANCE_FLAG_NONE;
	instanceDesc.mInstanceContributionToHitGroupIndex = 0;
	instanceDesc.mInstanceID = 0;
	instanceDesc.mInstanceMask = 1;
	memcpy(instanceDesc.mTransform, &sanMiguelProp.mWorldMatrix, sizeof(float[12]));
	instanceDesc.pBottomAS = pSanMiguelBottomAS;


	rs->beginCmd();
	asDesc = {};
	asDesc.mType = ACCELERATION_STRUCTURE_TYPE_TOP;
	asDesc.mFlags = ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	asDesc.mTop.mDescCount = 1;
	asDesc.mTop.pInstanceDescs = &instanceDesc;

	rs->addAccelerationStructure(&asDesc, &pSanMiguelAS);

	// Build Acceleration Structures
	RaytracingBuildASDesc buildASDesc = {};
	buildASDesc.pAccelerationStructure = pSanMiguelBottomAS;
	buildASDesc.mIssueRWBarrier = true;
	rs->buildAccelerationStructure(&buildASDesc);

	buildASDesc = {};
	buildASDesc.pAccelerationStructure = pSanMiguelAS;

	rs->buildAccelerationStructure(&buildASDesc);
	rs->flushCmd(true);
	rs->removeAccelerationStructureScratch(pSanMiguelBottomAS);
	rs->removeAccelerationStructureScratch(pSanMiguelAS);

	backend::SamplerParams samplerParams;

	samplerParams.filterMag = backend::SamplerFilterType::LINEAR;
	samplerParams.filterMin = backend::SamplerFilterType::LINEAR;
	samplerParams.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR;
	samplerParams.compareMode = backend::SamplerCompareMode::NONE;
	samplerParams.compareFunc = backend::SamplerCompareFunc::N;
	samplerParams.wrapS = backend::SamplerWrapMode::REPEAT;
	samplerParams.wrapT = backend::SamplerWrapMode::REPEAT;
	samplerParams.wrapR = backend::SamplerWrapMode::REPEAT;
	samplerParams.anisotropyLog2 = 0;
	samplerParams.padding0 = 0;
	samplerParams.padding1 = 0;
	samplerParams.padding2 = 0;
	auto linearSamplerHandle = rs->createTextureSampler(samplerParams);

	auto format = renderWindow->getColorFormat();

	TextureProperty texProperty;
	texProperty._width = ogreConfig.width;
	texProperty._height = ogreConfig.height;
	texProperty._tex_format = format;
	texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
	auto outPutTarget = rs->createRenderTarget("outputTarget", texProperty);
	OgreTexture* outputTexture = outPutTarget->getTarget();

	auto numFrame = ogreConfig.swapBufferCount;
	mFrameInfoList.resize(numFrame);
	
	desc.mBindingType = BufferObjectBinding_Storge;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = subMeshCount * sizeof(uint32_t);
	indexOffsetsBuffer =
		rs->createBufferObject(desc);

	DescriptorData descriptorData[10];
	Handle<HwBufferObject> vertexDataHandle = vertexData->getBuffer(0);
	Handle<HwBufferObject> indexDataHandle = indexData->getHandle();
	for (auto i = 0; i < numFrame; i++)
	{
		FrameInfo& frameInfo = mFrameInfoList[i];
		BufferDesc desc{};
		desc.mBindingType = BufferObjectBinding_Uniform;
		desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
		desc.bufferCreationFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
		desc.mSize = sizeof(ShadersConfigBlock);
		auto genConfigBuffer =
			rs->createBufferObject(desc);
		frameInfo.genConfigBuffer = genConfigBuffer;

		auto zeroSet = rs->createDescriptorSet(programHandle, 0);
		auto firstSet = rs->createDescriptorSet(programHandle, 1);
		frameInfo.zeroDescriptorSet = zeroSet;
		frameInfo.firstDescriptorSet = firstSet;
		descriptorData[0].mCount = 1;
		descriptorData[0].pName = "gRtScene";
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE;
		descriptorData[0].pAS = pSanMiguelAS;

		descriptorData[1].mCount = 1;
		descriptorData[1].pName = "indices";
		descriptorData[1].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[1].ppBuffers = &indexDataHandle;

		descriptorData[2].mCount = 1;
		descriptorData[2].pName = "vertexDataBuffer";
		descriptorData[2].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[2].ppBuffers = &vertexDataHandle;

		descriptorData[3].mCount = 1;
		descriptorData[3].pName = "linearSampler";
		descriptorData[3].descriptorType = DESCRIPTOR_TYPE_SAMPLER;
		descriptorData[3].ppSamplers = &linearSamplerHandle;

		descriptorData[4].mCount = 1;
		descriptorData[4].pName = "indexOffsets";
		descriptorData[4].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[4].ppBuffers = &indexOffsetsBuffer;

		descriptorData[5].mCount = 1;
		descriptorData[5].pName = "gOutput";
		descriptorData[5].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
		descriptorData[5].ppTextures = (const OgreTexture**)&outputTexture;

		rs->updateDescriptorSet(zeroSet, 6, descriptorData);

		descriptorData[0].mCount = 1;
		descriptorData[0].pName = "gSettings";
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[0].ppBuffers = &genConfigBuffer;
		rs->updateDescriptorSet(firstSet, 1, descriptorData);
	}

	BufferHandleLockGuard lockGuard(indexOffsetsBuffer);
	uint32_t* offsetsData = (uint32_t*)lockGuard.data();

	std::vector<OgreTexture*> diffuseList;

	diffuseList.reserve(256);

	for (auto i = 0; i < subMeshCount; i++)
	{
		auto* subEntity = sanMiguel->getSubEntity(i);
		IndexDataView* indexDataView = subEntity->getIndexView();
		offsetsData[i] = indexDataView->mIndexLocation;
		auto& mat = subEntity->getMaterial();
		mat->load(nullptr);
		auto& unitList = mat->getAllTexureUnit();
		auto* diffuseTex = unitList[0]->getRaw();
		diffuseList.push_back(diffuseTex);
	}
	std::shared_ptr<OgreTexture> defaultTex =
		TextureManager::getSingleton().getByName("white1x1.dds");
	for (auto i = subMeshCount; i < 256; i++)
	{
		diffuseList.push_back(defaultTex.get());
	}
	for (auto i = 0; i < numFrame; i++)
	{
		FrameInfo& frameInfo = mFrameInfoList[i];
		descriptorData[0].mCount = diffuseList.size();
		descriptorData[0].pName = "materialTextures";
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
		descriptorData[0].ppTextures = (const OgreTexture**)diffuseList.data();

		rs->updateDescriptorSet(frameInfo.zeroDescriptorSet, 1, descriptorData);
	}

	ComputePassCallback callback = [=](ComputePassInfo& info) {
		TextureBarrier uavBarriers[] = {
				{
				outPutTarget->getTarget(),
				RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
				RESOURCE_STATE_UNORDERED_ACCESS},
		};
		rs->resourceBarrier(0, nullptr, 1, uavBarriers, 0, nullptr);
		info.programHandle = programHandle;
		info.computeGroup = Ogre::Vector3i(180, 180, 1);
		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		FrameInfo* frameInfo = this->getFrameInfo(frameIndex);

		info.descSets.clear();
		info.descSets.push_back(frameInfo->zeroDescriptorSet);
		info.descSets.push_back(frameInfo->firstDescriptorSet);
		rs->pushGroupMarker("RayQuery");
		rs->beginComputePass(info);
		rs->endComputePass();
		rs->popGroupMarker();
		};

	auto clearBufferPass = createComputePass(callback, nullptr);
	renderPipeline->addRenderPass(clearBufferPass);

	if (1)
	{
		backend::SamplerParams samplerParams;

		samplerParams.filterMag = backend::SamplerFilterType::LINEAR;
		samplerParams.filterMin = backend::SamplerFilterType::LINEAR;
		samplerParams.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR;
		samplerParams.wrapS = backend::SamplerWrapMode::REPEAT;
		samplerParams.wrapT = backend::SamplerWrapMode::REPEAT;
		samplerParams.wrapR = backend::SamplerWrapMode::REPEAT;
		samplerParams.compareMode = backend::SamplerCompareMode::NONE;
		samplerParams.compareFunc = backend::SamplerCompareFunc::N;
		samplerParams.anisotropyLog2 = 0;
		samplerParams.padding0 = 0;
		samplerParams.padding1 = 0;
		samplerParams.padding2 = 0;
		auto repeatBillinearSampler = rs->createTextureSampler(samplerParams);
		auto winDepth = renderWindow->getDepthTarget();
		ShaderInfo shaderInfo;
		shaderInfo.shaderName = "presentShade";
		auto presentHandle = rs->createShaderProgram(shaderInfo, nullptr);
		for (auto i = 0; i < numFrame; i++)
		{
			auto& frameData = mFrameInfoList[i];
			auto zeroSet = rs->createDescriptorSet(presentHandle, 0);
			frameData.zeroDescriptorSetOfPresent = zeroSet;
			auto* tex = outPutTarget->getTarget();

			descriptorData[0].mCount = 1;
			descriptorData[0].pName = "SourceTexture";
			descriptorData[0].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
			descriptorData[0].ppTextures = (const OgreTexture**)&tex;

			descriptorData[1].mCount = 1;
			descriptorData[1].pName = "repeatBillinearSampler";
			descriptorData[1].descriptorType = DESCRIPTOR_TYPE_SAMPLER;
			descriptorData[1].ppSamplers = &repeatBillinearSampler;

			rs->updateDescriptorSet(zeroSet, 2, descriptorData);
		}

		backend::RasterState rasterState{};
		rasterState.depthWrite = false;
		rasterState.depthTest = false;
		rasterState.depthFunc = SamplerCompareFunc::A;
		rasterState.colorWrite = true;
		rasterState.renderTargetCount = 1;
		rasterState.pixelFormat[0] = Ogre::PixelFormat::PF_A8R8G8B8;
		auto pipelineHandle = rs->createPipeline(rasterState, presentHandle);

		RenderPassCallback presentCallback = [=, this](RenderPassInfo& info) {
			{
				RenderTargetBarrier rtBarriers[] =
				{
					{
						renderWindow->getColorTarget(),
						RESOURCE_STATE_PRESENT,
						RESOURCE_STATE_RENDER_TARGET
					},
					{
						outPutTarget,
						RESOURCE_STATE_UNORDERED_ACCESS,
						RESOURCE_STATE_PIXEL_SHADER_RESOURCE
					}
				};
				rs->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);
			}
			
			info.renderTargetCount = 1;
			info.renderTargets[0].renderTarget = renderWindow->getColorTarget();
			info.renderTargets[0].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
			info.depthTarget.depthStencil = nullptr;
			info.depthTarget.clearValue = { 0.0f, 0.0f };
			auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
			rs->pushGroupMarker("presentPass");
			rs->beginRenderPass(info);
			auto* frameData = getFrameInfo(frameIndex);
			rs->bindPipeline(presentHandle, pipelineHandle,
				&frameData->zeroDescriptorSetOfPresent, 1);
			rs->draw(3, 0);
			rs->endRenderPass(info);
			rs->popGroupMarker();

			{
				RenderTargetBarrier rtBarriers[] =
				{
					{
						renderWindow->getColorTarget(),
						RESOURCE_STATE_RENDER_TARGET,
						RESOURCE_STATE_PRESENT
					},
					{
						outPutTarget,
						RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
						RESOURCE_STATE_UNORDERED_ACCESS
					}
				};
				rs->resourceBarrier(0, nullptr, 0, nullptr, 2, rtBarriers);
			}
			};
		UpdatePassCallback updateCallback = [](float delta) {
			};
		auto presentPass = createUserDefineRenderPass(presentCallback, updateCallback);
		renderPipeline->addRenderPass(presentPass);
	}

	Ogre::Vector3 camPos2(80.0f, 60.0f, 50.0f);

	mGameCamera->setMoveSpeed(50.0f);

	Ogre::Vector3 lookAt = Ogre::Vector3(1.0f, 0.5f, 0.0f);
	mGameCamera->lookAt(camPos2, lookAt);
}


void RayTracingApp::RayTracingGltf(
	RenderPipeline* renderPipeline,
	RenderSystem* rs,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	auto rootNode = sceneManager->getRoot();
	auto root = sceneManager->getRoot();
    std::string meshname = "FlightHelmet.gltf";
    std::shared_ptr<Mesh> mesh = MeshManager::getSingletonPtr()->load(meshname);
	float aa = 1.0f;
	Ogre::Vector3 leftop = Ogre::Vector3(-aa, aa, 0.0f);
	Ogre::Vector3 leftbottom = Ogre::Vector3(-aa, -aa, 0.0f);
	Ogre::Vector3 righttop = Ogre::Vector3(aa, aa, 0.0f);
	Ogre::Vector3 rightbottom = Ogre::Vector3(aa, -aa, 0.0f);
	Ogre::Vector3 normal = Ogre::Vector3(0.0f, 0.0f, 1.0f);

    Entity* gltfEntity = sceneManager->createEntity(meshname, meshname);
    SceneNode* gltfNode = rootNode->createChildSceneNode(meshname);
	gltfNode->attachObject(gltfEntity);

	auto subMeshCount = mesh->getSubMeshCount();


	RaytracingShaderInfo shaderInfo;
	Handle<HwRaytracingProgram> raytracingHandle;
	if (1)
	{
		shaderInfo.rayGenShaderName = "rayGen.glsl";
		shaderInfo.rayMissShaderName = "rayMiss.glsl";
		shaderInfo.rayClosethitShaderName = "rayClosesthit.glsl";
		shaderInfo.rayAnyHitShaderName = "rayAnyhit.glsl";
		shaderInfo.rayGenEntryName = "main";
		shaderInfo.rayMissEntryName = "main";
		shaderInfo.rayClosethitEntryName = "main";
		shaderInfo.rayAnyHitEntryName = "main";
	}
	else
	{
		shaderInfo.rayGenShaderName = "rayTracing.hlsl";
		shaderInfo.rayMissShaderName = "rayTracing.hlsl";
		shaderInfo.rayClosethitShaderName = "rayTracing.hlsl";
		shaderInfo.rayAnyHitShaderName = "rayTracing.hlsl";
		shaderInfo.rayGenEntryName = "rayGenMain";
		shaderInfo.rayMissEntryName = "missMain";
		shaderInfo.rayClosethitEntryName = "closesthitMain";
		shaderInfo.rayAnyHitEntryName = "anyhitMain";
	}
	raytracingHandle = rs->createRaytracingProgram(shaderInfo);
	
	RayTracingContext context;
	initRayTracingContext(context, gltfEntity);
	
	

	TextureProperty texProperty;
	texProperty._width = ogreConfig.width;
	texProperty._height = ogreConfig.height;
	texProperty._tex_format = renderWindow->getColorFormat();
	texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
	auto outPutTarget = rs->createRenderTarget("outputTarget", texProperty);
	OgreTexture* storeImage = outPutTarget->getTarget();
	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(UniformData);
	auto uniformBuffer = rs->createBufferObject(desc);

	

	uint32_t geometryNodesSize = sizeof(GeometryNode) * subMeshCount;

	desc.mBindingType = BufferObjectBinding_Storge;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS;
	desc.mSize = geometryNodesSize;
	desc.mElementCount = subMeshCount;
	desc.mStructStride = sizeof(GeometryNode);
	auto geometryNodesBuffer = rs->createBufferObject(desc);
	
	auto numFrame = ogreConfig.swapBufferCount;
	mFrameInfoList.resize(numFrame);
	
	std::vector<OgreTexture*> textureList;
	std::vector<GeometryNode> geometryNodes;
	geometryNodes.resize(subMeshCount);
	textureList.reserve(15);
	for (auto i = 0; i < subMeshCount; i++)
	{
		auto* subMesh = mesh->getSubMesh(i);
		auto& mat = subMesh->getMaterial();

		VertexData* vertexData = subMesh->getVertexData();
		IndexData* indexData = subMesh->getIndexData();

		mat->updateVertexDeclaration(vertexData->getVertexDeclaration());

		mat->load(nullptr);

		

		auto* baseColorTexture = mat->getPbrTexture(TextureTypePbr_Albedo);
		baseColorTexture = mat->getTexture(0);
		int32_t baseColorIndex = textureList.size();
		textureList.push_back(baseColorTexture);

		auto* occlusionTexture = mat->getPbrTexture(TextureTypePbr_AmbientOcclusion);
		int32_t occlusionIndex = -1;
		if (occlusionTexture)
		{
			occlusionIndex = textureList.size();
			textureList.push_back(occlusionTexture);
		}
		
		

		GeometryNode& geometryNode = geometryNodes[i];
		geometryNode.vertexBufferDeviceAddress = rs->getBufferDeviceAddress(vertexData->getBuffer(0));
		geometryNode.indexBufferDeviceAddress = rs->getBufferDeviceAddress(indexData->getHandle());
		geometryNode.textureIndexBaseColor = baseColorIndex;
		geometryNode.textureIndexOcclusion = occlusionIndex;
	}

	rs->updateBufferObject(geometryNodesBuffer, (const char*)geometryNodes.data(), geometryNodesSize);

	DescriptorData descriptorData[10];
	for (auto i = 0; i < numFrame; i++)
	{
		Handle<HwDescriptorSet> zeroDescSet = rs->createDescriptorSet(raytracingHandle, 0);
		mFrameInfoList[i].zeroDescSetOfRaytracing = zeroDescSet;

		descriptorData[0].mCount = 1;
		descriptorData[0].pName = "topLevelAS";
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE;
		descriptorData[0].pAS = context.pTopAS;

		descriptorData[1].mCount = 1;
		descriptorData[1].pName = "image";
		descriptorData[1].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[1].ppTextures = (const OgreTexture**) & storeImage;

		descriptorData[2].mCount = 1;
		descriptorData[2].pName = "cam";
		descriptorData[2].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[2].ppBuffers = &uniformBuffer;

		descriptorData[3].mCount = 1;
		descriptorData[3].pName = "geometryNodes";
		descriptorData[3].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[3].ppBuffers = &geometryNodesBuffer;

		descriptorData[4].mCount = textureList.size();
		descriptorData[4].pName = "textures";
		descriptorData[4].descriptorType = DESCRIPTOR_TYPE_TEXTURE;
		descriptorData[4].ppTextures = (const OgreTexture**)textureList.data();

		rs->updateDescriptorSet(zeroDescSet, 5, descriptorData);
	}
	

	RenderPassCallback rayTracingCallback = [=, this](RenderPassInfo& info) {
		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		auto* frameInfo = getFrameInfo(frameIndex);
		auto descSet = frameInfo->zeroDescSetOfRaytracing;
		//rs->beginRenderPass(info);
		rs->bindPipeline(raytracingHandle, &descSet, 1);
		rs->traceRay(raytracingHandle);
		//rs->endRenderPass(info);
		{
			RenderTargetBarrier rtBarriers[] =
			{
				{
					renderWindow->getColorTarget(),
					RESOURCE_STATE_PRESENT,
					RESOURCE_STATE_COPY_DEST
				},
				{
					outPutTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_COPY_SOURCE
				}
			};
			rs->resourceBarrier(0, nullptr, 0, nullptr, 2, rtBarriers);
		}
		ImageCopyDesc copyDesc{};
		copyDesc.srcSubresource.aspectMask = 0;
		copyDesc.srcSubresource.baseArrayLayer = 0;
		copyDesc.srcSubresource.mipLevel = 0;
		copyDesc.srcSubresource.layerCount = 1;

		copyDesc.dstSubresource.aspectMask = 0;
		copyDesc.dstSubresource.baseArrayLayer = 0;
		copyDesc.dstSubresource.mipLevel = 0;
		copyDesc.dstSubresource.layerCount = 1;

		copyDesc.extent.width = outPutTarget->getWidth();
		copyDesc.extent.height = outPutTarget->getHeight();
		copyDesc.extent.depth = 1;
		rs->copyImage(renderWindow->getColorTarget(), outPutTarget, copyDesc);

		{
			RenderTargetBarrier rtBarriers[] =
			{
				{
					renderWindow->getColorTarget(),
					RESOURCE_STATE_COPY_DEST,
					RESOURCE_STATE_PRESENT
				},
				{
					outPutTarget,
					RESOURCE_STATE_COPY_SOURCE,
					RESOURCE_STATE_UNORDERED_ACCESS
				}
			};
			rs->resourceBarrier(0, nullptr, 0, nullptr, 2, rtBarriers);
		}
		};

	auto* cam = gameCamera->getCamera();
	UpdatePassCallback rayTracingUpdateCallback = [=, this](float delta) {
		const auto& view = cam->getViewMatrix();
		const auto& project = cam->getProjectMatrix();
		mUniformData.projInverse = project.transpose().inverse();
		mUniformData.viewInverse = view.transpose().inverse();
		mUniformData.frame++;
		rs->updateBufferObject(uniformBuffer,
			(const char*)&mUniformData, sizeof(mUniformData));

		if (gameCamera->changed())
		{
			mUniformData.frame = -1;
			gameCamera->updateChanged(false);
		}
		};

	auto rayTracingPass = createUserDefineRenderPass(
		rayTracingCallback, rayTracingUpdateCallback);
	renderPipeline->addRenderPass(rayTracingPass);

	gameCamera->setMoveSpeed(1.0f);
	Ogre::Vector3 camPos(0.0f, 0.1f, 1.0f);
	Ogre::Vector3 lookAt = Ogre::Vector3::ZERO;
	gameCamera->lookAt(camPos, lookAt);
	float aspect = ogreConfig.width / (float)ogreConfig.height;
	Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrixRH(
		Ogre::Math::PI / 3.0f, aspect, 0.1, 512.f);
	gameCamera->getCamera()->updateProjectMatrix(m);
	gameCamera->setCameraType(CameraMoveType_LookAt);
}

void RayTracingApp::RayTracingShadow(
	RenderPipeline* renderPipeline,
	RenderSystem* renderSystem,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	auto rootNode = sceneManager->getRoot();
	auto root = sceneManager->getRoot();
	std::string meshname = "vulkanscene_shadow.gltf";
	std::shared_ptr<Mesh> mesh = MeshManager::getSingletonPtr()->load(meshname);

	Entity* gltfEntity = sceneManager->createEntity(meshname, meshname);
	SceneNode* gltfNode = rootNode->createChildSceneNode(meshname);
	gltfNode->attachObject(gltfEntity);

	auto subMeshCount = mesh->getSubMeshCount();


	RaytracingShaderInfo shaderInfo;
	Handle<HwRaytracingProgram> raytracingHandle;
	
	if (0)
	{
		shaderInfo.rayTracingShaderName = "rayTracingShadow.hlsl";
		shaderInfo.rayGenEntryName = "rayGenMain";
		shaderInfo.rayMissEntryName = "missMain";
		//shaderInfo.rayShadowEntryName = "shadowMissmain";
		shaderInfo.rayClosethitEntryName = "closethitMain";
	}
	else
	{
		shaderInfo.rayGenShaderName = "rayShadowGen.glsl";
		shaderInfo.rayMissShaderName = "rayShadowMiss.glsl";
		shaderInfo.rayClosethitShaderName = "rayShadowClosethit.glsl";
		shaderInfo.rayGenEntryName = "main";
		shaderInfo.rayMissEntryName = "main";
		//shaderInfo.rayShadowEntryName = "shadowMissmain";
		shaderInfo.rayClosethitEntryName = "main";
	}	
	raytracingHandle = mRenderSystem->createRaytracingProgram(shaderInfo);

	RayTracingContext context;
	initRayTracingContext(context, gltfEntity);

	std::vector<GeometryNode> geometryNodes(subMeshCount);

	for (uint32_t i = 0; i < subMeshCount; i++)
	{
		SubEntity* subEntity = gltfEntity->getSubEntity(i);
		IndexDataView* indexView = subEntity->getIndexView();
		GeometryNode& node = geometryNodes[i];
		node.vertexOffset = indexView->mBaseVertexLocation;
		node.indexOffset = indexView->mIndexLocation;
	}
	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Storge;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS;
	desc.mElementCount = subMeshCount;
	desc.mStructStride = sizeof(GeometryNode);
	desc.mSize = subMeshCount * desc.mStructStride;
	auto geometryNodesBuffer = mRenderSystem->createBufferObject(desc);

	mRenderSystem->updateBufferObject(geometryNodesBuffer, 
		(const char*)geometryNodes.data(), desc.mSize);
	struct UBO
	{
		Ogre::Matrix4 viewInverse;
		Ogre::Matrix4 projInverse;
		Ogre::Vector4 lightPos;
		int vertexSize;
	};

	TextureProperty texProperty;
	texProperty._width = ogreConfig.width;
	texProperty._height = ogreConfig.height;
	texProperty._tex_format = renderWindow->getColorFormat();
	texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
	auto outPutTarget = mRenderSystem->createRenderTarget("outputTarget", texProperty);
	OgreTexture* storeImage = outPutTarget->getTarget();

	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(UBO);
	Handle<HwBufferObject> uniformBuffer = mRenderSystem->createBufferObject(desc);

	auto numFrame = ogreConfig.swapBufferCount;
	mFrameInfoList.resize(numFrame);


	DescriptorData descriptorData[10];
	VertexData* vertexData = mesh->getVertexData();
	IndexData* indexData = mesh->getIndexData();

	Handle<HwBufferObject> indexBufferHandle = indexData->getHandle();

	Handle<HwBufferObject> vertexBufferHandle = vertexData->getBuffer(0);

	for (auto i = 0; i < numFrame; i++)
	{
		Handle<HwDescriptorSet> zeroDescSet = mRenderSystem->createDescriptorSet(raytracingHandle, 0);
		mFrameInfoList[i].zeroDescSetOfRaytracing = zeroDescSet;

		descriptorData[0].mCount = 1;
		descriptorData[0].pName = "topLevelAS";
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE;
		descriptorData[0].pAS = context.pTopAS;

		descriptorData[1].mCount = 1;
		descriptorData[1].pName = "image";
		descriptorData[1].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[1].ppTextures = (const OgreTexture**)&storeImage;

		descriptorData[2].mCount = 1;
		descriptorData[2].pName = "ubo";
		descriptorData[2].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[2].ppBuffers = &uniformBuffer;

		descriptorData[3].mCount = 1;
		descriptorData[3].pName = "vertexDataBuffer";
		descriptorData[3].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[3].ppBuffers = &vertexBufferHandle;

		descriptorData[4].mCount = 1;
		descriptorData[4].pName = "indexDataBuffer";
		descriptorData[4].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[4].ppBuffers = &indexBufferHandle;

		descriptorData[5].mCount = 1;
		descriptorData[5].pName = "geometryNodes";
		descriptorData[5].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[5].ppBuffers = &geometryNodesBuffer;

		mRenderSystem->updateDescriptorSet(zeroDescSet, 6, descriptorData);
	}


	RenderPassCallback rayTracingCallback = [=, this](RenderPassInfo& info) {
		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		auto* frameInfo = getFrameInfo(frameIndex);
		auto descSet = frameInfo->zeroDescSetOfRaytracing;
		//rs->beginRenderPass(info);
		mRenderSystem->bindPipeline(raytracingHandle, &descSet, 1);
		mRenderSystem->traceRay(raytracingHandle);
		//rs->endRenderPass(info);
		{
			RenderTargetBarrier rtBarriers[] =
			{
				{
					renderWindow->getColorTarget(),
					RESOURCE_STATE_PRESENT,
					RESOURCE_STATE_COPY_DEST
				},
				{
					outPutTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_COPY_SOURCE
				}
			};
			mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 2, rtBarriers);
		}
		ImageCopyDesc copyDesc{};
		copyDesc.srcSubresource.aspectMask = 0;
		copyDesc.srcSubresource.baseArrayLayer = 0;
		copyDesc.srcSubresource.mipLevel = 0;
		copyDesc.srcSubresource.layerCount = 1;

		copyDesc.dstSubresource.aspectMask = 0;
		copyDesc.dstSubresource.baseArrayLayer = 0;
		copyDesc.dstSubresource.mipLevel = 0;
		copyDesc.dstSubresource.layerCount = 1;

		copyDesc.extent.width = outPutTarget->getWidth();
		copyDesc.extent.height = outPutTarget->getHeight();
		copyDesc.extent.depth = 1;
		mRenderSystem->copyImage(renderWindow->getColorTarget(), outPutTarget, copyDesc);

		{
			RenderTargetBarrier rtBarriers[] =
			{
				{
					renderWindow->getColorTarget(),
					RESOURCE_STATE_COPY_DEST,
					RESOURCE_STATE_PRESENT
				},
				{
					outPutTarget,
					RESOURCE_STATE_COPY_SOURCE,
					RESOURCE_STATE_UNORDERED_ACCESS
				}
			};
			mRenderSystem->resourceBarrier(0, nullptr, 0, nullptr, 2, rtBarriers);
		}
		};

	auto* cam = gameCamera->getCamera();
	UpdatePassCallback rayTracingUpdateCallback = [=, this](float delta) {
		
		};

	auto rayTracingPass = createUserDefineRenderPass(
		rayTracingCallback, rayTracingUpdateCallback);
	renderPipeline->addRenderPass(rayTracingPass);

	gameCamera->setMoveSpeed(1.0f);
	Ogre::Vector3 camPos(0.0f, 3.0f, -10.0f);
	Ogre::Vector3 lookAt = Ogre::Vector3::ZERO;
	gameCamera->lookAt(camPos, lookAt);
	float aspect = ogreConfig.width / (float)ogreConfig.height;
	Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrixRH(
		Ogre::Math::PI / 3.0f, aspect, 0.1, 512.f);
	gameCamera->getCamera()->updateProjectMatrix(m);
	gameCamera->setCameraType(CameraMoveType_LookAt);

	const auto& view = cam->getViewMatrix();
	const auto& project = cam->getProjectMatrix();
	UBO ubo;
	ubo.projInverse = project.transpose().inverse();
	ubo.viewInverse = view.transpose().inverse();

	ubo.vertexSize = vertexData->getVertexSize(0);
	ubo.lightPos = Ogre::Vector4(40.0f, -50.0f, 25.0f, 0.0f);
	mRenderSystem->updateBufferObject(uniformBuffer,
		(const char*)&ubo, sizeof(ubo));
}

void RayTracingApp::RayTracingBox(
	RenderPipeline* renderPipeline,
	RenderSystem* rs,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	auto rootNode = sceneManager->getRoot();
	auto root = sceneManager->getRoot();
	
	std::vector<uint16_t> indices =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	struct Vertex
	{
		Ogre::Vector3 position;
		Ogre::Vector3 normal;
	};

	// Cube vertices positions and corresponding triangle normals.
	std::vector<Vertex> vertices =
	{
		{ Ogre::Vector3(-1.0f, 1.0f, -1.0f), Ogre::Vector3(0.0f, 1.0f, 0.0f) },
		{ Ogre::Vector3(1.0f, 1.0f, -1.0f), Ogre::Vector3(0.0f, 1.0f, 0.0f) },
		{ Ogre::Vector3(1.0f, 1.0f, 1.0f), Ogre::Vector3(0.0f, 1.0f, 0.0f) },
		{ Ogre::Vector3(-1.0f, 1.0f, 1.0f), Ogre::Vector3(0.0f, 1.0f, 0.0f) },

		{ Ogre::Vector3(-1.0f, -1.0f, -1.0f), Ogre::Vector3(0.0f, -1.0f, 0.0f) },
		{ Ogre::Vector3(1.0f, -1.0f, -1.0f), Ogre::Vector3(0.0f, -1.0f, 0.0f) },
		{ Ogre::Vector3(1.0f, -1.0f, 1.0f), Ogre::Vector3(0.0f, -1.0f, 0.0f) },
		{ Ogre::Vector3(-1.0f, -1.0f, 1.0f), Ogre::Vector3(0.0f, -1.0f, 0.0f) },

		{ Ogre::Vector3(-1.0f, -1.0f, 1.0f), Ogre::Vector3(-1.0f, 0.0f, 0.0f) },
		{ Ogre::Vector3(-1.0f, -1.0f, -1.0f), Ogre::Vector3(-1.0f, 0.0f, 0.0f) },
		{ Ogre::Vector3(-1.0f, 1.0f, -1.0f), Ogre::Vector3(-1.0f, 0.0f, 0.0f) },
		{ Ogre::Vector3(-1.0f, 1.0f, 1.0f), Ogre::Vector3(-1.0f, 0.0f, 0.0f) },

		{ Ogre::Vector3(1.0f, -1.0f, 1.0f), Ogre::Vector3(1.0f, 0.0f, 0.0f) },
		{ Ogre::Vector3(1.0f, -1.0f, -1.0f), Ogre::Vector3(1.0f, 0.0f, 0.0f) },
		{ Ogre::Vector3(1.0f, 1.0f, -1.0f), Ogre::Vector3(1.0f, 0.0f, 0.0f) },
		{ Ogre::Vector3(1.0f, 1.0f, 1.0f), Ogre::Vector3(1.0f, 0.0f, 0.0f) },

		{ Ogre::Vector3(-1.0f, -1.0f, -1.0f), Ogre::Vector3(0.0f, 0.0f, -1.0f) },
		{ Ogre::Vector3(1.0f, -1.0f, -1.0f), Ogre::Vector3(0.0f, 0.0f, -1.0f) },
		{ Ogre::Vector3(1.0f, 1.0f, -1.0f), Ogre::Vector3(0.0f, 0.0f, -1.0f) },
		{ Ogre::Vector3(-1.0f, 1.0f, -1.0f), Ogre::Vector3(0.0f, 0.0f, -1.0f) },

		{ Ogre::Vector3(-1.0f, -1.0f, 1.0f), Ogre::Vector3(0.0f, 0.0f, 1.0f) },
		{ Ogre::Vector3(1.0f, -1.0f, 1.0f), Ogre::Vector3(0.0f, 0.0f, 1.0f) },
		{ Ogre::Vector3(1.0f, 1.0f, 1.0f), Ogre::Vector3(0.0f, 0.0f, 1.0f) },
		{ Ogre::Vector3(-1.0f, 1.0f, 1.0f), Ogre::Vector3(0.0f, 0.0f, 1.0f) },
	};

	auto subMeshCount = 1;


	RaytracingShaderInfo shaderInfo;
	Handle<HwRaytracingProgram> raytracingHandle;

	shaderInfo.rayGenShaderName = "rayTracingTest.hlsl";
	shaderInfo.rayMissShaderName = "rayTracingTest.hlsl";
	shaderInfo.rayClosethitShaderName = "rayTracingTest.hlsl";
	shaderInfo.rayGenEntryName = "MyRaygenShader";
	shaderInfo.rayMissEntryName = "MyMissShader";
	shaderInfo.rayClosethitEntryName = "MyClosestHitShader";


	raytracingHandle = rs->createRaytracingProgram(shaderInfo);


	AccelerationStructureDesc         asDesc = {};
	AccelerationStructureGeometryDesc geomDescs[128] = {};

	std::vector<TransformMatrix> transformMatrices;
	transformMatrices.reserve(subMeshCount);

	
	BufferDesc desc{};

	desc.mBindingType = BufferObjectBinding_Vertex;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT |
		BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS;
	desc.mElementCount = vertices.size();
	desc.mStructStride = sizeof(Vertex);
	desc.mSize = desc.mElementCount * desc.mStructStride;
	Handle< HwBufferObject> vertexBufferHandle = rs->createBufferObject(desc);

	rs->updateBufferObject(vertexBufferHandle, (const char*)vertices.data(), desc.mSize);

	desc.mBindingType = BufferObjectBinding_Index;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT |
		BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS;
	desc.mElementCount = indices.size();
	desc.mStructStride = sizeof(uint16_t);
	desc.mSize = desc.mElementCount * desc.mStructStride;
	desc.raw = true;
	Handle< HwBufferObject> indexBufferHandle = rs->createBufferObject(desc);
	rs->updateBufferObject(indexBufferHandle, (const char*)indices.data(), desc.mSize);

	uint32_t transformSize = sizeof(TransformMatrix) * subMeshCount;
	desc.mBindingType = BufferObjectBinding_Storge;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT | BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS;
	desc.mSize = transformSize;
	desc.mElementCount = subMeshCount;
	desc.mStructStride = sizeof(TransformMatrix);
	Handle< HwBufferObject> transformBufferHandle = rs->createBufferObject(desc);


	for (uint32_t i = 0; i < subMeshCount; i++)
	{
		TransformMatrix transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f };

		Ogre::Matrix4 m = Ogre::Matrix4::IDENTITY;
		transformMatrices.push_back(transformMatrix);
		geomDescs[i].mFlags = ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE;
		geomDescs[i].vertexBufferHandle = vertexBufferHandle;
		geomDescs[i].mVertexCount = (uint32_t)vertices.size();
		geomDescs[i].mVertexStride = sizeof(Vertex);
		geomDescs[i].mVertexElementType = VertexElementType::VET_FLOAT3;
		geomDescs[i].indexBufferHandle = indexBufferHandle;
		geomDescs[i].transformBufferHandle = transformBufferHandle;
		geomDescs[i].mIndexCount = indices.size();
		geomDescs[i].mIndexOffset = 0;
		geomDescs[i].mIndexType = INDEX_TYPE_UINT16;
	}

	rs->updateBufferObject(transformBufferHandle,
		(const char*)transformMatrices.data(), transformSize);
	AccelerationStructure* pSanMiguelBottomAS = nullptr;
	AccelerationStructure* pSanMiguelAS = nullptr;



	if (1)
	{
		asDesc.mBottom.mDescCount = subMeshCount;
		asDesc.mBottom.pGeometryDescs = geomDescs;
		asDesc.mType = ACCELERATION_STRUCTURE_TYPE_BOTTOM;
		asDesc.mFlags = ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

		rs->addAccelerationStructure(&asDesc, &pSanMiguelBottomAS);

		TransformMatrix transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f };

		AccelerationStructureInstanceDesc instanceDesc = {};
		instanceDesc.mFlags = ACCELERATION_STRUCTURE_INSTANCE_FLAG_NONE;
		instanceDesc.mInstanceContributionToHitGroupIndex = 0;
		instanceDesc.mInstanceID = 0;
		instanceDesc.mInstanceMask = 1;
		memcpy(instanceDesc.mTransform, &transformMatrix, sizeof(float[12]));
		instanceDesc.pBottomAS = pSanMiguelBottomAS;

		rs->beginCmd();
		asDesc = {};
		asDesc.mType = ACCELERATION_STRUCTURE_TYPE_TOP;
		asDesc.mFlags = ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		asDesc.mTop.mDescCount = 1;
		asDesc.mTop.pInstanceDescs = &instanceDesc;

		rs->addAccelerationStructure(&asDesc, &pSanMiguelAS);
		// Build Acceleration Structures
		RaytracingBuildASDesc buildASDesc = {};
		buildASDesc.pAccelerationStructure = pSanMiguelBottomAS;
		buildASDesc.mIssueRWBarrier = true;
		rs->buildAccelerationStructure(&buildASDesc);


		buildASDesc = {};
		buildASDesc.pAccelerationStructure = pSanMiguelAS;

		rs->buildAccelerationStructure(&buildASDesc);
		rs->flushCmd(true);
		rs->removeAccelerationStructureScratch(pSanMiguelBottomAS);
		rs->removeAccelerationStructureScratch(pSanMiguelAS);
	}



	TextureProperty texProperty;
	texProperty._width = ogreConfig.width;
	texProperty._height = ogreConfig.height;
	texProperty._tex_format = renderWindow->getColorFormat();
	texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
	auto outPutTarget = rs->createRenderTarget("outputTarget", texProperty);
	OgreTexture* storeImage = outPutTarget->getTarget();

	struct SceneConstantBuffer
	{
		Ogre::Matrix4 projectionToWorld;
		Ogre::Vector4 cameraPosition;
		Ogre::Vector4 lightPosition;
		Ogre::Vector4 lightAmbientColor;
		Ogre::Vector4 lightDiffuseColor;
	};

	struct CubeConstantBuffer
	{
		Ogre::Vector4 albedo;
	};
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(SceneConstantBuffer);
	auto uniformBuffer = rs->createBufferObject(desc);


	
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(CubeConstantBuffer);
	auto objectBuffer = rs->createBufferObject(desc);

	CubeConstantBuffer cubeBuffer;
	cubeBuffer.albedo = Ogre::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	rs->updateBufferObject(objectBuffer, (const char*)&cubeBuffer, sizeof(cubeBuffer));

	auto numFrame = ogreConfig.swapBufferCount;
	mFrameInfoList.resize(numFrame);


	DescriptorData descriptorData[10];
	for (auto i = 0; i < numFrame; i++)
	{
		Handle<HwDescriptorSet> zeroDescSet = rs->createDescriptorSet(raytracingHandle, 0);
		mFrameInfoList[i].zeroDescSetOfRaytracing = zeroDescSet;

		descriptorData[0].mCount = 1;
		descriptorData[0].pName = "Scene";
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE;
		descriptorData[0].pAS = pSanMiguelAS;

		descriptorData[1].mCount = 1;
		descriptorData[1].mLevel = 0;
		descriptorData[1].pName = "RenderTarget";
		descriptorData[1].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[1].ppTextures = (const OgreTexture**)&storeImage;

		descriptorData[2].mCount = 1;
		descriptorData[2].pName = "Indices";
		descriptorData[2].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[2].ppBuffers = &indexBufferHandle;

		descriptorData[3].mCount = 1;
		descriptorData[3].pName = "Vertices";
		descriptorData[3].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[3].ppBuffers = &vertexBufferHandle;

		descriptorData[4].mCount = 1;
		descriptorData[4].pName = "g_cubeCB";
		descriptorData[4].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[4].ppBuffers = &objectBuffer;

		descriptorData[5].mCount = 1;
		descriptorData[5].pName = "g_sceneCB";
		descriptorData[5].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[5].ppBuffers = &uniformBuffer;

		rs->updateDescriptorSet(zeroDescSet, 6, descriptorData);
	}


		RenderPassCallback rayTracingCallback = [=, this](RenderPassInfo& info) {
			auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
			auto* frameInfo = getFrameInfo(frameIndex);
			auto descSet = frameInfo->zeroDescSetOfRaytracing;

			rs->bindPipeline(raytracingHandle, &descSet, 1);
			rs->traceRay(raytracingHandle);

			{
				RenderTargetBarrier rtBarriers[] =
				{
					{
						renderWindow->getColorTarget(),
						RESOURCE_STATE_PRESENT,
						RESOURCE_STATE_COPY_DEST
					},
					{
						outPutTarget,
						RESOURCE_STATE_UNORDERED_ACCESS,
						RESOURCE_STATE_COPY_SOURCE
					}
				};
				rs->resourceBarrier(0, nullptr, 0, nullptr, 2, rtBarriers);
			}
			ImageCopyDesc copyDesc{};
			copyDesc.srcSubresource.aspectMask = 0;
			copyDesc.srcSubresource.baseArrayLayer = 0;
			copyDesc.srcSubresource.mipLevel = 0;
			copyDesc.srcSubresource.layerCount = 1;

			copyDesc.dstSubresource.aspectMask = 0;
			copyDesc.dstSubresource.baseArrayLayer = 0;
			copyDesc.dstSubresource.mipLevel = 0;
			copyDesc.dstSubresource.layerCount = 1;

			copyDesc.extent.width = outPutTarget->getWidth();
			copyDesc.extent.height = outPutTarget->getHeight();
			copyDesc.extent.depth = 1;
			rs->copyImage(renderWindow->getColorTarget(), outPutTarget, copyDesc);

			{
				RenderTargetBarrier rtBarriers[] =
				{
					{
						renderWindow->getColorTarget(),
						RESOURCE_STATE_COPY_DEST,
						RESOURCE_STATE_PRESENT
					},
					{
						outPutTarget,
						RESOURCE_STATE_COPY_SOURCE,
						RESOURCE_STATE_UNORDERED_ACCESS
					}
				};
				rs->resourceBarrier(0, nullptr, 0, nullptr, 2, rtBarriers);
			}
			};
		float aspect = ogreConfig.width / (float)ogreConfig.height;
		float fov = Ogre::Math::PI / 4.0f;
		Ogre::Matrix4 project = Ogre::Math::makePerspectiveMatrixLH(fov, aspect, 1.0f, 125.0f);
		Ogre::Vector3 m_eye(-3.53553391, 2.0f, -3.53553343);
		Ogre::Vector3 m_at = Ogre::Vector3::ZERO;
		Ogre::Vector3 m_up(0.262612849, 0.928476691, 0.262612849);
		Ogre::Matrix4 view = Ogre::Math::makeLookAtLH(m_eye, m_at, m_up);
		Ogre::Matrix4 viewProj = (project * view).transpose();
		SceneConstantBuffer sceneBuffer;
		sceneBuffer.projectionToWorld = viewProj.inverse();
		sceneBuffer.cameraPosition = m_eye;
		sceneBuffer.lightPosition = Ogre::Vector4(0.0f, 1.8f, -3.0f, 1.0f);
		sceneBuffer.lightAmbientColor = Ogre::Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		sceneBuffer.lightDiffuseColor = Ogre::Vector4(0.5f, 0.0f, 0.0f, 1.0f);
		mUniformData.frame++;
		rs->updateBufferObject(uniformBuffer,
			(const char*)&sceneBuffer, sizeof(sceneBuffer));
		auto* cam = gameCamera->getCamera();
		UpdatePassCallback rayTracingUpdateCallback = [=, this](float delta) {
			const Ogre::Matrix4& project = cam->getProjectMatrix();
			const Ogre::Matrix4& view = cam->getViewMatrix();
			Ogre::Matrix4 viewProj = project * view;
			SceneConstantBuffer sceneBuffer;
			sceneBuffer.projectionToWorld = viewProj.transpose().inverse();
			sceneBuffer.cameraPosition = cam->getDerivedPosition();;
			sceneBuffer.lightPosition = Ogre::Vector4(0.0f, 1.8f, -3.0f, 1.0f);
			sceneBuffer.lightAmbientColor = Ogre::Vector4(0.5f, 0.5f, 0.5f, 1.0f);
			sceneBuffer.lightDiffuseColor = Ogre::Vector4(0.5f, 0.0f, 0.0f, 1.0f);
			rs->updateBufferObject(uniformBuffer,
				(const char*)&sceneBuffer, sizeof(sceneBuffer));
			if (gameCamera->changed())
			{
				mUniformData.frame = -1;
				gameCamera->updateChanged(false);
			}
			};

		auto rayTracingPass = createUserDefineRenderPass(
			rayTracingCallback, rayTracingUpdateCallback);
		renderPipeline->addRenderPass(rayTracingPass);

		gameCamera->setMoveSpeed(100.0f);
		Ogre::Vector3 camPos(0.0f, 0.0f, -200.0f);
		Ogre::Vector3 lookAt = Ogre::Vector3::ZERO;
		gameCamera->lookAt(camPos, lookAt);
		aspect = ogreConfig.width / (float)ogreConfig.height;
		fov = Ogre::Math::PI / 3.0f;
		Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrixLH(fov, aspect, 1.0f, 1024);
		gameCamera->getCamera()->updateProjectMatrix(m);
		gameCamera->setCameraType(CameraMoveType_FirstPerson);
}

void RayTracingApp::RayTracingBasic(
	RenderPipeline* renderPipeline,
	RenderSystem* rs,
	Ogre::RenderWindow* renderWindow,
	Ogre::SceneManager* sceneManager,
	GameCamera* gameCamera)
{
	auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	auto rootNode = sceneManager->getRoot();
	auto root = sceneManager->getRoot();

	std::vector<BaseVertex> vertices = {
		{  Ogre::Vector3(1.0f,  1.0f, 0.0f ), Ogre::Vector3(0.0f, 0.0f, 0.0f) },
		{  Ogre::Vector3(-1.0f,  1.0f, 0.0f),  Ogre::Vector3(0.0f, 0.0f, 0.0f)},
		{ Ogre::Vector3(0.0f, -1.0f, 0.0f ),  Ogre::Vector3(0.0f, 0.0f, 0.0f)}
	};

	// Setup indices
	std::vector<uint16_t> indices = { 0, 1, 2 };

	std::string meshName = "RayTracingBasic";

	bool create = createManualMesh(meshName, vertices, indices);
	assert(create);
	Entity* entity = sceneManager->createEntity(meshName, meshName);
	SceneNode* entityNode = rootNode->createChildSceneNode(meshName);
	entityNode->attachObject(entity);
	uint32_t indexCount = static_cast<uint32_t>(indices.size());
	uint32_t indexSize = sizeof(uint16_t);

	auto subMeshCount = 1;


	RaytracingShaderInfo shaderInfo;
	Handle<HwRaytracingProgram> raytracingHandle;

	shaderInfo.rayGenShaderName = "rayTracingBasic.hlsl";
	shaderInfo.rayMissShaderName = "rayTracingBasic.hlsl";
	shaderInfo.rayClosethitShaderName = "rayTracingBasic.hlsl";
	shaderInfo.rayGenEntryName = "genMain";
	shaderInfo.rayMissEntryName = "missMain";
	shaderInfo.rayClosethitEntryName = "closethitMain";


	raytracingHandle = rs->createRaytracingProgram(shaderInfo);

	RayTracingContext context;
	initRayTracingContext(context, entity);

	TextureProperty texProperty;
	texProperty._width = ogreConfig.width;
	texProperty._height = ogreConfig.height;
	texProperty._tex_format = renderWindow->getColorFormat();
	texProperty._tex_usage = Ogre::TextureUsage::WRITEABLE;
	auto outPutTarget = rs->createRenderTarget("outputTarget", texProperty);
	OgreTexture* storeImage = outPutTarget->getTarget();

	struct CameraProperties
	{
		Ogre::Matrix4 viewInverse;
		Ogre::Matrix4 projInverse;
	};

	struct CubeConstantBuffer
	{
		Ogre::Vector4 albedo;
	};

	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mStartState = RESOURCE_STATE_GENERIC_READ;
	desc.mSize = sizeof(CameraProperties);
	auto uniformBuffer = rs->createBufferObject(desc);



	desc.mBindingType = BufferObjectBinding_Uniform;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = 0;
	desc.mSize = sizeof(CubeConstantBuffer);
	auto objectBuffer = rs->createBufferObject(desc);

	CubeConstantBuffer cubeBuffer;
	cubeBuffer.albedo = Ogre::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	rs->updateBufferObject(objectBuffer, (const char*)&cubeBuffer, sizeof(cubeBuffer));

	auto numFrame = ogreConfig.swapBufferCount;
	mFrameInfoList.resize(numFrame);


	DescriptorData descriptorData[10];
	for (auto i = 0; i < numFrame; i++)
	{
		Handle<HwDescriptorSet> zeroDescSet = rs->createDescriptorSet(raytracingHandle, 0);
		mFrameInfoList[i].zeroDescSetOfRaytracing = zeroDescSet;

		descriptorData[0].mCount = 1;
		descriptorData[0].pName = "topLevelAS";
		descriptorData[0].descriptorType = DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE;
		descriptorData[0].pAS = context.pTopAS;

		descriptorData[1].mCount = 1;
		descriptorData[1].mLevel = 0;
		descriptorData[1].pName = "image";
		descriptorData[1].descriptorType = DESCRIPTOR_TYPE_RW_TEXTURE;
		descriptorData[1].ppTextures = (const OgreTexture**)&storeImage;

		descriptorData[2].mCount = 1;
		descriptorData[2].pName = "cam";
		descriptorData[2].descriptorType = DESCRIPTOR_TYPE_BUFFER;
		descriptorData[2].ppBuffers = &uniformBuffer;

		rs->updateDescriptorSet(zeroDescSet, 3, descriptorData);
	}

	int kk = 0;
	RenderPassCallback rayTracingCallback = [=, this](RenderPassInfo& info) {
		auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
		auto* frameInfo = getFrameInfo(frameIndex);
		auto descSet = frameInfo->zeroDescSetOfRaytracing;

		rs->bindPipeline(raytracingHandle, &descSet, 1);
		rs->traceRay(raytracingHandle);

		{
			RenderTargetBarrier rtBarriers[] =
			{
				{
					renderWindow->getColorTarget(),
					RESOURCE_STATE_PRESENT,
					RESOURCE_STATE_COPY_DEST
				},
				{
					outPutTarget,
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_COPY_SOURCE
				}
			};
			rs->resourceBarrier(0, nullptr, 0, nullptr, 2, rtBarriers);
		}
		ImageCopyDesc copyDesc{};
		copyDesc.srcSubresource.aspectMask = 0;
		copyDesc.srcSubresource.baseArrayLayer = 0;
		copyDesc.srcSubresource.mipLevel = 0;
		copyDesc.srcSubresource.layerCount = 1;

		copyDesc.dstSubresource.aspectMask = 0;
		copyDesc.dstSubresource.baseArrayLayer = 0;
		copyDesc.dstSubresource.mipLevel = 0;
		copyDesc.dstSubresource.layerCount = 1;

		copyDesc.extent.width = outPutTarget->getWidth();
		copyDesc.extent.height = outPutTarget->getHeight();
		copyDesc.extent.depth = 1;
		rs->copyImage(renderWindow->getColorTarget(), outPutTarget, copyDesc);

		{
			RenderTargetBarrier rtBarriers[] =
			{
				{
					renderWindow->getColorTarget(),
					RESOURCE_STATE_COPY_DEST,
					RESOURCE_STATE_PRESENT
				},
				{
					outPutTarget,
					RESOURCE_STATE_COPY_SOURCE,
					RESOURCE_STATE_UNORDERED_ACCESS
				}
			};
			rs->resourceBarrier(0, nullptr, 0, nullptr, 2, rtBarriers);
		}
		};
	float aspect = ogreConfig.width / (float)ogreConfig.height;
	float fov = Ogre::Math::PI / 3.0f;
	Ogre::Matrix4 project = Ogre::Math::makePerspectiveMatrixRH(fov, aspect, 0.1f, 512.0f);
	Ogre::Vector3 m_eye(0.0f, 0.0f, -2.5f);
	Ogre::Vector3 m_at = Ogre::Vector3::ZERO;
	Ogre::Vector3 m_up(0.0f, 1.0f, 0.0f);
	Ogre::Matrix4 view = Ogre::Math::makeLookAtRH(m_eye, m_at, m_up);
	
	mUniformData.frame++;

	CameraProperties camProperty;

	camProperty.projInverse = project.transpose().inverse();
	camProperty.viewInverse = view.transpose().inverse();
	rs->updateBufferObject(uniformBuffer,
		(const char*)&camProperty, sizeof(camProperty));
	auto* cam = gameCamera->getCamera();
	UpdatePassCallback rayTracingUpdateCallback = [=, this](float delta) {
		const Ogre::Matrix4& project = cam->getProjectMatrix();
		const Ogre::Matrix4& view = cam->getViewMatrix();
		CameraProperties camProperty;
		camProperty.projInverse = project.transpose().inverse();
		camProperty.viewInverse = view.transpose().inverse();
		rs->updateBufferObject(uniformBuffer,
			(const char*)&camProperty, sizeof(camProperty));
		
		if (gameCamera->changed())
		{
			mUniformData.frame = -1;
			gameCamera->updateChanged(false);
		}
		};

	auto rayTracingPass = createUserDefineRenderPass(
		rayTracingCallback, rayTracingUpdateCallback);
	renderPipeline->addRenderPass(rayTracingPass);

	gameCamera->setMoveSpeed(1.0f);
	Ogre::Vector3 camPos(0.0f, 0.0f, -2.5f);
	Ogre::Vector3 lookAt = Ogre::Vector3::ZERO;
	gameCamera->lookAt(camPos, lookAt);
	aspect = ogreConfig.width / (float)ogreConfig.height;
	fov = Ogre::Math::PI / 3.0f;
	Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrixLH(fov, aspect, 1.0f, 125.0f);

	gameCamera->getCamera()->updateProjectMatrix(m);
	gameCamera->setCameraType(CameraMoveType_LookAt);
}

void RayTracingApp::initRayTracingContext(RayTracingContext& context, Ogre::Entity* entity)
{
	AccelerationStructureDesc         asDesc = {};
	AccelerationStructureGeometryDesc geomDescs[128] = {};

	uint32_t subEntityCount = entity->getNumSubEntities();

	std::vector<TransformMatrix> transformMatrices;
	transformMatrices.reserve(subEntityCount);

	uint32_t transformSize = sizeof(TransformMatrix) * subEntityCount;
	BufferDesc desc{};
	desc.mBindingType = BufferObjectBinding_Storge;
	desc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
	desc.bufferCreationFlags = BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT | 
		BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS;
	desc.mSize = transformSize;
	desc.mElementCount = subEntityCount;
	desc.mStructStride = sizeof(TransformMatrix);
	Handle< HwBufferObject> transformBufferHandle = mRenderSystem->createBufferObject(desc);


	for (uint32_t i = 0; i < subEntityCount; i++)
	{
		SubEntity* subEntity = entity->getSubEntity(i);
		VertexData* vertexData = subEntity->getVertexData();
		IndexData* indexData = subEntity->getIndexData();
		auto& mat = subEntity->getMaterial();
		auto materialFlag = mat->getMaterialFlags();

		
		const Ogre::Matrix4& subMatrix = subEntity->getModelMatrix();
		TransformMatrix transformMatrix;

		memcpy(&transformMatrix, (void*)&subMatrix, sizeof(transformMatrix));
		transformMatrices.push_back(transformMatrix);
		geomDescs[i].mFlags = (materialFlag & MATERIAL_FLAG_ALPHA_TESTED)
			? ACCELERATION_STRUCTURE_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION
			: ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE;
		IndexDataView* indexView = subEntity->getIndexView();
		geomDescs[i].vertexBufferHandle = vertexData->getBuffer(0);
		geomDescs[i].mVertexCount = (uint32_t)vertexData->getVertexCount();
		uint32_t stride = vertexData->getVertexSize(0);
		geomDescs[i].mVertexStride = stride;
		geomDescs[i].mVertexOffset = indexView->mBaseVertexLocation * stride;
		geomDescs[i].mVertexElementType = VertexElementType::VET_FLOAT3;
		geomDescs[i].indexBufferHandle = indexData->getHandle();
		geomDescs[i].transformBufferHandle = transformBufferHandle;
		uint32_t indexSize = indexData->getIndexSize();
		geomDescs[i].mIndexCount = indexView->mIndexCount;
		geomDescs[i].mIndexOffset = indexView->mIndexLocation * indexSize;
		geomDescs[i].mIndexType = indexSize==2 ? INDEX_TYPE_UINT16:INDEX_TYPE_UINT32;
	}

	mRenderSystem->updateBufferObject(transformBufferHandle,
		(const char*)transformMatrices.data(), transformSize);
	AccelerationStructure* pBottomAS = nullptr;
	AccelerationStructure* pTopAS = nullptr;


	asDesc.mBottom.mDescCount = subEntityCount;
	asDesc.mBottom.pGeometryDescs = geomDescs;
	asDesc.mType = ACCELERATION_STRUCTURE_TYPE_BOTTOM;
	asDesc.mFlags = ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

	mRenderSystem->addAccelerationStructure(&asDesc, &pBottomAS);

	TransformMatrix transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f };

	AccelerationStructureInstanceDesc instanceDesc = {};
	instanceDesc.mFlags = ACCELERATION_STRUCTURE_INSTANCE_FLAG_NONE;
	instanceDesc.mInstanceContributionToHitGroupIndex = 0;
	instanceDesc.mInstanceID = 0;
	instanceDesc.mInstanceMask = 1;
	memcpy(instanceDesc.mTransform, &transformMatrix, sizeof(float[12]));
	instanceDesc.pBottomAS = pBottomAS;


	mRenderSystem->beginCmd();
	asDesc = {};
	asDesc.mType = ACCELERATION_STRUCTURE_TYPE_TOP;
	asDesc.mFlags = ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	asDesc.mTop.mDescCount = 1;
	asDesc.mTop.pInstanceDescs = &instanceDesc;

	mRenderSystem->addAccelerationStructure(&asDesc, &pTopAS);

	// Build Acceleration Structures
	RaytracingBuildASDesc buildASDesc = {};
	buildASDesc.pAccelerationStructure = pBottomAS;
	buildASDesc.mIssueRWBarrier = true;
	mRenderSystem->buildAccelerationStructure(&buildASDesc);


	buildASDesc = {};
	buildASDesc.pAccelerationStructure = pTopAS;

	mRenderSystem->buildAccelerationStructure(&buildASDesc);
	mRenderSystem->flushCmd(true);
	mRenderSystem->removeAccelerationStructureScratch(pBottomAS);
	mRenderSystem->removeAccelerationStructureScratch(pTopAS);

	context.pTopAS = pTopAS;
	context.pBottomAS = pBottomAS;
}
