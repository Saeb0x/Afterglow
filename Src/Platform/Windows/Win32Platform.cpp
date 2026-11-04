#include "Platform/Platform.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

void* PlatformAllocateMemory(usize size)
{
	// NOTE(saeb): VirtualAlloc returns page-aligned, zero-initialized memory.
	return (VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
}

void PlatformFreeMemory(void* memory)
{
	if (memory)
	{
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}