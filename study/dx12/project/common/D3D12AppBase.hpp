#ifndef __D3D12_APP_BASE_HPP__
#define __D3D12_APP_BASE_HPP__

#include <Windows.h>
#include <wrl.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#include <stdexcept>
#include <iostream>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#define _DEBUG true
#define _DEBUG_GBV false

namespace my_lib
{
	class D3D12AppBase
	{
	private:
	protected:
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue;
	
	private:
		// DebugLayer有効化
		void DebugMode(UINT *dxgi_flags);
		// ハードウェアアダプタ検索
		Microsoft::WRL::ComPtr<IDXGIAdapter1> SearchHardwareAdapter(const Microsoft::WRL::ComPtr<IDXGIFactory3> &factory);
		// WARPデバイス設定
		void SetWARPDevice(const Microsoft::WRL::ComPtr<IDXGIFactory4> &factory, Microsoft::WRL::ComPtr<IDXGIAdapter>& warp_device);
	public:
		const UINT m_frame_buffer_count = 2;

		D3D12AppBase();
		virtual ~D3D12AppBase(){}

		void Initialize(HWND hWnd);
		void Terminate();

		virtual void Render(){}

		virtual void Prepare(){}
		virtual void CleanUp(){}
		virtual void MakeCommand(Microsoft::WRL::ComPtr<ID3D12CommandList>& command){}
	};
}

#endif
