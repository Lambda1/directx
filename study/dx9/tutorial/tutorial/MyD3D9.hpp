#ifndef __MY_D3D_HPP__
#define __MY_D3D_HPP__

#include <d3d9.h>

#include <iostream>
#include <string>

class MyD3D9
{
private:
	// Window関係
	inline static constexpr int m_init_window_x = 0, m_init_window_y = 0;    // ウィンドウ初期座標
	inline static const std::string m_window_class_name = "Direct3D9 LABEL"; // ウィンドウクラスラベル
	int m_window_width, m_window_height;   // ウィンドウサイズ
	const std::string m_window_title_name; // ウィンドウタイトル
	HWND m_hwnd;               // ウィンドウハンドル
	WNDCLASSEX m_window_class; // ウィンドウクラス

	// Direct3D9関係
	IDirect3D9* m_p_direct3d;              // IDirect3D9: デバイス作成や情報取得機能を提供
	IDirect3DDevice9* m_p_direct3d_device; // IDirect3DDevice9: デバイス
	D3DDISPLAYMODE m_direct3d_display_mode;

private:
	// コールバック関数
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
