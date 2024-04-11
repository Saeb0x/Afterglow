#pragma once

#include <rapidjson/document.h>
#include <memory>

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			class GameObject; // Forward declaration to avoid circular dependency

			namespace Component
			{
				class BaseComponent
				{
				public:
					BaseComponent();
					virtual ~BaseComponent() = default;

					inline GameObject* GetOwner() const { return m_Owner; }
					inline void SetOwner(GameObject* owner) { m_Owner = owner; }

					virtual void Start();
					virtual void Update(float deltaTime) = 0;
					virtual void ImGui();

					virtual void Serialize(rapidjson::Value& object, rapidjson::Document& document);
				private:
					GameObject* m_Owner;
				};
			}
		}
	}
}

