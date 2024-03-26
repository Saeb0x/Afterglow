#pragma once

#include <string>

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			class Texture
			{
			public:
				Texture(const std::string& texPath);
				~Texture();

				void Bind(unsigned int slot = 0) const;
				void Unbind() const;

				inline int GetWidth() const { return m_Width; }
				inline int GetHeight() const { return m_Height; }
				inline int GetBBP() const { return m_BBP; }
			private:
				unsigned int m_RendererID;
				std::string m_TexFilePath;
				unsigned char* m_LocalTexBuffer;
				int m_Width, m_Height, m_BBP;
			};
		}
	}
}
