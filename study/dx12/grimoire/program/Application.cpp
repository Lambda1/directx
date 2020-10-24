#include "Application.hpp"

// Window Procedure
LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lpawam)
{
	// �E�B���h�E�j��
	if (msg == WM_DESTROY)
	{
		// �v���O�����I���ʒm
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lpawam);
}

Application::Application():
	m_app_class_name(L"DirectX12_GRIMOIRE"),
	m_window_width(960), m_window_height(720), m_window_title(L"DirectX12"),
	m_hwnd{}, m_wnd_class{},
	m_device{nullptr},
	m_dxgi_factory{nullptr}, m_swap_chain{nullptr},
	m_cmd_allocator{nullptr}, m_cmd_list{nullptr}, m_cmd_queue{nullptr}
{
}

Application::~Application()
{

}

// public

// �C���X�^���X����
Application& Application::Instance()
{
	static Application instance;
	return instance;
}

// ������
void Application::Init()
{
	// �E�B���h�E�쐬
	CreateMyWindow();
	ShowWindow(m_hwnd, SW_SHOW);
	
	// Direct3D12������
	InitializeDXGI();
	InitializeCommand();

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
}

// ���C������
void Application::Run()
{

}

// �I������
void Application::Terminate()
{
	// �E�B���h�E�N���X����
	UnregisterClass(m_wnd_class.lpszClassName, m_wnd_class.hInstance);
}

// private

// Window����
void Application::CreateMyWindow()
{
	// �E�B���h�E�N���X�o�^
	m_wnd_class.cbSize = sizeof(WNDCLASSEX);
	m_wnd_class.lpfnWndProc = static_cast<WNDPROC>(WndProc);
	m_wnd_class.lpszClassName = m_app_class_name.c_str();
	m_wnd_class.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&m_wnd_class);

	// �E�B���h�E�T�C�Y�ύX
	RECT wrc = { 0, 0, m_window_width, m_window_height };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	
	m_hwnd = CreateWindow
	(
		m_wnd_class.lpszClassName,
		m_window_title.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		m_wnd_class.hInstance,
		nullptr
	);
}

// DXGI������
void Application::InitializeDXGI()
{
	// DXGIFactory�I�u�W�F�N�g����
	if(FAILED(CreateDXGIFactory(IID_PPV_ARGS(&m_dxgi_factory))))
	{
		ErrorHandling("CreateDXGIFactory is failed. :" + __LINE__);
	}
	// �f�o�C�X�I�u�W�F�N�g����
	D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };
	IDXGIAdapter* adapter = SearchAdapter(L"Intel");
	for (auto feature_level : feature_levels)
	{
		if (SUCCEEDED(D3D12CreateDevice(adapter, feature_level, IID_PPV_ARGS(&m_device)))) { break; }
	}
	// MONZA::Exception
	if (!m_device) { ErrorHandling("D3D12CreateDevice is failed. :" + __LINE__); }
}
// �R�}���h������
void Application::InitializeCommand()
{
	// �R�}���h�A���P�[�^����
	if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmd_allocator))))
	{
		ErrorHandling("CreateCommandAllocator is faild. :" + __LINE__);
	}
	// �R�}���h���X�g����
	if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmd_allocator, nullptr, IID_PPV_ARGS(&m_cmd_list))))
	{
		ErrorHandling("CreateCommandList is failed. :" + __LINE__);
	}
	// �R�}���h�L���[����
	D3D12_COMMAND_QUEUE_DESC cmd_queue_desc = {};
	cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmd_queue_desc.NodeMask = 0;
	cmd_queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (FAILED(m_device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(&m_cmd_queue))))
	{
		ErrorHandling("CreateCommandQueue is failed. :" + __LINE__);
	}
}

// �A�_�v�^����
IDXGIAdapter* Application::SearchAdapter(const std::wstring& adapter_name)
{
	std::vector<IDXGIAdapter*> adapters;
	// �A�_�v�^��
	IDXGIAdapter* tmp_adapter = nullptr;
	for (int i = 0; m_dxgi_factory->EnumAdapters(i, &tmp_adapter) != DXGI_ERROR_NOT_FOUND; ++i) { adapters.emplace_back(tmp_adapter); }
	// �A�_�v�^����
	for (auto adapter : adapters)
	{
		DXGI_ADAPTER_DESC adapter_desc = {};
		adapter->GetDesc(&adapter_desc);

		std::wstring str_desc = adapter_desc.Description;
		if (str_desc.find(adapter_name.c_str()) != std::string::npos)
		{
			OutputLog(L"[Adapter] " + str_desc);
			return adapter;
		}
	}
	return nullptr;
}
