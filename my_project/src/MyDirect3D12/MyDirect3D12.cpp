#include "MyDirect3D12.hpp"

namespace mla
{
	MyDirect3D12::MyDirect3D12(const std::wstring &adapter_name)
	{
		// アダプタの取得
		CheckSuccess(CreateDXGIFactory(IID_PPV_ARGS(m_dxgi_factory.ReleaseAndGetAddressOf())), "ERROR: CreateDXGIFactory");
		auto adapter = GetHardwareAdapter(adapter_name);
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
