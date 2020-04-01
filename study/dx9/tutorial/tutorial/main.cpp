#include <string>

#include <windows.h>
#include <d3d9.h>

// IDirect3D9: デバイス作成や情報取得機能を提供
IDirect3D9 *g_pD3D = nullptr;
// IDirect3DDevice9: デバイス
IDirect3DDevice9 *g_pd3dDevice = nullptr;

HRESULT InitD3D(HWND hWnd)
{
	// device初期化
	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (g_pD3D == nullptr) { return E_FAIL; }

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	// device
	HRESULT result = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice);
	if (FAILED(result))
	{
		return E_FAIL;
	}

	return S_OK;
}

void Cleanup()
{
    if( g_pd3dDevice != NULL )
        g_pd3dDevice->Release();

    if( g_pD3D != NULL )
        g_pD3D->Release();
}

void Render()
{
    if( NULL == g_pd3dDevice )
        return;

    // Clear the backbuffer to a blue color
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );

    // Begin the scene
    if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
    {
        // Rendering of scene objects can happen here

        // End the scene
        g_pd3dDevice->EndScene();
    }

    // Present the backbuffer contents to the display
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_DESTROY:
            Cleanup();
            PostQuitMessage( 0 );
            return 0;

        case WM_PAINT:
            Render();
            ValidateRect( hWnd, NULL );
            return 0;
    }

    return DefWindowProc( hWnd, msg, wParam, lParam );
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
    if( SUCCEEDED( InitD3D( hWnd ) ) )
    {
        // Show the window
        ShowWindow( hWnd, SW_SHOWDEFAULT );
        UpdateWindow( hWnd );

        // Enter the message loop
        MSG msg;
        while( GetMessage( &msg, nullptr, 0, 0 ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    UnregisterClass("D3D Tutorial", window_class.hInstance );

	return 0;
}