#include "OgreHeader.h"
#include "PbrMaterial.h"
#include "platform_log.h"
#include "ManualApplication.h"
int main()
{
	platform_log_init();
	PbrMaterial instance;

	AppInfo info;
	info.useCEGUI = true;
	info.useSRGB = false;
	ManualApplication app; 

	info.setup = [&instance, &app, &info](RenderSystem* rs, Ogre::RenderWindow* win, Ogre::SceneManager* sceneManager, GameCamera* gameCamera) {
		instance.setup(&app, rs, win, sceneManager, gameCamera);
		if (info.useCEGUI)
		{
			app.addUIPass();
		}
		};

	info.update = [&instance](float delta) {
		instance.update(delta);
		};
	info.cleanup = [&instance]() {
		};

	app.run(info);

	return 0;
}

