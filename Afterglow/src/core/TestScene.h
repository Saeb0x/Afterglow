#pragma once

#include "Scene.h"
#include "graphics/Renderer.h"

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
		private:
			void Update(float deltaTime) override;
		private:
			Graphics::VertexBuffer* vbo = nullptr;
			Graphics::IndexBuffer* ibo = nullptr;
			Graphics::VertexArray* vao = nullptr;
			Graphics::VertexLayout* layout = nullptr;
			Graphics::Shader* shader = nullptr;
		};
	}
}
