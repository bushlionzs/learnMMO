--vipļ��ȯ

--MisDescBegin
x256297_g_ScriptId = 256297
x256297_g_MissionIdPre =2614
x256297_g_MissionId = 2637
x256297_g_LevelLess	= 	42 
x256297_g_MissionIdNext = -1
x256297_g_Name	={}
x256297_g_ExtTarget={{type=20,n=1,target="�ڽ���̵�����һ�������̵깺��һ��VIPļ��ȯ����������Ա"}}
x256297_g_MissionName="�����ˡ���һ�ΰ��ļ��"
x256297_g_MissionInfo="\t����Ͱﹱ��һ������ҪҲ�ܼ򵥵Ļ�÷�ʽ�����ļ�������룬������Ӧ����Ϥһ�°��ļ�������̡���#G����̵�����һ�������̵�#W����һ��#GVIPļ��ȯ#W�����Ұ�"  --��������
x256297_g_MissionTarget=""		
x256297_g_MissionCompleted="\t�ܺã���Ȼ��#GVIPļ��ȯ#Wֻ���Ͻ�һ�Σ����Ժ���ÿ�춼��������ͨ�Ľ���̵깺����ͨ��#Gļ��ȯ#W�����ҡ�"					--�������npc˵���Ļ�
x256297_g_ContinueInfo="\t��ô����û������#GVIPļ��ȯ#W��"
x256297_g_MissionHelp =	"VIPļ��ȯ�ڽ���̵�����һ�������̵���Թ��򣬵���Ҫ���ĳ�ֵ�ﵽһ���Ľ����ܻ�����й�����Ӧ��Ʒ��Ȩ�ޡ�ÿ�����ÿ��������50��VIPļ��ȯ��"
x256297_g_DemandItem =	{11990116}						
x256297_g_BonusMoney1 =0
x256297_g_BonusMoney2 =0
x256297_g_BonusMoney3 =0
x256297_g_BonusMoney4 =0
x256297_g_BonusMoney5 =0
x256297_g_BonusMoney6 =0
x256297_g_BonusItem	=	{}
x256297_g_BonusChoiceItem ={}
x256297_g_ExpBonus = 1200000
x256297_g_BonusMoney8 =300
x256297_g_GuildExpBonus =1
x256297_g_NpcGUID ={{ guid = 129022, name = "������Ա"} }
--MisDescEnd
-- ���ȼ�
x256297_g_MaxLevel      = 160

--**********************************

function x256297_ProcEnumEvent(sceneId, selfId, NPCId, MissionId)
    local level = GetLevel(sceneId, selfId)
	if level < x256297_g_LevelLess or level >= x256297_g_MaxLevel then
		return
	end
	if GetGuildID( sceneId, selfId ) == -1 then
		--���ڰ����
		return
	end


	if IsQuestHaveDone(sceneId, selfId, x256297_g_MissionId) > 0 then --����ɹ�����
		return 
	end
	
	if IsQuestHaveDone(sceneId, selfId, x256297_g_MissionIdPre)> 0 then 
	if IsHaveQuest(sceneId,selfId, x256297_g_MissionId) <= 0 then
		local state = GetQuestStateNM(sceneId,selfId,NPCId,x256297_g_MissionId)
		AddQuestTextNM( sceneId, selfId, NPCId, x256297_g_MissionId, state, -1 )
	end
	end

end

function x256297_DispatchMissionInfo( sceneId, selfId, NPCId )
		if x256297_g_ExpBonus> 0 then
			AddQuestExpBonus(sceneId, x256297_g_ExpBonus )
		end
		if x256297_g_BonusMoney1 > 0 then
			AddQuestMoneyBonus1(sceneId, x256297_g_BonusMoney1 )
		end
		if x256297_g_BonusMoney2 > 0 then
			AddQuestMoneyBonus2(sceneId, x256297_g_BonusMoney2 )
		end
		if x256297_g_BonusMoney3 > 0 then
			AddQuestMoneyBonus3(sceneId, x256297_g_BonusMoney3 )
		end
		if x256297_g_BonusMoney4 > 0 then
			AddQuestMoneyBonus4(sceneId, x256297_g_BonusMoney4 )
		end
		if x256297_g_BonusMoney5 > 0 then
			AddQuestMoneyBonus5(sceneId, x256297_g_BonusMoney5 )
		end
		if x256297_g_BonusMoney6 > 0 then
			AddQuestMoneyBonus6(sceneId, x256297_g_BonusMoney6 )
		end
		if x256297_g_BonusMoney8 > 0 then
			AddQuestMoneyBonus8(sceneId, x256297_g_BonusMoney8 )
		end
		if x256297_g_GuildExpBonus>0 then
			AddQuestGuildExpBonus(sceneId,x256297_g_GuildExpBonus)
		end
		for i, item in x256297_g_BonusItem do
		   	AddQuestItemBonus(sceneId, item.item, item.n)
	 	end
		for i, item in x256297_g_BonusChoiceItem do
	    	AddQuestRadioItemBonus(sceneId, item.item, item.n)
	  	end
end
--**********************************

function x256297_ProcEventEntry(sceneId, selfId, NPCId, MissionId)	--����������ִ�д˽ű�
	if IsQuestHaveDone(sceneId, selfId, x256297_g_MissionId) > 0 then
		return 
	end
	if IsHaveQuest(sceneId,selfId, x256297_g_MissionId) > 0 then
			if x256297_CheckSubmit(sceneId, selfId, NPCId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"#Y"..x256297_g_MissionName)
				AddQuestText(sceneId,x256297_g_ContinueInfo)
				EndQuestEvent()
				DispatchQuestEventList(sceneId, selfId, NPCId, x256297_g_ScriptId, x256297_g_MissionId);
			else
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"#Y"..x256297_g_MissionName)
				AddQuestText(sceneId,x256297_g_MissionCompleted)
				AddQuestText(sceneId," ")
				x256297_DispatchMissionInfo( sceneId, selfId, NPCId )
				EndQuestEvent()
				DispatchQuestContinueInfoNM(sceneId, selfId, NPCId, x256297_g_ScriptId, x256297_g_MissionId);
			end
  elseif x256297_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then
			BeginQuestEvent(sceneId)
	  		AddQuestText(sceneId,"#Y"..x256297_g_MissionName)
			AddQuestText(sceneId,x256297_g_MissionInfo)
			AddQuestText( sceneId,"#Y����Ŀ�꣺")
			for i, item in x256297_g_ExtTarget do
					AddQuestText( sceneId,item.target)
			end 
			if x256297_g_MissionHelp ~= "" then
				AddQuestText(sceneId,"\n#GС��ʾ:#W")
				AddQuestText(sceneId,x256297_g_MissionHelp )
			end
			x256297_DispatchMissionInfo( sceneId, selfId, NPCId )
	  		EndQuestEvent()
			DispatchQuestInfoNM(sceneId, selfId, NPCId, x256297_g_ScriptId, x256297_g_MissionId);
  end
	
end
--**********************************

function x256297_ProcAcceptCheck(sceneId, selfId, NPCId)
		
	return 1
		

end
--**********************************
function x256297_CheckSubmit( sceneId, selfId, NPCId)  
                                                  
		if  IsHaveQuest(sceneId, selfId,x256297_g_MissionId) ~=1 then
			return 0
		end
		
	   	local 	misIndex = GetQuestIndexByID(sceneId, selfId,x256297_g_MissionId)                                                  
	   	if GetQuestParam(sceneId, selfId,misIndex,7,1) == 1 then
	   		return 1
	   	else 
	   		return 0
	   	end
	   	
	
end

--**********************************
function x256297_ProcQuestAccept( sceneId, selfId, NPCId, MissionId )
		if IsQuestHaveDone(sceneId, selfId, x256297_g_MissionId) > 0 then
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
--		local FrontMissiontId1, FrontMissiontId2, FrontMissiontId3 = GetFrontQuestIdNM( sceneId, selfId,x256297_g_MissionId  )
--		if FrontMissiontId1 ~= -1 then
--			if IsQuestHaveDoneNM( sceneId, selfId, FrontMissiontId1 ) == 0 then
--				return 0
--			end
--		end
--		if FrontMissiontId2 ~= -1 then
--			if IsQuestHaveDoneNM( sceneId, selfId, FrontMissiontId2 ) == 0 then
--				return 0
--			end
--		end
--		if FrontMissiontId3 ~= -1 then
--			if IsQuestHaveDoneNM( sceneId, selfId, FrontMissiontId3 ) == 0 then
--				return 0
--			end
--		end
		
		if x256297_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then

					local retmiss = AddQuest( sceneId, selfId, x256297_g_MissionId, x256297_g_ScriptId, 0, 0, 1,1)
					if retmiss ==0 then
						Msg2Player(sceneId, selfId, "��������ʧ�ܣ�", 0, 3)
						return 0
					else
		  			x256297_Msg2toplayer( sceneId, selfId,0)
		  			CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 25,1 )
		  			--CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 26,1 )
		  			CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 46,1 )
					for i, item in x256297_g_DemandItem do
						if GetItemCount(sceneId,selfId,item) >= 1 then		
							local misIndex = GetQuestIndexByID(sceneId,selfId,x256297_g_MissionId)                                                  
	    					SetQuestByIndex(sceneId,selfId,misIndex,0,1)
	    					SetQuestByIndex(sceneId,selfId,misIndex,7,1)
	    					CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 25,0 )
		  					--CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 26,0 )
		  					CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 46,0 )
						end
					end
					--x256297_ProcQuestItemChanged(sceneId, selfId, -1, MissionId)
					end
		  			
	 	end                                                                    
	     
end
--**********************************

function x256297_ProcQuestAbandon(sceneId, selfId, MissionId)
		if IsHaveQuest(sceneId,selfId, x256297_g_MissionId) <= 0 then
				return 
		end
		DelQuest(sceneId, selfId, x256297_g_MissionId)
	  x256297_Msg2toplayer( sceneId, selfId,1)
		CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 25,0 )
		--CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 26,0 )
		CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 46,0 )	
end
--**********************************
function x256297_CheckPlayerBagFull( sceneId ,selfId,selectRadioId )
	local result = 1
	local j = 0
	local bAdd = 0 --�Ƿ��Ѿ��ڹ̶���Ʒ�������Ӽ�����Ѿ�������������
	BeginAddItem(sceneId)
	for j, item in x256297_g_BonusItem do
		AddItem( sceneId, item.item, item.n )
  end
  for j, item in x256297_g_BonusChoiceItem do
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
function x256297_ProcQuestSubmit(sceneId, selfId, NPCId, selectRadioId, MissionId)
	if IsHaveQuest(sceneId,selfId, x256297_g_MissionId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x256297_CheckSubmit( sceneId, selfId, NPCId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û�дﵽ���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x256297_CheckPlayerBagFull( sceneId ,selfId,selectRadioId ) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"�����ռ䲻�㣬��������������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif GetGuildID( sceneId, selfId ) == -1 then
		--���ڰ����
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"����Ҫ�ȼ���һ����ᣡ")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
		
	else
				GetGuildQuestData(sceneId, selfId, GD_GUILD_INDEX_VIP_COLLECT_COUNT, x256297_g_ScriptId, -1, "OnReturn1" );		
   	end
   --CallScriptFunction( MISSION_SCRIPT, "ProcEventEntry", sceneId, selfId, NPCId, x256297_g_MissionIdNext )	
end

function x256297_OnReturn1(sceneId, selfId,MissionData,MissionId,targetId)
		

	if MissionData >= 49 then
		BeginQuestEvent(sceneId)
		AddQuestText(sceneId,"�ܱ�Ǹ��ÿ�����ÿ��ֻ�ܽ���50��VIPļ��ȯ������������")
		EndQuestEvent(sceneId)
		DispatchQuestTips(sceneId,selfId)
	elseif DelQuest(sceneId, selfId, x256297_g_MissionId) > 0 then
				local guildid = GetGuildID(sceneId,selfId);
				SetGuildQuestData(sceneId, guildid, GD_GUILD_INDEX_VIP_COLLECT_COUNT, 1, 1 );	
				x256297_Msg2toplayer( sceneId, selfId,2)
				DelItem( sceneId,selfId,11990116,1 )
				QuestCom(sceneId, selfId, x256297_g_MissionId)
				
				x256297_GetBonus( sceneId, selfId,NPCId)
				BeginAddItem(sceneId)
				for i, item in x256297_g_BonusItem do
 						AddBindItem( sceneId, item.item, item.n )
				end
				for i, item in x256297_g_BonusChoiceItem do
						if item.item == selectRadioId then
	    			    AddItem( sceneId, item.item, item.n )
						end
    		end
    		EndAddItem(sceneId,selfId)
    		--AddItemListToPlayer(sceneId,selfId)
   end
   --CallScriptFunction( MISSION_SCRIPT, "ProcEventEntry", sceneId, selfId, NPCId, x256297_g_MissionIdNext )	
end

function x256297_GetBonus( sceneId, selfId,NpcID)
	  if x256297_g_ExpBonus > 0 then
			AddExp(sceneId, selfId, x256297_g_ExpBonus);
  	end
		if x256297_g_BonusMoney1 > 0 then
	    AddMoney( sceneId, selfId, 1, x256297_g_BonusMoney1 )
	  end
		if x256297_g_BonusMoney2 > 0 then
	    AddMoney( sceneId, selfId, 0, x256297_g_BonusMoney2 )
	  end
		if x256297_g_BonusMoney3 > 0 then
	    AddMoney( sceneId, selfId, 3, x256297_g_BonusMoney3 )
		end
		if x256297_g_BonusMoney4 > 0 then
		local nRongYu = GetRongYu( sceneId, selfId )
			nRongYu = nRongYu + x256297_g_BonusMoney4
			SetRongYu( sceneId, selfId, nRongYu )
		end
		if x256297_g_BonusMoney5 > 0 then
			local nShengWang = GetShengWang( sceneId, selfId )
			nShengWang = nShengWang + x256297_g_BonusMoney5
			SetShengWang( sceneId, selfId, nShengWang )
		end
		if x256297_g_BonusMoney6 > 0 then
			AddHonor(sceneId,selfId,x256297_g_BonusMoney6)
		end

		if x256297_g_BonusMoney8 > 0 then
		AddGuildUserPoint(sceneId,selfId,x256297_g_BonusMoney8)	--���Ӱﹱ
		local BonusMoney8 = x256297_g_BonusMoney8
		local pointText = "���#R�ﹱ"..BonusMoney8.."��#o�Ľ���"
		BeginQuestEvent(sceneId)
		AddQuestText(sceneId, pointText);
		EndQuestEvent(sceneId)
		DispatchQuestTips(sceneId,selfId)			
  		Msg2Player(sceneId,selfId,pointText,8,2)
		end
-- ������ᾭ��
	local bangExp = x256297_g_GuildExpBonus 
	if bangExp > 0 then
		AddGuildExp(sceneId,selfId,bangExp)
		BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "���#R��ᾭ��"..bangExp.."��#o�Ľ���");
		EndQuestEvent(sceneId)
		DispatchQuestTips(sceneId,selfId)			
  		Msg2Player(sceneId,selfId,"���#R��ᾭ��"..bangExp.."��#o�Ľ���",8,2)
	end
end

function x256297_Msg2toplayer( sceneId, selfId,type)
			if type == 0 then
				Msg2Player(sceneId, selfId, "������������"..x256297_g_MissionName.."��", 0, 2)
		  	Msg2Player(sceneId, selfId, "������������"..x256297_g_MissionName.."��", 0, 3)
			elseif type == 1 then
				Msg2Player(sceneId, selfId, "������������"..x256297_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "������������"..x256297_g_MissionName.."��", 0, 3)
			elseif type == 2 then
				Msg2Player(sceneId, selfId, "�����������"..x256297_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "�����������"..x256297_g_MissionName.."��", 0, 3)
			if x256297_g_ExpBonus > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256297_g_ExpBonus.."#Y�Ľ���", 0, 2)
  			end
			if x256297_g_BonusMoney1 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R����"..x256297_g_BonusMoney1.."��#Y�Ľ���", 0, 2)
	  		end
			if x256297_g_BonusMoney2 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R����"..x256297_g_BonusMoney2.."��#Y�Ľ���", 0, 2)
	  		end
			if x256297_g_BonusMoney3 > 0 then
	  		  Msg2Player(sceneId, selfId, "#Y���#R��"..x256297_g_BonusMoney3.."��#Y�Ľ���", 0, 2)
			end
			if x256297_g_BonusMoney4 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R��͢����ֵ"..x256297_g_BonusMoney4.."#Y�Ľ���", 0, 2)
			end
			if x256297_g_BonusMoney5 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256297_g_BonusMoney5.."#Y�Ľ���", 0, 2)
			end
			if x256297_g_BonusMoney6 > 0 then
					Msg2Player(sceneId, selfId, "#Y���#R����"..x256297_g_BonusMoney6.."#Y�Ľ���", 0, 2)
			end
		end
end

function x256297_ProcQuestObjectKilled(sceneId, selfId, objdataId, objId, MissionId)
end


function x256297_ProcAreaEntered(sceneId, selfId, zoneId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "ProcAreaEntered",sceneId, selfId, zoneId, MissionId )	

end

function x256297_ProcTiming(sceneId, selfId, ScriptId, MissionId)
	 CallScriptFunction( MISSION_SCRIPT, "ProcTiming",sceneId, selfId, ScriptId, MissionId )
		
end

function x256297_ProcAreaLeaved(sceneId, selfId, ScriptId, MissionId)
	CallScriptFunction( MISSION_SCRIPT, "ProcAreaLeaved",sceneId, selfId, ScriptId, MissionId)
end

function x256297_ProcQuestItemChanged( sceneId, selfId, itemdataId, MissionId,optype)
	if	optype	==	0	then
		return
	end
	
	for i, item in x256297_g_DemandItem do
	
		if itemdataId == item then

			if GetItemCount(sceneId,selfId,item) >= 1 then
					local misIndex = GetQuestIndexByID(sceneId,selfId,x256297_g_MissionId)                                                  
		    		SetQuestByIndex(sceneId,selfId,misIndex,0,1)
		    		SetQuestByIndex(sceneId,selfId,misIndex,7,1)
		    		CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 25,0 )
		  			--CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 26,0 )
		  			CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 46,0 )
		  	else
		  			local misIndex = GetQuestIndexByID(sceneId,selfId,x256297_g_MissionId)                                                  
		    		SetQuestByIndex(sceneId,selfId,misIndex,0,0)
		    		SetQuestByIndex(sceneId,selfId,misIndex,7,0)
		  	end
		  	
		  	break
	  	
	  	end
	  	
	end

end


function x256297_ProcQuestAttach( sceneId, selfId, npcId, npcGuid, misIndex, MissionId )
	  local bFind  = 0
    for i ,item in x256297_g_NpcGUID do
        if item.guid == npcGuid then
            bFind = 1
            break
        end
    end
    if bFind == 0 then
        return
    end
		if IsHaveQuest(sceneId,selfId, x256297_g_MissionId) > 0 then
				local state = GetQuestStateNM(sceneId,selfId,npcId,x256297_g_MissionId)
				AddQuestTextNM( sceneId, selfId, npcId, x256297_g_MissionId, state, -1 )
		end
end

function x256297_MissionComplate( sceneId, selfId, targetId, selectId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "MissionComplate", sceneId, selfId, targetId, selectId, MissionId  )
end

function x256297_PositionUseItem( sceneId, selfId, BagIndex, impactId )
	return  CallScriptFunction( MISSION_SCRIPT, "PositionUseItem",sceneId, selfId, BagIndex, impactId)
end

function x256297_OnOpenItemBox( sceneId, selfId, targetId, gpType, needItemID )
	return CallScriptFunction( MISSION_SCRIPT, "OnOpenItemBox",sceneId, selfId, targetId, gpType, needItemID)
end

function x256297_OnRecycle( sceneId, selfId, targetId, gpType, needItemID )
	return CallScriptFunction( MISSION_SCRIPT, "OnRecycle",sceneId, selfId, targetId, gpType, needItemID)
end

function x256297_OnProcOver( sceneId, selfId, targetId )
	--CallScriptFunction( MISSION_SCRIPT, "OnProcOver",sceneId, selfId, targetId)
end

function x256297_OpenCheck( sceneId, selfId, targetId )
	--CallScriptFunction( MISSION_SCRIPT, "OpenCheck", sceneId, selfId, targetId )
end

function x256297_ProcQuestLogRefresh( sceneId, selfId, MissionId)
	
	if IsHaveQuest(sceneId,selfId, x256297_g_MissionId) ~= 1 then
		return 
	end
	
	BeginQuestEvent(sceneId)	
	if x256297_g_ExpBonus> 0 then
		AddQuestExpBonus(sceneId, x256297_g_ExpBonus )
	end
	if x256297_g_BonusMoney1 > 0 then
		AddQuestMoneyBonus1(sceneId, x256297_g_BonusMoney1 )
	end
	if x256297_g_BonusMoney2 > 0 then
		AddQuestMoneyBonus2(sceneId, x256297_g_BonusMoney2 )
	end
	if x256297_g_BonusMoney3 > 0 then
		AddQuestMoneyBonus3(sceneId, x256297_g_BonusMoney3 )
	end
	if x256297_g_BonusMoney4 > 0 then
		AddQuestMoneyBonus4(sceneId, x256297_g_BonusMoney4 )
	end
	if x256297_g_BonusMoney5 > 0 then
		AddQuestMoneyBonus5(sceneId, x256297_g_BonusMoney5 )
	end
	if x256297_g_BonusMoney6 > 0 then
		AddQuestMoneyBonus6(sceneId, x256297_g_BonusMoney6 )
	end
	if x256297_g_BonusMoney8 > 0 then
		AddQuestMoneyBonus8(sceneId, x256297_g_BonusMoney8 )
	end
	if x256297_g_GuildExpBonus>0 then
		AddQuestGuildExpBonus(sceneId,x256297_g_GuildExpBonus)
	end
	for i, item in x256297_g_BonusItem do
	   	AddQuestItemBonus(sceneId, item.item, item.n)
 	end
	for i, item in x256297_g_BonusChoiceItem do
    	AddQuestRadioItemBonus(sceneId, item.item, item.n)
  	end
	EndQuestEvent()
	DispatchQuestLogUpdate(sceneId, selfId, x256297_g_MissionId);
end