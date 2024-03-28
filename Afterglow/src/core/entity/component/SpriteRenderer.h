#pragma once

#include "BaseComponent.h"

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
					SpriteRenderer();
					~SpriteRenderer();

					void Start() override;
					void Update(float deltaTime) override;
				private:
					bool m_FirstTime;
				};
			}
		}
	}
}

