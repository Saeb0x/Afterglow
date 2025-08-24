#include "agpch.h"
#include "Buffer.h"

#include "Afterglow/Core/Assert.h"

#include "Renderer.h"
#include "Afterglow/Platform/OpenGL/OpenGLVertexBuffer.h"
#include "Afterglow/Platform/OpenGL/OpenGLIndexBuffer.h"

namespace Afterglow
{
	VertexBuffer* VertexBuffer::Create(uint32_t size, const float* vertices)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");	
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLVertexBuffer(size, vertices);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t count, const uint32_t* indices)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLIndexBuffer(count, indices);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}
}