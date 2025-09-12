#include "agpch.h"
#include "OpenGLShader.h"
#include "Afterglow/Core/Log.h"
#include "Afterglow/Core/Assert.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Afterglow
{
	static std::string ReadFile(const std::filesystem::path& shadersFilePath)
	{
		std::ifstream file(shadersFilePath, std::ios::in | std::ios::binary);
		if (!file)
		{
			AG_ERROR("Failed to open shaders file: {0}", shadersFilePath.string());
			return "";
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	OpenGLShader::OpenGLShader(const std::filesystem::path& shadersFilePath)
	{
		std::string shadersFileSource = ReadFile(shadersFilePath);
		if (shadersFileSource.empty())
		{
			AG_ASSERT(false, "Shaders file is empty!");
			return;
		}

		const std::string vertexToken = "#vertex";
		size_t vertexPos = shadersFileSource.find(vertexToken);

		const std::string fragmentToken = "#fragment";
		size_t fragmentPos = shadersFileSource.find(fragmentToken);

		AG_ASSERT(vertexPos != std::string::npos && fragmentPos != std::string::npos, "Shader file must contain both #vertex and #fragment sections!");

		std::string vertexSource = shadersFileSource.substr(vertexPos + vertexToken.length(), fragmentPos - (vertexPos + vertexToken.length()));
		std::string fragmentSource = shadersFileSource.substr(fragmentPos + fragmentToken.length());

		auto trim = [](std::string& s)
			{
				size_t first = s.find_first_not_of(" \n\r\t");
				size_t last = s.find_last_not_of(" \n\r\t");

				if (first == std::string::npos || last == std::string::npos)
					s.clear();
				else
					s = s.substr(first, last - first + 1);
			};
		trim(vertexSource);
		trim(fragmentSource);

		const std::string debugName = shadersFilePath.stem().string();
		Compile(debugName, vertexSource, fragmentSource);
	}

	OpenGLShader::OpenGLShader(const std::string& debugName, const std::string& vertexSource, const std::string& fragmentSource)
	{
		Compile(debugName, vertexSource, fragmentSource);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Compile(const std::string& debugName, const std::string& vertexSource, const std::string& fragmentSource)
	{
		uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);

		// Send the vertex shader source code to GL.
		const char* source = vertexSource.c_str();
		glShaderSource(vertexShader, 1, &source, 0);

		glCompileShader(vertexShader);

		int isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == 0)
		{
			int maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<char> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(vertexShader);

			AG_ERROR("Vertex shader compilation failed: {0}", infoLog.data());
			AG_ASSERT(false, "Vertex shader compilation failed!");

			return;
		}

		uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		// Send the fragment shader source code to GL
		source = fragmentSource.c_str();
		glShaderSource(fragmentShader, 1, &source, 0);

		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == 0)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<char> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			glDeleteShader(fragmentShader);
			glDeleteShader(vertexShader);

			AG_ERROR("Fragment shader compilation failed: {0}", infoLog.data());
			AG_ASSERT(false, "Fragment shader compilation failed!");

			return;
		}

		// Now time to link them together into a program.
		m_RendererID = glCreateProgram();

		glAttachShader(m_RendererID, vertexShader);
		glAttachShader(m_RendererID, fragmentShader);

		glLinkProgram(m_RendererID);

		int isLinked = 0;
		glGetProgramiv(m_RendererID, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == 0)
		{
			int maxLength = 0;
			glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<char> infoLog(maxLength);
			glGetProgramInfoLog(m_RendererID, maxLength, &maxLength, &infoLog[0]);

			glDeleteProgram(m_RendererID);
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			AG_ERROR("Shader program linking failed: {0}", infoLog.data());
			AG_ASSERT(false, "Shader program linking failed!");

			return;
		}

		glDetachShader(m_RendererID, vertexShader);
		glDetachShader(m_RendererID, fragmentShader);

		AG_INFO("Shaders in shader file ({0}.glsl) are compiled successfully and program is linked", debugName);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& uniform, int num) const
	{
		int uniformLocation = glGetUniformLocation(m_RendererID, uniform.c_str());
		glUniform1i(uniformLocation, num);
	}

	void OpenGLShader::SetFloat(const std::string& uniform, float num) const
	{
		int uniformLocation = glGetUniformLocation(m_RendererID, uniform.c_str());
		glUniform1f(uniformLocation, num);
	}

	void OpenGLShader::SetFloat2(const std::string& uniform, const glm::vec2& vec) const
	{
		int uniformLocation = glGetUniformLocation(m_RendererID, uniform.c_str());
		glUniform2f(uniformLocation, vec.x, vec.y);
	}

	void OpenGLShader::SetFloat3(const std::string& uniform, const glm::vec3& vec) const
	{
		int uniformLocation = glGetUniformLocation(m_RendererID, uniform.c_str());
		glUniform3f(uniformLocation, vec.x, vec.y, vec.z);
	}

	void OpenGLShader::SetFloat4(const std::string& uniform, const glm::vec4& vec) const
	{
		int uniformLoc = glGetUniformLocation(m_RendererID, uniform.c_str());
		glUniform4fv(uniformLoc, 1, glm::value_ptr(vec));
	}

	void OpenGLShader::SetMat4(const std::string& uniform, const glm::mat4& mat) const
	{
		int uniformLoc = glGetUniformLocation(m_RendererID, uniform.c_str());
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(mat));
	}
}