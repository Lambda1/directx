#ifndef __MY_DX12_HPP__
#define __MY_DX12_HPP__

#include <string>
#include <vector>

#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dx12.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace mla
{
	class mydx12
	{
	public:
		mydx12(const UINT &width, const UINT &height);
		virtual ~mydx12();

		virtual void Init(HWND hwnd);
		void Terminate();

		virtual void Render();

		virtual void Prepare();
		virtual void CleanUp();
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12CommandList>& cmd);

		const UINT m_frame_buffer_count; // バッファリング数

	protected:
		const UINT m_width, m_height;

		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cmd_queue;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swap_chain;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtv_heap;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_render_target;
		
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsv_heap;
		Microsoft::WRL::ComPtr<ID3D12Resource1> m_depth_buffer;

		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_cmd_allocator;

		std::vector<Microsoft::WRL::ComPtr<ID3D12Fence1>> m_fence;

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
		Microsoft::WRL::ComPtr<IDXGISwapChain1> CreateSwapChain(const HWND &hwnd, const Microsoft::WRL::ComPtr<IDXGIFactory3>& factory);
		void CreateRTV(std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> &render_target, const UINT &buffer_count);
		void CreateCommandAllocator(const UINT &size);
		void CreateFences(const UINT& size);

		// デバッグ関係
		void EnableDebug(const bool &enable_gbv);
		void EnableDebugLayer(Microsoft::WRL::ComPtr<ID3D12Debug> &debug);
		void EnableGPUBasedValidation(const Microsoft::WRL::ComPtr<ID3D12Debug> &debug);
	};
}

#endif
