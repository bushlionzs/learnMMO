--���ػ͡���������

--MisDescBegin
x256213_g_ScriptId = 256213
x256213_g_MissionId = 1667
x256213_g_LevelLess	= 	20
x256213_g_MissionIdNext = 1628
x256213_g_MissionIdPre =1661
x256213_g_Name	={}
x256213_g_ExtTarget={{type=20,n=1,target="�ҵ�@npc_138569ѧϰ��ʹ��3��#R���֮��#W"}}
x256213_g_MissionName="�����ˡ����ܵĽ���"
x256213_g_MissionInfo="\t��������ֿ��������Ѿ������˻��������漼�ܣ���Ҫ�����ͷ�أ���������������ҿ��Ը���һ���İ�������Ȼ���ⲻ������ѵġ�"  --��������
x256213_g_MissionTarget="�ҵ�@npc_138569ѧϰ��ʹ��3��#R���֮��#W"		
x256213_g_MissionCompleted="\t�ܺã�ϣ�����������������"					--�������npc˵���Ļ�
x256213_g_ContinueInfo="\t�Ҷ�û���������˲�����Ȥ��"
x256213_g_MissionHelp =	"\t�ҵ�@npc_138569��ѧϰ��ʹ��3��#R���֮��#W��"
x256213_g_DemandItem ={}
x256213_g_BonusMoney1 =500
x256213_g_BonusMoney2 =0
x256213_g_BonusMoney3 =0
x256213_g_BonusMoney4 =0
x256213_g_BonusMoney5 =0
x256213_g_BonusMoney6 =0
x256213_g_BonusItem	=	{}
x256213_g_BonusChoiceItem ={}
x256213_g_ExpBonus = 11500
x256213_g_NpcGUID ={}
--MisDescEnd
--**********************************

function x256213_ProcEnumEvent(sceneId, selfId, NPCId, MissionId)

	if GetLevel(sceneId, selfId) < 20 then
		return
	end

	if IsQuestHaveDone(sceneId, selfId, x256213_g_MissionId) > 0 then
		return 
	end
	if IsQuestHaveDone(sceneId, selfId, x256213_g_MissionIdPre)> 0 then
		if GetZhiye( sceneId, selfId)==4 then
			local state = GetQuestStateNM(sceneId,selfId,NPCId,x256213_g_MissionId)
			AddQuestTextNM( sceneId, selfId, NPCId, x256213_g_MissionId, state, -1 )
		end
	end
end

function x256213_DispatchMissionInfo( sceneId, selfId, NPCId )
		if x256213_g_ExpBonus> 0 then
			AddQuestExpBonus(sceneId, x256213_g_ExpBonus )
		end
		if x256213_g_BonusMoney1 > 0 then
			AddQuestMoneyBonus1(sceneId, x256213_g_BonusMoney1 )
		end
		if x256213_g_BonusMoney2 > 0 then
			AddQuestMoneyBonus2(sceneId, x256213_g_BonusMoney2 )
		end
		if x256213_g_BonusMoney3 > 0 then
			AddQuestMoneyBonus3(sceneId, x256213_g_BonusMoney3 )
		end
		if x256213_g_BonusMoney4 > 0 then
			AddQuestMoneyBonus4(sceneId, x256213_g_BonusMoney4 )
		end
		if x256213_g_BonusMoney5 > 0 then
			AddQuestMoneyBonus5(sceneId, x256213_g_BonusMoney5 )
		end
		if x256213_g_BonusMoney6 > 0 then
			AddQuestMoneyBonus6(sceneId, x256213_g_BonusMoney6 )
		end
		for i, item in x256213_g_BonusItem do
		   	AddQuestItemBonus(sceneId, item.item, item.n)
	  end
		for i, item in x256213_g_BonusChoiceItem do
	    	AddQuestRadioItemBonus(sceneId, item.item, item.n)
	  end
end
--**********************************

function x256213_ProcEventEntry(sceneId, selfId, NPCId, MissionId)	--����������ִ�д˽ű�
	if IsQuestHaveDone(sceneId, selfId, x256213_g_MissionId) > 0 then
		return 
	end
	if IsHaveQuest(sceneId,selfId, x256213_g_MissionId) > 0 then
			if x256213_CheckSubmit(sceneId, selfId, NPCId) <= 0 then
						BeginQuestEvent(sceneId)
						AddQuestText(sceneId,"#Y"..x256213_g_MissionName)
						AddQuestText(sceneId,x256213_g_ContinueInfo)
						EndQuestEvent()
						DispatchQuestEventList(sceneId, selfId, NPCId, x256213_g_ScriptId, x256213_g_MissionId);
			else
						BeginQuestEvent(sceneId)
						AddQuestText(sceneId,"#Y"..x256213_g_MissionName)
						AddQuestText(sceneId,x256213_g_MissionCompleted)
						AddQuestText(sceneId," ")
						x256213_DispatchMissionInfo( sceneId, selfId, NPCId )
						EndQuestEvent()
						DispatchQuestContinueInfoNM(sceneId, selfId, NPCId, x256213_g_ScriptId, x256213_g_MissionId);
			end
  elseif x256213_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then
			BeginQuestEvent(sceneId)
	  	AddQuestText(sceneId,"#Y"..x256213_g_MissionName)
			AddQuestText(sceneId,x256213_g_MissionInfo)
			AddQuestText( sceneId,"#Y����Ŀ�꣺")
			for i, item in x256213_g_ExtTarget do
					AddQuestText( sceneId,item.target)
			end 
			if x256213_g_MissionHelp ~= "" then
				AddQuestText(sceneId,"\n#GС��ʾ:#W")
				AddQuestText(sceneId,x256213_g_MissionHelp )
			end
			x256213_DispatchMissionInfo( sceneId, selfId, NPCId )
	  	EndQuestEvent()
			DispatchQuestInfoNM(sceneId, selfId, NPCId, x256213_g_ScriptId, x256213_g_MissionId);
  end
	
end
--**********************************

function x256213_ProcAcceptCheck(sceneId, selfId, NPCId)
					return 1
end
--**********************************
function x256213_CheckSubmit( sceneId, selfId, NPCId)
	local misIndex = GetQuestIndexByID( sceneId, selfId, x256213_g_MissionId )
	if GetQuestParam(sceneId,selfId,misIndex,0) ==1 then 
	        return 1
	end
	return 0
	
end
--**********************************
function x256213_ProcQuestAccept( sceneId, selfId, NPCId, MissionId )
		if IsQuestHaveDone(sceneId, selfId, x256213_g_MissionId) > 0 then
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
		local FrontMissiontId1, FrontMissiontId2, FrontMissiontId3 = GetFrontQuestIdNM( sceneId, selfId,x256213_g_MissionId  )
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
		
		if x256213_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then
					local retmiss = AddQuest( sceneId, selfId, x256213_g_MissionId, x256213_g_ScriptId, 0, 0, 0)
					if retmiss ==0 then
						Msg2Player(sceneId, selfId, "��������ʧ�ܣ�", 0, 3)
						return 0
					else
		  			x256213_Msg2toplayer( sceneId, selfId,0)
		  			if x256213_CheckSubmit(sceneId, selfId, NPCId) > 0 then
								local misIndex = GetQuestIndexByID(sceneId,selfId,x256213_g_MissionId)                                                  
	    					SetQuestByIndex(sceneId,selfId,misIndex,0,1)
	    					SetQuestByIndex(sceneId,selfId,misIndex,7,1)
	   				end
		  		end
	 	end                                                                    
	     
end
--**********************************

function x256213_ProcQuestAbandon(sceneId, selfId, MissionId)
		if IsHaveQuest(sceneId,selfId, x256213_g_MissionId) <= 0 then
				return 
		end
		DelQuest(sceneId, selfId, x256213_g_MissionId)
	  x256213_Msg2toplayer( sceneId, selfId,1)
			
end
--**********************************
function x256213_CheckPlayerBagFull( sceneId ,selfId,selectRadioId )
	local result = 1
	local j = 0
	local bAdd = 0 --�Ƿ��Ѿ��ڹ̶���Ʒ�������Ӽ�����Ѿ�������������
	BeginAddItem(sceneId)
	for j, item in x256213_g_BonusItem do
		AddItem( sceneId, item.item, item.n )
  end
  for j, item in x256213_g_BonusChoiceItem do
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
function x256213_ProcQuestSubmit(sceneId, selfId, NPCId, selectRadioId, MissionId)
	if IsHaveQuest(sceneId,selfId, x256213_g_MissionId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x256213_CheckSubmit( sceneId, selfId, NPCId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û�дﵽ���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x256213_CheckPlayerBagFull( sceneId ,selfId,selectRadioId ) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"����������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif DelQuest(sceneId, selfId, x256213_g_MissionId) > 0 then
				x256213_Msg2toplayer( sceneId, selfId,2)
				QuestCom(sceneId, selfId, x256213_g_MissionId)
				x256213_GetBonus( sceneId, selfId,NPCId)
				BeginAddItem(sceneId)
				for i, item in x256213_g_BonusItem do
 						AddItem( sceneId, item.item, item.n )
				end
				for i, item in x256213_g_BonusChoiceItem do
						if item.item == selectRadioId then
	    			    AddItem( sceneId, item.item, item.n )
						end
    		end
    		EndAddItem(sceneId,selfId)
    		--AddItemListToPlayer(sceneId,selfId)
   end
   CallScriptFunction( MISSION_SCRIPT, "ProcEventEntry", sceneId, selfId, NPCId, x256213_g_MissionIdNext )	
end

function x256213_GetBonus( sceneId, selfId,NpcID)
	  if x256213_g_ExpBonus > 0 then
			AddExp(sceneId, selfId, x256213_g_ExpBonus);
  	end
		if x256213_g_BonusMoney1 > 0 then
	    AddMoney( sceneId, selfId, 1, x256213_g_BonusMoney1 )
	  end
		if x256213_g_BonusMoney2 > 0 then
	    AddMoney( sceneId, selfId, 0, x256213_g_BonusMoney2 )
	  end
		if x256213_g_BonusMoney3 > 0 then
	    AddMoney( sceneId, selfId, 3, x256213_g_BonusMoney3 )
		end
		if x256213_g_BonusMoney4 > 0 then
		local nRongYu = GetRongYu( sceneId, selfId )
			nRongYu = nRongYu + x256213_g_BonusMoney4
			SetRongYu( sceneId, selfId, nRongYu )
		end
		if x256213_g_BonusMoney5 > 0 then
			local nShengWang = GetShengWang( sceneId, selfId )
			nShengWang = nShengWang + x256213_g_BonusMoney5
			SetShengWang( sceneId, selfId, nShengWang )
		end
		if x256213_g_BonusMoney6 > 0 then
			AddHonor(sceneId,selfId,x256213_g_BonusMoney6)
		end
end

function x256213_Msg2toplayer( sceneId, selfId,type)
		if type == 0 then
				Msg2Player(sceneId, selfId, "������������"..x256213_g_MissionName.."��", 0, 2)
		  	Msg2Player(sceneId, selfId, "������������"..x256213_g_MissionName.."��", 0, 3)
		elseif type == 1 then
				Msg2Player(sceneId, selfId, "������������"..x256213_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "������������"..x256213_g_MissionName.."��", 0, 3)
		elseif type == 2 then
				Msg2Player(sceneId, selfId, "�����������"..x256213_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "�����������"..x256213_g_MissionName.."��", 0, 3)
				if x256213_g_ExpBonus > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256213_g_ExpBonus.."#Y�Ľ���", 0, 2)
  			end
				if x256213_g_BonusMoney1 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R����"..x256213_g_BonusMoney1.."��#Y�Ľ���", 0, 2)
	  		end
				if x256213_g_BonusMoney2 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R����"..x256213_g_BonusMoney2.."��#Y�Ľ���", 0, 2)
	  		end
				if x256213_g_BonusMoney3 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R��"..x256213_g_BonusMoney3.."��#Y�Ľ���", 0, 2)
				end
				if x256213_g_BonusMoney4 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R��͢����ֵ"..x256213_g_BonusMoney4.."#Y�Ľ���", 0, 2)
				end
				if x256213_g_BonusMoney5 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256213_g_BonusMoney5.."#Y�Ľ���", 0, 2)
				end
				if x256213_g_BonusMoney6 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256213_g_BonusMoney6.."#Y�Ľ���", 0, 2)
				end
		end
end

function x256213_ProcQuestObjectKilled(sceneId, selfId, objdataId, objId, MissionId)
end


function x256213_ProcAreaEntered(sceneId, selfId, zoneId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "ProcAreaEntered",sceneId, selfId, zoneId, MissionId )	

end

function x256213_ProcTiming(sceneId, selfId, ScriptId, MissionId)
	 CallScriptFunction( MISSION_SCRIPT, "ProcTiming",sceneId, selfId, ScriptId, MissionId )
		
end

function x256213_ProcAreaLeaved(sceneId, selfId, ScriptId, MissionId)
	CallScriptFunction( MISSION_SCRIPT, "ProcAreaLeaved",sceneId, selfId, ScriptId, MissionId)
end

function x256213_ProcQuestItemChanged(sceneId, selfId, itemdataId, MissionId)
	CallScriptFunction( MISSION_SCRIPT, "ProcQuestItemChanged",sceneId, selfId, itemdataId, MissionId )
end


function x256213_ProcQuestAttach( sceneId, selfId, npcId, npcGuid, misIndex, MissionId )

end

function x256213_MissionComplate( sceneId, selfId, targetId, selectId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "MissionComplate", sceneId, selfId, targetId, selectId, MissionId  )
end

function x256213_PositionUseItem( sceneId, selfId, BagIndex, impactId )
	return  CallScriptFunction( MISSION_SCRIPT, "PositionUseItem",sceneId, selfId, BagIndex, impactId)
end

function x256213_OnOpenItemBox( sceneId, selfId, targetId, gpType, needItemID )
	return CallScriptFunction( MISSION_SCRIPT, "OnOpenItemBox",sceneId, selfId, targetId, gpType, needItemID)
end

function x256213_OnRecycle( sceneId, selfId, targetId, gpType, needItemID )
	return CallScriptFunction( MISSION_SCRIPT, "OnRecycle",sceneId, selfId, targetId, gpType, needItemID)
end

function x256213_OnProcOver( sceneId, selfId, targetId )
	CallScriptFunction( MISSION_SCRIPT, "OnProcOver",sceneId, selfId, targetId)
end

function x256213_OpenCheck( sceneId, selfId, targetId )
	CallScriptFunction( MISSION_SCRIPT, "OpenCheck", sceneId, selfId, targetId )
end