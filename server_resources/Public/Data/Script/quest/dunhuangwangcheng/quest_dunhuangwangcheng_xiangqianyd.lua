--ѯ��״��

--MisDescBegin
x256220_g_ScriptId = 256220
x256220_g_MissionIdPre = 3094
x256220_g_MissionId = 3095
x256220_g_LevelLess	= 	25 
x256220_g_MissionIdNext = 3053
x256220_g_Name	={}
x256220_g_ExtTarget={{type=20,n=1,target="��һ��#G�����#W��#G��������װ#W��@npc_82069һ��#G��ʯ#W"}}
x256220_g_MissionName="�����ˡ���һ����Ƕ"
x256220_g_MissionInfo="\t���װ������ѣ��ڸղŵ������У�������һ��#G��ʯ#W������Ƕ��һ�������#G��������װ��#W�ϡ�\n\t�������������װ����Ƕ��ʦ���������������#G��ʯ��Ƕ#W��ѡ����#G��ʯ��Ƕ#W�Ĺ�������ͬʱ����Ҳ���Զ��򿪡�\n\t �Ҽ������Ҫ #G��Ƕ��ʯ��װ��#W��Ҫ#G��Ƕ�ı�ʯ#W��������Ʒ���Զ������ڱ�ʯ��Ƕ�������У������#G��Ƕ#W��ť����ʯ�ͻ������ı���Ƕ����Ӧ��װ���ϡ���Ƕ��ɺ����رչ���������Ƕ�ʹ˽���������Ƕ����װ��Ҳ���Զ��ص����ı����С�"  --��������
x256220_g_MissionTarget=""		
x256220_g_MissionCompleted="\t�������Ѿ���ϸ�����Ǽ���Ƕ����ʯ������װ������ϸ�鿴һ���������ԣ������������Ӧ�����������˰ɣ�"					--�������npc˵���Ļ�
x256220_g_ContinueInfo="\t�㲻Ը��Ϊ����Ǽ�����װ����Ƕ��ʯ��"
x256220_g_MissionHelp =	""
x256220_g_DemandItem =	   {10014001,10024001,10034001,10044001,10054001,10064001,10074001,10084001,10094001,10104001,10114001,10124001,10234001,10234011,10234021,10234031,10234041,10234051,10254001,10254011,10254021,10254031,10254041,10254051,10274001,10274011,10274021,10274031,10274041,10274051,10244001, 10244011, 10244021,	10244031,	10244041,	10244051,	10264001,	10264011,	10264021,	10264031,	10264041,	10264051,	10204001, 10204011,	10204021,	10204031,	10204041,	10204051}					
x256220_g_BonusMoney1 =0
x256220_g_BonusMoney2 =0
x256220_g_BonusMoney3 =0
x256220_g_BonusMoney4 =0
x256220_g_BonusMoney5 =0
x256220_g_BonusMoney6 =0
x256220_g_BonusItem	=	{}
x256220_g_BonusChoiceItem ={}
x256220_g_ExpBonus = 10200
x256220_g_NpcGUID ={{ guid = 138554, name = "��ɭ��"} }
--MisDescEnd
--**********************************
x256220_g_MaxLevel   = 40

function x256220_ProcEnumEvent(sceneId, selfId, NPCId, MissionId)
    local level = GetLevel(sceneId, selfId)
	if level < x256220_g_LevelLess or level >= x256220_g_MaxLevel then
		return
	end


	if IsQuestHaveDone(sceneId, selfId, x256220_g_MissionId) > 0 then
		return 
	end
	
	if IsQuestHaveDone(sceneId, selfId, x256220_g_MissionIdPre)> 0 then 
	if IsHaveQuest(sceneId,selfId, x256220_g_MissionId) <= 0 then
		local state = GetQuestStateNM(sceneId,selfId,NPCId,x256220_g_MissionId)
		AddQuestTextNM( sceneId, selfId, NPCId, x256220_g_MissionId, state, -1 )
	end
	end

end

function x256220_DispatchMissionInfo( sceneId, selfId, NPCId )
		if x256220_g_ExpBonus> 0 then
			AddQuestExpBonus(sceneId, x256220_g_ExpBonus )
		end
		if x256220_g_BonusMoney1 > 0 then
			AddQuestMoneyBonus1(sceneId, x256220_g_BonusMoney1 )
		end
		if x256220_g_BonusMoney2 > 0 then
			AddQuestMoneyBonus2(sceneId, x256220_g_BonusMoney2 )
		end
		if x256220_g_BonusMoney3 > 0 then
			AddQuestMoneyBonus3(sceneId, x256220_g_BonusMoney3 )
		end
		if x256220_g_BonusMoney4 > 0 then
			AddQuestMoneyBonus4(sceneId, x256220_g_BonusMoney4 )
		end
		if x256220_g_BonusMoney5 > 0 then
			AddQuestMoneyBonus5(sceneId, x256220_g_BonusMoney5 )
		end
		if x256220_g_BonusMoney6 > 0 then
			AddQuestMoneyBonus6(sceneId, x256220_g_BonusMoney6 )
		end
		for i, item in x256220_g_BonusItem do
		   	AddQuestItemBonus(sceneId, item.item, item.n)
	 	end
		for i, item in x256220_g_BonusChoiceItem do
	    	AddQuestRadioItemBonus(sceneId, item.item, item.n)
	  	end
end
--**********************************

function x256220_ProcEventEntry(sceneId, selfId, NPCId, MissionId)	--����������ִ�д˽ű�
	if IsQuestHaveDone(sceneId, selfId, x256220_g_MissionId) > 0 then
		return 
	end
	if IsHaveQuest(sceneId,selfId, x256220_g_MissionId) > 0 then
			if x256220_CheckSubmit(sceneId, selfId, NPCId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"#Y"..x256220_g_MissionName)
				AddQuestText(sceneId,x256220_g_ContinueInfo)
				EndQuestEvent()
				DispatchQuestEventList(sceneId, selfId, NPCId, x256220_g_ScriptId, x256220_g_MissionId);
			else
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"#Y"..x256220_g_MissionName)
				AddQuestText(sceneId,x256220_g_MissionCompleted)
				AddQuestText(sceneId," ")
				x256220_DispatchMissionInfo( sceneId, selfId, NPCId )
				EndQuestEvent()
				DispatchQuestContinueInfoNM(sceneId, selfId, NPCId, x256220_g_ScriptId, x256220_g_MissionId);
			end
  elseif x256220_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then
			BeginQuestEvent(sceneId)
	  		AddQuestText(sceneId,"#Y"..x256220_g_MissionName)
			AddQuestText(sceneId,x256220_g_MissionInfo)
			AddQuestText( sceneId,"#Y����Ŀ�꣺")
			for i, item in x256220_g_ExtTarget do
					AddQuestText( sceneId,item.target)
			end 
			if x256220_g_MissionHelp ~= "" then
				AddQuestText(sceneId,"\n#GС��ʾ:#W")
				AddQuestText(sceneId,x256220_g_MissionHelp )
			end
			x256220_DispatchMissionInfo( sceneId, selfId, NPCId )
	  		EndQuestEvent()
			DispatchQuestInfoNM(sceneId, selfId, NPCId, x256220_g_ScriptId, x256220_g_MissionId);
  end
	
end
--**********************************

function x256220_ProcAcceptCheck(sceneId, selfId, NPCId)
		
	return 1
		

end
--**********************************
function x256220_CheckSubmit( sceneId, selfId, NPCId)  
                                                  
		if  IsHaveQuest(sceneId, selfId,x256220_g_MissionId) ~=1 then
			return 0
		end
		
	   	local 	misIndex = GetQuestIndexByID(sceneId, selfId,x256220_g_MissionId)                                                  
	   	if GetQuestParam(sceneId, selfId,misIndex,7,1) == 1 then
	   		return 1
	   	else 
	   		return 0
	   	end
	   	
	
end

--**********************************
function x256220_ProcQuestAccept( sceneId, selfId, NPCId, MissionId )
		if IsQuestHaveDone(sceneId, selfId, x256220_g_MissionId) > 0 then
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
		local FrontMissiontId1, FrontMissiontId2, FrontMissiontId3 = GetFrontQuestIdNM( sceneId, selfId,x256220_g_MissionId  )
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
		
		if x256220_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then
					local retmiss = AddQuest( sceneId, selfId, x256220_g_MissionId, x256220_g_ScriptId, 0, 0, 0,1)
					if retmiss ==0 then
						Msg2Player(sceneId, selfId, "��������ʧ�ܣ�", 0, 3)
						return 0
					else
		  			x256220_Msg2toplayer( sceneId, selfId,0)
						for i, item in x256220_g_DemandItem do
						local equip	=	GetEquipItemCountByLess(sceneId, selfId, 1,2,item)
						local bag	=	GetBagItemCountByLess(sceneId, selfId, 1,2,item)  --0--���� 1--��� 2--��ʯ
							if equip >= 1 or bag >=1 then
								local misIndex = GetQuestIndexByID(sceneId,selfId,x256220_g_MissionId)                                                  
			    				SetQuestByIndex(sceneId,selfId,misIndex,0,1)
			    				SetQuestByIndex(sceneId,selfId,misIndex,7,1)
			    				CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 36,0 )
									CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 48,0 )
									CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 37,0 )
									CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 38,0 )
			    				break
							end
						end
					end
		  			
	 	end                                                                    
	     
end
--**********************************

function x256220_ProcQuestAbandon(sceneId, selfId, MissionId)
		if IsHaveQuest(sceneId,selfId, x256220_g_MissionId) <= 0 then
				return 
		end
		DelQuest(sceneId, selfId, x256220_g_MissionId)
	  x256220_Msg2toplayer( sceneId, selfId,1)
		CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 36,0 )
		CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 48,0 )
		CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 37,0 )
		CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 38,0 )	
end
--**********************************
function x256220_CheckPlayerBagFull( sceneId ,selfId,selectRadioId )
	local result = 1
	local j = 0
	local bAdd = 0 --�Ƿ��Ѿ��ڹ̶���Ʒ�������Ӽ�����Ѿ�������������
	BeginAddItem(sceneId)
	for j, item in x256220_g_BonusItem do
		AddItem( sceneId, item.item, item.n )
  end
  for j, item in x256220_g_BonusChoiceItem do
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
function x256220_ProcQuestSubmit(sceneId, selfId, NPCId, selectRadioId, MissionId)
	if IsHaveQuest(sceneId,selfId, x256220_g_MissionId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x256220_CheckSubmit( sceneId, selfId, NPCId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û�дﵽ���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x256220_CheckPlayerBagFull( sceneId ,selfId,selectRadioId ) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"�����ռ䲻�㣬��������������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif DelQuest(sceneId, selfId, x256220_g_MissionId) > 0 then
				x256220_Msg2toplayer( sceneId, selfId,2)
				QuestCom(sceneId, selfId, x256220_g_MissionId)
				x256220_GetBonus( sceneId, selfId,NPCId)
				BeginAddItem(sceneId)
				for i, item in x256220_g_BonusItem do
 						AddBindItem( sceneId, item.item, item.n )
				end
				for i, item in x256220_g_BonusChoiceItem do
						if item.item == selectRadioId then
	    			    AddItem( sceneId, item.item, item.n )
						end
    		end
    		EndAddItem(sceneId,selfId)
    		--AddItemListToPlayer(sceneId,selfId)
   end
   CallScriptFunction( MISSION_SCRIPT, "ProcEventEntry", sceneId, selfId, NPCId, x256220_g_MissionIdNext )	
end

function x256220_GetBonus( sceneId, selfId,NpcID)
	  if x256220_g_ExpBonus > 0 then
			AddExp(sceneId, selfId, x256220_g_ExpBonus);
  	end
		if x256220_g_BonusMoney1 > 0 then
	    AddMoney( sceneId, selfId, 1, x256220_g_BonusMoney1 )
	  end
		if x256220_g_BonusMoney2 > 0 then
	    AddMoney( sceneId, selfId, 0, x256220_g_BonusMoney2 )
	  end
		if x256220_g_BonusMoney3 > 0 then
	    AddMoney( sceneId, selfId, 3, x256220_g_BonusMoney3 )
		end
		if x256220_g_BonusMoney4 > 0 then
		local nRongYu = GetRongYu( sceneId, selfId )
			nRongYu = nRongYu + x256220_g_BonusMoney4
			SetRongYu( sceneId, selfId, nRongYu )
		end
		if x256220_g_BonusMoney5 > 0 then
			local nShengWang = GetShengWang( sceneId, selfId )
			nShengWang = nShengWang + x256220_g_BonusMoney5
			SetShengWang( sceneId, selfId, nShengWang )
		end
		if x256220_g_BonusMoney6 > 0 then
			AddHonor(sceneId,selfId,x256220_g_BonusMoney6)
		end
end

function x256220_Msg2toplayer( sceneId, selfId,type)
		if type == 0 then
				Msg2Player(sceneId, selfId, "������������"..x256220_g_MissionName.."��", 0, 2)
		  	Msg2Player(sceneId, selfId, "������������"..x256220_g_MissionName.."��", 0, 3)
		elseif type == 1 then
				Msg2Player(sceneId, selfId, "������������"..x256220_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "������������"..x256220_g_MissionName.."��", 0, 3)
		elseif type == 2 then
				Msg2Player(sceneId, selfId, "�����������"..x256220_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "�����������"..x256220_g_MissionName.."��", 0, 3)
				if x256220_g_ExpBonus > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256220_g_ExpBonus.."#Y�Ľ���", 0, 2)
  			end
				if x256220_g_BonusMoney1 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R����"..x256220_g_BonusMoney1.."��#Y�Ľ���", 0, 2)
	  		end
				if x256220_g_BonusMoney2 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R����"..x256220_g_BonusMoney2.."��#Y�Ľ���", 0, 2)
	  		end
				if x256220_g_BonusMoney3 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R��"..x256220_g_BonusMoney3.."��#Y�Ľ���", 0, 2)
				end
				if x256220_g_BonusMoney4 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R��͢����ֵ"..x256220_g_BonusMoney4.."#Y�Ľ���", 0, 2)
				end
				if x256220_g_BonusMoney5 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256220_g_BonusMoney5.."#Y�Ľ���", 0, 2)
				end
				if x256220_g_BonusMoney6 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256220_g_BonusMoney6.."#Y�Ľ���", 0, 2)
				end
		end
end

function x256220_ProcQuestObjectKilled(sceneId, selfId, objdataId, objId, MissionId)
end


function x256220_ProcAreaEntered(sceneId, selfId, zoneId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "ProcAreaEntered",sceneId, selfId, zoneId, MissionId )	

end

function x256220_ProcTiming(sceneId, selfId, ScriptId, MissionId)
	 CallScriptFunction( MISSION_SCRIPT, "ProcTiming",sceneId, selfId, ScriptId, MissionId )
		
end

function x256220_ProcAreaLeaved(sceneId, selfId, ScriptId, MissionId)
	CallScriptFunction( MISSION_SCRIPT, "ProcAreaLeaved",sceneId, selfId, ScriptId, MissionId)
end

function x256220_ProcQuestItemChanged(sceneId, selfId, itemdataId, MissionId)
	CallScriptFunction( MISSION_SCRIPT, "ProcQuestItemChanged",sceneId, selfId, itemdataId, MissionId )
end


function x256220_ProcQuestAttach( sceneId, selfId, npcId, npcGuid, misIndex, MissionId )
	  local bFind  = 0
    for i ,item in x256220_g_NpcGUID do
        if item.guid == npcGuid then
            bFind = 1
            break
        end
    end
    if bFind == 0 then
        return
    end
		if IsHaveQuest(sceneId,selfId, x256220_g_MissionId) > 0 then
				local state = GetQuestStateNM(sceneId,selfId,npcId,x256220_g_MissionId)
				AddQuestTextNM( sceneId, selfId, npcId, x256220_g_MissionId, state, -1 )
				if x256220_CheckSubmit(sceneId, selfId, NPCId) <= 0 then
						CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 36,1 )
						CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 48,1 )
						CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 37,1 )
						CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 38,1 )
				end		
		end
end

function x256220_MissionComplate( sceneId, selfId, targetId, selectId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "MissionComplate", sceneId, selfId, targetId, selectId, MissionId  )
end

function x256220_PositionUseItem( sceneId, selfId, BagIndex, impactId )
	return  CallScriptFunction( MISSION_SCRIPT, "PositionUseItem",sceneId, selfId, BagIndex, impactId)
end

function x256220_OnOpenItemBox( sceneId, selfId, targetId, gpType, needItemID )
	return CallScriptFunction( MISSION_SCRIPT, "OnOpenItemBox",sceneId, selfId, targetId, gpType, needItemID)
end

function x256220_OnRecycle( sceneId, selfId, targetId, gpType, needItemID )
	return CallScriptFunction( MISSION_SCRIPT, "OnRecycle",sceneId, selfId, targetId, gpType, needItemID)
end

function x256220_OnProcOver( sceneId, selfId, targetId )
	CallScriptFunction( MISSION_SCRIPT, "OnProcOver",sceneId, selfId, targetId)
end

function x256220_OpenCheck( sceneId, selfId, targetId )
	CallScriptFunction( MISSION_SCRIPT, "OpenCheck", sceneId, selfId, targetId )
end