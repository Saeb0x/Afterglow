#if !defined(AG_OPENGLRENDERER_H)
#define AG_OPENGLRENDERER_H

#include "Core/Types.h"

struct OpenGLRendererContextInfo
{
	uint8 MajorVersion;
	uint8 MinorVersion;
	char Profile[32];
	bool ForwardCompatible;
};

void OpenGLRendererLoadFunctions();

OpenGLRendererContextInfo OpenGLRendererGetContextInfo();

void OpenGLRendererClear(real32 r, real32 g, real32 b, real32 a);
void OpenGLRendererSetViewport(uint32 width, uint32 height);

void OpenGLRendererSetupQuad();
void OpenGLRendererDrawQuad(bool wireframe);

#endif