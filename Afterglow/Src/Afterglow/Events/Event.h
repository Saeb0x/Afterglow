#pragma once

#include "agpch.h"

namespace Afterglow
{
	enum class EventType
	{
		NONE = 0,
		WindowClose, WindowResize,
		KeyPress, KeyRelease,
		MouseButtonPress, MouseButtonRelease, MouseMove
	};

	class Event
	{
	public:
		virtual ~Event() = default;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }
		inline bool IsHandled() const { return m_IsHandled; }
	public:
		bool m_IsHandled = false;
	};

	#define DEFINE_EVENT_TYPE(type)													\
		static EventType GetStaticType() { return EventType::type; }				\
		inline EventType GetEventType() const override { return GetStaticType(); }	\
		inline const char* GetName() const override { return #type; }

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& e) : m_Event(e) {}

		template<typename T, typename F>
		bool Dispatch(const F& func) const 
		{	
			if(m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.m_IsHandled = func(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};
}