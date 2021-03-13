#include "mydx12.hpp"

namespace mla
{
	mydx12::mydx12() :
		m_frame_buffer_count(2)
	{
		EnableDebug(true);
	}

	mydx12::~mydx12()
	{

	}

	void mydx12::Init(HWND hwnd)
	{
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
