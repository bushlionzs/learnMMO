#pragma once
#include <engine_struct.h>
#include "pass.h"
#include "game_camera.h"
#include "DriverBase.h"
#include "vctBase.h"
#include "OgreCamera.h"

#define VCT_SCENE_VOLUME_SIZE 256
#define VCT_MIPS 6

struct VctFrameResourceInfo
{
	Handle<HwDescriptorSet> zeroSet;
	Handle<HwDescriptorSet> firstSet;
	Handle<HwDescriptorSet> zeroShadowSet;
	Handle<HwBufferObject>  modelObjectHandle;
	Handle<HwBufferObject>  matObjectHandle;
	Handle<HwBufferObject>  skinObjectHandle;
	bool update;
	Handle<HwDescriptorSet> zeroSetOfVoxelization;	
};

struct VctFrameData
{
	Handle <HwDescriptorSet> mipmapPrepareZeroSet;
	Handle <HwDescriptorSet> mipmapResultZeroSet[VCT_MIPS];
	Handle<HwBufferObject>   mipmapBlockHandle[VCT_MIPS];
	Handle <HwDescriptorSet> tracingConeZeroSet;
};

struct VoxelizationBlock
{
	Ogre::Matrix4 worldVoxelCube;
	Ogre::Matrix4 viewProjection;
	Ogre::Matrix4 shadowViewProjection;
	float worldVoxelScale;
};

struct VCTMainBlock
{
	Ogre::Vector4 CameraPos;
	Ogre::Vector2 UpsampleRatio;
	float IndirectDiffuseStrength;
	float IndirectSpecularStrength;
	float MaxConeTraceDistance;
	float AOFalloff;
	float SamplingFactor;
	float VoxelSampleOffset;
};

struct MipmapBlock
{
	int MipDimension;
	int MipLevel;
};

class VoxelConeTracingApp
{
public:
	VoxelConeTracingApp();
	~VoxelConeTracingApp();

	void setup(
		RenderPipeline* renderPipeline,
		RenderSystem* renderSystem,
		Ogre::RenderWindow* renderWindow,
		Ogre::SceneManager* sceneManager,
		GameCamera* gameCamera);
	void update(float delta);
private:
	void sceneGeometryPass();
	void shadowPass();
	void voxelizationPass();
	void computePass();
	void initScene();
	void addEntry(
		const std::string& entryName,
		const std::string& meshName,
		const Ogre::Vector3& position,
		const Ogre::Matrix4& rotate,
		const Ogre::Vector4& color
	);
	void initFrameResource(uint32_t frameIndex, Renderable* r);
private:
	static const uint32_t sceneGeometryPassBit = 3;
	static const uint32_t shadowPassBit = 5;
	static const uint32_t voxelizationPassBit = 7;
	SceneManager* mSceneManager = nullptr;
	GameCamera* mGameCamera = nullptr;
	RenderSystem* mRenderSystem = nullptr;
	RenderWindow* mRenderWindow = nullptr;
	RenderPipeline* mRenderPipeline;
	FrameConstantBuffer mFrameConstantBuffer;
	
	VoxelizationContext mVoxelizationContext;

	Handle<HwProgram> mSceneGeometryProgramHandle;
	Handle<HwPipeline> mSceneGeometryPipelineHandle;

	Handle<HwProgram> mShadowProgramHandle;
	Handle<HwPipeline> mShadowPipelineHandle;

	Handle<HwProgram> mVoxellizationProgramHandle;
	Handle<HwPipeline> mVoxellizationPipelineHandle;

	Handle<HwComputeProgram> mMipmapPrepareHandle;
	Handle<HwComputeProgram> mMipmapMainHandle;
	Handle<HwComputeProgram> mTracingConeHandle;
	Handle<HwBufferObject>  MipmapBlockHandle;
	Handle<HwBufferObject> tracingVoxelizationBlockHandle;
	Handle<HwBufferObject> tracingMainBlockHandle;
	std::vector<VctFrameData> mComputeFrameData;
	
	Ogre::Vector3 mLightPos;
	Ogre::Vector3 mLightTarget;

	Ogre::Matrix4 mLightView;
	Ogre::Matrix4 mLightProject;

	VoxelizationBlock mVoxelizationBlock;

	VCTMainBlock mVctMainBlock;
};