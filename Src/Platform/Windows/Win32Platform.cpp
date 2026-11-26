#include "Platform/Platform.h"

#include "Core/Log.h"
#include "Core/Assert.h"
#include "Renderer/Renderer.h"
#include "App/App.h"

#include <stdarg.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

static bool Win32Running;

static struct
{
	HANDLE Handle;
} Win32Console;

static struct
{
	HINSTANCE Instance;
	const char* ClassName = "AfterglowWindowClass";
	HWND Handle;
	char Title[64];
	uint16 Width;
	uint16 Height;
} Win32Window;

static void Win32ConsoleLogOutputCallback(LogVerbosity verbosity, const char* text, usize length)
{
	(void)verbosity;

	DWORD written = 0;
	WriteFile(Win32Console.Handle, text, (DWORD)length, &written, nullptr);
}

static void Win32Cleanup()
{
	if(!RendererShutdown() || !DestroyWindow(Win32Window.Handle) ||
	   !UnregisterClass(Win32Window.ClassName, Win32Window.Instance))
	{
		PlatformError("Couldn't cleanup resources, error %lu.", GetLastError());
	}
}

static LRESULT CALLBACK Win32WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
		{
			AG_LOG_DEBUG("Window: Created \"%s\" with a client area of %ux%u.", Win32Window.Title,
						 Win32Window.Width, Win32Window.Height);

			if(!RendererInit((void*)window, Win32Window.Width, Win32Window.Height))
			{
				PlatformError("Couldn't initialize the renderer, error %lu.", GetLastError());
			}

			if(!AppInit())
			{
				PlatformError("Couldn't initialize the app.");
			}

			return (0);
		}

		case WM_CLOSE:
		{
			Win32Cleanup();
			PostQuitMessage(0);

			return (0);
		}

		case WM_SIZE:
		{
			if(Win32Window.Width != LOWORD(lParam) || Win32Window.Height != HIWORD(lParam))
			{
				Win32Window.Width = LOWORD(lParam);
				Win32Window.Height = HIWORD(lParam);
				RendererResize(Win32Window.Width, Win32Window.Height);

				AG_LOG_DEBUG(
					"Window: Client area has been resized (%ux%u) and context viewport updated.",
					Win32Window.Width, Win32Window.Height);
			}

			return (0);
		}

		case WM_KEYDOWN:
		{
			if(wParam == VK_ESCAPE)
			{
				Win32Cleanup();
				PostQuitMessage(0);
			}

			return (0);
		}
	}

	return (DefWindowProcA(window, message, wParam, lParam));
}

void PlatformError(const char* format, ...)
{
	char errorMessage[1024];

	va_list arguments;
	va_start(arguments, format);
	int errorMessageLength = vsnprintf(errorMessage, sizeof(errorMessage), format, arguments);
	va_end(arguments);

	AG_LOG_ERROR("Platform: %s",
				 (errorMessageLength > 0 || errorMessageLength < (int)sizeof(errorMessage))
					 ? errorMessage
					 : format);

	if(IsDebuggerPresent())
	{
		AG_DEBUGBREAK();
	}

	MessageBoxA(nullptr,
				(errorMessageLength > 0 || errorMessageLength < (int)sizeof(errorMessage)
					 ? errorMessage
					 : format),
				"Afterglow", MB_OK | MB_ICONERROR);
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

void PlatformInitConsole()
{
	if(AllocConsole())
	{
		SetConsoleTitleA("Afterglow Console");
		MoveWindow(GetConsoleWindow(), 0, 0, 1200, 400, false);

		HANDLE consoleOutput =
			CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
						OPEN_EXISTING, 0, nullptr);

		if(consoleOutput != INVALID_HANDLE_VALUE)
		{
			Win32Console.Handle = consoleOutput;
			LogSetOutputCallback(Win32ConsoleLogOutputCallback);
			return;
		}
	}

	PlatformError("Couldn't initialize the console, error %lu.", GetLastError());
}

void PlatformCreateWindow(const char* title, uint16 width, uint16 height)
{
	snprintf(Win32Window.Title, sizeof(Win32Window.Title), "%s", title);
	Win32Window.Width = width;
	Win32Window.Height = height;

	Win32Window.Instance = GetModuleHandleA(nullptr);

	WNDCLASSA windowClass = {};
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Win32WindowProc;
	windowClass.hInstance = Win32Window.Instance;
	windowClass.hCursor = LoadCursorA(nullptr, IDC_ARROW);
	windowClass.lpszClassName = Win32Window.ClassName;

	if(!RegisterClassA(&windowClass))
	{
		PlatformError("Couldn't register the window class, error %lu.", GetLastError());
	}

	RECT rect = {};
	rect.right = (LONG)Win32Window.Width;
	rect.bottom = (LONG)Win32Window.Height;
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	AdjustWindowRect(&rect, style, FALSE);

	Win32Window.Handle =
		CreateWindowExA(0, Win32Window.ClassName, Win32Window.Title, style, CW_USEDEFAULT,
						CW_USEDEFAULT, (rect.right - rect.left), (rect.bottom - rect.top), nullptr,
						nullptr, Win32Window.Instance, nullptr);

	if(!Win32Window.Handle)
	{
		PlatformError("Couldn't create the window, error %lu.", GetLastError());
	}
}

void PlatformLoop()
{
	Win32Running = true;
	while(Win32Running)
	{
		MSG message;
		while(PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if(message.message == WM_QUIT)
			{
				Win32Running = false;
			}

			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		if(!Win32Running)
		{
			break;
		}

		AppUpdate();
		AppRender();
	}
}