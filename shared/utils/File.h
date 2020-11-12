#pragma once
#ifdef _WIN32

#pragma comment(lib, "version.lib")
#pragma comment(lib, "shlwapi.lib")

#include <Windows.h>
#include <Shlwapi.h>
#include <string>
#include "Process.h"

namespace File {
	static bool Exists(const std::wstring& path) {
		return PathFileExists(path.c_str());
	}

	static std::wstring GetCurrentDir() {
		wchar_t CurPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, CurPath);
		return CurPath;
	}

	static std::wstring GetModuleDir(HMODULE hmodule)
	{
		wchar_t cpath[MAX_PATH] = { 0 };
		GetModuleFileName(hmodule, cpath, MAX_PATH);
		std::wstring path = cpath;
		return path.substr(0, path.find_last_of(L"\\/"));
	}

	static std::wstring GetModuleName(const HMODULE module) {

		wchar_t fileName[MAX_PATH];
		GetModuleFileName(module, fileName, MAX_PATH);

		std::wstring fullPath = fileName;

		size_t lastIndex = fullPath.find_last_of(L"\\") + 1;
		return fullPath.substr(lastIndex, fullPath.length() - lastIndex);
	}

	static std::wstring GetModuleNameWithoutExtension(const HMODULE module) {

		const std::wstring fileNameWithExtension = GetModuleName(module);

		size_t lastIndex = fileNameWithExtension.find_last_of(L".");
		if (lastIndex == -1) {
			return fileNameWithExtension;
		}

		return fileNameWithExtension.substr(0, lastIndex);
	}
}
#endif
