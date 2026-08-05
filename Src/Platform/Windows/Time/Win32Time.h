#if !defined(AFTERGLOW_WIN32TIME_H)
#define AFTERGLOW_WIN32TIME_H

#include "Engine/Types.h"

#include <windows.h>

LARGE_INTEGER Win32GetPerformanceCounterFrequency();
LARGE_INTEGER Win32GetPerformanceCounterTicks();
real32 Win32GetSecondsElapsed(LARGE_INTEGER frequency, LARGE_INTEGER start, LARGE_INTEGER end);
real32 Win32GetFramesPerSecond(real32 secondsElapsed);

#endif
