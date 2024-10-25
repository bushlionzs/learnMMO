#include "OgreHeader.h"
#include "ShadowMap.h"
#include "engine_manager.h"
#include "myutils.h"
#include "OgreResourceManager.h"
#include "OgreMeshManager.h"
#include "OgreTextureManager.h"
#include "OgreRenderTexture.h"
#include "OgreCamera.h"
#include "OgreAnimationState.h"
#include "OgreLight.h"
#include "OgreMaterialManager.h"
#include "OgreViewport.h"
#include "OgreRenderWindow.h"
#include "renderSystem.h"
#include "game_camera.h"
#include "OgreSceneManager.h"
#include "OgreEntity.h"
#include "OgreRoot.h"
#include "OgreVertexData.h"
#include "PassUtil.h"
#include "pass.h"
#include "shaderManager.h"
#include "forgeCommon.h"
#include "OgreMaterial.h"
#include "OgreTextureUnit.h"
#include "OgreRenderable.h"

#define SAN_MIGUEL_OFFSETX          150.f
#define MESH_SCALE                  10.f
ShadowMap::ShadowMap()
{

}

ShadowMap::~ShadowMap()
{

}


void ShadowMap::setup(
    RenderPipeline* renderPipeline,
    RenderSystem* renderSystem,
    RenderWindow* renderWindow,
    Ogre::SceneManager* sceneManager,
    GameCamera* gameCamera
)
{
    mRenderPipeline = renderPipeline;
    mRenderSystem = renderSystem;
    mRenderWindow = renderWindow;
    mSceneManager = sceneManager;
    mGameCamera = gameCamera;
    base2();
}

void ShadowMap::update(float delta)
{
    mGameCamera->update(delta);
    if (mAnimationState)
    {
        mAnimationState->addTime(delta);
    }

    if (mLightNode)
    {
        mLightNode->yaw(Ogre::Radian(3.14) * delta * 0.1);
    }

    auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
    updateFrameData(frameIndex);

    
}


FrameGraphId<FrameGraphTexture> ShadowMap::fgPass(FrameGraph& fg)
{
    FrameGraphTexture::Descriptor colorBufferDesc = {
        .width = mRenderWindow->getWidth(),
        .height = mRenderWindow->getHeight(),
        .format = backend::TextureFormat::RGB16F
    };

    PassUtil::PassConfig config;


    config.rs = mRenderSystem;
    config.target = mRenderWindow;
    config.scene = Ogre::Root::getSingleton().getSceneManager(MAIN_SCENE_MANAGER);

    config.cam = config.scene->getCamera(MAIN_CAMERA);
    auto color = PassUtil::colorPass(fg, "Color Pass", colorBufferDesc, config);
    return color;
}
void ShadowMap::base1()
{
	auto root = mSceneManager->getRoot();

	MeshManager::getSingletonPtr()->CreateSphere("sphere.mesh", 0.5f, 20, 20);
	MeshManager::getSingletonPtr()->CreateCylinder("cylinder.mesh", 0.5f, 0.3f, 3.0f, 20, 20);

	float aa = 20.0f;
	float h = -2.0f;
	Ogre::Vector3 leftop = Ogre::Vector3(-aa, h, -aa);
	Ogre::Vector3 leftbottom = Ogre::Vector3(-aa, h, aa);
	Ogre::Vector3 righttop = Ogre::Vector3(aa, h, -aa);
	Ogre::Vector3 rightbottom = Ogre::Vector3(aa, h, aa);
	Ogre::Vector3 normal = Ogre::Vector3(0.0f, 1.0f, 0.0f);
	std::string meshName = "myrect";
	auto mesh = MeshManager::getSingletonPtr()->createRect(
		nullptr,
		meshName,
		leftop, leftbottom, righttop, rightbottom, normal);
	auto mat = MaterialManager::getSingleton().getByName("myground");
	mesh->getSubMesh(0)->setMaterial(mat);
	Entity* ground = mSceneManager->createEntity("ground", meshName);
	SceneNode* groundnode = root->createChildSceneNode("ground");
	groundnode->attachObject(ground);

	Entity* sphere = mSceneManager->createEntity("sphere", "sphere.mesh");
	SceneNode* spherenode = root->createChildSceneNode("sphere");
	spherenode->setPosition(Ogre::Vector3(0.0f, 1.5f, 0.0f));
	spherenode->attachObject(sphere);
	spherenode->_getDerivedPosition();
	Entity* cylinder = mSceneManager->createEntity("cylinder", "cylinder.mesh");
	SceneNode* cylindernode = root->createChildSceneNode("cylinder");
	cylindernode->attachObject(cylinder);
	cylindernode->setPosition(Ogre::Vector3(0, 0.0, 0.0f));


	mesh = MeshManager::getSingleton().load("ninja.mesh");
	Entity* ninja = mSceneManager->createEntity("ninja", "ninja.mesh");

	SceneNode* ninjanode = root->createChildSceneNode("ninja");
	ninjanode->attachObject(ninja);
	ninjanode->setPosition(Ogre::Vector3(-4, -2, -1.0f));
	ninjanode->yaw(Ogre::Radian(3.14f));
	float s = 0.015f;
	ninjanode->setScale(Ogre::Vector3(s, s, s));

	ninja->setCastShadows(true);
	sphere->setCastShadows(true);
	cylinder->setCastShadows(true);

	mAnimationState = ninja->getAnimationState(std::string("Walk"));
	if (mAnimationState)
	{
		mAnimationState->setEnabled(true);
		mAnimationState->setLoop(true);
	}

	mGameCamera->updatePosition(Ogre::Vector3(0, 0.0f, 8.0f));
	mGameCamera->setMoveSpeed(10.0f);

	light = mSceneManager->createLight("shadow");
	light->setLightType(LightType_Direction, 0);

	mLightNode = root->createChildSceneNode("light");

	auto m4 = Ogre::Math::makeLookAtLH(
		Ogre::Vector3(-20, 20, -20), Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_Y);

	auto q = m4.extractQuaternion();

	auto subnode = mLightNode->createChildSceneNode("shadow");
	subnode->attachObject(light);
	subnode->setOrientation(q);
	subnode->setPosition(Ogre::Vector3(-20, 20, -20));

	light->mViewMatrix = Ogre::Math::makeLookAtLH(
		Ogre::Vector3(-20, 20, -20), Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_Y);


	int width = 10;
	int height = 10;

	Real left = -width / 2.0f;
	Real right = width / 2.0f;
	Real top = height / 2.0f;
	Real bottom = -height / 2.0f;


	light->mProjMatrix = Ogre::Math::makeOrthoLH(left, right, bottom, top, 1.0f, 10000.0f);


	auto shadowSize = 1024;

	auto shadowMap = mRenderSystem->createRenderTarget(
		"shadow", shadowSize, shadowSize, Ogre::PF_DEPTH32F,
        Ogre::TextureUsage::DEPTH_ATTACHMENT);

    bool useShadow = false;
    RenderPassInput renderInput;
    renderInput.cam = light;
    renderInput.color = nullptr;
    renderInput.depth = shadowMap;
    renderInput.sceneMgr = mSceneManager;
    renderInput.shadowMapTarget = nullptr;
    renderInput.shadowPass = true;
    if (useShadow)
    {
        auto shadowPass = createStandardRenderPass(renderInput);
        mRenderPipeline->addRenderPass(shadowPass);
    }
    

    renderInput.cam = mGameCamera->getCamera();
    renderInput.color = mRenderWindow->getColorTarget();
    renderInput.depth = mRenderWindow->getDepthTarget();
    renderInput.sceneMgr = mSceneManager;
    if (useShadow)
    {
        renderInput.shadowMapTarget = shadowMap;
        renderInput.light = light;
    }
    
    renderInput.shadowPass = false;
    
    auto mainPass = createStandardRenderPass(renderInput);
    mRenderPipeline->addRenderPass(mainPass);
}

#define BATCH_FACE_COUNT_LOW_BIT 2
#define BATCH_FACE_COUNT_MASK 0x1FFC
#define BATCH_GEOMETRY_LOW_BIT 0
#define BATCH_GEOMETRY_MASK 0x3
enum
{
    GEOMSET_OPAQUE = 0,
    GEOMSET_ALPHA_CUTOUT
};


void ShadowMap::updateFrameData(uint32_t i)
{
    auto frameIndex = i;
    auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
    auto width = ogreConfig.width;
    auto height = ogreConfig.height;

    auto cam = mGameCamera->getCamera();
    const Ogre::Matrix4& view = cam->getViewMatrix();
    const Ogre::Matrix4& project = cam->getProjectMatrix();


    if (!mFrameData.empty())
    {
        FrameData& frameData = mFrameData[frameIndex];
        
        cameraUniform.mView = view.transpose();
        cameraUniform.mProject = project.transpose();
        cameraUniform.mViewProject = (project * view).transpose();
        cameraUniform.mInvView = cameraUniform.mView.inverse();
        cameraUniform.mInvProj = cameraUniform.mProject.inverse();
        cameraUniform.mInvViewProject = cameraUniform.mViewProject.inverse();
        cameraUniform.mCameraPos = cam->getDerivedPosition();
        cameraUniform.mNear = cam->getNear();
        cameraUniform.mFar = cam->getFar();
        cameraUniform.mFarNearDiff = cameraUniform.mFar - cameraUniform.mNear;
        cameraUniform.mFarNear = cameraUniform.mFar * cameraUniform.mNear;
        cameraUniform.mTwoOverRes = Ogre::Vector2(1.5f / width, 1.5f / height);
        cameraUniform.mWindowSize = Ogre::Vector2(width, height);

        mRenderSystem->updateBufferObject(
            frameData.cameraUniformHandle,
            (const char*)&cameraUniform,
            sizeof(cameraUniform));

        Ogre::Vector3 lightSourcePos(10.f, 0.0f, 10.f);
        lightSourcePos[0] += (20.f);
        lightSourcePos[0] += (SAN_MIGUEL_OFFSETX);

        Ogre::Matrix4 rotation = Ogre::Math::makeRotateMatrixXY(
            lightCpuSettings.mSunControl.x, lightCpuSettings.mSunControl.y);

        Ogre::Matrix4 translation = Ogre::Math::makeTranslateMatrix(-lightSourcePos);

        auto newLightDir = rotation.inverse() * Ogre::Vector4(0, 0, 1, 0);

        auto lightViewMatrix = rotation * translation;

        Real left = -140.0f;
        Real right = 140.0f;
        Real top = 90.0f;
        Real bottom = -210.0f;

        auto lightProjMatrix = Ogre::Math::makeOrthoLH(left, right, bottom, top, -220, 100);

        esmMeshUniformBlock.worldViewProjMat =
            (lightProjMatrix * lightViewMatrix * meshInfoStruct.mWorldMat).transpose();
        esmMeshUniformBlock.viewID = 1;
        mRenderSystem->updateBufferObject(
            frameData.esmUniformBlockHandle,
            (const char*)&esmMeshUniformBlock,
            sizeof(esmMeshUniformBlock));

        lightUniformBlock.mLightPosition = Ogre::Vector4(0.0f);
        lightUniformBlock.mLightViewProj = (lightProjMatrix * lightViewMatrix).transpose();
        lightUniformBlock.mLightColor = Ogre::Vector4(1, 1, 1, 1);
        lightUniformBlock.mLightUpVec = lightViewMatrix.getUp();
        lightUniformBlock.mLightDir = newLightDir.xyz();

        mRenderSystem->updateBufferObject(
            frameData.ligthUniformHandle,
            (const char*)&lightUniformBlock,
            sizeof(lightUniformBlock));


        perFrameVBConstants.numViewports = 2;
        perFrameVBConstants.transform[VIEW_CAMERA].vp = (project * view).transpose();
        perFrameVBConstants.transform[VIEW_CAMERA].mvp = (project * view * meshInfoStruct.mWorldMat).transpose();
        perFrameVBConstants.cullingViewports[VIEW_CAMERA].windowSize = { (float)width, (float)height };
        perFrameVBConstants.cullingViewports[VIEW_CAMERA].sampleCount = 1;


        perFrameVBConstants.transform[VIEW_SHADOW].mvp =
            (lightProjMatrix * lightViewMatrix * meshInfoStruct.mWorldMat).transpose();
        perFrameVBConstants.cullingViewports[VIEW_SHADOW].windowSize = { 2048,2048 };
        perFrameVBConstants.cullingViewports[VIEW_SHADOW].sampleCount = 1;

        mRenderSystem->updateBufferObject(
            frameData.perFrameConstantsBuffer,
            (const char*)&perFrameVBConstants,
            sizeof(perFrameVBConstants));

        meshUniformBlock.worldViewProjMat = (project * view * meshInfoStruct.mWorldMat).transpose();
        meshUniformBlock.viewID = 0;
        mRenderSystem->updateBufferObject(
            frameData.objectUniformBlockHandle,
            (const char*)&meshUniformBlock,
            sizeof(meshUniformBlock));

        mRenderSystem->updateBufferObject(sssEnabledHandle,
            (const char*)&sssEnabled, sizeof(sssEnabled));
        mRenderSystem->updateBufferObject(esmInputConstantsHandle,
            (const char*)&esmConstants, sizeof(esmConstants));
        mRenderSystem->updateBufferObject(renderSettingsUniformHandle,
            (const char*)&renderSetting, sizeof(renderSetting));
    }
}

void ShadowMap::base2()
{
    auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
	auto rootNode = mSceneManager->getRoot();
    auto root = mSceneManager->getRoot();
    std::shared_ptr<OgreTexture> defaultTex = 
        TextureManager::getSingleton().getByName("white1x1.dds");
    
    meshInfoStruct.mColor = Ogre::Vector4(1.f);
    meshInfoStruct.mScale = Ogre::Vector3(MESH_SCALE);
    meshInfoStruct.mScaleMat = Ogre::Matrix4::IDENTITY;
    meshInfoStruct.mScaleMat.setScale(meshInfoStruct.mScale);
    float finalXTranslation = SAN_MIGUEL_OFFSETX;
    meshInfoStruct.mTranslation = Ogre::Vector3(finalXTranslation, 0.f, 0.f);
    meshInfoStruct.mOffsetTranslation = Ogre::Vector3(0.0f, 0.f, 0.f);
    meshInfoStruct.mTranslationMat = Ogre::Matrix4::IDENTITY;
    meshInfoStruct.mTranslationMat.setTrans(meshInfoStruct.mTranslation);

    meshInfoStruct.mWorldMat = meshInfoStruct.mTranslationMat * meshInfoStruct.mScaleMat;

	std::string meshname = "SanMiguel.bin";
    std::shared_ptr<Mesh> mesh = loadSanMiguel(meshname);
    
	Entity* sanMiguel = mSceneManager->createEntity(meshname, meshname);
	SceneNode* sanMiguelNode = rootNode->createChildSceneNode(meshname);
	sanMiguelNode->attachObject(sanMiguel);

    Ogre::Vector3 camPos(120.f + SAN_MIGUEL_OFFSETX, 98.f, 14.f);

	mGameCamera->updatePosition(camPos);
	mGameCamera->setMoveSpeed(10.0f);

    uint32_t visibilityBufferFilteredIndexCount[NUM_GEOMETRY_SETS] = {};
    auto numFrame = ogreConfig.swapBufferCount;
    mFrameData.resize(numFrame);
    auto subMeshCount = mesh->getSubMeshCount();
    std::vector<MeshConstants> meshConstants(subMeshCount);
    auto* rs = mRenderSystem;
    
    VertexData* vertexData = mesh->getVertexData();
    VertexDeclaration* vertexDecl = vertexData->getVertexDeclaration();
    Handle<HwBufferObject> vertexDataHandle = vertexData->getBuffer(0); 
    IndexData* indexData = mesh->getIndexData();
    Handle<HwBufferObject> indexDataHandle = indexData->getHandle();
    
    uint32_t meshConstantsBufferSize = subMeshCount * sizeof(MeshConstants);
    meshConstantsBuffer =
        rs->createBufferObject(
            BufferObjectBinding::BufferObjectBinding_Storge,
            BufferUsage::DYNAMIC,
            meshConstantsBufferSize,
            "meshConstantsBuffer");

    // Calculate mesh constants and filter containers
    for (auto i = 0; i < subMeshCount; i++)
    {
        auto subMesh = mesh->getSubMesh(i);

        auto& mat = subMesh->getMaterial();
        auto materialFlag = mat->getMaterialFlags();

        uint32_t geomSet = materialFlag & MATERIAL_FLAG_ALPHA_TESTED ? GEOMSET_ALPHA_CUTOUT : GEOMSET_OPAQUE;

        auto indexView = subMesh->getIndexView();
        visibilityBufferFilteredIndexCount[geomSet] += indexView->mIndexCount;

        meshConstants[i].indexOffset = indexView->mIndexLocation;
        meshConstants[i].vertexOffset = indexView->mBaseVertexLocation;
        meshConstants[i].materialID = i;
        meshConstants[i].twoSided =  (materialFlag & MATERIAL_FLAG_TWO_SIDED) ? 1 : 0;
    }

    rs->updateBufferObject(meshConstantsBuffer, (const char*)meshConstants.data(), 
        meshConstantsBufferSize);

    
    uint32_t maxIndices = 0;
    for (uint32_t geomSet = 0; geomSet < NUM_GEOMETRY_SETS; ++geomSet)
    {
        maxIndices += visibilityBufferFilteredIndexCount[geomSet];
    }

    for (auto i = 0; i < NUM_CULLING_VIEWPORTS; i++)
    {
        filteredIndexBuffer[i] =
            rs->createBufferObject(
                BufferObjectBinding::BufferObjectBinding_Storge | BufferObjectBinding::BufferObjectBinding_Index,
                BufferUsage::DYNAMIC,
                maxIndices * sizeof(uint32_t));
    }
    uint32_t computeThread = 256;

    auto maxFilterBatches = (maxIndices / 3) / (computeThread >> 1);

    uint32_t filterDispatchGroupSize = maxFilterBatches * sizeof(FilterDispatchGroupData);

    uint32_t dispatchGroupCount = 0;

    //group bufer

    
    dispatchGroupCount = 0;
    filterDispatchGroupDataBuffer =
        rs->createBufferObject(
            BufferObjectBinding::BufferObjectBinding_Storge,
            BufferUsage::DYNAMIC,
            filterDispatchGroupSize);
    BufferHandleLockGuard lockGuard(filterDispatchGroupDataBuffer);

    FilterDispatchGroupData* dispatchGroupData = (FilterDispatchGroupData*)lockGuard.data();

    for (auto i = 0; i < subMeshCount; i++)
    {
        auto subMesh = mesh->getSubMesh(i);

        uint32_t triangleCount = subMesh->getIndexView()->mIndexCount / 3;
        uint32_t numDispatchGroups = (triangleCount + computeThread - 1) / computeThread;
        auto& mat = subMesh->getMaterial();

        auto materialFlag = mat->getMaterialFlags();

        uint32_t geomSet = materialFlag & MATERIAL_FLAG_ALPHA_TESTED ? GEOMSET_ALPHA_CUTOUT : GEOMSET_OPAQUE;
        for (uint32_t groupIdx = 0; groupIdx < numDispatchGroups; ++groupIdx)
        {
            FilterDispatchGroupData& groupData = dispatchGroupData[dispatchGroupCount++];
            const uint32_t firstTriangle = groupIdx * computeThread;
            const uint32_t lastTriangle = std::min(firstTriangle + computeThread, triangleCount);
            const uint32_t trianglesInGroup = lastTriangle - firstTriangle;

            // Fill GPU filter batch data

            groupData.meshIndex = i;
            groupData.instanceDataIndex = 0;

            groupData.geometrySet_faceCount = 
                ((trianglesInGroup << BATCH_FACE_COUNT_LOW_BIT) & BATCH_FACE_COUNT_MASK) |
                ((geomSet << BATCH_GEOMETRY_LOW_BIT) & BATCH_GEOMETRY_MASK);

            // Offset relative to the start of the mesh
            groupData.indexOffset = firstTriangle * 3;
        }
    }
    
    
    //global data
    VBConstants constants[2];
    uint32_t indexOffset = 0;

    for (uint32_t geomSet = 0; geomSet < NUM_GEOMETRY_SETS; ++geomSet)
    {
        constants[geomSet].indexOffset = indexOffset;
        indexOffset += visibilityBufferFilteredIndexCount[geomSet];
    }

    vbConstantsBuffer =
        mRenderSystem->createBufferObject(BufferObjectBinding::BufferObjectBinding_Uniform, 
            BufferUsage::STATIC, sizeof(VBConstants) * 2);

    rs->updateBufferObject(vbConstantsBuffer, (const char*)&constants[0], sizeof(VBConstants) * 2);

    renderSettingsUniformHandle = rs->createBufferObject(
        BufferObjectBinding::BufferObjectBinding_Uniform,
        BufferUsage::STATIC,
        sizeof(renderSetting)
    );

    rs->updateBufferObject(renderSettingsUniformHandle,
        (const char*)&renderSetting, sizeof(renderSetting));

    esmInputConstantsHandle = rs->createBufferObject(
        BufferObjectBinding::BufferObjectBinding_Uniform,
        BufferUsage::STATIC,
        sizeof(ESMInputConstants)
    );

    rs->updateBufferObject(esmInputConstantsHandle,
        (const char*)&esmConstants, sizeof(esmConstants));
    sssEnabledHandle = rs->createBufferObject(
        BufferObjectBinding::BufferObjectBinding_Uniform,
        BufferUsage::STATIC,
        sizeof(uint32_t)
    );
    rs->updateBufferObject(sssEnabledHandle, (const char*)&sssEnabled, sizeof(sssEnabled));

    uint32_t indirectDrawArgBufferSize = NUM_GEOMETRY_SETS * NUM_CULLING_VIEWPORTS * 8 * sizeof(uint32_t);

    

    for (auto i = 0; i < numFrame; i++)
    {
        auto& frameData = mFrameData[i];
        frameData.frameBufferObject =
            mRenderSystem->createBufferObject(
                BufferObjectBinding::BufferObjectBinding_Uniform,
                BufferUsage::DYNAMIC, sizeof(FrameConstantBuffer));

        frameData.indirectDrawArgBuffer =
            mRenderSystem->createBufferObject(
                BufferObjectBinding::BufferObjectBinding_Storge | BufferObjectBinding_InDirectBuffer,
                BufferUsage::DYNAMIC,
                indirectDrawArgBufferSize);

        frameData.objectUniformBlockHandle =
            rs->createBufferObject(
                BufferObjectBinding::BufferObjectBinding_Uniform,
                BufferUsage::DYNAMIC,
                sizeof(MeshInfoUniformBlock),
                "objectUniformBlock Buffer");

        frameData.indirectDataBuffer =
            rs->createBufferObject(
                BufferObjectBinding::BufferObjectBinding_Storge,
                BufferUsage::DYNAMIC,
                maxIndices * sizeof(uint32_t));

        frameData.perFrameConstantsBuffer =
            rs->createBufferObject(
                BufferObjectBinding::BufferObjectBinding_Uniform,
                BufferUsage::DYNAMIC,
                sizeof(PerFrameVBConstants),
                "PerFrameVBConstants Buffer Desc");

        frameData.esmUniformBlockHandle =
            rs->createBufferObject(
                BufferObjectBinding::BufferObjectBinding_Uniform,
                BufferUsage::DYNAMIC,
                sizeof(MeshInfoUniformBlock),
                "esmUniformBlock Buffer");

        frameData.cameraUniformHandle = rs->createBufferObject(
            BufferObjectBinding::BufferObjectBinding_Uniform,
            BufferUsage::STATIC,
            sizeof(cameraUniform)
        );
        frameData.ligthUniformHandle = rs->createBufferObject(
            BufferObjectBinding::BufferObjectBinding_Uniform,
            BufferUsage::STATIC,
            sizeof(lightUniformBlock)
        );
    }
    
    //clear buffer pass
    bool visibity = true;

    if(visibity)
    {
        ShaderInfo shaderInfo;
        shaderInfo.shaderName = "clearBuffer";
        auto clearBufferProgramHandle = rs->createComputeProgram(shaderInfo);

        Handle<HwDescriptorSetLayout> zeroLayout = rs->getDescriptorSetLayout(clearBufferProgramHandle, 0);
        


        for (auto i = 0; i < numFrame; i++)
        {
            Handle<HwDescriptorSet> descrSet = rs->createDescriptorSet(zeroLayout);
            rs->updateDescriptorSetBuffer(descrSet, 0, &mFrameData[i].indirectDrawArgBuffer, 1);
            rs->updateDescriptorSetBuffer(descrSet, 2, &vbConstantsBuffer, 1);

            mFrameData[i].clearBufferDescrSet = descrSet;
        }
        

        ComputePassCallback callback = [clearBufferProgramHandle, rs, this](ComputePassInfo& info) {
            info.programHandle = clearBufferProgramHandle;
            info.computeGroup = Ogre::Vector3i(1, 1, 1);
            auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
            FrameData* frameData = this->getFrameData(frameIndex);
            if (frameData->update)
                return;
            info.descSets.clear();
            info.descSets.push_back(frameData->clearBufferDescrSet);
            rs->pushGroupMarker("clearBuffer");
            rs->beginComputePass(info);
            rs->endComputePass();
            rs->popGroupMarker();
            // Clear Buffers Synchronization 
            {
                BufferBarrier barriers[2];

                barriers[0] =
                {
                    filterDispatchGroupDataBuffer,
                    RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_UNORDERED_ACCESS
                };

                barriers[1] =
                {
                    frameData->indirectDrawArgBuffer,
                    RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_UNORDERED_ACCESS
                };

                rs->resourceBarrier(2, &barriers[0], 0, nullptr);
            }
            };
        auto clearBufferPass = createComputePass(callback);
        mRenderPipeline->addRenderPass(clearBufferPass);

    }
    
    //filter triangles pass
    if(visibity)
    {
        ShaderInfo shaderInfo;
        shaderInfo.shaderName = "filterTriangles";
        auto filterTrianglesProgramHandle = rs->createComputeProgram(shaderInfo);
       
        Handle<HwDescriptorSetLayout> zeroLayoutHandle =
            rs->getDescriptorSetLayout(filterTrianglesProgramHandle, 0);

        Handle<HwDescriptorSetLayout> firstLayoutHandle =
            rs->getDescriptorSetLayout(filterTrianglesProgramHandle, 1);


        for (auto i = 0; i < numFrame; i++)
        {
            auto& frameData = mFrameData[i];
            
            Handle <HwDescriptorSet> zeroDescSet = rs->createDescriptorSet(zeroLayoutHandle);
            frameData.zeroDescSetOfFilter = zeroDescSet;
            rs->updateDescriptorSetBuffer(zeroDescSet, 0, &frameData.indirectDrawArgBuffer, 1);

            
            rs->updateDescriptorSetBuffer(zeroDescSet, 1, &vertexDataHandle, 1);
            rs->updateDescriptorSetBuffer(zeroDescSet, 2, &vbConstantsBuffer, 1);
            
            rs->updateDescriptorSetBuffer(zeroDescSet, 4, &indexDataHandle, 1);
            rs->updateDescriptorSetBuffer(zeroDescSet, 5, &meshConstantsBuffer, 1);


            Handle <HwDescriptorSet> firstDescSet = rs->createDescriptorSet(firstLayoutHandle);
            frameData.firstDescSetOfFilter = firstDescSet;
            rs->updateDescriptorSetBuffer(firstDescSet, 1, &frameData.perFrameConstantsBuffer, 1);

            rs->updateDescriptorSetBuffer(firstDescSet, 3, &filterDispatchGroupDataBuffer, 1);

            rs->updateDescriptorSetBuffer(firstDescSet, 6, &frameData.indirectDataBuffer, 1);

            rs->updateDescriptorSetBuffer(firstDescSet, 8, &filteredIndexBuffer[0], NUM_CULLING_VIEWPORTS);
        }
        ComputePassCallback callback = [=, this](ComputePassInfo& info) {
            auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
            auto* frameData = this->getFrameData(frameIndex);
            if (frameData->update)
                return;
           // frameData->update = true;
            info.programHandle = filterTrianglesProgramHandle;
            info.computeGroup = Ogre::Vector3i(dispatchGroupCount, 1, 1);
            info.descSets.clear();
            info.descSets.push_back(frameData->zeroDescSetOfFilter);
            info.descSets.push_back(frameData->firstDescSetOfFilter);
            auto* rs = Ogre::Root::getSingleton().getRenderSystem();
            rs->pushGroupMarker("filterTriangles");
            rs->beginComputePass(info);
            rs->endComputePass();
            rs->popGroupMarker();
            {
                const uint32_t numBarriers = NUM_CULLING_VIEWPORTS + 2;
                BufferBarrier  barriers[numBarriers] = {};
                uint32_t       barrierCount = 0;

                barriers[barrierCount++] =
                {
                    frameData->indirectDrawArgBuffer,
                    RESOURCE_STATE_UNORDERED_ACCESS, 
                    RESOURCE_STATE_INDIRECT_ARGUMENT | RESOURCE_STATE_SHADER_RESOURCE
                };

                barriers[barrierCount++] =
                {
                    frameData->indirectDataBuffer,
                    RESOURCE_STATE_UNORDERED_ACCESS, 
                    RESOURCE_STATE_SHADER_RESOURCE
                };

                for (auto i = 0; i < 2; i++)
                {
                    barriers[barrierCount++] = 
                    { 
                        filteredIndexBuffer[i], 
                        RESOURCE_STATE_UNORDERED_ACCESS,
                        RESOURCE_STATE_INDEX_BUFFER | RESOURCE_STATE_SHADER_RESOURCE
                    };
                }
                

                rs->resourceBarrier(barrierCount, barriers, 0, nullptr);
            }
            };

        auto filterTrianglesPass = createComputePass(callback);
        mRenderPipeline->addRenderPass(filterTrianglesPass);

    }

    backend::SamplerParams samplerParams;

    samplerParams.filterMag = backend::SamplerFilterType::NEAREST;
    samplerParams.filterMin = backend::SamplerFilterType::NEAREST;
    samplerParams.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST;
    samplerParams.compareMode = backend::SamplerCompareMode::NONE;
    samplerParams.compareFunc = backend::SamplerCompareFunc::N;
    samplerParams.wrapS = backend::SamplerWrapMode::CLAMP_TO_EDGE;
    samplerParams.wrapT = backend::SamplerWrapMode::CLAMP_TO_EDGE;
    samplerParams.wrapR = backend::SamplerWrapMode::CLAMP_TO_EDGE;
    samplerParams.anisotropyLog2 = 0;
    samplerParams.padding0 = 0;
    samplerParams.padding1 = 0;
    samplerParams.padding2 = 0;
    auto nearSamplerHandle = rs->createTextureSampler(samplerParams);

    auto shadowSize = 2048;

    esmShadowMap = mRenderSystem->createRenderTarget(
        "shadow", shadowSize, shadowSize, Ogre::PF_DEPTH32F,
        Ogre::TextureUsage::DEPTH_ATTACHMENT);

    //draw esm shadow map
    if(1)
    {
        ShaderInfo shaderInfo;
        shaderInfo.shaderName = "meshDepth";
        auto meshDepthHandle = rs->createShaderProgram(shaderInfo, nullptr);
        shaderInfo.shaderName = "meshDepthAlpha";
        auto meshDepthAlphaHandle = rs->createShaderProgram(shaderInfo, nullptr);

        backend::RasterState rasterState{};
        rasterState.colorWrite = false;
        rasterState.depthWrite = true;
        rasterState.depthTest = true;
        rasterState.pixelFormat = Ogre::PixelFormat::PF_A8R8G8B8;
        rasterState.depthFunc = SamplerCompareFunc::LE;
        auto meshDepthPipelineHandle = rs->createPipeline(rasterState, meshDepthHandle);

        auto meshDepthAlphaPipelineHandle = rs->createPipeline(rasterState, meshDepthAlphaHandle);

  
        auto zeroLayout = rs->getDescriptorSetLayout(meshDepthHandle, 0);
        auto thirdLayout = rs->getDescriptorSetLayout(meshDepthHandle, 3);

        auto zeroLayoutAlpha = rs->getDescriptorSetLayout(meshDepthAlphaHandle, 0);
        auto firstLayoutAlpha = rs->getDescriptorSetLayout(meshDepthAlphaHandle, 1);
        auto thirdLayoutAlpha = rs->getDescriptorSetLayout(meshDepthAlphaHandle, 3);

        std::vector<OgreTexture*> diffuseList;
        bool alpha = true;
        if (1)
        {
            auto subMeshCount = mesh->getSubMeshCount();
            diffuseList.reserve(256);
            for (auto i = 0; i < subMeshCount; i++)
            {
                auto* subMesh = mesh->getSubMesh(i);
                auto& mat = subMesh->getMaterial();
                mat->updateVertexDeclaration(vertexDecl);
                mat->load(nullptr);
                auto& unitList = mat->getAllTexureUnit();
                auto* diffuseTex = unitList[0]->getRaw();
                diffuseList.push_back(diffuseTex);
            }

            for (auto i = subMeshCount; i < 256; i++)
            {
                diffuseList.push_back(defaultTex.get());
            }
        }
        

        for (auto i = 0; i < numFrame; i++)
        {
            FrameData& frameData = mFrameData[i];

            auto zeroDescrSetOfShadowPass = rs->createDescriptorSet(zeroLayout);
            rs->updateDescriptorSetBuffer(zeroDescrSetOfShadowPass, 4, &vertexDataHandle, 1);

            auto thirdDescrSetOfShadowPass = rs->createDescriptorSet(thirdLayout);
            rs->updateDescriptorSetBuffer(thirdDescrSetOfShadowPass, 0,
                &frameData.esmUniformBlockHandle, 1);
            frameData.zeroDescrSetOfShadowPass = zeroDescrSetOfShadowPass;
            frameData.thirdDescrSetOfShadowPass = thirdDescrSetOfShadowPass;

            //alpha

            if (alpha)
            {
                auto zeroDescrSetOfShadowPassAlpha =
                    rs->createDescriptorSet(zeroLayoutAlpha);
                rs->updateDescriptorSetBuffer(zeroDescrSetOfShadowPassAlpha, 4, &vertexDataHandle, 1);

                auto firstDescrSetOfShadowPassAlpha =
                    rs->createDescriptorSet(firstLayoutAlpha);

                rs->updateDescriptorSetBuffer(firstDescrSetOfShadowPassAlpha, 2,
                    &frameData.indirectDataBuffer, 1);

                auto thirdDescrSetOfShadowPassAlpha =
                    rs->createDescriptorSet(thirdLayoutAlpha);

                rs->updateDescriptorSetBuffer(thirdDescrSetOfShadowPassAlpha, 0,
                    &frameData.esmUniformBlockHandle, 1);
                rs->updateDescriptorSetSampler(zeroDescrSetOfShadowPassAlpha, 5, nearSamplerHandle);
                rs->updateDescriptorSetTexture(zeroDescrSetOfShadowPassAlpha, 6,
                    diffuseList.data(), diffuseList.size());

                frameData.zeroDescrSetOfShadowPassAlpha = zeroDescrSetOfShadowPassAlpha;
                frameData.firstDescrSetOfShadowPassAlpha = firstDescrSetOfShadowPassAlpha;
                frameData.thirdDescrSetOfShadowPassAlpha = thirdDescrSetOfShadowPassAlpha;
            }
            
        }

        RenderPassCallback shadowCallback = [=, this](RenderPassInfo& info) {
            auto* rs = Ogre::Root::getSingleton().getRenderSystem();
            RenderTargetBarrier rtBarriers[] =
            {
                {
                    esmShadowMap,
                    RESOURCE_STATE_SHADER_RESOURCE,
                    RESOURCE_STATE_DEPTH_WRITE
                }
            };
            rs->resourceBarrier(0, nullptr, 1, rtBarriers);
            auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
            info.renderTargetCount = 0;
            info.depthTarget.depthStencil = esmShadowMap;
            info.depthTarget.clearValue = { 1.0f, 0.0f };
            rs->pushGroupMarker("DrawEsmShadowMap");
            rs->beginRenderPass(info);
            auto target = VIEW_SHADOW;
            
            FrameData* frameData = this->getFrameData(frameIndex);
            Handle<HwDescriptorSet> tmp[4];
            tmp[0] = frameData->zeroDescrSetOfShadowPass;
            tmp[1] = Handle<HwDescriptorSet>();
            tmp[2] = Handle<HwDescriptorSet>();
            tmp[3] = frameData->thirdDescrSetOfShadowPass;
            rs->bindPipeline(meshDepthHandle, meshDepthPipelineHandle, &tmp[0], 4);
            rs->bindIndexBuffer(filteredIndexBuffer[target], 4);
            uint64_t indirectBufferByteOffset =
                GET_INDIRECT_DRAW_ELEM_INDEX(target, 0, 0) * sizeof(uint32_t);

            rs->drawIndexedIndirect(frameData->indirectDrawArgBuffer, indirectBufferByteOffset, 1, 32);
            if (1)
            {
                tmp[0] = frameData->zeroDescrSetOfShadowPassAlpha;
                tmp[1] = frameData->firstDescrSetOfShadowPassAlpha;
                tmp[2] = Handle<HwDescriptorSet>();
                tmp[3] = frameData->thirdDescrSetOfShadowPassAlpha;
                rs->bindPipeline(meshDepthAlphaHandle, meshDepthAlphaPipelineHandle, &tmp[0], 4);


                indirectBufferByteOffset =
                    GET_INDIRECT_DRAW_ELEM_INDEX(VIEW_SHADOW, 1, 0) * sizeof(uint32_t);

                rs->drawIndexedIndirect(frameData->indirectDrawArgBuffer, indirectBufferByteOffset, 1, 32);
            }
            

            rs->endRenderPass(info);

            rs->popGroupMarker();
            };

        UpdatePassCallback shadowUpdateCallback = [=, this](float delta) {
            };
        auto shadowPass = createUserDefineRenderPass(shadowCallback, shadowUpdateCallback);
        mRenderPipeline->addRenderPass(shadowPass);
    }
    //visibility buffer pass
    if(1)
    {
        ShaderInfo shaderInfo;
        shaderInfo.shaderName = "visibilityBuffer";
        auto vbBufferPassHandle = rs->createShaderProgram(shaderInfo, nullptr);
        shaderInfo.shaderName = "visibilityBufferAlpha";
        auto vbBufferPassAlphaHandle = rs->createShaderProgram(shaderInfo, nullptr);
        backend::RasterState rasterState{};
        rasterState.depthWrite = true;
        rasterState.depthTest = true;
        rasterState.depthFunc = backend::SamplerCompareFunc::GE;
        rasterState.colorWrite = true;
        rasterState.renderTargetCount = 1;
        rasterState.pixelFormat = Ogre::PixelFormat::PF_A8B8G8R8;
        auto vbBufferPasssPipelineHandle = rs->createPipeline(rasterState, vbBufferPassHandle);

        auto vbBufferPasssAlphaPipelineHandle = rs->createPipeline(rasterState, vbBufferPassAlphaHandle);
        auto width = mRenderWindow->getWidth();
        auto height = mRenderWindow->getHeight();

        visibilityBufferTarget = rs->createRenderTarget("visibilityBufferTarget",
            width, height, Ogre::PixelFormat::PF_A8B8G8R8, Ogre::TextureUsage::COLOR_ATTACHMENT);



        auto zeroLayout = rs->getDescriptorSetLayout(vbBufferPassHandle, 0);
        auto thirdLayout = rs->getDescriptorSetLayout(vbBufferPassHandle, 3);


        auto zeroLayoutAlpha = rs->getDescriptorSetLayout(vbBufferPassAlphaHandle, 0);
        auto firstLayoutAlpha = rs->getDescriptorSetLayout(vbBufferPassAlphaHandle, 1);
        auto thirdLayoutAlpha = rs->getDescriptorSetLayout(vbBufferPassAlphaHandle, 3);

        auto subMeshCount = mesh->getSubMeshCount();

        std::vector<OgreTexture*> diffuseList;
        diffuseList.reserve(subMeshCount);
        for (auto i = 0; i < subMeshCount; i++)
        {
            auto* subMesh = mesh->getSubMesh(i);
            auto& mat = subMesh->getMaterial();

            auto& unitList = mat->getAllTexureUnit();
            auto* diffuseTex = unitList[0]->getRaw();
            diffuseList.push_back(diffuseTex);
        }

        for (auto i = subMeshCount; i < 256; i++)
        {
            diffuseList.push_back(defaultTex.get());
        }

        for (auto i = 0; i < numFrame; i++)
        {
            FrameData& frameData = mFrameData[i];
            frameData.zeroDescrSetOfVbPass = rs->createDescriptorSet(zeroLayout);
            rs->updateDescriptorSetBuffer(frameData.zeroDescrSetOfVbPass, 3, &vertexDataHandle, 1);


            frameData.thirdDescrSetOfVbPass = rs->createDescriptorSet(thirdLayout);
            rs->updateDescriptorSetBuffer(
                frameData.thirdDescrSetOfVbPass,
                0, 
                &frameData.objectUniformBlockHandle, 1);
            ///
            auto zeroDescrSetOfVbPassAlpha  = rs->createDescriptorSet(zeroLayoutAlpha);
            auto firstDescrSetOfVbPassAlpha = rs->createDescriptorSet(firstLayoutAlpha);
            auto thirdDescrSetOfVbPassAlpha = rs->createDescriptorSet(thirdLayoutAlpha);
            frameData.zeroDescrSetOfVbPassAlpha = zeroDescrSetOfVbPassAlpha;
            frameData.firstDescrSetOfVbPassAlpha = firstDescrSetOfVbPassAlpha;
            frameData.thirdDescrSetOfVbPassAlpha = thirdDescrSetOfVbPassAlpha;

            rs->updateDescriptorSetBuffer(zeroDescrSetOfVbPassAlpha, 2, &vbConstantsBuffer, 1);
            rs->updateDescriptorSetBuffer(zeroDescrSetOfVbPassAlpha, 3, &vertexDataHandle, 1);
            rs->updateDescriptorSetSampler(zeroDescrSetOfVbPassAlpha, 5, nearSamplerHandle);
            rs->updateDescriptorSetTexture(zeroDescrSetOfVbPassAlpha, 6,
                diffuseList.data(), diffuseList.size());

            rs->updateDescriptorSetBuffer(firstDescrSetOfVbPassAlpha, 2,
                &frameData.indirectDataBuffer, 1);
            rs->updateDescriptorSetBuffer(firstDescrSetOfVbPassAlpha, 1,
                &frameData.perFrameConstantsBuffer, 1);

            rs->updateDescriptorSetBuffer(thirdDescrSetOfVbPassAlpha,
                0, &frameData.objectUniformBlockHandle, 1);
        }
        
        auto winDepth = mRenderWindow->getDepthTarget();
        RenderPassCallback visibilityBufferCallback = [=, this](RenderPassInfo& info) {
            auto* rs = Ogre::Root::getSingleton().getRenderSystem();
            auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
            info.renderTargetCount = 1;
            info.renderTargets[0].renderTarget =  visibilityBufferTarget;
            info.renderTargets[0].clearColour = { 1.0f, 1.0f, 1.0f, 1.000000000f };
            info.depthTarget.depthStencil = winDepth;
            info.depthTarget.clearValue = { 0.0f, 0.0f };

            rs->pushGroupMarker("visibilityBuffer");
            rs->beginRenderPass(info);
            rs->bindIndexBuffer(filteredIndexBuffer[VIEW_CAMERA], 4);
            FrameData* frameData = this->getFrameData(frameIndex);

            auto nullSet = Handle<HwDescriptorSet>();
            Handle<HwDescriptorSet> tmp[4];
            tmp[0] = frameData->zeroDescrSetOfVbPass;
            tmp[1] = nullSet;
            tmp[2] = nullSet;
            tmp[3] = frameData->thirdDescrSetOfVbPass;
            rs->bindPipeline(vbBufferPassHandle, vbBufferPasssPipelineHandle, &tmp[0], 4);
            uint64_t indirectBufferByteOffset =
                GET_INDIRECT_DRAW_ELEM_INDEX(VIEW_CAMERA, 0, 0) * sizeof(uint32_t);
            
            
            rs->drawIndexedIndirect(frameData->indirectDrawArgBuffer, indirectBufferByteOffset, 1, 32);

            tmp[0] = frameData->zeroDescrSetOfVbPassAlpha;
            tmp[1] = frameData->firstDescrSetOfVbPassAlpha;
            tmp[2] = nullSet;
            tmp[3] = frameData->thirdDescrSetOfVbPassAlpha;
            rs->bindPipeline(vbBufferPassAlphaHandle, vbBufferPasssAlphaPipelineHandle, &tmp[0], 4);
            indirectBufferByteOffset =
                GET_INDIRECT_DRAW_ELEM_INDEX(VIEW_CAMERA, 1, 0) * sizeof(uint32_t);
            rs->drawIndexedIndirect(frameData->indirectDrawArgBuffer, indirectBufferByteOffset, 1, 32);
            //rs->drawIndexed(1470735, 1, 5587923, 0, 0);
            rs->endRenderPass(info);
            rs->popGroupMarker();
            };

        UpdateCallback vbUpdateCallback = [=, this](float delta)
            {
            };
        auto vbPass = createUserDefineRenderPass(visibilityBufferCallback, vbUpdateCallback);
        mRenderPipeline->addRenderPass(vbPass);
    }
    

    //visibility shade pass
    if(1)
    {
        ShaderInfo shaderInfo;
        shaderInfo.shaderName = "visibilityBufferShade";
        auto programHandle = rs->createShaderProgram(shaderInfo, nullptr);
        backend::RasterState rasterState{};
        rasterState.depthWrite = false;
        rasterState.depthTest = false;
        rasterState.depthFunc = SamplerCompareFunc::A;
        rasterState.colorWrite = true;
        rasterState.renderTargetCount = 1;
        rasterState.pixelFormat = Ogre::PixelFormat::PF_A8R8G8B8_SRGB;
        auto pipelineHandle = rs->createPipeline(rasterState, programHandle);
        auto zeroLayout = rs->getDescriptorSetLayout(programHandle, 0);
        auto firstLayout = rs->getDescriptorSetLayout(programHandle, 1);

        backend::SamplerParams samplerParams;

        samplerParams.filterMag = backend::SamplerFilterType::LINEAR;
        samplerParams.filterMin = backend::SamplerFilterType::LINEAR;
        samplerParams.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR;
        samplerParams.wrapS = backend::SamplerWrapMode::REPEAT;
        samplerParams.wrapT = backend::SamplerWrapMode::REPEAT;
        samplerParams.wrapR = backend::SamplerWrapMode::REPEAT;
        samplerParams.compareMode = backend::SamplerCompareMode::NONE;
        samplerParams.compareFunc = backend::SamplerCompareFunc::N;
        samplerParams.anisotropyLog2 = 3;
        samplerParams.padding0 = 0;
        samplerParams.padding1 = 0;
        samplerParams.padding2 = 0;
        auto textureSamplerHandle = rs->createTextureSampler(samplerParams);

        samplerParams.filterMag = backend::SamplerFilterType::LINEAR;
        samplerParams.filterMin = backend::SamplerFilterType::LINEAR;
        samplerParams.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_LINEAR;
        samplerParams.compareMode = backend::SamplerCompareMode::NONE;
        samplerParams.compareFunc = backend::SamplerCompareFunc::LE;
        samplerParams.wrapS = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.wrapT = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.wrapR = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.anisotropyLog2 = 0;

        auto clampMiplessLinearSamplerHandle = rs->createTextureSampler(samplerParams);
        samplerParams.filterMag = backend::SamplerFilterType::NEAREST;
        samplerParams.filterMin = backend::SamplerFilterType::NEAREST;
        samplerParams.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST;
        samplerParams.compareMode = backend::SamplerCompareMode::NONE;
        samplerParams.compareFunc = backend::SamplerCompareFunc::LE;
        samplerParams.wrapS = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.wrapT = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.wrapR = backend::SamplerWrapMode::CLAMP_TO_EDGE;

        auto clampMiplessNearSamplerHandle = rs->createTextureSampler(samplerParams);
        samplerParams.filterMag = backend::SamplerFilterType::NEAREST;
        samplerParams.filterMin = backend::SamplerFilterType::NEAREST;
        samplerParams.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST;
        samplerParams.wrapS = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.wrapT = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.wrapR = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.compareMode = backend::SamplerCompareMode::NONE;
        auto clampBorderNearSamplerHandle = rs->createTextureSampler(samplerParams);

        samplerParams.filterMag = backend::SamplerFilterType::NEAREST;
        samplerParams.filterMin = backend::SamplerFilterType::NEAREST;
        samplerParams.mipMapMode = backend::SamplerMipMapMode::MIPMAP_MODE_NEAREST;
        samplerParams.wrapS = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.wrapT = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.wrapR = backend::SamplerWrapMode::CLAMP_TO_EDGE;
        samplerParams.compareMode = backend::SamplerCompareMode::COMPARE_TO_TEXTURE;
        samplerParams.compareFunc = backend::SamplerCompareFunc::LE;
        auto shadowCmpSamplerHandle = rs->createTextureSampler(samplerParams);

        
        for (auto frame = 0; frame < numFrame; frame++)
        {
            FrameData& frameData = mFrameData[frame];
            frameData.zeroDescrSetOfVbShadePass =
                rs->createDescriptorSet(zeroLayout);
            frameData.firstDescrSetOfVbShadePass =
                rs->createDescriptorSet(firstLayout);
            auto& zeroSet = frameData.zeroDescrSetOfVbShadePass;
            auto& firstSet = frameData.firstDescrSetOfVbShadePass;

            rs->updateDescriptorSetBuffer(zeroSet, 6, &meshConstantsBuffer, 1);
            rs->updateDescriptorSetSampler(zeroSet, 7, textureSamplerHandle);
            rs->updateDescriptorSetSampler(zeroSet, 8, clampMiplessLinearSamplerHandle);
            rs->updateDescriptorSetSampler(zeroSet, 9, clampMiplessNearSamplerHandle);
            rs->updateDescriptorSetSampler(zeroSet, 10, clampBorderNearSamplerHandle);
            rs->updateDescriptorSetSampler(zeroSet, 11, shadowCmpSamplerHandle);

            Ogre::OgreTexture* tex = visibilityBufferTarget->getTarget();
            rs->updateDescriptorSetTexture(zeroSet, 18, &tex, 1);

            auto subMeshCount = mesh->getSubMeshCount();

            std::vector<OgreTexture*> diffuseList;
            std::vector<OgreTexture*> specularList;
            std::vector<OgreTexture*> normalList;
            diffuseList.reserve(256);
            specularList.reserve(256);
            normalList.reserve(256);
            for (auto i = 0; i < subMeshCount; i++)
            {
                auto* subMesh = mesh->getSubMesh(i);
                auto& mat = subMesh->getMaterial();

                auto& unitList = mat->getAllTexureUnit();
                auto* diffuseTex = unitList[0]->getRaw();
                auto* specularTex = unitList[1]->getRaw();
                auto* normalTex = unitList[2]->getRaw();
                diffuseList.push_back(diffuseTex);
                specularList.push_back(specularTex);
                normalList.push_back(normalTex);
            }

            for (auto i = subMeshCount; i < 256; i++)
            {
                diffuseList.push_back(defaultTex.get());
                specularList.push_back(defaultTex.get());
                normalList.push_back(defaultTex.get());
            }
            OgreTexture* esmShadow = esmShadowMap->getTarget();
            rs->updateDescriptorSetTexture(zeroSet, 24, &esmShadow, 1);
            rs->updateDescriptorSetTexture(zeroSet, 25, diffuseList.data(), diffuseList.size());
            rs->updateDescriptorSetTexture(zeroSet, 27, specularList.data(), specularList.size());
            rs->updateDescriptorSetTexture(zeroSet, 26, normalList.data(), normalList.size());

            rs->updateDescriptorSetBuffer(zeroSet, 35, &vertexDataHandle, 1);

            rs->updateDescriptorSetBuffer(zeroSet, 2, &vbConstantsBuffer, 1);

            rs->updateDescriptorSetBuffer(firstSet, 4, &filteredIndexBuffer[0], 1);
            rs->updateDescriptorSetBuffer(firstSet, 5, &frameData.indirectDataBuffer, 1);
            rs->updateDescriptorSetBuffer(firstSet, 13, &frameData.objectUniformBlockHandle, 1);
            rs->updateDescriptorSetBuffer(firstSet, 14, &frameData.cameraUniformHandle, 1);
            rs->updateDescriptorSetBuffer(firstSet, 15, &frameData.ligthUniformHandle, 1);
            rs->updateDescriptorSetBuffer(firstSet, 16, &renderSettingsUniformHandle, 1);

            rs->updateDescriptorSetBuffer(firstSet, 17, &esmInputConstantsHandle, 1);
            rs->updateDescriptorSetBuffer(firstSet, 19, &sssEnabledHandle, 1);
        }
        shadePassTarget = rs->createRenderTarget("shadePassTarget",
            ogreConfig.width, ogreConfig.height, Ogre::PixelFormat::PF_A8R8G8B8_SRGB,
            Ogre::TextureUsage::COLOR_ATTACHMENT);
        auto winDepth = mRenderWindow->getDepthTarget();
        RenderPassCallback shadeCallback = [=, this](RenderPassInfo& info) {
            RenderTargetBarrier rtBarriers[] = 
            { 
                {
                    esmShadowMap,
                    RESOURCE_STATE_DEPTH_WRITE,
                    RESOURCE_STATE_SHADER_RESOURCE
                },
                { 
                    visibilityBufferTarget,
                    RESOURCE_STATE_RENDER_TARGET, 
                    RESOURCE_STATE_SHADER_RESOURCE 
                }
            };
            rs->resourceBarrier(0, nullptr, 2, rtBarriers);
            info.renderTargetCount = 1;
            info.renderTargets[0].renderTarget = shadePassTarget;
            info.renderTargets[0].clearColour = { 0.0f, 0.0f, 0.0f, 0.0f };
            info.depthTarget.depthStencil = nullptr;
            info.depthTarget.clearValue = { 0.0f, 0.0f };
            auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
            rs->pushGroupMarker("shadePass");
            rs->beginRenderPass(info);
            auto* frameData = getFrameData(frameIndex);
            Handle<HwDescriptorSet> tmp[2];
            tmp[0] = frameData->zeroDescrSetOfVbShadePass;
            tmp[1] = frameData->firstDescrSetOfVbShadePass;
            rs->bindPipeline(programHandle, pipelineHandle, tmp, 2);
            rs->draw(3, 0);
            rs->endRenderPass(info);
            rs->popGroupMarker();
            };
        UpdatePassCallback updateCallback = [](float delta) {
            };
        auto shadePass = createUserDefineRenderPass(shadeCallback, updateCallback);
        mRenderPipeline->addRenderPass(shadePass);
    }

    //present pass
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
        auto winDepth = mRenderWindow->getDepthTarget();
        ShaderInfo shaderInfo;
        shaderInfo.shaderName = "presentShade";
        auto presentHandle = rs->createShaderProgram(shaderInfo, nullptr);
        auto zeroLayout = rs->getDescriptorSetLayout(presentHandle, 0);
        for (auto i = 0; i < numFrame; i++)
        {
            FrameData& frameData = mFrameData[i];
            auto zeroSet = rs->createDescriptorSet(zeroLayout);
            frameData.zeroDescrSetOfPresentPass = zeroSet;
            auto* tex = shadePassTarget->getTarget();
            rs->updateDescriptorSetTexture(zeroSet, 0, &tex, 1, true);
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
            RenderTargetBarrier rtBarriers[] =
            {
                {
                    shadePassTarget,
                    RESOURCE_STATE_RENDER_TARGET,
                    RESOURCE_STATE_SHADER_RESOURCE
                }
            };
            rs->resourceBarrier(0, nullptr, 1, rtBarriers);
            info.renderTargetCount = 1;
            info.renderTargets[0].renderTarget = mRenderWindow->getColorTarget();
            info.renderTargets[0].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
            info.depthTarget.depthStencil = nullptr;
            info.depthTarget.clearValue = { 0.0f, 0.0f };
            auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
            rs->pushGroupMarker("presentPass");
            rs->beginRenderPass(info);
            auto* frameData = getFrameData(frameIndex);
            rs->bindPipeline(presentHandle, pipelineHandle,
                &frameData->zeroDescrSetOfPresentPass, 1);
            rs->draw(3, 0);
            rs->endRenderPass(info);
            rs->popGroupMarker();
            rtBarriers[0] =
            {
                mRenderWindow->getColorTarget(),
                RESOURCE_STATE_RENDER_TARGET,
                RESOURCE_STATE_PRESENT
            };
            rs->resourceBarrier(0, nullptr, 1, rtBarriers);
            };
        UpdatePassCallback updateCallback = [](float delta) {
            };
        auto presentPass = createUserDefineRenderPass(presentCallback, updateCallback);
        mRenderPipeline->addRenderPass(presentPass);
    }
    Ogre::Vector3 camPos2(120.f + SAN_MIGUEL_OFFSETX, 98.f, 14.f);

    mGameCamera->setMoveSpeed(50.0f);

    Ogre::Vector3 lookAt = camPos2 + Ogre::Vector3(-1.0f, 0.1f, 0.0f);
    mGameCamera->lookAt(camPos2, lookAt);
    float aspectInverse = ogreConfig.height / (float)ogreConfig.width;

    Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrixLHReverseZ(
        Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 1000.f);
    mGameCamera->getCamera()->updateProjectMatrix(m);
}

void ShadowMap::execute(RenderSystem* rs)
{
    auto& ogreConfig = ::Root::getSingleton().getEngineConfig();
    RenderPassInfo info;
    auto cam = mGameCamera->getCamera();
    auto sceneManager = mSceneManager;
    info.renderTargetCount = 1;
    info.renderTargets[0].renderTarget = mRenderWindow->getColorTarget();
    info.depthTarget.depthStencil = mRenderWindow->getDepthTarget();;
    info.renderTargets[0].clearColour = { 0.678431f, 0.847058f, 0.901960f, 1.000000000f };
    info.depthTarget.clearValue = { 0.0f, 0.0f };
    info.cam = cam;
    static EngineRenderList engineRenerList;
    sceneManager->getSceneRenderList(cam, engineRenerList, false);
    auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
    auto& frameData = mFrameData[frameIndex];
    for (auto r : engineRenerList.mOpaqueList)
    {
        Ogre::Material* mat = r->getMaterial().get();

        if (!mat->isLoaded())
        {
            mat->load(nullptr);
            r->createFrameResource();
            for (auto i = 0; i < ogreConfig.swapBufferCount; i++)
            {
                FrameResourceInfo* resourceInfo = r->getFrameResourceInfo(i);
                auto frameHandle = frameData.frameBufferObject;
                rs->updateDescriptorSetBuffer(resourceInfo->zeroSet, 1, &frameHandle, 1);
                rs->updateDescriptorSetBuffer(resourceInfo->zeroShadowSet, 1, &frameHandle, 1);
            }
            r->updateModelMatrix(meshInfoStruct.mWorldMat);
        }
        r->updateFrameResource(frameIndex);
    }
    rs->beginRenderPass(info);
    for (auto r : engineRenerList.mOpaqueList)
    {
        Ogre::Material* mat = r->getMaterial().get();
        auto frameIndex = Ogre::Root::getSingleton().getCurrentFrameIndex();
        FrameResourceInfo* resourceInfo = r->getFrameResourceInfo(frameIndex);
        Handle<HwDescriptorSet> descriptorSet[2];
        descriptorSet[0] = resourceInfo->zeroSet;
        descriptorSet[1] = resourceInfo->firstSet;

        auto programHandle = mat->getProgram();
        auto piplineHandle = mat->getPipeline();
        rs->bindPipeline(programHandle, piplineHandle, descriptorSet, 2);


        VertexData* vertexData = r->getVertexData();
        IndexData* indexData = r->getIndexData();
        vertexData->bind(nullptr);
        indexData->bind();
        IndexDataView* view = r->getIndexView();
        rs->drawIndexed(view->mIndexCount, 1,
            view->mIndexLocation, view->mBaseVertexLocation, 0);
    }
    rs->endRenderPass(info);

    RenderTargetBarrier rtBarriers[] =
    {
        {
            mRenderWindow->getColorTarget(),
            RESOURCE_STATE_RENDER_TARGET,
            RESOURCE_STATE_PRESENT
        }
    };
    rs->resourceBarrier(0, nullptr, 1, rtBarriers);
}

void ShadowMap::base3()
{
    auto& ogreConfig = Ogre::Root::getSingleton().getEngineConfig();
    auto rootNode = mSceneManager->getRoot();
    auto root = mSceneManager->getRoot();

    std::string meshname = "SanMiguel.bin";
    std::shared_ptr<Mesh> mesh = loadSanMiguel(meshname);

    Entity* sanMiguel = mSceneManager->createEntity(meshname, meshname);
    SceneNode* sanMiguelNode = rootNode->createChildSceneNode(meshname);
    sanMiguelNode->attachObject(sanMiguel);
    RenderPassInput renderInput;
    renderInput.cam = mGameCamera->getCamera();
    renderInput.color = mRenderWindow->getColorTarget();
    renderInput.depth = mRenderWindow->getDepthTarget();
    renderInput.sceneMgr = mSceneManager;
    auto mainPass = createStandardRenderPass(renderInput);
    mRenderPipeline->addRenderPass(mainPass);

    mGameCamera->setMoveSpeed(2.0f);
    auto camPos = Ogre::Vector3(0, 5.0f, 12.0f);
    auto lookAtPos = camPos + Ogre::Vector3(0, 1, -1);
    mGameCamera->lookAt(camPos, lookAtPos);
    float aspectInverse = ogreConfig.height / (float)ogreConfig.width;

    Ogre::Matrix4 m = Ogre::Math::makePerspectiveMatrixLHReverseZ(
        Ogre::Math::PI / 2.0f, aspectInverse, 0.1, 10000.f);
    mGameCamera->getCamera()->updateProjectMatrix(m);
}
