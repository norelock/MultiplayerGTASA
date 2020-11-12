#include "stdafx.h"

static LONG NTAPI HandleVariant(PEXCEPTION_POINTERS exceptionInfo)
{
	SetForegroundWindow(GetDesktopWindow());
	return (exceptionInfo->ExceptionRecord->ExceptionCode == STATUS_INVALID_HANDLE) ? EXCEPTION_CONTINUE_EXECUTION : EXCEPTION_CONTINUE_SEARCH;
}

void InvokeEntryPoint(void(*entryPoint)())
{
	// SEH call to prevent STATUS_INVALID_HANDLE
	__try
	{
		// and call the entry point
		entryPoint();
	}
	__except (HandleVariant(GetExceptionInformation()))
	{
		MessageBox(NULL, L"Could not launch GTA San Andreas", L"Launch error", MB_OK);
	}
}

CInjector::CInjector()
{
	WCHAR CurPath[MAX_PATH];
	GetCurrentDirectoryW(MAX_PATH, CurPath);

	path = CurPath;
	path.append(L"\\");
}

HWND __stdcall CreateWindowExAHook(
	DWORD     dwExStyle,
	LPCSTR    lpClassName,
	LPCSTR    lpWindowName,
	DWORD     dwStyle,
	int       X,
	int       Y,
	int       nWidth,
	int       nHeight,
	HWND      hWndParent,
	HMENU     hMenu,
	HINSTANCE hInstance,
	LPVOID    lpParam
)
{
	return CreateWindowExA(WS_EX_OVERLAPPEDWINDOW, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

bool CInjector::Run(const std::wstring & gameFolder, const std::wstring &altPath)
{
	memset(&siStartupInfo, 0, sizeof(siStartupInfo));
	memset(&piProcessInfo, 0, sizeof(piProcessInfo));
	siStartupInfo.cb = sizeof(siStartupInfo);

	SetCurrentDirectory(gameFolder.c_str());
	SetEnvironmentVariable(L"PATH", gameFolder.c_str());

	FILE* gameFile = _wfopen((gameFolder + L"\\gta_sa.exe").c_str(), L"rb");

	if (!gameFile)
	{
		MessageBox(NULL, (gameFolder + L"Could not find GTA San Andreas").c_str(), L"Launch error", MB_OK);
		return false;
	}

	// Find the file length and allocate a related buffer
	uint32_t length;

	fseek(gameFile, 0, SEEK_END);
	length = ftell(gameFile);

	std::vector<uint8_t> data(length);

	// Seek back to the start and read the file
	rewind(gameFile);
	fread(data.data(), 1, length, gameFile);

	// Close the file, and continue on
	fclose(gameFile);

	ExecutableLoader exeLoader(data.data()); 

    exeLoader.SetLibraryLoader([](const char* libName)
    {
        printf("Loading lib: %s\n", libName);
        std::string strLibName(libName);
        std::transform(strLibName.begin(), strLibName.end(), strLibName.begin(), ::tolower);
        if (strLibName == "vorbisfile.dll")
            return LoadLibraryW(L"libs\\vorbisFile.dll");
        HMODULE module = LoadLibraryA(libName);
        if (!module) {
            std::stringstream ss;
            ss << "Library \"" << libName << "\" not loaded!";
            MessageBoxA(NULL, ss.str().c_str(), "Panic!", 0);
            TerminateProcess(GetCurrentProcess(), 0);
        }
        return module;
    });

	exeLoader.SetFunctionResolver([](HMODULE module, const char* functionName) -> LPVOID
	{
		if (!HIWORD((DWORD)functionName))
		{
			uint16_t ordinalNumber = LOWORD((DWORD)functionName);
		}
		else {
			if (!_stricmp(functionName, "CreateWindowExA")) {
				return (LPVOID)CreateWindowExAHook;
			}
		}
		return (LPVOID)GetProcAddress(module, functionName);
	});

	exeLoader.LoadIntoModule(GetModuleHandle(nullptr));

	SetDllDirectory((altPath + L"\\libs").c_str());
	for (std::wstring lib : libs)
	{
		HANDLE altModule = LoadLibraryW(lib.c_str());
		if (!altModule) {
			std::wstringstream ss;
			ss << L"Library \"" << lib.c_str() << L"\" not loaded!";
			MessageBoxW(NULL, ss.str().c_str(), L"Panic!", 0);
			TerminateProcess(GetCurrentProcess(), 0);
		}
	}

	// Get the entry point
	auto entryPoint = reinterpret_cast<void(*)()>(exeLoader.GetEntryPoint());

	// Call the entry point
	AddVectoredExceptionHandler(0, HandleVariant);
	InvokeEntryPoint(entryPoint);
	/*
	if (!CreateProcess((folder + L"\\gta_sa.exe").c_str(), const_cast<LPWSTR>(path.c_str()), NULL, NULL, true, CREATE_SUSPENDED | PROCESS_QUERY_INFORMATION, NULL, folder.c_str(), &siStartupInfo, &piProcessInfo))
	{
		MessageBox(NULL, (folder + L"Could not launch GTA San Andreas").c_str(), L"Launch error", MB_OK);
		return false;
	}*/
	return true;
}

void CInjector::Inject()
{
	WaitUntilGameStarts();
	Sleep(1);

	HANDLE process = piProcessInfo.hProcess;

	SetDllPath(process, path + L"libs");

	for each (const std::wstring& lib in libs)
	{
		if (!Inject(process, lib))
		{
			MessageBox(NULL, L"Could not load alt:SA Client", L"alt:SA Load Error", MB_OK | MB_ICONERROR);
			return;
		}
	}

	ResumeThread(piProcessInfo.hThread);


	SetDllPath(process, L"");
	
	CloseHandle(process);
}

void CInjector::PushLibrary(const std::wstring& name)
{
	if (std::ifstream(name).bad())
		return;
	
	libs.push_back(name);
}

void CInjector::WaitUntilGameStarts()
{
	while (!Process::GetProcessIDByName(L"gta_sa.exe"));
}

bool CInjector::SetDllPath(HANDLE process, const std::wstring & _path)
{
	if (!process)
		return false;

	LPVOID SetDllDirectoryW_ = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "SetDllDirectoryW");
	LPVOID lpPath = VirtualAllocEx(process, NULL, (_path.length() + 1) * sizeof(WCHAR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(process, lpPath, _path.c_str(), (_path.length() + 1) * sizeof(WCHAR), NULL);
	HANDLE remoteThread_ = CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)SetDllDirectoryW_, lpPath, 0, NULL);
	WaitForSingleObject(remoteThread_, INFINITE);

	VirtualFreeEx(process, lpPath, (_path.length() + 1) * sizeof(WCHAR), MEM_RELEASE);
	CloseHandle(remoteThread_);

	return true;
}

bool CInjector::Inject(HANDLE process, const std::wstring& dllName)
{
	if (!process)
		return false;

	LPVOID LoadLibraryW_ = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
	LPVOID lpDllName = VirtualAllocEx(process, NULL, (dllName.length() + 1) * sizeof(WCHAR), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(process, lpDllName, dllName.c_str(), (dllName.length() + 1) * sizeof(WCHAR), NULL);
	HANDLE remoteThread_ = CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryW_, lpDllName, 0, NULL);

	WaitForSingleObject(remoteThread_, INFINITE);

	VirtualFreeEx(process, lpDllName, (dllName.length() + 1) * sizeof(WCHAR), MEM_RELEASE);
	CloseHandle(remoteThread_);

	return true;
}

CInjector::~CInjector()
{
}

