#ifndef __MY_D3D_HPP__
#define __MY_D3D_HPP__

#include <d3d9.h>

#include <iostream>
#include <string>

class MyD3D9
{
private:
	// Window�֌W
	inline static constexpr int m_init_window_x = 0, m_init_window_y = 0;    // �E�B���h�E�������W
	inline static const std::string m_window_class_name = "Direct3D9 LABEL"; // �E�B���h�E�N���X���x��
	int m_window_width, m_window_height;   // �E�B���h�E�T�C�Y
	const std::string m_window_title_name; // �E�B���h�E�^�C�g��
	HWND m_hwnd;               // �E�B���h�E�n���h��
	WNDCLASSEX m_window_class; // �E�B���h�E�N���X

	// Direct3D9�֌W
	IDirect3D9* m_p_direct3d;              // IDirect3D9: �f�o�C�X�쐬����擾�@�\���
	IDirect3DDevice9* m_p_direct3d_device; // IDirect3DDevice9: �f�o�C�X
	D3DDISPLAYMODE m_direct3d_display_mode;

private:
	// �R�[���o�b�N�֐�
	static LRESULT WINAPI MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void InitDirect3D();
	void CleanUp();
	void Rendering();

	void OutputDirect3dDetail();

public:
	MyD3D9(const std::string &window_title, const int &width, const int &height);
	~MyD3D9();

	void MainLoop();
};

#endif
