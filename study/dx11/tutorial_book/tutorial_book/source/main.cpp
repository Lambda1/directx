#include <Windows.h>
#include <d3d11.h>

/* グローバル変数 */
// ウィンドウ設定
HWND g_hWnd = nullptr;
LPCWSTR	w_name = L"Window";
const int w_width = 640, w_height = 480;
// D3D11
ID3D11Device* g_p_device;
ID3D11DeviceContext* g_p_device_context;
IDXGISwapChain* g_p_swap_chain;
ID3D11RenderTargetView* g_p_rtv;
ID3D11Texture2D* g_p_ds;
ID3D11DepthStencilView* g_p_dsv;

/* D3D11関係 */
// D3D11: 初期化
HRESULT InitDirect3D()
{
	// デバイス・スワップチェーンの作成
	DXGI_SWAP_CHAIN_DESC sd;

	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = w_width;
	sd.BufferDesc.Height = w_height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL feature_levels = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL* p_feature_level = nullptr;

	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &feature_levels, 1, D3D11_SDK_VERSION, &sd, &g_p_swap_chain, &g_p_device, p_feature_level, &g_p_device_context);

	// バックバッファのレンダーターゲットビュー(RTV)を作成
	ID3D11Texture2D* p_back;
	
	g_p_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&p_back);
	g_p_device->CreateRenderTargetView(p_back, nullptr, &g_p_rtv);
	p_back->Release();

	// デプスステンシルビュー(DSV)を作成
	D3D11_TEXTURE2D_DESC desc_depth;

	desc_depth.Width = w_width;
	desc_depth.Height = w_height;
	desc_depth.MipLevels = 1;
	desc_depth.ArraySize = 1;
	desc_depth.Format = DXGI_FORMAT_D32_FLOAT;
	desc_depth.SampleDesc.Count = 1;
	desc_depth.SampleDesc.Quality = 0;
	desc_depth.Usage = D3D11_USAGE_DEFAULT;
	desc_depth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc_depth.CPUAccessFlags = 0;
	desc_depth.MipLevels = 0;

	g_p_device->CreateTexture2D(&desc_depth, nullptr, &g_p_ds);
	g_p_device->CreateDepthStencilView(g_p_ds, nullptr, &g_p_dsv);

	// ビューポート設定
	D3D11_VIEWPORT vp;
	vp.Width = w_width;
	vp.Height = w_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	
	g_p_device_context->RSSetViewports(1, &vp);

	// レンダーターゲットビュー・デプスステンシルビューを設定
	g_p_device_context->OMSetRenderTargets(1, &g_p_rtv, g_p_dsv);

	return S_OK;
}

// ウィンドウプロシージャ関数
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0); break;
	default:
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

// メイン処理
void App()
{

}

// エントリーポイント
INT WINAPI WinMain(HINSTANCE h_inst, HINSTANCE h_prev_inst, LPSTR sz_str, INT i_cmd_show)
{
	// ウィンドウ作成
	WNDCLASSEX wc = { 0 };

	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = h_inst;
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
	wc.lpszClassName = w_name;

	if (!RegisterClassEx(&wc)) { return 1; }

	g_hWnd = CreateWindow(w_name, w_name, WS_OVERLAPPEDWINDOW, 0, 0, w_width, w_height, 0, 0, h_inst, 0);

	ShowWindow(g_hWnd, SW_SHOW);
	UpdateWindow(g_hWnd);

	// メッセージループ
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { DispatchMessage(&msg); }
		else
		{
			App();
		}
	}

	return 0;
}