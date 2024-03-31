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
					m_TextureCoordinatesLayout = std::vector<glm::vec2>(
						{
							glm::vec2({1.0f, 1.0f}),
							glm::vec2({1.0f, 0.0f}),
							glm::vec2({0.0f, 0.0f}),
							glm::vec2({0.0f, 1.0f})
						}
					);
					m_TextureCoordinates = glm::vec2{ 0.0f, 0.0f};
					m_Texture = nullptr;
				}

				SpriteRenderer::SpriteRenderer(const std::shared_ptr<Graphics::Texture>& texture)
				{
					m_Color = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
					m_TextureCoordinatesLayout = std::vector<glm::vec2>(
						{
							glm::vec2({1.0f, 1.0f}),
							glm::vec2({1.0f, 0.0f}),
							glm::vec2({0.0f, 0.0f}),
							glm::vec2({0.0f, 1.0f})
						}
					);
					m_TextureCoordinates = glm::vec2{ 0.0f, 0.0f };
					m_Texture = texture;
				}

				SpriteRenderer::~SpriteRenderer() {}

				void SpriteRenderer::Start()
				{
				
				}

				void SpriteRenderer::Update(float deltaTime)
				{
					
				}
			}
		}
	}
}