#pragma once

#include "Scene.h"

namespace Afterglow
{
	namespace Core
	{
		class TestScene : public Scene
		{
		public:
			TestScene();
			~TestScene();

			void Init() override;
			void Update(float deltaTime) override;
		private:
			void LoadResources();
		};
	}
}
