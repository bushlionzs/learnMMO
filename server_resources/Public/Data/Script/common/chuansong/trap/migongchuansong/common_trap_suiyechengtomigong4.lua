--������ɽ���Թ�4��
x552012_g_ScriptId = 552012
x552012_g_ask ={"ǰ���Թ��Ĳ�","ȷ��"}
x552012_g_answer ={"\t��ȷ��Ҫǰ�������Թ��Ĳ���"}

function x552012_ProcEventEntry(sceneId, selfId, NPCId, MissionId,which)	--����������ִ�д˽ű�
		if which == 0 then
			BeginQuestEvent(sceneId)
			AddQuestNumText(sceneId, x552012_g_ScriptId, x552012_g_ask[1],0,1)
		  EndQuestEvent()
			DispatchQuestEventList(sceneId, selfId, NPCId)
		elseif which == 1 then
			BeginQuestEvent(sceneId)
			AddQuestText(sceneId,x552012_g_answer[1])
			AddQuestNumText(sceneId, x552012_g_ScriptId, x552012_g_ask[2],0,2)
		  EndQuestEvent()
			DispatchQuestEventList(sceneId, selfId, NPCId)
		elseif which == 2 then
			local selfcountry =GetCurCountry( sceneId, selfId)
            --TimerCommand( sceneId, selfId, 1, 7010, 3, -1, -1, -1 )
           -- TimerCommand( sceneId, selfId, 2, 7010, 3, -1, -1, -1 )
            local BusId = GetBusId(sceneId, selfId)
			if selfcountry == 0 then
				if BusId ~=-1 then
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                    ReadyBusChangeScene(sceneId, 107,BusId)
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                   NewWorld( sceneId, selfId, selfcountry * 100 + 107, 191 + random( 0, 5), 82 + random( 0, 5), 552012)
					DeleteBus(sceneId, BusId,0)	
				else
            		if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                   NewWorld( sceneId, selfId, selfcountry * 100 + 107, 191 + random( 0, 5), 82 + random( 0, 5), 552012)
            	end
			elseif selfcountry ==1 then
				if BusId ~=-1 then
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                    ReadyBusChangeScene(sceneId, 207,BusId)
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                   NewWorld( sceneId, selfId, selfcountry * 100 + 107, 190 + random( 0, 5), 83 + random( 0, 5), 552012)
					DeleteBus(sceneId, BusId,0)	
				else
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                   NewWorld( sceneId, selfId, selfcountry * 100 + 107, 190 + random( 0, 5), 83 + random( 0, 5), 552012)
				end
			elseif selfcountry ==2 then
				if BusId ~=-1 then
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                    ReadyBusChangeScene(sceneId, 307,BusId)
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                   NewWorld( sceneId, selfId, selfcountry * 100 + 107, 110 + random( 0, 5), 76 + random( 0, 5), 552012)
					DeleteBus(sceneId, BusId,0)	
				else
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                   NewWorld( sceneId, selfId, selfcountry * 100 + 107, 110 + random( 0, 5), 76 + random( 0, 5), 552012)
				end
			elseif selfcountry ==3 then
				if BusId ~=-1 then
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                    ReadyBusChangeScene(sceneId, 407,BusId)
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                   NewWorld( sceneId, selfId, selfcountry * 100 + 107, 87 + random( 0, 5), 132 + random( 0, 5), 552012)
					DeleteBus(sceneId, BusId,0)	
				else
					if CallScriptFunction(COUNTRY_BATTLE_LITE_SCRIPT,"CheckCountryBattleLiteSceneTrans",sceneId,selfId,newSceneId) == 0 then
							return
					end
                   NewWorld( sceneId, selfId, selfcountry * 100 + 107, 87 + random( 0, 5), 132 + random( 0, 5), 552012)
				end
			end
		end

	
	
end


--**********************************

--�о��¼�

--**********************************

function x552012_ProcEnumEvent(sceneId, selfId, NPCId, MissionId)
			AddQuestNumText(sceneId, x552012_g_ScriptId, x552012_g_ask[1],0,1)
end