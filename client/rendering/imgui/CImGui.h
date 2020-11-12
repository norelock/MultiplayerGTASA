#pragma once
class CImGui
{
public:
	CImGui() {};
	~CImGui() {};

	bool bInitialized	= false;
	bool bFocus			= false;
	bool bShowFPS		= true;
	bool bConnectScreen = false;

	HWND hwnd					= nullptr;
	IDirect3DDevice9* device	= nullptr;

	ImGuiIO* io			= nullptr;

	void Initialize(HWND hwnd, IDirect3DDevice9* device);
	void Destroy();
	void Draw();
};

