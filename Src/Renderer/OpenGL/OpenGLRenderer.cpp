#include "OpenGLRenderer.h"
#include "Core/Config.h"

#include <cstdio>

#if AG_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#error "Afterglow: Unsupported platform for this OpenGL renderer."
#endif
#include <gl/GL.h>
#include "GL/glext.h"

static OpenGLRendererContextInfo ContextInfo;

OpenGLRendererContextInfo OpenGLRendererGetContextInfo()
{
	GLint majorVersion = 0, minorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	ContextInfo.MajorVersion = (uint32)majorVersion;
	ContextInfo.MinorVersion = (uint32)minorVersion;

	GLint profileMask = 0;
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);

	if(profileMask & GL_CONTEXT_CORE_PROFILE_BIT)
	{
		snprintf(ContextInfo.Profile, sizeof(ContextInfo.Profile), "Core");
	}
	else if(profileMask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
	{
		snprintf(ContextInfo.Profile, sizeof(ContextInfo.Profile), "Compatibility");
	}

	GLint flagsMask = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flagsMask);

	if(flagsMask & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
	{
		ContextInfo.ForwardCompatible = true;
	}

	return (ContextInfo);
}

void OpenGLRendererClear(real32 r, real32 g, real32 b, real32 a)
{
	glClearColor((GLfloat)r, (GLfloat)g, (GLfloat)b, (GLfloat)a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLRendererSetViewportSize(uint32 width, uint32 height)
{
	glViewport(0, 0, width, height);
}