#if !defined(AG_PLATFORM_H)
#define AG_PLATFORM_H

#include "Core/Types.h"

struct WindowConfig
{
	const char* Title;
	uint32 Width;
	uint32 Height;
};

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

#endif