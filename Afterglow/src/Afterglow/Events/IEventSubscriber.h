#pragma once

#include "EventBus.h"

namespace Afterglow
{
	/**
	 * @brief Interface for classes that want to subscribe to events with automatic management.
	 */
	class IEventSubscriber
	{
	public:
		virtual ~IEventSubscriber() = default;

	protected:
		template<typename EventType>
		void Subscribe(std::function<void(EventType&)> callback)
		{
			auto subscription = EventBus::GetInstance().Subscribe<EventType>(callback);
			m_Subscriptions.push_back(std::move(subscription));
		}

		void UnsubscribeAll()
		{
			m_Subscriptions.clear();
		}

		void LogSubscriptions() const
		{
			AG_LOG_INFO("[IEventSubscriber] Active Subscriptions:\n");
			for (const auto& sub : m_Subscriptions)
			{
				AG_LOG_INFO(" - {0} {1} (ID:{2})\n", sub.IsValid() ? "[ACTIVE]" : "[INVALID]", sub.GetEventType().name(), sub.GetId());
			}
		}

	private:
		std::vector<EventSubscription> m_Subscriptions;
	};
}
