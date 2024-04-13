#include "Sprite.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Graphics
		{
			Sprite::Sprite(const std::shared_ptr<Texture>& texture) : m_Width(0), m_Height(0)
			{
				m_Texture = texture;
				m_TextureCoordinates = std::vector<glm::vec2>(
					{
						glm::vec2({1.0f, 1.0f}),
						glm::vec2({1.0f, 0.0f}),
						glm::vec2({0.0f, 0.0f}),
						glm::vec2({0.0f, 1.0f})
					}
				);
			}

			Sprite::Sprite(const std::shared_ptr<Texture>& texture, const std::vector<glm::vec2>& textureCoordinatesLayout) : m_Width(0), m_Height(0)
			{
				m_Texture = texture;
				m_TextureCoordinates = textureCoordinatesLayout;
			}

			void Sprite::Serialize(rapidjson::Value& object, rapidjson::Document& document)
			{
				rapidjson::Value sprite(rapidjson::kObjectType);
				rapidjson::Value texture(rapidjson::kObjectType);

				// Serialize Texture
				if (m_Texture) {
					rapidjson::Value texturePath(rapidjson::kStringType);
					texturePath.SetString(m_Texture->GetFilePath().c_str(), static_cast<rapidjson::SizeType>(m_Texture->GetFilePath().length()), document.GetAllocator());
				
					texture.AddMember("ID", m_Texture->GetTextureID(), document.GetAllocator());
					texture.AddMember("Path", texturePath, document.GetAllocator());
					texture.AddMember("Width", m_Texture->GetWidth(), document.GetAllocator());
					texture.AddMember("Height", m_Texture->GetHeight(), document.GetAllocator());
					sprite.AddMember("Texture", texture, document.GetAllocator());
				}

				// Serialize Texture Coordinates
				rapidjson::Value textureCoordinates(rapidjson::kArrayType);
				for (const auto& texCoord : m_TextureCoordinates) {
					rapidjson::Value coord(rapidjson::kObjectType);
					coord.AddMember("x", texCoord.x, document.GetAllocator());
					coord.AddMember("y", texCoord.y, document.GetAllocator());
					textureCoordinates.PushBack(coord, document.GetAllocator());
				}
				sprite.AddMember("Texture Coordinates", textureCoordinates, document.GetAllocator());

				object.AddMember("Sprite", sprite, document.GetAllocator());
			}

			Sprite::~Sprite() {}
		}
	}
}

