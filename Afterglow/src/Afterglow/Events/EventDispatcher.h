#pragma once

#include "Event.h"
#include <typeindex>

namespace Afterglow
{
	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event) 
		{
		}

		template <typename EventType>
		bool Dispatch(std::function<bool(const EventType&)> handler)
		{
			if (typeid(m_Event) == typeid(EventType))
			{
				m_Event.m_Handled = handler(static_cast<const EventType&>(m_Event));
				return true;
			}

			return false;
		}

	private:
		Event& m_Event;
	};
}