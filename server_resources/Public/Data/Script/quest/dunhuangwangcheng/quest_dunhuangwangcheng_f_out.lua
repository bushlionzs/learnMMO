--�����سǽű�

x256010_g_ScriptId = 256010
x256010_g_ask ={"�ص�����","ȷ��"}
x256010_g_answer ={"\t��ȷ��Ҫ�뿪�˸���������������"}

function x256010_ProcEventEntry(sceneId, selfId, NPCId, MissionId,which)	--����������ִ�д˽ű�
		if which == 0 then
			BeginQuestEvent(sceneId)
			AddQuestNumText(sceneId, x256010_g_ScriptId, x256010_g_ask[1],0,1)
		  EndQuestEvent()
			DispatchQuestEventList(sceneId, selfId, NPCId)
		elseif which == 1 then
			BeginQuestEvent(sceneId)
			AddQuestText(sceneId,x256010_g_answer[1])
			AddQuestNumText(sceneId, x256010_g_ScriptId, x256010_g_ask[2],0,2)
		  EndQuestEvent()
			DispatchQuestEventList(sceneId, selfId, NPCId)
		elseif which == 2 then
			local selfcountry =GetCurCountry( sceneId, selfId)
            TimerCommand( sceneId, selfId, 1, 7010, 3, -1, -1, -1 )
            TimerCommand( sceneId, selfId, 2, 7010, 3, -1, -1, -1 )
			if selfcountry == 0 then
            NewWorld( sceneId, selfId, selfcountry * 100 + 50, 75 + random( 0, 5), 180 + random( 0, 5), 256010)
			elseif selfcountry ==1 then
			NewWorld( sceneId, selfId, selfcountry * 100 + 50, 75 + random( 0, 5), 180 + random( 0, 5), 256010)
			elseif selfcountry ==2 then
			NewWorld( sceneId, selfId, selfcountry * 100 + 50, 181 + random( 0, 5), 76 + random( 0, 5), 256010)
			elseif selfcountry ==3 then
			NewWorld( sceneId, selfId, selfcountry * 100 + 50, 181 + random( 0, 5), 76 + random( 0, 5), 256010)
			end
		end

	
	
end


--**********************************

--�о��¼�

--**********************************

function x256010_ProcEnumEvent(sceneId, selfId, NPCId, MissionId)
			AddQuestNumText(sceneId, x256010_g_ScriptId, x256010_g_ask[1],0,1)
end







