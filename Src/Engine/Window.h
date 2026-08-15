#if !defined(AFTERGLOW_WINDOW_H)
#define AFTERGLOW_WINDOW_H

#include "Engine/Types.h"

enum WindowFlags : uint32
{
    WindowFlags_None = 0,
    WindowFlags_Fullscreen = 1 << 0
};

bool8 WindowCreate(cstring16 title, uint32 width, uint32 height, uint32 flags = WindowFlags_None);
bool8 WindowPumpEvents();
void WindowShutdown();

#endif
