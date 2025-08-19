#pragma once

#include <string>
#include <functional>

#include "Events/Event.h"

namespace Afterglow
{
	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;
		bool VSync;

		WindowProps(const std::string& title = "Afterglow Engine", unsigned int width = 1280, unsigned int height = 720, bool vSync = true)
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

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;
	};
}