#include "./MyD3D9.hpp"

MyD3D9::MyD3D9(const std::string& window_title, const int& width, const int& height) :
	m_window_width(width), m_window_height(height), m_window_title_name(window_title),
	m_hwnd{}, m_window_class{},
	m_p_direct3d(nullptr), m_p_direct3d_device(nullptr)
{
	// WindowClass登録
	m_window_class =
	{
		sizeof(WNDCLASSEX), CS_CLASSDC, MyD3D9::MessageProc,
		0L, 0L, GetModuleHandle(nullptr),
		nullptr, nullptr, nullptr,
		nullptr, m_window_class_name.c_str(), nullptr
	};
	RegisterClassEx(&m_window_class);

	// WindowApplication作成
	m_hwnd = CreateWindow
	(
		m_window_class_name.c_str(), m_window_title_name.c_str(), WS_OVERLAPPEDWINDOW,
		m_init_window_x, m_init_window_y, m_window_width, m_window_height,
		GetDesktopWindow(), nullptr, m_window_class.hInstance, nullptr
	);

	// Direct3D初期化
	InitDirect3D();

	// Show Window
	ShowWindow(m_hwnd, SW_SHOWDEFAULT);
	UpdateWindow(m_hwnd);
}

MyD3D9::~MyD3D9()
{
	// Direct3D: 解放処理
	CleanUp();

	// WindowAPI: 解放処理
	bool is_check = UnregisterClass(m_window_class_name.c_str(), m_window_class.hInstance);
	if (!is_check)
	{
		std::cerr << "ERROR " << __LINE__ << ": UnregisterClass returns 0." << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

// private

// コールバック: ウィンドウメッセージ
LRESULT WINAPI MyD3D9::MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		ValidateRect(hWnd, nullptr);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Direct3Dの初期化
void MyD3D9::InitDirect3D()
{
	// コンポーネント取得
	m_p_direct3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_p_direct3d)
	{
		std::cerr << "ERROR: Direct3DCreate9 is NULL." << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// インターフェース定義
	D3DPRESENT_PARAMETERS d3d_pp;
	// ウィンドウモード
	d3d_pp.Windowed = TRUE;
	// バックバッファサイズ
	d3d_pp.BackBufferWidth = m_window_width;
	d3d_pp.BackBufferHeight = m_window_height;
	// バックバッファ色数
	d3d_pp.BackBufferFormat = D3DFMT_UNKNOWN;
	// バックバッファ数
	d3d_pp.BackBufferCount = 1;
	// マルチサンプリング
	d3d_pp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3d_pp.MultiSampleQuality = 0;
	// スワッピング
	d3d_pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// ウィンドウハンドル指定
	d3d_pp.hDeviceWindow = nullptr;
	// 深度ステンシルバッファ
	d3d_pp.EnableAutoDepthStencil = TRUE;
	// ステンシルバッファフォーマット
	d3d_pp.AutoDepthStencilFormat = D3DFMT_D16;
	// バックバッファからフロントバッファへの転送オプション
	d3d_pp.Flags = 0;
	// リフレッシュレート
	d3d_pp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	// 書き換えタイミング
	d3d_pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

	// デバイス取得
	HRESULT is_device_create = m_p_direct3d->CreateDevice
	(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3d_pp, &m_p_direct3d_device
	);
	if (FAILED(is_device_create))
	{
		std::cerr << "ERROR " << __LINE__ << ": CreateDevice is failed." << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

// Direct3D 終了処理
void MyD3D9::CleanUp()
{
	if (m_p_direct3d_device) { m_p_direct3d_device->Release(); }
	if (m_p_direct3d) { m_p_direct3d->Release(); }
}

// public

// メイン処理
void MyD3D9::MainLoop()
{
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}
