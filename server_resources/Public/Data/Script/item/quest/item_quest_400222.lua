
x400222_g_scriptId = 400222 
x400222_g_Impact1 = 3001 --Ч��ID�������Ӻ���

function x400222_ProcEventEntry( sceneId, selfId, bagIndex )
--print(111)
	return CallScriptFunction( MISSION_SCRIPT, "PositionUseItem", sceneId, selfId, bagIndex, x400222_g_Impact1 )
end

function x400222_IsSkillLikeScript( sceneId, selfId)
	return 0; --����ű���Ҫ����֧��    
	--����0���������ProcEventEntry�߼�
	--����1����������߼�
end

function x400222_CancelImpacts( sceneId, selfId )
	return 0; --����Ҫ����ӿڣ���Ҫ�����պ���,����ʼ�շ���0��
end

function x400222_OnConditionCheck( sceneId, selfId )
	return 0
end

function x400222_OnDeplete( sceneId, selfId )
	return 0
end

function x400222_OnActivateOnce( sceneId, selfId )
end

function x400222_OnActivateEachTick( sceneId, selfId)
	return 1; --���������Խű�, ֻ�����պ���.
end