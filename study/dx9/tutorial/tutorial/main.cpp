#include <iostream>
#include <string>

#include <windows.h>
#include <d3d9.h>

// IDirect3D9: �f�o�C�X�쐬����擾�@�\���
IDirect3D9* p_direct3d = nullptr;
// IDirect3DDevice9: �f�o�C�X
IDirect3DDevice9* p_direct3d_device = nullptr;

HRESULT InitD3D(HWND hWnd)
{
	// device������
	p_direct3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (p_direct3d == nullptr) { return E_FAIL; }

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	// �X�N���[�����[�h
	d3dpp.Windowed = TRUE;
	// �_�u���o�b�t�@�����O����swap����
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	// �o�b�N�o�b�t�@�̐F��
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	// CreateDevice
	// �g�p����O���t�B�b�N�A�_�v�^, �n�[�h�E�F�A�`��, �A�v���P�[�V�����n���h��, �f�o�C�X����t���O(�n�[�h, �\�t�g), �f�o�C�X�|�C���^, �������f�o�C�X
	HRESULT result = p_direct3d->CreateDevice
	(
		D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &p_direct3d_device
	);

	if (FAILED(result)) { return E_FAIL; }
	return S_OK;
}

// �I������
void Cleanup()
{
	// Direct3D: �������
	if (p_direct3d_device != nullptr){ p_direct3d_device->Release(); }
	if (p_direct3d != nullptr) { p_direct3d->Release(); }
}

void Render()
{
	if (p_direct3d_device == nullptr)
	{
		return;
	}

	// ��ʂ𔒃N���A
	p_direct3d_device->Clear
	(
		0, nullptr, D3DCLEAR_TARGET,
		D3DCOLOR_XRGB(255, 255, 255), 1.0f, 0
	);

	// Begin the scene
	if (SUCCEEDED(p_direct3d_device->BeginScene()))
	{
		p_direct3d_device->EndScene();
	}

	// Present the backbuffer contents to the display
	p_direct3d_device->Present(NULL, NULL, NULL, NULL);
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// main loop
	switch (msg)
	{
	// WM_DESTROY: �E�B���h�E�j����
	case WM_DESTROY:
		Cleanup();
		PostQuitMessage(0);
		return 0;
	
	// WM_PAINT: �ŏ��ɃE�B���h�E���\�����ꂽ�Ƃ�, �E�B���h�E�𓮂������Ƃ� �ȂǂɃ|�X�g
	case WM_PAINT:
		Render();
		ValidateRect(hWnd, NULL);
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}


int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
{
	// �萔
	const std::string window_title = "Direct3D Tutorial";
	const std::string window_name = "Direct3D Tutorial 01";

	// Window class�̓o�^
	WNDCLASSEX window_class = { sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, window_title.c_str(), nullptr };
	RegisterClassEx(&window_class);

	// window application�̍쐬.
	constexpr int default_x = 100, default_y = 100;
	constexpr int width = 300, height = 300;
	HWND hWnd = CreateWindow(window_title.c_str(), window_name.c_str(), WS_OVERLAPPEDWINDOW, default_x, default_y, width, height, GetDesktopWindow(), nullptr, window_class.hInstance, nullptr);

	// Initialize Direct3D
	HRESULT init_direct3d_results = InitD3D(hWnd);
	if (FAILED(init_direct3d_results)) { return 1; }

	// Show the window
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	// Enter the message loop
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass("D3D Tutorial", window_class.hInstance);

	return 0;
}