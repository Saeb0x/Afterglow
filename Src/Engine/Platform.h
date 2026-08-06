#if !defined(AFTERGLOW_PLATFORM_H)
#define AFTERGLOW_PLATFORM_H

#include "Engine/Types.h"

struct GameInput;
struct PlatformSurface;

struct SurfaceDimensions
{
    int32 Width;
    int32 Height;
};

bool32 PlatformCreateSurface(const char* title, int32 width, int32 height, PlatformSurface** outSurface);
void PlatformShowSurface(PlatformSurface* surface);
void PlatformGetSurfaceDimensions(PlatformSurface* surface, SurfaceDimensions* outDims);

void PlatformPumpEvents(GameInput* input);
bool32 PlatformShouldQuit();
bool32 PlatformConsumeResize(SurfaceDimensions* outDims);
bool32 PlatformIsSuspended();

uint64 PlatformGetClockFrequency();
uint64 PlatformGetClockTicks();
real32 PlatformGetSecondsElapsed(uint64 frequency, uint64 start, uint64 end);

#endif
