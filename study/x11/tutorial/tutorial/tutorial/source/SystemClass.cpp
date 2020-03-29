#include "SystemClass.hpp"

// public

SystemClass::SystemClass():
	m_application_name(), m_hinstance(), m_hwnd()
{
}

SystemClass::SystemClass(const SystemClass & other):
	m_application_name(), m_hinstance(), m_hwnd()
{
}

bool SystemClass::Initialize()
{
}

void SystemClass::Shutdown()
{
}

void SystemClass::Run()
{
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
}

// private

bool SystemClass::Frame()
{
}

void SystemClass::InitializeWindows(const int& screen_width, const int& screen_height)
{
}

void SystemClass::ShutdownWindows()
{
}
