#include "main.h"
#include "rendering/ingame/tasks.h"

bool CPlayerPedManager::m_bPlayerSlots[MAX_PLAYERS_NUM];
CPlayerPed*	CPlayerPedManager::m_pPlayers[MAX_PLAYERS_NUM];
CControllerState CPlayerPedManager::m_RemotePlayerKeys[MAX_PLAYERS_NUM];
bool CPlayerPedManager::m_bLocalPlayerNeedsRespawn;
int CPlayerPedManager::m_iLocalPlayerDeathTime;

CControllerState	localPlayerKeys;
CMatrix				cameraAimMatrix;
eCamMode			localPlayerCameraMode;



void __fastcall CPlayerPed__ProcessControl_Hook(CPlayerPed * This)
{
    static void(__thiscall* CPlayerPed__ProcessControl)(CPlayerPed* This) = decltype(CPlayerPed__ProcessControl)(0x60EA90);

	if (This != FindPlayerPed(0))
	{
		int currentPlayerID = CPlayerPedManager::GetPlayerPedSlot(This);
		//printf("pcontrol: %d\n", currentPlayerID);
		if (currentPlayerID == -1) return;

		// set player to focus
		CWorld::PlayerInFocus = currentPlayerID;

		// set remote player's keys
		CPad::GetPad(currentPlayerID)->NewState = CPlayerPedManager::m_RemotePlayerKeys[currentPlayerID];

		// save the internal cammode.
		localPlayerCameraMode = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode;

		// onfoot mouse looking mode.
		//TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode = eCamMode::MODE_NONE;

		// save local player's aim
		cameraAimMatrix = *(CMatrix*)&TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront;

		// set remote player's aim
		*(CMatrix*)&TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront = cameraAimMatrix;//set remote here

		// call the internal CPlayerPed[]::Process
        CPlayerPed__ProcessControl(This);

		CWorld::PlayerInFocus = 0;

		// restore the camera mode.
		TheCamera.m_aCams[TheCamera.m_nActiveCam].m_nMode = localPlayerCameraMode;

		//restore local player's aim
		*(CMatrix*)&TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront = cameraAimMatrix;

        return;
	}
    CPlayerPed__ProcessControl(This);
}

CPlayerInfo* __fastcall CPlayerPed__GetPlayerInfoForThisPlayerPed_Hook(CPlayerPed * This)
{
	if (This == FindPlayerPed(0))return &CWorld::Players[0];
	for (int i = 0; i < MAX_PLAYERS_NUM; i++)
	{
		if (This == CWorld::Players[i].m_pPed)return &CWorld::Players[i];
	}
	return nullptr;
}

void __fastcall CTaskManager__SetTask_Hook(CTaskManager * This, DWORD EDX, CTask *task, int tasksId, bool unused)
{
	if (FindPlayerPed(0) == This->m_pPed)
	{
		//if ()
		if (task)
		{
			int taskID = task->GetId();
			/*if (taskID == eTaskType::TASK_COMPLEX_JUMP)
			{
				CNetworkEntityManager::localPlayer->SendTaskEvent(eTaskEventType::TASK_EVENT_JUMP);
			}*/
			printf("New task for player ped: %d: %s\n", taskID, TaskNames[taskID]);
		}
		else
		{
			CTask* _task = This->m_aPrimaryTasks[tasksId];
			if(_task)printf("New task for player ped: nullptr %d: %s\n", _task->GetId(), TaskNames[_task->GetId()]);
		}
	}
	This->SetTask(task, tasksId, unused);
}

void __fastcall CTaskManager__SetTaskSecondary_Hook(CTaskManager * This, DWORD EDX, CTask *task, int tasksId)
{
    if (FindPlayerPed(0) == This->m_pPed)
    {
        if (task)
        {
            int taskID = task->GetId();
            /*if (taskID == eTaskType::TASK_SIMPLE_DUCK)
            {
                CNetworkEntityManager::localPlayer->SendTaskEvent(eTaskEventType::TASK_EVENT_DUCK);
            }*/
            printf("New task for player ped: %d: %s\n", taskID, TaskNames[taskID]);
        }
        else
        {
            CTask* _task = This->m_aSecondaryTasks[tasksId];
            if (_task)printf("New task for player ped: nullptr %d: %s\n", _task->GetId(), TaskNames[_task->GetId()]);
        }
    }
    This->SetTaskSecondary(task, tasksId);
}


CPad* __cdecl CPad__GetPad_Hook(int number)
{
    return &CPad__Pads[CWorld::PlayerInFocus];
}

CVehicle* __cdecl FindPlayerVehicle_Hook(int playerid, bool bIncludeRemote)
{
	if (playerid < 0)
	{
		if (CWorld::Players[0].m_pPed && CWorld::Players[0].m_pPed->m_nPedFlags.bInVehicle)
		{
			if(!bIncludeRemote || !CWorld::Players[0].m_pRemoteVehicle)return CWorld::Players[0].m_pPed->m_pVehicle;
			else return CWorld::Players[0].m_pRemoteVehicle;
		}
		else return nullptr;
	}
	return FindPlayerVehicle(playerid, bIncludeRemote);
}

void CPlayerPedManager::Init() 
{
	//patch FindPlayerVehicle
	patch::Nop(0x60F2C4, 25);

	//patch::SetUInt(0x568A94 + 2, *(unsigned int*)&CWorld::Players[1]);

	for (int i = 0; i < MAX_PLAYERS_NUM; i++)
	{
		m_bPlayerSlots[i] = false;
		m_pPlayers[i] = nullptr;
		m_RemotePlayerKeys[i] = CControllerState();
	}
	m_bPlayerSlots[0] = true; //0 is always local player 
    m_bPlayerSlots[1] = true;

	m_bLocalPlayerNeedsRespawn = false;

	Events::gameProcessEvent += []
	{

		if (COptions::IsGameReady())
		{
			if (!CPlayerPedManager::m_bLocalPlayerNeedsRespawn && (CWorld::Players[0].m_pPed->m_fHealth <= 0.0f || CWorld::Players[0].m_pPed->m_nPedFlags.bIsBeingArrested))
			{
				CPlayerPedManager::m_bLocalPlayerNeedsRespawn = true;
				CPlayerPedManager::m_iLocalPlayerDeathTime = CTimer::m_snTimeInMilliseconds;
				printf("Local player set to respawn\n");
			}
			if (CPlayerPedManager::m_bLocalPlayerNeedsRespawn && CTimer::m_snTimeInMilliseconds - CPlayerPedManager::m_iLocalPlayerDeathTime >= 3500)
			{
				CWorld::Players[0].m_pPed->m_fHealth = CWorld::Players[0].m_pPed->m_fMaxHealth;
				CWorld::Players[0].m_nPlayerState = ePlayerState::PLAYERSTATE_PLAYING;
				CPlayerPed * ped = FindPlayerPed(0);
				ped->m_nPedFlags.bInVehicle = false;
				// Set player position
				CVector spawn = { 2488.562f, -1666.865f, 12.8757f };
				ped->SetPosn(spawn); 
				CStreaming::LoadScene(&spawn);
				ped->m_nPedFlags.bIsPedDieAnimPlaying = false;
				ped->RestartNonPartialAnims();
				CReferences::RemoveReferencesToPlayer();
				ped->SetInitialState(false);
				CMessages::ClearMessages(true);
				ped->RestoreHeadingRate();
				TheCamera.SetCameraDirectlyBehindForFollowPed_CamOnAString();
				TheCamera.RestoreWithJumpCut();
				ped->m_nPedFlags.bIsBeingArrested = false;
				CHud::ResetWastedText();
				ped->SetWantedLevel(0);
				CPlayerPedManager::m_bLocalPlayerNeedsRespawn = false;
				printf("Local player respawned\n");
			}
		}
	};
}

int CPlayerPedManager::Create(int modelid, const CVector& position) 
{
	CModelManager::LoadModel(modelid);
	int playerid = GetFreePlayerSlot(); 


	CPlayerInfo * info = &CWorld::Players[playerid];
	//info->m_PlayerData = *reinterpret_cast<CPlayerInfo*>(&CWorld__Players[0])->m_pPed->m_pPlayerData;
	CPlayerPed* player = operator_new<CPlayerPed>(playerid, 0);

	info->m_pPed = player;
	info->m_nPlayerState = 0;
    info->m_bDoesNotGetTired = true;
	player->m_nWeaponAccuracy = 100;
	player->m_nPedType = ePedType::PED_TYPE_PLAYER1;
	printf("[CPlayerPedManager]Create - %d\n", playerid);
	CWorld::Add(player);
	player->SetModelIndex(modelid);
	player->SetOrientation(0.0f, 0.0f, 0.0f);
	player->SetPosn(position);
    //CPlayerInfo* playerinfo = player->GetPlayerInfoForThisPlayerPed();
    //assert(playerinfo != nullptr );
	m_bPlayerSlots[playerid] = true;
	m_pPlayers[playerid] = player;
	return playerid;
}

int CPlayerPedManager::GetFreePlayerSlot()
{
	for (int i = 1; i < MAX_PLAYERS_NUM; i++)
	{
        if (!m_bPlayerSlots[i])  return i;
	}
	return -1;
}

int CPlayerPedManager::GetPlayerPedSlot(CPlayerPed * player)
{
	for (int i = 0; i < MAX_PLAYERS_NUM; i++)
	{
		if (m_pPlayers[i] == player) return i;
	}
	return -1;
}

void CPlayerPedManager::ProcessEnterCarAsPassenger()
{
	CVehicle* foundVeh = nullptr;
	float distance = 1000.0;
	for (int i = 0; i < CPools::ms_pVehiclePool->m_nSize; i++)
	{
		CVehicle *veh = CPools::ms_pVehiclePool->GetAt(i);
		if (veh)
		{
			float dist = DistanceBetweenPoints(veh->GetPosition(), FindPlayerPed(0)->GetPosition());
			if (dist < distance)
			{
				foundVeh = veh;
				distance = dist;
			}
		}
	}
	if (foundVeh)
	{
		float dist = DistanceBetweenPoints(foundVeh->GetPosition(), FindPlayerPed(0)->GetPosition());
		if (dist < 8.0f)
		{
			CTask* task = plugin::CallMethodAndReturn<CTask*, 0x63B030, void*, CVehicle*, int, int, bool>(CTask::operator new(44), foundVeh, 0, 3000, false);
			//CTask* task = new CTaskComplexEnterCarAsPassengerTimed(foundVeh, 0, false, );
			FindPlayerPed(0)->m_pIntelligence->m_TaskMgr.SetTask(task, 3, false);
		}
	}
}