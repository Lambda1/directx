#include "MyDirect3D12.hpp"

namespace mla
{
	MyDirect3D12::MyDirect3D12(const std::wstring &adapter_name):
		m_device{nullptr},
		m_dxgi_factory{nullptr}, m_dxgi_swap_chain{nullptr}
	{
		// アダプタの取得
		CheckSuccess(CreateDXGIFactory(IID_PPV_ARGS(&m_dxgi_factory)), "ERROR: CreateDXGIFactory");
		IDXGIAdapter* adapter = GetHardwareAdapter(adapter_name);
		// デバイスオブジェクトの生成
		CheckSuccess(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)), "ERROR: D3D12CreateDevice");
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
	IDXGIAdapter* MyDirect3D12::GetHardwareAdapter(const std::wstring& adapter_name)
	{
		// アダプタ列挙
		std::vector<IDXGIAdapter*> adapters;
		for (int i = 0;; ++i)
		{
			IDXGIAdapter* tmp = nullptr;
			if (m_dxgi_factory->EnumAdapters(i, &tmp) == DXGI_ERROR_NOT_FOUND) { break; }
			adapters.emplace_back(tmp);
		}
		// アダプタ取得
		IDXGIAdapter* adapter = nullptr;
		for (auto adpt : adapters)
		{
			DXGI_ADAPTER_DESC adpt_desc{};
			adpt->GetDesc(&adpt_desc);
			
			std::wstring adpt_str = adpt_desc.Description;
			if (!adapter && adpt_str.find(adapter_name) != std::wstring::npos) { adapter = adpt; }
			else { adpt->Release(); }
		}
		return adapter;
	}
}
