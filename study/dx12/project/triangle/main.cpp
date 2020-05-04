#include <Windows.h>

#include <stdexcept>

#include "SimpleTriangle.hpp"

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (msg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int n_cmd_show)
{
	my_lib::SimpleTriangle the_app{};

	// ウィンドウクラスの登録
	WNDCLASSEX window_class{};
	window_class.cbSize = sizeof(window_class);
	window_class.style = CS_HREDRAW | CS_VREDRAW; // (横|縦)ウィンドウ変化時に全体を再描画
	window_class.lpfnWndProc = WndProc;
	window_class.hInstance = hInstance;
	window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	window_class.lpszClassName = L"Hello, DirectX 12";
	RegisterClassEx(&window_class);

	DWORD dw_style = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
	RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rect, dw_style, FALSE);

	HWND hWnd = CreateWindow
	(
		window_class.lpszClassName,
		L"Hello DirectX 12",
		dw_style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		nullptr,
		nullptr,
		hInstance,
		&the_app
	);

	try
	{
		the_app.Initialize(hWnd);

		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&the_app));
		ShowWindow(hWnd, n_cmd_show);

		MSG msg{};
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			the_app.Render();
		}

		the_app.Terminate();
		return static_cast<int>(msg.wParam);
	}
	catch (std::runtime_error e)
	{
		DebugBreak();
		OutputDebugStringA(e.what() + '\n');
	}

	return 0;
}