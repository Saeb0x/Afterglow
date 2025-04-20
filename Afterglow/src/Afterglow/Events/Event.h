#pragma once

namespace Afterglow
{
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowMinimize, WindowMaximize, WindowFocus, WindowLostFocus, WindowMoved,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = 1 << 0,
		EventCategoryWindow = 1 << 1,
		EventCategoryInput = 1 << 2,
		EventCategoryKeyboard = 1 << 3,
		EventCategoryMouse = 1 << 4
	};

	class Event
	{
		friend class EventDispatcher;
	public:

		virtual EventType GetType() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category) 
		{
			return GetCategoryFlags() & category;
		}
		
		inline bool IsHandled() const { return m_IsHandled; }
	protected:
		bool m_IsHandled = false;
	};

#define EVENT_CLASS_TYPE(type) \
	static EventType StaticType() { return EventType::type; } \
	virtual EventType GetType() const override { return StaticType(); } \
	virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
	virtual int GetCategoryFlags() const override { return category; }
}