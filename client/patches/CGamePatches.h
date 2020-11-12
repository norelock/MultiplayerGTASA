#pragma once

class CGamePatches
{
    static bool SetBranchPointer(uintptr_t currentAddress, uintptr_t src, injector::memory_pointer dest, bool vp = true);
public:
    
    static bool AdjustBranchPointer(uintptr_t Address, uintptr_t src, injector::memory_pointer dest, bool vp = true)
    {
        return SetBranchPointer(GetGlobalAddress(Address), GetGlobalAddress(src), dest, vp);
    }

	static void Init();
	static void LoadScreenPatches();
	static void MenuPatches();
	static void RunningScriptHook();
	static void GameLogicPatches();
	static void GameCorePatches();
	static void WantedPatches();
	static void CrashfixHooks();
	static void GameplayPatches();
	static void InputPatches();
	static void PopulationPatches();
	static void LimitPatches();
	static void PlayerInfoPatch();
    static void PlayerPedPatch();
    static void TaskManagerPatch();
    static void RenderManagerPatch();
    static void ExecuteEXEPatches();
	static void FindPlayerVehiclePatch();
	static void GetPadPatch();
	static void ForcedFullscreenPatches();
};



