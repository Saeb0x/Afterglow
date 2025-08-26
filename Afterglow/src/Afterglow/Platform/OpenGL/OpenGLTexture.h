#pragma once

#include "Afterglow/Core/Renderer/Texture.h"

namespace Afterglow
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		~OpenGLTexture2D() override;

		int32_t GetWidth() const override { return m_Width;}
		int32_t GetHeight() const override { return m_Height; }
		const std::string& GetPath() const override { return m_Path; }
		uint32_t GetRendererID() const override { return m_RendererID; };

		void Bind(uint32_t slot) const override;

	private:
		uint32_t m_RendererID;
		std::string m_Path;
		uint32_t m_Width, m_Height;
	};
}