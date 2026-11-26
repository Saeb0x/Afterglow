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

struct GLRHIContextInfo
{
	GLint MajorVersion;
	GLint MinorVersion;
	char Profile[16];
	bool IsForwardCompatible;
};

static struct
{
#if AG_PLATFORM_WINDOWS
	HWND WindowHandle;
	HDC WindowDeviceContext;
	HGLRC Context;
#endif
	GLRHIContextInfo ContextInfo;
} GLRHIState;

PFNGLCLEARBUFFERFVPROC glClearBufferfv = nullptr;

PFNGLCREATEVERTEXARRAYSPROC glCreateVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;

PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;

PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLDELETEPROGRAMPROC glDeleteProgram = nullptr;

static void* GLRHIGetOpenGLFunction(const char* name)
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
	(void)name;
	return (nullptr);
#endif
}

static void GLRHILoadFunctions()
{
	glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)GLRHIGetOpenGLFunction("glClearBufferfv");

	glCreateVertexArrays =
		(PFNGLCREATEVERTEXARRAYSPROC)GLRHIGetOpenGLFunction("glCreateVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GLRHIGetOpenGLFunction("glBindVertexArray");
	glDeleteVertexArrays =
		(PFNGLDELETEVERTEXARRAYSPROC)GLRHIGetOpenGLFunction("glDeleteVertexArrays");
	glGenBuffers = (PFNGLGENBUFFERSPROC)GLRHIGetOpenGLFunction("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)GLRHIGetOpenGLFunction("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)GLRHIGetOpenGLFunction("glBufferData");
	glVertexAttribPointer =
		(PFNGLVERTEXATTRIBPOINTERPROC)GLRHIGetOpenGLFunction("glVertexAttribPointer");
	glEnableVertexAttribArray =
		(PFNGLENABLEVERTEXATTRIBARRAYPROC)GLRHIGetOpenGLFunction("glEnableVertexAttribArray");

	glCreateShader = (PFNGLCREATESHADERPROC)GLRHIGetOpenGLFunction("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)GLRHIGetOpenGLFunction("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)GLRHIGetOpenGLFunction("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)GLRHIGetOpenGLFunction("glGetShaderiv");
	glAttachShader = (PFNGLATTACHSHADERPROC)GLRHIGetOpenGLFunction("glAttachShader");
	glDeleteShader = (PFNGLDELETESHADERPROC)GLRHIGetOpenGLFunction("glDeleteShader");

	glCreateProgram = (PFNGLCREATEPROGRAMPROC)GLRHIGetOpenGLFunction("glCreateProgram");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)GLRHIGetOpenGLFunction("glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)GLRHIGetOpenGLFunction("glUseProgram");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)GLRHIGetOpenGLFunction("glDeleteProgram");
}

static void GLRHIGetContextInfo(GLRHIContextInfo* contextInfo)
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
		contextInfo->IsForwardCompatible = true;
	}
}

bool RHIInit(void* windowHandle, uint16 windowWidth, uint16 windowHeight)
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

	GLRHIState.WindowHandle = (HWND)windowHandle;
	GLRHIState.WindowDeviceContext = GetDC(GLRHIState.WindowHandle);

	int supportedPixelFormatIndex =
		ChoosePixelFormat(GLRHIState.WindowDeviceContext, &desiredPixelFormat);
	PIXELFORMATDESCRIPTOR supportedPixelFormat = {};
	DescribePixelFormat(GLRHIState.WindowDeviceContext, supportedPixelFormatIndex,
						sizeof(supportedPixelFormat), &supportedPixelFormat);

	if(!SetPixelFormat(GLRHIState.WindowDeviceContext, supportedPixelFormatIndex,
					   &desiredPixelFormat))
	{
		return (false);
	}

	HGLRC dummyContext = wglCreateContext(GLRHIState.WindowDeviceContext);
	if(!wglMakeCurrent(GLRHIState.WindowDeviceContext, dummyContext))
	{
		return (false);
	}

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
		(PFNWGLCREATECONTEXTATTRIBSARBPROC)GLRHIGetOpenGLFunction("wglCreateContextAttribsARB");

	wglMakeCurrent(GLRHIState.WindowDeviceContext, nullptr);
	wglDeleteContext(dummyContext);

	const int contextAttributes[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,
									 4,
									 WGL_CONTEXT_MINOR_VERSION_ARB,
									 5,
									 WGL_CONTEXT_PROFILE_MASK_ARB,
									 WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
									 WGL_CONTEXT_FLAGS_ARB,
									 WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
									 0};

	GLRHIState.Context =
		wglCreateContextAttribsARB(GLRHIState.WindowDeviceContext, nullptr, contextAttributes);

	if(!wglMakeCurrent(GLRHIState.WindowDeviceContext, GLRHIState.Context))
	{
		return (false);
	}

	GLRHILoadFunctions();
	RHISetViewport(windowWidth, windowHeight);

	GLRHIGetContextInfo(&GLRHIState.ContextInfo);
	AG_LOG_DEBUG("RHI: Created an OpenGL context (%u.%u, %s Profile%s).",
				 GLRHIState.ContextInfo.MajorVersion, GLRHIState.ContextInfo.MinorVersion,
				 GLRHIState.ContextInfo.Profile,
				 (GLRHIState.ContextInfo.IsForwardCompatible) ? ", Forward Compatible" : "");

	return (true);
#else
	(void)windowHandle, (void)windowWidth, (void)windowHeight;
	return (false);
#endif
}

bool RHIShutdown()
{
#if AG_PLATFORM_WINDOWS
	if(!wglMakeCurrent(GLRHIState.WindowDeviceContext, nullptr) ||
	   !wglDeleteContext(GLRHIState.Context))
	{
		return (false);
	}

	GLRHIState.WindowHandle = nullptr;
	GLRHIState.WindowDeviceContext = nullptr;

	return (true);
#else
	return (false);
#endif
}

void RHISetViewport(uint16 windowWidth, uint16 windowHeight)
{
	glViewport(0, 0, windowWidth, windowHeight);
}

void RHISubmit(const RHICommandList* commandList)
{
	for(usize i = 0; i < commandList->Count; i++)
	{
		switch(commandList->Commands[i].Type)
		{
			case RHICOMMANDTYPE_CLEAR:
			{
				const real32 clearColor[4] = {
					commandList->Commands[i].Clear.Red, commandList->Commands[i].Clear.Green,
					commandList->Commands[i].Clear.Blue, commandList->Commands[i].Clear.Alpha};
				glClearBufferfv(GL_COLOR, 0, clearColor);

				break;
			}

			default:
			{
				break;
			}
		}
	}
}

void RHIPresent()
{
#if AG_PLATFORM_WINDOWS
	SwapBuffers(GLRHIState.WindowDeviceContext);
#endif
}