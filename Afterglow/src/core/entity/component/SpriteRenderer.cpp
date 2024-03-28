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
				SpriteRenderer::SpriteRenderer() : BaseComponent(), m_FirstTime(false)
				{
				}

				SpriteRenderer::~SpriteRenderer() {}

				void SpriteRenderer::Start()
				{
					std::cout << "SpriteRenderer: Staring!" << std::endl;
				}

				void SpriteRenderer::Update(float deltaTime)
				{
					if (!m_FirstTime)
					{
						std::cout << "SpriteRenderer: Updating!" << std::endl;
						m_FirstTime = true;
					}
				}
			}
		}
	}
}