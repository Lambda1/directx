#include <windows.h>

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd)
{
    MessageBox(NULL, "Hello World!", "Just another Hello World program!", MB_ICONEXCLAMATION | MB_OK);

    return 0;
}