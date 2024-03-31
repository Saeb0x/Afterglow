#pragma once

#include <vector>
#include <memory>

#include "../entity/component/SpriteRenderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "IndexBuffer.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			class RenderBatch
			{
			public:
				RenderBatch(unsigned int maxBatchSize);
				~RenderBatch();

				void Start();
				void Render();

				inline unsigned int GetMaxBatchSize() const { return m_MaxBatchSize; }
				inline bool HasRoom() const { return m_HasRoom; }

				void AddSprite(std::shared_ptr<Entity::Component::SpriteRenderer> spriteRenderer);
			private:
				std::vector<unsigned int> GenerateIndices();
				void LoadElementIndices(std::vector<unsigned int>& elements, unsigned int index);
				void LoadVertexProperties(int index);
			private:
				const int COUNT_PER_VERTEX = 9;
	
				unsigned int m_MaxBatchSize;
				std::vector<std::shared_ptr<Entity::Component::SpriteRenderer>> m_Sprites;
				unsigned int m_SpritesCount = 0;
				bool m_HasRoom = true;
				std::vector<float> m_Vertices;
		
				std::vector<std::shared_ptr<Graphics::Texture>> m_Textures;
				int m_TextureSlots[8] = { 0,1,2,3,4,5,6,7 };

				std::shared_ptr<Shader> m_Shader;
				std::shared_ptr<VertexArray> m_VAO = nullptr;
				std::shared_ptr<VertexBuffer> m_VBO = nullptr;
				std::shared_ptr<VertexLayout> m_Layout = nullptr;
				std::shared_ptr<IndexBuffer> m_IBO = nullptr;
			};
		}
	}
}