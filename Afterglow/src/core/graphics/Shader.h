#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			class Shader
			{
			public:
				Shader(const std::string& vFilePath, const std::string& fFilePath);
				~Shader();

				void SetUniform4f(const char* uniform, float f1, float f2, float f3, float f4);
				void SetUniformMatrix4fv(const char* uniform, const glm::mat4& matrix);
				void SetUniform1f(const char* uniform, float f1);
				void SetUniform1i(const char* uniform, int i1);
				// More Uniform Setters ...

				void Bind() const;
				void Unbind() const;
			private:
				unsigned int m_RendererID;
				std::unordered_map<const char*, int> m_UniformsCache;
			private:
				int GetUniformLocation(const char* uniform);
				std::string ParseShader(const std::string& filePath);
				int CreateShader(unsigned int type, const std::string& source);
				int CreateShaderProgram(const std::string& vertexSource, const std::string& fragmentSource);
			};
		}
	}
}

