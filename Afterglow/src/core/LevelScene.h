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

			void Update(float deltaTime) override;
		private:
			bool m_SwitchScene;
			float m_SwitchSceneTime;
		};
	}
}

