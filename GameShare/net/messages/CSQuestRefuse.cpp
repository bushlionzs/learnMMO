#include "stdafx.h"
#include "CSQuestRefuse.h"
#include "server_manager.h"
#include "script/LuaSystem.h"
#include "ScriptDef.h"
#include "map/game_map.h"
#include "gameobject/Player.h"
#include "item/ItemInstance.h"
#include "map/map_manager.h"
#include "gameobject/ObjectManager.h"
#include "gameobject/Monster.h"
#include "item/Item_Interface.h"
#include "data/data_manager.h"
#include "Share/tab/TabDefine_Map_Quest.h"

CSQuestRefuse::CSQuestRefuse() :
	NetPacket(CS_QUEST_REFUSE)
{

}

CSQuestRefuse::~CSQuestRefuse()
{

}

bool CSQuestRefuse::process()
{
	GameMap* pMap = MapManager::GetSingletonPtr()->getMap(mMapId);

	Object* pObj = pMap->GetSpecificObjByID(mPlayerId);
	if (pObj == NULL || pObj->GetObjType() != Object::OBJECT_CLASS_PLAYER)
	{
		return false;
	}

	Player* pPlayer = (Player*)pObj;

	pObj = pMap->GetSpecificObjByID(mTargetId);

	if (pObj == NULL || pObj->GetObjType() != Object::OBJECT_CLASS_MONSTER)
	{
		return false;
	}

	Monster* pMonster = (Monster*)pObj;

	NPC_QUEST* pQuest = DataManager::GetSingletonPtr()->getNpcQuest(pMonster->GetGUID());
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	if (pQuest != NULL)
	{
		LuaSystem* luaSystem = ServerManager::GetSingletonPtr()->getLuaSystem();
		for (int32 i = 0; i < pQuest->m_EventCount; i++)
		{
			luaSystem->RunScriptFunction
			(
				mMapId,
				pQuest->m_EventList[i],
				DEF_PROC_ENUM_EVENT_FN,
				mMapId,
				mPlayerId,
				mTargetId,
				-1
			);
		}
	}

	return true;
}

void CSQuestRefuse::setPlayerId(int32_t playerId)
{
	mPlayerId = playerId;
}

void CSQuestRefuse::setMapId(int32_t mapId)
{
	mMapId = mapId;
}

void CSQuestRefuse::setTargetId(int32_t objId)
{
	mTargetId = objId;
}

void CSQuestRefuse::setIdScript(int32_t idScript)
{
	mIdScript = idScript;
}