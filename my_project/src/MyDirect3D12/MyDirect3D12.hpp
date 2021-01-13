#ifndef __MY_DIRECT3D12_HPP__
#define __MY_DIRECT3D12_HPP__

#include <iostream>
#include <string>
#include <vector>

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace mla
{
	class MyDirect3D12
	{
	private:
		void CheckSuccess(const HRESULT &is_ok, const std::string &err_msg);
		
		IDXGIAdapter* GetHardwareAdapter(const std::wstring &adapter_name);
	public:
		MyDirect3D12(const std::wstring &adapter_name = L"Intel");
		~MyDirect3D12();

	private:
		ID3D12Device* m_device;
		IDXGIFactory6* m_dxgi_factory;
		IDXGISwapChain4* m_dxgi_swap_chain;
	};
}

#endif
