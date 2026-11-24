#include "Renderer.h"

#if AG_RHI_OPENGL
#include "RHI/OpenGL/OpenGLRHI.cpp"
#else
#error "Afterglow: An RHI must be selected at compile-time."
#endif

bool RendererInit(void* windowHandle)
{
	if(!RHIInit(windowHandle))
	{
		return (false);
	}

	return (true);
}

bool RendererShutdown()
{
	if(!RHIShutdown())
	{
		return (false);
	}

	return (true);
}

void RendererSetViewport(uint16 width, uint16 height)
{
	RHISetViewport(width, height);
}

void RendererClear(real32 red, real32 green, real32 blue, real32 alpha)
{
	RHIClear(red, green, blue, alpha);
}

void RendererSwapBuffers()
{
	RHISwapBuffers();
}