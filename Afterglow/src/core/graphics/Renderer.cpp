#include "Renderer.h"

#include <iostream>
#include <algorithm>

#include "RenderBatch.h"

namespace Afterglow {
	namespace Core {
		namespace Graphics
		{
			void GLClearError()
			{
				while (glGetError() != GL_NO_ERROR);
			}

			bool GLLogError(const char* function, const char* file, int line)
			{
				while (unsigned int error = glGetError())
				{
					std::cout << "[OpenGL Error: " << error << "] " << function << " " << file << " " << line << std::endl;
					return false;
				}

				return true;
			}

			Renderer& Renderer::GetInstance()
			{
				static Renderer rendererInstance;
				return rendererInstance;
			}

			void Renderer::Draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader)
			{
				vao.Bind();
				ibo.Bind();
				shader.Bind();

				GLCall(glDrawElements(GL_TRIANGLES, ibo.GetCount(), GL_UNSIGNED_INT, nullptr));
			}

			void Renderer::Add(const std::shared_ptr<Entity::GameObject>& gameObject)
			{
				auto spriteRenderer = gameObject->GetComponent<Entity::Component::SpriteRenderer>();
				if (spriteRenderer != nullptr)
				{
					Add(spriteRenderer);
				}
			}

			void Renderer::Add(const std::shared_ptr<Entity::Component::SpriteRenderer>& spriteRenderer)
			{
				bool added = false;
				for (const auto& batch : m_RenderBatches)
				{
					if (batch->HasRoom() && batch->GetZIndex() == spriteRenderer->GetOwner()->GetZIndex())
					{
						std::shared_ptr<Graphics::Texture> tex = spriteRenderer->GetTexture();
						if (!tex || (batch->HasTexture(tex) || batch->HasTextureRoom()))
						{
							batch->AddSprite(spriteRenderer);
							added = true;
							break;
						}
					}
				}

				if (!added)
				{
					std::shared_ptr<RenderBatch> newBatch = std::make_shared<RenderBatch>(MAX_BATCH_SIZE, spriteRenderer->GetOwner()->GetZIndex());
					newBatch->Start();
					m_RenderBatches.push_back(newBatch);
					newBatch->AddSprite(spriteRenderer);

					std::sort(m_RenderBatches.begin(), m_RenderBatches.end(), [](const std::shared_ptr<RenderBatch>& batch1, const std::shared_ptr<RenderBatch>& batch2) {
						return batch1->GetZIndex() < batch2->GetZIndex(); });

				}
			}

			void Renderer::Render()
			{
				for (const auto& batch : m_RenderBatches)
				{
					batch->Render();
				}
			}

			Renderer::Renderer() {}
			Renderer::~Renderer() {}
		}
	}
}

