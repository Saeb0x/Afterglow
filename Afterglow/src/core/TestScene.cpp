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

			std::shared_ptr<Graphics::TextureAtlas> spritesheet = m_AssetPool.GetSpriteSheet("res/textures/Tileset.png");

			auto logoObject = std::make_shared<Entity::GameObject>("Tile28 Object", std::make_shared<Graphics::Transform>(glm::vec2(100,100), glm::vec2(256, 256)));
			logoObject->AddComponent(std::make_shared<Entity::Component::SpriteRenderer>(spritesheet->GetSprite(27)));
			AddGameObjectToScene(logoObject);

			auto pointerObject = std::make_shared<Entity::GameObject>("Tile53 Object", std::make_shared<Graphics::Transform>(glm::vec2(400, 100), glm::vec2(256, 256)));
			pointerObject->AddComponent(std::make_shared<Entity::Component::SpriteRenderer>(spritesheet->GetSprite(52)));
			AddGameObjectToScene(pointerObject);
		}

		void TestScene::LoadResources()
		{
			m_AssetPool.AddShader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");
			m_AssetPool.AddSpriteSheet("res/textures/Tileset.png", 32, 32, 65, 0);
		}

		void TestScene::Update(float deltaTime)
		{
			for (const auto& gameObj : m_GameObjects)
			{
				gameObj->Update(deltaTime);
			}

			m_Renderer.Render();
		}
	}
}