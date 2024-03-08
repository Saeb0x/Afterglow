#pragma once

namespace Afterglow
{
	namespace Core
	{
		class Scene
		{
		public:
			Scene();
			virtual ~Scene();

			virtual void Update(float deltaTime) = 0;
		};
	}
}

