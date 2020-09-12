#include <Windows.h>

#ifdef _DEBUG
#include <iostream>
#endif

void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	std::cout << format << " " << valist << std::endl;
	va_end(valist);
#endif
}

#ifdef _DEBUG
int main()
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{

#ifdef _DEBUG
	DebugOutputFormatString("Show window test.");
	std::getchar();
#endif

	return 0;
}