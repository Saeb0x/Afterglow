#include "SpriteRenderer.h"
#include "../GameObject.h"

#include <iostream>

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			namespace Component
			{
				SpriteRenderer::SpriteRenderer(const glm::vec4& color) : BaseComponent(), m_LastTransform(nullptr), m_IsDirty(false)
				{
					m_Color = color;
					m_Sprite = std::make_shared<Graphics::Sprite>(nullptr);
				}

				SpriteRenderer::SpriteRenderer(const std::shared_ptr<Graphics::Sprite>& sprite) : m_LastTransform(nullptr), m_IsDirty(false)
				{
					m_Sprite = sprite;
					m_Color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
				}

				SpriteRenderer::~SpriteRenderer() {}

				void SpriteRenderer::SetColor(const glm::vec4& color)
				{
					if (m_Color != color)
					{
						m_Color = color;
						m_IsDirty = true;
					}
				}

				void SpriteRenderer::SetSprite(const std::shared_ptr<Graphics::Sprite>& sprite)
				{
					m_Sprite = sprite;
					m_IsDirty = true;
				}

				void SpriteRenderer::Start() 
				{
					m_LastTransform = GetOwner()->GetTransform()->Copy();
				}

				void SpriteRenderer::Update(float deltaTime) 
				{
					if (!(m_LastTransform == GetOwner()->GetTransform()))
					{
						GetOwner()->GetTransform()->Copy(m_LastTransform);
						m_IsDirty = true;
					}
				}
			}
		}
	}
}