----------------------------------------------------
-- �һ��ι���
----------------------------------------------------

x570034_g_ScriptId 					= 570034

x570034_g_LevelMin					= 80
x570034_g_LevelMax					= 100


x570034_g_ItemId1					= 11000220
x570034_g_NeedItemCount1				= 1	
x570034_g_ItemId2					= 11010014
x570034_g_NeedItemCount2				= 4	
x570034_g_BonusItemId				= 11000402



x570034_g_MissionName				= "���һ����ι���"
x570034_g_EnterInfo					= "\n\t1��#G�ι���ʯ#W��4��#Gҹ����#W�һ�1��#G�ι���#W��"
-------------------------
--***********************
--ProcEnumEvent
--***********************
-------------------------

function x570034_ProcEnumEvent(sceneId, selfId, targetId, MissionId)

	AddQuestNumText(sceneId, x570034_g_ScriptId, x570034_g_MissionName,3,1);
	
end

---------------------------------------------------------------------------------------------------
--���븱��������ʾ
---------------------------------------------------------------------------------------------------
function x570034_DispatchEnterInfo( sceneId, selfId, NPCId )

	BeginQuestEvent(sceneId)

		--������Ϣ
		local suipiannum = GetItemCount(sceneId, selfId, x570034_g_ItemId1)      
		local yemingzhu = GetItemCount(sceneId, selfId, x570034_g_ItemId2)       
		local strText1 = format("\n\t��������#R%d��#W#G�ι���ʯ#W��",suipiannum) 
		local strText2 = format("\n\t��������#R%d��#W#Gҹ����#W��",yemingzhu)    
		                                                                         
		
		AddQuestText(sceneId,"#Y"..x570034_g_MissionName)
		AddQuestText(sceneId,x570034_g_EnterInfo)
		AddQuestText(sceneId,strText1)     
		AddQuestText(sceneId,strText2)     
        AddQuestItemBonus(sceneId, x570034_g_BonusItemId,1)
		--AddQuestText(sceneId," ")

		


	EndQuestEvent()
	DispatchQuestInfo(sceneId, selfId, NPCId, x570034_g_ScriptId, -1);
end


---------------------------------------------------------------------------------------------------
--������븱��
---------------------------------------------------------------------------------------------------
function x570034_ProcAcceptCheck( sceneId, selfId, NPCId )
    return 1
end

----------------------------------------------------------------------------------------------
--��ȷ����
----------------------------------------------------------------------------------------------
function x570034_ProcAccept( sceneId, selfId )

	x570034_OnRequestSubmit(sceneId, selfId);
end


-------------------------
--***********************
--ProcEventEntry
--***********************
-------------------------


function x570034_ProcEventEntry(sceneId, selfId, NPCId, nScriptId, nFlag)

	if nFlag== nil then
		return
	end

	x570034_DispatchEnterInfo( sceneId, selfId, NPCId )
	
end


function x570034_OnRequestSubmit(sceneId, selfId)

	local iRet = x570034_CheckRequest(sceneId, selfId)

	if iRet>0 then
		x570034_OnApproveRequest(sceneId, selfId)
	end
end

function x570034_CheckRequest(sceneId, selfId)

	

	local ItemCnt = GetItemCount( sceneId, selfId, x570034_g_ItemId1 )

	if ItemCnt<x570034_g_NeedItemCount1 then
		local str =  format("�ι���ʯ����%d����",x570034_g_NeedItemCount1)

		BeginQuestEvent(sceneId);
			AddQuestText(sceneId, str);
		EndQuestEvent();

		DispatchQuestTips(sceneId, selfId);
		return 0;
	end

	local ItemCnt = GetItemCount( sceneId, selfId, x570034_g_ItemId2 )

	if ItemCnt<x570034_g_NeedItemCount2 then
		local str =  format("ҹ���鲻��%d����",x570034_g_NeedItemCount2)

		BeginQuestEvent(sceneId);
			AddQuestText(sceneId, str);
		EndQuestEvent();

		DispatchQuestTips(sceneId, selfId);
		return 0;
	end

	

	return 1;
	
end

function x570034_OnApproveRequest(sceneId, selfId)

	if x570034_GetBonus(sceneId, selfId)>0 then
		local iRet = DelItem(sceneId, selfId,x570034_g_ItemId1,x570034_g_NeedItemCount1)
    local iRet = DelItem(sceneId, selfId,x570034_g_ItemId2,x570034_g_NeedItemCount2)

	end
		
	
	
end


function x570034_GetBonus(sceneId, selfId)
	BeginAddItem(sceneId)
	AddBindItem( sceneId, x570034_g_BonusItemId, 1 )
	
	local ret = EndAddItem(sceneId,selfId)
	if ret > 0 then
		AddItemListToPlayer(sceneId,selfId)
		BeginQuestEvent(sceneId)
			AddQuestText(sceneId,"�һ��ɹ���")
		EndQuestEvent(sceneId)
		DispatchQuestTips(sceneId,selfId)
		return 1;
	else
		BeginQuestEvent(sceneId)
		AddQuestText(sceneId,"��Ʒ���������޷��õ���Ʒ��")
		EndQuestEvent(sceneId)
		DispatchQuestTips(sceneId,selfId)
		return 0;
	end
end
