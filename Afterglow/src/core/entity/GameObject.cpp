#include "GameObject.h"

#include "component/BaseComponent.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			GameObject::GameObject(const std::string& name) : m_Name(name), m_Components({}), m_Transform(std::make_shared<Graphics::Transform>()) {}

			GameObject::GameObject(const std::string& name, std::shared_ptr<Graphics::Transform> transform) : m_Name(name), m_Components({}), m_Transform(transform) {}

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
		}
	}
}