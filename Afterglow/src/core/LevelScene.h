#pragma once

#include "Scene.h"

namespace Afterglow
{
	namespace Core
	{
		class LevelScene : public Scene
		{
		public:
			LevelScene();
			~LevelScene();

			void Init() override;
			void Update(float deltaTime) override;
		};
	}
}

