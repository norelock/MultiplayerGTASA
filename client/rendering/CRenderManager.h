#pragma once
class CRenderTemplate
{
public:
	virtual void Draw() = 0;
};

class CRenderManager
{

	CRenderManager();
public:

    static void Init();
	static void InitFont();
	static void Draw();
	static void DestroyFont();

	static void RenderText(const char *sz, RECT rect, DWORD dwColor);
	static void DrawProgressBar(CRect size, int value, CRGBA color, float max = 100.0f);
	static SIZE MeasureText(const char * szString);
	static void DrawLine(int x1, int y1, int x2, int y2, CRGBA color);
	static void DrawCircle(int x, int y, int radius, CRGBA color, bool fill = false);
	static void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, CRGBA color);
	static void DrawPolygon(RECT* points, int points_count, CRGBA color, bool fill = false);

	static void SetVideoMode(int videoMode);
    static HWND m_hwnd;

    static CImGui* m_pImGui;

	static IDirect3DDevice9 * m_pDevice;
	static ID3DXFont * m_pDXFont;
	static int m_iFontSize;

	static std::vector<CRenderTemplate*> m_pGuiContainer;
};

int __cdecl RsKeyboardEventHandler_Hook(RsEvent a1, void* a2);