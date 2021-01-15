#include "MyDirect3D12.hpp"

namespace mla
{
	MyDirect3D12::MyDirect3D12(const HWND &hwnd, const int &window_width, const int &window_height, const std::wstring &adapter_name)
	{
		// アダプタの取得
		CheckSuccess(CreateDXGIFactory(IID_PPV_ARGS(m_dxgi_factory.ReleaseAndGetAddressOf())), "ERROR: CreateDXGIFactory");
		WRL::ComPtr<IDXGIAdapter> adapter = GetHardwareAdapter(adapter_name);
		// デバイスオブジェクトの作成
		CheckSuccess(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf())), "ERROR: D3D12CreateDevice"); // MONZA
		
		// コマンドアロケータの作成
		CheckSuccess(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmd_allocator.ReleaseAndGetAddressOf())), "ERROR: CreateCommandAllocator");
		// コマンドリストの作成
		CheckSuccess(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmd_allocator.Get(), nullptr, IID_PPV_ARGS(m_cmd_list.ReleaseAndGetAddressOf())), "ERROR: CreateCommandList");
		// コマンドキューの作成
		D3D12_COMMAND_QUEUE_DESC cmd_queue_desc = {};
		cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 全コマンドリストを実行可能
		cmd_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; // デフォルト
		cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // デフォルト
		cmd_queue_desc.NodeMask = 0; // 1つのGPUのみ
		CheckSuccess(m_device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(m_cmd_queue.ReleaseAndGetAddressOf())), "ERROR: CreateCommandQueue");

		// スワップチェーンの作成
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {};
		swap_chain_desc.Width = window_width;
		swap_chain_desc.Height = window_height;
		swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ピクセルフォーマット
		swap_chain_desc.Stereo = false; // 立体視
		swap_chain_desc.SampleDesc.Count = 1; // サンプル回数(AA)
		swap_chain_desc.SampleDesc.Quality = 0; // サンプル品質(AA)
		swap_chain_desc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // 使用法
		swap_chain_desc.BufferCount = 2;
		swap_chain_desc.Scaling = DXGI_SCALING_STRETCH; // 伸縮
		swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // フレームバッファの内容維持
		swap_chain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // アルファ値はGPUに任せる
		swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ウィンドウとフルスクの切り替え可能
		CheckSuccess(m_dxgi_factory->CreateSwapChainForHwnd(m_cmd_queue.Get(), hwnd, &swap_chain_desc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(m_dxgi_swap_chain.ReleaseAndGetAddressOf())), "ERROR: CreateSwapChainForHwnd");

		// ディスクリプタヒープの作成(RTV)
		D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
		rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtv_heap_desc.NodeMask = 0;
		rtv_heap_desc.NumDescriptors = 2;
		rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		CheckSuccess(m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(m_rtv_heaps.ReleaseAndGetAddressOf())), "ERROR: CreateDescriptorHeap");
	}

	MyDirect3D12::~MyDirect3D12()
	{
	}
	
	// private 
	// エラーチェック処理
	void MyDirect3D12::CheckSuccess(const HRESULT& is_ok, const std::string &err_msg)
	{
		if (FAILED(is_ok))
		{
			std::cerr << err_msg << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}
	WRL::ComPtr<IDXGIAdapter> MyDirect3D12::GetHardwareAdapter(const std::wstring& adapter_name)
	{
		// アダプタ列挙
		std::vector<WRL::ComPtr<IDXGIAdapter>> adapters;
		for (int i = 0;; ++i)
		{
			WRL::ComPtr<IDXGIAdapter> tmp = nullptr;
			if (m_dxgi_factory->EnumAdapters(i, tmp.ReleaseAndGetAddressOf()) == DXGI_ERROR_NOT_FOUND) { break; }
			adapters.emplace_back(tmp);
		}
		// アダプタ取得
		WRL::ComPtr<IDXGIAdapter> adapter = nullptr;
		for (auto adpt : adapters)
		{
			DXGI_ADAPTER_DESC adpt_desc{};
			adpt->GetDesc(&adpt_desc);
			
			std::wstring adpt_str = adpt_desc.Description;
			if (!adapter && adpt_str.find(adapter_name) != std::wstring::npos) { adapter = adpt; }
		}
		return adapter;
	}
}
