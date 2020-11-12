#pragma once
class CLoadingWindow : public CSingleton<CLoadingWindow>
{
	HWND hWnd;
	HINSTANCE hInstance;
	Gdiplus::Image* splash;
	Gdiplus::SolidBrush* backBrush;
	Gdiplus::SolidBrush* frontBrush;
	Gdiplus::SolidBrush* textBrush;
	Gdiplus::Font* font;

	float loadProgress;
	std::wstring upperText;
	std::wstring lowerText;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
public:
	CLoadingWindow();
	~CLoadingWindow();

	HWND GetHwnd() const { return hWnd; }
	void SetProgress(float progress, const std::wstring& text);
	void Init(HINSTANCE hInstance);
	void Destroy();
};

