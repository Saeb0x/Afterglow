#include "agpch.h"
#include "EventBus.h"

namespace Afterglow
{
	EventBus& Afterglow::EventBus::GetInstance()
	{
		static EventBus instance;
		return instance;
	}

	void EventSubscription::Unsubscribe()
	{
		if (m_Valid && m_EventBus)
		{
			m_EventBus->UnsubscribeById(m_EventType, m_Id);
			m_Valid = false;
		}
	}
}