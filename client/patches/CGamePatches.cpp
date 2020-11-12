#include "main.h"
#include "CGamePatches.h"
#include "CStats.h"
#include <filesystem>
#include <game_sa/RenderWare.h>

bool CGamePatches::SetBranchPointer(uintptr_t currentAddress, uintptr_t src, injector::memory_pointer dest, bool vp)
{
    auto inst = patch::Get<uint8_t>(currentAddress, false);
    if (inst == 0xE8
        || inst == 0xE9)
    {
        uintptr_t funcptr = injector::ReadRelativeOffset(currentAddress + 1, 4, vp).as_int();

        if (funcptr == src)
        {
            injector::MakeRelativeOffset(currentAddress + 1, dest, 4, vp);
            return true;
        }
    }
    return false;
}

void CGamePatches::Init()
{
	ForcedFullscreenPatches();
	LoadScreenPatches();
	MenuPatches();
	RunningScriptHook();
	GameLogicPatches();
	GameCorePatches();
	WantedPatches();
	CrashfixHooks();
	GameplayPatches();
	InputPatches();
	PlayerInfoPatch();
    PlayerPedPatch();
    PopulationPatches();
    TaskManagerPatch();
    RenderManagerPatch();  
	FindPlayerVehiclePatch();
	GetPadPatch();
    ExecuteEXEPatches();  // Should be always called LAST !
	//this->LimitPatches(); using OpenLA
}

// Thank you Botder
bool Hooked_PSelectDevice()
{
	RwInt32 const numSubSystems = std::min<RwInt32>(RwEngineGetNumSubSystems(), 16);

	if (!numSubSystems)
		return FALSE;

	// RwSubSystemInfo subSystemInfo[MAX_SUBSYSTEMS] = { 0 };
	auto subSystemInfo = reinterpret_cast<RwSubSystemInfo *>(0xC8CFC0);

	for (RwInt32 i = 0; i < numSubSystems; ++i)
		RwEngineGetSubSystemInfo(&subSystemInfo[i], i);

	RwInt32 const subSystem = RwEngineGetCurrentSubSystem();
	RwInt32 const videoMode = RwEngineGetCurrentVideoMode();

	/*if (!RwEngineSetSubSystem(subSystem))
	return FALSE;

	if (!RwEngineSetVideoMode(videoMode))
	return FALSE;*/

	RwVideoMode vm = { 0 };
	RwEngineGetVideoModeInfo(&vm, videoMode);
	HWND const hWnd = RsGlobal.ps->window;

	// Hack: Force windowed mode
	vm.flags = RwVideoModeFlag(0);

	if (vm.flags & rwVIDEOMODEEXCLUSIVE) {
		RsGlobal.maximumWidth = vm.width;
		RsGlobal.maximumHeight = vm.height;
		RsGlobal.ps->fullScreen = TRUE;
		SetWindowLong(hWnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		RwD3D9EngineSetRefreshRate(vm.refRate);
	}
	else {
		RECT rect = { 0 };
		GetClientRect(hWnd, &rect);
		RsGlobal.maximumWidth = rect.right;
		RsGlobal.maximumHeight = rect.bottom;
		int x = (vm.width - rect.right) / 2;
		int y = (vm.height - rect.bottom) / 2;
		SetWindowPos(hWnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	RwD3D9EngineSetMultiSamplingLevels(*reinterpret_cast<RwUInt32 *>(0xBA6810));
	return TRUE;
}

void CGamePatches::ForcedFullscreenPatches()
{
	DWORD originalProtect;
	auto const address = reinterpret_cast<void *>(0x74887B);
	unsigned char const instructions[2] = { 0xEB, 0x5D };
	VirtualProtect(address, 6, PAGE_READWRITE, &originalProtect);
	std::memset(address, 0x90, 6);
	std::memcpy(address, instructions, 2);
	VirtualProtect(address, 6, originalProtect, &originalProtect);

	patch::RedirectJump(0x746190, Hooked_PSelectDevice);
}

void SimulateCopyrightScreen()
{
	CLoadingScreen::m_currDisplayedSplash = 0;
	CLoadingScreen::m_timeSinceLastScreen -= 1000.f;
	CLoadingScreen::m_bFadeInNextSplashFromBlack = 1;
}

void Hooked_LoadingScreen(char * message, char * message2)
{
	if(message || message2) printf("Loading screen: %s %s\n", (message ? message : " "), (message2 ? message2 : " "));
	CLoadingScreen::NewChunkLoaded();
}

void Hooked_RenderSplash()
{
	CSprite2d::InitPerFrame();
}

void Hooked_Init3()
{
	CGame::Init3("DATA\\GTA.DAT");
	printf("Game engine initialized!\n");
	COptions::m_bEngineInitialized = true;
}

void Hooked_LoadSplashes(bool bStarting, bool bNvidia)
{
	LPTSTR cmd = GetCommandLine();
	std::string str(cmd);
	str = str.substr(0, str.find_last_of("\\/")).erase(0,1) + "\\res\\";
    _chdir(str.c_str());

    if (!std::filesystem::exists("custom.txd")) {
        printf("Missing custom.txd, skipping the replacement");
        return;
    }
	int index = CTxdStore::AddTxdSlot("custom");
	CTxdStore::LoadTxd(index, "custom.txd");
	CTxdStore::AddRef(index);
	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(index);
	for (int i = 0; i < 7; i++)
	{
		CLoadingScreen::m_aSplashes[i].SetTexture("altsplash");
	}
	CTxdStore::PopCurrentTxd();
	CFileMgr::SetDir("");
}

void Hooked_DrawBarChart(float posX, float posY, unsigned __int16 width, unsigned int height, float progress, int progressAdded, bool drawPercentage, bool drawBlackBorder, RwRGBA color, RwRGBA progressAddedColor)
{
	float newWidth = RsGlobal.maximumWidth - (posX*2);
	CSprite2d::DrawBarChart(posX, posY, newWidth, height, progress, progressAdded, drawPercentage, drawBlackBorder, color, progressAddedColor);
}

int Hooked_rwerror(int a1, ...)
{
	printf("RenderWare Error: 0x%X\n", a1);
	return a1;
}

HMODULE __stdcall Hooked_LoadLibraryA(LPCSTR lpLibFileName)   
{
	printf("Loading library: %s\n", lpLibFileName);
	return LoadLibraryA(lpLibFileName);
}

void CGamePatches::LoadScreenPatches()
{
	// Modify the loading bar
	patch::ReplaceFunctionCall(0x590480, Hooked_DrawBarChart);

	//replace loading screen
	patch::ReplaceFunctionCall(0x5902C6, Hooked_LoadSplashes);

	//let us know when the game finishes loading/initialization
	patch::ReplaceFunctionCall(0x53BCD9, Hooked_Init3);

	//replace loadscreen
	patch::ReplaceFunction(0x53DED0, Hooked_LoadingScreen);

	//replace rwerror
	patch::ReplaceFunction(0x8088D0, Hooked_rwerror);

	//hook internal LoadLibraryA wrapper
	patch::ReplaceFunction(0x81E412, Hooked_LoadLibraryA);

	//replace splash rendering
	//patch::ReplaceFunctionCall(0x590597, Hooked_RenderSplash);

	// Disable CLoadingScreen::LoadSplashes
	//patch::PutRetn(0x5900B0);
    

	// Start the game at state 5
	// Disable gGameState = 0 setting
	patch::Nop(0x747483, 6);

	// Put the game where the user wants (default's to the copyright screen)
	// GS_INIT_ONCE:5
	patch::Set(0xC8D4C0, 5);

	// Disable Copyright screen
	// Hook the copyright screen fading in/out and simulates that it has happened
	patch::Nop(0x748C2B, 5);
	patch::ReplaceFunctionCall(0x748C9A, SimulateCopyrightScreen);

	// Disable loading screen rendering
	//patch::Nop(0x590D9F, 5);
	//patch::Set<UINT8>(0x590D9F, 0xC3);
	
	// Skip fading screen rendering
	//patch::RedirectJump(0x590AE4, (void*)0x590C9E);
	
	// Disable loading bar rendering
	//patch::Nop(0x5905B4, 5);

	// Disable audio tune from loading screen
	patch::Nop(0x748CF6, 5);
}

void ProcessFrontEndMenu()
{
	static bool menuFirstProcessed = false;
	if (!menuFirstProcessed)
	{
		// Start the game now - resume the timers
		patch::SetUChar(0xB7CB49, 0);
		patch::SetUChar(0xBA67A4, 0);
        patch::SetInt(0xC8D4C0, 8); // gGameState
		menuFirstProcessed = true;
	}
	// Call the original function - Sets render states
	plugin::Call<0x573A60>();
}

void CGamePatches::MenuPatches()
{
	// Disable CMenuManager::Process
	//patch::PutRetn(0x57B440);
    
	// Disable menu by skipping CMenuManager::DrawStandardMenus call in CMenuManager::DrawBackground
	patch::Nop(0x57BA57, 6);

	// No background texture drawing in menu
	patch::RedirectShortJump(0x57B9CA);

	// No input process for menu
	patch::Nop(0x57B457, 5);

	// No background black rectangle
	patch::Nop(0x57B9BB, 5);

	// No more ESC button processing
	patch::RedirectShortJump(0x576B8D);
    patch::RedirectShortJump(0x576BAF);

	// No frontend texture loading (Disable CMenuManager::LoadAllTextures)
	//patch::PutRetn(0x572EC0);
    

	// Allow widescreen resolutions
	patch::SetUInt(0x745B81, 0x9090587D);
	patch::SetUInt(0x74596C, 0x9090127D);
	patch::Nop(0x745970, 2);
	patch::Nop(0x745B85, 2);
	patch::Nop(0x7459E1, 2);

	// Hook menu process
	patch::ReplaceFunctionCall(0x57C2BC, ProcessFrontEndMenu);

	// Allow Alt+Tab without pausing the game
	int patchAddress = NULL;
	if (*(BYTE *)0x748ADD == 0xFF && *(BYTE *)0x748ADE == 0x53)
		patchAddress = 0x748A8D;
	else
		patchAddress = 0x748ADD;

	patch::Nop(patchAddress, 6);

	// Disable MENU AFTER alt + tab
	patch::SetUChar(0x53BC78, 0x00);

	// Disable menu after focus loss
    patch::PutRetn(0x53BC60);
    
	// Disable GTA Setting g_bIsForegroundApp to false on focus lost
	/*patch::Nop(0x747FFE, 6);
	patch::Nop(0x748054, 10);*/
}


void Hook_CRunningScript__Process()
{
    static bool scriptProcessed = false;
	if (!scriptProcessed)
	{
		// Change player model ID
		patch::SetUChar(0x60D5FF + 1, 7);

		// CPlayerPed::SetupPlayerPed
		CPlayerPed::SetupPlayerPed(0);

		//Set player in focus
		CWorld::PlayerInFocus = 0;

		// Set player position
		FindPlayerPed(0)->SetPosn(2488.562f, -1666.865f, 12.8757f);

		FindPlayerPed(0)->GetPlayerInfoForThisPlayerPed()->m_bDoesNotGetTired = true;
	
		//Setup weapon skills
		for (int i = 69; i < 80; i++)
		{
			CStats::SetStatValue(i, 1000.0);
		}

		CWeaponManager::GiveWeapon(FindPlayerPed(0), eWeaponType::WEAPON_M4, 1000, false);
		CWeaponManager::GiveWeapon(FindPlayerPed(0), eWeaponType::WEAPON_SAWNOFF, 1000, false);
		CWeaponManager::GiveWeapon(FindPlayerPed(0), eWeaponType::WEAPON_DESERT_EAGLE, 1000, false);

		// CStreaming::LoadScene
		CVector spawn = { 2488.562f, -1666.865f, 12.8757f };
		CStreaming::LoadScene(&spawn);

		//CVehicleManager::Create(411, { 2488.562f, -1660.865f, 12.8757f });

		// First tick processed
		scriptProcessed = true;

        COptions::m_bLocalPlayerSpawned = true;
        COptions::m_iStartTime = CTimer::m_snTimeInMilliseconds;

		printf("[CGamePatches] CRunningScript hook finished\n");

		//ShowCursor(true);
		//FrontEndMenuManager.m_bDrawMouse = true;
	}
}

void Hook_CPopulation__Update()
{
	CPopulation::ManagePopulation();
	switch (CWeather::WeatherRegion)
	{
	case 0:
	case 1:
	case 4:
		CPopulation::CurrentWorldZone = 0;
		break;
	case 2:
		CPopulation::CurrentWorldZone = 1;
		break;
	case 3:
		CPopulation::CurrentWorldZone = 2;
		break;
	}
}

bool __fastcall Hook_FallUnderMap(CPed* ped)
{
	if (ped == FindPlayerPed(0))
	{
		printf("stop falling under the map niga\n");
	}
	return ped->IsPlayer();
}

void CGamePatches::RunningScriptHook()
{
	// Disable CTheScripts::Init
	//patch::PutRetn(0x468D50);

	// Disable CTheScripts::Update
	//patch::PutRetn(0x46A000);

	// Disable CTheScripts::StartTestScript
	//patch::PutRetn(0x464D40);

	// Disable CRunningScript::ProcessOneCommand
	patch::PutRetn(0x469EB0);

	// Disable opcode table
	patch::Nop(0x8A6168, 0x6C);

	// Don't load the SCM Script
    patch::RedirectShortJump(0x468EB5, (void*)0x468EE9);

	// Hook script process (so we can spawn a local player)
	patch::ReplaceFunctionCall(0x46A21B, Hook_CRunningScript__Process);
	//patch::ReplaceFunctionCall(0x46A000, Hook_CRunningScript__Process);

	// Fixes collision problem caused by population patches
	//patch::ReplaceFunctionCall(0x616650, Hook_CPopulation__Update);
	//patch::PutRetn(0x616650+5);

	patch::ReplaceFunctionCall(0x565D1E, Hook_FallUnderMap);
}

void Hook_CRestart__FindClosestHospitalRestartPoint(float a1, float a2, float a3, RwV3d *a4, float *a5)
{
	a4 = &CRestart::OverridePosition->ToRwV3d();
	a5 = &CRestart::OverrideHeading;

	a1 = 2488.562f;
	a2 = -1666.865f;
	a3 = 12.8757f;

	CRestart::bOverrideRestart = false;

	printf("called CRestart::Find...");
}

void Hook_RestorePlayer()
{
	// Set CGamelogic::GameState to 0
	patch::SetChar(0x96A8B0, 0);

	// Set player position
	FindPlayerPed(0)->SetPosn(2488.562f, -1666.865f, 12.8757f);

	// RestorePlayerStuffDuringResurrection
	Call<0x442060, CPed*, RwV3d, float>(FindPlayerPed(0), CVector(2488.562f, -1666.865f, 12.8757f).ToRwV3d(), 0.0);
}

void CGamePatches::GameLogicPatches()
{
	// Fix killing ped during car jacking (#4319)
	// by using CTaskComplexLeaveCar instead of CTaskComplexLeaveCarAndDie
	patch::SetUChar(0x63F576, 0xEB);

	// Let us sprint everywhere (always return 0 from CSurfaceData::isSprint)
	patch::SetUInt(0x55E870, 0xC2C03366);
	patch::SetUShort(0x55E874, 0x0004);

	// Throwable weapons are now more accurate
	patch::SetUChar(0x742685, 0x90);
	patch::SetUChar(0x742686, 0xE9);

	// Disable blur caused by high speed vehicles
	patch::Nop(0x704E8A, 5);

	//CPedStats::fHeadingChangeRate fix
	patch::SetFloat(0x5BFA1D + 4, 9.5f);

	// Fix 14ms delay
	patch::SetUShort(0x53E923, 0x43EB); // jump to CTimer::Update
	patch::SetUChar(0x53E99F, 0x10); // fix the stack
	patch::Nop(0x53E9A5, 1); // nop "pop esi"

	// Fix walk weapons strafe with high fps
	static float FixedDeltaMoveCommand = 10.1f;
	patch::SetPointer(0x61E0C0 + 2, &FixedDeltaMoveCommand);
	patch::Nop(0x61E0CA, 0x6); // nop multiplying by 0.07

	// Dont fade the camera on death
	patch::Nop(0x442DE7, 16);

	// Disable CGarages::Update
	patch::Nop(0x53C0B7, 5);

	// Disable CIdleCam::Process
	patch::PutRetn(0x522C80);

	// No gta_sa.set writing
	//patch::PutRetn(0x57C8F0);
	patch::PutRetn(0x57C660);

	// Disable CGameLogic::Update
	patch::PutRetn(0x442AD0);

	// Disable CGameLogic::CalcDistanceToForbiddenTrainCrossing
	patch::PutRetn(0x4418E0);

	// Don't give cigar/beer to player on spawn
    patch::RedirectShortJump(0x5FBA26);

	// Disable CObject::ProcessSamSiteBehaviour
	patch::PutRetn(0x5A07D0);

	// Disable CCheat::DoCheats
	//patch::PutRetn(0x439AF0);
	// Disable CCheat::ToggleCheat
	//patch::PutRetn(0x438370);

	// Disable the game's replay system (recording & playing - see CReplay stuff)
	// F1 = Play the last 30 second of gameplay
	// F2 = Save the last 30 second of gameplay to Hard Disk
	// F3 = Play a replay from Hard Disk (replay.rep)
	// F1/F3 = Finish playback (if a replay is playing)
	// CReplay::FinishPlayback
	patch::PutRetn(0x45F050);
	// CReplay::TriggerPlayback
	patch::PutRetn(0x4600F0);
	// CReplay::Update
	patch::PutRetn(0x460500);
	// PlayReplayFromHD
	patch::PutRetn(0x460390);

	// No "JCK_HLP" message
    patch::RedirectShortJump(0x63E8DF);

	// Disable CFont::Initialize
	//patch::PutRetn(0x5BA690);
	// Disable CFont::Shutdown
	//patch::PutRetn(0x7189B0);

	// Disable CPlayerInfo::MakePlayerSafe
	patch::PutRetn(0x56E870, 8);

	// Disable CInterestingEvents::ScanForNearbyEntities
	//patch::PutRetn(0x605A30);

	// Disable CGangWars::Update
	patch::PutRetn(0x446610);

	// Disable CConversations::Update
	//patch::PutRetn(0x43C590);
	// Disable CPedToPlayerConversations::Update
	//patch::PutRetn(0x43B0F0);

	// Disable ValidateVersion
	// Contains a stupid check for 'grandtheftauto3' string in peds.col
	patch::PutRetn(0x5BA060);

	// Disable CShopping::LoadStats
	patch::PutRetn(0x49B6A0);

	// Disable CEntryExitManager::Update
	patch::PutRetn(0x440D10);

	// Stop CTaskSimpleCarDrive::ProcessPed from exiting passengers with CTaskComplexSequence
	patch::Nop(0x644C18, 1);
	patch::SetUChar(0x644C19, 0xE9);

	// Disable CPlayerInfo::WorkOutEnergyFromHunger (Prevents dying from starvation)
	patch::PutRetn(0x56E610);

	// Disable CFileLoader::LoadPickup
	patch::PutRetn(0x5B47B0);

	// Disable Interior_c::AddPickups
	patch::PutRetn(0x591F90);

	// Make CEventDamage::IsSameEventForAI return false
	//patch::SetUChar(0x4C01F0, 0xB0);
	//patch::SetUChar(0x4C01F1, 0x00);
	//patch::Nop(0x4C01F2, 3);

	// Don't lock the cursor at 0,0
	patch::PutRetn(0x7453F0);

	// Fix for sliding over objects and vehicles (ice floor)
	patch::RedirectJump(0x5E1E72, (void*)0x5E1F30);
	patch::Nop(0x5E1E77, 1);

	// Avoid GTA setting vehicle primary color to white (1) after setting a new paintjob
	patch::Nop(0x6D65C5, 11);

	// Disable CGridRef::Init
	patch::PutRetn(0x71D4E0);

	// Don't change velocity when colliding with peds in a vehicle
	patch::SetUInt(0x5F12CA, 0x901CC483);
	patch::Nop(0x5F12CA + 4, 1);

	// Disable ped vehicles damage when flipped
	patch::SetUShort(0x6A776B, 0xD8DD);
	patch::Nop(0x6A776D, 4);

	// Disable player vehicle damage when flipped
	patch::SetUShort(0x570E7F, 0xD8DD);
	patch::Nop(0x570E81, 4);

	// No vehicle rewards
	patch::Nop(0x6D16D6, 1);
    patch::RedirectJump(0x6D16D6 + 1, (void*)0x6D17D5);
	patch::Nop(0x6D1999, 1);
    patch::RedirectJump(0x6D1999 + 1, (void*)0x6D1A36);

	// Increase Streaming_rwObjectInstancesList limit (disables flicker)
	patch::SetInt(0x5B8E55, 7500 * 0xC);
	patch::SetInt(0x5B8EB0, 7500 * 0xC);

	// Disable SecuromStateDisplay
	patch::PutRetn(0x744AE0);

	// SetWindowText
	patch::Set(0x619608, PROJECT_NAME);

	// Disable CIniFile::LoadIniFile (gta3.ini)
	//patch::PutRetn(0x56D070);

	// Disable CStreaming::ReadIniFile (stream.ini)
	//patch::PutRetn(0x5BCCD0);

	// No random hydraulics for cars
    patch::RedirectShortJump(0x6B0BC2);

	// Allow boats to rotate
	patch::SetUChar(0x6F2089, 0x58);
	patch::Nop(0x6F208A, 4);

	// force static shadows on peds
	patch::Nop(0x5E6766, 2);
	// force static shadows on vehicles
	patch::SetChar(0x7113C0, 0xEB);

	// Stop CPed::ProcessControl from calling CVisibilityPlugins::SetClumpAlpha
	//patch::Nop(0x5E8E84, 5);

	// Stop CVehicle::UpdateClumpAlpha from calling CVisibilityPlugins::SetClumpAlpha
	//patch::Nop(0x6D29CB, 5);

	// Disable CVehicle::DoDriveByShootings
	patch::Nop(0x741FD0, 3);
	patch::PutRetn(0x741FD0);

	// Disable CTaskSimplePlayerOnFoot::PlayIdleAnimations (ret 4)
	//patch::PutRetn(0x6872C0, 4);

	// Hack to make the choke task use 0 time left remaining when he starts to just stand there looking. So he won't do
	// that.
	patch::SetUChar(0x620607, 0x33);
	patch::SetUChar(0x620608, 0xC0);

	patch::SetUChar(0x620618, 0x33);
	patch::SetUChar(0x620619, 0xC0);
	patch::SetUChar(0x62061A, 0x90);
	patch::SetUChar(0x62061B, 0x90);
	patch::SetUChar(0x62061C, 0x90);

	// Disable speed limits
    patch::PutRetn0(0x72DDD0);

	// Increase tail light corona's intensity
	patch::SetUChar(0x6E1A22, 0xF0);

	// Disable stealth-kill aiming (holding knife up)
	patch::Nop(0x685DFB, 5);
	patch::SetUChar(0x685DFB, 0x33);
	patch::SetUChar(0x685DFC, 0xC0);
	patch::Nop(0x685C3E, 5);
	patch::SetUChar(0x685C3E, 0x33);
	patch::SetUChar(0x685C3F, 0xC0);
	patch::Nop(0x685DC4, 5);
	patch::SetUChar(0x685DC4, 0x33);
	patch::SetUChar(0x685DC5, 0xC0);
	patch::Nop(0x685DE6, 5);
	patch::SetUChar(0x685DE6, 0x33);
	patch::SetUChar(0x685DE7, 0xC0);

	// Disable stealth-kill rotation in CTaskSimpleStealthKill::ProcessPed
	// Used to face the dying ped away from the killer.
	patch::Nop(0x62E63F, 6);
	patch::SetUChar(0x62E63F, 0xDD);
	patch::SetUChar(0x62E640, 0xD8);
	patch::Nop(0x62E659, 6);
	patch::SetUChar(0x62E659, 0xDD);
	patch::SetUChar(0x62E65A, 0xD8);
	patch::Nop(0x62E692, 6);
	patch::SetUChar(0x62E692, 0xDD);
	patch::SetUChar(0x62E693, 0xD8);

	// Force the MrWhoopee music to load even if we are not the driver.
	patch::SetUChar(0x4F9CCE, 0xCE);

	// Limit fps
	RsGlobal.frameLimit = 100;
	FrontEndMenuManager.m_bFrameLimiterOn = true;
}

void DoNothing()
{
	CFileLoader::LoadObjectTypes("data\\default.ide");
	CFileLoader::LoadObjectTypes("data\\vehicles.ide");
	CFileLoader::LoadObjectTypes("data\\peds.ide");
}

void CGamePatches::GameCorePatches()
{
	FrontEndMenuManager.m_fDrawDistance = 1000.0f;

	// Patch semaphore, allow more than one game instances

	static char cdstream[9];
	sprintf(cdstream, "%d", GetTickCount());
	patch::SetRaw(0x858AD4, cdstream, 9);


	// Set streaming memory to 128MB
	//patch::SetUInt(0x5B8E6A, 134217728);

	// Dont print aspect ratios
	patch::Nop(0x745997, 5);

	// Dont print soundmanager text
	patch::Nop(0x5B97C9, 5);
	 
	// Use our icon
	patch::SetUChar(0x7486A5, 1);

	// Patch IsAlreadyRunning 
    patch::PutRetn0(0x7468E0);

	// Don't catch WM_SYSKEYDOWN and WM_SYSKEYUP (fixes Alt+F4)
    patch::RedirectJump(0x748220, (void*)0x748446);
	patch::SetUChar(0x7481E3, 0x5C); // esi -> ebx
	patch::SetUChar(0x7481EA, 0x53); // esi -> ebx
	patch::SetUChar(0x74820D, 0xFB); // esi -> ebx
	patch::SetChar(0x7481EF, 0x54 - 0x3C); // use stack space for new lParam
	patch::SetChar(0x748200, 0x4C - 0x3C); // use stack space for new lParam
	patch::SetChar(0x748214, 0x4C - 0x3C); // use stack space for new lParam

    patch::RedirectJump(0x74826A, (void*)0x748446);
	patch::SetUChar(0x74822D, 0x5C); // esi -> ebx
	patch::SetUChar(0x748234, 0x53); // esi -> ebx
	patch::SetUChar(0x748257, 0xFB); // esi -> ebx
	patch::SetChar(0x748239, 0x54 - 0x3C); // use stack space for new lParam
	patch::SetChar(0x74824A, 0x4C - 0x3C); // use stack space for new lParam
	patch::SetChar(0x74825E, 0x4C - 0x3C); // use stack space for new lParam
	
	// Disable loading default.dat in CGame::Initialize
	//patch::ReplaceFunctionCall(0x53BC95, DoNothing);
}

void CGamePatches::WantedPatches()
{
	// Disable Police Mavericks and News choppers at 3+ wanted stars
	// by making CWanted::NumOfHelisRequired always return 0
	patch::PutRetn0(0x561FA0);
    
	// Disable CWanted::UpdateEachFrame
	patch::Nop(0x53BFF6, 5);

	// Disable CWanted::Update
	patch::PutRetn(0x562C90);

	// Disable CCrime::ReportCrime
	patch::PutRetn(0x532010);

	// Disable CWanted::SetWantedLevel
	patch::PutRetn(0x562470,4);
	
	// Disable military zones (5star wanted level)
    patch::RedirectShortJump(0x72DF0D);
	
	// Disable CGameLogic::SetPlayerWantedLevelForForbiddenTerritories
	// No 4-star wanted level on visiting LV and SF
	patch::PutRetn(0x441770);

	// Disable CRoadBlocks::Init
	patch::PutRetn(0x461100);

	// Disable CRoadBlocks::GenerateRoadBlocks
	patch::PutRetn(0x4629E0);

	// Disable CRoadBlocks::GenerateRoadBlockPedsForCar
	patch::PutRetn(0x461170);
}

UINT32 Return_CTaskComplexCarSlowBeDraggedOut__PrepareVehicleForPedExit = 0x6485B2;
UINT32 Return_CTaskComplexCarSlowBeDraggedOut__PrepareVehicleForPedExit_Invalid = 0x6485E1;
void CTaskComplexCarSlowBeDraggedOut__PrepareVehicleForPedExit_Custom()
{
	//CTN_LOGWARNING("CTaskComplexCarSlowBeDraggedOut race condition");
}

void __declspec(naked) Hook_CTaskComplexCarSlowBeDraggedOut__PrepareVehicleForPedExit()
{
	__asm
	{
		test eax, eax
		jz InvalidVehicle

		mov ecx, [eax + 460h]
		jmp Return_CTaskComplexCarSlowBeDraggedOut__PrepareVehicleForPedExit

		InvalidVehicle :
		pushad
			call CTaskComplexCarSlowBeDraggedOut__PrepareVehicleForPedExit_Custom
			popad
			jmp Return_CTaskComplexCarSlowBeDraggedOut__PrepareVehicleForPedExit_Invalid
	}
}

void* (*ColModelPool_new)(unsigned int);
void __declspec(naked) Patched_LoadColFileFix(int size)
{
	__asm
	{
		// Perform the original operation (new ColModel)
		push[esp + 4]
		mov eax, ColModelPool_new
		call eax
		add esp, 4

		// Now, the fix is here, edi should contain the ColModel pointer, but it doesn't!
		// Let's fix it
		mov edi, eax
		ret
	}
}

void CGamePatches::CrashfixHooks()
{
	//Anim crash in CPlayerPed::ProcessControl
	patch::Nop(0x609C08, 39);

	// No DirectPlay dependency
	// Increase compatibility for Windows 8+
	patch::SetUChar(0x74754A, 0xB8);
	patch::SetUInt(0x74754B, 0x900);

	// Don't create a ped group on player creation (Fixes a crash)
	// TODO: Proper CPlayerPed creation
	patch::Nop(0x60D64D, 1);
	patch::SetUChar(0x60D64E, 0xE9);

	// Disable the call to FxSystem_c::GetCompositeMatrix in CAEFireAudioEntity::UpdateParameters
	// Which was causing a crash. The crash happens if you create 40 or
	// so vehicles that catch fire (upside down) then delete them, repeating a few times.
	patch::Nop(0x4DCF87, 6);

	// Fixed a crash (race condition triggered when jacking a vehicle)
	patch::Nop(0x6485AC, 6);
    patch::RedirectJump(0x6485AC, Hook_CTaskComplexCarSlowBeDraggedOut__PrepareVehicleForPedExit);

	// Fix mirror crash
	patch::SetUChar(0x7271CB + 0, 0x85); // test eax, eax
	patch::SetUChar(0x7271CB + 1, 0xC0);
	patch::SetUChar(0x7271CB + 2, 0x74); // je 0x727203
	patch::SetUChar(0x7271CB + 3, 0x34);
	patch::SetUChar(0x7271CB + 4, 0x83); // add esp, 04
	patch::SetUChar(0x7271CB + 5, 0xC4);
	patch::SetUChar(0x7271CB + 6, 0x04);
	patch::SetUChar(0x7271CB + 7, 0xC6); // mov byte ptr [00C7C728],01

	// No FxMemoryPool_c::Optimize (causes heap corruption)
	patch::Nop(0x5C25D3, 5);

	// Satchel charge crash fix
	patch::Nop(0x738F3A, 83);

	// Fixes the crash caused by using COLFILE for a building etc
	ColModelPool_new = CColModel::operator new;
	patch::ReplaceFunctionCall(0x5B4F2E, Patched_LoadColFileFix);
}

/*
void __fastcall CWeapon__Fire(CWeapon *weapon, DWORD EDX, CPed *owner, CVector *vecOrigin, CVector *vecEffectPosn, CEntity *targetEntity, CVector *vecTarget, CVector *arg_14)
{
	if (owner == FindPlayerPed(0))
	{
		BulletSyncEvent ev;
		ev.ownerNetworkID = CNetworkEntityManager::localPlayer->iNetworkID;
		ev.victimNetworkID = CNetworkEntityManager::FindNetworkIDByEntity(targetEntity);
		ev.vecOrigin = *(zplm_vec3_t*)&vecOrigin;
		ev.vecTarget = *(zplm_vec3_t*)&vecTarget;
		ev.vecEffectPosn = *(zplm_vec3_t*)&vecEffectPosn;
		ev.vecOriginForDriveBy = *(zplm_vec3_t*)&arg_14;
		ev.weaponID = weapon->m_nType;
		if (weapon->Fire(owner, vecOrigin, vecEffectPosn, targetEntity, vecTarget, arg_14))
		{
			CNetworkEntityManager::localPlayer->ProcessBullet(&ev);
			printf("Sending bullet to network!\n");
		}
	}
}
*/

void __fastcall CWeapon__DoBulletImpact(CWeapon *weapon,DWORD EDX, CEntity *owner, CEntity *victim, CVector *startPoint, CVector *endPoint, CColPoint *colPoint, int a7)
{
    if (owner == FindPlayerPed(0))
    {
        BulletImpactSyncEvent ev;
        ev.ownerNetworkID = CNetworkEntityManager::localPlayer->iNetworkID;
        ev.victimNetworkID = CNetworkEntityManager::FindNetworkIDByEntity(victim);
        ev.startPoint = *(zplm_vec3_t*)&startPoint;
        ev.endPoint = *(zplm_vec3_t*)&endPoint;
        ev.arg7 = a7;
        ev.weaponID = weapon->m_nType;
        weapon->DoBulletImpact(owner, victim, startPoint, endPoint, colPoint, a7);
        CNetworkEntityManager::localPlayer->ProcessBullet(&ev);
    }
}

void CGamePatches::GameplayPatches()
{
    /*
	patch::RedirectCall(0x61ECCD, CWeapon__Fire);
	patch::RedirectCall(0x628328, CWeapon__Fire);
	patch::RedirectCall(0x62B109, CWeapon__Fire);
	patch::RedirectCall(0x62B12A, CWeapon__Fire);
	patch::RedirectCall(0x68626D, CWeapon__Fire);
	patch::RedirectCall(0x686283, CWeapon__Fire);
	patch::RedirectCall(0x686787, CWeapon__Fire);
    */

    patch::RedirectCall(0x73CD92, CWeapon__DoBulletImpact);
    patch::RedirectCall(0x741199, CWeapon__DoBulletImpact);
    patch::RedirectCall(0x7411DF, CWeapon__DoBulletImpact);
    patch::RedirectCall(0x7412DF, CWeapon__DoBulletImpact);
    patch::RedirectCall(0x741E30, CWeapon__DoBulletImpact);
    

	// Disable vehicle name rendering
	patch::Nop(0x58FBE9, 5);

	//Disable procedurally generated terrain objects
	patch::Nop(0x53C159, 5);

	//Disable heathaze
	patch::PutRetn(0x701780);

	// Disable CStats::IncrementStat (returns at start of function)
	patch::PutRetn(0x55C180);

	// DISABLE STATS DECREMENTING
	patch::Nop(0x559FD5, 7);
	patch::Nop(0x559FEB, 7);

	// DISABLE STATS MESSAGES
	patch::PutRetn(0x55B980);
	patch::PutRetn(0x559760);

	//Disallow mouse movement
	//patch::SetUChar(0x6194A0, 0xE9);

	//Toggle time passing off
	patch::PutRetn(0x52CF10);

	//CPlayerInfo and ProcessControl shit
	patch::Nop(0x60F2C4, 25);

	// Stop ped rotations from the camera
	//patch::Nop(0x6884C4, 6);
}

void CGamePatches::InputPatches()
{
	// Disable re-initialization of DirectInput mouse device by the game
	patch::SetUChar(0x576CCC, 0xEB);
	patch::SetUChar(0x576EBA, 0xEB);
	patch::SetUChar(0x576F8A, 0xEB);

	// Make sure DirectInput mouse device is set non-exclusive (may not be needed?)
	patch::SetUInt(0x7469A0, 0x909000B0);

	//CPed::Say patch
	patch::SetUInt(0x5EFFE0, 0x900018C2);
}

void CGamePatches::PopulationPatches()
{
	// Disable CPopulation::Initialise x
	patch::PutRetn(0x610E10);

	// Disable CPopCycle::Initialise x
	patch::PutRetn(0x5BC090);

	// Disable CPedType::LoadPedData x
	//patch::PutRetn(0x608B30);

	// Disable CDecisionMakerTypesFileLoader::LoadDefaultDecisionMaker x
	//patch::PutRetn(0x5BF400);
	// Disable CPedStats::LoadPedStats x
	//patch::PutRetn(0x5BB890);
	// Change CPedStats::fHeadingChangeRate (was 15.0) x
	patch::SetFloat(0x5BFA1D + 4, 9.5f);

	// Disable CStreaming::StreamVehiclesAndPeds_Always ?
	patch::PutRetn(0x40B650);

	// Stop the loading of ambient traffic models and textures x
	// by skipping CStreaming::StreamVehiclesAndPeds() and CStreaming::StreamZoneModels()
	//patch::RedirectShortJump(0x40E7DF);
	patch::Nop(0x40E7E1, 5);
	patch::PutRetn(0x40A560);

	// Disable CPopulation::AddToPopulation x
	patch::PutRetn0(0x614720);
    

	// Disable CPopulation::Update
	patch::PutRetn(0x616650);

	// Disable CPopulation::RemovePed
	patch::PutRetn(0x610F20);

	// Disable CPopulation::RemoveAllRandomPeds
	patch::PutRetn(0x6122C0);

	// Disable CPopulation::RemovePedsIfPoolGetsFull
	patch::PutRetn(0x616300);

	// Disable CTrain::DoTrainGenerationAndRemoval
	patch::PutRetn(0x6F7900);

	// Prevent trains spawning with peds
	patch::RedirectShortJump(0x6F7865);
	patch::RedirectJump(0x6F8E7B, (void*)0x6F8F89);
	patch::Nop(0x6F8E80, 1);

	// Disable CPlane::DoPlaneGenerationAndRemoval
	patch::PutRetn(0x6CD2F0);

	// Disable CCarCtrl::GenerateEmergencyServicesCar
	patch::PutRetn(0x42F9C0);
	
	// Disable CCarCtrl::GenerateOneEmergencyServicesCar
	//patch::PutRetn(0x42B7D0);
	patch::PutRetn(0x42BB2C);

	// Disable CCarCtrl::ScriptGenerateOneEmergencyServicesCar
	patch::PutRetn(0x42FBC0);

	// Disable CTheCarGenerators::Process
	patch::PutRetn(0x6F3F40);

	// Disable CCarCtrl::RemoveDistantCars
	patch::PutRetn(0x42CD10);

	// Disable CCarCtrl::PossiblyRemoveVehicle
	patch::PutRetn(0x424F80);

	// Disable CCarCtrl::RemoveCarsIfThePoolGetsFull
	patch::PutRetn(0x4322B0);

	// Disable CCarCtrl::GenerateRandomCars
	patch::PutRetn(0x4341C0);

	// Disable CCarCtrl::GenerateOneRandomCar
	patch::PutRetn(0x430050);

    /*
	patch::ReplaceFunctionCall(0x53C1C1, CCarCtrl__GenerateRandomCars_Hook);
    patch::ReplaceFunctionCall(0x53C030, CPopulation__Update_Hook);
    patch::ReplaceFunctionCall(0x53C054, CPopulation__Update_Hook);
    patch::ReplaceFunctionCall(0x615D4B, CPopulation__AddToPopulation_Hook);
    patch::ReplaceFunctionCall(0x61679B, CPopulation__AddToPopulation_Hook);

    Patch_Funcs.push_back(
        [](uint32_t Address) -> bool
		{
			return AdjustBranchPointer(Address, 0x612710, CPopulation__AddPed_Hook, false);
		}
    );
	*/
}

void CGamePatches::LimitPatches()
{
	//Inc task pool
	patch::SetUChar(0x551140, 0xFF);

	//Inc ped pool pool
	patch::SetUChar(0x550FF2, 1000);

	//Inc intelligence pool
	patch::SetUChar(0x551283, 210);

	//Inc event pool
	patch::SetUChar(0x551178, 0x01);

	//Inc matrices pool
	patch::SetUChar(0x54F3A2, 0x10);

	//Inc ccolmodel pool
	patch::SetUChar(0x551108, 0x77);

	//Inc dummies pool
	patch::SetUChar(0x5510D0, 0x0F);

	//Inc objects pool
	patch::SetUChar(0x551098, 0x02);
}


CPlayerInfo CWorld__Players[MAX_PLAYERS_NUM];
CPad CPad__Pads[MAX_PLAYERS_NUM];

void CGamePatches::PlayerInfoPatch()
{
	//InGame Limit of CWorld::Players allocations
	patch::SetUChar(0x84E98A+1, MAX_PLAYERS_NUM);
	patch::SetUChar(0x856505+1, MAX_PLAYERS_NUM);

    //InGame Limit of Pads allocations
    patch::SetUChar(0x84E1FA + 1, MAX_PLAYERS_NUM);
    patch::SetUChar(0x856465 + 1, MAX_PLAYERS_NUM);

	CWorld::Players = CWorld__Players;

    Patch_Funcs.push_back( 
        [](uint32_t Address) -> bool
    {
        
        uint32_t Content = patch::GetUInt(Address, false);
        
        if (Content == 0xB7CD98)
        {

            patch::Set(Address, CWorld__Players, false);

        }
        else if (Content > 0xB7CD98 && Content <= (0xB7CD98 + sizeof(CPlayerInfo)))
        {
            uint32_t offset = (Content - 0xB7CD98);

            patch::Set(Address, uint32_t(CWorld__Players) + offset, false);
        }
        else
            return false;

        return true;
    });

    Patch_Funcs.push_back(
        [](uint32_t Address) -> bool
    {
        uint32_t Content = patch::GetUInt(Address, false);
        if (Content == 0xB73458)
        {

            patch::Set(Address, CPad__Pads, false);
        }
        else if (Content > 0xB73458 && Content <= 0xB73458 + sizeof(CPad) && (*(unsigned char*)(Address) != 0xE8))
        {
            //printf("found address in offset range: 0x%X\n", i);
            uint32_t offset = (Content - 0xB73458);

            patch::Set(Address, uint32_t(CPad__Pads) + offset, false);
        }
        else
            return false;

        return true;
    });

    Events::drawHudEvent.after += []() {
        CPlayerPed* player = FindPlayerPed(0);
        if (!player) return;

        /*gamefont::Print({
            Format("CurrentRot: %f",player->m_fCurrentRotation),
            Format("Heading: %f",player->GetHeading()),
            Format("AimingRot : %f",player->m_fAimingRotation),

            }, 100.0f, 100.0f);*/
    };

    //printf("Patched %d %d CWorld::Players xrefs - %dms\n", found_pads_abs, found2_pads_offset, (GetTickCount() - starttime));
	//printf("Patched %d %d CPad::Pads xrefs - %dms\n", found_CWP_abs, found2_CWP_offset, (GetTickCount() - starttime));
}

void CGamePatches::PlayerPedPatch()
{
	Patch_Funcs.push_back(
		[](uint32_t Address) -> bool
	{
		return AdjustBranchPointer(Address, 0x609FF0, CPlayerPed__GetPlayerInfoForThisPlayerPed_Hook, false);
	}
	);

    patch::SetPointer(0x86D190, CPlayerPed__ProcessControl_Hook);
}

void CGamePatches::FindPlayerVehiclePatch()
{
	Patch_Funcs.push_back(
		[](uint32_t Address) -> bool
	{
		return AdjustBranchPointer(Address, 0x56E0D0, FindPlayerVehicle_Hook, false);
	}
	);
}

void CGamePatches::GetPadPatch()
{
	Patch_Funcs.push_back(
		[](uint32_t Address) -> bool
	{
		return AdjustBranchPointer(Address, 0x53FB70, CPad__GetPad_Hook, false);
	}
	);
}
void CGamePatches::TaskManagerPatch()
{
	Patch_Funcs.push_back(
		[](uint32_t Address) -> bool
	{
		return AdjustBranchPointer(Address, 0x681AF0, CTaskManager__SetTask_Hook, false);
	}
	);

    Patch_Funcs.push_back(
        [](uint32_t Address) -> bool
    {
        return AdjustBranchPointer(Address, 0x681B60, CTaskManager__SetTaskSecondary_Hook, false);
    }
    );
    
}

void CGamePatches::RenderManagerPatch()
{
	Patch_Funcs.push_back(
		[](uint32_t Address) -> bool
	{
		return AdjustBranchPointer(Address, 0x619560, RsKeyboardEventHandler_Hook, false);
	}
	);
}

void CGamePatches::ExecuteEXEPatches()
{
    auto starttime = GetTickCount();
    injector::scoped_unprotect xprotect(0x401000, 0x856E00 - 0x401000);
    for (uint32_t address = 0x401000; address < 0x856E00; address++)
    {
        for (auto& func : Patch_Funcs)
        {
            if (func(address))  address += 3;
        }
    }
    printf("Executed All EXEPatches - %dms\n",(GetTickCount() - starttime));
}