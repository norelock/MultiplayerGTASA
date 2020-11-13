#include "main.h"
#include "patches/CGamePatches.h"

std::vector<bool(__cdecl*)(unsigned int)> Patch_Funcs;

class client 
{
	public:
		client()
		{
			DisableThreadLibraryCalls(GetModuleHandle(NULL));

			AllocConsole();
			freopen("CONOUT$", "w", stdout);

			printf(PROJECT_INFO);

			CGamePatches::Init();
			CVehicleManager::Init();
			CPlayerPedManager::Init();
			CPedManager::Init();
			CRenderManager::Init();
			CNetworkManager::Init();
			COptions::Init();
			LuaScript::Init();
		}
} _client;
