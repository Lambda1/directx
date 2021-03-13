#include "mydx12.hpp"

namespace mla
{
	mydx12::mydx12() :
		m_frame_buffer_count(2)
	{
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
