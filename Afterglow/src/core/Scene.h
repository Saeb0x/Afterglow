#pragma once

#include "OrthographicCamera.h"

namespace Afterglow
{
	namespace Core
	{
		class Scene
		{
		public:
			Scene();
			virtual ~Scene();

			virtual void Init() = 0;
			virtual void Update(float deltaTime) = 0;
		protected:
			OrthographicCamera* orthoCamera;
		};
	}
}

