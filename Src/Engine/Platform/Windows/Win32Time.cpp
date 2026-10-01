#include "Engine/Platform/Windows/Win32Time.h"

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct Time
{
    LARGE_INTEGER Frequency;
    LARGE_INTEGER LastCounter;
};
static Time TimeData;

void Win32TimeInit()
{
    QueryPerformanceFrequency(&TimeData.Frequency);
    QueryPerformanceCounter(&TimeData.LastCounter);
}

real64 Win32TimeTick()
{
    LARGE_INTEGER currentCounter;
    QueryPerformanceCounter(&currentCounter);

    real64 deltaSeconds = (real64)(currentCounter.QuadPart - TimeData.LastCounter.QuadPart) / (real64)TimeData.Frequency.QuadPart;
    TimeData.LastCounter = currentCounter;

    return(deltaSeconds);
}

void Win32TimeReset()
{
    QueryPerformanceCounter(&TimeData.LastCounter);
}
