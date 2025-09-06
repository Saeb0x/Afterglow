#include "agpch.h"
#include "OpenGLShader.h"
#include "Afterglow/Core/Log.h"
#include "Afterglow/Core/Assert.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

namespace Afterglow
{
	OpenGLShader::OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource)
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

		AG_INFO("GLSL shaders are compiled successfully and program is linked");
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::UniformMat4(const std::string& name, const glm::mat4& mat) const
	{
		int uniformLoc = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(mat));
	}
}