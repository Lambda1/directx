#ifndef __D3D12_APP_BASE_HPP__
#define __D3D12_APP_BASE_HPP__

#include <Windows.h>
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include "./d3dx12.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

#include <vector>

#include <filesystem>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

#define _DEBUG_GBV false

namespace my_lib
{
	class D3D12AppBase
	{
	private:
	protected:
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> m_swap_chain;
		
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtv_heap, m_dsv_heap;
		UINT m_rtv_descripter_size;
		
		std::vector<Microsoft::WRL::ComPtr<ID3D12Resource1>> m_render_targets;
		Microsoft::WRL::ComPtr<ID3D12Resource1> m_depth_buffer;
		
		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_command_allocators;
		
		HANDLE m_fence_wait_event;
		std::vector<Microsoft::WRL::ComPtr<ID3D12Fence1>> m_frame_fences;
		std::vector<UINT64> m_frame_fence_values;

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;
		
		UINT m_frame_index = 0;

		virtual void PrepareDescriptorHeaps();
		void PrepareRenderTargetView();

		void CreateDepthBuffer(const int& width, const int& height);
		void CreateCommandAllocators();
		void CreateFrameFences();

		void WaitPreviousFrame();

		// Shader
		HRESULT CompileShaderFromFile(const std::wstring& file_name, const std::wstring& profile, Microsoft::WRL::ComPtr<ID3DBlob>& shader_blob, Microsoft::WRL::ComPtr<ID3DBlob>& error_msg);

	private:
		// DebugLayer有効化
		void DebugMode(UINT *dxgi_flags);
		// ハードウェアアダプタ検索
		Microsoft::WRL::ComPtr<IDXGIAdapter1> SearchHardwareAdapter(const Microsoft::WRL::ComPtr<IDXGIFactory3> &factory);
		// WARPデバイス設定
		void SetWARPDevice(const Microsoft::WRL::ComPtr<IDXGIFactory4> &factory, Microsoft::WRL::ComPtr<IDXGIAdapter>& warp_device);
	public:
		const UINT m_frame_buffer_count = 2;
		const UINT m_gpu_wait_timeout = (10 * 1000);

		D3D12AppBase();
		virtual ~D3D12AppBase(){}

		void Initialize(HWND hWnd);
		void Terminate();

		virtual void Render();

		virtual void Prepare(){}
		virtual void CleanUp(){}
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& command){}
	};
}

#endif
