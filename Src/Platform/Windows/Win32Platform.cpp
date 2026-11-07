#include "Core/Utility.h"
#include "Platform/Platform.h"
#include "Core/Assert.h"
#include "Core/Log.h"

#include <cstdarg>
#include <cstdio>
#include <process.h>

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

static LRESULT CALLBACK Win32WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return (0);
		}
	}

	return (DefWindowProcA(window, message, wParam, lParam));
}

static HWND Win32CreateWindow(HINSTANCE instance, const WindowConfig* config)
{
	const char* windowClassName = "AfterglowWindowClass";

	WNDCLASSA windowClass = {};
	windowClass.lpfnWndProc = Win32WindowProc;
	windowClass.hInstance = instance;
	windowClass.hCursor = LoadCursorA(nullptr, IDC_ARROW);
	windowClass.lpszClassName = windowClassName;

	if(!RegisterClassA(&windowClass))
	{
		PlatformFatalError("Couldn't register the window class, error %lu.", GetLastError());
	}

	DWORD style = WS_OVERLAPPEDWINDOW;

	RECT rect = {};
	rect.right = (LONG)config->Width;
	rect.bottom = (LONG)config->Height;
	AdjustWindowRect(&rect, style, FALSE);

	HWND window = CreateWindowExA(0, windowClassName, config->Title, style, CW_USEDEFAULT,
								  CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
								  nullptr, nullptr, instance, nullptr);
	if(!window)
	{
		PlatformFatalError("Couldn't create the window, error %lu.", GetLastError());
	}

	ShowWindow(window, SW_SHOW);

	AG_LOG_INFO("Window: Created \"%s\" with a client area of %ux%u.", config->Title, config->Width,
				config->Height);
	return (window);
}

static void Win32PumpMessages()
{
	MSG message;
	while(GetMessageA(&message, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);
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

static unsigned __stdcall Win32ThreadEntry(void* parameter)
{
	PlatformThread* thread = (PlatformThread*)parameter;
	thread->Function(thread->Data);

	return (0);
}

bool PlatformThreadStart(PlatformThread* thread, PlatformThreadFunc function, void* data,
						 const char* name)
{
	AG_ASSERT(thread);
	AG_ASSERT(function);
	AG_ASSERT(name);

	thread->Function = function;
	thread->Data = data;

	uintptr_t handle = _beginthreadex(nullptr, 0, Win32ThreadEntry, thread, 0, nullptr);
	if(!handle)
	{
		AG_LOG_ERROR("Thread: Couldn't start \"%s\".", name);
		return (false);
	}

	thread->Handle = (void*)handle;

	wchar_t wideName[64];
	if(MultiByteToWideChar(CP_ACP, 0, name, -1, wideName, AG_ARRAYCOUNT(wideName)))
	{
		SetThreadDescription((HANDLE)handle, wideName);
	}

	return (true);
}

void PlatformThreadWait(PlatformThread* thread)
{
	AG_ASSERT(thread);
	AG_ASSERT(thread->Handle);

	WaitForSingleObject((HANDLE)thread->Handle, INFINITE);
	CloseHandle((HANDLE)thread->Handle);
	thread->Handle = nullptr;
}