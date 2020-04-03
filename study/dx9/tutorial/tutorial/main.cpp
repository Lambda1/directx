#include <iostream>
#include <string>

#include <windows.h>
#include <d3d9.h>

// IDirect3D9: デバイス作成や情報取得機能を提供
IDirect3D9* p_direct3d = nullptr;
// IDirect3DDevice9: デバイス
IDirect3DDevice9* p_direct3d_device = nullptr;

HRESULT InitD3D(HWND hWnd)
{
	// device初期化
	p_direct3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (p_direct3d == nullptr) { return E_FAIL; }

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	// スクリーンモード
	d3dpp.Windowed = TRUE;
	// ダブルバッファリング時のswap動作
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// バックバッファの色数
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	// CreateDevice
	// 使用するグラフィックアダプタ, ハードウェア描画, アプリケーションハンドル, デバイス動作フラグ(ハード, ソフト), デバイスポインタ, 初期化デバイス
	HRESULT result = p_direct3d->CreateDevice
	(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &p_direct3d_device
	);

	if (FAILED(result)) { return E_FAIL; }
	return S_OK;
}

// 終了処理
void Cleanup()
{
	// Direct3D: 解放処理
	if (p_direct3d_device != nullptr){ p_direct3d_device->Release(); }
	if (p_direct3d != nullptr) { p_direct3d->Release(); }
}

void Render()
{
	if (p_direct3d_device == nullptr)
	{
		return;
	}

	// 画面を白クリア
	p_direct3d_device->Clear
	(
		0, nullptr, D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0
	);

	// Begin the scene
	if (SUCCEEDED(p_direct3d_device->BeginScene()))
	{
		p_direct3d_device->EndScene();
	}

	// Present the backbuffer contents to the display
	p_direct3d_device->Present(NULL, NULL, NULL, NULL);
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// main loop
	switch (msg)
	{
	// WM_DESTROY: ウィンドウ破棄時
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
	
	// WM_PAINT: 最初にウィンドウが表示されたとき, ウィンドウを動かしたとき などにポスト
	case WM_PAINT:
		Render();
		ValidateRect(hWnd, NULL);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
{
	// 定数
	const std::string window_title = "Direct3D Tutorial";
	const std::string window_name = "Direct3D Tutorial 01";

	// Window classの登録
	WNDCLASSEX window_class = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, window_title.c_str(), nullptr };
	RegisterClassEx(&window_class);

	// window applicationの作成.
	constexpr int default_x = 100, default_y = 100;
	constexpr int width = 300, height = 300;
	HWND hWnd = CreateWindow(window_title.c_str(), window_name.c_str(), WS_OVERLAPPEDWINDOW, default_x, default_y, width, height, GetDesktopWindow(), nullptr, window_class.hInstance, nullptr);

	// Initialize Direct3D
	HRESULT init_direct3d_results = InitD3D(hWnd);
	if (FAILED(init_direct3d_results)) { return 1; }

	// Show the window
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	// Enter the message loop
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass("D3D Tutorial", window_class.hInstance);

	return 0;
}