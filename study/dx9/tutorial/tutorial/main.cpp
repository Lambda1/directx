#include "MyD3D9.hpp"

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
{
	MyD3D9 my_direct3d9("TEST", 100, 100);

	my_direct3d9.MainLoop();

	return 0;
}