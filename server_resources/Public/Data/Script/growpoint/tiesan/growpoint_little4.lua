--������

x301424_g_GrowpointId = 706 		--��Ӧ������ID

x301424_g_MissionList = {7537,7538,7539,7551,7552}
x301424_g_BuffList={7709,7710,7711,0}
x301424_g_PosX = 71
x301424_g_PosZ = 207  
function x301424_HaveAccepted(sceneId, selfId)

	for i, item in x301424_g_MissionList do
		if IsHaveQuestNM( sceneId, selfId, item ) > 0 then
			return 1
		end
	end
	return -1
end

function x301424_RandBuff(sceneId, selfId)
	local kindIndex = random(1, 100);
	if kindIndex >= 1 and kindIndex <= 40 then
		return x301424_g_BuffList[1]
	elseif kindIndex > 40 and kindIndex <= 80 then
		return x301424_g_BuffList[4]
	elseif kindIndex > 80 and kindIndex <= 90 then
		return x301424_g_BuffList[2]
	elseif kindIndex > 90 and kindIndex <= 100 then
		return x301424_g_BuffList[3]
	end
end


--���ɺ�����ʼ************************************************************************
function 	x301424_OnCreate(sceneId,growPointType,x,y)
	local ItemBoxId = ItemBoxEnterScene(x, y, x301424_g_GrowpointId, sceneId, 0, -1)
	SetGrowPointObjID(sceneId,x301424_g_GrowpointId,x, y,ItemBoxId)
end
--���ɺ�������**********************************************************************


--��ǰ������ʼ&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
function	 x301424_OnOpen(sceneId,selfId,targetId)
		--PrintStr("x301424_OnOpen")
    return x301424_OnOpenItemBox(sceneId, selfId, targetId, x301424_g_GrowpointId, -1)	
end
--��ǰ��������&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


--���պ�����ʼ########################################################################
function	 x301424_OnRecycle(sceneId,selfId,targetId)
	return   x301424_OnGuildRecycle( sceneId, selfId, targetId, x301424_g_GrowpointId, -1 )
end
--���պ�������########################################################################



--�򿪺�����ʼ@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
function	x301424_OnProcOver(sceneId,selfId,targetId)
	--CallScriptFunction( MISSION_SCRIPT, "OnProcOver", sceneId, selfId, targetId )
end
--�򿪺�������@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@



function	x301424_OpenCheck(sceneId,selfId,AbilityId,AblityLevel)
	--CallScriptFunction( MISSION_SCRIPT, "OpenCheck", sceneId, selfId, targetId )
end


--**********************************************************************
--�ɼ�����
--��ItemBox
function x301424_OnOpenItemBox( sceneId, selfId, targetId, gpType, needItemID )
	
	if x301424_HaveAccepted(sceneId, selfId) < 0 then
		BeginQuestEvent(sceneId)
		AddQuestText(sceneId,"��û�������塿�������������עϵͳ���档")
		EndQuestEvent(sceneId)
		DispatchQuestTips(sceneId,selfId)
		return -31
	end
	    
	return 0
end

--����
function x301424_OnGuildRecycle( sceneId, selfId, targetId, gpType, needItemID )
	
	local buff = x301424_RandBuff(sceneId, selfId)
	if buff == 0 then
		SetPos(sceneId, selfId, x301424_g_PosX, x301424_g_PosZ)
		return 1
	end
	
	if buff == 7709 then
		if IsHaveSpecificImpact( sceneId, selfId, 7710 ) > 0 then
			CancelSpecificImpact(sceneId, selfId,7710)
		end
		SendSpecificImpactToUnit(sceneId, selfId, selfId, selfId, buff, 0)
		BeginQuestEvent(sceneId);
		AddQuestText(sceneId, "�����ƶ��ٶ�15��");
		EndQuestEvent();
		DispatchQuestTips(sceneId, selfId);
		return 1
	end

	if buff == 7710 then
		if IsHaveSpecificImpact( sceneId, selfId, 7709 ) > 0 then
			CancelSpecificImpact(sceneId, selfId,7709)
		end
		SendSpecificImpactToUnit(sceneId, selfId, selfId, selfId, buff, 0)
		BeginQuestEvent(sceneId);
		AddQuestText(sceneId, "�����ƶ��ٶ�5��");
		EndQuestEvent();
		DispatchQuestTips(sceneId, selfId);
		return 1
	end
	
	if buff == 7711 then
		if IsHaveSpecificImpact( sceneId, selfId, buff ) > 0 then
			CancelSpecificImpact(sceneId, selfId,buff)
		end
		SendSpecificImpactToUnit(sceneId, selfId, selfId, selfId, buff, 1000)
		BeginQuestEvent(sceneId);
		AddQuestText(sceneId, "ѣ��2��");
		EndQuestEvent();
		DispatchQuestTips(sceneId, selfId);
		return 1
	end
	return 0
end