#if !defined(AFTERGLOW_WIN32WINDOW_H)
#define AFTERGLOW_WIN32WINDOW_H

#include <SSTL/Core/Types.h>
#include <SSTL/Core/String.h>

bool Win32WindowCreate(StackAllocator* allocator, StringView8 title, uint32 width, uint32 height);
bool Win32WindowPumpEvents();
void Win32WindowShutdown();

#endif
