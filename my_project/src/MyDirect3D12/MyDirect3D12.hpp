#ifndef __MY_DIRECT3D12_HPP__
#define __MY_DIRECT3D12_HPP__

#include <iostream>
#include <string>
#include <vector>

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace mla
{
	using namespace Microsoft;
	class MyDirect3D12
	{
	private:
		void CheckSuccess(const HRESULT &is_ok, const std::string &err_msg);
		
		WRL::ComPtr<IDXGIAdapter> GetHardwareAdapter(const std::wstring &adapter_name);
	public:
		MyDirect3D12(const HWND &hwnd, const int &window_width, const int &window_height, const std::wstring &adapter_name = L"Intel");
		~MyDirect3D12();
		
		void EnableDebugLayer();

		void ClearRenderTarget(const FLOAT *col);
		void BeginDraw();
		void EndDraw();
		
		WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList();
		WRL::ComPtr<IDXGISwapChain4> GetSwapChain();

	private:
		WRL::ComPtr<ID3D12Device> m_device;
		WRL::ComPtr<IDXGIFactory6> m_dxgi_factory;
		WRL::ComPtr<IDXGISwapChain4> m_dxgi_swap_chain;
		WRL::ComPtr<ID3D12CommandAllocator> m_cmd_allocator;
		WRL::ComPtr<ID3D12GraphicsCommandList> m_cmd_list;
		WRL::ComPtr<ID3D12CommandQueue> m_cmd_queue;
		WRL::ComPtr<ID3D12DescriptorHeap> m_rtv_heaps;
		std::vector<ID3D12Resource*> m_back_buffers;
		WRL::ComPtr<ID3D12Fence> m_fence;
		UINT64 m_fence_value;
	};
}

#endif
