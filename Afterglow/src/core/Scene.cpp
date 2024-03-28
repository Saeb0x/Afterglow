#include "Scene.h"

namespace Afterglow
{
	namespace Core
	{
		Scene::Scene() : m_OrthographicCamera(nullptr), m_IsRunning(false) {}

		void Scene::Start()
		{
			for (const auto& gameObject : m_GameObjects)
			{
				gameObject->Start();
			}

			m_IsRunning = true;
		}

		void Scene::AddGameObjectToScene(std::unique_ptr<Entity::GameObject> gameObject)
		{
			if (!m_IsRunning)
			{
				m_GameObjects.push_back(std::move(gameObject));
			}
			else
			{
				m_GameObjects.push_back(std::move(gameObject));
				gameObject->Start();
			}
		}
	}
}