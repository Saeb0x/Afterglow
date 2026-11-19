#include "Platform/Platform.h"
#include "Core/Utility.h"
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Renderer/OpenGL/OpenGLRenderer.h"

#include <cstdarg>
#include <cstdio>
#include <process.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <gl/GL.h>
#include "GL/wglext.h"

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
	uint32 Width;
	uint32 Height;
};
static Win32Window Window;

static void Win32ConsoleLogOutputCallback(LogVerbosity verbosity, const char* text, usize length)
{
	DWORD written = 0;
	WriteFile(Console.Handle, text, (DWORD)length, &written, nullptr);
}

static void Win32Cleanup()
{
	if(!wglMakeCurrent(Window.DeviceContext, nullptr) || !wglDeleteContext(Window.OpenGLContext) ||
	   !DestroyWindow(Window.Handle) || !UnregisterClass(Window.ClassName, Window.Instance))
	{
		PlatformFatalError("Couldn't cleanup resources, error %lu.", GetLastError());
	}
}

static LRESULT CALLBACK Win32WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
		{
			PIXELFORMATDESCRIPTOR pixelFormat = {};
			pixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pixelFormat.nVersion = 1;
			pixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pixelFormat.iPixelType = PFD_TYPE_RGBA;
			pixelFormat.cColorBits = 32;
			pixelFormat.cDepthBits = 24;
			pixelFormat.cStencilBits = 8;

			Window.DeviceContext = GetDC(window);
			int supportedpixelFormat = ChoosePixelFormat(Window.DeviceContext, &pixelFormat);
			if(!SetPixelFormat(Window.DeviceContext, supportedpixelFormat, &pixelFormat))
			{
				AG_LOG_WARNING("Window: Couldn't set the pixel format for the window client area's "
							   "device context.");
			}

			HGLRC dummyOpenGLContext = wglCreateContext(Window.DeviceContext);
			if(!wglMakeCurrent(Window.DeviceContext, dummyOpenGLContext))
			{
				AG_LOG_WARNING("Window: Couldn't make the dummy OpenGL context current.");
			}

			PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
				(PFNWGLCREATECONTEXTATTRIBSARBPROC)PlatformGetOpenGLFunction(
					"wglCreateContextAttribsARB");

			wglMakeCurrent(Window.DeviceContext, nullptr);
			wglDeleteContext(dummyOpenGLContext);

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
				AG_LOG_WARNING("Window: Couldn't make the main OpenGL context current.");
			}

			OpenGLRendererLoadFunctions();

			OpenGLRendererSetViewport(Window.Width, Window.Height);
			OpenGLRendererPrepare();

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

			if(Window.Width != LOWORD(lParam) || Window.Height != HIWORD(lParam))
			{
				Window.Width = LOWORD(lParam);
				Window.Height = HIWORD(lParam);
				OpenGLRendererSetViewport(Window.Width, Window.Height);

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

void PlatformCreateWindow(const char* title, uint32 width, uint32 height)
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
		PlatformFatalError("Couldn't register the window class, error %lu.", GetLastError());
	}

	RECT rect = {};
	rect.right = (LONG)Window.Width;
	rect.bottom = (LONG)Window.Height;
	DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	AdjustWindowRect(&rect, style, FALSE);

	Window.Handle = CreateWindowExA(0, Window.ClassName, Window.Title, style, CW_USEDEFAULT,
									CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
									nullptr, nullptr, Window.Instance, nullptr);
	if(!Window.Handle)
	{
		PlatformFatalError("Couldn't create the window, error %lu.", GetLastError());
	}

	OpenGLRendererContextInfo contextInfo = OpenGLRendererGetContextInfo();

	AG_LOG_INFO("Window: Created \"%s\" with a client area of %ux%u and an OpenGL context (%u.%u, "
				"%s Profile%s).",
				Window.Title, Window.Width, Window.Height, contextInfo.MajorVersion,
				contextInfo.MinorVersion, contextInfo.Profile,
				(contextInfo.ForwardCompatible) ? ", Forward Compatible" : "");
}

void PlatformFatalError(const char* format, ...)
{
	char errorMessage[1024];

	va_list arguments;
	va_start(arguments, format);
	int errorMessageLength = vsnprintf(errorMessage, sizeof(errorMessage), format, arguments);
	va_end(arguments);

	AG_LOG_ERROR("Platform: %s", (errorMessageLength >= 0) ? errorMessage : format);

	if(IsDebuggerPresent())
	{
		AG_DEBUGBREAK();
	}

	MessageBoxA(nullptr, ((errorMessageLength >= 0) ? errorMessage : format), "Afterglow",
				MB_OK | MB_ICONERROR);
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

void PlatformInitConsole()
{
	if(AllocConsole())
	{
		SetConsoleTitleA("Afterglow");
		MoveWindow(GetConsoleWindow(), 0, 0, 1200, 400, false);

		HANDLE output = CreateFileA("CONOUT$", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
									nullptr, OPEN_EXISTING, 0, nullptr);

		if(output != INVALID_HANDLE_VALUE)
		{
			Console.Handle = output;
			LogSetOutputCallback(Win32ConsoleLogOutputCallback);
		}
	}
}

void PlatformGameLoop()
{
	MSG message;
	while(GetMessageA(&message, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&message);
		DispatchMessageA(&message);

		OpenGLRendererClear(0.63f, 0.87f, 1.0f, 1.0f);
		OpenGLRendererRender();

		SwapBuffers(Window.DeviceContext);
	}
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