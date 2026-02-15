#pragma once

namespace Afterglow
{
	class Application
	{
	public:
		Application() = default;
		virtual ~Application() = default;

		void Run();

		virtual void OnInit() {}
		virtual void OnUpdate() {}
		virtual void OnShutdown() {}

	private:
		bool b_Running;
	};

	// To be implemented by CLIENT/GAME
	Application* CreateApplication();
}