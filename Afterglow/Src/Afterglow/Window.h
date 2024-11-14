#pragma once

#include "agpch.h"

#include "Events/Event.h"

namespace Afterglow
{
	struct WindowProps
	{
		unsigned int m_Width, m_Height;
		std::string m_Title;
		bool m_FullScreen, m_VSync;

		WindowProps(unsigned int width = 1280, unsigned int height = 720, const std::string& title = "Afterglow Engine", bool isFullScreen = false)
		: m_Width(width), m_Height(height), m_Title(title), m_FullScreen(m_FullScreen), m_VSync(false) {}
	};

	class Window
	{
	public:
		using EventCallbackFunc = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void SetEventCallback(const EventCallbackFunc& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};
}
