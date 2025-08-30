#include "agpch.h"
#include "Texture.h"

#include "Afterglow/Core/Renderer/Renderer.h"
#include "Afterglow/Platform/OpenGL/OpenGLTexture.h"

namespace Afterglow
{
    std::shared_ptr<Texture2D> Texture2D::Create(const std::string& path)
    {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLTexture2D>(path);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
    }
}