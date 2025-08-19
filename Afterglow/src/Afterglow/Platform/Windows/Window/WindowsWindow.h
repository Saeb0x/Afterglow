#pragma once

#include "Afterglow/Core/Window.h"

#include <GLFW/glfw3.h>

namespace Afterglow
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		~WindowsWindow() override;

		void* GetNativeWindow() const override { return m_Window; };

		void OnUpdate() override;

		void SetEventCallback(const EventCallbackFunc& callback) override { m_Data.EventCallback = callback; }

		void SetVSync(bool enabled) override;
		bool IsVSyncEnabled() const override { return m_Data.VSync; }

		unsigned int GetWidth() const override { return m_Data.Width; }
		unsigned int GetHeight() const override { return m_Data.Height; }

	private:
		void Init(const WindowProps& props);
		void Shutdown();
	
	private:
		GLFWwindow* m_Window;
		
		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;

			EventCallbackFunc EventCallback;
		};
		WindowData m_Data;
	};
}

