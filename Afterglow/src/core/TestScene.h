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
			Graphics::VertexBuffer* vbo;
			Graphics::IndexBuffer* ibo;
			Graphics::VertexArray* vao;
			Graphics::VertexLayout* layout;
			Graphics::Shader* shader;
		};
	}
}
