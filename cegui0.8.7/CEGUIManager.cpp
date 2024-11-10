#include "OgreHeader.h"
#include "CEGUIManager.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreRenderWindow.h"
#include "OgreViewport.h"
#include <CEGUI/System.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/FontManager.h>
#include <CEGUI/SchemeManager.h>
#include <CEGUI/System.h>
//#include <CEGUI/ImagesetManager.h>
#include <CEGUI/Window.h>
#include <CEGUI/widgets/PushButton.h>
#include <CEGUI/ImageManager.h>
#include "Renderer.h"


using namespace CEGUI;
template<> CEGUIManager* Ogre::Singleton<CEGUIManager>::msSingleton = 0;

CEGUIManager::CEGUIManager()
{
	
}

CEGUIManager::~CEGUIManager()
{

}

bool CEGUIManager::_initialise(Ogre::RenderTarget* window)
{
	NOTICE_LOG("CEGUI initialise begin");

	mSceneManager = Ogre::Root::getSingleton().createSceneManger(std::string("cegui"));

	mCamera = mSceneManager->createCamera("cegui_camera");
	mRenderWindow = window;
	auto width = mRenderWindow->getWidth();
	auto height = mRenderWindow->getHeight();
	
	Real left = -width / 2.0f;
	Real right = width / 2.0f;
	Real top = height / 2.0f;
	Real bottom = -height / 2.0f;

	left = 0.0f;
	top = 0.0f;
	right = width;
	bottom = height;
	auto projectMatrix =
		Ogre::Math::makeOrthoLH(left, right, bottom, top, 0.1, 1000.0f);
	mCamera->updateProjectMatrix(projectMatrix);
	//mCamera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
	//mCamera->setOrthoWindow(width, height);
	Ogre::Vector3 eyePos = Ogre::Vector3(0, 0, -10);
	auto m = Ogre::Math::makeLookAtLH(eyePos, Ogre::Vector3::ZERO, Ogre::Vector3::UNIT_Y);
	mCamera->updateViewMatrix(m);
	InputManager::getSingleton().addListener(this);

	Ogre::Root::getSingleton().addFrameListener(this);

	CEGUI::OgreRenderer& render = CEGUI::OgreRenderer::create();

	CEGUI::System::create(render, nullptr, 0, nullptr, 0, "");

	CEGUI::System::getSingleton().notifyDisplaySizeChanged(
		CEGUI::Sizef((float)width, (float)height));

	auto& rt = render.getDefaultRenderTarget();
	auto& context = CEGUI::System::getSingleton().createGUIContext(rt);

	mGUIContext = &context;

	mRoot = (CEGUI::DefaultWindow*)CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "Root");
	CEGUIManager::getSingleton().getGUIContext()->setRootWindow(mRoot);

	ImageManager::getSingleton().loadImageset("MouseCursor.imageset");
	context.getMouseCursor().setDefaultImage("MouseCursor/Normal");
	
	if(1)
	{
		ImageManager::getSingleton().loadImageset("ui_duihua_1.imageset.xml");
		ImageManager::getSingleton().loadImageset("ui_mainboard_2.imageset.xml");
		ImageManager::getSingleton().loadImageset("ui_mainboard_3.imageset.xml");
		ImageManager::getSingleton().loadImageset("DriveIcons.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_icons_1.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_item_xiangzi_2.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_quest_gaoshi_1.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_equip_dao_2.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_equip_pifeng_1.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_equip_shangyi_1.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_equip_xianglian_1.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_equip_xie_1.imageset");
		ImageManager::getSingleton().loadImageset("ui_mainui_1.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_item_hongyao_1.imageset");

		ImageManager::getSingleton().loadImageset("cjsh_equip_shoutao_1.imageset");
		ImageManager::getSingleton().loadImageset("han2_equip_jiachuan_4.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_item_baoshi_3.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_item_xiangzi_1.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_npc_touxiang_8.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_npc_touxiang_13.imageset");
		ImageManager::getSingleton().loadImageset("ui_mainboard_5.imageset");
		ImageManager::getSingleton().loadImageset("cjsh_touxiang.imageset");
		
		ImageManager::getSingleton().loadImageset("test.imageset");
		
		CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
		CEGUI::SchemeManager::getSingleton().createFromFile("VanillaSkin.scheme");

		_icon_init();

		FontManager& fontManager(FontManager::getSingleton());
		fontManager.createFromFile("simhei8.font");
		CEGUI::Font& font(fontManager.createFromFile("simhei12.font"));
		mGUIContext->setDefaultFont(&font);
	}

	NOTICE_LOG("CEGUI initialise end");
	return true;
}

void CEGUIManager::addRenderable(Ogre::Renderable* r)
{
	mRenderables.push_back(r);
}

const std::vector<Renderable*>& CEGUIManager::getRenderableList()
{
	mRenderables.clear();
	mGUIContext->draw();

	return mRenderables;
}

const AxisAlignedBox& CEGUIManager::getBoundingBox(void) const
{
	static AxisAlignedBox box;
	box.setInfinite();
	return box;
}

const char* CEGUIManager::getFullIconName(const std::string& name)
{
	auto itor = mIconsMap.find(name);
	if (itor != mIconsMap.end())
	{
		return itor->second.c_str();
	}

	return nullptr;
}

void CEGUIManager::_icon_init()
{
	auto it = ImageManager::getSingleton().getIterator();
	std::string fullname;
	for (it.toStart(); !it.isAtEnd(); it++)
	{
		fullname = it.getCurrentKey().c_str();
		auto pos = fullname.find_last_of('/');
		if (pos != std::string::npos)
		{
			auto itor = mIconsMap.insert(std::make_pair(fullname.substr(pos + 1, fullname.size()), fullname));
			//assert(itor.second);
		}
	}
}

void CEGUIManager::injectMouseMove(int _absx, int _absy, int _absz)
{
	mGUIContext->injectMousePosition(_absx, _absy);
}

void CEGUIManager::injectMousePress(int _absx, int _absy, OIS::MouseButtonID _id)
{
	CEGUI::MouseButton dummy;
	dummy = (CEGUI::MouseButton)_id;
	//mGUIContext->injectMousePosition(_absx, _absy);
	mGUIContext->injectMouseButtonDown(dummy);

}

void CEGUIManager::injectMouseRelease(int _absx, int _absy, OIS::MouseButtonID _id)
{
	CEGUI::MouseButton dummy;
	dummy = (CEGUI::MouseButton)_id;
	//mGUIContext->injectMousePosition(_absx, _absy);
	mGUIContext->injectMouseButtonUp(dummy);
}

void CEGUIManager::injectKeyPress(KeyCode _key, uint32_t _text)
{
	
}

void CEGUIManager::injectKeyRelease(KeyCode _key)
{

}

void CEGUIManager::injectMouseWheel(int _absz)
{

}

bool CEGUIManager::frameStarted(const FrameEvent& evt)
{
	mGUIContext->injectTimePulse(evt.timeSinceLastFrame);
	return true;
}

bool CEGUIManager::isMouseInGUI()
{
	CEGUI::Window* current = mGUIContext->getWindowContainingMouse();
	if (current == nullptr)
	{
		return false;
	}

	std::string name = current->getName().c_str();
	if (current == mRoot)
	{
		return false;
	}
	return true;
}

void CEGUIManager::ChangeMouseCursor(MouseType type)
{
	const char* cursor = "MouseCursor/Normal";
	switch (type)
	{
	case MouseType_Normal:
		cursor = "MouseCursor/Normal";
		break;
	case MouseType_Attack:
		cursor = "MouseCursor/Attack";
		break;
	case MouseType_Speak:
		cursor = "MouseCursor/Speak";
		break;
	}
	mGUIContext->getMouseCursor().setImage(cursor);
}