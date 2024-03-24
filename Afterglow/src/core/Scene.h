#pragma once

#include "OrthographicCamera.h"
#include "graphics/Renderer.h"

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
			OrthographicCamera* m_OrthographicCamera;
			Graphics::Renderer& m_Renderer = Graphics::Renderer::GetInstance();
		};
	}
}

