#include "Win32Time.h"

#include <windows.h>

uint64 PlatformGetClockFrequency()
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    return((uint64)frequency.QuadPart);
}

uint64 PlatformGetClockTicks()
{
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);

    return((uint64)ticks.QuadPart);
}

real32 PlatformGetSecondsElapsed(uint64 frequency, uint64 start, uint64 end)
{
    return((real32)(end - start) / (real32)frequency);
}
