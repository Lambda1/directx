#include <Windows.h>

// �O���[�o���ϐ�
HWND g_hWnd = nullptr;

// �E�B���h�E�v���V�[�W���֐�
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0); break;
	default:
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}

// ���C������
void App()
{

}

// �G���g���[�|�C���g
INT WINAPI WinMain(HINSTANCE h_inst, HINSTANCE h_prev_inst, LPSTR sz_str, INT i_cmd_show)
{
	// �E�B���h�E�ݒ�
	LPCWSTR	w_name = L"Window";
	const int w_width = 640, w_height = 480;

	// �E�B���h�E�쐬
	WNDCLASSEX wc = { 0 };

	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = h_inst;
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
	wc.lpszClassName = w_name;

	if (!RegisterClassEx(&wc)) { return 1; }

	g_hWnd = CreateWindow(w_name, w_name, WS_OVERLAPPEDWINDOW, 0, 0, w_width, w_height, 0, 0, h_inst, 0);

	ShowWindow(g_hWnd, SW_SHOW);
	UpdateWindow(g_hWnd);

	// ���b�Z�[�W���[�v
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) { DispatchMessage(&msg); }
		else
		{
			App();
		}
	}

	return 0;
}