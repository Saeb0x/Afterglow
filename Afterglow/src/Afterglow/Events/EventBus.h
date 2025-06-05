#pragma once

#include "Event.h"

#include <typeindex>
#include <vector>

namespace Afterglow
{
	class EventBus;

	/**
	 * @brief RAII wrapper for event subscriptions. Automatically unsubscribes when destroyed.
	 */
	class EventSubscription
	{
	public:
		EventSubscription() = default;
		EventSubscription(EventBus* bus, std::type_index eventType, size_t id)
			: m_EventBus(bus), m_EventType(eventType), m_Id(id), m_Valid(true)
		{
		}

		~EventSubscription() 
		{ 
			Unsubscribe(); 
		}

		// Move-only semantics.
		EventSubscription(const EventSubscription&) = delete;
		EventSubscription& operator=(const EventSubscription&) = delete;

		EventSubscription(EventSubscription&& other) noexcept
			: m_EventBus(other.m_EventBus), m_EventType(other.m_EventType),
			m_Id(other.m_Id), m_Valid(other.m_Valid)
		{
			other.m_Valid = false;
		}

		EventSubscription& operator=(EventSubscription&& other) noexcept
		{
			if (this != &other)
			{
				Unsubscribe();
				m_EventBus = other.m_EventBus;
				m_EventType = other.m_EventType;
				m_Id = other.m_Id;
				m_Valid = other.m_Valid;
				other.m_Valid = false;
			}

			return *this;
		}

		void Unsubscribe();
		inline std::type_index GetEventType() const { return m_EventType; }
		inline size_t GetId() const { return m_Id; }
		inline bool IsValid() const { return m_Valid; }
	private:
		EventBus* m_EventBus = nullptr;
		std::type_index m_EventType = typeid(void);
		size_t m_Id = 0;
		bool m_Valid = false;
	};

	/**
	 * @brief Events in Afterglow allow parts of the codebase to communicate without tight coupling. The EventBus lets listeners subscribe to specific event types and broadcasts events to all matching subscribers when published.
	 */
	class EventBus final
	{
	public:
		static EventBus& GetInstance();

		template<typename EventType>
		EventSubscription Subscribe(std::function<bool(const EventType&)> callback)
		{
			auto& callbacksVec = m_CallbacksByEventType[typeid(EventType)];
			size_t id = m_NextId++;

			callbacksVec.emplace_back(CallbackEntry
				{
				id,
				[callback](const Event& e) 
					{
						callback(static_cast<const EventType&>(e));
					}
				}
			);

			return EventSubscription(this, typeid(EventType), id);
		}

		// We publish to exact-type listeners. May extend this to base-type listeners as well. 
		void Publish(const Event& e)
		{
			auto it = m_CallbacksByEventType.find(typeid(e));
			if (it != m_CallbacksByEventType.end())
			{
				// Copy the vector in case callbacks modify subscriptions during iteration.
				auto callbacks = it->second;
				for (const auto& entry : callbacks)
				{
					entry.callback(e);
				}
			}
		}

	private:
		void UnsubscribeById(std::type_index eventType, size_t id)
		{
			auto it = m_CallbacksByEventType.find(eventType);
			if (it == m_CallbacksByEventType.end()) return;

			auto& callbacksVec = it->second;
			callbacksVec.erase(
				std::remove_if(callbacksVec.begin(), callbacksVec.end(),
					[id](const CallbackEntry& entry) 
					{ 
						return entry.id == id; 
					}
				),
				callbacksVec.end()
			);

			// Clean up the Event if it has no listeners.
			if (callbacksVec.empty())
				m_CallbacksByEventType.erase(it);
		}

	private:
		friend class EventSubscription;

		struct CallbackEntry
		{
			size_t id;
			std::function<void(const Event&)> callback;
		};

	private:
		std::unordered_map<std::type_index, std::vector<CallbackEntry>> m_CallbacksByEventType;
		size_t m_NextId = 0;

	private:
		EventBus() = default;
		~EventBus() = default;
		EventBus(const EventBus&) = delete;
		EventBus(EventBus&&) = delete;
		EventBus& operator=(const EventBus&) = delete;
		EventBus& operator=(EventBus&&) = delete;
	};
}