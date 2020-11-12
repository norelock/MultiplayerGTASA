#pragma once

//config
//#define GUI_VERSION

//pre-defs
#define HAS_SOCKLEN_T 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define LIBRG_CUSTOM_INCLUDES
#define _CRT_SECURE_NO_WARNINGS
#define ALTSA_SERVER

#pragma warning(disable:4244)
#pragma warning(disable:4305)
#pragma warning(disable:4309)
#pragma warning(disable:4005)

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

//deps
#include <librg/zpl/zpl.h>
#include <librg/zpl/zpl_math.h>
#include <librg/enet.h>
#include <librg/librg.h>

//shared
#include <config.h>
#include <types.h>
#include <entities/CNetworkEntity.h>
#include <entities/CNetworkPlayer.h>
#include <entities/CNetworkVehicle.h>

//server
#include "CLog.h"
#include "CNetworkManager.h"

//entities
#include "CServerPlayer.h"
#include "CNetworkEntityManager.h"



#ifdef GUI_VERSION
#include "imgui.h"
#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_dx11.h"

#pragma comment(lib, "dwmapi.lib")
#include "Dwmapi.h"
#include <d3d11.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#endif
