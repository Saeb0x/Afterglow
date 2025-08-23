#pragma once

#include "Event.h"

#include <string>
#include <sstream>

namespace Afterglow
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(uint32_t width, uint32_t height)
			: m_Width(width), m_Height(height) 
		{
		}
		~WindowResizeEvent() override = default;

		static EventType GetStaticType() { return EventType::WindowResize; }
		EventType GetType() const override { return GetStaticType(); }
		const char* GetName() const override { return "WindowResizeEvent"; }
		int GetCategoryFlags() const override { return EventCategoryWindow; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Window Resize Event: " << m_Width << ", " << m_Height;
			return ss.str();
		}

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
	
	private:
		uint32_t m_Width, m_Height;
	};


	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() = default;
		~WindowCloseEvent() override = default;

		static EventType GetStaticType() { return EventType::WindowClose; }
		EventType GetType() const override { return GetStaticType(); }
		const char* GetName() const override { return "WindowCloseEvent"; }
		int GetCategoryFlags() const override { return EventCategoryWindow; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "Window Close Event!";
			return ss.str();
		}
	};
}