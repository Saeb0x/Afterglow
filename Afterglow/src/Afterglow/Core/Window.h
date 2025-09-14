#pragma once

#include "Events/Event.h"

namespace Afterglow
{
	struct WindowProps
	{
		std::string Title;
		uint16_t Width;
		uint16_t Height;
		bool VSync;

		WindowProps(const std::string& title = "Afterglow Engine", uint16_t width = 1280, uint16_t height = 720, bool vSync = true)
			: Title(title), Width(width), Height(height), VSync(vSync) 
		{
		}
	};

	// Interface representing a desktop system based Window.
	class Window
	{
	public:
		virtual ~Window() = default;

		static Window* Create(const WindowProps& props);
		virtual void* GetNativeWindow() const = 0;

		virtual void OnUpdate() = 0;

		using EventCallbackFunc = std::function<void(Event&)>;
		virtual void SetEventCallback(const EventCallbackFunc& callback) = 0;

		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSyncEnabled() const = 0;

		virtual uint16_t GetWidth() const = 0;
		virtual uint16_t GetHeight() const = 0;
	};
}