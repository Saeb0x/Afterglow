#include "App.h"

#include "Renderer/OpenGL/OpenGLRenderer.h"

void AppInit()
{
	OpenGLRendererSetupQuad();
}

void AppUpdate()
{
}

void AppRender()
{
	OpenGLRendererClear(0.63f, 0.87f, 1.0f, 1.0f);
	OpenGLRendererDrawQuad(false);
}