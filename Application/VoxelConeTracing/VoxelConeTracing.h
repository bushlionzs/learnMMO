#pragma once
#include <engine_struct.h>
#include "pass.h"
#include "game_camera.h"
#include "DriverBase.h"

struct FrameData
{
	Handle <HwDescriptorSet> zeroDescSet;
	Handle <HwDescriptorSet> firstDescSet;
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
	void renderObject(
		Ogre::Renderable* r, 
		Handle<HwProgram> programHandle,
		Handle<HwPipeline> pipelineHandle);
	void updateObject(Ogre::Renderable* r, Handle<HwPipeline> pipelineHandle);
private:
	SceneManager* mSceneManager = nullptr;
	GameCamera* mGameCamera = nullptr;
	RenderSystem* mRenderSystem = nullptr;
	RenderWindow* mRenderWindow = nullptr;
	FrameConstantBuffer mFrameConstantBuffer;
	

	std::vector<FrameData> mFrameData;

	
	VoxelizationContext mVoxelizationContext;
	VoxelPassInfo mVoxelPassInfo;
};