#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <iostream>
#include <vector>
#include <string>

// �O���[�o���ϐ�
const int window_width = 800;
const int window_height = 600;
// D3D12
ID3D12Device* p_device = nullptr;
IDXGIFactory6* p_dxgi_factory = nullptr;
IDXGISwapChain4* p_swap_chain = nullptr;

// WinAPI
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E�j������
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// �f�o�b�O�p�֐�
void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	std::cout << format << " " << valist << std::endl;
	va_end(valist);
#endif
}

// DirectX12
void InitDirectX()
{
	// �A�_�v�^��
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&p_dxgi_factory))))
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}
	std::vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* p_tmp_adapter = nullptr;
	for (int i = 0; p_dxgi_factory->EnumAdapters(i, &p_tmp_adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.emplace_back(p_tmp_adapter);
	}
	// �A�_�v�^����
	IDXGIAdapter* p_adapter = nullptr;
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring str_desc = adesc.Description;
		std::wcout << str_desc << std::endl;
		if (str_desc.find(L"Intel") != std::string::npos)
		{
			if (!p_adapter) { p_adapter = adpt; }
		}
	}

	// �f�o�C�X�I�u�W�F�N�g����
	D3D_FEATURE_LEVEL level;
	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_1 };
	for (auto lv : levels)
	{
		if (SUCCEEDED(D3D12CreateDevice(p_adapter, lv, IID_PPV_ARGS(&p_device))))
		{
			level = lv;
			break;
		}
	}
	if (!p_device)
	{
		std::cout << __LINE__ << std::endl;
		std::exit(EXIT_FAILURE);
	}
}

#ifdef _DEBUG
int main()
#else
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
int main()
#endif
{
	// �E�B���h�E�N���X�̐����E�o�^
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;
	w.lpszClassName = TEXT("DX12 Sample");
	w.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&w);

	// �E�B���h�E�T�C�Y�̒���
	RECT wrc = { 0, 0, window_width, window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow
	(
		w.lpszClassName,
		TEXT("DX12 �e�X�g"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr
	);

#ifdef _DEBUG
	DebugOutputFormatString("Show window test.");
#endif

	// D3D12�̏�����
	InitDirectX();

	// �E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);
	
	// ���[�v
	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}