
--��ȸս������

x310114_g_ScriptId              			= 310114
x310114_g_Leader_Index          			= 5
x310114_g_MissionName           			="�����ҡ���ս��ȸ֮��"
x310114_g_MissionName1          			="�����ҡ���ȸս����ѯ"
x310114_g_NeedMoney             			= 50*1000
x310114_g_GuildLevel            			= 1
x310114_g_GuildMemberNum       				= 1


x310114_g_ZhuQue_Signup                  	= 100     --����
x310114_g_ZhuQue_WaitBattle              	= 101     --�����������ȴ���ȸս��
x310114_g_ZhuQue_BeginBattle             	= 102     --��ʼ��ȸս��
x310114_g_ZhuQue_EndBattle               	= 103     --������ȸս��

x310114_g_Leader_Index                      =  5


----------------------------------------------------------------------------------------------
--ö��
----------------------------------------------------------------------------------------------
function x310114_ProcEnumEvent( sceneId, selfId, targetId, MissionId )

	--�Ϸ�����
    if GetMergeDBToggle() == 1 then
        return
    end
    
  --�������ݿ⿪��
	local UpdateDBflag = GetUpdateDBBeforeToggle()
	if UpdateDBflag==1 then
		return
	end
    
    --���Է�����
    --if GetTestServerToggle() == 0 then
    --    return
    --end

    --ս���Լ��Ŀ���
    if GetBattleSceneToggle( BATTLESCENE_TYPE_ZHUQUEBATTLELITE ) == 0 then
        return
    end

    --�����
    local nGuildId = GetGuildID(sceneId,selfId )
    if nGuildId < 0 then
        return
    end
    
    --����Ƿ��й���
    local nHumanGUID = GetPlayerGUID( sceneId,selfId )
    local nCountryId,nKingGUID,szKingName,nCountryGold = GetCountrySimpleData( sceneId,selfId )
    if nKingGUID <= 0 or nHumanGUID == nKingGUID then
    	return    	
    end
    
    --�������ѡ��
    local nBattleStatus = CallScriptFunction(310113,"GetBattleState",sceneId)
	if nBattleStatus == x310114_g_ZhuQue_Signup then
		AddQuestNumText(sceneId,x310114_g_ScriptId,x310114_g_MissionName1,3,x310114_g_ScriptId);
	end

    --����Ƿ��ǰ���
    local pos = GetGuildOfficial(sceneId, selfId)
	if pos ~= x310114_g_Leader_Index then
        return
    end
    
    --����Ƿ�������
    if IsSpecialCountryOffical(sceneId,selfId,COUNTRY_POSITION_QINGLONG_LEADER) == 1 then
    	return
    end
    
    --����Ƿ�����ȸ
    if IsSpecialCountryOffical(sceneId,selfId,COUNTRY_POSITION_ZHUQUE_LEADER) == 1 then
    	return
    end
    
    --����Ƿ��ǹ���
    if IsSpecialCountryOffical(sceneId,selfId,COUNTRY_POSITION_KING) == 1 then
    	return
    end
    
    
	--�������ѡ��
	-- if nBattleStatus == x310114_g_ZhuQue_Signup then
    	AddQuestNumText(sceneId,x310114_g_ScriptId,x310114_g_MissionName,3);
    -- end
    
end

----------------------------------------------------------------------------------------------
--Ĭ���¼�
----------------------------------------------------------------------------------------------
function x310114_ProcEventEntry( sceneId, selfId, targetId,scriptId,idExt )

    if idExt == 0 then
    
    	local nBattleStatus = CallScriptFunction(310113,"GetBattleState",sceneId)
    	if nBattleStatus ~= x310114_g_ZhuQue_Signup then
    		--�û����
    		BeginQuestEvent(sceneId)
			AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ��ܱ�Ǹ������ÿ���յ�20��25~20��40֮��������ȸս����");
			EndQuestEvent(sceneId)
			DispatchQuestEventList(sceneId,selfId,targetId)
    		return
    	end

        --��ѯ���
        local nGuildId = GetGuildID( sceneId,selfId )
        GameBattleQuerySignupState( sceneId,selfId,0 )
        SetPlayerRuntimeData(sceneId,selfId,RD_COMMON_NPCID,targetId)

    elseif idExt == 1 then

        --�Ƿ��ǰ���
        local pos = GetGuildOfficial(sceneId, selfId)
        if pos ~= x310114_g_Leader_Index then
            BeginQuestEvent(sceneId)
            AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ��ܱ�Ǹ���������ǰ����޷������Ϊ��ȸ֮����");
            EndQuestEvent(sceneId)
            DispatchQuestEventList(sceneId,selfId,targetId)
            return
        end

        local nGuildId = GetGuildID(sceneId,selfId )
        local nLevel,nMemberNum,nExp = GetGuildSimpleData( nGuildId )

        --���ȼ�������
        if nLevel < x310114_g_GuildLevel then
            BeginQuestEvent(sceneId)
            AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ����ȼ����㣲����");
            EndQuestEvent(sceneId)
            DispatchQuestEventList(sceneId,selfId,targetId)
            return
        end

        --����Ա�����Ƿ�����
        if nMemberNum < x310114_g_GuildMemberNum then
            BeginQuestEvent(sceneId)
            AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ�����Ա����30�ˣ�");
            EndQuestEvent(sceneId)
            DispatchQuestEventList(sceneId,selfId,targetId)
            return
        end
        
        --����Ƿ�������
	    if IsSpecialCountryOffical(sceneId,selfId,COUNTRY_POSITION_QINGLONG_LEADER) == 1 then
	    	return
	    end
	    
	    --����Ƿ�����ȸ
	    if IsSpecialCountryOffical(sceneId,selfId,COUNTRY_POSITION_ZHUQUE_LEADER) == 1 then
	    	return
	    end
	    
	    --����Ƿ��ǹ���
	    if IsSpecialCountryOffical(sceneId,selfId,COUNTRY_POSITION_KING) == 1 then
	    	return
	    end
        
        --����Ƿ��Ǿݵ�
	    local nLairdM1,_,_,nLairdDate = GetGuildLairdSceneId(nGuildId) 
	    if nLairdM1 > 0 then
	    	BeginQuestEvent(sceneId)
            AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ����������ز������룡");
            EndQuestEvent(sceneId)
            DispatchQuestEventList(sceneId,selfId,targetId)
	    	return 
	    end

        --���϶�ͨ������������
        GameBattleExecuteSignup( sceneId,selfId, 0 )
        SetPlayerRuntimeData(sceneId,selfId,RD_COMMON_NPCID,targetId)

    elseif idExt == x310114_g_ScriptId then    	
    	
    	--��Բ�ѯ
    	GameBattleQueryMatchState(sceneId,selfId,0)
    	SetPlayerRuntimeData(sceneId,selfId,RD_COMMON_NPCID,targetId)
    	
    end

    return 1

end

----------------------------------------------------------------------------------------------
--��ѯ��ȸս������״̬��Ӧ
----------------------------------------------------------------------------------------------
function x310114_OnQueryZhuQueBattleSignupState( sceneId,selfId,nResult,nGuildA,nGuildB )
    
    local targetId = GetPlayerRuntimeData(sceneId,selfId,RD_COMMON_NPCID)
    if nResult == 0 then
    	--OK,��ѯ״̬�������������
    	
    elseif nResult == 100 then
    	
    	--�û����
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ��ܱ�Ǹ������ÿ���յ�20��25~20��40֮��������ȸս����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		
    elseif nResult == 101 then
    
    	--���û�б���
        x310114_ZhuQueBattleSignup( sceneId,selfId,targetId,nGuildA,nGuildB )
        
    elseif nResult == 102 then
    	
    	--�Ѿ�����
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t�ܱ�Ǹ���Ѿ�����������ս��ȸ֮���������޷��ٴ���ս��ȸ֮����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		
    elseif nResult == 103 then
    	
    	--û�а��
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ������а�����������ս��");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 104 then
    
    	--���ȼ�����
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ����ȼ�����2����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 105 then
    
    	--���ǰ���
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ������ǰ����ſ������룡");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 106 then
    	
    	--�����������
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ�����Ա����30�ˣ�");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 107 then
    
    	--���������,������������
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ������������");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 108 then
    
    	--���������,�����ǹ�����
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ������ǹ����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 109 then
    	
    	--���ʵ������
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ����ʵ����������Ѿ�����İ��ʵ����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		
	elseif nResult == 110 then
    	
    	--��������
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ������Ѿ�����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		
	elseif nResult == 111 then
		
		--������������
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ����������ػ���");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		
    else
    
    	--δ֪����
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ�δ֪ԭ�򣬿�����ȸս���������Ѵ����ߣ�");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    end

    return 1

end

----------------------------------------------------------------------------------------------
--��ս����
----------------------------------------------------------------------------------------------
function x310114_ZhuQueBattleSignup( sceneId,selfId,targetId ,nGuildA, nGuildB)

	
	SetPlayerRuntimeData(sceneId,selfId,RD_COMMON_NPCID,targetId)
    
    --�Ƿ��ǰ���
    local pos = GetGuildOfficial(sceneId, selfId)
	if pos ~= x310114_g_Leader_Index then
        BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t�ܱ�Ǹ���������ǰ����޷�������ս��ȸ֮����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		return 0
	end

    local nGuildId = GetGuildID(sceneId,selfId )
    local nLevel,nMemberNum,nExp = GetGuildSimpleData( nGuildId )

    --���ȼ�������
    if nLevel < x310114_g_GuildLevel then
        BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ����ȼ�����2����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		return 0
    end

    --����Ա�����Ƿ�����
    if nMemberNum < x310114_g_GuildMemberNum then
        BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ�����Ա����30�ˣ�");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		return 0
    end
    
    
    --����Ƿ�������
    if IsSpecialCountryOffical(sceneId,selfId,COUNTRY_POSITION_QINGLONG_LEADER) == 1 then
    	return
    end
    
    --����Ƿ�����ȸ
    if IsSpecialCountryOffical(sceneId,selfId,COUNTRY_POSITION_ZHUQUE_LEADER) == 1 then
    	return
    end
    
    --����Ƿ��ǹ���
    if IsSpecialCountryOffical(sceneId,selfId,COUNTRY_POSITION_KING) == 1 then
    	return
    end
    
    --����Ƿ��Ǿݵ�
    local nLairdM1,_,_,nLairdDate = GetGuildLairdSceneId(nGuildId) 
    if nLairdM1 > 0 then
    	BeginQuestEvent(sceneId)
        AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ����������ز������룡");
        EndQuestEvent(sceneId)
        DispatchQuestEventList(sceneId,selfId,targetId)
    	return 
    end

    --�Ƿ�����ȸս������ʱ��
    local nBattleStatus = CallScriptFunction(310113,"GetBattleState",sceneId)
	if nBattleStatus ~= x310114_g_ZhuQue_Signup then
 		BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ��ܱ�Ǹ������ÿ���յ�20��25~20��40֮��������ȸս����");
		EndQuestEvent(sceneId)
        DispatchQuestEventList(sceneId,selfId,targetId)
		return 0
	end
	
	--�ָ����������ȸ�����д���
	local msg = ""
	local nCountryGuildId = CountryGetOfficerGUID(GetCurCountry(sceneId,selfId),1)
	if nCountryGuildId <= 0 then

		
		--û����ȸ�ػ������
		if nGuildA >= 0 and nGuildB >= 0 then
			
			--����Ѿ�������ս��
			local strGuildA = GetGuildName( nGuildA )
			local strGuildB = GetGuildName( nGuildB )
			msg = format("#Y�����ҡ���ս��ȸ֮��#W\n \n#G��ǰû����ȸ�ػ����\n�����᣺#R%s\n#G�����᣺#R%s\n \n��ȷ��Ҫ������ս��",strGuildA,strGuildB)
			
		elseif nGuildA >= 0 and nGuildB < 0 then
			--���ֻ��һ����ս
			local strGuildA = GetGuildName( nGuildA )
			msg = format("#Y�����ҡ���ս��ȸ֮��#W\n \n#G��ǰû����ȸ�ػ����\n�����᣺#R%s\n#G�����᣺#W������\n \n��ȷ��Ҫ������ս��",strGuildA)
		else
			--���û������ս
			msg = "#Y�����ҡ���ս��ȸ֮��#W\n \n#G��ǰû����ȸ�ػ����\n�����᣺#W������\n#G�����᣺#W������\n \n��ȷ��Ҫ������ս��"
		end
		
	else
		--�������ػ������
		if nGuildB >= 0 then
			--����Ѿ�������ս��
			
			local strGuildA = GetGuildName( nGuildA )
			local strGuildB = GetGuildName( nGuildB )
			msg = format("#Y�����ҡ���ս��ȸ֮��#W\n \n#G��ȸ�ػ����:#R%s#r#G������:#R%s\n \n��ȷ��Ҫ������ս��",strGuildA,strGuildB)
			
		else
			--���û������ս
			local strGuildA = GetGuildName( nGuildA )
			msg = format("#Y�����ҡ���ս��ȸ֮��#W\n \n#G��ȸ�ػ����:#R%s#r#G�����᣺#W������\n \n��ȷ��Ҫ������ս��",strGuildA)
		end
	end

    BeginQuestEvent(sceneId)
    AddQuestText(sceneId, msg);
    AddQuestNumText(sceneId,x310114_g_ScriptId,"ȷ��..",3,1);
    EndQuestEvent(sceneId)
    DispatchQuestEventList(sceneId,selfId,targetId)

    return 1
end

function x310114_ProcAcceptCheck( sceneId, selfId, NPCId )
    return 1
end

----------------------------------------------------------------------------------------------
--����
----------------------------------------------------------------------------------------------
function x310114_ProcAccept( sceneId, selfId )
end


----------------------------------------------------------------------------------------------
--������
----------------------------------------------------------------------------------------------
function x310114_OnZhuQueBattleSignupResult( sceneId,selfId,nResult,nGuildA, nGuildB,nSignupCount)

	local targetId = GetPlayerRuntimeData(sceneId,selfId,RD_COMMON_NPCID)
	
	if nResult == 0 then

		local strMsg = ""
		local strMsg1= ""
		if nSignupCount > 0 then
        	strMsg = format("#Y�����ҡ���ս��ȸ֮��#W#r\t��ϲ�����ɹ�������ȸս����ĿǰΪֹ����%d����ᱨ���μӡ�",nSignupCount)
        	strMsg1= format("��ϲ�����ɹ�������ȸս����ĿǰΪֹ����%d����ᱨ���μӡ�",nSignupCount)
        else
        	strMsg = "#Y�����ҡ���ս��ȸ֮��#W#r\t��ϲ�����ɹ�������ȸս����Ŀǰ���˱���ỹû��������ᱨ���μӡ�"
			strMsg1= "��ϲ�����ɹ�������ȸս����Ŀǰ���˱���ỹû��������ᱨ���μӡ�"
        end
	
    	local nGuildId = GetGuildID(sceneId,selfId )        
       
        LuaAllScenceM2Guild(sceneId,strMsg1,nGuildId,3,1)
        LuaAllScenceM2Guild(sceneId,strMsg1,nGuildId,2,1)

        BeginQuestEvent(sceneId)
		AddQuestText(sceneId, strMsg);
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 100 then
    	
    	--�û����
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ��ܱ�Ǹ������ÿ���յ�20��25~20��40֮��������ȸս����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		
    elseif nResult == 101 then
    
    	--���û�б���
        x310114_ZhuQueBattleSignup( sceneId,selfId,targetId,nGuildA,nGuildB )
        
    elseif nResult == 102 then
    	
    	--�Ѿ�����
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t�ܱ�Ǹ���Ѿ�����������ս��ȸ֮���������޷��ٴ���ս��ȸ֮����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		
    elseif nResult == 103 then
    	
    	--û�а��
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ������а�����������ս��ȸ֮����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 104 then
    
    	--���ȼ�����
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ����ȼ�����2����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 105 then
    
    	--���ǰ���
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ������ǰ����ſ������룡");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 106 then
    	
    	--�����������
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ�����Ա����30�ˣ�");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 107 then
    
    	--���������,������������
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ������������");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 108 then
    
    	--���������,�����ǹ�����
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ������ǹ����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    	
    elseif nResult == 109 then
    	
    	--���ʵ������
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ����ʵ����������Ѿ�����İ��ʵ����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		
	elseif nResult == 110 then
    	
    	--��������
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ������Ѿ�����");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		
	elseif nResult == 111 then
		
		--������������
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ����������ػ���");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
		
    elseif nResult == -1 then
    
    	--δ֪����
    	BeginQuestEvent(sceneId)
		AddQuestText(sceneId, "#Y�����ҡ���ս��ȸ֮��#W#r\t����ʧ�ܣ�δ֪ԭ�򣬿�����ȸս���������Ѵ����ߣ�");
		EndQuestEvent(sceneId)
		DispatchQuestEventList(sceneId,selfId,targetId)
    end

    return 1

end


----------------------------------------------------------------------------------------------
--��ѯ���
----------------------------------------------------------------------------------------------
function x310114_OnQueryCurrentGuild(sceneId,selfId,nGuildA,nGuildB,nCount)
	
	local targetId = GetPlayerRuntimeData(sceneId,selfId,RD_COMMON_NPCID)
	
	--�ָ����������ȸ�����д���
	local msg = ""
	local nCountryGuildId = CountryGetOfficerGUID(GetCurCountry(sceneId,selfId),1)
	if nCountryGuildId <= 0 then
		
		--û����ȸ�ػ������
		if nGuildA >= 0 and nGuildB >= 0 then
			
			--����Ѿ�������ս��
			local strGuildA = GetGuildName( nGuildA )
			local strGuildB = GetGuildName( nGuildB )
			msg = format("#Y�����ҡ���ȸս����ѯ#W\n \n#G��ǰû����ȸ�ػ����\n#G�����᣺#R%s\n#G�����᣺#R%s",strGuildA,strGuildB)
			
		elseif nGuildA >= 0 and nGuildB < 0 then
			--���ֻ��һ����ս
			local strGuildA = GetGuildName( nGuildA )
			msg = format("#Y�����ҡ���ȸս����ѯ#W\n \n#G��ǰû����ȸ�ػ����\n#G�����᣺#R%s\n#G�����᣺#W������",strGuildA)
		else
			--���û������ս
			msg = "#Y�����ҡ���ȸս����ѯ#W\n \n#G��ǰû����ȸ�ػ����\n#G�����᣺#W������\n#G�����᣺#W������"
		end
		
	else
		--����ȸ�ػ������
		if nGuildB >= 0 then
			--����Ѿ�������ս��
			
			local strGuildA = GetGuildName( nGuildA )
			local strGuildB = GetGuildName( nGuildB )
			msg = format("#Y�����ҡ���ȸս����ѯ#W\n \n#G��ȸ�ػ����:#R%s\n#G������:#R%s",strGuildA,strGuildB)
			
		else
			local strGuildA = GetGuildName( nGuildA )
			--���û������ս
			msg = format("#Y�����ҡ���ȸս����ѯ#W\n \n#G��ȸ�ػ����:#R%s\n#G�����᣺#W������",strGuildA)
		end
	end

    BeginQuestEvent(sceneId)
    AddQuestText(sceneId, msg);
    EndQuestEvent(sceneId)
    DispatchQuestEventList(sceneId,selfId,targetId)

    return 1
    	
end

----------------------------------------------------------------------------------------------
--���������洦��
----------------------------------------------------------------------------------------------
function x310114_OnSignupInstead(sceneId,nCountryId,nGuildA,nGuildB)
	
	local strGuildB = GetGuildName(nGuildB)
	local msg = format("#R�ܱ�Ǹ����ȸս���ı��������ѱ����ʵ�����ߵ�%s��ᶥ��",strGuildB)  
	
	local nHumanCount = GetScenePlayerCount( sceneId )
    local i = 0
    for i = 0,nHumanCount-1 do
        local objId = GetScenePlayerObjId( sceneId,i)
        if objId >= 0 and IsPlayerStateNormal(sceneId,objId) == 1 then
        
            local nGuildId = GetGuildID( sceneId,objId )
            if nGuildId == nGuildA then
            	
            	
            	LuaScenceM2Player(sceneId,objId,msg,GetName(sceneId,objId),3,1)          	
            	LuaScenceM2Player(sceneId,objId,msg,GetName(sceneId,objId),2,1)
            end
        end
    end
	
end


----------------------------------------------------------------------------------------------
--����������
----------------------------------------------------------------------------------------------
function x310114_ProcAcceptCheck( sceneId, selfId, NPCId )
    return 1
end

----------------------------------------------------------------------------------------------
--����
----------------------------------------------------------------------------------------------
function x310114_ProcAccept( sceneId, selfId )
end

----------------------------------------------------------------------------------------------
--����
----------------------------------------------------------------------------------------------
function x310114_ProcQuestAbandon( sceneId, selfId, MissionId )
end

----------------------------------------------------------------------------------------------
--����
----------------------------------------------------------------------------------------------
function x310114_OnContinue( sceneId, selfId, targetId )
end

----------------------------------------------------------------------------------------------
--����Ƿ�����ύ
----------------------------------------------------------------------------------------------
function x310114_CheckSubmit( sceneId, selfId )
end

----------------------------------------------------------------------------------------------
--�ύ
----------------------------------------------------------------------------------------------
function x310114_ProcQuestSubmit( sceneId, selfId, targetId,selectRadioId, MissionId )
end

----------------------------------------------------------------------------------------------
--ɱ����������
----------------------------------------------------------------------------------------------
function x310114_ProcQuestObjectKilled( sceneId, selfId, objdataId, objId, MissionId )
end

----------------------------------------------------------------------------------------------
--���������¼�
----------------------------------------------------------------------------------------------
function x310114_ProcAreaEntered( sceneId, selfId, zoneId, MissionId )
end

----------------------------------------------------------------------------------------------
--���߸ı�
----------------------------------------------------------------------------------------------
function x310114_ProcQuestItemChanged( sceneId, selfId, itemdataId, MissionId )
end