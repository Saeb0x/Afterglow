#if !defined(AFTERGLOW_WIN32WINDOW_H)
#define AFTERGLOW_WIN32WINDOW_H

#include "Engine/Types.h"

bool8 Win32WindowCreate(cstring16 title, uint32 width, uint32 height);
bool8 Win32WindowPumpEvents();
void Win32WindowShutdown();

#endif
