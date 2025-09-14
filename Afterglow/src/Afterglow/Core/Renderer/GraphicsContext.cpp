#include "agpch.h"
#include "GraphicsContext.h"

#include "Afterglow/Core/Assert.h"
#include "Renderer2D.h"
#include "Afterglow/Platform/OpenGL/OpenGLContext.h"

namespace Afterglow
{
	std::shared_ptr<GraphicsContext> GraphicsContext::Create(void* windowHandle)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLContext>(windowHandle);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}
}