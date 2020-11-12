#pragma once

class CInjector : public CSingleton<CInjector>
{
	STARTUPINFO siStartupInfo;
	PROCESS_INFORMATION piProcessInfo;
	std::vector<std::wstring> libs;
	std::wstring path;

	void WaitUntilGameStarts();
	bool SetDllPath(HANDLE process, const std::wstring& _path);
	bool Inject(HANDLE process, const std::wstring& dllName);

public:
	CInjector();
	~CInjector();

	bool Run(const std::wstring& gameFolder, const std::wstring& altPath);
	void Inject();
	void PushLibrary(const std::wstring& path);
};