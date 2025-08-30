#pragma once

#include "Afterglow/Core/Renderer/Texture.h"

namespace Afterglow
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		~OpenGLTexture2D() override;

		uint32_t GetWidth() const override { return m_Width;}
		uint32_t GetHeight() const override { return m_Height; }
		uint32_t GetChannels() const override { return m_Channels; }
		const std::string& GetPath() const override { return m_Path; }
		uint32_t GetRendererID() const override { return m_RendererID; };

		void Bind(uint32_t slot) const override;

	private:
		uint32_t m_RendererID;
		std::string m_Path;
		uint32_t m_Width, m_Height, m_Channels;
	};
}