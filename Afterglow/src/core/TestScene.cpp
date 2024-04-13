#include "TestScene.h"

#include "utils/ResourcePool.h"
#include "SerializationManager.h"
#include "entity/component/RigidBody.h"

#include "input/MouseListener.h"

#include <iostream>
#include <imgui.h>

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

			decorationTileset = m_ResourcePool.GetSpriteSheet("res/textures/Tileset.png");
			std::shared_ptr<Graphics::TextureAtlas> spritesheet = m_ResourcePool.GetSpriteSheet("res/textures/spritesheet.png");

			if (m_LevelLoaded)
			{
				if (m_GameObjects.at(1))
					m_ActiveGameObject = m_GameObjects.at(1);
				return;
			}

			m_Character = std::make_shared<Entity::GameObject>("Main Character", std::make_shared<Graphics::Transform>(glm::vec2(700,100), glm::vec2(256, 256)), 1);
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
			m_ResourcePool.AddSpriteSheet("res/textures/Tileset.png", 32, 32, 64, 0);
		}

		void TestScene::Update(float deltaTime)
		{			
			for (const auto& gameObj : m_GameObjects)
			{
				gameObj->Update(deltaTime);
			}

			m_Renderer.Render();
		}

		void TestScene::ImGui()
		{
			ImGui::Begin("Content Browser");

			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 itemSpacing = ImGui::GetStyle().ItemSpacing;

			float windowRMC = windowPos.x + windowSize.x;

			for (unsigned int i = 0; i < decorationTileset->GetCount(); i++)
			{
				Graphics::Sprite sprite = *decorationTileset->GetSprite(i);
				
				int spriteWidth = sprite.GetWidth() * 2;
				int spriteHeight = sprite.GetHeight() * 2;
				int id = sprite.GetId();
				std::vector<glm::vec2> texCoords = sprite.GetTextureCoordinates();

				ImGui::PushID(i);
				if (ImGui::ImageButton((ImTextureID)id, ImVec2((float)spriteWidth, (float)spriteHeight), ImVec2(texCoords[0].x, texCoords[0].y), ImVec2(texCoords[2].x, texCoords[2].y)))
				{
					std::cout << "Button " << i << " Clicked!" << std::endl;
				}
				ImGui::PopID();

				ImVec2 lastButtonPos = ImGui::GetItemRectMax();
				float nextButtonX = lastButtonPos.x + itemSpacing.x + spriteWidth;
				if (i + 1 < decorationTileset->GetCount() && nextButtonX < windowRMC)
				{
					ImGui::SameLine();
				}
			}

			ImGui::End();
		}
	}
}