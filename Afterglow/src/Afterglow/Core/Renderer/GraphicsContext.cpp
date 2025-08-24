#include "agpch.h"
#include "GraphicsContext.h"

#include "Afterglow/Core/Assert.h"
#include "Renderer.h"
#include "Afterglow/Platform/OpenGL/OpenGLContext.h"

namespace Afterglow
{
	GraphicsContext* GraphicsContext::Create(void* windowHandle)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLContext(windowHandle);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}
}