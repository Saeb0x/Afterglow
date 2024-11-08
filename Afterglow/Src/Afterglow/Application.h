#pragma once

namespace Afterglow
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined by CLIENT APP
	Application* CreateApplication();
}

