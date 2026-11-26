#if !defined(AG_RENDERER_H)
#define AG_RENDERER_H

#include "Core/Types.h"

bool RendererInit(void* windowHandle, uint16 width, uint16 height);
bool RendererShutdown();
void RendererResize(uint16 width, uint16 height);

void RendererBegin();
void RendererEnd();

#endif