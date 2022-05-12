#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
#include "lib/wimer.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    return libwimer_main(hInstance, hPrevInstance, lpszCmdLine, nCmdShow);
}
