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