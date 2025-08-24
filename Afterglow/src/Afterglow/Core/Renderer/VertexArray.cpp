#include "agpch.h"
#include "VertexArray.h"

#include "Afterglow/Core/Assert.h"
#include "Renderer.h"
#include "Afterglow/Platform/OpenGL/OpenGLVertexArray.h"

namespace Afterglow
{
	VertexArray* VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexArray();
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}
}