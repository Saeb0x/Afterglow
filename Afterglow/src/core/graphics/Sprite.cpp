#include "Sprite.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			Sprite::Sprite(const std::shared_ptr<Texture>& texture)
			{
				m_Texture = texture;
				m_TextureCoordinates = std::vector<glm::vec2>(
					{
						glm::vec2({1.0f, 1.0f}),
						glm::vec2({1.0f, 0.0f}),
						glm::vec2({0.0f, 0.0f}),
						glm::vec2({0.0f, 1.0f})
					}
				);
			}

			Sprite::Sprite(const std::shared_ptr<Texture>& texture, const std::vector<glm::vec2>& textureCoordinatesLayout)
			{
				m_Texture = texture;
				m_TextureCoordinates = textureCoordinatesLayout;
			}
			Sprite::~Sprite() {}
		}
	}
}

