#include "agpch.h"
#include "Shader.h"

#include "Afterglow/Core/Assert.h"
#include "Renderer.h"
#include "Afterglow/Platform/OpenGL/OpenGLShader.h"

namespace Afterglow
{
	Shader* Shader::Create(const std::string& vertexSource, const std::string& fragmentSource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::OpenGL:
			return new OpenGLShader(vertexSource, fragmentSource);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}
}