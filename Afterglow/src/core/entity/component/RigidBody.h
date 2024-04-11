#pragma once

#include "BaseComponent.h"

#include <glm/glm.hpp>

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			namespace Component
			{
				class RigidBody : public BaseComponent
				{
				public:
					RigidBody();
					RigidBody(int colliderType, float friction, const glm::vec3& velocity);

					~RigidBody();

					inline int GetColliderType() const { return m_ColliderType; }
					inline float GetFriction() const { return m_Friction; }
					inline glm::vec3& GetVelocity() { return m_Velocity; }

					void Start() override;
					void Update(float deltaTime) override;
					void ImGui() override;
					void Serialize(rapidjson::Value& object, rapidjson::Document& document) override;

				private:
					int m_ColliderType;
					float m_Friction;
					glm::vec3 m_Velocity;
				};
			}
		}
	}
}

