#include "mydx12.hpp"

namespace mla
{
	mydx12::mydx12(const UINT &width, const UINT &height) :
		m_width(width), m_height(height),
		m_frame_buffer_count(2)
	{
		m_render_target.resize(m_frame_buffer_count);
	}

	mydx12::~mydx12()
	{

	}

	void mydx12::Init(HWND hwnd)
	{
		UINT dxgi_flags = 0;
#if _DEBUG
		// デバッグ初期化
		EnableDebug(false);
		dxgi_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
		// DXGIFactory生成
		Microsoft::WRL::ComPtr<IDXGIFactory3> factory;
		CreateDXGIFactory2(dxgi_flags, IID_PPV_ARGS(factory.ReleaseAndGetAddressOf()));
		auto adapter = SearchHardwareAdapter(L"NVIDIA", factory);

		// D3D12デバイス生成
		HRESULT create_device_result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf()));
		ErrorCheck(create_device_result, m_device.Get(), L"Error: D3D12CreateDevice");

		// CommandQueue生成
		D3D12_COMMAND_QUEUE_DESC cmd_queue_desc{ D3D12_COMMAND_LIST_TYPE_DIRECT, 0, D3D12_COMMAND_QUEUE_FLAG_NONE, 0 };
		m_device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(m_cmd_queue.ReleaseAndGetAddressOf()));

		// SwapChain生成
		auto sc1 = CreateSwapChain(hwnd, factory);
		sc1.As(&m_swap_chain); // SwapChain1 to SwapChain4

		// RTVディスクリプタ生成
		D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, m_frame_buffer_count, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
		m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(m_rtv_heap.ReleaseAndGetAddressOf()));
		ErrorCheck(S_OK, m_rtv_heap.Get(), L"ERROR: CreateDescriptorHEAP");
		// RTV生成
		CreateRTV(m_render_target, m_frame_buffer_count);
		
		// デプスバッファ生成
		auto res_desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
		D3D12_CLEAR_VALUE depth_clear_value{};
		depth_clear_value.Format = res_desc.Format;
		depth_clear_value.DepthStencil.Depth = 1.0f;
		depth_clear_value.DepthStencil.Stencil = 0;
		const auto heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		m_device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE, &res_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depth_clear_value, IID_PPV_ARGS(m_depth_buffer.ReleaseAndGetAddressOf()));

		// DSVディスクリプタ生成
		D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 0 };
		m_device->CreateDescriptorHeap(&dsv_heap_desc, IID_PPV_ARGS(m_dsv_heap.ReleaseAndGetAddressOf()));
		ErrorCheck(S_OK, m_dsv_heap.Get(), L"ERROR: CreateDescriptorHeap");
		// DSV生成
		D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{ DXGI_FORMAT_D32_FLOAT, D3D12_DSV_DIMENSION_TEXTURE2D, D3D12_DSV_FLAG_NONE, {0}};
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsv_handle{ m_dsv_heap->GetCPUDescriptorHandleForHeapStart() };
		m_device->CreateDepthStencilView(m_depth_buffer.Get(), &dsv_desc, dsv_handle);

		// CommandAllocator生成
		CreateCommandAllocator(m_frame_buffer_count);

		// Fence生成
		CreateFences(m_frame_buffer_count);
	}

	void mydx12::Terminate()
	{

	}

	void mydx12::Render()
	{
	}
	
	void mydx12::Prepare()
	{
	}
	
	void mydx12::CleanUp()
	{
	}
	
	void mydx12::MakeCommand(Microsoft::WRL::ComPtr<ID3D12CommandList>& cmd)
	{
	}

	// private
	// DX12対応ハードウェアアダプタ検索
	Microsoft::WRL::ComPtr<IDXGIAdapter1> mydx12::SearchHardwareAdapter(const std::wstring &use_adapter_name, const Microsoft::WRL::ComPtr<IDXGIFactory3>& factory)
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		for (UINT adapter_index = {}; factory->EnumAdapters1(adapter_index, &adapter) != DXGI_ERROR_NOT_FOUND; ++adapter_index)
		{
			DXGI_ADAPTER_DESC1 desc{};
			adapter->GetDesc1(&desc);
			
			// 取得するアダプタの確認
			std::wstring adapter_name = desc.Description;
			if (adapter_name.find(use_adapter_name) == std::wstring::npos) { continue; }
			if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == DXGI_ADAPTER_FLAG_SOFTWARE) { continue; }

			// DX12対応の確認
			Microsoft::WRL::ComPtr<ID3D12Device> device;
			auto hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
			if (SUCCEEDED(hr) && device) { break; }
			adapter = nullptr;
		}
		return adapter;
	}
	// スワップチェイン生成
	Microsoft::WRL::ComPtr<IDXGISwapChain1> mydx12::CreateSwapChain(const HWND &hwnd, const Microsoft::WRL::ComPtr<IDXGIFactory3>& factory)
	{
		DXGI_SWAP_CHAIN_DESC1 swap_chain_desc{};
		swap_chain_desc.BufferCount = m_frame_buffer_count;
		swap_chain_desc.Width = m_width;
		swap_chain_desc.Height = m_height;
		swap_chain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swap_chain_desc.SampleDesc.Count = 1;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swap_chain;
		factory->CreateSwapChainForHwnd(m_cmd_queue.Get(), hwnd, &swap_chain_desc, nullptr, nullptr, &swap_chain);
		ErrorCheck(S_OK, swap_chain.Get(), L"ERROR: CreateSwapChainForHwnd");

		return swap_chain;
	}

	void mydx12::CreateRTV(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>& render_target, const UINT &buffer_count)
	{
		const UINT desc_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle{ m_rtv_heap->GetCPUDescriptorHandleForHeapStart() };

		for (UINT i = 0; i < buffer_count; ++i)
		{
			m_swap_chain->GetBuffer(i, IID_PPV_ARGS(&m_render_target[i]));
			m_device->CreateRenderTargetView(m_render_target[i].Get(), nullptr, rtv_handle);
			rtv_handle.Offset(1, desc_size);
		}
	}
	// コマンドアロケータ生成
	void mydx12::CreateCommandAllocator(const UINT& size)
	{
		m_cmd_allocator.resize(size);
		for (int i = 0; i < size; ++i)
		{
			m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmd_allocator[i].ReleaseAndGetAddressOf()));
			ErrorCheck(S_OK, m_cmd_allocator[i].Get(), L"ERROR: CreateCommandAllocator");
		}
	}
	// フェンス生成
	void mydx12::CreateFences(const UINT& size)
	{
		m_fence.resize(size);
		for (int i = 0; i < size; ++i)
		{
			m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence[i].ReleaseAndGetAddressOf()));
			ErrorCheck(S_OK, m_fence[i].Get(), L"ERROR: CreateFence");
		}
	}

	// デバッグ有効化
	void mydx12::EnableDebug(const bool& enable_gbv)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debug;
		EnableDebugLayer(debug);
		if (enable_gbv) { EnableGPUBasedValidation(debug); }
	}
	// デバッグレイヤ有効化
	void mydx12::EnableDebugLayer(Microsoft::WRL::ComPtr<ID3D12Debug> &debug)
	{
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
		{
			debug->EnableDebugLayer();
		}
		else
		{
			OutputDebugString(L"ERROR: D3D12GetDebugInterface");
		}
	}
	// GBV有効化
	void mydx12::EnableGPUBasedValidation(const Microsoft::WRL::ComPtr<ID3D12Debug> &debug)
	{
		Microsoft::WRL::ComPtr<ID3D12Debug3> debug3;
		debug.As(&debug3);
		debug3->SetEnableGPUBasedValidation(true);
	}
}
