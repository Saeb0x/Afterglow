#include "Scene.h"
#include "SerializationManager.h"

#include <imgui.h>
#include <sstream>

namespace Afterglow
{
	namespace Core
	{
		Scene::Scene() : m_OrthographicCamera(nullptr), m_IsRunning(false), m_GameObjects({}), m_ActiveGameObject(nullptr), m_LevelLoaded(false) {}

		void Scene::Start()
		{
			for (const auto& gameObject : m_GameObjects)
			{
				gameObject->Start();
				m_Renderer.Add(gameObject);
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
				m_Renderer.Add(gameObject);
			}
		}

		void Scene::SceneImGui()
		{
			if (m_ActiveGameObject)
			{
				ImGui::Begin("Inspector");
				m_ActiveGameObject->ImGui();
				ImGui::End();
			}

			ImGui();
		}

		void Scene::ImGui() {}

		void Scene::SaveExit()
		{
			SerializationManager::GetInstance().Serialize(m_GameObjects, "TestSceneLevel.json");
		}

		void Scene::Load()
		{
			// Read the JSON file into a string
			std::string jsonString;
			std::ifstream inFile("TestSceneLevel.json");
			if (inFile.is_open()) {
				std::stringstream buffer;
				buffer << inFile.rdbuf();
				jsonString = buffer.str();
				inFile.close();
			}
			else {
				return;
			}

			// Deserialize GameObjects from the JSON array
			std::vector<std::shared_ptr<Entity::GameObject>> gameObjects = SerializationManager::GetInstance().Deserialize(jsonString);

			// Add the GameObjects to the scene
			for (const auto& gameObject : gameObjects) {
				AddGameObjectToScene(gameObject);
			}

			m_LevelLoaded = true;
		}
	}
}