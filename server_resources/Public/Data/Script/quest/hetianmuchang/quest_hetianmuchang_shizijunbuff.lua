--���ػ͡���������  ��ǹ

--MisDescBegin
x203718_g_ScriptId = 203718
x203718_g_MissionIdPre =3395
x203718_g_MissionId = 3375
x203718_g_LevelLess	= 	1
x203718_g_MissionIdNext = 3387
x203718_g_Name	={}
x203718_g_ExtTarget={{type=20,n=1,target="�ҵ�@npc_126049ѧϰ��ʹ��2������ʥս����"}}
x203718_g_MissionName="�����֡�ְҵ�Ľ���"
x203718_g_MissionInfo="\t���Ѿ���Ϥ������ı�Ҫ������������Χ�Ļ�������������Ҫ��ǿ�ľ����������ʵ���ˣ����ǵļ��ܴ�ʦ���԰�����ٵ�����������������\n\t���ڣ������ȥ�����ˡ�"  --��������
x203718_g_MissionTarget=""		
x203718_g_MissionCompleted="\t�Ҿ�������ļ��ܴ�ʦ�������Ը�⣬�ҽ����������ĵ�ʦ��\n\t���ѧ���ϣ���������Լ�ɱ�б�û�����ʱ��ǵ�������ѧϰ�µļ��ܡ�"					--�������npc˵���Ļ�
x203718_g_ContinueInfo="\t���������������ܵȼ�����������Ϳ���ѧϰ��������ǹ�ļ��ܡ�\n\t���#G��ǹ����ѧϰ#W��ť���ܽ��뼼��ѧϰ���棬����#G�����ļ���#W��ʾ�����������յļ��ܣ������Ӧͼ�꼴��ѧϰ���ܣ���Ȼ��Ҫ��һ���ľ�������ӣ�\n\t���ǵ�ѧϰ�꼼�ܺ�Ҫ���Լ�#G�������ڼ��ܵ�ͼ��#W�ϵ��·�#G�������#W����������Ҫ��ʱ�򣬷����ݵ�ʹ�á�"
x203718_g_MissionHelp =	"#G�����԰��¼����ϵ�#R2#G�����������������·����ܿ�����е�#R����ʥս#G���ܡ�#W"
x203718_g_DemandItem ={}
x203718_g_ExpBonus = 855
x203718_g_BonusMoney1 =132
x203718_g_BonusMoney2 =0
x203718_g_BonusMoney3 =0
x203718_g_BonusMoney4 =0
x203718_g_BonusMoney5 =20
x203718_g_BonusMoney6 =0
x203718_g_BonusItem	=	{}
x203718_g_BonusChoiceItem ={}
x203718_g_NpcGUID ={{ guid = 126049, name = "ʮ�־����ܴ�ʦ"} } --����ظ���
--MisDescEnd
--**********************************

function x203718_ProcEnumEvent(sceneId, selfId, NPCId, MissionId)


	if IsQuestHaveDone(sceneId, selfId, x203718_g_MissionId) > 0 then
		return 
	end
	if IsQuestHaveDone(sceneId, selfId, x203718_g_MissionIdPre)> 0 then
		if GetZhiye( sceneId, selfId)==6 then
			local state = GetQuestStateNM(sceneId,selfId,NPCId,x203718_g_MissionId)
			AddQuestTextNM( sceneId, selfId, NPCId, x203718_g_MissionId, state, -1 )
		end
	end
end

function x203718_DispatchMissionInfo( sceneId, selfId, NPCId )
		if x203718_g_ExpBonus> 0 then
			AddQuestExpBonus(sceneId, x203718_g_ExpBonus )
		end
		if x203718_g_BonusMoney1 > 0 then
			AddQuestMoneyBonus1(sceneId, x203718_g_BonusMoney1 )
		end
		if x203718_g_BonusMoney2 > 0 then
			AddQuestMoneyBonus2(sceneId, x203718_g_BonusMoney2 )
		end
		if x203718_g_BonusMoney3 > 0 then
			AddQuestMoneyBonus3(sceneId, x203718_g_BonusMoney3 )
		end
		if x203718_g_BonusMoney4 > 0 then
			AddQuestMoneyBonus4(sceneId, x203718_g_BonusMoney4 )
		end
		if x203718_g_BonusMoney5 > 0 then
			AddQuestMoneyBonus5(sceneId, x203718_g_BonusMoney5 )
		end
		if x203718_g_BonusMoney6 > 0 then
			AddQuestMoneyBonus6(sceneId, x203718_g_BonusMoney6 )
		end
		for i, item in x203718_g_BonusItem do
		   	AddQuestItemBonus(sceneId, item.item, item.n)
	  end
		for i, item in x203718_g_BonusChoiceItem do
	    	AddQuestRadioItemBonus(sceneId, item.item, item.n)
	  end
end
--**********************************

function x203718_ProcEventEntry(sceneId, selfId, NPCId, MissionId)	--����������ִ�д˽ű�
	if IsQuestHaveDone(sceneId, selfId, x203718_g_MissionId) > 0 then
		return 
	end
	if IsHaveQuest(sceneId,selfId, x203718_g_MissionId) > 0 then
			if x203718_CheckSubmit(sceneId, selfId, NPCId) <= 0 then
						BeginQuestEvent(sceneId)
						AddQuestText(sceneId,"#Y"..x203718_g_MissionName)
						AddQuestText(sceneId,x203718_g_ContinueInfo)
						EndQuestEvent()
						DispatchQuestEventList(sceneId, selfId, NPCId, x203718_g_ScriptId, x203718_g_MissionId);
			else
						BeginQuestEvent(sceneId)
						AddQuestText(sceneId,"#Y"..x203718_g_MissionName)
						AddQuestText(sceneId,x203718_g_MissionCompleted)
						AddQuestText(sceneId," ")
						x203718_DispatchMissionInfo( sceneId, selfId, NPCId )
						EndQuestEvent()
						DispatchQuestContinueInfoNM(sceneId, selfId, NPCId, x203718_g_ScriptId, x203718_g_MissionId);
			end
  elseif x203718_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then
			BeginQuestEvent(sceneId)
	  	AddQuestText(sceneId,"#Y"..x203718_g_MissionName)
			AddQuestText(sceneId,x203718_g_MissionInfo)
			AddQuestText( sceneId,"#Y����Ŀ�꣺")
			for i, item in x203718_g_ExtTarget do
					AddQuestText( sceneId,item.target)
			end 
			if x203718_g_MissionHelp ~= "" then
				AddQuestText(sceneId,"\n#GС��ʾ:#W")
				AddQuestText(sceneId,x203718_g_MissionHelp )
			end
			x203718_DispatchMissionInfo( sceneId, selfId, NPCId )
	  	EndQuestEvent()
			DispatchQuestInfoNM(sceneId, selfId, NPCId, x203718_g_ScriptId, x203718_g_MissionId);
  end
	
end
--**********************************

function x203718_ProcAcceptCheck(sceneId, selfId, NPCId)
		if IsQuestHaveDone(sceneId, selfId, x203718_g_MissionIdPre)> 0 then
					return 1
		end
		return 0

end
--**********************************
function x203718_CheckSubmit( sceneId, selfId, NPCId)
	local misIndex = GetQuestIndexByID( sceneId, selfId, x203718_g_MissionId )
	if GetQuestParam(sceneId,selfId,misIndex,0) ==1 then 
					CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 23,0 ) 
	        return 1
	end
	return 0
	
end
--**********************************
function x203718_ProcQuestAccept( sceneId, selfId, NPCId, MissionId )
		if IsQuestHaveDone(sceneId, selfId, x203718_g_MissionId) > 0 then
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
		local FrontMissiontId1, FrontMissiontId2, FrontMissiontId3 = GetFrontQuestIdNM( sceneId, selfId,x203718_g_MissionId  )
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
		if x203718_ProcAcceptCheck(sceneId, selfId, NPCId) > 0 then
					local retmiss = AddQuest( sceneId, selfId, x203718_g_MissionId, x203718_g_ScriptId, 0, 0, 0, 1)
					if retmiss ==0 then
						Msg2Player(sceneId, selfId, "��������ʧ�ܣ�", 0, 3)
						return 0
					else
		  			x203718_Msg2toplayer( sceneId, selfId,0)
		  			  CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 23,1 )
		  			if x203718_CheckSubmit(sceneId, selfId, NPCId) > 0 then
								local misIndex = GetQuestIndexByID(sceneId,selfId,x203718_g_MissionId)                                                  
	    					SetQuestByIndex(sceneId,selfId,misIndex,0,1)
	    					SetQuestByIndex(sceneId,selfId,misIndex,7,1)
	    			end
		  		end
	 	end                                                                    
	     
end
--**********************************

function x203718_ProcQuestAbandon(sceneId, selfId, MissionId)
		if IsHaveQuest(sceneId,selfId, x203718_g_MissionId) <= 0 then
				return 
		end
		DelQuest(sceneId, selfId, x203718_g_MissionId)
	  x203718_Msg2toplayer( sceneId, selfId,1)
		CallScriptFunction( UTILITY_SCRIPT, "ShowNewUITips", sceneId, selfId, 23,0 )
			
end
--**********************************
function x203718_CheckPlayerBagFull( sceneId ,selfId,selectRadioId )
	local result = 1
	local j = 0
	local bAdd = 0 --�Ƿ��Ѿ��ڹ̶���Ʒ�������Ӽ�����Ѿ�������������
	BeginAddItem(sceneId)
	for j, item in x203718_g_BonusItem do
		AddItem( sceneId, item.item, item.n )
  end
  for j, item in x203718_g_BonusChoiceItem do
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
function x203718_ProcQuestSubmit(sceneId, selfId, NPCId, selectRadioId, MissionId)
	if IsHaveQuest(sceneId,selfId, x203718_g_MissionId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x203718_CheckSubmit( sceneId, selfId, NPCId) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"��û�дﵽ���������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif x203718_CheckPlayerBagFull( sceneId ,selfId,selectRadioId ) <= 0 then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId,"����������")
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				return 0
	elseif DelQuest(sceneId, selfId, x203718_g_MissionId) > 0 then
				x203718_Msg2toplayer( sceneId, selfId,2)
				QuestCom(sceneId, selfId, x203718_g_MissionId)
				--QuestCom(sceneId, selfId, 3098)
				x203718_GetBonus( sceneId, selfId,NPCId)
				BeginAddItem(sceneId)
				for i, item in x203718_g_BonusItem do
 						AddItem( sceneId, item.item, item.n )
				end
				for i, item in x203718_g_BonusChoiceItem do
						if item.item == selectRadioId then
	    			    AddItem( sceneId, item.item, item.n )
						end
    		end
    		EndAddItem(sceneId,selfId)
    		--AddItemListToPlayer(sceneId,selfId)
   end
   CallScriptFunction(MISSION_SCRIPT, "ProcEventEntry", sceneId, selfId, NPCId, x203718_g_MissionIdNext )	
end

function x203718_GetBonus( sceneId, selfId,NpcID)
	  if x203718_g_ExpBonus > 0 then
			AddExp(sceneId, selfId, x203718_g_ExpBonus);
  	end
		if x203718_g_BonusMoney1 > 0 then
	    AddMoney( sceneId, selfId, 1, x203718_g_BonusMoney1 )
	  end
		if x203718_g_BonusMoney2 > 0 then
	    AddMoney( sceneId, selfId, 0, x203718_g_BonusMoney2 )
	  end
		if x203718_g_BonusMoney3 > 0 then
	    AddMoney( sceneId, selfId, 3, x203718_g_BonusMoney3 )
		end
		if x203718_g_BonusMoney4 > 0 then
		local nRongYu = GetRongYu( sceneId, selfId )
			nRongYu = nRongYu + x203718_g_BonusMoney4
			SetRongYu( sceneId, selfId, nRongYu )
		end
		if x203718_g_BonusMoney5 > 0 then
			local nShengWang = GetShengWang( sceneId, selfId )
			nShengWang = nShengWang + x203718_g_BonusMoney5
			SetShengWang( sceneId, selfId, nShengWang )
		end
		if x203718_g_BonusMoney6 > 0 then
			AddHonor(sceneId,selfId,x203718_g_BonusMoney6)
		end
end

function x203718_Msg2toplayer( sceneId, selfId,type)
		if type == 0 then
				Msg2Player(sceneId, selfId, "������������"..x203718_g_MissionName.."��", 0, 2)
		  	Msg2Player(sceneId, selfId, "������������"..x203718_g_MissionName.."��", 0, 3)
		elseif type == 1 then
				Msg2Player(sceneId, selfId, "������������"..x203718_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "������������"..x203718_g_MissionName.."��", 0, 3)
		elseif type == 2 then
				Msg2Player(sceneId, selfId, "�����������"..x203718_g_MissionName.."��", 0, 2)
				Msg2Player(sceneId, selfId, "�����������"..x203718_g_MissionName.."��", 0, 3)
				if x203718_g_ExpBonus > 0 then
					Msg2Player(sceneId, selfId, "#cffcf00���#G����"..x203718_g_ExpBonus.."#cffcf00�Ľ���", 0, 2)
  			end
				if x203718_g_BonusMoney1 > 0 then
	  		  Msg2Player(sceneId, selfId, "#cffcf00���#G����"..x203718_g_BonusMoney1.."��#cffcf00�Ľ���", 0, 2)
	  		end
				if x203718_g_BonusMoney2 > 0 then
	  		  Msg2Player(sceneId, selfId, "#cffcf00���#G����"..x203718_g_BonusMoney2.."��#cffcf00�Ľ���", 0, 2)
	  		end
				if x203718_g_BonusMoney3 > 0 then
	  		  Msg2Player(sceneId, selfId, "#cffcf00���#G��"..x203718_g_BonusMoney3.."��#cffcf00�Ľ���", 0, 2)
				end
				if x203718_g_BonusMoney4 > 0 then
					Msg2Player(sceneId, selfId, "#cffcf00���#G��͢����ֵ"..x203718_g_BonusMoney4.."#cffcf00�Ľ���", 0, 2)
				end
				if x203718_g_BonusMoney5 > 0 then
					Msg2Player(sceneId, selfId, "#cffcf00���#G����"..x203718_g_BonusMoney5.."#cffcf00�Ľ���", 0, 2)
				end
				if x203718_g_BonusMoney6 > 0 then
					Msg2Player(sceneId, selfId, "#cffcf00���#G����"..x203718_g_BonusMoney6.."#cffcf00�Ľ���", 0, 2)
				end
		end
end

function x203718_ProcQuestObjectKilled(sceneId, selfId, objdataId, objId, MissionId)
end


function x203718_ProcAreaEntered(sceneId, selfId, zoneId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "ProcAreaEntered",sceneId, selfId, zoneId, MissionId )	

end

function x203718_ProcTiming(sceneId, selfId, ScriptId, MissionId)
	 CallScriptFunction( MISSION_SCRIPT, "ProcTiming",sceneId, selfId, ScriptId, MissionId )
		
end

function x203718_ProcAreaLeaved(sceneId, selfId, ScriptId, MissionId)
	CallScriptFunction( MISSION_SCRIPT, "ProcAreaLeaved",sceneId, selfId, ScriptId, MissionId)
end

function x203718_ProcQuestItemChanged(sceneId, selfId, itemdataId, MissionId)
	CallScriptFunction( MISSION_SCRIPT, "ProcQuestItemChanged",sceneId, selfId, itemdataId, MissionId )
end


function x203718_ProcQuestAttach( sceneId, selfId, npcId, npcGuid, misIndex, MissionId )
local bFind  = 0
    for i ,item in x203718_g_NpcGUID do
        if item.guid == npcGuid then
            bFind = 1
            break
        end
    end
    if bFind == 0 then
        return
    end
		if IsHaveQuest(sceneId,selfId, x203718_g_MissionId) > 0 then
				local state = GetQuestStateNM(sceneId,selfId,npcId,x203718_g_MissionId)
				AddQuestTextNM( sceneId, selfId, npcId, x203718_g_MissionId, state, -1 )
		end
end

function x203718_MissionComplate( sceneId, selfId, targetId, selectId, MissionId )
	return CallScriptFunction( MISSION_SCRIPT, "MissionComplate", sceneId, selfId, targetId, selectId, MissionId  )
end

function x203718_PositionUseItem( sceneId, selfId, BagIndex, impactId )
	return  CallScriptFunction( MISSION_SCRIPT, "PositionUseItem",sceneId, selfId, BagIndex, impactId)
end

function x203718_OnOpenItemBox( sceneId, selfId, targetId, gpType, needItemID )
	return CallScriptFunction( MISSION_SCRIPT, "OnOpenItemBox",sceneId, selfId, targetId, gpType, needItemID)
end

function x203718_OnRecycle( sceneId, selfId, targetId, gpType, needItemID )
	return CallScriptFunction( MISSION_SCRIPT, "OnRecycle",sceneId, selfId, targetId, gpType, needItemID)
end

function x203718_OnProcOver( sceneId, selfId, targetId )
	CallScriptFunction( MISSION_SCRIPT, "OnProcOver",sceneId, selfId, targetId)
end

function x203718_OpenCheck( sceneId, selfId, targetId )
	CallScriptFunction( MISSION_SCRIPT, "OpenCheck", sceneId, selfId, targetId )
end






