#pragma once

#include "Event.h"

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
			if (m_Event.GetType() == EventTypeT::StaticType())
			{
				m_Event.m_IsHandled = handler(static_cast<EventTypeT&>(m_Event));
				return true;
			}

			return false;
		}

	private:
		Event& m_Event;
	};
}