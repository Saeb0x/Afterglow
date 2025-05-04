#pragma once

#include "Event.h"
#include <typeindex>

namespace Afterglow
{
	/* 
		 Events in Afterglow allow parts of the codebase to communicate without tight 
		 coupling. The EventBus lets listeners subscribe to specific event types and broadcasts 
		 events to all matching subscribers when published.
	*/
	class EventBus
	{
	public:
		static EventBus& GetInstance();

		// Returns an index we can later use to unsubscribe.
		template<typename EventType>
		size_t Subscribe(std::function<void(EventType&)> callback)
		{
			auto& listener = m_Listeners[typeid(EventType)];
			listener.push_back([callback](Event& e) 
				{
					callback(static_cast<EventType&>(e));
				}
			);

			return listener.size() - 1;
		}

		// Right now we need to explicitly unsubscribe :) NEEDS WORK.
		template<typename EventType>
		void Unsubscribe(size_t listenerIndex)
		{
			auto it = m_Listeners.find(typeid(EventType));
			if (it == m_Listeners.end()) return;
			
			auto& vec = it->second;
			if (listenerIndex < vec.size())
				vec.erase(vec.begin() + listenerIndex);
			
			// If no more listeners, drop the key entirely.
			if (vec.empty())
				m_Listeners.erase(it);
		}

		void Publish(Event& event)
		{
			// We publich to base listeners then exact-type listeners.
			auto base = m_Listeners.find(typeid(Event));
			if (base != m_Listeners.end()) {
				for (auto& listener : base->second) {
					listener(event);
				}
			}

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
		EventBus(const EventBus&) = delete;
		const EventBus& operator=(const EventBus&) = delete;

	private:
		std::unordered_map<std::type_index, std::vector<std::function<void(Event&)>>> m_Listeners;
	};
}

