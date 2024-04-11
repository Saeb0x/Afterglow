#pragma once

#include "OrthographicCamera.h"
#include "graphics/Renderer.h"
#include "utils/ResourcePool.h"

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
			inline Utils::ResourcePool& GetResourcePool() const { return m_ResourcePool; }
			inline bool GetIsLevelLoaded() const { return m_LevelLoaded; }

			inline void SetActiveGameObject(const std::shared_ptr<Entity::GameObject>& gameObject) { m_ActiveGameObject = gameObject; }
			
			void SceneImGui();
			virtual void ImGui();

			void SaveExit();
			void Load();
		protected:
			OrthographicCamera* m_OrthographicCamera;
			Graphics::Renderer& m_Renderer = Graphics::Renderer::GetInstance();
			Utils::ResourcePool& m_ResourcePool = Utils::ResourcePool::GetInstance();
		protected:
			bool m_IsRunning;
			std::vector<std::shared_ptr<Entity::GameObject>> m_GameObjects;
			std::shared_ptr<Entity::GameObject> m_ActiveGameObject;
			bool m_LevelLoaded;
		};
	}
}

