#if !defined(AFTERGLOW_WIN32WINDOW_H)
#define AFTERGLOW_WIN32WINDOW_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

bool Win32WindowCreate(StackAllocator* allocator, StringView8 title, uint32 width, uint32 height);
bool Win32WindowPumpEvents();
void Win32WindowShutdown();
HWND Win32WindowGetHandle();

#endif
