#if !defined(AG_RENDERER_H)
#define AG_RENDERER_H

#include "Core/Types.h"

bool RendererInit(void* windowHandle);
bool RendererShutdown();

void RendererSetViewport(uint16 width, uint16 height);
void RendererClear(real32 red, real32 green, real32 blue, real32 alpha);
void RendererSwapBuffers();

#endif