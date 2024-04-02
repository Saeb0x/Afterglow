#include "TextureAtlas.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			TextureAtlas::TextureAtlas(std::shared_ptr<Texture> texture, int spriteWidth, int spriteHeight, int spritesCount, int spacing)
			{
				m_Texture = texture;
				
				// Top-left of the texture to be the first sprite, Bottom-right to be the last

				int currentX = 0;
				int currentY = texture->GetHeight() - spriteHeight; // The bottom-left corner of the first Top-left sprite 

				/*
					*(leftX, topY)		*(rightX, topY)	
							
					*(leftX, bottomY)	*(rightX, bottomY)
				*/
				for (int i = 0; i < spritesCount; i++)
				{
					float topY = (currentY + spriteHeight) / (float)m_Texture->GetHeight(); // To be normalized 
					float bottomY = currentY / (float)m_Texture->GetHeight();
					float rightX = (currentX + spriteWidth) / (float)m_Texture->GetWidth();
					float leftX = currentX / (float)m_Texture->GetWidth(); 

					std::vector<glm::vec2> texCoords = std::vector<glm::vec2>(
						{
							glm::vec2({rightX, topY}), 
							glm::vec2({rightX, bottomY}),
							glm::vec2({leftX, bottomY}),
							glm::vec2({leftX, topY})
						});

					std::shared_ptr<Sprite> sprite = std::make_shared<Sprite>(m_Texture, texCoords);
					m_Sprites.push_back(sprite);

					currentX += spriteWidth + spacing;
					if (currentX >= m_Texture->GetWidth())
					{
						currentX = 0;
						currentY -= spriteHeight + spacing;
					}
				}
			}

			TextureAtlas::~TextureAtlas() {}
		}
	}
}