#include "stdafx.h"

LRESULT CALLBACK CLoadingWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CLoadingWindow& window = CLoadingWindow::Instance();

	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		Gdiplus::Graphics graphics(hdc);

		graphics.DrawImage(window.splash, 0, 0);

		Gdiplus::StringFormat format;
		format.SetAlignment(Gdiplus::StringAlignmentCenter);
		format.SetLineAlignment(Gdiplus::StringAlignmentFar);

		graphics.DrawString(window.upperText.c_str(), -1, window.font, Gdiplus::RectF(67, 347, 168, 25), &format, window.textBrush);

		graphics.FillRectangle(window.frontBrush, 78, 293, 144, 4);
		graphics.FillRectangle(window.backBrush, 78, 293, (int)round(144 * window.loadProgress), 4);

		EndPaint(hWnd, &ps);

		return 0;
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CLoadingWindow::SetProgress(float progress, const std::wstring& text)
{
	loadProgress = progress;
	upperText = text;

	RECT rect{ 0, 0, 300, 400 };
	InvalidateRect(hWnd, &rect, false);

	UpdateWindow(hWnd);
}

void CLoadingWindow::Init(HINSTANCE hInstance)
{
	hInstance = hInstance;

	loadProgress = 0.0;
	upperText = L"LOADING...";
	lowerText = L"";

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"_fivenet_launcher_class_";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	RegisterClassEx(&wcex);

	splash = Gdiplus::Bitmap::FromResource(hInstance, MAKEINTRESOURCEW(IDB_BITMAP1));

	backBrush = new Gdiplus::SolidBrush(Gdiplus::Color(43, 43, 43));
	frontBrush = new Gdiplus::SolidBrush(Gdiplus::Color(69, 107, 53));
	textBrush = new Gdiplus::SolidBrush(Gdiplus::Color(165, 165, 165));

	font = new Gdiplus::Font(&Gdiplus::FontFamily(L"Segoe UI"), 10, Gdiplus::FontStyle::FontStyleBold, Gdiplus::Unit::UnitPixel);

	RECT rect;
	GetClientRect(GetDesktopWindow(), &rect);
	rect.left = (rect.right / 2) - (300 / 2);
	rect.top = (rect.bottom / 2) - (400 / 2);

	hWnd = CreateWindow(wcex.lpszClassName, L"alt:SA Launcher",
		WS_POPUP,
		rect.left, rect.top, 300, 400, 0, 0, hInstance, 0);
}

void CLoadingWindow::Destroy()
{
	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
}

CLoadingWindow::CLoadingWindow()
{
	
}


CLoadingWindow::~CLoadingWindow()
{
	//delete splash;
	//delete backBrush;
	//delete frontBrush;
	//delete font;
}


