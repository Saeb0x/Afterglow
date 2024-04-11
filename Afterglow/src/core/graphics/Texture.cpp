#include "Texture.h"
#include "Renderer.h"

#include <stb_image.h>

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			Texture::Texture(const std::string& texPath) : 
				m_TexFilePath(texPath), m_LocalTexBuffer(nullptr), m_Width(0), m_Height(0), m_BBP(0)
			{
				stbi_set_flip_vertically_on_load(1);
				m_LocalTexBuffer = stbi_load(texPath.c_str(), &m_Width, &m_Height, &m_BBP, 4);

				GLCall(glGenTextures(1, &m_RendererID));
				GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));

				// Repeat image in both directions
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

				// When stretching/shrinking an image, pixelate 
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
				GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

				GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalTexBuffer));
				GLCall(glBindTexture(GL_TEXTURE_2D, 0));

				if (m_LocalTexBuffer)
					stbi_image_free(m_LocalTexBuffer);
			}

			Texture::~Texture()
			{
				GLCall(glDeleteTextures(1, &m_RendererID));
			}

			void Texture::Bind(unsigned int slot)
			{
				GLCall(glActiveTexture(GL_TEXTURE0 + slot + 1))
				GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID));
			}

			void Texture::Unbind() const
			{
				GLCall(glBindTexture(GL_TEXTURE_2D, 0));
			}
		}
	}
}