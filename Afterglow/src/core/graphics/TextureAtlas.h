#pragma once

#include <memory>
#include <vector>

#include "Sprite.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			class TextureAtlas
			{
			public:
				TextureAtlas(std::shared_ptr<Texture> texture, int spriteWidth, int spriteHeight, int spritesCount, int spacing);
				~TextureAtlas();

				inline const std::shared_ptr<Sprite>& GetSprite(int index) const { return m_Sprites[index]; }
			private:
				std::shared_ptr<Texture> m_Texture;
				std::vector<std::shared_ptr<Sprite>> m_Sprites = std::vector<std::shared_ptr<Sprite>>({});
			};
		}
	}
}

