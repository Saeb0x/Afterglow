#pragma once

#include "Afterglow/Core/Base.h"
#include <string>

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
		EventCategoryApplication = agBIT(0),
		EventCategoryWindow = agBIT(1),
		EventCategoryInput = agBIT(2),
		EventCategoryKeyboard = agBIT(3),
		EventCategoryMouse = agBIT(4)
	};

	class Event
	{
		friend class EventDispatcher;
	public:

		virtual EventType GetType() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) const
		{
			return GetCategoryFlags() & category;
		}
		
		inline bool IsHandled() const { return m_Handled; }
	protected:
		bool m_Handled = false;
	};
}