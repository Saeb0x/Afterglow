#include "agpch.h"
#include "Shader.h"

#include "Afterglow/Core/Assert.h"
#include "Renderer.h"
#include "Afterglow/Platform/OpenGL/OpenGLShader.h"

namespace Afterglow
{
	Shader* Shader::Create(const std::filesystem::path& shadersFilePath)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLShader(shadersFilePath);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}

	Shader* Shader::Create(const std::string& debugName, const std::string& vertexSource, const std::string& fragmentSource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return new OpenGLShader(debugName, vertexSource, fragmentSource);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}
}