#pragma once

#include <GL/glew.h>

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"

#include "../entity/GameObject.h"
#include "../entity/component/SpriteRenderer.h"

namespace Afterglow {
	namespace Core {
		namespace Graphics
		{
			#define ASSERT(x) if (!x) __debugbreak();
			#define GLCall(x) GLClearError();\
				x;\
				ASSERT(GLLogError(#x, __FILE__, __LINE__))

			void GLClearError();
			bool GLLogError(const char* function, const char* file, int line);

			class RenderBatch; // Forward declaration to avoid circular dependency

			class Renderer
			{
			public:
				static Renderer& GetInstance();
				void Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader);

				void Add(const std::shared_ptr<Entity::GameObject>& gameObject);
				void Add(const std::shared_ptr<Entity::Component::SpriteRenderer>& spriteRenderer);
				void Render();
			private:
				Renderer();
				~Renderer();
				Renderer(const Renderer&);
				const Renderer& operator=(const Renderer&);
			private:
				const int MAX_BATCH_SIZE = 1000;
				std::vector<std::shared_ptr<RenderBatch>> m_RenderBatches = std::vector<std::shared_ptr<RenderBatch>>();
			};
		}
	}
}

