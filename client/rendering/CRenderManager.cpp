#include "main.h"

HWND CRenderManager::m_hwnd;

CImGui* CRenderManager::m_pImGui;

IDirect3DDevice9 * CRenderManager::m_pDevice;
ID3DXFont * CRenderManager::m_pDXFont;
int CRenderManager::m_iFontSize;

std::vector<CRenderTemplate*> CRenderManager::m_pGuiContainer;

WNDPROC	orig_wndproc;
HWND	orig_wnd;

//RenderWare hooking
static auto& RwInitialized = *reinterpret_cast<bool *>(0xC920E8);
using RsEventHandler_t = RsEventStatus(__cdecl *)(int, std::intptr_t);
static auto RsEventHandler = reinterpret_cast<RsEventHandler_t>(0x619B60);
static auto& gGameState = *reinterpret_cast<std::uint32_t *>(0xC8D4C0);


LRESULT CALLBACK wnd_proc(HWND wnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	if (CRenderManager::m_pImGui->bInitialized)
	{
		if (CRenderManager::m_pImGui->bFocus)
		{
			ShowCursor(TRUE);
			CRenderManager::m_pImGui->io->MouseDrawCursor = true;

			if (ImGui_ImplWin32_WndProcHandler(wnd, umsg, wparam, lparam))
				return true;
		}
		else
		{
			ShowCursor(FALSE);
			CRenderManager::m_pImGui->io->MouseDrawCursor = FALSE;
		}
	}

	switch (umsg)
	{
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0);
		ExitProcess(-1);
		return 0;
		break;

	case WM_MOUSEMOVE:
		POINT ul, lr;
		RECT rect;
		GetClientRect(wnd, &rect);

		ul.x = rect.left;
		ul.y = rect.top;
		lr.x = rect.right;
		lr.y = rect.bottom;

		MapWindowPoints(wnd, nullptr, &ul, 1);
		MapWindowPoints(wnd, nullptr, &lr, 1);

		rect.left = ul.x;
		rect.top = ul.y;
		rect.right = lr.x;
		rect.bottom = lr.y;

		if (GetActiveWindow() == orig_wnd ? true : false)
			ClipCursor(&rect);
		break;

	case WM_MOUSEHOVER:
		break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		int vkey = (int)wparam;
		if (vkey == VK_F8)
		{
			RsGlobal.quit = 1;
		}
		if (vkey == VK_F9)
		{
            CRenderManager::m_pImGui->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
            CRenderManager::m_pImGui->bFocus = !CRenderManager::m_pImGui->bFocus;
            CRenderManager::m_pImGui->bConnectScreen = !CRenderManager::m_pImGui->bConnectScreen;
		}
		if (vkey == VK_F10)
		{
			CNetworkManager::AttemptConnect("127.0.0.1", 7766, "Player");
		}
		if (vkey == 'G')
		{
			CPlayerPedManager::ProcessEnterCarAsPassenger();
		}
       /* if (vkey == 'J')
        {
            
            CPlayerPed* player = FindPlayerPed(0);
            if (player)
            {
                eTaskType activeType = player->m_pIntelligence->m_TaskMgr.GetActiveTask()->GetId();
                if (activeType != TASK_COMPLEX_JUMP)
                    player->m_pIntelligence->m_TaskMgr.SetTask(new CTaskSimpleDuckToggle(-1), 3, false);
            }
        }
        if (vkey == 'K')
        {
            CPlayerPed* player = FindPlayerPed(0);
            if (player)
            {
                eTaskType activeType = player->m_pIntelligence->m_TaskMgr.GetActiveTask()->GetId();
                //eTaskType simplestActiveType = player->m_pIntelligence->m_TaskMgr.GetSimplestActiveTask()->GetId();

                if (activeType != TASK_COMPLEX_JUMP && player->m_pIntelligence->GetTaskDuck(true))
                {
                    CTaskSimpleDuckToggle duckToggle(0);
                    duckToggle.ProcessPed(player);                   
                }
                else if (activeType != TASK_COMPLEX_JUMP)
                {
                    player->m_pIntelligence->m_TaskMgr.SetTask(new CTaskComplexJump(0), 3, false);
                }
            }
        }*/
		break;
	}
	
	case WM_SIZE:
	{
		RwRect r = { 0 };
		r.w = LOWORD(lparam);
		r.h = HIWORD(lparam);

		if (RwInitialized && r.h > 0 && r.w > 0)
		{
			RsEventHandler(rsCAMERASIZE, reinterpret_cast<std::intptr_t>(&r));

			if (r.w != LOWORD(lparam) && r.h != HIWORD(lparam))
			{
				ReleaseCapture();

				WINDOWPLACEMENT wp;
				GetWindowPlacement(wnd, &wp);

				if (wp.showCmd == SW_SHOWMAXIMIZED)
					SendMessage(wnd, WM_WINDOWPOSCHANGED, 0, 0);
			}
		}
		break;
	}
	case WM_SIZING:
	{
		if (RwInitialized && gGameState == 9) {
			RsEventHandler(26, 1);
		}
		break;
	}
	}

	return CallWindowProc(orig_wndproc, wnd, umsg, wparam, lparam);
}

HWND(__cdecl* original_InitInstance)(HINSTANCE inst) = (HWND(__cdecl*)(HINSTANCE))0x745560;
HWND __cdecl InitInstance_Hook(HINSTANCE inst)
{
	HWND hwnd = original_InitInstance(inst);
	if (hwnd)
	{
		orig_wnd = hwnd;
		orig_wndproc = (WNDPROC)(UINT_PTR)SetWindowLong(hwnd, GWL_WNDPROC, (LONG)(UINT_PTR)wnd_proc);
	}
	return hwnd;
}

int(__cdecl* original_RsKeyboardEventHandler)(RsEvent, void*) = (int(__cdecl*)(RsEvent, void*))0x619560;
int __cdecl RsKeyboardEventHandler_Hook(RsEvent a1, void* a2)
{
	if (!CRenderManager::m_pImGui->bFocus && CRenderManager::m_pImGui->bInitialized) return original_RsKeyboardEventHandler(a1, a2);
	else return 0;
}

void Hooked_CPad__UpdateMouse(CPad* This)
{
	if (!CRenderManager::m_pImGui->bFocus && CRenderManager::m_pImGui->bInitialized) This->UpdateMouse();
}

void CRenderManager::Init()
{
    patch::ReplaceFunctionCall(0x7487A8, InitInstance_Hook);
	patch::ReplaceFunctionCall(0x541DD7, Hooked_CPad__UpdateMouse, true);

	//m_pGuiContainer.push_back(new CDebugScreen());
	m_pGuiContainer.push_back(new CNameTags());
	m_pGuiContainer.push_back(new CChat());
	CRenderManager::m_pImGui = new CImGui();

	Events::drawingEvent += []
	{
		CRenderManager::Draw();
	};
	Events::initRwEvent += [] 
	{
		CRenderManager::InitFont();
		
		if (CRenderManager::m_pImGui && !CRenderManager::m_pImGui->bInitialized) {
			CRenderManager::m_pImGui->Initialize(orig_wnd, CRenderManager::m_pDevice);
		}
	};
	Events::shutdownRwEvent += [] 
	{
		CRenderManager::DestroyFont();
		
		if (CRenderManager::m_pImGui && CRenderManager::m_pImGui->bInitialized)		{
			CRenderManager::m_pImGui->Destroy();
		}
	};
	Events::d3dLostEvent += [] 
	{
		CRenderManager::DestroyFont();

        if (CRenderManager::m_pImGui && CRenderManager::m_pImGui->bInitialized) {
            CRenderManager::m_pImGui->Destroy();
		}
	};
	Events::d3dResetEvent += [] 
	{
		CRenderManager::DestroyFont();
		CRenderManager::InitFont();

        if (CRenderManager::m_pImGui && !CRenderManager::m_pImGui->bInitialized) {
            CRenderManager::m_pImGui->Initialize(orig_wnd, CRenderManager::m_pDevice);
		}
	};
}

void CRenderManager::InitFont()
{
	m_pDevice = reinterpret_cast<IDirect3DDevice9 *>(RwD3D9GetCurrentD3DDevice());
	
	int screenWidth = screen::GetScreenWidth();

	if (screenWidth < 1024)
	{
		m_iFontSize = 14;
	}
	else if (screenWidth == 1024)
	{
		m_iFontSize = 16;
	}
	else if (screenWidth > 1024 && screenWidth <= 2048)
	{
		m_iFontSize = 18;
	}
	else if (screenWidth > 2048)
	{
		m_iFontSize = 20;
	}	
	D3DXCreateFont(m_pDevice, m_iFontSize, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial", &m_pDXFont);
}

void CRenderManager::DestroyFont()
{
	if (m_pDXFont)
	{
		m_pDXFont->Release();
		m_pDXFont = NULL;
	}
}

void CRenderManager::Draw()
{
	if (m_pDXFont)
	{
		int screenHeight = screen::GetScreenHeight();

		for (auto& it : m_pGuiContainer)
		{
		    it->Draw();
		}
		//RenderText("SAndbox", { 10, 10 }, 0xFFFFFFFF);
		m_pDXFont->DrawText(NULL, "multiplayer test", -1, new RECT{ 2, screenHeight - 14 }, DT_NOCLIP | DT_LEFT, 0xFFFFFFFF);
	}
	
    if (CRenderManager::m_pImGui && CRenderManager::m_pImGui->bInitialized)
        CRenderManager::m_pImGui->Draw();
}

void CRenderManager::RenderText(const char *sz, RECT rect, DWORD dwColor)
{
	//Black outline
	m_pDXFont->DrawText(NULL, sz, -1, new RECT{ rect.left - 1, rect.top }, DT_NOCLIP | DT_LEFT, 0xFF000000);
	m_pDXFont->DrawText(NULL, sz, -1, new RECT{ rect.left + 1, rect.top }, DT_NOCLIP | DT_LEFT, 0xFF000000);
	m_pDXFont->DrawText(NULL, sz, -1, new RECT{ rect.left, rect.top - 1 }, DT_NOCLIP | DT_LEFT, 0xFF000000);
	m_pDXFont->DrawText(NULL, sz, -1, new RECT{ rect.left, rect.top + 1 }, DT_NOCLIP | DT_LEFT, 0xFF000000);

	m_pDXFont->DrawText(NULL, sz, -1, &rect, DT_NOCLIP | DT_LEFT, dwColor);
}

SIZE CRenderManager::MeasureText(const char * szString)
{
	RECT rect;
	SIZE ret;

	m_pDXFont->DrawText(0, szString, -1, &rect, DT_CALCRECT | DT_LEFT, 0xFF000000);
	ret.cx = rect.right - rect.left;
	ret.cy = rect.bottom - rect.top;

	return ret;
}

void CRenderManager::DrawProgressBar(CRect size, int value, CRGBA color, float max)
{
	CRGBA darkbg = color;
	if (color.r < 65)darkbg.r = 0;
	else darkbg.r = color.r - 65;
	if (color.g < 65)darkbg.g = 0;
	else darkbg.g = color.g - 65;
	if (color.b < 65)darkbg.b = 0;
	else darkbg.b = color.b - 65;

	CSprite2d::DrawRect(CRect(size.left, size.top, size.right, size.bottom), CRGBA(0, 0, 0, 255));
	CSprite2d::DrawRect(CRect(size.left + 1, size.top + 1, size.right - 1, size.bottom - 1), darkbg);

	int len = (size.right - size.left);
	CSprite2d::DrawRect(CRect(size.left + 1, size.top + 1, size.left + ceil((len / max)*value) - 1, size.bottom - 1), color);
}


void CRenderManager::DrawLine(int x1, int y1, int x2, int y2, CRGBA color)
{
	RwIm2DVertex vertex[2];
	vertex[0].x = x1;
	vertex[0].y = y1;
	vertex[1].x = x2;
	vertex[1].y = y2;
	vertex[0].emissiveColor = color.ToInt();
	vertex[1].emissiveColor = color.ToInt();

	RwIm2DRenderLine(vertex, 2, 0, 1);
}

void CRenderManager::DrawCircle(int x, int y, int radius, CRGBA color, bool fill)
{
	int points = 360;
	RwIm2DVertex* pVertex = new RwIm2DVertex[points + 1];
	for (int i = 0; i <= points; i++)
		pVertex[i] =
	{
		x + radius * cos(D3DX_PI * (i / (points / 2.0f))),
		y - radius * sin(D3DX_PI * (i / (points / 2.0f))),
		0.0f, 1.0f,
		color.ToInt(),
		0.0f, 0.0f
	};
	int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0, v6 = 0, v7 = 0, v8 = 0, v9 = 0, v10 = 0;

	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATETEXTURERASTER, (void*)&v1);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATESRCBLEND, (void*)&v2);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEDESTBLEND, (void*)&v3);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEFOGENABLE, (void*)&v4);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATETEXTUREFILTER, (void*)&v5);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATESHADEMODE, (void*)&v6);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEZTESTENABLE, (void*)&v7);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEZWRITEENABLE, (void*)&v8);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)&v9);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEALPHATESTFUNCTION, (void*)&v10);

	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)0);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESRCBLEND, (void*)5);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)6);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)0);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)2);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void*)1);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)0);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)1);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)1);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void*)8);

	if(fill)RwIm2DRenderPrimitive(RwPrimitiveType::rwPRIMTYPETRIFAN, pVertex, points + 1);
	else RwIm2DRenderPrimitive(RwPrimitiveType::rwPRIMTYPETRISTRIP, pVertex, points + 1);

	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)v1);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESRCBLEND, (void*)v2);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)v3);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)v4);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)v5);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void*)v6);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)v7);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)v8);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)v9);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void*)v10);
}

void CRenderManager::DrawPolygon(RECT* points, int points_count, CRGBA color, bool fill)
{
	RwIm2DVertex* pVertex = new RwIm2DVertex[points_count];
	for (int i = 0; i <= points_count; i++)
		pVertex[i] =
	{
		(RwReal)points[i].left,
		(RwReal)points[i].top,
		0.0f, 1.0f,
		color.ToInt(),
		0.0f, 0.0f
	};
	int v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0, v6 = 0, v7 = 0, v8 = 0, v9 = 0, v10 = 0;

	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATETEXTURERASTER, (void*)&v1);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATESRCBLEND, (void*)&v2);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEDESTBLEND, (void*)&v3);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEFOGENABLE, (void*)&v4);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATETEXTUREFILTER, (void*)&v5);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATESHADEMODE, (void*)&v6);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEZTESTENABLE, (void*)&v7);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEZWRITEENABLE, (void*)&v8);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)&v9);
	RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEALPHATESTFUNCTION, (void*)&v10);

	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)0);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESRCBLEND, (void*)5);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)6);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)0);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)2);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void*)1);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)0);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)1);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)1);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void*)8);

	if(!fill)RwIm2DRenderPrimitive(RwPrimitiveType::rwPRIMTYPEPOLYLINE, pVertex, points_count);
	else RwIm2DRenderPrimitive(RwPrimitiveType::rwPRIMTYPETRILIST, pVertex, points_count);

	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)v1);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESRCBLEND, (void*)v2);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)v3);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)v4);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)v5);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void*)v6);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)v7);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)v8);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)v9);
	RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEALPHATESTFUNCTION, (void*)v10);
}


void CRenderManager::DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, CRGBA color)
{
	RwIm2DVertex vertex[3];

	vertex[0].x = x1;
	vertex[0].y = y1;
	vertex[1].x = x2;
	vertex[1].y = y2;
	vertex[2].x = x3;
	vertex[2].y = y3;
	vertex[0].emissiveColor = color.ToInt();
	vertex[1].emissiveColor = color.ToInt();
	vertex[2].emissiveColor = color.ToInt();

	RwIm2DRenderTriangle(vertex, 3, 0, 1, 2);
}

void CRenderManager::SetVideoMode(int videoMode)
{
	DWORD address = 0x745C70;
	_asm
	{
		push    videoMode
		call    address
		add     esp, 4
	}
}
