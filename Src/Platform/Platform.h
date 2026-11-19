#if !defined(AG_PLATFORM_H)
#define AG_PLATFORM_H

#include "Core/Types.h"

typedef void (*PlatformThreadFunc)(void* data);

struct PlatformThread
{
	PlatformThreadFunc Function;
	void* Data;
	void* Handle;
};

void PlatformFatalError(const char* format, ...);

void* PlatformAllocateMemory(usize size);
void PlatformFreeMemory(void* memory);

bool PlatformThreadStart(PlatformThread* thread, PlatformThreadFunc function, void* data,
						 const char* name);
void PlatformThreadWait(PlatformThread* thread);

void PlatformInitConsole();

void PlatformCreateWindow(const char* title, uint32 width, uint32 height);
void PlatformGameLoop();

void* PlatformGetOpenGLFunction(const char* name);

#endif