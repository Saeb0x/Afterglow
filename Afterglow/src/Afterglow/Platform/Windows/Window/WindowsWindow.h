#pragma once

#include "Afterglow/Core/Window.h"
#include "Afterglow/Core/Renderer/GraphicsContext.h"

struct GLFWwindow;

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

		uint32_t GetWidth() const override { return m_Data.Width; }
		uint32_t GetHeight() const override { return m_Data.Height; }

	private:
		void Init(const WindowProps& props);
		void Shutdown();
	
	private:
		GLFWwindow* m_Window;
		std::unique_ptr<GraphicsContext> m_Context;
		
		struct WindowData
		{
			std::string Title;
			uint32_t Width, Height;
			bool VSync;

			EventCallbackFunc EventCallback;
		};
		WindowData m_Data;
	};
}

