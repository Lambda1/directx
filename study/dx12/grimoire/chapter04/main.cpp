#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

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
ID3D12GraphicsCommandList* p_cmd_list = nullptr;
ID3D12CommandQueue* p_cmd_queue = nullptr;
std::vector<ID3D12Resource*> back_buffers;
ID3D12DescriptorHeap* rtv_heaps = nullptr;
ID3D12Fence* p_fence = nullptr;
UINT64 fence_val = 0;

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
void InitDirectX(HWND& hwnd)
{
	// アダプタ列挙
#ifdef _DEBUG
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&p_dxgi_factory))))
#else
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&p_dxgi_factory))))
#endif
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
	back_buffers.resize(swc_desc.BufferCount);
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

	// フェンス生成
	if (FAILED(p_device->CreateFence(fence_val, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&p_fence))))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}
}

void EnableDebugLayer()
{
	ID3D12Debug* debug_layer = nullptr;
	HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_layer));
	if (FAILED(hr))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}

	debug_layer->EnableDebugLayer();
	debug_layer->Release();
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
#ifdef _DEBUG
	EnableDebugLayer();
#endif
	InitDirectX(hwnd);

	// ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);

	// 変数定義
	DirectX::XMFLOAT3 vertices[] =
	{
		{-1.0f, -1.0f, 0.0f},
		{-1.0f,  1.0f, 0.0f},
		{ 1.0f, -1.0f, 0.0f}
	};

	// 頂点バッファ生成
	D3D12_HEAP_PROPERTIES heap_prop = {};
	heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC res_desc = {};
	res_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	res_desc.Width = sizeof(vertices);
	res_desc.Height = 1;
	res_desc.DepthOrArraySize = 1;
	res_desc.MipLevels = 1;
	res_desc.Format = DXGI_FORMAT_UNKNOWN;
	res_desc.SampleDesc.Count = 1;
	res_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	res_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vert_buff = nullptr;
	if (FAILED(p_device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vert_buff))))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}

	// 頂点データのマップ
	DirectX::XMFLOAT3* vert_map = nullptr;
	if (FAILED(vert_buff->Map(0, nullptr, (void**)&vert_map)))
	{
		std::cout << __LINE__ << std::endl; std::exit(EXIT_FAILURE);
	}
	std::copy(std::begin(vertices), std::end(vertices), vert_map);
	vert_buff->Unmap(0, nullptr);

	// ループ
	HRESULT hr = FALSE;
	MSG msg = {};
	while (true)
	{
		// WinAPI
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			break;
		}

		// D3D12
		// コマンドクリア
		UINT bb_idx = p_swap_chain->GetCurrentBackBufferIndex();
		
		// バリア設定
		D3D12_RESOURCE_BARRIER barrier_desc = {};
		barrier_desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier_desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier_desc.Transition.pResource = back_buffers[bb_idx];
		barrier_desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier_desc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier_desc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		p_cmd_list->ResourceBarrier(1, &barrier_desc);
		
		// RT設定
		auto rtv_h = rtv_heaps->GetCPUDescriptorHandleForHeapStart();
		rtv_h.ptr += bb_idx * p_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		p_cmd_list->OMSetRenderTargets(1, &rtv_h, false, nullptr);
		
		// RTクリア
		float clear_color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
		p_cmd_list->ClearRenderTargetView(rtv_h, clear_color, 0, nullptr);
		
		barrier_desc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier_desc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		p_cmd_list->ResourceBarrier(1, &barrier_desc);
		
		// クローズ
		p_cmd_list->Close();

		// コマンドリスト実行
		ID3D12CommandList* cmd_lists[] = { p_cmd_list };
		p_cmd_queue->ExecuteCommandLists(1, cmd_lists);

		// フェンス処理
		p_cmd_queue->Signal(p_fence, ++fence_val);
		if (p_fence->GetCompletedValue() != fence_val)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr); // イベントハンドル取得
			p_fence->SetEventOnCompletion(fence_val, event);

			WaitForSingleObject(event, INFINITE); // イベント終了待ち

			CloseHandle(event);
		}

		// 解放
		p_cmd_allocator->Reset();
		p_cmd_list->Reset(p_cmd_allocator, nullptr); // 再度コマンドリストをためる準備

		// スワップ
		p_swap_chain->Present(1, 0);
	}

	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}