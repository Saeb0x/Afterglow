#include "Platform/Platform.h"
#include "Core/Log.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#if AG_DEBUG
static HANDLE Win32ConsoleOutput;

static void Win32LogToConsole(LogVerbosity verbosity, const char* text, usize length)
{
	DWORD written = 0;
	WriteFile(Win32ConsoleOutput, text, (DWORD)length, &written, nullptr);
}

static void Win32ConsoleInit()
{
	if(AllocConsole())
	{
		SetConsoleTitleA("Afterglow Console");

		HANDLE output = CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
									nullptr, OPEN_EXISTING, 0, nullptr);

		if(output != INVALID_HANDLE_VALUE)
		{
			Win32ConsoleOutput = output;
			LogSetCallback(Win32LogToConsole);
		}
	}
}
#endif

void* PlatformAllocateMemory(usize size)
{
	// NOTE(saeb): VirtualAlloc returns page-aligned, zero-initialized memory.
	return (VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
}

void PlatformFreeMemory(void* memory)
{
	if(memory)
	{
		VirtualFree(memory, 0, MEM_RELEASE);
	}
}