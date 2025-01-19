#include "OgreHeader.h"
#include "SDFGI.h"
#include "platform_log.h"
#include "ManualApplication.h"
int main()
{
	SDFGIApp instance;

	AppInfo info;
	info.enableRayTracing = true;
	ManualApplication app;
	info.engineType = EngineType_Vulkan;
	//info.engineType = EngineType_Dx12;
	info.setup = [&instance, &app](RenderSystem* rs, Ogre::RenderWindow* win, Ogre::SceneManager* sceneManager, GameCamera* gameCamera) {
		instance.setup(&app, rs, win, sceneManager, gameCamera);
		};

	info.update = [&instance](float delta) {
		instance.update(delta);
		};
	info.cleanup = [&instance]() {
		};

	app.run(info);


	return 0;
}

