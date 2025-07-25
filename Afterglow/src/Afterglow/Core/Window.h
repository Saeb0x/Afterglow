#pragma once

#include <string>

namespace Afterglow
{
	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;
		bool VSync;

		WindowProps(const std::string& title = "Afterglow Engine", unsigned int width = 1280, unsigned int height = 720, bool vSync = false)
			: Title(title), Width(width), Height(height), VSync(vSync) {}
	};

	// Interface representing a desktop system based Window.
	class Window
	{
	public:
		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;
		virtual bool IsVSyncEnabled() const = 0;

		virtual void SetVSync(bool enabled) = 0;

		static Window* Create(const WindowProps& props = WindowProps());

		virtual void* GetNativeWindow() const = 0;
	};
}