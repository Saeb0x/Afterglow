#pragma once

#include "Event.h"

namespace Afterglow
{
/* 
	 Events in Afterglow allow systems to communicate without tight coupling. The 
	 EventBus lets listeners subscribe to specific event types and broadcasts events 
	 to all matching subscribers when published.
*/

	class EventBus
	{
	public:
		static EventBus& GetInstance();

		template<typename EventType>
		void Subscribe(std::function<void(EventType&)> callback)
		{
			auto& listener = m_Listeners[typeid(EventType)];
			listener.push_back([callback](Event& e) 
				{
					callback(static_cast<EventType&>(e));
				}
			);
		}

		void Publish(Event& event)
		{
			auto it = m_Listeners.find(typeid(event));
			if (it != m_Listeners.end()) {
				for (auto& listener : it->second) {
					listener(event); 
				}
			}
		}

	private:
		EventBus();
		~EventBus();
		EventBus(const EventBus&);
		const EventBus& operator=(const EventBus&);

	private:
		std::unordered_map<std::type_index, std::vector<std::function<void(Event&)>>> m_Listeners;
	};
}

