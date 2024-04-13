#pragma once

#include "Scene.h"

namespace Afterglow
{
	namespace Core
	{
		class TestScene : public Scene
		{
		public:
			TestScene();
			~TestScene();

			void Init() override;
			void Update(float deltaTime) override;

			void ImGui() override;
		private:
			void LoadResources();
		private:
			std::shared_ptr<Entity::GameObject> m_Character = nullptr;
			std::shared_ptr<Entity::Component::SpriteRenderer> spriteRenderer = nullptr;
			std::shared_ptr<Graphics::TextureAtlas> decorationTileset = nullptr;
		};
	}
}
