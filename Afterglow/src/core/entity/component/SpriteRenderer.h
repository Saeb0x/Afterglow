#pragma once

#include "BaseComponent.h"
#include <glm/glm.hpp>

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
					~SpriteRenderer();

					inline const glm::vec4& GetColor() const { return m_Color; }

					void Start() override;
					void Update(float deltaTime) override;
				private:
					glm::vec4 m_Color;
				};
			}
		}
	}
}

