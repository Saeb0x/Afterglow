#include "agpch.h"
#include "VertexArray.h"

#include "Afterglow/Core/Assert.h"
#include "Renderer2D.h"
#include "Afterglow/Platform/OpenGL/OpenGLVertexArray.h"

namespace Afterglow
{
	std::shared_ptr<VertexArray> VertexArray::Create()
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLVertexArray>();
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}
}