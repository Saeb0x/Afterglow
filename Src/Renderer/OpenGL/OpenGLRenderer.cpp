#include "OpenGLRenderer.h"

#include "Core/Config.h"
#include "Platform/Platform.h"

#include <stdio.h>

#if AG_PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#error "Afterglow: Unsupported platform for this OpenGL renderer."
#endif
#include <gl/GL.h>
#include "GL/glext.h"

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

static OpenGLRendererContextInfo ContextInfo;

static GLuint QuadShaderProgram;
static GLuint QuadVAO;

OpenGLRendererContextInfo OpenGLRendererGetContextInfo()
{
	GLint majorVersion = 0, minorVersion = 0;
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	ContextInfo.MajorVersion = (uint8)majorVersion;
	ContextInfo.MinorVersion = (uint8)minorVersion;

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

void OpenGLRendererLoadFunctions()
{
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)PlatformGetOpenGLFunction("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)PlatformGetOpenGLFunction("glBindVertexArray");
	glGenBuffers = (PFNGLGENBUFFERSPROC)PlatformGetOpenGLFunction("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)PlatformGetOpenGLFunction("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)PlatformGetOpenGLFunction("glBufferData");
	glVertexAttribPointer =
		(PFNGLVERTEXATTRIBPOINTERPROC)PlatformGetOpenGLFunction("glVertexAttribPointer");
	glEnableVertexAttribArray =
		(PFNGLENABLEVERTEXATTRIBARRAYPROC)PlatformGetOpenGLFunction("glEnableVertexAttribArray");

	glCreateShader = (PFNGLCREATESHADERPROC)PlatformGetOpenGLFunction("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)PlatformGetOpenGLFunction("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)PlatformGetOpenGLFunction("glCompileShader");
	glAttachShader = (PFNGLATTACHSHADERPROC)PlatformGetOpenGLFunction("glAttachShader");
	glDeleteShader = (PFNGLDELETESHADERPROC)PlatformGetOpenGLFunction("glDeleteShader");

	glCreateProgram = (PFNGLCREATEPROGRAMPROC)PlatformGetOpenGLFunction("glCreateProgram");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)PlatformGetOpenGLFunction("glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)PlatformGetOpenGLFunction("glUseProgram");
}

void OpenGLRendererClear(real32 r, real32 g, real32 b, real32 a)
{
	glClearColor((GLfloat)r, (GLfloat)g, (GLfloat)b, (GLfloat)a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLRendererSetViewport(uint32 width, uint32 height)
{
	glViewport(0, 0, width, height);
}

void OpenGLRendererSetupQuad()
{
	glGenVertexArrays(1, &QuadVAO);
	glBindVertexArray(QuadVAO);

	float QuadVertices[12] = {-0.5f, -0.5f, 0.0f, -0.5f, 0.5f,  0.0f,
							  0.5f,  0.5f,  0.0f, 0.5f,  -0.5f, 0.0f};

	GLuint QuadVBO;
	glGenBuffers(1, &QuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int QuadIndices[6] = {3, 0, 1, 3, 1, 2};

	GLuint QuadEBO;
	glGenBuffers(1, &QuadEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, QuadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(QuadIndices), QuadIndices, GL_STATIC_DRAW);

	const char* vertexShaderSource = "#version 330 core\n"
									 "layout (location = 0) in vec3 aPos;\n"
									 "void main()\n"
									 "{\n"
									 "	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
									 "}\0";
	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	const char* fragmentShaderSource = "#version 330 core\n"
									   "out vec4 FragColor;\n"
									   "void main()\n"
									   "{\n"
									   "	FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
									   "}\0";
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	QuadShaderProgram = glCreateProgram();
	glAttachShader(QuadShaderProgram, vertexShader);
	glAttachShader(QuadShaderProgram, fragmentShader);
	glLinkProgram(QuadShaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void OpenGLRendererDrawQuad(bool wireframe)
{
	glUseProgram(QuadShaderProgram);
	glBindVertexArray(QuadVAO);
	glPolygonMode(GL_FRONT_AND_BACK, (wireframe) ? GL_LINE : GL_FILL);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}