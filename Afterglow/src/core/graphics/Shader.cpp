#include "Shader.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			Shader::Shader(const std::string& vFilePath, const std::string& fFilePath)
			{
				std::string vertexSrc = ParseShader(vFilePath);
				std::string fragmentSrc = ParseShader(fFilePath);

				m_RendererID = CreateShaderProgram(vertexSrc, fragmentSrc);
			}

			Shader::~Shader()
			{
				GLCall(glDeleteProgram(m_RendererID));
			}

			void Shader::SetUniform4f(const char* uniform, float f1, float f2, float f3, float f4)
			{
				int location = GetUniformLocation(uniform);
				glUniform4f(location, f1, f2, f3, f4);
			}

			void Shader::SetUniformMatrix4fv(const char* uniform, const glm::mat4& matrix)
			{
				int location = GetUniformLocation(uniform);
				glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
			}

			void Shader::Bind() const
			{
				GLCall(glUseProgram(m_RendererID));
			}

			void Shader::Unbind() const
			{
				GLCall(glUseProgram(0));
			}

			int Shader::GetUniformLocation(const char* uniform)
			{
				if (m_UniformsCache.find(uniform) != m_UniformsCache.end())
					return m_UniformsCache[uniform];

				m_UniformsCache[uniform] = glGetUniformLocation(m_RendererID, uniform);

				return m_UniformsCache[uniform];
			}

			std::string Shader::ParseShader(const std::string& filePath)
			{
				std::ifstream file(filePath);

				if (!file.is_open())
				{
					std::cerr << "Failed to open: " << filePath << std::endl;
					return "";
				}

				std::stringstream shaderCode;

				std::string line;
				while (getline(file, line))
				{
					shaderCode << line << "\n";
				}

				file.close();
				return shaderCode.str();
			}

			int Shader::CreateShader(unsigned int type, const std::string& source)
			{
				GLCall(unsigned int id = glCreateShader(type));
				const char* src = source.c_str();

				GLCall(glShaderSource(id, 1, &src, nullptr));
				GLCall(glCompileShader(id));

				int compResult;
				GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &compResult));

				if (!compResult)
				{
					int logLength;
					GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength));

					char* message = (char*)_malloca(logLength * sizeof(char));
					GLCall(glGetShaderInfoLog(id, logLength, &logLength, message));

					std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
					std::cerr << message << std::endl;

					GLCall(glDeleteShader(id));
					return 0;
				}

				return id;
			}

			int Shader::CreateShaderProgram(const std::string& vertexSource, const std::string& fragmentSource)
			{
				GLCall(unsigned int program = glCreateProgram());
				unsigned int vShader = CreateShader(GL_VERTEX_SHADER, vertexSource);
				unsigned int fShader = CreateShader(GL_FRAGMENT_SHADER, fragmentSource);

				GLCall(glAttachShader(program, vShader));
				GLCall(glAttachShader(program, fShader));

				GLCall(glLinkProgram(program));
				GLCall(glValidateProgram(program));

				GLCall(glDeleteShader(vShader));
				GLCall(glDeleteShader(fShader));

				return program;
			}
		}
	}
}
