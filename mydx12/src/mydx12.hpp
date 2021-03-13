#ifndef __MY_DX12_HPP__
#define __MY_DX12_HPP__

#include <string>

#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace mla
{
	class mydx12
	{
	public:
		mydx12();
		virtual ~mydx12();

		virtual void Init(HWND hwnd);
		void Terminate();

		virtual void Render();

		virtual void Prepare();
		virtual void CleanUp();
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12CommandList>& cmd);

		const UINT m_frame_buffer_count; // バッファリング数

	protected:
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;

		template<class T>
		void ErrorCheck(const HRESULT& result, T* ptr, const std::wstring &msg)
		{
			if (FAILED(result) || !ptr)
			{
#if _DEBUG
				OutputDebugStringW(msg.c_str());
#endif
				std::exit(EXIT_FAILURE);
			}
		}
	private:
		// 初期化関係
		Microsoft::WRL::ComPtr<IDXGIAdapter1> SearchHardwareAdapter(const std::wstring &use_adapter_name, const Microsoft::WRL::ComPtr<IDXGIFactory3>& factory);

		// デバッグ関係
		void EnableDebug(const bool &enable_gbv);
		void EnableDebugLayer(Microsoft::WRL::ComPtr<ID3D12Debug> &debug);
		void EnableGPUBasedValidation(const Microsoft::WRL::ComPtr<ID3D12Debug> &debug);
	};
}

#endif
