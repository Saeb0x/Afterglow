#include "Platform/Platform.h"
#include "Core/Log.h"
#include "Core/Assert.h"

#include <cstdarg>
#include <cstdio>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

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
		SetConsoleTitleA("Afterglow");

		HANDLE output = CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
									nullptr, OPEN_EXISTING, 0, nullptr);

		if(output != INVALID_HANDLE_VALUE)
		{
			Win32ConsoleOutput = output;
			LogSetCallback(Win32LogToConsole);
		}
	}
}

void PlatformFatalError(const char* format, ...)
{
	char errorMessage[1024];

	va_list arguments;
	va_start(arguments, format);
	int length = vsnprintf(errorMessage, sizeof(errorMessage), format, arguments);
	va_end(arguments);

	const char* text = (length >= 0) ? errorMessage : format;

	AG_LOG_ERROR("%s", text);

	if(IsDebuggerPresent())
	{
		AG_DEBUGBREAK();
	}

	MessageBoxA(nullptr, text, "Afterglow", MB_OK | MB_ICONERROR);
	ExitProcess(1);
}

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