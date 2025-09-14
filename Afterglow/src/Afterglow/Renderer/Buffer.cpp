#include "agpch.h"
#include "Buffer.h"

#include "Afterglow/Core/Assert.h"

#include "Renderer2D.h"
#include "Afterglow/Platform/OpenGL/OpenGLVertexBuffer.h"
#include "Afterglow/Platform/OpenGL/OpenGLIndexBuffer.h"

namespace Afterglow
{
	std::shared_ptr<VertexBuffer> VertexBuffer::Create(uint32_t size, const float* vertices)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");	
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLVertexBuffer>(size, vertices);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}

	std::shared_ptr<IndexBuffer> IndexBuffer::Create(uint32_t count, const uint32_t* indices)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLIndexBuffer>(count, indices);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}
}