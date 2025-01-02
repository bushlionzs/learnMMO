#pragma once

#include <engine_struct.h>
#include "pass.h"
#include "game_camera.h"
#include "DriverBase.h"

class BasicApplication
{
public:
	struct FrameData
	{
		Handle<HwDescriptorSet> zeroSet;
		Handle<HwBufferObject> passUniformBuffer;
	};
	BasicApplication();
	~BasicApplication();

	void setup(
		RenderPipeline* renderPipeline,
		RenderSystem* renderSystem,
		Ogre::RenderWindow* renderWindow,
		Ogre::SceneManager* sceneManager,
		GameCamera* gameCamera);
	void update(float delta);

	void addCustomDirectory();
	void updateFrameData(ICamera* camera, FrameConstantBuffer& frameBuffer);
private:
	void base1();
	void base2();
	void base3();
	void base4();
	void base5();
	
	void base6();
private:
	AnimationState* mAnimationState = nullptr;
	std::vector<FrameData> mFrameData;
	SceneManager* mSceneManager;
	GameCamera* mGameCamera;
	RenderSystem* mRenderSystem;
	RenderWindow* mRenderWindow;
	RenderPipeline* mRenderPipeline;
};