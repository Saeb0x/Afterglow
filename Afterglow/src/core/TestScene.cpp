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

			int xOffset = 10;
			int yOffset = 10;
			float totalWidth = (float)(600 - xOffset * 2);
			float totalHeight = (float)(300 - yOffset * 2);
			float sizeX = totalWidth / 100.0f;
			float sizeY = totalHeight / 100.0f;

			for (int x = 0; x < 100; x++)
			{
				for (int y = 0; y < 100; y++)
				{
					float xPos = xOffset + (x * sizeX);
					float yPos = yOffset + (y * sizeY);

					auto gameObject = std::make_shared<Entity::GameObject>(("Obj" + std::to_string(x) + "" + std::to_string(y)), std::make_shared<Transform>(glm::vec2(xPos, yPos), glm::vec2(sizeX, sizeY)));
					gameObject->AddComponent(std::make_shared<Entity::Component::SpriteRenderer>(glm::vec4(xPos/totalWidth, yPos/totalHeight, 1, 1)));
					AddGameObjectToScene(gameObject);
				}
			}
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