#pragma once

#include "Afterglow/Core/Window.h"

#include <GLFW/glfw3.h>

namespace Afterglow
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }
		inline bool IsVSync() const override { return m_Data.VSync; }

		void SetVSync(bool enabled) override;

	private:
		void Init(const WindowProps& props);
		void Shutdown();
	
	private:
		GLFWwindow* m_Window;
		WindowProps m_Data;
	};
}

