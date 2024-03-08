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

			void Update(float deltaTime) override;
		};
	}
}

