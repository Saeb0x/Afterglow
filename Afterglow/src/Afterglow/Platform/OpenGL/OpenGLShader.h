#pragma once

#include "Afterglow/Core/Renderer/Shader.h"

namespace Afterglow
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& vertexSource, const std::string& fragmentSource);
		~OpenGLShader() override;

		void Bind() const override;
		void Unbind() const override;
		uint32_t GetRendererID() const override { return m_RendererID; }

		void UniformMat4(const std::string& name, const glm::mat4& mat) const override;

	private:
		uint32_t m_RendererID;
	};
}
