#include "agpch.h"
#include "RenderCommand.h"

#include "Afterglow/Platform/OpenGL/OpenGLRendererAPI.h"

namespace Afterglow
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}