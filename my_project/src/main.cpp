#include "MyDirect3D12/MyDirect3D12.hpp"

#include <Windows.h>

#include <iostream>

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);

	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// ウィンドウクラス初期化
HWND InitializeWindowClass(WNDCLASSEX *wnd_class_ex, const LPCWSTR &app_name, const LPCWSTR &title_name, const int &window_width, const int &window_height)
{
	// ウィンドウクラスの登録
	wnd_class_ex->cbSize = sizeof(WNDCLASSEX);
	wnd_class_ex->lpfnWndProc = reinterpret_cast<WNDPROC>(WindowProcedure);
	wnd_class_ex->lpszClassName = app_name;
	wnd_class_ex->hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(wnd_class_ex);

	// ウィンドウサイズの補正
	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow
	(
		wnd_class_ex->lpszClassName,
		title_name,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wnd_class_ex->hInstance,
		nullptr
	);

	return hwnd;
}
// ウィンドウクラスの破棄
void TerminateWindowClass(WNDCLASSEX* wnd_class_ex)
{
	// ウィンドウクラスの登録解除
	if (wnd_class_ex->lpszClassName) { UnregisterClass(wnd_class_ex->lpszClassName, wnd_class_ex->hInstance); }
}

int WINAPI WinMain(_In_ HINSTANCE h_instance, _In_opt_  HINSTANCE h_prev_instance, _In_ LPSTR lp_cmd_line, _In_ int n_show_cmd)
{
	// ウィンドウ設定
	const LPCWSTR app_name = L"DirectX12";
	const LPCWSTR title_name = L"MyDirectX12";
	const int window_width = 1280;
	const int window_height = 720;
	// ウィンドウオブジェクトの取得
	WNDCLASSEX wnd_class_ex = {};
	HWND hwnd = InitializeWindowClass(&wnd_class_ex, app_name, title_name, window_height, window_height);
	// ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);

	// D3D12初期化
	mla::MyDirect3D12 my_d3d{};

	// メイン処理
	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT) { break; }
	}

	// ウィンドウオブジェクトの解除
	TerminateWindowClass(&wnd_class_ex);

	return 0;
}