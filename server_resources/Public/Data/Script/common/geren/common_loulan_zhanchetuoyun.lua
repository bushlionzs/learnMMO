--MisDescBegin

x300762_g_ScriptId 	  = 300762
x300762_g_MissionKind 				= 	1                       --��������
x300762_g_Mis_Count   = 1--ÿ������Ĵ���
x300762_g_ExtTarget					=	{ {type=20,n=1,target="ս��"} }
x300762_g_MissionName = "����ս��"

--MisDescEnd

x300762_CostGold		= 1000

x300762_g_MissionId = 7031
x300762_g_DestPosX = 162
x300762_g_DestPosZ = 165
				

function x300762_ProcEnumEvent( sceneId, selfId, targetId, MissionId )

	AddQuestNumText(sceneId,x300762_g_ScriptId,x300762_g_MissionName,3,-1)

end




--�������ť������������ִ�д˽ű�
function x300762_ProcEventEntry( sceneId, selfId, targetId, MissionId,nExtIdx )

	-- local bRet = x300762_IsCanAccept( sceneId, selfId )

	-- if bRet == 1 then
		BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "\t������ȡս�����ͺ󣬲�����#Y1���𿨣��ֽ�#W���Ǿ��ܽ�ս�����͵�����Ŀ�ĵغܽ��ĵط���#r\tѡ���ֽ����ѣ������԰�B���򿪰������棬��#G����ʹ��#W#Y�ֽ�/��#W�����϶Թ����ɡ�");
		EndQuestEvent(sceneId)
		DispatchQuestInfo(sceneId, selfId, targetId, x300762_g_ScriptId,  -1);
		return
	-- end
	-- if GetGoldMode( sceneId, selfId ) == 1 then
				-- �ֽ�ģʽ
				-- if GetMoney(sceneId, selfId,2) < x300762_CostGold then
					-- BeginQuestEvent(sceneId)
					-- AddQuestText(sceneId, "�ܱ�Ǹ��������Я�����ֽ𲻹�֧���������˷��á�");
					-- EndQuestEvent(sceneId)
					-- DispatchQuestTips(sceneId,selfId)
					-- Msg2Player(sceneId,selfId,"�ܱ�Ǹ��������Я�����ֽ𲻹�֧���������˷��á�",8,2)
					-- return -2
				-- end
			-- else
				-- ��ģʽ
				-- if GetMoney(sceneId, selfId,3) < x300762_CostGold then
					-- BeginQuestEvent(sceneId)
					-- AddQuestText(sceneId, "�ܱ�Ǹ��������Я���Ľ𿨲���֧���������˷��á�");
					-- EndQuestEvent(sceneId)
					-- DispatchQuestTips(sceneId,selfId)
					-- Msg2Player(sceneId,selfId,"�ܱ�Ǹ��������Я���Ľ𿨲���֧���������˷��á�",8,2)	
					-- return -2
				-- end
			-- end

end

--����Ƿ��ܽ��ܴ�����
--����ֵ��-2--�ֽ𲻹�  -1--û��������� 0--�ڳ�������Ч����	1--���Խ�������
function x300762_IsCanAccept( sceneId, selfId )
	
	local bHaveMission = 0
	bHaveMission = IsHaveQuestNM(sceneId, selfId,x300762_g_MissionId )
	if bHaveMission > 0 then
		local valid = IsValidMyselfBus(sceneId, selfId,x300762_g_MissionId)
		if valid == 0 then
			BeginQuestEvent(sceneId)
			AddQuestText(sceneId, "�ܱ�Ǹ������ս��û�ڸ������޷�����,���ҵ�ս��������");
			EndQuestEvent(sceneId)
			DispatchQuestTips(sceneId,selfId)
			Msg2Player(sceneId,selfId,"�ܱ�Ǹ������ս��û�ڸ������޷�����,���ҵ�ս��������",8,2)
			return 0
		end
	


		--�õ�����ֽ�����ģʽ
		if GetGoldMode( sceneId, selfId ) == 1 then
			-- �ֽ�ģʽ
			if GetMoney(sceneId, selfId,2) < x300762_CostGold then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId, "�ܱ�Ǹ��������Я�����ֽ𲻹�֧���������˷��á�");
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				Msg2Player(sceneId,selfId,"�ܱ�Ǹ��������Я�����ֽ𲻹�֧���������˷��á�",8,2)	
				return -2
			end
		else 
			-- ��ģʽ
			if GetMoney(sceneId, selfId,3) < x300762_CostGold then
				BeginQuestEvent(sceneId)
				AddQuestText(sceneId, "�ܱ�Ǹ��������Я���Ľ𿨲���֧���������˷��á�");
				EndQuestEvent(sceneId)
				DispatchQuestTips(sceneId,selfId)
				Msg2Player(sceneId,selfId,"�ܱ�Ǹ��������Я���Ľ𿨲���֧���������˷��á�",8,2)	
				return -2
			end
		end

		return 1
					
	end

	-- if IsHaveQuestNM(sceneId, selfId,7001 ) > 0 then
		-- ��7001
		-- BeginQuestEvent(sceneId)
		-- AddQuestText(sceneId, "�ܱ�Ǹ������ֻ���ܿ�Խ���߾���ս���������⡣");
		-- EndQuestEvent(sceneId)
		-- DispatchQuestTips(sceneId,selfId)
		-- Msg2Player(sceneId,selfId,"�ܱ�Ǹ������ֻ���ܵ���#G��ת��#W��#G���ƹ�#W�������⡣",8,2)
	-- else
		--û���κιٸ�Ѻ��
		 BeginQuestEvent(sceneId)
		 AddQuestText(sceneId, "�ܱ�Ǹ������������ȡս�������ܽ������ˣ�");
		 EndQuestEvent(sceneId)
		 DispatchQuestTips(sceneId,selfId)
		 Msg2Player(sceneId,selfId,"�ܱ�Ǹ������������ȡս�������ܽ������ˣ�",8,2)
	-- end

	return -1;

end


--**********************************
--����
--**********************************
function x300762_ProcAcceptCheck( sceneId, selfId, targetId, MissionId )
				
	local bRet = x300762_IsCanAccept( sceneId, selfId )

	if bRet ~= 1 then
		return
	end

	--�õ�����ֽ�����ģʽ
	if GetGoldMode( sceneId, selfId ) == 1 then
		--�ֽ�ģʽ
		if CostMoney( sceneId, selfId, 2,x300762_CostGold,301 ) == -1 then
			BeginQuestEvent(sceneId)
			AddQuestText(sceneId, "�ܱ�Ǹ��������Я�����ֽ𲻹�֧���������˷��á�");
			EndQuestEvent(sceneId)
			DispatchQuestTips(sceneId,selfId)
			Msg2Player(sceneId,selfId,"�ܱ�Ǹ��������Я�����ֽ𲻹�֧���������˷��á�",8,2)	
			return
		end
	else 
		--��ģʽ
		if CostMoney( sceneId, selfId, 3,x300762_CostGold,301 ) == -1 then
			BeginQuestEvent(sceneId)
			AddQuestText(sceneId, "�ܱ�Ǹ��������Я���Ľ𿨲���֧���������˷��á�");
			EndQuestEvent(sceneId)
			DispatchQuestTips(sceneId,selfId)
			Msg2Player(sceneId,selfId,"�ܱ�Ǹ��������Я���Ľ𿨲���֧���������˷��á�",8,2)	
			return
		end
	end

	local InBus = IsBusMember(sceneId, selfId)

	if InBus ~= 1 then
		return
	end

	local BusId = GetBusId(sceneId, selfId)

	local bHaveMission = IsHaveQuestNM(sceneId, selfId,x300762_g_MissionId )
	if bHaveMission > 0 then

		if BusId ~=-1 then
			ReadyBusChangeScene(sceneId, 58,BusId)
			NewWorld(sceneId, selfId, 58, x300762_g_DestPosX, x300762_g_DestPosZ, x300762_g_ScriptId)	
			DeleteBus(sceneId, BusId,0)			
		end

		-- if BusId ~=-1 then
			-- SetBusPos(sceneId,BusId,x300762_g_DestPosX, x300762_g_DestPosZ)
			-- SetPos(sceneId,selfId,x300762_g_DestPosX, x300762_g_DestPosZ)		
		-- end
	end

end


--**********************************
--����
--**********************************
function x300762_ProcAccept( sceneId, selfId )	
end

--�������
--����1�����ɹ���0����������ʧ��
function x300762_ProcQuestSubmit( sceneId, selfId, targetId, selectId, MissionId )
end


--********************************************************************
--����
--********************************************************************
function x300762_ProcQuestAbandon( sceneId, selfId,MissionId )  	
end


function x300762_ProcQuestAttach( sceneId, selfId, npcId, npcGuid,misIndex, MissionId )
end

---------------------------------------------------------------------------------------------------
--������־ˢ��
---------------------------------------------------------------------------------------------------
function x300762_QuestLogRefresh( sceneId, selfId, MissionId)
end

function x300762_ProcQuestLogRefresh( sceneId, selfId, MissionId)

end



