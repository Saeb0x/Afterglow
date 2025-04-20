#include "agpch.h"
#include "Application.h"

#include "Afterglow/Events/WindowEvents.h"
#include "Log.h"

namespace Afterglow
{
	void Application::Run()
	{
		WindowCloseEvent e;

		AG_LOG_WARNING(e.ToString());

		while (true);
	}
}