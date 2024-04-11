#include "SerializationManager.h"
#include "entity/GameObject.h"

#include <unordered_set>

namespace Afterglow
{
	namespace Core
	{
		SerializationManager::SerializationManager() {}
		SerializationManager::~SerializationManager() {}

		SerializationManager& SerializationManager::GetInstance()
		{
			static SerializationManager serializeInstance;
			return serializeInstance;
		}

		std::string SerializationManager::Serialize(const std::vector<std::shared_ptr<Entity::GameObject>>& gameObjects, const std::string& filePath)
		{
			rapidjson::Document document;
			document.SetArray(); // Top-level object to be an array

			for (const std::shared_ptr<Entity::GameObject>& object : gameObjects)
			{
				// Remove any existing objects with the same name
				for (rapidjson::Value::ValueIterator itr = document.Begin(); itr != document.End(); )
				{
					if ((*itr)["Name"].IsString() && (*itr)["Name"].GetString() == object->GetName())
					{
						itr = document.Erase(itr);
					}
					else
					{
						++itr;
					}
				}

				// Add or update the object in the document
				rapidjson::Value gameObject(rapidjson::kObjectType);
				rapidjson::Value objectName(object->GetName().c_str(), static_cast<rapidjson::SizeType>(object->GetName().length()), document.GetAllocator());
				gameObject.AddMember("Name", objectName, document.GetAllocator());

				object->Serialize(gameObject, document);
				document.PushBack(gameObject, document.GetAllocator());
			}

			// Serialize the JSON document to a string
			rapidjson::StringBuffer buffer;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
			document.Accept(writer);

			// Write the serialized JSON string to a file
			std::ofstream outFile(filePath);
			if (outFile.is_open()) {
				outFile << buffer.GetString();
				outFile.close();
			}
			else {
				std::cerr << "Error: Unable to open file " << filePath << std::endl;
			}

			return buffer.GetString();
		}

		std::vector<std::shared_ptr<Entity::GameObject>> SerializationManager::Deserialize(const std::string& jsonString)
		{
			std::vector<std::shared_ptr<Entity::GameObject>> gameObjects;

			rapidjson::Document document;
			document.Parse(jsonString.c_str());

			if (!document.IsArray()) {
				throw std::runtime_error("Invalid JSON format: expected an array");
			}

			for (rapidjson::SizeType i = 0; i < document.Size(); ++i) {
				const rapidjson::Value& object = document[i];
				std::shared_ptr<Entity::GameObject> gameObject = DeserializeGameObject(object);
				if (gameObject) {
					gameObjects.push_back(gameObject);
				}
			}

			return gameObjects;
		}

		std::shared_ptr<Entity::GameObject> SerializationManager::DeserializeGameObject(const rapidjson::Value& object)
		{
			if (!object.IsObject()) {
				throw std::runtime_error("Invalid JSON format: Expected an object");
			}

			std::string gameObjectName = "Unnamed GameObject"; // Default name if not specified in JSON
			if (object.HasMember("Name") && object["Name"].IsString()) {
				gameObjectName = object["Name"].GetString();
			}

			int gameObjectZIndex = 0;
			if (object.HasMember("Z-Index") && object["Z-Index"].IsNumber()) {
				gameObjectZIndex = object["Z-Index"].GetInt();
			}

			glm::vec2 Position = glm::vec2({0.0f,0.0f});
			glm::vec2 Scale = glm::vec2({ 0.0f,0.0f });

			// Deserialize Transform
			if (object.HasMember("Transform") && object["Transform"].IsObject()) {
				const auto& transform = object["Transform"];

				if (transform.HasMember("Position") && transform["Position"].IsObject()) {
					const auto& position = transform["Position"];
					if (position.HasMember("x") && position.HasMember("y")) {
						float posX = position["x"].GetFloat();
						float posY = position["y"].GetFloat();
						
						Position = glm::vec2(posX, posY);
					}
				}

				if (transform.HasMember("Scale") && transform["Scale"].IsObject()) {
					const auto& scale = transform["Scale"];
					if (scale.HasMember("x") && scale.HasMember("y")) {
						float scaleX = scale["x"].GetFloat();
						float scaleY = scale["y"].GetFloat();
						
						Scale = glm::vec2(scaleX, scaleY);
					}
				}
			}
			std::shared_ptr<Entity::GameObject> gameObject = std::make_shared<Entity::GameObject>(gameObjectName, std::make_shared<Graphics::Transform>(Position, Scale), gameObjectZIndex);

			// Deserialize SpriteRenderer component
			if (object.HasMember("Sprite Renderer") && object["Sprite Renderer"].IsObject()) {
				const auto& spriteRenderer = object["Sprite Renderer"];
				std::shared_ptr<Entity::Component::SpriteRenderer> sr = DeserializeSpriteRenderer(spriteRenderer);
				if (sr) {
					gameObject->AddComponent(sr);
				}
			}

			// Deserialize RigidBody component
			if (object.HasMember("Rigid Body") && object["Rigid Body"].IsObject()) {
				const auto& rigigBody = object["Rigid Body"];
				std::shared_ptr<Entity::Component::RigidBody> rb = DeserializeRigidBody(rigigBody);
				if (rb) {
					gameObject->AddComponent(rb);
				}
			}

			return gameObject;
		}

		std::shared_ptr<Entity::Component::SpriteRenderer> SerializationManager::DeserializeSpriteRenderer(const rapidjson::Value& object)
		{
			if (!object.IsObject()) {
				throw std::runtime_error("Invalid JSON format: Expected an object for SpriteRenderer");
			}

			if (object.HasMember("Color") && object["Color"].IsObject()) {
				const auto& color = object["Color"];

				if (!color.IsObject()) {
					throw std::runtime_error("Invalid JSON format: Missing color data for SpriteRenderer");
				}

				float r = color["r"].GetFloat();
				float g = color["g"].GetFloat();
				float b = color["b"].GetFloat();
				float a = color["a"].GetFloat();
				glm::vec4 colorVec(r, g, b, a);

				if (object.HasMember("Sprite") && object["Sprite"].IsObject()) {
					const auto& spriteObj = object["Sprite"];

					// Deserialize the texture
					if (spriteObj.HasMember("Texture") && spriteObj["Texture"].IsObject()) {
						const auto& textureObj = spriteObj["Texture"];

						// Extract texture properties
						std::string texturePath = textureObj["Path"].GetString();
						unsigned int textureID = textureObj["ID"].GetUint();
						int width = textureObj["Width"].GetInt();
						int height = textureObj["Height"].GetInt();

						auto texture = std::make_shared<Graphics::Texture>(texturePath);
						texture->SetWidth(width);
						texture->SetHeight(height);

						// Deserialize the texture coordinates
						if (spriteObj.HasMember("Texture Coordinates") && spriteObj["Texture Coordinates"].IsArray()) {
							const auto& textureCoordsArray = spriteObj["Texture Coordinates"];
							std::vector<glm::vec2> textureCoordinates;
							for (rapidjson::SizeType i = 0; i < textureCoordsArray.Size(); ++i) {
								const auto& coordObj = textureCoordsArray[i];
								float x = coordObj["x"].GetFloat();
								float y = coordObj["y"].GetFloat();
								textureCoordinates.push_back(glm::vec2(x, y));
							}

							// Create and set the sprite
							auto sprite = std::make_shared<Graphics::Sprite>(texture, textureCoordinates);

							// Create a new SpriteRenderer component with the deserialized sprite
							return std::make_shared<Entity::Component::SpriteRenderer>(sprite);
						}
					}
				}

				// Create a new SpriteRenderer component with the deserialized color
				return std::make_shared<Entity::Component::SpriteRenderer>(colorVec);
			}

			throw std::runtime_error("Invalid JSON format: SpriteRenderer data not found");
		}

		std::shared_ptr<Entity::Component::RigidBody> SerializationManager::DeserializeRigidBody(const rapidjson::Value& object)
		{
			if (!object.IsObject()) {
				throw std::runtime_error("Invalid JSON format: Expected an object for RigidBody");
			}

			if (object.HasMember("ColliderType") && object["ColliderType"].IsInt() &&
				object.HasMember("Friction") && object["Friction"].IsFloat() &&
				object.HasMember("Velocity") && object["Velocity"].IsObject()) {

				int colliderType = object["ColliderType"].GetInt();
				float friction = object["Friction"].GetFloat();

				const auto& velocityObj = object["Velocity"];
				float velocityX = velocityObj["x"].GetFloat();
				float velocityY = velocityObj["y"].GetFloat();
				float velocityZ = velocityObj["z"].GetFloat();
				glm::vec3 velocity(velocityX, velocityY, velocityZ);

				return std::make_shared<Entity::Component::RigidBody>(colliderType, friction, velocity);
			}

			throw std::runtime_error("Invalid JSON format: RigidBody data not found or incomplete");
		}
	}
}