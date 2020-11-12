#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <fstream>
#include <gdiplus.h>

#include <cmath>
#include <string>
#include <vector>
#include <thread>
#include <codecvt>

#include <TlHelp32.h>
#include <Shellapi.h>

#include "resource.h"

//json lib
#include "json.hpp"

// shared
#include "CSingleton.h"
#include "utils/File.h"
#include "utils/Process.h"

// launcher
#include "launcher/CConfig.h"
#include "launcher/CInjector.h"
#include "launcher/ExecutableLoader.h"

//windows
#include "windows/CLoadingWindow.h"

#include "Launcher.h"
