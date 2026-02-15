#include "Application.h"

namespace Afterglow
{
	void Application::Run()
	{
		OnInit();

		b_Running = true;
		while (b_Running)
		{
			OnUpdate();
		}

		OnShutdown();
	}
}