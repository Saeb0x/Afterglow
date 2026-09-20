#if !defined(AFTERGLOW_WIN32RENDER_H)
#define AFTERGLOW_WIN32RENDER_H

#include "Engine/Render/Render.h"

#include <SSTL/Core/Types.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

bool Win32RenderInit(HWND windowHandle, uint32 width, uint32 height);
void Win32RenderClear(Color color);
void Win32RenderPresent();
void Win32RenderShutdown();

#endif
