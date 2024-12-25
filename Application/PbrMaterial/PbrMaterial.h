#pragma once
#include "OgreAnimationState.h"
#include <engine_struct.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/FontManager.h>
#include <CEGUI/SchemeManager.h>
#include <CEGUI/System.h>
//#include <CEGUI/ImagesetManager.h>
#include <CEGUI/Window.h>
#include <CEGUI/widgets/PushButton.h>
#include <CEGUI/ImageManager.h>
#include <DefaultWindow.h>
#include <FrameWindow.h>
#include <CEGUIManager.h>
#include "pass.h"

class Role;
class PbrMaterial : public InputListener
{
public:
	PbrMaterial();
	~PbrMaterial();

	void setup(
		RenderPipeline* renderPipeline,
		RenderSystem* renderSystem,
		Ogre::RenderWindow* renderWindow,
		Ogre::SceneManager* sceneManager,
		GameCamera* gameCamera);
	void update(float delta);
	void uiInit();
	void updateRenderMode(uint32_t mode);
	void updateMaterialType(uint32_t type);
	void updateLightDirection(const Ogre::Vector3& dir);
	void example1(
		RenderPipeline* renderPipeline,
		RenderSystem* renderSystem,
		Ogre::RenderWindow* renderWindow,
		Ogre::SceneManager* sceneManager,
		GameCamera* gameCamera);
	void example2(
		RenderPipeline* renderPipeline,
		RenderSystem* renderSystem,
		Ogre::RenderWindow* renderWindow,
		Ogre::SceneManager* sceneManager,
		GameCamera* gameCamera);
private:
	CEGUI::GUIContext* mGUIContext = nullptr;
	CEGUI::Window* mRoot = nullptr;
	std::vector<Ogre::Entity*> matBallList;
	uint32_t example_type = 1;
	Entity* gltfEntity = nullptr;
	Ogre::OgreTexture* prefilteredMap = nullptr;
	Ogre::OgreTexture* irradianceMap = nullptr;
	Ogre::OgreTexture* brdf = nullptr;

	Ogre::RenderTarget* prefilteredTarget = nullptr;
	Ogre::RenderTarget* irradianceTarget = nullptr;
	Ogre::RenderTarget* brdfTarget = nullptr;
	//SceneManager* mSceneManager;
	//GameCamera* mGameCamera;
	//RenderSystem* mRenderSystem;
	//RenderWindow* mRenderWindow;
};