#pragma once

#include <memory>

namespace Afterglow
{
	struct LoggerConfig;
	struct WindowConfig;
	class Window;

	class Application
	{
	public:
		Application();
		virtual ~Application() = default;

		void Run();
		void Close() { b_Running = false; }

		virtual void OnInit() {}
		virtual void OnUpdate() {}
		virtual void OnShutdown() {}
		
		// Override this to customize logger config
		virtual LoggerConfig GetLoggerConfig();
		
		// Override this to customize window config
		virtual WindowConfig GetWindowConfig();
		
		Window& GetWindow() { return *m_Window; }

	private:
		std::unique_ptr<Window> m_Window;
		bool b_Running;
	};

	// To be implemented by CLIENT/GAME
	Application* CreateApplication();
}