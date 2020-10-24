#include "Application.hpp"

// Window Procedure
LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lpawam)
{
	// ウィンドウ破棄
	if (msg == WM_DESTROY)
	{
		// プログラム終了通知
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lpawam);
}

Application::Application():
	m_app_class_name(L"DirectX12_GRIMOIRE"),
	m_window_width(960), m_window_height(720), m_window_title(L"DirectX12"),
	m_hwnd{}, m_wnd_class{}
{

}

Application::~Application()
{

}

// public

// インスタンス生成
Application& Application::Instance()
{
	static Application instance;
	return instance;
}

// 初期化
void Application::Init()
{
	CreateGameWindow();
	ShowWindow(m_hwnd, SW_SHOW);

	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			break;
		}
	}
}

// メイン処理
void Application::Run()
{

}

// 終了処理
void Application::Terminate()
{
	// ウィンドウクラス解除
	UnregisterClass(m_wnd_class.lpszClassName, m_wnd_class.hInstance);
}

// private

// Window生成
void Application::CreateGameWindow()
{
	// ウィンドウクラス登録
	m_wnd_class.cbSize = sizeof(WNDCLASSEX);
	m_wnd_class.lpfnWndProc = static_cast<WNDPROC>(WndProc);
	m_wnd_class.lpszClassName = m_app_class_name.c_str();
	m_wnd_class.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&m_wnd_class);

	// ウィンドウサイズ変更
	RECT wrc = { 0, 0, m_window_width, m_window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	
	m_hwnd = CreateWindow
	(
		m_wnd_class.lpszClassName,
		m_window_title.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		m_wnd_class.hInstance,
		nullptr
	);
}
