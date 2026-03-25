#include "OpenGLRendererAPI.h"
#include <glad/glad.h>

namespace Afterglow
{
	void OpenGLRendererAPI::Init()
	{
		// NOTE(saeb): Load OpenGL extensions
	}

	void OpenGLRendererAPI::SetClearColor(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}