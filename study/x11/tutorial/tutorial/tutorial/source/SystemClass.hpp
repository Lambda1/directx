#ifndef __SYSTEM_CLASS_HPP__
#define __SYSTEM_CLASS_HPP__

// windows.h���̈ꕔ�w�b�_�����O(�R���p�C��������)
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

class SystemClass
{
	LPCWSTR m_application_name;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

private:
	bool Frame();
	void InitializeWindows(const int &screen_width, const int &screen_height);
	void ShutdownWindows();

public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
};

#endif