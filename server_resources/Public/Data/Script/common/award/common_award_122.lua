
x580121_g_ScriptId = 580121
x580121_g_MissionName = "��ȡ�ƺŲ���"

--һ�������ʾ�Ľ�Ʒ��
x580121_g_MaxOnceAwardNum = 1

--�����NPC��
function x580121_OnEnumerate( sceneId, selfId, targetId, MissionId )
	return 0
	
end



--�������ť������������ִ�д˽ű�
function x580121_OnDefaultEvent( sceneId, selfId, targetId,state,index )	
	
	--δ�жϱ����Ƿ����㹻�ռ�

	x580121_DispatchMissionInfo( sceneId, selfId, targetId,index)

end

function x580121_DispatchMissionInfo( sceneId, selfId, targetId,index)

	SetPlayerRuntimeData( sceneId, selfId, RD_HUMAN_CURR_AWARD_INDEX, index )

	local title,text,scriptId,BagNeedSpace,MinSpliceNum = GetAwardInfo( sceneId, selfId,index )--���������ӱ��еõ�����
	local str = format("%s \n  %s",title,text)

	BeginQuestEvent(sceneId)
	AddQuestText(sceneId,str)
	EndQuestEvent(sceneId)
	DispatchQuestEventList(sceneId,selfId,targetId)
	DispatchQuestInfo(sceneId, selfId, targetId, x580121_g_ScriptId, -1);

end

--�콱
function x580121_AddAward( sceneId, selfId,itemNum,index, dbIndex )

	local title,text,scriptId,BagNeedSpace,MinSpliceNum = GetAwardInfo( sceneId, selfId,index )--���������ӱ��еõ�����
     local level = GetLevel(sceneId, selfId)
     local exp = level * 240000
     AddExp(sceneId, selfId, exp)  
		AwardTitle(sceneId, selfId, 40)
		--�콱�ɹ�
		BeginQuestEvent(sceneId)
		local sex = GetSex(sceneId,selfId)
		local TitleName = GetTitleNameByTitleID(40, sex)
        local level1 = GetLevel(sceneId, selfId)
        local exp1 = level1 * 240000
		local DoneMSG = format("��ϲ�����������%s�ƺ�",TitleName)
		local MSG     = format("�������%d�㾭��",exp1)
		AddQuestText(sceneId,DoneMSG)
		AddQuestText(sceneId,MSG)
		EndQuestEvent(sceneId)
		DispatchQuestTips(sceneId,selfId)
		
		--д�ɹ�LOG
		local logFmtMsg         = "%u,%d,\"%s\",%d,%d,%d,%d,%d" --GUID������ID���������ȼ�����ǰ������DBIndex,ItemIndex,�ɹ�(1) / ʧ��(0)
		local logMsg = format(logFmtMsg,GetPlayerGUID( sceneId,selfId ),GetWorldId(sceneId,selfId),GetName( sceneId,selfId ),GetLevel(sceneId,selfId),sceneId,dbIndex,index,1)
		WriteLog(16,logMsg)


end



--**********************************
--����
--**********************************
function x580121_OnAccept( sceneId, selfId )
end

function x580121_CheckAccept( sceneId, selfId, NPCId )
	return 1
end
--�����ȷ����
function x580121_CheckAndAccept( sceneId, selfId, NPCId )
end

--**********************************
--����
--**********************************
function x580121_OnAbandon( sceneId, selfId, MissionId )
end
--**********************************
--����
--**********************************
function x580121_OnContinue( sceneId, selfId, targetId )
end
--**********************************
--����Ƿ�����ύ
--**********************************
function x580121_CheckSubmit( sceneId, selfId )
end
--**********************************
--�ύ
--**********************************
function x580121_OnSubmit( sceneId, selfId, targetId,selectRadioId, MissionId )
end
--**********************************
--ɱ����������
--**********************************
function x580121_OnKillObject( sceneId, selfId, objdataId, objId, MissionId )
end
--**********************************
--���������¼�