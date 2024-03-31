#pragma once

#include "BaseComponent.h"
#include "../../graphics/Texture.h"

#include <glm/glm.hpp>
#include <memory>

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			namespace Component
			{
				class SpriteRenderer : public BaseComponent
				{
				public:
					SpriteRenderer(const glm::vec4& color);
					SpriteRenderer(const std::shared_ptr<Graphics::Texture>& texture);

					~SpriteRenderer();

					inline const glm::vec4& GetColor() const { return m_Color; }
					inline const std::shared_ptr<Graphics::Texture>& GetTexture() const { return m_Texture; }
					inline const std::vector <glm::vec2>& GetTextureCoordinates() const { return m_TextureCoordinatesLayout; }

					void Start() override;
					void Update(float deltaTime) override;
				private:
					glm::vec4 m_Color;
					std::vector<glm::vec2> m_TextureCoordinatesLayout;
					glm::vec2 m_TextureCoordinates;
					std::shared_ptr<Graphics::Texture> m_Texture;
				};
			}
		}
	}
}

