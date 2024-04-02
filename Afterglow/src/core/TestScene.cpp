#include "TestScene.h"

#include "utils/AssetPool.h"
#include <iostream>

namespace Afterglow
{
	namespace Core
	{
		using namespace Graphics;

		TestScene::TestScene() : Scene() {}

		TestScene::~TestScene()
		{
			delete m_OrthographicCamera;
		}

		void TestScene::Init()
		{
			std::cout << "I'm in the testing Scene!" << std::endl;
			LoadResources();

			m_OrthographicCamera = new OrthographicCamera(0.0f, 32.0f * 40.0f, 0.0f, 32.0f * 21.0f);

			std::shared_ptr<Graphics::TextureAtlas> spritesheet = m_AssetPool.GetSpriteSheet("res/textures/spritesheet.png");

			m_Character = std::make_shared<Entity::GameObject>("Main Character", std::make_shared<Graphics::Transform>(glm::vec2(100,100), glm::vec2(256, 256)));
			m_Character->AddComponent(std::make_shared<Entity::Component::SpriteRenderer>(spritesheet->GetSprite(0)));
			AddGameObjectToScene(m_Character);

			auto pointerObject = std::make_shared<Entity::GameObject>("Pointer Object", std::make_shared<Graphics::Transform>(glm::vec2(400, 100), glm::vec2(256, 256)));
			pointerObject->AddComponent(std::make_shared<Entity::Component::SpriteRenderer>(std::make_shared<Graphics::Sprite>(m_AssetPool.GetTexture("res/textures/pointer.png"))));
			AddGameObjectToScene(pointerObject);
		}

		void TestScene::LoadResources()
		{
			m_AssetPool.AddShader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");
			m_AssetPool.AddTexture("res/textures/pointer.png");
			m_AssetPool.AddSpriteSheet("res/textures/spritesheet.png", 32, 32, 12, 0);
		}

		void TestScene::Update(float deltaTime)
		{
			m_Character->GetTransform()->GetPosition().x += 30 * deltaTime;

			for (const auto& gameObj : m_GameObjects)
			{
				gameObj->Update(deltaTime);
			}

			m_Renderer.Render();
		}
	}
}