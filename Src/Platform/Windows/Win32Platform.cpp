#include "Platform/Platform.h"

#include "Core/Log.h"
#include "Core/Assert.h"
#include "Rendering/RHI/OpenGL/OpenGLRHI.h"
#include "App/App.h"

#include <stdarg.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <gl/GL.h>
#include "GL/wglext.h"

static bool Win32Running;

struct Win32Console
{
	HANDLE Handle;
};
static Win32Console Console;

struct Win32Window
{
	HINSTANCE Instance;
	const char* ClassName = "AfterglowWindowClass";
	HWND Handle;
	HDC DeviceContext;
	HGLRC OpenGLContext;
	char Title[64];
	uint16 Width;
	uint16 Height;
};
static Win32Window Window;

static void Win32ConsoleLogOutputCallback(LogVerbosity verbosity, const char* text, usize length)
{
	DWORD written = 0;
	WriteFile(Console.Handle, text, (DWORD)length, &written, nullptr);
}

static bool Win32CreateOpenGLContext(HWND windowHandle)
{
	PIXELFORMATDESCRIPTOR desiredPixelFormat = {};
	desiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	desiredPixelFormat.nVersion = 1;
	desiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
	desiredPixelFormat.cColorBits = 24;
	desiredPixelFormat.cAlphaBits = 8;
	desiredPixelFormat.cDepthBits = 24;
	desiredPixelFormat.cStencilBits = 8;
	desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

	Window.DeviceContext = GetDC(windowHandle);

	int supportedPixelFormatIndex = ChoosePixelFormat(Window.DeviceContext, &desiredPixelFormat);
	PIXELFORMATDESCRIPTOR supportedPixelFormat = {};
	DescribePixelFormat(Window.DeviceContext, supportedPixelFormatIndex,
						sizeof(supportedPixelFormat), &supportedPixelFormat);

	if(!SetPixelFormat(Window.DeviceContext, supportedPixelFormatIndex, &desiredPixelFormat))
	{
		return (false);
	}

	HGLRC dummyContext = wglCreateContext(Window.DeviceContext);
	if(!wglMakeCurrent(Window.DeviceContext, dummyContext))
	{
		return (false);
	}

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
		(PFNWGLCREATECONTEXTATTRIBSARBPROC)PlatformGetOpenGLFunction("wglCreateContextAttribsARB");

	wglMakeCurrent(Window.DeviceContext, nullptr);
	wglDeleteContext(dummyContext);

	const int contextAttributes[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,
									 3,
									 WGL_CONTEXT_MINOR_VERSION_ARB,
									 3,
									 WGL_CONTEXT_PROFILE_MASK_ARB,
									 WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
									 WGL_CONTEXT_FLAGS_ARB,
									 WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
									 0};

	Window.OpenGLContext =
		wglCreateContextAttribsARB(Window.DeviceContext, nullptr, contextAttributes);

	if(!wglMakeCurrent(Window.DeviceContext, Window.OpenGLContext))
	{
		return (false);
	}

	return (true);
}

static void Win32Cleanup()
{
	if(!wglMakeCurrent(Window.DeviceContext, nullptr) || !wglDeleteContext(Window.OpenGLContext) ||
	   !DestroyWindow(Window.Handle) || !UnregisterClass(Window.ClassName, Window.Instance))
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
			if(!Win32CreateOpenGLContext(window))
			{
				PlatformError("Couldn't create an OpenGL context, error %lu.", GetLastError());
			}

			OpenGLRHILoadFunctions();
			OpenGLRHISetViewport(Window.Width, Window.Height);

			AppInit();

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
			if(Window.Width != (uint16)LOWORD(lParam) || Window.Height != (uint16)HIWORD(lParam))
			{
				Window.Width = (uint16)LOWORD(lParam);
				Window.Height = (uint16)HIWORD(lParam);
				OpenGLRHISetViewport(Window.Width, Window.Height);

				AG_LOG_DEBUG(
					"Window: Client area has been resized (%ux%u) and context viewport updated.",
					Window.Width, Window.Height);
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
				 (errorMessageLength < 0 || errorMessageLength >= (int)sizeof(errorMessage))
					 ? format
					 : errorMessage);

	if(IsDebuggerPresent())
	{
		AG_DEBUGBREAK();
	}

	MessageBoxA(nullptr,
				((errorMessageLength < 0 || errorMessageLength >= (int)sizeof(errorMessage))
					 ? format
					 : errorMessage),
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
			Console.Handle = consoleOutput;
			LogSetOutputCallback(Win32ConsoleLogOutputCallback);
			return;
		}
	}

	PlatformError("Couldn't initialize the console, error %lu.", GetLastError());
}

void* PlatformGetOpenGLFunction(const char* name)
{
	void* funcAddress = (void*)wglGetProcAddress(name);
	if(funcAddress == 0 || (funcAddress == (void*)0x1) || (funcAddress == (void*)0x2) ||
	   (funcAddress == (void*)0x3) || (funcAddress == (void*)-1))
	{
		HMODULE glModule = LoadLibraryA("Opengl32.dll");
		funcAddress = (void*)GetProcAddress(glModule, name);
	}

	return (funcAddress);
}

void PlatformCreateWindow(const char* title, uint16 width, uint16 height)
{
	snprintf(Window.Title, sizeof(Window.Title), "%s", title);
	Window.Width = width;
	Window.Height = height;

	Window.Instance = GetModuleHandleA(nullptr);

	WNDCLASSA windowClass = {};
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Win32WindowProc;
	windowClass.hInstance = Window.Instance;
	windowClass.hCursor = LoadCursorA(nullptr, IDC_ARROW);
	windowClass.lpszClassName = Window.ClassName;

	if(!RegisterClassA(&windowClass))
	{
		PlatformError("Couldn't register the window class, error %lu.", GetLastError());
	}

	RECT rect = {};
	rect.right = (LONG)Window.Width;
	rect.bottom = (LONG)Window.Height;
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	AdjustWindowRect(&rect, style, FALSE);

	Window.Handle =
		CreateWindowExA(0, Window.ClassName, Window.Title, style, CW_USEDEFAULT, CW_USEDEFAULT,
						(int)(rect.right - rect.left), (int)(rect.bottom - rect.top), nullptr,
						nullptr, Window.Instance, nullptr);

	if(!Window.Handle)
	{
		PlatformError("Couldn't create the window, error %lu.", GetLastError());
	}

	OpenGLRHIContextInfo contextInfo = OpenGLRHIGetContextInfo();

	AG_LOG_INFO("Window: Created \"%s\" with a client area of %ux%u and an OpenGL context (%u.%u, "
				"%s Profile%s).",
				Window.Title, Window.Width, Window.Height, contextInfo.MajorVersion,
				contextInfo.MinorVersion, contextInfo.Profile,
				(contextInfo.ForwardCompatible) ? ", Forward Compatible" : "");
}

void PlatformGameLoop()
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
		SwapBuffers(Window.DeviceContext);
	}
}