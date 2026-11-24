#include "RHI/RHI.h"

#include "Core/Config.h"
#include "Core/Log.h"

#include <stdio.h>

#if AG_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <gl/GL.h>
#include "GL/wglext.h"
#else
#error "Afterglow: Unsupported platform for this OpenGL RHI."
#endif
#include "GL/glext.h"

struct OpenGLContextInfo
{
	GLint MajorVersion;
	GLint MinorVersion;
	char Profile[16];
	bool ForwardCompatible;
};

static struct
{
#if AG_PLATFORM_WINDOWS
	HWND WindowHandle;
	HDC WindowDeviceContext;
	HGLRC Context;
#endif
	OpenGLContextInfo ContextInfo;
} OpenGLState;

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;

PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;

PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;

static void* OpenGLRHIGetOpenGLFunction(const char* name)
{
#if AG_PLATFORM_WINDOWS
	void* funcAddress = (void*)wglGetProcAddress(name);
	if(funcAddress == 0 || (funcAddress == (void*)0x1) || (funcAddress == (void*)0x2) ||
	   (funcAddress == (void*)0x3) || (funcAddress == (void*)-1))
	{
		HMODULE glModule = LoadLibraryA("Opengl32.dll");
		funcAddress = (void*)GetProcAddress(glModule, name);
	}

	return (funcAddress);
#else
	return (nullptr);
#endif
}

static void OpenGLRHILoadFunctions()
{
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)OpenGLRHIGetOpenGLFunction("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)OpenGLRHIGetOpenGLFunction("glBindVertexArray");
	glGenBuffers = (PFNGLGENBUFFERSPROC)OpenGLRHIGetOpenGLFunction("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)OpenGLRHIGetOpenGLFunction("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)OpenGLRHIGetOpenGLFunction("glBufferData");
	glVertexAttribPointer =
		(PFNGLVERTEXATTRIBPOINTERPROC)OpenGLRHIGetOpenGLFunction("glVertexAttribPointer");
	glEnableVertexAttribArray =
		(PFNGLENABLEVERTEXATTRIBARRAYPROC)OpenGLRHIGetOpenGLFunction("glEnableVertexAttribArray");

	glCreateShader = (PFNGLCREATESHADERPROC)OpenGLRHIGetOpenGLFunction("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)OpenGLRHIGetOpenGLFunction("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)OpenGLRHIGetOpenGLFunction("glCompileShader");
	glAttachShader = (PFNGLATTACHSHADERPROC)OpenGLRHIGetOpenGLFunction("glAttachShader");
	glDeleteShader = (PFNGLDELETESHADERPROC)OpenGLRHIGetOpenGLFunction("glDeleteShader");

	glCreateProgram = (PFNGLCREATEPROGRAMPROC)OpenGLRHIGetOpenGLFunction("glCreateProgram");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)OpenGLRHIGetOpenGLFunction("glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)OpenGLRHIGetOpenGLFunction("glUseProgram");
}

static void OpenGLRHIGetContextInfo(OpenGLContextInfo* contextInfo)
{
	glGetIntegerv(GL_MAJOR_VERSION, &contextInfo->MajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &contextInfo->MinorVersion);

	GLint profileMask;
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
	if(profileMask & GL_CONTEXT_CORE_PROFILE_BIT)
	{
		snprintf(contextInfo->Profile, sizeof(contextInfo->Profile), "Core");
	}
	else if(profileMask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
	{
		snprintf(contextInfo->Profile, sizeof(contextInfo->Profile), "Compatibility");
	}

	GLint flagsMask;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flagsMask);
	if(flagsMask & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
	{
		contextInfo->ForwardCompatible = true;
	}
}

bool RHIInit(void* windowHandle)
{
#if AG_PLATFORM_WINDOWS
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

	OpenGLState.WindowHandle = (HWND)windowHandle;
	OpenGLState.WindowDeviceContext = GetDC(OpenGLState.WindowHandle);

	int supportedPixelFormatIndex =
		ChoosePixelFormat(OpenGLState.WindowDeviceContext, &desiredPixelFormat);
	PIXELFORMATDESCRIPTOR supportedPixelFormat = {};
	DescribePixelFormat(OpenGLState.WindowDeviceContext, supportedPixelFormatIndex,
						sizeof(supportedPixelFormat), &supportedPixelFormat);

	if(!SetPixelFormat(OpenGLState.WindowDeviceContext, supportedPixelFormatIndex,
					   &desiredPixelFormat))
	{
		return (false);
	}

	HGLRC dummyContext = wglCreateContext(OpenGLState.WindowDeviceContext);
	if(!wglMakeCurrent(OpenGLState.WindowDeviceContext, dummyContext))
	{
		return (false);
	}

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
		(PFNWGLCREATECONTEXTATTRIBSARBPROC)OpenGLRHIGetOpenGLFunction("wglCreateContextAttribsARB");

	wglMakeCurrent(OpenGLState.WindowDeviceContext, nullptr);
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

	OpenGLState.Context =
		wglCreateContextAttribsARB(OpenGLState.WindowDeviceContext, nullptr, contextAttributes);

	if(!wglMakeCurrent(OpenGLState.WindowDeviceContext, OpenGLState.Context))
	{
		return (false);
	}

	OpenGLRHILoadFunctions();
	OpenGLRHIGetContextInfo(&OpenGLState.ContextInfo);

	AG_LOG_DEBUG("RHI: Created an OpenGL context (%u.%u, %s Profile%s).",
				 OpenGLState.ContextInfo.MajorVersion, OpenGLState.ContextInfo.MinorVersion,
				 OpenGLState.ContextInfo.Profile,
				 (OpenGLState.ContextInfo.ForwardCompatible) ? ", Forward Compatible" : "");

	return (true);
#else
	(void)windowHandle;
	return (false);
#endif
}

bool RHIShutdown()
{
#if AG_PLATFORM_WINDOWS
	if(!wglMakeCurrent(OpenGLState.WindowDeviceContext, nullptr) ||
	   !wglDeleteContext(OpenGLState.Context))
	{
		return (false);
	}

	return (true);
#else
	return (false);
#endif
}

void RHISetViewport(uint16 width, uint16 height)
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void RHIClear(real32 red, real32 green, real32 blue, real32 alpha)
{
	glClearColor((GLclampf)red, (GLclampf)green, (GLclampf)blue, (GLclampf)alpha);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RHISwapBuffers()
{
#if AG_PLATFORM_WINDOWS
	SwapBuffers(OpenGLState.WindowDeviceContext);
#endif
}