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

	private:
		uint32_t m_RendererID;
	};
}
