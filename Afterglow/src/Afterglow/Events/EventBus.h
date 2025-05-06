#pragma once

#include "Event.h"

#include <typeindex>
#include <vector>

namespace Afterglow
{
	/* 
		 Events in Afterglow allow parts of the codebase to communicate without tight 
		 coupling. The EventBus lets listeners subscribe to specific event types and broadcasts 
		 events to all matching subscribers when published.
	*/
	
	using EventCallbackFn = std::function<void(Event&)>;
	
	class EventBus
	{
	public:
		static EventBus& GetInstance();

		// Returns an index we can later use to unsubscribe.
		template<typename EventType>
		size_t Subscribe(EventCallbackFn callback)
		{
			auto& callbacksVec = m_CallbacksByEventType[typeid(EventType)];
			callbacksVec.push_back([callback](Event& e)
				{
					callback(static_cast<EventType&>(e));
				}
			);

			return callbacksVec.size() - 1;
		}

		template<typename EventType>
		void Unsubscribe(size_t listenerIndex)
		{
			auto it = m_CallbacksByEventType.find(typeid(EventType));
			if (it == m_CallbacksByEventType.end()) return;

			auto& callbacksVec = it->second;
			if (listenerIndex < callbacksVec.size())
				callbacksVec.erase(callbacksVec.begin() + listenerIndex);

			// If no more listeners, drop the key entirely.
			if (callbacksVec.empty())
				m_CallbacksByEventType.erase(it);
		}

		void Publish(Event& e)
		{
			// We publish to base-type listeners then exact-type listeners.
			auto baseType = m_CallbacksByEventType.find(typeid(Event));
			if (baseType != m_CallbacksByEventType.end()) {
				for (auto& listener : baseType->second) {
					listener(e);
				}
			}

			auto exactType = m_CallbacksByEventType.find(typeid(e));
			if (exactType != m_CallbacksByEventType.end()) {
				for (auto& listener : baseType->second) {
					listener(e);
				}
			}
		}

	private:
		EventBus() = default;
		~EventBus() = default;
		EventBus(const EventBus&) = delete;
		const EventBus& operator=(const EventBus&) = delete;

	private:
		std::unordered_map<std::type_index, std::vector<EventCallbackFn>> m_CallbacksByEventType;
	};
}