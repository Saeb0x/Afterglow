#include "RigidBody.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			namespace Component
			{
				RigidBody::RigidBody() : BaseComponent(), m_ColliderType(0), m_Friction(0.8f), m_Velocity(glm::vec3({ 0.0f, 0.0f, 0.0f })) {}
				RigidBody::RigidBody(int colliderType, float friction, const glm::vec3& velocity) : BaseComponent(), m_ColliderType(colliderType), m_Friction(friction), m_Velocity(velocity) {}
				
				RigidBody::~RigidBody() {}

				void RigidBody::Start() {}
				void RigidBody::Update(float deltaTime) {}

				void RigidBody::ImGui()
				{
					ImGui::InputInt("Collider Type", &m_ColliderType);
					ImGui::InputFloat("Friction", &m_Friction);
					ImGui::InputFloat3("Velocity", glm::value_ptr(m_Velocity));
				}

				void RigidBody::Serialize(rapidjson::Value& object, rapidjson::Document& document)
				{
                    if (!document.IsArray()) {
                        std::cerr << "RapidJSON Document is not an array" << std::endl;
                        return;
                    }

					rapidjson::Value rigidBody(rapidjson::kObjectType);

					rigidBody.AddMember("ColliderType", m_ColliderType, document.GetAllocator());
					rigidBody.AddMember("Friction", m_Friction, document.GetAllocator());

                    rapidjson::Value velocity(rapidjson::kObjectType);
                    velocity.AddMember("x", m_Velocity.x, document.GetAllocator());
                    velocity.AddMember("y", m_Velocity.y, document.GetAllocator());
                    velocity.AddMember("z", m_Velocity.z, document.GetAllocator());
					rigidBody.AddMember("Velocity", velocity, document.GetAllocator());

					object.AddMember("Rigid Body", rigidBody, document.GetAllocator());
				}

			}
		}
	}
}