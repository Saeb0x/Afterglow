#pragma once

#include "Event.h"
#include "Afterglow/Core/Log.h"

namespace Afterglow
{
	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event) {}

		template <typename EventTypeT, typename EventHandlerFunc>
		bool Dispatch(const EventHandlerFunc& handler)
		{
			if (m_Event.GetType == EventTypeT::StaticType())
			{
				m_Event.IsHandled = handler(static_cast<EventTyepT&>(m_Event));
				return true;
			}

			AG_LOG_WARNING("Event Dispatcher: type mismatch...");
			return false;
		}

	private:
		Event& m_Event;
	};
}