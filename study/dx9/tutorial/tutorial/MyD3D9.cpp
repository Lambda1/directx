#include "./MyD3D9.hpp"

MyD3D9::MyD3D9(const std::string& window_title, const int& width, const int& height):
	m_window_width(width), m_window_height(height), m_window_title_name(window_title),
	m_hwnd{}, m_window_class{},
	m_p_direct3d(nullptr), m_p_direct3d_device(nullptr)
{
	m_window_class = { sizeof(WNDCLASSEX), m_window_title_name.c_str() };
}

// コールバック: ウィンドウメッセージ
LRESULT WINAPI MyD3D9::MssageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

}
