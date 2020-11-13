#pragma once

//pre-defs
#define HAS_SOCKLEN_T 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define LIBRG_CUSTOM_INCLUDES
#define ALTSA_CLIENT

#pragma warning(disable:4244)
#pragma warning(disable:4305)
#pragma warning(disable:4309)
#pragma warning(disable:4005)

//deps
#include "librg/zpl/zpl.h"
#include "librg/zpl/zpl_math.h"
#include "librg/enet.h"
#include "librg/librg.h"

#include "plugin.h"
#include "d3dx9.h"
#include <DbgHelp.h>

using namespace plugin;

//system
#include <iostream>
#include <ostream>
#include <fstream>
#include <stdlib.h>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <cmath>

//psdk
#include "RenderWare.h"
#include "CVector.h"
#include "CEntity.h"
#include "CStreaming.h"
#include "CModelInfo.h"
#include "CMonsterTruck.h"
#include "CQuadBike.h"
#include "CHeli.h"
#include "CPlane.h"
#include "CBmx.h"
#include "CTrailer.h"
#include "CBoat.h"
#include "CWorld.h"
#include "CTheScripts.h"
#include "CTimer.h"
#include "CPlayerPed.h"
#include "CFileLoader.h"
#include "CLoadingScreen.h"
#include "CWorld.h"
#include "CColModel.h"
#include "CMenuManager.h"
#include "CPopulation.h"
#include "CCamera.h"
#include "CWeaponInfo.h"
#include "CPed.h"
#include "CCivilianPed.h"
#include "CSprite.h"
#include "CGame.h"
#include "CPathFind.h"
#include "CCarCtrl.h"
#include "CCarEnterExit.h"
#include "CHud.h"
#include "CMessages.h"
#include "CReferences.h"
#include "CTaskComplexWanderStandard.h"
#include "CTaskComplexJump.h"
//#include "CTaskSimpleDuckToggle.h"
#include "CPathFind.h"
#include "C3dMarkers.h"
#include "CRadar.h"
#include "CWeather.h"
#include "CRestart.h"
#include "CTxdStore.h"

//MAX_PLAYERS_NUM for internal usage
//player on index 1 is buggy, reserverd for coop leftover in the game
#define MAX_PLAYERS_NUM 300

// dear imgui
#include "imgui.h"
#include "examples\imgui_impl_dx9.h"
#include "examples\imgui_impl_win32.h"

// render multiplayer code
#include "config.h"
#include "options/COptions.h"
#include "rendering/imgui/CImGui.h"
#include "rendering/CRenderManager.h"
#include "models/CModelManager.h"
#include "entities/sub/CWeaponManager.h"
#include "entities/CVehicleManager.h"
#include "entities/CPedManager.h"
#include "entities/CPlayerPedManager.h"
//#include "rendering/debug/CDebugScreen.h"
#include "rendering/ingame/CNameTags.h"
#include "rendering/ingame/CChat.h"

#include "tasks/CTaskSerializer.h"

//shared
#include "types.h"
#include "entities\CNetworkEntity.h"
#include "entities\CNetworkPlayer.h" 
#include "entities\CNetworkVehicle.h" 
#include "config.h"

//client
#include "networking/CNetworkManager.h"

//entities
#include "networking/entities/CClientPlayer.h"
#include "networking/entities/CClientVehicle.h"
#include "networking/entities/CNetworkEntityManager.h"


// scripting languages
// Lua
#include "scripting/lua/LuaScript.h"

//externals
extern CPlayerInfo CWorld__Players[MAX_PLAYERS_NUM];
extern CPad CPad__Pads[MAX_PLAYERS_NUM];

extern WNDPROC	orig_wndproc;
extern HWND		orig_wnd;

extern LRESULT CALLBACK wnd_proc(HWND wnd, UINT umsg, WPARAM wparam, LPARAM lparam);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern std::vector<bool(__cdecl*)(unsigned int)> Patch_Funcs; // bool(__cdecl*)(unsigned int Address)


void printStack(void);