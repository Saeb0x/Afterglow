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

			m_OrthographicCamera = new OrthographicCamera(0.0f, 32.0f * 40.0f, 0.0f, 32.0f * 21.0f);

			auto logoObject = std::make_shared<Entity::GameObject>("Logo Object", std::make_shared<Graphics::Transform>(glm::vec2(100,100), glm::vec2(256, 256)));
			logoObject->AddComponent(std::make_shared<Entity::Component::SpriteRenderer>(m_AssetPool.GetTexture("res/textures/logo.png")));
			AddGameObjectToScene(logoObject);

			auto pointerObject = std::make_shared<Entity::GameObject>("Pointer Object", std::make_shared<Graphics::Transform>(glm::vec2(400, 100), glm::vec2(256, 256)));
			pointerObject->AddComponent(std::make_shared<Entity::Component::SpriteRenderer>(m_AssetPool.GetTexture("res/textures/pointer.png")));
			AddGameObjectToScene(pointerObject);

			LoadResources();
		}

		void TestScene::LoadResources()
		{
			m_AssetPool.GetShader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");
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