#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <wrl.h>

// DirectX12
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
// D3D12 extendo library
#include "../../Common/d3dx12.h"

// STL Headers
#include <algorithm>
#include <cassert>
#include <chrono>

#include <iostream>

// Helper functions
#include "./MyHelper.hpp"

// グローバル変数
// swap chain back buffers
const uint8_t g_num_frames = 3;
// use warp adapter
bool g_use_warp = false;

uint32_t g_client_width = 1280;
uint32_t g_client_height = 720;

// DirextX12の初期化判定
bool g_is_initialized = false;

// Windowハンドル
HWND g_hWnd;
// WindowRectangle(フルスクリーン時に使用)
RECT g_window_rect;

// DirectX12
Microsoft::WRL::ComPtr<ID3D12Device2> g_device;
Microsoft::WRL::ComPtr<ID3D12CommandQueue> g_command_queue;
Microsoft::WRL::ComPtr<IDXGISwapChain4> g_swap_chain;
Microsoft::WRL::ComPtr<ID3D12Resource> g_back_buffers[g_num_frames];
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> g_command_list;
Microsoft::WRL::ComPtr<ID3D12CommandAllocator> g_command_allocators[g_num_frames];
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> g_rtv_descriptor_heap;
UINT g_rtv_descriptor_size;
UINT g_current_back_buffer_index;

// GPU同期
Microsoft::WRL::ComPtr<ID3D12Fence> g_fence;
uint64_t g_fence_value = 0;
uint64_t g_frame_fence_values[g_num_frames] = {};
HANDLE g_fence_event;

// swap chain
bool g_vsync = true;
bool g_tearing_supported = false;
bool g_full_screen = false;


// 関数処理
// コールバック関数
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// 引数パース
void ParseCommandLineArguments()
{
	int argc;
	wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

	for (size_t i = 0; i < argc; ++i)
	{
		if (::wcscmp(argv[i], L"-w") == 0 || ::wcscmp(argv[i], L"--width") == 0)  { g_client_width = ::wcstol(argv[++i], nullptr, 10); }
		if (::wcscmp(argv[i], L"-h") == 0 || ::wcscmp(argv[i], L"--height") == 0) { g_client_height = ::wcstol(argv[++i], nullptr, 10); }
		if (::wcscmp(argv[i], L"-warp") == 0 || ::wcscmp(argv[i], L"--warp") == 0) { g_use_warp = true; }
	}

	// メモリ開放
	::LocalFree(argv);
}

// DebugLayer
void EnableDebugLayer()
{
#if defined(__DEBUG)
	Microsoft::WRL::ComPtr<ID3D12Debug> debug_interface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface)));
	debug_interface->EnableDebugLayer();
#endif
}

// Window登録
void RegisterWindowClass(HINSTANCE hInst, const wchar_t* window_class_name)
{
	WNDCLASSEXW window_class = {};

	window_class.cbSize = sizeof(WNDCLASSEXW);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = &WndProc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = hInst;
	window_class.hIcon = ::LoadIcon(hInst, nullptr);
	window_class.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	window_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	window_class.lpszMenuName = nullptr;
	window_class.lpszClassName = window_class_name;
	window_class.hIconSm = ::LoadIcon(hInst, nullptr);

	static ATOM atom = ::RegisterClassExW(&window_class);
	assert(atom > 0);
}

// Window作成
HWND MyCreateWindow(const wchar_t* window_class_name, HINSTANCE hInst, const wchar_t* window_title, uint32_t width, uint32_t height)
{
	// スクリーンサイズ取得
	int screen_width = ::GetSystemMetrics(SM_CXSCREEN);
	int screen_height = ::GetSystemMetrics(SM_CYSCREEN);

	std::cout << screen_width << " " << screen_height << std::endl;

	RECT window_rect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
	::AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);
	
	// window sizeの設定
	int window_width = window_rect.right - window_rect.left;
	int window_height = window_rect.bottom - window_rect.top;

	// windowを画面内に収める
	int window_x = std::max<int>(0, (screen_width - window_width) / 2);
	int window_y = std::max<int>(0, (screen_height - window_height) / 2);

	// window作成
	HWND hWnd = ::CreateWindowExW
	(
		NULL, window_class_name, window_title, WS_OVERLAPPEDWINDOW,
		window_x, window_y, window_width, window_height,
		nullptr, nullptr, hInst, nullptr
	);
	assert(hWnd && "Failed to create window");

	return hWnd;
}

// DirectX12 Adapterの照会
Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(const bool use_warp)
{
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgi_factory;
	UINT create_factory_flags = 0;
#if defined(__DEBUG)
	create_factory_flags = DXGI_CREATE_FACTORY_DEBUG;
#endif
	ThrowIfFailed(CreateDXGIFactory2(create_factory_flags, IID_PPV_ARGS(&dxgi_factory)));

	Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgi_adapter_1;
	Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgi_adapter_4;

	if (use_warp)
	{
		ThrowIfFailed(dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&dxgi_adapter_1)));
		ThrowIfFailed(dxgi_adapter_1.As(&dxgi_adapter_4));
	}
	else
	{
		SIZE_T max_dedicated_video_memory = 0;
		// 利用可能なGPUアダプタの列挙
		for (UINT i = 0; dxgi_factory->EnumAdapters1(i, &dxgi_adapter_1) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 dxgi_adapter_desc1;
			dxgi_adapter_1->GetDesc1(&dxgi_adapter_desc1);

			if ((dxgi_adapter_desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(dxgi_adapter_1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr))&&
				dxgi_adapter_desc1.DedicatedVideoMemory > max_dedicated_video_memory)
			{
				max_dedicated_video_memory = dxgi_adapter_desc1.DedicatedVideoMemory;
				ThrowIfFailed(dxgi_adapter_1.As(&dxgi_adapter_4));
			}
		}
	}
	return dxgi_adapter_4;
}

// DirectX12デバイスの作成
Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter)
{
	Microsoft::WRL::ComPtr<ID3D12Device2> d3d12_device2;
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12_device2)));

#if defined(__DEBUG)
	// デバッグモード時にデバッグメッセージを表示
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> p_info_queue;
	if (SUCCEEDED(d3d12_device2.As(&p_info_queue)))
	{
		p_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		p_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		p_info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// メッセージカテゴリの抑制
		// D3D12_MESSAGE_CATEGORY categories[0] = {};
		// 重要度に応じてメッセージを抑制
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		// 個々のIDでメッセージを抑制
		D3D12_MESSAGE_ID deny_ids[] = 
		{
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // 不明
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                       // グラフィックデバック中にキャプチャフレームをすると発生
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                     // グラフィックデバック中にキャプチャフレームをすると発生
		};

		D3D12_INFO_QUEUE_FILTER new_filter = {};
		// new_filter.DenyList.NumCategories = _countof(categories);
		// new_filter.DenyList.pCategoryList = categories;
		new_filter.DenyList.NumSeverities = _countof(severities);
		new_filter.DenyList.pSeverityList = severities;
		new_filter.DenyList.NumIDs = _countof(deny_ids);
		new_filter.DenyList.pIDList = deny_ids;

		ThrowIfFailed(p_info_queue->PushStorageFilter(&new_filter));
	}
#endif
	return d3d12_device2;
}