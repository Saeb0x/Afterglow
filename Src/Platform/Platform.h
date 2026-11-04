#if !defined(AG_PLATFORM_H)
#define AG_PLATFORM_H

#include "Core/Types.h"

void* PlatformAllocateMemory(usize size);
void PlatformFreeMemory(void* memory);

#endif