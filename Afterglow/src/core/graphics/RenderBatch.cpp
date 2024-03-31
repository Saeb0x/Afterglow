#include "RenderBatch.h"

#include "../Window.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			RenderBatch::RenderBatch(unsigned int maxBatchSize) : m_MaxBatchSize(maxBatchSize), m_Sprites(maxBatchSize), m_Vertices(maxBatchSize * 4 * COUNT_PER_VERTEX)
			{
				m_Shader = Window::GetScene()->GetAssetPool().GetShader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");
			}

			RenderBatch::~RenderBatch() {}
			
			void RenderBatch::Start()
			{
				m_VAO = std::make_shared<VertexArray>();

				m_VBO = std::make_shared<VertexBuffer>((const void*)0, static_cast<unsigned int>(m_Vertices.size() * sizeof(float)));

				m_Layout = std::make_shared<VertexLayout>();
				m_Layout->Push<float>(2);
				m_Layout->Push<float>(4);

				m_VAO->AddBuffer(*m_VBO, *m_Layout);

				std::vector<unsigned int> indices = GenerateIndices();
				m_IBO = std::make_shared<IndexBuffer>(indices.data(), static_cast<unsigned int>(indices.size()));
			}

			void RenderBatch::Render()
			{
				// Rebuffer all data every frame
				m_VBO->Bind();
				GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, m_Vertices.size() * sizeof(float), m_Vertices.data()));

				m_Shader->Bind();
				m_Shader->SetUniformMatrix4fv("u_ProjectionViewMatrix", Window::GetScene()->GetCamera()->GetProjectionViewMatrix());

				m_VAO->Bind();
				m_VAO->AddBuffer(*m_VBO, *m_Layout);
				
				m_IBO->Bind();
				GLCall(glDrawElements(GL_TRIANGLES, m_SpritesCount * 6, GL_UNSIGNED_INT, 0));

				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
				m_VAO->Unbind();
				m_VBO->Unbind();
				m_IBO->Unbind();
				m_Shader->Unbind();
			}

			void RenderBatch::AddSprite(std::shared_ptr<Entity::Component::SpriteRenderer> spriteRenderer)
			{
				int index = m_SpritesCount;
				
				m_Sprites[index] = spriteRenderer; 
				m_SpritesCount++;

				// Properties to local vertices array
				LoadVertexProperties(index);

				if (m_SpritesCount >= m_MaxBatchSize)
				{
					m_HasRoom = false;
				}
			}

			void RenderBatch::LoadVertexProperties(int index)
			{
				std::shared_ptr<Entity::Component::SpriteRenderer> sr = m_Sprites[index];

				// Offset within array (4 vertices per sprite)
				int offset = index * 4 * COUNT_PER_VERTEX;

				glm::vec4 color = sr->GetColor();

				float xAdd = 1.0f;
				float yAdd = 1.0f;

				for (short i = 0; i < 4; i++)
				{
					if (i == 1)
						yAdd = 0.0f;
					else if (i == 2)
						xAdd = 0.0f;
					else if (i == 3)
						yAdd = 1.0f;
			
					// Load Position
					m_Vertices[offset] = sr->GetOwner()->GetTransform()->GetPosition().x + (xAdd * sr->GetOwner()->GetTransform()->GetScale().x);
					m_Vertices[offset + 1] = sr->GetOwner()->GetTransform()->GetPosition().y + (yAdd * sr->GetOwner()->GetTransform()->GetScale().y);

					// Load Color
					m_Vertices[offset + 2] = color.x;
					m_Vertices[offset + 3] = color.y;
					m_Vertices[offset + 4] = color.z;
					m_Vertices[offset + 5] = color.w;

					offset += COUNT_PER_VERTEX;
				}
			}

			std::vector<unsigned int> RenderBatch::GenerateIndices()
			{
				// 6 indices per quad (3 per triangle)
				std::vector<unsigned int> elements(6 * m_MaxBatchSize);

				for (unsigned int i = 0; i < m_MaxBatchSize; i++)
				{
					LoadElementIndices(elements, i);
				}

				return elements;
			}

			void RenderBatch::LoadElementIndices(std::vector<unsigned int>& elements, unsigned int index)
			{
				int offsetArrayIndex = 6 * index;
				int offset = 4 * index;

				// 3, 2, 0, 0, 2, 1
				
				// Triangle 1
				elements[offsetArrayIndex] = offset + 3;
				elements[offsetArrayIndex + 1] = offset + 2;
				elements[offsetArrayIndex + 2] = offset + 0;

				// Triangle 2
				elements[offsetArrayIndex + 3] = offset + 0;
				elements[offsetArrayIndex + 4] = offset + 2;
				elements[offsetArrayIndex + 5] = offset + 1;
			}
		}
	}
}