#if !defined(WIN32WINDOW_H)

#include "Core/Types.h"

#include <windows.h>

struct WindowDimensions
{
    int32 Width;
    int32 Height;
};

bool Win32CreateWindow(HINSTANCE instance, const char* title, int32 width, int32 height, HWND* outWindowHandle);

void Win32ProcessPendingMessages();
bool Win32WindowShouldQuit();
bool Win32WindowConsumeResize(WindowDimensions* outDims);
bool Win32WindowIsMinimized();

void Win32ShowWindow(HWND windowHandle);
void Win32GetWindowDimensions(HWND windowHandle, WindowDimensions* outDims);

#define WIN32WINDOW_H
#endif
