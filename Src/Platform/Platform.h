#if !defined(AG_PLATFORM_H)
#define AG_PLATFORM_H

#include "Core/Types.h"

void PlatformError(const char* format, ...);
void* PlatformAllocateMemory(usize size);
void PlatformFreeMemory(void* memory);

void PlatformInitConsole();

void* PlatformGetOpenGLFunction(const char* name);

void PlatformCreateWindow(const char* title, uint16 width, uint16 height);
void PlatformGameLoop();

#endif