#include "agpch.h"
#include "Framebuffer.h"

#include "Renderer2D.h"
#include "Afterglow/Platform/OpenGL/OpenGLFramebuffer.h"

namespace Afterglow
{
    std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
    {
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLFramebuffer>(spec);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
    }
}