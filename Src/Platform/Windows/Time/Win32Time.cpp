#include "Win32Time.h"

LARGE_INTEGER Win32GetPerformanceCounterFrequency()
{
    LARGE_INTEGER performanceCounterFrequency;
    QueryPerformanceFrequency(&performanceCounterFrequency);

    return(performanceCounterFrequency);
}

LARGE_INTEGER Win32GetPerformanceCounterTicks()
{
    LARGE_INTEGER performanceCounterTicks;
    QueryPerformanceCounter(&performanceCounterTicks);

    return(performanceCounterTicks);
}

real32 Win32GetSecondsElapsed(LARGE_INTEGER frequency, LARGE_INTEGER start, LARGE_INTEGER end)
{
    return((real32)(end.QuadPart - start.QuadPart) / (real32)frequency.QuadPart);
}

real32 Win32GetFramesPerSecond(real32 secondsElapsed)
{
    return(1.0f / secondsElapsed);
}
