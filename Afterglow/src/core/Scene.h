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
			virtual ~Scene() = default;

			virtual void Init() = 0;
			virtual void Update(float deltaTime) = 0;

			void Start();
			void AddGameObjectToScene(std::shared_ptr<Entity::GameObject> gameObject);

			inline OrthographicCamera* GetCamera() const { return m_OrthographicCamera; }
		protected:
			OrthographicCamera* m_OrthographicCamera;
			Graphics::Renderer& m_Renderer = Graphics::Renderer::GetInstance();
		protected:
			bool m_IsRunning;
			std::vector<std::shared_ptr<Entity::GameObject>> m_GameObjects;
		};
	}
}

