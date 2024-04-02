#pragma once

#include "Texture.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			class Sprite
			{
			public:
				Sprite(const std::shared_ptr<Texture>& texture);
				Sprite(const std::shared_ptr<Texture>& texture, const std::vector<glm::vec2>& textureCoordinatesLayout);

				inline const std::shared_ptr<Texture>& GetTexture() const { return m_Texture; }
				inline const std::vector<glm::vec2>& GetTextureCoordinates() const { return m_TextureCoordinates; }
				
				~Sprite();

			private:
				std::shared_ptr<Texture> m_Texture;
				std::vector<glm::vec2> m_TextureCoordinates;
			};
		}
	}
}

