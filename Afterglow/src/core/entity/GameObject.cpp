#include "GameObject.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			GameObject::GameObject(const std::string& name) : m_Name(name), m_Components({}) {}

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