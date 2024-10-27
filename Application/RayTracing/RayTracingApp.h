#pragma once

#include <engine_struct.h>
#include "pass.h"
#include "game_camera.h"
#include "DriverBase.h"

class RayTracingApp
{
public:
	RayTracingApp();
	~RayTracingApp();
	void setup(
		RenderPipeline* renderPipeline,
		RenderSystem* renderSystem,
		Ogre::RenderWindow* renderWindow,
		Ogre::SceneManager* sceneManager,
		GameCamera* gameCamera);
	void update(float delta);
	virtual bool appInit();
	virtual void appUpdate(float delta);
	virtual EngineType getEngineType();
	virtual bool isUseMyGUI();
};