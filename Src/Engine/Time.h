#if !defined(AFTERGLOW_TIME_H)
#define AFTERGLOW_TIME_H

#include "Engine/Types.h"

uint64 GetClockFrequency();
uint64 GetClockTicks();
real32 GetSecondsElapsed(uint64 frequency, uint64 start, uint64 end);

#endif
