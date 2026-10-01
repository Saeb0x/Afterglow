#if !defined(AFTERGLOW_WIN32TIME_H)
#define AFTERGLOW_WIN32TIME_H

#include <SSTL/Core/Types.h>

void Win32TimeInit();
real64 Win32TimeTick(); // Seconds since the previous tick or reset, unclamped
void Win32TimeReset(); // The next tick measures from now

#endif
