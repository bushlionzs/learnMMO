#pragma once
#include "OgreSingleton.h"
#include "OgreMoveObject.h"
#include "InputManager.h"
#include "OgreFrameListener.h"
#include <CEGUI/CEGUI.h>
namespace CEGUI
{
	class GUIContext;
}

enum MouseType
{
	MouseType_Normal = 0,
	MouseType_Attack = 1,
	MouseType_Speak  = 2,
};

class GameToolTip;

class CEGUIManager : public Ogre::Singleton<CEGUIManager>,
	public FrameListener,
	public InputListener
{
public:
	CEGUIManager();
	~CEGUIManager();

	bool _initialise(Ogre::RenderTarget* window);

	virtual void injectMouseMove(int _absx, int _absy, int _absz);
	virtual void injectMousePress(int _absx, int _absy, OIS::MouseButtonID _id);
	virtual void injectMouseRelease(int _absx, int _absy, OIS::MouseButtonID _id);
	virtual void injectKeyPress(KeyCode _key, uint32_t _text);
	virtual void injectKeyRelease(KeyCode _key);

	virtual void injectMouseWheel(int _absz);

	Ogre::Camera* getCamera()
	{
		return mCamera;
	}

	Ogre::SceneManager* getSceneManager()
	{
		return mSceneManager;
	}

	Ogre::Viewport* getViewport()
	{
		return mViewPort;
	}
	virtual bool frameStarted(const FrameEvent& evt);

	Ogre::RenderTarget* getRenderWindows()
	{
		return mRenderWindow;
	}

	CEGUI::GUIContext* getGUIContext()
	{
		return mGUIContext;
	}

	void addRenderable(Ogre::Renderable* r);

	virtual const std::vector<Renderable*>& getRenderableList();

	virtual const AxisAlignedBox& getBoundingBox(void) const;

	const char* getFullIconName(const std::string& name);

	CEGUI::Window* getRootWindow()
	{
		return mRoot;
	}

	void setToolTip(GameToolTip* tip)
	{
		mToolTip = tip;
	}

	GameToolTip* getToolTip()
	{
		return mToolTip;
	}
	bool isMouseInGUI();

	void ChangeMouseCursor(MouseType type);
private:
	void _icon_init();
private:

	Ogre::Camera* mCamera;
	Ogre::Viewport* mViewPort;
	Ogre::SceneNode* mCameraNode;
	Ogre::SceneManager* mSceneManager;
	Ogre::RenderTarget* mRenderWindow;
	int32_t mOldAbsZ;

	Ogre::String mResourceFileName;

	CEGUI::GUIContext* mGUIContext = nullptr;

	CEGUI::Window* mRoot = nullptr;

	std::map<Ogre::String, Ogre::String>	mIconsMap;

	GameToolTip* mToolTip = nullptr;

	std::vector<Renderable*> mRenderables;
};