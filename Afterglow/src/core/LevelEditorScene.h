#pragma once

#include "Scene.h"

namespace Afterglow
{
	namespace Core
	{
		class LevelEditorScene : public Scene
		{
		public:
			LevelEditorScene();
			~LevelEditorScene();

			void Init() override;
			void Update(float deltaTime) override;
		};
	}
}

