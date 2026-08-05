#if !defined(AFTERGLOW_WIN32WINDOW_H)
#define AFTERGLOW_WIN32WINDOW_H

#include "Engine/Types.h"

#include <windows.h>

struct GameInput;

struct WindowDimensions
{
    int32 Width;
    int32 Height;
};

bool32 Win32CreateWindow(HINSTANCE instance, const char* title, int32 width, int32 height, HWND* outWindowHandle);

void Win32ProcessPendingMessages(GameInput* input);
bool32 Win32WindowShouldQuit();
bool32 Win32WindowConsumeResize(WindowDimensions* outDims);
bool32 Win32WindowIsMinimized();

void Win32ShowWindow(HWND windowHandle);
void Win32GetWindowDimensions(HWND windowHandle, WindowDimensions* outDims);

#endif
