
------------------------------------------------------------------------------------------
--һ����Ʒ��Ĭ�Ͻű�
--"�����̽�����Ųʣ�"

--�ű���

x404906_g_scriptId 		= 404906 

x404906_g_LevelMin		=	20
--��Ҫ�ĵȼ�


--**********************************
--�¼��������
--**********************************
function x404906_ProcEventEntry( sceneId, selfId, bagIndex )
-- ����Ҫ����ӿڣ���Ҫ�����պ���
end

--**********************************
--�����Ʒ��ʹ�ù����Ƿ������ڼ��ܣ�
--ϵͳ����ִ�п�ʼʱ�����������ķ���ֵ���������ʧ������Ժ�������Ƽ��ܵ�ִ�С�
--����1���������Ƶ���Ʒ�����Լ������Ƽ��ܵ�ִ�У�����0�����Ժ���Ĳ�����
--**********************************
function x404906_IsSkillLikeScript( sceneId, selfId)
	return 1; --����ű���Ҫ����֧��
end

--**********************************
--ֱ��ȡ��Ч����
--ϵͳ��ֱ�ӵ�������ӿڣ���������������ķ���ֵȷ���Ժ�������Ƿ�ִ�С�
--����1���Ѿ�ȡ����ӦЧ��������ִ�к�������������0��û�м�⵽���Ч��������ִ�С�
--**********************************
function x404906_CancelImpacts( sceneId, selfId )
	return 0; --����Ҫ����ӿڣ���Ҫ�����պ���,����ʼ�շ���0��
end

--**********************************
--���������ڣ�
--ϵͳ���ڼ��ܼ���ʱ����������ӿڣ���������������ķ���ֵȷ���Ժ�������Ƿ�ִ�С�
--����1���������ͨ�������Լ���ִ�У�����0���������ʧ�ܣ��жϺ���ִ�С�
--**********************************
function x404906_OnConditionCheck( sceneId, selfId )
	
	--У��ʹ�õ���Ʒ
	
	if(1~=VerifyUsedItem(sceneId, selfId)) then
		return 0
	end
	return 1; --����Ҫ�κ�����������ʼ�շ���1��
end

--**********************************
--���ļ�⼰������ڣ�
--ϵͳ���ڼ������ĵ�ʱ����������ӿڣ���������������ķ���ֵȷ���Ժ�������Ƿ�ִ�С�
--����1�����Ĵ���ͨ�������Լ���ִ�У�����0�����ļ��ʧ�ܣ��жϺ���ִ�С�
--ע�⣺�ⲻ�⸺�����ĵļ��Ҳ�������ĵ�ִ�С�
--**********************************
function x404906_OnDeplete( sceneId, selfId )
	
	
	--if(DepletingUsedItem(sceneId, selfId)) == 1 then
	--	return 1;
	--end
	--return 0;
	
	return 1;
end

--**********************************
--ֻ��ִ��һ����ڣ�
--������˲�����ܻ���������ɺ��������ӿڣ������������Ҹ��������������ʱ�򣩣�������
--����Ҳ����������ɺ��������ӿڣ����ܵ�һ��ʼ�����ĳɹ�ִ��֮�󣩡�
--����1�������ɹ�������0������ʧ�ܡ�
--ע�������Ǽ�����Чһ�ε����
--**********************************
function x404906_OnActivateOnce( sceneId, selfId, impactId )
	
	
	if x404906_IsOneWeekPassed(sceneId, selfId)<=0 then
		BeginQuestEvent(sceneId)
		AddQuestText(sceneId,"��������һ��ֻ�ܿ���һ��,������������������")
		EndQuestEvent(sceneId)
		DispatchQuestTips(sceneId,selfId)
		return
	end

	BeginAddItem(sceneId)
	AddBindItem( sceneId, 11000504, 1 )
	
	local ret = EndAddItem(sceneId,selfId)

	if ret>0 then

		if(DepletingUsedItem(sceneId, selfId)) == 1 then
			BeginQuestEvent(sceneId)
			AddQuestText(sceneId,"�������̽��������˽�����Ʒ")
			EndQuestEvent(sceneId)
			DispatchQuestTips(sceneId,selfId)
			AddItemListToPlayer(sceneId,selfId)

			x404906_SaveDoTime(sceneId, selfId)
			
		else
			BeginQuestEvent(sceneId)
			AddQuestText(sceneId,"ʹ����Ʒ�쳣��")
			EndQuestEvent(sceneId)
			DispatchQuestTips(sceneId,selfId)
		end
	else
		BeginQuestEvent(sceneId)
		AddQuestText(sceneId,"�����������޷��õ���Ʒ��")
		EndQuestEvent(sceneId)
		DispatchQuestTips(sceneId,selfId)
	end

end

--**********************************
--��������������ڣ�
--�������ܻ���ÿ����������ʱ��������ӿڡ�
--���أ�1�����´�������0���ж�������
--ע�������Ǽ�����Чһ�ε����
--**********************************
function x404906_OnActivateEachTick( sceneId, selfId)
	return 1; --���������Խű�, ֻ�����պ���.
end


---------------------------------------------------------------------------------------------------
--�ж�ʱ����û�е�
---------------------------------------------------------------------------------------------------
function x404906_IsOneWeekPassed(sceneId, selfId)

	
	local curWeek = GetWeekIndex();

	local lastWeek = GetQuestData(sceneId, selfId, MD_WEIWANG_XUANCHAI_WEEK[1], MD_WEIWANG_XUANCHAI_WEEK[2],MD_WEIWANG_XUANCHAI_WEEK[3] );

	if curWeek ~= lastWeek then
		return 1
	else
		return 0;
	end
	
	
	

end

---------------------------------------------------------------------------------------------------
--����do ��ʱ��
---------------------------------------------------------------------------------------------------
function x404906_SaveDoTime(sceneId, selfId)

	local curWeek = GetWeekIndex();
	SetQuestData(sceneId, selfId, MD_WEIWANG_XUANCHAI_WEEK[1], MD_WEIWANG_XUANCHAI_WEEK[2], MD_WEIWANG_XUANCHAI_WEEK[3], curWeek)
	
	
	
end