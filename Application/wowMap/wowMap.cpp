#include "OgreHeader.h"
#include "wowMap.h"
#include "engine_manager.h"
#include "myutils.h"
#include "OgreResourceManager.h"
#include "OgreMaterialManager.h"
#include "OgreAnimationState.h"
#include <map_index.hpp>
#include "World.h"

WowMap::WowMap()
{

}

WowMap::~WowMap()
{

}

bool WowMap::appInit()
{
	ApplicationBase::appInit();

	std::string wmo = "WORLD\\WMO\\KHAZMODAN\\BUILDINGS\\DWARVEN_1STORY_HOVEL\\MUD_HOVEL\\MUD_HOVEL.WMO";
	mWorld = new World("Azeroth", 0);
	std::string m2name = "CREATURE\\AKAMA\\AKAMA.M2";
	m2name = "CREATURE\\GOBLIN\\GLBLINSHREDDER.M2";
	m2name = "CREATURE\\GOBLIN\\GOBLIN.M2";
	m2name = "ITEM\\OBJECTCOMPONENTS\\AMMO\\ARROWFIREFLIGHT_01.M2";
	
	Ogre::Vector3 pos(20037.3f, 150.875f, 16829.3f);
	mWorld->mapIndex.loadTile(pos);
	mGameCamera->setDistance(5.0f);
	mGameCamera->setMoveSpeed(25.0f);

	Ogre::Vector3 rolePos = Ogre::Vector3(20037.3f, 150.875f, 16829.3f);
	EngineManager::getSingletonPtr()->setMyPosition(rolePos);
	return true;
}

void WowMap::appUpdate(float delta)
{
	ApplicationBase::appUpdate(delta);
	if (mAnimationState)
	{
		mAnimationState->addTime(delta);
	}
}

EngineType WowMap::getEngineType()
{
	return EngineType_Dx11;
}

void WowMap::addCustomDirectory()
{
	ResourceManager::getSingletonPtr()->addDirectory(std::string("D:\\wow3.3.5\\Data"), "wow", true);
}