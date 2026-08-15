#if !defined(AFTERGLOW_WINDOW_H)
#define AFTERGLOW_WINDOW_H

#include "Engine/Types.h"

bool8 WindowCreate(cstring16 title, uint32 width, uint32 height);
bool8 WindowPumpEvents();
void WindowShutdown();

#endif
