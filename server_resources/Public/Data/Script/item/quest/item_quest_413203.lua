
x413203_g_scriptId = 413203 
x413203_g_Impact1 = 8313 --Ч��ID�������Ӻ���

function x413203_ProcEventEntry( sceneId, selfId, bagIndex )
	return CallScriptFunction( MISSION_SCRIPT, "PositionUseItem", sceneId, selfId, bagIndex, x413203_g_Impact1 )
end

function x413203_IsSkillLikeScript( sceneId, selfId)
	return 0; --����ű���Ҫ����֧��    
	--����0���������ProcEventEntry�߼�
	--����1����������߼�

end

function x413203_CancelImpacts( sceneId, selfId )
	return 0; --����Ҫ����ӿڣ���Ҫ�����պ���,����ʼ�շ���0��

end

function x413203_OnConditionCheck( sceneId, selfId )
	return 0
end

function x413203_OnDeplete( sceneId, selfId )
	return 0
end

function x413203_OnActivateOnce( sceneId, selfId )
end

function x413203_OnActivateEachTick( sceneId, selfId)
	return 1; --���������Խű�, ֻ�����պ���.
end