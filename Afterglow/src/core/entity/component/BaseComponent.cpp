#include "BaseComponent.h"

#include "../GameObject.h"

namespace Afterglow
{
	namespace Core
	{
		namespace Entity
		{
			namespace Component
			{
				BaseComponent::BaseComponent() : m_Owner(nullptr) {}

				void BaseComponent::Start() {}
				void BaseComponent::ImGui() {}
				void BaseComponent::Serialize(rapidjson::Value& object, rapidjson::Document& document) {}
			}
		}
	}
}
