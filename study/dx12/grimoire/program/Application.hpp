#ifndef __APPLICATION_HPP__
#define __APPLICATION_HPP__

#include <Windows.h>

#include <string>

#include "Utils.hpp"

// ƒVƒ“ƒOƒ‹ƒgƒ“

class Application
{
public:
	~Application();

	static Application& Instance();

	void Init();
	void Run();
	void Terminate();

private:
	Application();
	Application(const Application&) = delete;
	void operator=(const Application&) = delete;

	void CreateGameWindow();

private:
	std::wstring m_app_class_name;
	int m_window_width, m_window_height;
	std::wstring m_window_title;
	HWND m_hwnd;
	WNDCLASSEX m_wnd_class;
};

#endif
