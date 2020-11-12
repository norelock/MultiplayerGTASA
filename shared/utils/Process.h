#pragma once
#ifdef _WIN32

#include <Windows.h>
#include <winnt.h>
#include <tlhelp32.h>
#include <cstdint>
#include "String.h"

namespace Process {
#pragma region WINAPI
	// Does NOT close handle!
	static DWORD GetProcessIDByName(LPCWSTR name)
	{
		DWORD pid = 0;

		// Create toolhelp snapshot.
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 process;
		ZeroMemory(&process, sizeof(process));
		process.dwSize = sizeof(process);

		// Walkthrough all processes.
		if (Process32First(snapshot, &process))
		{
			do
			{
				if (!wcscmp(process.szExeFile, name))
				{
					pid = process.th32ProcessID;
					break;
				}
			} while (Process32Next(snapshot, &process));
		}

		CloseHandle(snapshot);

		return pid;
	}

	static HMODULE GetModuleHandleE(LPCWSTR modulename = nullptr) {
		HMODULE module;
#ifdef _DEBUG
		if (!GetModuleHandleEx((DWORD)nullptr, modulename, &module) || module == nullptr) {
			printf("ERROR WHILE GETTING MODULE HANDLE: %d", GetLastError());
			//std::wstring out = L"ERROR WHILE GETTING MODULE HANDLE: " + WString::from_number(GetLastError()) + L" | MODULE HANDLE: " + WString::from_number((uint64_t)module);
			//MessageBox(NULL, out.c_str(), L"ERROR", MB_OK);
			//TerminateProcess(NULL, 1);
		}
#endif
		return module;
	}

	static uint64_t GetModuleBase(LPCWSTR modulename = nullptr) {
		return (uint64_t)GetModuleHandleE(modulename);
	}
#pragma endregion

#pragma region NTAPI
	typedef LONG(NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
	typedef LONG(NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);

	/* Does not close the handle */
	static void SuspendByHandle(HANDLE process)
	{
		NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(GetModuleHandleE(L"ntdll"), "NtSuspendProcess");
		pfnNtSuspendProcess(process);
	}

	/* Does not seem to work */
	static void ResumeByHandle(HANDLE process)
	{
		NtResumeProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(GetModuleHandleE(L"ntdll"), "NtResumeProcess");
		pfnNtResumeProcess(process);
	}

	static void Suspend() {
		HANDLE process = GetCurrentProcess();
		SuspendByHandle(process);
		CloseHandle(process);
	}

	static void Resume() {
		HANDLE process = GetCurrentProcess();
		ResumeByHandle(process);
		CloseHandle(process);
	}
#pragma endregion
}

#endif