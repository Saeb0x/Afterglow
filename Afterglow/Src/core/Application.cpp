#include "Application.h"

namespace Afterglow
{
	Application::Application()
		: b_Running(true)
	{
	}

	void Application::Run()
	{
		AG_CORE_INFO("Afterglow Engine starting...");

		OnInit();
		while (b_Running)
		{
			OnUpdate();
			Close();
		}
		OnShutdown();

		AG_CORE_INFO("Afterglow Engine shutting down...");
	}
}