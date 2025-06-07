#pragma once

#include "Afterglow/Events/Event.h"

namespace Afterglow
{
	class Layer
	{
	public:
		Layer(const std::string& debugName = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetDebugName() const { return m_DebugName; }

	protected:
		std::string m_DebugName;
	};
}