#include "OgreHeader.h"
#include "RayTracingApp.h"
#include "myutils.h"
#include "OgreResourceManager.h"
#include "OgreMaterialManager.h"
#include "OgreCamera.h"

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

}

void RayTracingApp::update(float delta)
{

}

bool RayTracingApp::appInit()
{
	

	return true;
}

void RayTracingApp::appUpdate(float delta)
{
	
}

EngineType RayTracingApp::getEngineType()
{
	return EngineType_Vulkan;
}

bool RayTracingApp::isUseMyGUI()
{
	return false;
}