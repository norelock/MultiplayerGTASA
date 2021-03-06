#include "stdafx.h"
#include <shlobj.h>

#include "Log.h"

#pragma comment(lib, "version.lib")

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		std::string tmp = (const char *)lpData;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}

std::wstring gtadrm;
int gtaversion;
bool supported;
std::wstring gtapath;

static int GetGTAVersion() 
{
	HMODULE hModule = LoadLibraryEx((gtapath + L"\\gta_sa.exe").c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);
	if (!hModule)
		throw std::exception("hModule was null");

	char* sVersionFile = new char[512];
	if (GetModuleFileNameA(hModule, sVersionFile, 512) == 0)
		return NULL;

	if (sVersionFile == NULL)
	{
		throw std::exception("GetModulePath returned 0");
		return NULL;
	}

	// Version Info Size
	DWORD dwVersionInfoSize = GetFileVersionInfoSizeA(sVersionFile, NULL);
	if (dwVersionInfoSize == NULL)
	{
		throw std::exception("GetFileVersionInfoSize returned 0");
		return NULL;
	}

	// Version Info
	VS_FIXEDFILEINFO* pFileInfo = (VS_FIXEDFILEINFO*) new BYTE[dwVersionInfoSize];
	DWORD dwVersionHandle = NULL;
	if (!GetFileVersionInfoA(sVersionFile, dwVersionHandle, dwVersionInfoSize, pFileInfo))
	{
		throw std::exception("GetFileVersionInfo failed");

		// Cleanup
		delete[] pFileInfo;

		return NULL;
	}

	// Query
	UINT uiFileInfoLength = 0;
	VS_FIXEDFILEINFO* pVersionInfo = NULL;
	if (!VerQueryValueA(pFileInfo, "\\", (LPVOID*)&pVersionInfo, &uiFileInfoLength) || uiFileInfoLength == 0)
	{
		throw std::exception("VerQueryValue failed");

		// Cleanup
		delete[] pFileInfo;
		if (pVersionInfo != NULL)
			delete pVersionInfo;

		return NULL;
	}

	// Signature
	if (pVersionInfo->dwSignature != 0xFEEF04BD)
	{
		throw std::exception("Signature mismatch, got %X");

		// Cleanup
		delete[] pFileInfo;
		if (pVersionInfo != NULL)
			delete pVersionInfo;

		return NULL;
	}

	int ver = (pVersionInfo->dwFileVersionLS >> 16) & 0xffff;

	// Cleanup
	delete[] pFileInfo;
	FreeLibrary(hModule);

	return ver;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	TCHAR _altPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, _altPath);
	std::wstring altPath = std::wstring(_altPath);

	// Creating the logger instance & making out file log
	auto& logger = Log::Instance();
	logger.AddOut(new Log::FileStream(altPath + L"\\logs\\launcher.log"));

	// Creating window instance
	auto& window = CLoadingWindow::Instance();

	// Creating config instance
	auto& config = CConfig::Instance();

	// Creating injector instance
	auto& injector = CInjector::Instance();

	std::wstringstream ss;
	bool missing = false;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Initializing window
	window.Init(hInstance);

	// Open loading window & update it
	ShowWindow(window.GetHwnd(), nCmdShow);
	UpdateWindow(window.GetHwnd());

	// Load required libraries from defined list and push to injector
	for (std::wstring& file : requiredFiles)
	{
		if (!File::Exists(file))
		{
			if (!missing)
			{
				ss << L"The following files are missing:" << std::endl;
				missing = true;
			}

			ss << L"  - " << file << std::endl;
		}

		injector.PushLibrary(file);
		logger.Put(logger.Time, " Pushed library as memory", file.c_str(), logger.Endl);
	}

	// If files don't exists or are missing then we throwing the error window
	if (missing)
	{
		window.Destroy();

		MessageBox(NULL, ss.str().c_str(), L"Error while running client", MB_OK | MB_ICONERROR);
		return 1;
	}

	// Load config
	if (!config.Load(L"config.json"))
	{
		TCHAR path[MAX_PATH];

		BROWSEINFO bi = { 0 };
		bi.lpszTitle = L"Select your installation of Grand Theft Auto San Andreas...";
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM)L"";

		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

		if (pidl != 0)
		{
			SHGetPathFromIDList(pidl, path);

			IMalloc * imalloc = 0;
			if (SUCCEEDED(SHGetMalloc(&imalloc)))
			{
				imalloc->Free(pidl);
				imalloc->Release();
			}

			config.SetPath(path);
			config.Save(L"config.json");
		}
	}

	// Getting game path from config file
	gtapath = config.GetPath();

	std::thread([&] {
		MSG msg;

		while (GetMessage(&msg, NULL, 0, 0) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}).detach();

	window.Destroy();
	Gdiplus::GdiplusShutdown(gdiplusToken);

	// Run game and inject libraries
	if (!injector.Run(config.GetPath(), altPath))
		MessageBox(NULL, L"Failed to load Render Multiplayer client\nSee client.log", L"Load Error", MB_OK | MB_ICONERROR);
	
	return 0;
}