--ֱ������ 80  12030373

------------------------------------------------------------------------------------------
--һ����Ʒ��Ĭ�Ͻű�

--�ű���

x418045_g_scriptId = 418045
x418045_g_ItemList = {}
x418045_g_LevelMin = 1	
x418045_g_LevelExp = {10,45,140,325,630,1085,1720,2565,3650,5005,6660,8645,10990,13725,16880,20485,24570,29165,34300,68956,95810,120000,150579,187824,201192,222841,259924,302592,310990,1251143,1320651,1390160,1469597,1539105,1618543,1697980,1777418,1856856,3813009,4855871,5085377,5302804,5544390,5785976,6039640,6293305,6559050,6824794,13939491,11446321,11908062,12381955,12880149,19672605,16027280,16646985,17278842,17935000,27364242,28024350,34189707,41700232,50847380,61985378,94430849,105471995,117777061,131488420,200133111,203278075,247418457,301083868,366318707,445604125,903251606,1006824457,1122079362,1250317003,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379,1392981379}	
x418045_g_LevelLim = 80	

--��Ҫ�ĵȼ�


--**********************************
--�¼��������
--**********************************
function x418045_ProcEventEntry( sceneId, selfId, bagIndex )
-- ����Ҫ����ӿڣ���Ҫ�����պ���
end

--**********************************
--�����Ʒ��ʹ�ù����Ƿ������ڼ��ܣ�
--ϵͳ����ִ�п�ʼʱ�����������ķ���ֵ���������ʧ������Ժ�������Ƽ��ܵ�ִ�С�
--����1���������Ƶ���Ʒ�����Լ������Ƽ��ܵ�ִ�У�����0�����Ժ���Ĳ�����
--**********************************
function x418045_IsSkillLikeScript( sceneId, selfId)
	return 1; --����ű���Ҫ����֧��
end

--**********************************
--ֱ��ȡ��Ч����
--ϵͳ��ֱ�ӵ�������ӿڣ���������������ķ���ֵȷ���Ժ�������Ƿ�ִ�С�
--����1���Ѿ�ȡ����ӦЧ��������ִ�к�������������0��û�м�⵽���Ч��������ִ�С�
--**********************************
function x418045_CancelImpacts( sceneId, selfId )
	return 0; --����Ҫ����ӿڣ���Ҫ�����պ���,����ʼ�շ���0��
end

--**********************************
--���������ڣ�
--ϵͳ���ڼ��ܼ���ʱ����������ӿڣ���������������ķ���ֵȷ���Ժ�������Ƿ�ִ�С�
--����1���������ͨ�������Լ���ִ�У�����0���������ʧ�ܣ��жϺ���ִ�С�
--**********************************
function x418045_OnConditionCheck( sceneId, selfId )
	
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
function x418045_OnDeplete( sceneId, selfId )
	
	
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
function x418045_OnActivateOnce( sceneId, selfId, impactId )

	local a = GetLevel(sceneId,selfId)
	local b = x418045_g_LevelLim
	local xExp = 0
	local flag = 0

	for i, item in x418045_g_LevelExp do
		
		local nLevel = GetLevel(sceneId,selfId)
		if i < nLevel then
			flag = 0
		else
			flag = 1
		end
		
		if flag == 1 then
			if nLevel < x418045_g_LevelLim then
				AddExpAutoLevelup(sceneId,selfId,item)
			else
				break
			end
		end
	end	

	if a < x418045_g_LevelLim then
		DepletingUsedItem(sceneId, selfId)
	end

end

--**********************************
--��������������ڣ�
--�������ܻ���ÿ����������ʱ��������ӿڡ�
--���أ�1�����´�������0���ж�������
--ע�������Ǽ�����Чһ�ε����
--**********************************
function x418045_OnActivateEachTick( sceneId, selfId)
	return 1; --���������Խű�, ֻ�����պ���.
end