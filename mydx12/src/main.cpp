#include <Windows.h>

#include <string>

#include "./mydx12.hpp"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (msg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

HWND InitWin(const HINSTANCE& h_instance, const std::wstring &class_name, const int &win_width, const int &win_height, const std::wstring &win_name)
{
	WNDCLASSEX wc{};
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = h_instance;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = class_name.c_str();
	RegisterClassEx(&wc);

	DWORD dw_style = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
	RECT rect = {0, 0, win_width, win_height};
	AdjustWindowRect(&rect, dw_style, FALSE);

	HWND hwnd = CreateWindow
	(
		wc.lpszClassName,
		win_name.c_str(),
		dw_style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,
		nullptr,
		h_instance,
		nullptr
	);

	return hwnd;
}

int WINAPI WinMain(_In_ HINSTANCE h_instance, _In_opt_ HINSTANCE h_prev_instance, _In_ LPSTR lp_cmd_line, _In_ int m_cmd_show)
{
	// ウィンドウ初期化
	HWND hwnd = InitWin(h_instance, L"MyDirectX12", 640, 480, L"DirectX12");
	// ウィンドウ表示
	ShowWindow(hwnd, m_cmd_show);

	// DirectX12初期化
	mla::mydx12 mdx12{};
	mdx12.Init(hwnd);

	// メイン処理
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		mdx12.Render();
	}

	mdx12.Terminate();

	return 0;
}