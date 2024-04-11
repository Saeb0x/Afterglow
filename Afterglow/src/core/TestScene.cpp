#include "TestScene.h"

#include "utils/ResourcePool.h"
#include "SerializationManager.h"
#include "entity/component/RigidBody.h"

#include <iostream>
#include <imgui.h>

namespace Afterglow
{
	namespace Core
	{
		using namespace Graphics;

		std::shared_ptr<Entity::Component::SpriteRenderer> spriteRenderer = nullptr;

		TestScene::TestScene() : Scene() {}

		TestScene::~TestScene()
		{
			delete m_OrthographicCamera;
		}

		void TestScene::Init()
		{
			std::cout << "I'm in the testing Scene!" << std::endl;
			
			m_OrthographicCamera = new OrthographicCamera(0.0f, 32.0f * 40.0f, 0.0f, 32.0f * 21.0f);
			LoadResources();

			if (m_LevelLoaded)
			{
				if (m_GameObjects.at(1) != nullptr)
				{
					m_ActiveGameObject = m_GameObjects.at(1);
				}
				return;
			}

			std::shared_ptr<Graphics::TextureAtlas> spritesheet = m_ResourcePool.GetSpriteSheet("res/textures/spritesheet.png");

			m_Character = std::make_shared<Entity::GameObject>("Main Character", std::make_shared<Graphics::Transform>(glm::vec2(100,100), glm::vec2(256, 256)), 1);
			m_Character->AddComponent(std::make_shared<Entity::Component::SpriteRenderer>(spritesheet->GetSprite(0)));

			AddGameObjectToScene(m_Character);

			auto pointerObject = std::make_shared<Entity::GameObject>("Pointer Object", std::make_shared<Graphics::Transform>(glm::vec2(400, 100), glm::vec2(256, 256)), -1);
			
			spriteRenderer = std::make_shared<Entity::Component::SpriteRenderer>(glm::vec4({ 1.0f, 0.0f, 0.0f, 1.0f }));
			pointerObject->AddComponent(spriteRenderer);

			auto rigidBody = std::make_shared<Entity::Component::RigidBody>();
			pointerObject->AddComponent(rigidBody);

			AddGameObjectToScene(pointerObject);

			m_ActiveGameObject = pointerObject;
		}

		void TestScene::LoadResources()
		{
			m_ResourcePool.AddShader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");
			m_ResourcePool.AddTexture("res/textures/pointer.png");
			m_ResourcePool.AddSpriteSheet("res/textures/spritesheet.png", 32, 32, 12, 0);
		}

		void TestScene::Update(float deltaTime)
		{
			m_GameObjects.at(0)->GetTransform()->GetPosition().x += 30 * deltaTime;

			for (const auto& gameObj : m_GameObjects)
			{
				gameObj->Update(deltaTime);
			}

			m_Renderer.Render();
		}

		void TestScene::ImGui()
		{
			ImGui::Begin("Test Window");
			ImGui::Text("Some text");
			ImGui::End();
		}
	}
}