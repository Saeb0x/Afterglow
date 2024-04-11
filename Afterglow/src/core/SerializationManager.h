#pragma once

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <fstream>
#include <string>
#include <iostream>
#include <glm/glm.hpp>

#include "entity/component/SpriteRenderer.h"
#include "entity/component/RigidBody.h"

namespace Afterglow
{
	namespace Core
	{
		class SerializationManager
		{
		public:
			static SerializationManager& GetInstance();

			static std::string Serialize(const std::vector<std::shared_ptr<Entity::GameObject>>& gameObjects, const std::string& filePath);

			static std::vector<std::shared_ptr<Entity::GameObject>> Deserialize(const std::string& jsonString);
			static std::shared_ptr<Entity::GameObject> DeserializeGameObject(const rapidjson::Value& object);
			static std::shared_ptr<Entity::Component::SpriteRenderer> DeserializeSpriteRenderer(const rapidjson::Value& object);
			static std::shared_ptr<Entity::Component::RigidBody> DeserializeRigidBody(const rapidjson::Value& object);

		private:
			SerializationManager();
			~SerializationManager();
			SerializationManager(const SerializationManager&);
			const SerializationManager& operator=(const SerializationManager&);
		};
	}
}
