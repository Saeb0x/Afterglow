#include "SpriteRenderer.h"

#include <iostream>

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			namespace Component
			{
				SpriteRenderer::SpriteRenderer(const glm::vec4& color) : BaseComponent() 
				{
					m_Color = color;
					m_Sprite = std::make_shared<Graphics::Sprite>(nullptr);
				}

				SpriteRenderer::SpriteRenderer(const std::shared_ptr<Graphics::Sprite>& sprite)
				{
					m_Sprite = sprite;
					m_Color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
				}

				SpriteRenderer::~SpriteRenderer() {}

				void SpriteRenderer::Start() {}

				void SpriteRenderer::Update(float deltaTime) {}
			}
		}
	}
}