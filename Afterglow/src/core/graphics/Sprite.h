#pragma once

#include "Texture.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <rapidjson/document.h>

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			class Sprite
			{
			public:
				Sprite(const std::shared_ptr<Texture>& texture);
				Sprite(const std::shared_ptr<Texture>& texture, const std::vector<glm::vec2>& textureCoordinatesLayout);

				inline void SetWidth(int width) { m_Width = width; }
				inline void SetHeight(int height) { m_Height = height; }

				inline const std::shared_ptr<Texture>& GetTexture() const { return m_Texture; }
				inline const std::vector<glm::vec2>& GetTextureCoordinates() const { return m_TextureCoordinates; }
				inline int GetWidth() const { return m_Width; }
				inline int GetHeight() const { return m_Height; }
				inline unsigned int GetId() const { return m_Texture->GetTextureID(); }

				void Serialize(rapidjson::Value& object, rapidjson::Document& document);

				~Sprite();

			private:
				std::shared_ptr<Texture> m_Texture;
				std::vector<glm::vec2> m_TextureCoordinates;
				int m_Width;
				int m_Height;
			};
		}
	}
}

