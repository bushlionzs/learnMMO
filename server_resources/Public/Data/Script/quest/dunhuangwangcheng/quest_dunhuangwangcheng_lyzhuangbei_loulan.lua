--����ʯ�һ�

--MisDescBegin
x256260_g_ScriptId = 256260
x256260_g_MissionIdPre =3066
x256260_g_MissionId = 2600
x256260_g_LevelLess	= 	35
x256260_g_MissionIdNext = 4228
x256260_g_Name	={}
x256260_g_ExtTarget={{type=20,n=1,target="�һ�һ������ñ��"}}
x256260_g_MissionName="�����ˡ�����ʯ�һ�"
x256260_g_MissionInfo="\t�װ������ѣ����ҵ������кܶ���߼�������������װ��������Щ��װ����������и����������������������㹻������ʯ���������Ƕһ�һ����\n\t�������ʯ�һ��󣬻������ʯ�һ��̵ꡣ������ѡ��һ���Ʒ��#G����һ�����ñ��#W����������ʯ�����㹻������µ���һ����һ��ͻ�ɹ���"  --��������
x256260_g_MissionTarget=""		
x256260_g_MissionCompleted="\t���װ����Ȼ�����պ�ɳ�����������𵽲��ɵ͹������á�\n\tһ��Ҫ��ס���պ��ٻ������ʯ��������������һ�װ����"					--�������npc˵���Ļ�
x256260_g_ContinueInfo="\t�㲻��һ�һ������ñ����"
x256260_g_MissionHelp =	""
x256260_g_DemandItem =	   {10244001, 10244011, 10244021,	10244031,	10244041,	10244051,	10264001,	10264011,	10264021,	10264031,	10264041,	10264051,	10204001, 10204011,	10204021,	10204031,	10204041,	10204051,10204061,10269218,10249218,10204071,10269222,10249222,10204081,10269226,10249226,10204091,10269230,10249230,10204101,10269234,10249234,10204111,10269238,10249238}						
x256260_g_ExpBonus = 24000
x256260_g_BonusMoney1 =176
x256260_g_BonusMoney2 =0
x256260_g_BonusMoney3 =0
x256260_g_BonusMoney4 =0
x256260_g_BonusMoney5 =21
x256260_g_BonusMoney6 =0
x256260_g_BonusItem	=	{}
x256260_g_BonusChoiceItem ={}
x256260_g_NpcGUID ={{ guid = 123746
, name = "����ʯ�һ�"} }
--MisDescEnd
--**********************************
x256260_g_MaxLevel   = 50

function x256260_ProcEnumEvent(sceneId, selfId, NPCId, MissionId)
    local level = GetLevel(sceneId, selfId)
	if level < x256260_g_LevelLess or level >= x256260_g_MaxLevel then
		return
	end


	if IsQuestHaveDone(sceneId, selfId, x256260_g_MissionId) > 0 then
		return 
	end
	
	if IsQuestHaveDone(sceneId, selfId, x256260_g_MissionIdPre)> 0 then 
	if IsHaveQuest(sceneId,selfId, x256260_g_MissionId) <= 0 then
		local state = GetQuestStateNM(sceneId,selfId,NPCId,x256260_g_MissionId)
		AddQuestTextNM( sceneId, selfId, NPCId, x256260_g_MissionId, state, -1 )
	end
	end

end

function x256260_DispatchMissionInfo( sceneId, selfId, NPCId )
		if x256260_g_ExpBonus> 0 then
			AddQuestExpBonus(sceneId, x256260_g_ExpBonus )
		end
		if x256260_g_BonusMoney1 > 0 then
			AddQuestMoneyBonus1(sceneId, x256260_g_BonusMoney1 )
		end
		if x256260_g_BonusMoney2 > 0 then
			AddQuestMoneyBonus2(sceneId, x256260_g_BonusMoney2 )
		end
		if x256260_g_BonusMoney3 > 0 then
			AddQuestMoneyBonus3(sceneId, x256260_g_BonusMoney3 )
		end
		if x256260_g_BonusMoney4 > 0 then
			AddQuestMoneyBonus4(sceneId, x256260_g_BonusMoney4 )
		end
		if x256260_g_BonusMoney5 > 0 then
			AddQuestMoneyBonus5(sceneId, x256260_g_BonusMoney5 )
		end
		if x256260_g_BonusMoney6 > 0 then
			AddQuestMoneyBonus6(sceneId, x256260_g_BonusMoney6 )
		end
		for i, item in x256260_g_BonusItem do
		   	AddQuestItemBonus(sceneId, item.item, item.n)
	 	end
		for i, item in x256260_g_BonusChoiceItem do
	    	AddQuestRadioItemBonus(sceneId, item.item, item.n)
	  	end
end
--**********************************

function x256260_ProcEventEntry(sceneId, selfId, NPCId, MissionId)	--����������ִ�д˽ű�
	if IsQuestHaveDone(sceneId, selfId, x256260_g_MissionId) > 0 then
		return 
	end
	if IsHaveQuest(sceneId,selfId, x256260_g_MissionId) > 0 then
			if x256260_CheckSubmit(sceneId, selfId, NPCId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"#Y"..x256260_g_MissionName)
				AddQuestText(sceneId,x256260_g_ContinueInfo)
				EndQuestEvent()
				DispatchQuestEventList(sceneId, selfId, NPCId, x256260_g_ScriptId, x256260_g_MissionId);
			else
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"#Y"..x256260_g_MissionName)
				AddQuestText(sceneId,x256260_g_MissionCompleted)
				AddQuestText(sceneId," ")
				x256260_DispatchMissionInfo( sceneId, selfId, NPCId )
				EndQuestEvent()
				DispatchQuestContinueInfoNM(sceneId, selfId, NPCId, x256260_g_ScriptId, x256260_g_MissionId);
			end
  elseif x256260_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then
			BeginQuestEvent(sceneId)
	  		AddQuestText(sceneId,"#Y"..x256260_g_MissionName)
			AddQuestText(sceneId,x256260_g_MissionInfo)
			AddQuestText( sceneId,"#Y����Ŀ�꣺")
			for i, item in x256260_g_ExtTarget do
					AddQuestText( sceneId,item.target)
			end 
			if x256260_g_MissionHelp ~= "" then
				AddQuestText(sceneId,"\n#GС��ʾ:#W")
				AddQuestText(sceneId,x256260_g_MissionHelp )
			end
			x256260_DispatchMissionInfo( sceneId, selfId, NPCId )
	  		EndQuestEvent()
			DispatchQuestInfoNM(sceneId, selfId, NPCId, x256260_g_ScriptId, x256260_g_MissionId);
  end
	
end
--**********************************

function x256260_ProcAcceptCheck(sceneId, selfId, NPCId)
		
	return 1
		

end
--**********************************
function x256260_CheckSubmit( sceneId, selfId, NPCId)  
                                                  
		if  IsHaveQuest(sceneId, selfId,x256260_g_MissionId) ~=1 then
			return 0
		end
		
	   	local 	misIndex = GetQuestIndexByID(sceneId, selfId,x256260_g_MissionId)                                                  
	   	if GetQuestParam(sceneId, selfId,misIndex,7,1) == 1 then
	   		return 1
	   	else 
	   		return 0
	   	end
	   	
	
end

--**********************************
function x256260_ProcQuestAccept( sceneId, selfId, NPCId, MissionId )
		if IsQuestHaveDone(sceneId, selfId, x256260_g_MissionId) > 0 then
				return 
		end
		if IsQuestFullNM(sceneId,selfId)==1 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"�ɽ�������������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 
		end
		
		--���ǰ������
	local FrontMissiontId1, FrontMissiontId2, FrontMissiontId3 = GetFrontQuestIdNM( sceneId, selfId,x256260_g_MissionId  )
	if FrontMissiontId1 ~= -1 then
		if IsQuestHaveDoneNM( sceneId, selfId, FrontMissiontId1 ) == 0 then
			return 0
		end
	end
	if FrontMissiontId2 ~= -1 then
		if IsQuestHaveDoneNM( sceneId, selfId, FrontMissiontId2 ) == 0 then
			return 0
		end
	end
	if FrontMissiontId3 ~= -1 then
		if IsQuestHaveDoneNM( sceneId, selfId, FrontMissiontId3 ) == 0 then
			return 0
		end
	end
		
		if x256260_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then

					local retmiss = AddQuest( sceneId, selfId, x256260_g_MissionId, x256260_g_ScriptId, 0, 0, 1,1)
					if retmiss ==0 then
						Msg2Player(sceneId, selfId, "��������ʧ�ܣ�", 0, 3)
						return 0
					else
		  			x256260_Msg2toplayer( sceneId, selfId,0)
		  			
					for i, item in x256260_g_DemandItem do
						if GetItemCount(sceneId,selfId,x256260_g_DemandItem[i]) >= 1 then		
							local misIndex = GetQuestIndexByID(sceneId,selfId,x256260_g_MissionId)                                                  
	    					SetQuestByIndex(sceneId,selfId,misIndex,0,1)
	    					SetQuestByIndex(sceneId,selfId,misIndex,7,1)
						end
					end
					--x256260_ProcQuestItemChanged(sceneId, selfId, -1, MissionId)
					end
		  			
	 	end                                                                    
	     
end
--**********************************

function x256260_ProcQuestAbandon(sceneId, selfId, MissionId)
		if IsHaveQuest(sceneId,selfId, x256260_g_MissionId) <= 0 then
				return 
		end
		DelQuest(sceneId, selfId, x256260_g_MissionId)
	  x256260_Msg2toplayer( sceneId, selfId,1)
			
end
--**********************************
function x256260_CheckPlayerBagFull( sceneId ,selfId,selectRadioId )
	local result = 1
	local j = 0
	local bAdd = 0 --�Ƿ��Ѿ��ڹ̶���Ʒ�������Ӽ�����Ѿ�������������
	BeginAddItem(sceneId)
	for j, item in x256260_g_BonusItem do
		AddItem( sceneId, item.item, item.n )
  end
  for j, item in x256260_g_BonusChoiceItem do
        if item.item == selectRadioId then
            AddItem( sceneId, item.item, item.n )
            break
        end
  end
  local ret = EndAddItem(sceneId,selfId)
    if ret <= 0 then
        if result == 1 then
            result = 0
        end
    end
	return result
end
--**********************************
function x256260_ProcQuestSubmit(sceneId, selfId, NPCId, selectRadioId, MissionId)
	if IsHaveQuest(sceneId,selfId, x256260_g_MissionId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x256260_CheckSubmit( sceneId, selfId, NPCId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û�дﵽ���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x256260_CheckPlayerBagFull( sceneId ,selfId,selectRadioId ) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"�����ռ䲻�㣬��������������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif DelQuest(sceneId, selfId, x256260_g_MissionId) > 0 then
				x256260_Msg2toplayer( sceneId, selfId,2)
				QuestCom(sceneId, selfId, x256260_g_MissionId)
				x256260_GetBonus( sceneId, selfId,NPCId)
				BeginAddItem(sceneId)
				for i, item in x256260_g_BonusItem do
 						AddBindItem( sceneId, item.item, item.n )
				end
				for i, item in x256260_g_BonusChoiceItem do
						if item.item == selectRadioId then
	    			    AddItem( sceneId, item.item, item.n )
						end
    		end
    		EndAddItem(sceneId,selfId)
    		--AddItemListToPlayer(sceneId,selfId)
   end
   CallScriptFunction( MISSION_SCRIPT, "ProcEventEntry", sceneId, selfId, NPCId, x256260_g_MissionIdNext )	
end

function x256260_GetBonus( sceneId, selfId,NpcID)
	  if x256260_g_ExpBonus > 0 then
			AddExp(sceneId, selfId, x256260_g_ExpBonus);
  	end
		if x256260_g_BonusMoney1 > 0 then
	    AddMoney( sceneId, selfId, 1, x256260_g_BonusMoney1 )
	  end
		if x256260_g_BonusMoney2 > 0 then
	    AddMoney( sceneId, selfId, 0, x256260_g_BonusMoney2 )
	  end
		if x256260_g_BonusMoney3 > 0 then
	    AddMoney( sceneId, selfId, 3, x256260_g_BonusMoney3 )
		end
		if x256260_g_BonusMoney4 > 0 then
		local nRongYu = GetRongYu( sceneId, selfId )
			nRongYu = nRongYu + x256260_g_BonusMoney4
			SetRongYu( sceneId, selfId, nRongYu )
		end
		if x256260_g_BonusMoney5 > 0 then
			local nShengWang = GetShengWang( sceneId, selfId )
			nShengWang = nShengWang + x256260_g_BonusMoney5
			SetShengWang( sceneId, selfId, nShengWang )
		end
		if x256260_g_BonusMoney6 > 0 then
			AddHonor(sceneId,selfId,x256260_g_BonusMoney6)
		end
end

function x256260_Msg2toplayer( sceneId, selfId,type)
		if type == 0 then
				Msg2Player(sceneId, selfId, "������������"..x256260_g_MissionName.."��", 0, 2)
		  	Msg2Player(sceneId, selfId, "������������"..x256260_g_MissionName.."��", 0, 3)
		elseif type == 1 then
				Msg2Player(sceneId, selfId, "������������"..x256260_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "������������"..x256260_g_MissionName.."��", 0, 3)
		elseif type == 2 then
				Msg2Player(sceneId, selfId, "�����������"..x256260_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "�����������"..x256260_g_MissionName.."��", 0, 3)
				if x256260_g_ExpBonus > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256260_g_ExpBonus.."#Y�Ľ���", 0, 2)
  			end
				if x256260_g_BonusMoney1 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R����"..x256260_g_BonusMoney1.."��#Y�Ľ���", 0, 2)
	  		end
				if x256260_g_BonusMoney2 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R����"..x256260_g_BonusMoney2.."��#Y�Ľ���", 0, 2)
	  		end
				if x256260_g_BonusMoney3 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R��"..x256260_g_BonusMoney3.."��#Y�Ľ���", 0, 2)
				end
				if x256260_g_BonusMoney4 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R��͢����ֵ"..x256260_g_BonusMoney4.."#Y�Ľ���", 0, 2)
				end
				if x256260_g_BonusMoney5 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256260_g_BonusMoney5.."#Y�Ľ���", 0, 2)
				end
				if x256260_g_BonusMoney6 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256260_g_BonusMoney6.."#Y�Ľ���", 0, 2)
				end
		end
end

function x256260_ProcQuestObjectKilled(sceneId, selfId, objdataId, objId, MissionId)
end


function x256260_ProcAreaEntered(sceneId, selfId, zoneId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "ProcAreaEntered",sceneId, selfId, zoneId, MissionId )	

end

function x256260_ProcTiming(sceneId, selfId, ScriptId, MissionId)
	 CallScriptFunction( MISSION_SCRIPT, "ProcTiming",sceneId, selfId, ScriptId, MissionId )
		
end

function x256260_ProcAreaLeaved(sceneId, selfId, ScriptId, MissionId)
	CallScriptFunction( MISSION_SCRIPT, "ProcAreaLeaved",sceneId, selfId, ScriptId, MissionId)
end

function x256260_ProcQuestItemChanged( sceneId, selfId, itemdataId, MissionId,optype)
	if	optype	==	0	then
		return
	end
	
	for i, item in x256260_g_DemandItem do
	
		if itemdataId == item then

			if GetItemCount(sceneId,selfId,item) >= 1 then
					local misIndex = GetQuestIndexByID(sceneId,selfId,x256260_g_MissionId)                                                  
		    		SetQuestByIndex(sceneId,selfId,misIndex,0,1)
		    		SetQuestByIndex(sceneId,selfId,misIndex,7,1)
		  	else
		  			local misIndex = GetQuestIndexByID(sceneId,selfId,x256260_g_MissionId)                                                  
		    		SetQuestByIndex(sceneId,selfId,misIndex,0,0)
		    		SetQuestByIndex(sceneId,selfId,misIndex,7,0)
		  	end
		  	
		  	break
	  	
	  	end
	  	
	end

end


function x256260_ProcQuestAttach( sceneId, selfId, npcId, npcGuid, misIndex, MissionId )
	  local bFind  = 0
    for i ,item in x256260_g_NpcGUID do
        if item.guid == npcGuid then
            bFind = 1
            break
        end
    end
    if bFind == 0 then
        return
    end
		if IsHaveQuest(sceneId,selfId, x256260_g_MissionId) > 0 then
				local state = GetQuestStateNM(sceneId,selfId,npcId,x256260_g_MissionId)
				AddQuestTextNM( sceneId, selfId, npcId, x256260_g_MissionId, state, -1 )
		end
end

function x256260_MissionComplate( sceneId, selfId, targetId, selectId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "MissionComplate", sceneId, selfId, targetId, selectId, MissionId  )
end

function x256260_PositionUseItem( sceneId, selfId, BagIndex, impactId )
	return  CallScriptFunction( MISSION_SCRIPT, "PositionUseItem",sceneId, selfId, BagIndex, impactId)
end

function x256260_OnOpenItemBox( sceneId, selfId, targetId, gpType, needItemID )
	return CallScriptFunction( MISSION_SCRIPT, "OnOpenItemBox",sceneId, selfId, targetId, gpType, needItemID)
end

function x256260_OnRecycle( sceneId, selfId, targetId, gpType, needItemID )
	return CallScriptFunction( MISSION_SCRIPT, "OnRecycle",sceneId, selfId, targetId, gpType, needItemID)
end

function x256260_OnProcOver( sceneId, selfId, targetId )
	CallScriptFunction( MISSION_SCRIPT, "OnProcOver",sceneId, selfId, targetId)
end

function x256260_OpenCheck( sceneId, selfId, targetId )
	CallScriptFunction( MISSION_SCRIPT, "OpenCheck", sceneId, selfId, targetId )
end