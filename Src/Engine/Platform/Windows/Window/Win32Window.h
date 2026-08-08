#if !defined(AFTERGLOW_WIN32WINDOW_H)
#define AFTERGLOW_WIN32WINDOW_H

#include "Engine/Types.h"
#include "Engine/Platform.h"

#include <windows.h>

struct WindowDimensions
{
    int32 Width;
    int32 Height;
};

void Win32ShowWindow();
bool8 Win32WindowShouldQuit();
bool8 Win32WindowConsumeResize(WindowDimensions* outDims);

HWND Win32GetWindowHandle();
void Win32GetWindowDimensions(WindowDimensions* outDims);
bool8 Win32WindowIsMinimized();

#endif
