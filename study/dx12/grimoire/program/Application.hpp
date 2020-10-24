#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <string>
#include <vector>

#include "Utils.hpp"

// �V���O���g��

class Application
{
public:
	~Application();

	static Application& Instance();

	void Init();
	void Run();
	void Terminate();

private:
	Application();
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;

	void CreateMyWindow();
	void InitializeDXGI();

	IDXGIAdapter* SearchAdapter(const std::wstring &adapter_name);

private:
	// �E�B���h�E�֌W
	std::wstring m_app_class_name;
	int m_window_width, m_window_height;
	std::wstring m_window_title;
	HWND m_hwnd;
	WNDCLASSEX m_wnd_class;
	// DirectX�֌W
	ID3D12Device* m_device;
	IDXGIFactory6* m_dxgi_factory;
	IDXGISwapChain4* m_swap_chain;
};

#endif
