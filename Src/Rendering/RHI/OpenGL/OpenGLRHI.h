#if !defined(AG_OPENGLRHI_H)
#define AG_OPENGLRHI_H

#include "Core/Types.h"

struct OpenGLRHIContextInfo
{
	uint8 MajorVersion;
	uint8 MinorVersion;
	char Profile[16];
	bool ForwardCompatible;
};

OpenGLRHIContextInfo OpenGLRHIGetContextInfo();
void OpenGLRHILoadFunctions();

void OpenGLRHIClear(real32 r, real32 g, real32 b, real32 a);
void OpenGLRHISetViewport(uint16 width, uint16 height);

void OpenGLRHISetupQuad();
void OpenGLRHIDrawQuad(bool wireframe);

#endif