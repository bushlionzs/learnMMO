--MisDescBegin

x300877_g_ScriptId 					= 	300877                  --�ű�ID
x300877_g_MissionId 				= 	7645                    --����ID

x300877_g_MissionKind 				= 	1                       --��������

x300877_g_MissionName				= 	"�����ˡ��ڽ�Ѻ��"                --��������(512)
x300877_g_MissionTarget				= 	"ɱ��@npc_72005"          --����Ŀ��
x300877_g_MissionInfo				= 	"\tɱ��@npc_72004��@npc_72005,�ҵ�@npc_137600"            --������Ϣ
x300877_g_ContinueInfo				= 	"@npc_137602һֱ�����Ұ������@npc_72004��@npc_72005����ȴһֱ�鲻������"  --���������Ϣ
x300877_g_MissionCompleted			= 	"@npc_137602�Ѿ����ż������ˣ����������ְ�Ԫ��ħ����������ˣ�����Ͽ�֪ͨ@npc_137400��"  --���������Ϣ
x300877_g_MissionHelp				=	""          --������ʾ��Ϣ

--Ŀ��
x300877_g_ExtTarget					=	{ {type=20,n=3,target="@npc_72005"} }


--����
x300877_g_ExpBonus					= 	1000                    	--���������� ����ֵ��0���ַ���Ϊ�գ���ʾû�и������
x300877_g_BonusItem					=	{}	--��������Ʒ

x300877_g_BonusMoney1               = 	0    --������������
x300877_g_BonusMoney2               =   0    --�������ǰ�����
x300877_g_BonusMoney3               =   0    --�������󶨽��
x300877_g_BonusMoney4               =   0  --��������͢����
x300877_g_BonusMoney5               =   0  --��������������
x300877_g_BonusMoney6               =   10  --����������

x300877_g_BonusChoiceItem           =   {}

--MisDescEnd


--������
x300877_g_SubMissionID				=	{ { MisID=7632,MissionName = "���ڽ𡿺ڽ�Ѻ��",LevelLess = 30,LevelMax = 40  }, 
										  {	MisID=7633,MissionName = "���ڽ𡿺ڽ�Ѻ��",LevelLess = 40,LevelMax = 50  },
										  {	MisID=7634,MissionName = "���ڽ𡿺ڽ�Ѻ��",LevelLess = 50,LevelMax = 60  },
										  {	MisID=7635,MissionName = "���ڽ𡿺ڽ�Ѻ��",LevelLess = 60,LevelMax = 70  },
										  {	MisID=7636,MissionName = "���ڽ𡿺ڽ�Ѻ��",LevelLess = 70,LevelMax = 80  },	
										  {	MisID=7637,MissionName = "���ڽ𡿺ڽ�Ѻ��",LevelLess = 80,LevelMax = 150  },											  
										}								


--**********************************
--�о��¼�
--**********************************
function x300877_ProcEnumEvent( sceneId, selfId, NPCId )

	local index = 0
	local level = GetLevel(sceneId,selfId)
	for i,itm in x300877_g_SubMissionID do
		if level >= itm.LevelLess and level < itm.LevelMax then
			index = i
			break
		end
	end
	
	local bHaveMission = x300877_IsHaveMission(sceneId, selfId)
	if bHaveMission <= 0 and index > 0 then
		local state = GetQuestStateNM(sceneId,selfId,NPCId,x300877_g_SubMissionID[index].MisID)
		AddQuestNumText(sceneId,x300877_g_SubMissionID[index].MisID,x300877_g_SubMissionID[index].MissionName,state,index);
	end
end


--**********************************
--������ں���
--**********************************
function x300877_ProcEventEntry(sceneId, selfId, NPCId, MissionId,nExtIdx)	--����������ִ�д˽ű�
end

--����Ƿ�������
function x300877_IsHaveMission( sceneId, selfId )

	local bHaveMission = 0
	for i,itm in x300877_g_SubMissionID do
		bHaveMission = IsHaveQuestNM(sceneId, selfId,itm.MisID );
		if bHaveMission > 0 then
			break
		end
	end
	
	return bHaveMission
end

---------------------------------------------------------------------------------------------------
--��������
---------------------------------------------------------------------------------------------------
function x300877_ProcQuestAbandon(sceneId, selfId, MissionId)
end


---------------------------------------------------------------------------------------------------
--�ύ���񣬲��������Ƿ���ɻ���������ʾ���������Ϣ�������������Ϣ
---------------------------------------------------------------------------------------------------
function x300877_ProcQuestSubmit(sceneId, selfId, NPCId, selectRadioId, MissionId)
end

function x300877_ProcQuestLogRefresh( sceneId, selfId, MissionId)
end
