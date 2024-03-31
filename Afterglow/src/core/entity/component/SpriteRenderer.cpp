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