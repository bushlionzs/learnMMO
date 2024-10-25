#pragma once
#include "ApplicationBase.h"
#include "forgeCommon.h"
#include "pass.h"
#include "engine_struct.h"
class GameCamera;
class RenderSystem;
class Ogre::RenderWindow;
class Ogre::SceneManager;

#define NUM_CULLING_VIEWPORTS 5
#define NUM_GEOMETRY_SETS 2

struct FrameData
{
    //在compute shader中，保存索引对应的batchMeshIndex
	Handle<HwBufferObject> indirectDataBuffer;
	Handle<HwBufferObject> indirectDrawArgBuffer;

	Handle<HwBufferObject> perFrameConstantsBuffer;
	
	Handle<HwBufferObject> objectUniformBlockHandle;
	Handle<HwBufferObject> esmUniformBlockHandle;

	Handle<HwBufferObject> cameraUniformHandle;
	Handle<HwBufferObject> ligthUniformHandle;

	//clear buffer pass
	Handle<HwDescriptorSet> clearBufferDescrSet;
	//for filter angles pass
	Handle <HwDescriptorSet> zeroDescSetOfFilter;
	Handle <HwDescriptorSet> firstDescSetOfFilter;

	//for shadow pass
	Handle <HwDescriptorSet> zeroDescrSetOfShadowPass;
	Handle <HwDescriptorSet> thirdDescrSetOfShadowPass;

	Handle <HwDescriptorSet> zeroDescrSetOfShadowPassAlpha;
	Handle <HwDescriptorSet> firstDescrSetOfShadowPassAlpha;
	Handle <HwDescriptorSet> thirdDescrSetOfShadowPassAlpha;
	//for visibility pass

	Handle <HwDescriptorSet> zeroDescrSetOfVbPass;
	Handle <HwDescriptorSet> thirdDescrSetOfVbPass;

	Handle <HwDescriptorSet> zeroDescrSetOfVbPassAlpha;
	Handle <HwDescriptorSet> firstDescrSetOfVbPassAlpha;
	Handle <HwDescriptorSet> thirdDescrSetOfVbPassAlpha;

	//for visibility shade pass
	Handle <HwDescriptorSet> zeroDescrSetOfVbShadePass;
	Handle <HwDescriptorSet> firstDescrSetOfVbShadePass;
	//present shader
	Handle <HwDescriptorSet> zeroDescrSetOfPresentPass;
	//
	Handle<HwBufferObject> frameBufferObject;

	bool update = false;
};

class ShadowMap
{
public:
	ShadowMap();
	~ShadowMap();
	void setup(
		RenderPipeline* renderPipeline,
		RenderSystem* renderSystem,
		Ogre::RenderWindow* renderWindow,
		Ogre::SceneManager* sceneManager,
		GameCamera* gameCamera);
	void update(float delta);

	FrameGraphId<FrameGraphTexture> fgPass(FrameGraph& fg);

	FrameData* getFrameData(uint32_t frameIndex)
	{
		return &mFrameData[frameIndex];
	}
private:
	void base1();
	void base2();
	void base3();
	void updateFrameData(uint32_t i);
	void execute(RenderSystem* rs);
private:
	AnimationState* mAnimationState = nullptr;

	SceneNode* mLightNode = nullptr;

	Ogre::Light* light = nullptr;

	Ogre::SceneManager* mSceneManager;
	GameCamera* mGameCamera;
	RenderSystem* mRenderSystem;
	RenderWindow* mRenderWindow;
	RenderPipeline* mRenderPipeline;

	std::vector<FrameData> mFrameData;

	Handle<HwBufferObject> filteredIndexBuffer[NUM_CULLING_VIEWPORTS];

	Handle<HwBufferObject> vbConstantsBuffer;
	Handle<HwBufferObject> filterDispatchGroupDataBuffer;
	Handle<HwBufferObject> meshConstantsBuffer;
	Handle<HwBufferObject> renderSettingsUniformHandle;
	Handle<HwBufferObject> esmInputConstantsHandle;
	Handle<HwBufferObject> sssEnabledHandle;
	VBConstants vbConstants[NUM_GEOMETRY_SETS];

	Ogre::RenderTarget* esmShadowMap;
	Ogre::RenderTarget* visibilityBufferTarget;
	Ogre::RenderTarget* shadePassTarget;

	PerFrameVBConstants    perFrameVBConstants;
	MeshInfoUniformBlock   meshUniformBlock;
	MeshInfoUniformBlock   esmMeshUniformBlock;
	CameraUniform          cameraUniform;
	LightUniformBlock      lightUniformBlock;
	RenderSettingsUniformData renderSetting;
	ESMInputConstants   esmConstants;
	uint32_t          sssEnabled = 0;

	MeshInfoStruct    meshInfoStruct;

	LightCpuSettings  lightCpuSettings;

	FrameConstantBuffer mFrameConstantBuffer;

};