#include "main.h"

void CImGui::Initialize(HWND hwnd, IDirect3DDevice9* device)
{
	if (!bInitialized) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		io = &ImGui::GetIO(); (void)io;
		io->IniFilename = NULL;
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX9_Init(device);
		ImGui::StyleColorsDark();

		bInitialized	= true;
		bFocus			= false;
	}
}

void CImGui::Destroy()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	bInitialized = false;
	bFocus = false;
	bShowFPS = false;
}

char Nickname[MAX_PLAYER_NAME];
char IP[16] = "127.0.0.1";
int Port = 7766;

void CImGui::Draw()
{
	if (!this->bConnectScreen)
	{
		if (CRenderManager::m_pImGui->bFocus)
		{
			CRenderManager::m_pImGui->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
			CRenderManager::m_pImGui->bFocus = false;
		}
	}
	if (this->bConnectScreen)
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		ImGui::SetNextWindowPosCenter();
		ImGui::Begin("alternative San Andreas multiplayer ", &this->bConnectScreen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavInputs);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("\tWelcome to alt:SA v0.0.1\n\t\t  - Alpha Version - ");
		ImGui::Separator();

		ImGui::InputText("Nickname", Nickname, MAX_PLAYER_NAME, 0, NULL, Nickname);
		ImGui::InputText("IP", IP, 16, 0, NULL, IP);
		ImGui::InputInt("Port", &Port);

		if (ImGui::Button("Connect"))
		{
			CNetworkManager::AttemptConnect(IP, Port, Nickname);
			this->bConnectScreen = false;
			CRenderManager::m_pImGui->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
			CRenderManager::m_pImGui->bFocus = false;
		}

		ImGui::End();

		ImGui::EndFrame();

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
}