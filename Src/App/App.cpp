#include "App.h"

#include "Rendering/RHI/OpenGL/OpenGLRHI.h"

void AppInit()
{
	OpenGLRHISetupQuad();
}

void AppUpdate()
{
}

void AppRender()
{
	OpenGLRHIClear(0.63f, 0.87f, 1.0f, 1.0f);
	OpenGLRHIDrawQuad(false);
}