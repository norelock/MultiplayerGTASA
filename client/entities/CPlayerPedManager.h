#pragma once
class CPlayerPedManager
{
    CPlayerPedManager();
public:

    static void Init();
	static int Create(int modelid, const CVector& position);
	static int GetFreePlayerSlot();
	static int GetPlayerPedSlot(CPlayerPed * player);
	static void ProcessEnterCarAsPassenger();

	static bool m_bPlayerSlots[MAX_PLAYERS_NUM];
	static CPlayerPed*	m_pPlayers[MAX_PLAYERS_NUM];
	static CControllerState m_RemotePlayerKeys[MAX_PLAYERS_NUM];
	static bool m_bLocalPlayerNeedsRespawn;
	static int m_iLocalPlayerDeathTime;
};

void __fastcall CPlayerPed__ProcessControl_Hook(CPlayerPed * This);
void __fastcall CTaskManager__SetTask_Hook(CTaskManager * This, DWORD EDX, CTask *task, int tasksId, bool unused);
void __fastcall CTaskManager__SetTaskSecondary_Hook(CTaskManager * This, DWORD EDX, CTask *task, int tasksId);
CVehicle* __cdecl FindPlayerVehicle_Hook(int playerid, bool bIncludeRemote);
CPad* __cdecl CPad__GetPad_Hook(int number);
CPlayerInfo* __fastcall CPlayerPed__GetPlayerInfoForThisPlayerPed_Hook(CPlayerPed * This);