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
#include "OgreCamera.h"
#include "game_camera.h"
#include "OgreMurmurHash3.h"
#include "OgreTextureManager.h"
#include "OgreMeshManager.h"
#include "OgreSubEntity.h"
#include "forgeCommon.h"

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
	if (0)
	{
		RayQuery(renderPipeline, renderSystem, renderWindow, sceneManager, gameCamera);
	}
	else
	{
		RayTracing(renderPipeline, renderSystem, renderWindow, sceneManager, gameCamera);
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

	auto zeroLayout = rs->getDescriptorSetLayout(programHandle, 0);
	auto firstLayout = rs->getDescriptorSetLayout(programHandle, 1);


	std::string meshname = "SanMiguel.bin";
	std::shared_ptr<Mesh> mesh = loadSanMiguel(meshname);

	Entity* sanMiguel = sceneManager->createEntity(meshname, meshname);
	SceneNode* sanMiguelNode = rootNode->createChildSceneNode(meshname);
	sanMiguelNode->attachObject(sanMiguel);

	auto subMeshCount = mesh->getSubMeshCount();
	/************************************************************************/
			// Creation Acceleration Structure
   /************************************************************************/
	AccelerationStructureDesc         asDesc = {};
	AccelerationStructureGeometryDesc geomDescs[512] = {};
	VertexData* vertexData = mesh->getVertexData();
	IndexData* indexData = mesh->getIndexData();
	for (uint32_t i = 0; i < subMeshCount; i++)
	{
		SubMesh* subMesh = mesh->getSubMesh(i);
		auto& mat = subMesh->getMaterial();

		auto materialFlag = mat->getMaterialFlags();

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
	}

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
	auto outPutTarget = rs->createRenderTarget("outputTarget",
		ogreConfig.width, ogreConfig.height, format, Ogre::TextureUsage::WRITEABLE);
	OgreTexture* outputTexture = outPutTarget->getTarget();

	auto numFrame = ogreConfig.swapBufferCount;
	mFrameInfoList.resize(numFrame);

	indexOffsetsBuffer =
		rs->createBufferObject(
			BufferObjectBinding::BufferObjectBinding_Storge,
			0,
			subMeshCount * sizeof(uint32_t));


	Handle<HwBufferObject> vertexDataHandle = vertexData->getBuffer(0);
	Handle<HwBufferObject> indexDataHandle = indexData->getHandle();
	for (auto i = 0; i < numFrame; i++)
	{
		FrameInfo& frameInfo = mFrameInfoList[i];
		auto genConfigBuffer =
			rs->createBufferObject(
				BufferObjectBinding::BufferObjectBinding_Uniform,
				BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT,
				sizeof(ShadersConfigBlock));
		frameInfo.genConfigBuffer = genConfigBuffer;

		auto zeroSet = rs->createDescriptorSet(zeroLayout);
		auto firstSet = rs->createDescriptorSet(firstLayout);
		frameInfo.zeroDescriptorSet = zeroSet;
		frameInfo.firstDescriptorSet = firstSet;
		rs->updateDescriptorSetAccelerationStructure(zeroSet, 0, pSanMiguelAS);
		rs->updateDescriptorSetBuffer(zeroSet, 1, &indexDataHandle, 1);
		rs->updateDescriptorSetBuffer(zeroSet, 2, &vertexDataHandle, 1);
		rs->updateDescriptorSetBuffer(zeroSet, 5, &indexOffsetsBuffer, 1);
		rs->updateDescriptorSetSampler(zeroSet, 6, linearSamplerHandle);
		rs->updateDescriptorSetTexture(zeroSet, 8, &outputTexture, 1, TextureBindType_RW_Image);
		rs->updateDescriptorSetBuffer(firstSet, 0, &genConfigBuffer, 1);
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
		rs->updateDescriptorSetTexture(frameInfo.zeroDescriptorSet, 7, diffuseList.data(), diffuseList.size());
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
		auto zeroLayout = rs->getDescriptorSetLayout(presentHandle, 0);
		for (auto i = 0; i < numFrame; i++)
		{
			auto& frameData = mFrameInfoList[i];
			auto zeroSet = rs->createDescriptorSet(zeroLayout);
			frameData.zeroDescriptorSetOfPresent = zeroSet;
			auto* tex = outPutTarget->getTarget();
			rs->updateDescriptorSetTexture(zeroSet, 0, &tex, 1, TextureBindType_Image);
			rs->updateDescriptorSetSampler(zeroSet, 1, repeatBillinearSampler);
		}

		backend::RasterState rasterState{};
		rasterState.depthWrite = false;
		rasterState.depthTest = false;
		rasterState.depthFunc = SamplerCompareFunc::A;
		rasterState.colorWrite = true;
		rasterState.renderTargetCount = 1;
		rasterState.pixelFormat = Ogre::PixelFormat::PF_A8R8G8B8_SRGB;
		auto pipelineHandle = rs->createPipeline(rasterState, presentHandle);

		RenderPassCallback presentCallback = [=, this](RenderPassInfo& info) {
			TextureBarrier textureBarriers[] =
			{
				{
					outPutTarget->getTarget(),
					RESOURCE_STATE_UNORDERED_ACCESS,
					RESOURCE_STATE_PIXEL_SHADER_RESOURCE
				}
			};
			rs->resourceBarrier(0, nullptr, 1, textureBarriers, 0, nullptr);
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
			RenderTargetBarrier rtBarriers[] =
			{
				renderWindow->getColorTarget(),
				RESOURCE_STATE_RENDER_TARGET,
				RESOURCE_STATE_PRESENT
			};
			rs->resourceBarrier(0, nullptr, 0, nullptr, 1, rtBarriers);
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


void RayTracingApp::RayTracing(
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
	/*auto mesh = MeshManager::getSingletonPtr()->createRect(
		nullptr,
		meshname,
		leftop, leftbottom, righttop, rightbottom, normal);*/
    Entity* gltfEntity = sceneManager->createEntity(meshname, meshname);
    SceneNode* gltfNode = rootNode->createChildSceneNode(meshname);
	gltfNode->attachObject(gltfEntity);

	auto subMeshCount = mesh->getSubMeshCount();
	AccelerationStructureDesc         asDesc = {};
	AccelerationStructureGeometryDesc geomDescs[128] = {};
	
	std::vector<TransformMatrix> transformMatrices;
	transformMatrices.reserve(subMeshCount);

	uint32_t transformSize = sizeof(TransformMatrix) * subMeshCount;

	Handle< HwBufferObject> transformBufferHandle =rs->createBufferObject(
		BufferObjectBinding::BufferObjectBinding_Storge,
		BUFFER_CREATION_FLAG_ACCELERATION_STRUCTURE_BUILD_INPUT | BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS,
		transformSize
	);

	
	for (uint32_t i = 0; i < subMeshCount; i++)
	{
		SubMesh* subMesh = mesh->getSubMesh(i);
		VertexData* vertexData = subMesh->getVertexData();
		IndexData* indexData = subMesh->getIndexData();
		auto& mat = subMesh->getMaterial();
		auto materialFlag = mat->getMaterialFlags();
		
		SubEntity* subEntity = gltfEntity->getSubEntity(i);
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
		geomDescs[i].transformBufferHandle = transformBufferHandle;
		IndexDataView* indexView = subMesh->getIndexView();
		geomDescs[i].mIndexCount = indexView->mIndexCount;
		geomDescs[i].mIndexOffset = indexView->mIndexLocation * sizeof(uint32_t);
		geomDescs[i].mIndexType = INDEX_TYPE_UINT32;
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
			0.0f, -1.0f, 0.0f, 0.0f,
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
	
	auto format = renderWindow->getColorFormat();
	auto outPutTarget = rs->createRenderTarget("outputTarget",
		ogreConfig.width, ogreConfig.height, format, Ogre::TextureUsage::WRITEABLE);
	OgreTexture* storeImage = outPutTarget->getTarget();

	auto uniformBuffer = rs->createBufferObject(
		BufferObjectBinding::BufferObjectBinding_Uniform,
		0,
		sizeof(UniformData)
	);

	ShaderInfo shaderInfo;
	shaderInfo.shaderName = "raytracing";
	Handle<HwRaytracingProgram> raytracingHandle =
		rs->createRaytracingProgram(shaderInfo);
	auto zeroLayout = rs->getDescriptorSetLayout(raytracingHandle, 0);

	uint32_t geometryNodesSize = sizeof(GeometryNode) * subMeshCount;
	auto geometryNodesBuffer = rs->createBufferObject(
		BufferObjectBinding::BufferObjectBinding_Storge,
		BUFFER_CREATION_FLAG_SHADER_DEVICE_ADDRESS,
		geometryNodesSize
	);
	
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
		
		VertexData* vertexData = subMesh->getVertexData();
		IndexData* indexData = subMesh->getIndexData();

		GeometryNode& geometryNode = geometryNodes[i];
		geometryNode.vertexBufferDeviceAddress = rs->getBufferDeviceAddress(vertexData->getBuffer(0));
		geometryNode.indexBufferDeviceAddress = rs->getBufferDeviceAddress(indexData->getHandle());
		geometryNode.textureIndexBaseColor = baseColorIndex;
		geometryNode.textureIndexOcclusion = occlusionIndex;
	}

	rs->updateBufferObject(geometryNodesBuffer, (const char*)geometryNodes.data(), geometryNodesSize);

	for (auto i = 0; i < numFrame; i++)
	{
		Handle<HwDescriptorSet> zeroDescSet = rs->createDescriptorSet(zeroLayout);
		mFrameInfoList[i].zeroDescSetOfRaytracing = zeroDescSet;
		rs->updateDescriptorSetAccelerationStructure(zeroDescSet, 0, pSanMiguelAS);
		rs->updateDescriptorSetTexture(zeroDescSet,
			1, &storeImage, 1, TextureBindType_RW_Image);
		rs->updateDescriptorSetBuffer(zeroDescSet, 2, &uniformBuffer, 1);
		rs->updateDescriptorSetBuffer(zeroDescSet, 4, &geometryNodesBuffer, 1);
		rs->updateDescriptorSetTexture(
			zeroDescSet, 5, textureList.data(), textureList.size(),
			TextureBindType_Combined_Image_Sampler);
	}
	

	RenderPassCallback rayTracingCallback = [=, this](RenderPassInfo& info) {
		info.renderTargetCount = 1;
		info.renderTargets[0].renderTarget = renderWindow->getColorTarget();
		info.depthTarget.depthStencil = renderWindow->getDepthTarget();
		info.renderTargets[0].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
		info.depthTarget.clearValue = { 0.0f, 0.0f };
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
					RESOURCE_STATE_UNDEFINED,
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
		
		rs->copyImage(renderWindow->getColorTarget(), outPutTarget);

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
		mUniformData.projInverse = project.inverse().transpose();
		mUniformData.viewInverse = view.inverse().transpose();
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
	Ogre::Vector3 camPos(0.0f, 0.0, 2.0f);
	Ogre::Vector3 lookAt = Ogre::Vector3::ZERO;
	gameCamera->lookAt(camPos, lookAt);
	float aspect = ogreConfig.width / (float)ogreConfig.height;
	Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrixLH(
		Ogre::Math::PI / 3.0f, aspect, 0.1, 1000.f);
	gameCamera->getCamera()->updateProjectMatrix(m);
	gameCamera->setCameraType(CameraMoveType_ThirdPerson);
}
