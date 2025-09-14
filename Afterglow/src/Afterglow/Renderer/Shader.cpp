#include "agpch.h"
#include "Shader.h"

#include "Afterglow/Core/Assert.h"
#include "Renderer2D.h"
#include "Afterglow/Platform/OpenGL/OpenGLShader.h"

namespace Afterglow
{
	std::shared_ptr<Shader> Shader::Create(const std::filesystem::path& shadersFilePath)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLShader>(shadersFilePath);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}

	std::shared_ptr<Shader> Shader::Create(const std::string& debugName, const std::string& vertexSource, const std::string& fragmentSource)
	{
		switch (Renderer2D::GetAPI())
		{
		case RendererAPI::API::None:
			AG_ASSERT(false, "RendererAPI::None is not supported!");
			return nullptr;
		case RendererAPI::API::OpenGL:
			return std::make_shared<OpenGLShader>(debugName, vertexSource, fragmentSource);
		}

		AG_ASSERT(false, "Unsupported Renderer API!");
		return nullptr;
	}

	ShaderLibrary& ShaderLibrary::GetInstance()
	{
		static ShaderLibrary instance;
		return instance;
	}

	void ShaderLibrary::Add(const std::string& shaderName, const std::shared_ptr<Shader>& shader)
	{
		AG_ASSERT(!Exists(shaderName), "Shader already exists: " + shaderName);
		m_Shaders[shaderName] = shader;
	}

	std::shared_ptr<Shader> ShaderLibrary::Load(const std::filesystem::path& shadersFilePath, const std::string& shaderName)
	{
		auto shader = std::shared_ptr<Shader>(Shader::Create(shadersFilePath));

		Add(shaderName.empty() ? shadersFilePath.stem().string() : shaderName, shader);

		return shader;
	}

	std::shared_ptr<Shader> ShaderLibrary::Get(const std::string& shaderName) const
	{
		AG_ASSERT(Exists(shaderName), "Shader not found: " + shaderName);
		return m_Shaders.at(shaderName);
	}

	bool ShaderLibrary::Exists(const std::string& shaderName) const
	{
		return m_Shaders.find(shaderName) != m_Shaders.end();
	}
}