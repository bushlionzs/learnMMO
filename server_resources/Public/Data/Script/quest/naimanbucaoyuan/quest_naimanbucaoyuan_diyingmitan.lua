--��Ӫ��̽

--MisDescBegin
x202002_g_ScriptId = 202002
x202002_g_MissionIdPre = 1012
x202002_g_MissionId = 1020
x202002_g_LevelLess	= 30	 
x202002_g_MissionIdNext = 1013
x202002_g_Name	={}
x202002_g_ExtTarget={{type=20,n=1,target="�ҵ�@npcsp_Ǳ���ĺ�����_105191"}}
x202002_g_MissionName="���糡����Ӫ��̽"
x202002_g_MissionInfo="\t�������Ǻ��ó�Ǳ���ڵ�Ӫ֮�ڵģ���һ�������ڵ�Ӫ��Ĳݴ��У�ȥ����������Ӫ�ҵ����ɣ�"  --��������
x202002_g_MissionTarget=""		
x202002_g_MissionCompleted="\t����ô��֪���Ҳ�������ѵ��ǳ��������������ҵģ� "					--�������npc˵���Ļ�
x202002_g_ContinueInfo=""
x202002_g_MissionHelp =	""
x202002_g_DemandItem ={}
x202002_g_ExpBonus = 55000
x202002_g_BonusMoney1 =285
x202002_g_BonusMoney2 =0
x202002_g_BonusMoney3 =0
x202002_g_BonusMoney4 =0
x202002_g_BonusMoney5 =49
x202002_g_BonusMoney6 =0
x202002_g_BonusItem	=	{{item = 11000200,n=10}}
x202002_g_BonusChoiceItem ={}
x202002_g_NpcGUID ={{ guid = 105191, name = "������"} }
x202002_g_GroupPoint	={ {type=10,id =312,target = "������"} }
x202002_g_npcid = -1
--MisDescEnd
--**********************************

function x202002_ProcEnumEvent(sceneId, selfId, NPCId, MissionId)


	if IsQuestHaveDone(sceneId, selfId, x202002_g_MissionId) > 0 then
		return 
	end
	if IsQuestHaveDone(sceneId, selfId, x202002_g_MissionIdPre)> 0 then
		--if IsQuestHaveDone(sceneId, selfId, x202002_g_MissionIdPre[2])> 0 then
		--	if IsQuestHaveDone(sceneId, selfId, x202002_g_MissionIdPre[3])> 0 then
				--if IsQuestHaveDone(sceneId, selfId, x202002_g_MissionIdPre[4])> 0 then
						if IsHaveQuest(sceneId,selfId, x202002_g_MissionId) <= 0 then
								local state = GetQuestStateNM(sceneId,selfId,NPCId,x202002_g_MissionId)
								AddQuestTextNM( sceneId, selfId, NPCId, x202002_g_MissionId, state, -1 )
						end
				--end
		--	end
		--end
	end
end

function x202002_DispatchMissionInfo( sceneId, selfId, NPCId )
		if x202002_g_ExpBonus> 0 then
			AddQuestExpBonus(sceneId, x202002_g_ExpBonus )
		end
		if x202002_g_BonusMoney1 > 0 then
			AddQuestMoneyBonus1(sceneId, x202002_g_BonusMoney1 )
		end
		if x202002_g_BonusMoney2 > 0 then
			AddQuestMoneyBonus2(sceneId, x202002_g_BonusMoney2 )
		end
		if x202002_g_BonusMoney3 > 0 then
			AddQuestMoneyBonus3(sceneId, x202002_g_BonusMoney3 )
		end
		if x202002_g_BonusMoney4 > 0 then
			AddQuestMoneyBonus4(sceneId, x202002_g_BonusMoney4 )
		end
		if x202002_g_BonusMoney5 > 0 then
			AddQuestMoneyBonus5(sceneId, x202002_g_BonusMoney5 )
		end
		if x202002_g_BonusMoney6 > 0 then
			AddQuestMoneyBonus6(sceneId, x202002_g_BonusMoney6 )
		end
		for i, item in x202002_g_BonusItem do
		   	AddQuestItemBonus(sceneId, item.item, item.n)
	  end
		for i, item in x202002_g_BonusChoiceItem do
	    	AddQuestRadioItemBonus(sceneId, item.item, item.n)
	  end
end
--**********************************

function x202002_ProcEventEntry(sceneId, selfId, NPCId, MissionId)	--����������ִ�д˽ű�
	if IsQuestHaveDone(sceneId, selfId, x202002_g_MissionId) > 0 then
		return 
	end
	if IsHaveQuest(sceneId,selfId, x202002_g_MissionId) > 0 then
			if x202002_CheckSubmit(sceneId, selfId, NPCId) <= 0 then
						BeginQuestEvent(sceneId)
						AddQuestText(sceneId,"#Y"..x202002_g_MissionName)
						AddQuestText(sceneId,x202002_g_MissionCompleted)
						EndQuestEvent()
						DispatchQuestEventList(sceneId, selfId, NPCId, x202002_g_ScriptId, x202002_g_MissionId);
			else
						BeginQuestEvent(sceneId)
						AddQuestText(sceneId,"#Y"..x202002_g_MissionName)
						AddQuestText(sceneId,x202002_g_MissionCompleted)
						AddQuestText(sceneId," ")
						x202002_DispatchMissionInfo( sceneId, selfId, NPCId )
						EndQuestEvent()
						DispatchQuestContinueInfoNM(sceneId, selfId, NPCId, x202002_g_ScriptId, x202002_g_MissionId);
			end
  elseif x202002_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then
			BeginQuestEvent(sceneId)
	  	AddQuestText(sceneId,"#Y"..x202002_g_MissionName)
			AddQuestText(sceneId,x202002_g_MissionInfo)
			AddQuestText( sceneId,"#Y����Ŀ�꣺")
			for i, item in x202002_g_ExtTarget do
					AddQuestText( sceneId,item.target)
			end 
			if x202002_g_MissionHelp ~= "" then
				AddQuestText(sceneId,"\n#GС��ʾ:#W")
				AddQuestText(sceneId,x202002_g_MissionHelp )
			end
			x202002_DispatchMissionInfo( sceneId, selfId, NPCId )
	  	EndQuestEvent()
			DispatchQuestInfoNM(sceneId, selfId, NPCId, x202002_g_ScriptId, x202002_g_MissionId);
  end
	
end
--**********************************

function x202002_ProcAcceptCheck(sceneId, selfId, NPCId)
		
						return 1
		

end
--**********************************
function x202002_CheckSubmit( sceneId, selfId, NPCId)

	        return 1
	
end
--**********************************
function x202002_ProcQuestAccept( sceneId, selfId, NPCId, MissionId )
		if IsQuestHaveDone(sceneId, selfId, x202002_g_MissionId) > 0 then
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
		local FrontMissiontId1, FrontMissiontId2, FrontMissiontId3 = GetFrontQuestIdNM( sceneId, selfId,x202002_g_MissionId  )
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
		
		if x202002_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then
					local retmiss = AddQuest( sceneId, selfId, x202002_g_MissionId, x202002_g_ScriptId, 0, 0, 0,1)
					if retmiss ==0 then
						Msg2Player(sceneId, selfId, "��������ʧ�ܣ�", 0, 3)
						return 0
					else
		  			x202002_Msg2toplayer( sceneId, selfId,0)
						--if x202002_CheckSubmit(sceneId, selfId, NPCId) > 0 then
	    			--end
		  		end
	 	end                                                                    
	     
end
--**********************************

function x202002_ProcQuestAbandon(sceneId, selfId, MissionId)
		if IsHaveQuest(sceneId,selfId, x202002_g_MissionId) <= 0 then
				return 
		end
		DelQuest(sceneId, selfId, x202002_g_MissionId)
	  x202002_Msg2toplayer( sceneId, selfId,1)
			
end
--**********************************
function x202002_CheckPlayerBagFull( sceneId ,selfId,selectRadioId )
	local result = 1
	local j = 0
	local bAdd = 0 --�Ƿ��Ѿ��ڹ̶���Ʒ�������Ӽ�����Ѿ�������������
	BeginAddItem(sceneId)
	for j, item in x202002_g_BonusItem do
		AddBindItem( sceneId, item.item, item.n )
  end
  for j, item in x202002_g_BonusChoiceItem do
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
function x202002_ProcQuestSubmit(sceneId, selfId, NPCId, selectRadioId, MissionId)
	if IsHaveQuest(sceneId,selfId, x202002_g_MissionId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x202002_CheckSubmit( sceneId, selfId, NPCId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û�дﵽ���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x202002_CheckPlayerBagFull( sceneId ,selfId,selectRadioId ) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"����������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif DelQuest(sceneId, selfId, x202002_g_MissionId) > 0 then
				x202002_Msg2toplayer( sceneId, selfId,2)
				QuestCom(sceneId, selfId, x202002_g_MissionId)
				x202002_GetBonus( sceneId, selfId,NPCId)
				BeginAddItem(sceneId)
				for i, item in x202002_g_BonusItem do
 						AddBindItem( sceneId, item.item, item.n )
				end
				for i, item in x202002_g_BonusChoiceItem do
						if item.item == selectRadioId then
	    			    AddItem( sceneId, item.item, item.n )
						end
    		end
    		EndAddItem(sceneId,selfId)
    		AddItemListToPlayer(sceneId,selfId)
   end
   CallScriptFunction( MISSION_SCRIPT, "ProcEventEntry", sceneId, selfId, NPCId, x202002_g_MissionIdNext )	
end

function x202002_GetBonus( sceneId, selfId,NpcID)
	  if x202002_g_ExpBonus > 0 then
			AddExp(sceneId, selfId, x202002_g_ExpBonus);
  	end
		if x202002_g_BonusMoney1 > 0 then
	    AddMoney( sceneId, selfId, 1, x202002_g_BonusMoney1 )
	  end
		if x202002_g_BonusMoney2 > 0 then
	    AddMoney( sceneId, selfId, 0, x202002_g_BonusMoney2 )
	  end
		if x202002_g_BonusMoney3 > 0 then
	    AddMoney( sceneId, selfId, 3, x202002_g_BonusMoney3 )
		end
		if x202002_g_BonusMoney4 > 0 then
		local nRongYu = GetRongYu( sceneId, selfId )
			nRongYu = nRongYu + x202002_g_BonusMoney4
			SetRongYu( sceneId, selfId, nRongYu )
		end
		if x202002_g_BonusMoney5 > 0 then
			local nShengWang = GetShengWang( sceneId, selfId )
			nShengWang = nShengWang + x202002_g_BonusMoney5
			SetShengWang( sceneId, selfId, nShengWang )
		end
		if x202002_g_BonusMoney6 > 0 then
			AddHonor(sceneId,selfId,x202002_g_BonusMoney6)
		end
end

function x202002_Msg2toplayer( sceneId, selfId,type)
		if type == 0 then
				Msg2Player(sceneId, selfId, "������������"..x202002_g_MissionName.."��", 0, 2)
		  	Msg2Player(sceneId, selfId, "������������"..x202002_g_MissionName.."��", 0, 3)
		elseif type == 1 then
				Msg2Player(sceneId, selfId, "������������"..x202002_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "������������"..x202002_g_MissionName.."��", 0, 3)
		elseif type == 2 then
				Msg2Player(sceneId, selfId, "�����������"..x202002_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "�����������"..x202002_g_MissionName.."��", 0, 3)
				if x202002_g_ExpBonus > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x202002_g_ExpBonus.."#Y�Ľ���", 0, 2)
  			end
				if x202002_g_BonusMoney1 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R����"..x202002_g_BonusMoney1.."��#Y�Ľ���", 0, 2)
	  		end
				if x202002_g_BonusMoney2 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R����"..x202002_g_BonusMoney2.."��#Y�Ľ���", 0, 2)
	  		end
				if x202002_g_BonusMoney3 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R��"..x202002_g_BonusMoney3.."��#Y�Ľ���", 0, 2)
				end
				if x202002_g_BonusMoney4 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R��͢����ֵ"..x202002_g_BonusMoney4.."#Y�Ľ���", 0, 2)
				end
				if x202002_g_BonusMoney5 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x202002_g_BonusMoney5.."#Y�Ľ���", 0, 2)
				end
				if x202002_g_BonusMoney6 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x202002_g_BonusMoney6.."#Y�Ľ���", 0, 2)
				end
		end
end

function x202002_ProcQuestObjectKilled(sceneId, selfId, objdataId, objId, MissionId)
end


function x202002_ProcAreaEntered(sceneId, selfId, zoneId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "ProcAreaEntered",sceneId, selfId, zoneId, MissionId )	

end

function x202002_ProcTiming(sceneId, selfId, ScriptId, MissionId)
	 CallScriptFunction( MISSION_SCRIPT, "ProcTiming",sceneId, selfId, ScriptId, MissionId )
		
end

function x202002_ProcAreaLeaved(sceneId, selfId, ScriptId, MissionId)
	CallScriptFunction( MISSION_SCRIPT, "ProcAreaLeaved",sceneId, selfId, ScriptId, MissionId)
end

function x202002_ProcQuestItemChanged(sceneId, selfId, itemdataId, MissionId)
	CallScriptFunction( MISSION_SCRIPT, "ProcQuestItemChanged",sceneId, selfId, itemdataId, MissionId )
end


function x202002_ProcQuestAttach( sceneId, selfId, npcId, npcGuid, misIndex, MissionId )
	  local bFind  = 0
    for i ,item in x202002_g_NpcGUID do
        if item.guid == npcGuid then
            bFind = 1
            break
        end
    end
    if bFind == 0 then
        return
    end
		if IsHaveQuest(sceneId,selfId, x202002_g_MissionId) > 0 then
				local state = GetQuestStateNM(sceneId,selfId,npcId,x202002_g_MissionId)
				AddQuestTextNM( sceneId, selfId, npcId, x202002_g_MissionId, state, -1 )
		end
end

function x202002_MissionComplate( sceneId, selfId, targetId, selectId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "MissionComplate", sceneId, selfId, targetId, selectId, MissionId  )
end

function x202002_PositionUseItem( sceneId, selfId, BagIndex, impactId )
	return  CallScriptFunction( MISSION_SCRIPT, "PositionUseItem",sceneId, selfId, BagIndex, impactId)
end

function x202002_OnOpenItemBox( sceneId, selfId, targetId, gpType, needItemID )
		if IsQuestHaveDone(sceneId, selfId, x202002_g_MissionIdNext) > 0 then
			if IsHaveQuest( sceneId, selfId, x202002_g_MissionId) == 0 then
  		      Msg2Player( sceneId, selfId, "��û�н��ܡ����顿��Ӫ��̽�����ܲɼ�", 8, 3)
  		      return 1
  		  end
  		end	  
    		if IsObjValid (sceneId,x202002_g_npcid) == 1 then
					if GetName(sceneId,x202002_g_npcid)=="������" then
							Msg2Player(sceneId, selfId, "��������վ������濴����", 0, 3)
								local misIndex = GetQuestIndexByID(sceneId,selfId,x202002_g_MissionId)                                                  
			    			SetQuestByIndex(sceneId,selfId,misIndex,0,1)
			    			SetQuestByIndex(sceneId,selfId,misIndex,7,1)
							return 1
					else
							return 0
					end
			else
					return 0
			end					
-- return CallScriptFunction( MISSION_SCRIPT, "OnOpenItemBox",sceneId, selfId, targetId, gpType, needItemID)
end

function x202002_OnRecycle( sceneId, selfId, targetId, gpType, needItemID )
	if IsObjValid (sceneId,x202002_g_npcid) == 1 then
			if GetName(sceneId,x202002_g_npcid)=="������" then
					Msg2Player(sceneId, selfId, "��������վ������濴����", 0, 3)
					return 1
			else
					x202002_g_npcid=CreateMonster(sceneId, 153,173,188,3,1,-1,105191,20,30000,0,"������")
					local misIndex = GetQuestIndexByID(sceneId,selfId,x202002_g_MissionId)                                                  
	    			SetQuestByIndex(sceneId,selfId,misIndex,0,1)
	    			SetQuestByIndex(sceneId,selfId,misIndex,7,1)
					return 1
			end
	else
			x202002_g_npcid=CreateMonster(sceneId, 153,173,188,3,1,-1,105191,20,30000,180,"������")
			local misIndex = GetQuestIndexByID(sceneId,selfId,x202002_g_MissionId)                                                  
	    	SetQuestByIndex(sceneId,selfId,misIndex,0,1)
	    	SetQuestByIndex(sceneId,selfId,misIndex,7,1)
			return 1
	end				
end

function x202002_OnProcOver( sceneId, selfId, targetId )
	CallScriptFunction( MISSION_SCRIPT, "OnProcOver",sceneId, selfId, targetId)
end

function x202002_OpenCheck( sceneId, selfId, targetId )
	CallScriptFunction( MISSION_SCRIPT, "OpenCheck", sceneId, selfId, targetId )
end