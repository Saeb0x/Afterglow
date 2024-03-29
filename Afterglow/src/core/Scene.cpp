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

		void Scene::AddGameObjectToScene(std::shared_ptr<Entity::GameObject> gameObject)
		{
			if (!m_IsRunning)
			{
				m_GameObjects.push_back(gameObject);
			}
			else
			{
				m_GameObjects.push_back(gameObject);
				gameObject->Start();
			}
		}
	}
}