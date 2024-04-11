#include "GameObject.h"

#include "component/BaseComponent.h"
#include "../SerializationManager.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			GameObject::GameObject(const std::string& name) : m_Name(name), m_Components({}), m_Transform(std::make_shared<Graphics::Transform>()), m_ZIndex(0) {}

			GameObject::GameObject(const std::string& name, std::shared_ptr<Graphics::Transform> transform, int zIndex) : m_Name(name), m_Components({}), m_Transform(transform), m_ZIndex(zIndex) {}

			GameObject::~GameObject() {}

			void GameObject::Start()
			{
				for (const auto& component : m_Components)
				{
					component->Start();
				}
			}

			void GameObject::Update(float deltaTime)
			{
				for (const auto& component : m_Components)
				{
					component->Update(deltaTime);
				}
			}

			void GameObject::ImGui()
			{
				for (const auto& component : m_Components)
				{
					component->ImGui();
				}
			}

			void GameObject::Serialize(rapidjson::Value& object, rapidjson::Document& document)
			{
				for (const auto& component : m_Components)
				{
					component->Serialize(object, document);
				}
			}

			void GameObject::Deserialize()
			{
			}
		}
	}
}