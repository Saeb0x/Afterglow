#if !defined(AG_OPENGLRENDERER_H)
#define AG_OPENGLRENDERER_H

#include "Core/Types.h"

struct OpenGLRendererContextInfo
{
	uint32 MajorVersion;
	uint32 MinorVersion;
	char Profile[32];
	bool ForwardCompatible;
};

OpenGLRendererContextInfo OpenGLRendererGetContextInfo();

void OpenGLRendererClear(real32 r, real32 g, real32 b, real32 a);
void OpenGLRendererSetViewportSize(uint32 width, uint32 height);

#endif