#pragma once

#include "Afterglow/Window.h"

#include <GLFW/glfw3.h>

namespace Afterglow
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		~WindowsWindow();

		void OnUpdate() override;

		unsigned int GetWidth() const override;
		unsigned int GetHeight() const override;

		void SetEventCallback(const EventCallbackFunc& callback) override;

		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
	private:
		void Init(const WindowProps& props);
		void Shutdown();
	private:
		GLFWwindow* m_Window;

		struct WindowData
		{
			std::string m_Title;
			unsigned int m_Width, m_Height;
			bool m_FullScreen, m_VSync;
			EventCallbackFunc m_EventCallback;
		};

		WindowData m_Data;
	};
}

