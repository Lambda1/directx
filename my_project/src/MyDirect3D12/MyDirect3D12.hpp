#ifndef __MY_DIRECT3D12_HPP__
#define __MY_DIRECT3D12_HPP__

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace mla
{
	using namespace Microsoft;
	class MyDirect3D12
	{
	private:
		void CheckSuccess(const HRESULT& is_ok, const std::string& err_msg);

		WRL::ComPtr<IDXGIAdapter> GetHardwareAdapter(const std::wstring& adapter_name);
	public:
		MyDirect3D12(const HWND& hwnd, const int& window_width, const int& window_height, const std::wstring& adapter_name = L"Intel");
		~MyDirect3D12();

		void EnableDebugLayer();
		void EnableDebugReportObject();

		void ClearRenderTarget(const FLOAT* col);
		void BeginDraw();
		void EndDraw();

		void ErrorBlob(WRL::ComPtr<ID3DBlob>& err_blob);

		WRL::ComPtr<ID3D12Resource> CreateCommitedResource(const D3D12_HEAP_PROPERTIES& heap_prop, const D3D12_RESOURCE_DESC& desc, const D3D12_RESOURCE_STATES &state);

		// マッピング
		template<class T>
		void Mapping(const T* data, const size_t& data_size, WRL::ComPtr<ID3D12Resource>& buff)
		{
			T* map = nullptr;
			CheckSuccess(buff->Map(0, nullptr, reinterpret_cast<void**>(&map)), "ERROR: Map");
			std::memcpy(map, data, data_size);
			buff->Unmap(0, nullptr);
		}

		void CompileBasicShader(const std::wstring& vs_path, const std::wstring& ps_path, D3D12_GRAPHICS_PIPELINE_STATE_DESC* g_pipeline);

		void SetPipelineState();
		void SetGraphicsRootSignature(const WRL::ComPtr<ID3D12RootSignature>& root_signature);
		void SetViewAndScissor();
		void SetPrimitiveTopology(const D3D12_PRIMITIVE_TOPOLOGY& type);

		WRL::ComPtr<ID3D12Device> GetDevice();
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
		std::vector<WRL::ComPtr<ID3D12Resource>> m_back_buffers;
		WRL::ComPtr<ID3D12Fence> m_fence;
		WRL::ComPtr<ID3D12PipelineState> m_pipeline_state;
		UINT64 m_fence_value;

		D3D12_VIEWPORT m_view_port;
		D3D12_RECT m_scissor_rect;
	};
}

#endif
