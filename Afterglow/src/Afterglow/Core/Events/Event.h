#pragma once

#include "Afterglow/Core/Base.h"

namespace Afterglow
{
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowMinimize, WindowMaximize, WindowFocus, WindowLostFocus,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = AG_BIT(0),
		EventCategoryWindow = AG_BIT(1),
		EventCategoryInput = AG_BIT(2),
		EventCategoryKeyboard = AG_BIT(3),
		EventCategoryMouse = AG_BIT(4)
	};

	class Event
	{
	public:
		virtual ~Event() = default;

		virtual EventType GetType() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }

		bool IsInCategory(EventCategory category) const
		{
			return GetCategoryFlags() & category;
		}

	public:
		bool b_Handled = false;
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event)
			: m_Event(event)
		{
		}
		~EventDispatcher() = default;

		// EventCallback will be deduced by the compiler.
		template<typename EventType, typename EventCallback>
		bool Dispatch(const EventCallback& callback)
		{
			AG_ASSERT_STATIC((std::is_invocable_r_v<bool, EventCallback, EventType&>), "EventCallback must be callable with (EventType&) and return bool");

			if (m_Event.GetType() == EventType::GetStaticType())
			{
				m_Event.b_Handled |= callback(static_cast<EventType&>(m_Event));
				return true;
			}

			return false;
		}

	private:
		Event& m_Event;
	};
}