#include "OgreHeader.h"
#include "ShadowMap.h"
#include "platform_log.h"
#include "SimpleApp.h"
#include "ManualApplication.h"

int main()
{
	platform_log_init();
	ShadowMap instance;
	
    AppInfo info;
	ManualApplication app;
	info.setup = [&instance, &app](RenderSystem* rs, Ogre::RenderWindow* win, Ogre::SceneManager* sceneManager, GameCamera* gameCamera) {
		instance.setup(&app, rs, win, sceneManager, gameCamera);
		};
	
	info.update = [&instance](float delta) {
		instance.update(delta);
	};
	info.cleanup = [&instance]() {
		};
	bool manual = true;
	if (manual)
	{
		
		app.run(info);
	}
	else
	{
		info.fgpass = [&instance](FrameGraph& fg) {
			return instance.fgPass(fg);
			};
		SimpleApp app;
		app.run(info);
	}
	
	return 0;
}

