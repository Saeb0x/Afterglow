#pragma once

#include "Afterglow/Core/Renderer/Shader.h"

namespace Afterglow
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::filesystem::path& shadersFilePath);
		OpenGLShader(const std::string& debugName, const std::string& vertexSource, const std::string& fragmentSource);
		~OpenGLShader() override;

		void Bind() const override;
		void Unbind() const override;
		uint32_t GetRendererID() const override { return m_RendererID; }

		void SetInt(const std::string& uniform, int num) const override;
		void SetFloat(const std::string& uniform, float num) const override;
		void SetFloat2(const std::string& uniform, const glm::vec2& vec) const override;
		void SetFloat3(const std::string& uniform, const glm::vec3& vec) const override;
		void SetFloat4(const std::string& uniform, const glm::vec4& vec) const override;
		void SetMat4(const std::string& uniform, const glm::mat4& mat) const override;

	private:
		void Compile(const std::string& debugName, const std::string& vertexSource, const std::string& fragmentSource);

	private:
		uint32_t m_RendererID;
	};
}