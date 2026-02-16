#pragma once

#include "Log.h"

namespace Afterglow
{
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

		virtual LoggerConfig GetLoggerConfig() { return LoggerConfig(); }

	private:
		bool b_Running;
	};

	// To be implemented by CLIENT/GAME
	Application* CreateApplication();
}