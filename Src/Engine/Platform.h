#if !defined(AFTERGLOW_PLATFORM_H)
#define AFTERGLOW_PLATFORM_H

#include "Engine/Types.h"

#include "SSTL/Memory.h"

struct GameInput;

struct GameMemory
{
    bool32 Initialized;
    sstl::Arena Engine;
    sstl::Arena Permanent;
    sstl::Arena Transient;
};

bool8 PlatformInit(const char* title, int32 width, int32 height, GameMemory** outMemory);
void PlatformPumpEvents(GameInput* input);
void PlatformShutdown();

uint64 PlatformGetClockFrequency();
uint64 PlatformGetClockTicks();
real32 PlatformGetSecondsElapsed(uint64 frequency, uint64 start, uint64 end);

#endif
