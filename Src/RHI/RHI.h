#if !defined(AG_RHI_H)
#define AG_RHI_H

#include "Core/Types.h"

bool RHIInit(void* windowHandle);
bool RHIShutdown();

void RHISetViewport(uint16 width, uint16 height);
void RHIClear(real32 red, real32 green, real32 blue, real32 alpha);
void RHISwapBuffers();

#endif