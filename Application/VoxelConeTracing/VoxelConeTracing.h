#pragma once
#include <engine_struct.h>
#include "pass.h"
#include "game_camera.h"
#include "DriverBase.h"
#include "vctBase.h"
#include "OgreCamera.h"

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
	void sceneGeometryPass();
	void shadowPass();
	void initScene();
	void addEntry(
		const std::string& entryName,
		const std::string& meshName,
		const Ogre::Vector3& position,
		const Ogre::Matrix4& rotate,
		const Ogre::Vector4& color
	);
private:
	static const uint32_t sceneGeometryPassBit = 3;
	static const uint32_t shadowPassBit = 5;
	SceneManager* mSceneManager = nullptr;
	GameCamera* mGameCamera = nullptr;
	RenderSystem* mRenderSystem = nullptr;
	RenderWindow* mRenderWindow = nullptr;
	RenderPipeline* mRenderPipeline;
	FrameConstantBuffer mFrameConstantBuffer;
	
	std::vector<FrameData> mFrameData;

	VoxelizationContext mVoxelizationContext;
};