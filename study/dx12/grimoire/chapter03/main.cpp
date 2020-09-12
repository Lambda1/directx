#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <iostream>
#include <vector>
#include <string>

// グローバル変数
const int window_width = 800;
const int window_height = 600;
// D3D12
ID3D12Device* p_device = nullptr;
IDXGIFactory6* p_dxgi_factory = nullptr;
IDXGISwapChain4* p_swap_chain = nullptr;
ID3D12CommandAllocator* p_cmd_allocator = nullptr;
ID3D12CommandList* p_cmd_list = nullptr;
ID3D12CommandQueue* p_cmd_queue = nullptr;

// WinAPI
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// ウィンドウ破棄処理
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// デバッグ用関数
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	std::cout << format << " " << valist << std::endl;
	va_end(valist);
#endif
}

// DirectX12
void InitDirectX(HWND &hwnd)
{
	// アダプタ列挙
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&p_dxgi_factory))))
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}
	std::vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* p_tmp_adapter = nullptr;
	for (int i = 0; p_dxgi_factory->EnumAdapters(i, &p_tmp_adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.emplace_back(p_tmp_adapter);
	}
	// アダプタ検索
	IDXGIAdapter* p_adapter = nullptr;
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring str_desc = adesc.Description;
		std::wcout << str_desc << std::endl;
		if (str_desc.find(L"Intel") != std::string::npos)
		{
			if (!p_adapter) { p_adapter = adpt; }
		}
	}

	// デバイスオブジェクト生成
	D3D_FEATURE_LEVEL level;
	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_1 };
	for (auto lv : levels)
	{
		if (SUCCEEDED(D3D12CreateDevice(p_adapter, lv, IID_PPV_ARGS(&p_device))))
		{
			level = lv;
			break;
		}
	}
	if (!p_device)
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// コマンドアロケータ生成
	if (FAILED(p_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&p_cmd_allocator))))
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}
	// コマンドリスト生成
	if (FAILED(p_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p_cmd_allocator, nullptr, IID_PPV_ARGS(&p_cmd_list))))
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// コマンドキュー生成
	D3D12_COMMAND_QUEUE_DESC cmd_queu_desc = {};
	cmd_queu_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // タイムアウト無し
	cmd_queu_desc.NodeMask = 0; // アダプタ1つの場合は0
	cmd_queu_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmd_queu_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // コマンドリストに合わせる
	if (FAILED(p_device->CreateCommandQueue(&cmd_queu_desc, IID_PPV_ARGS(&p_cmd_queue))))
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// スワップチェーン生成
	DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
	swap_chain_desc.Width = window_width;
	swap_chain_desc.Height = window_height;
	swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.Stereo = false;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swap_chain_desc.BufferCount = 2;
	swap_chain_desc.Scaling = DXGI_SCALING_STRETCH; // バックバッファは伸縮可能
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // フリップ後は破棄
	swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ウィンドウ・フルスク切り替え
	if (FAILED(p_dxgi_factory->CreateSwapChainForHwnd(p_cmd_queue, hwnd, &swap_chain_desc, nullptr, nullptr, (IDXGISwapChain1**)&p_swap_chain)))
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// ディスクリプタヒープ生成
	D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
	heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heap_desc.NodeMask = 0;
	heap_desc.NumDescriptors = 2;
	heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ID3D12DescriptorHeap* rtv_heaps = nullptr;
	if (FAILED(p_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&rtv_heaps))))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}

	// スワップチェーンとメモリの紐づけ
	DXGI_SWAP_CHAIN_DESC swc_desc = {};
	if (FAILED(p_swap_chain->GetDesc(&swc_desc)))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}
	std::vector<ID3D12Resource*> back_buffers(swc_desc.BufferCount);
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv_heaps->GetCPUDescriptorHandleForHeapStart();
	for (int index = 0; index < swc_desc.BufferCount; ++index)
	{
		if (FAILED(p_swap_chain->GetBuffer(index, IID_PPV_ARGS(&back_buffers[index]))))
		{
			std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
		}
		handle.ptr += (index * p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		p_device->CreateRenderTargetView(back_buffers[index], nullptr, handle);
	}
}

#ifdef _DEBUG
int main()
#else
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
int main()
#endif
{
	// ウィンドウクラスの生成・登録
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = TEXT("DX12 Sample");
	w.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&w);

	// ウィンドウサイズの調整
	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウオブジェクトの生成
	HWND hwnd = CreateWindow
	(
		w.lpszClassName,
		TEXT("DX12 テスト"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr
	);

#ifdef _DEBUG
	DebugOutputFormatString("Show window test.");
#endif

	// D3D12の初期化
	InitDirectX(hwnd);

	// ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);
	
	// ループ
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

	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}