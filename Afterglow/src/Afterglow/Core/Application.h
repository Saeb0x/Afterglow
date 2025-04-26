#pragma once

#include "Window.h"

namespace Afterglow
{
	class Application
	{
	public:
		Application();
		virtual ~Application() = default;

		virtual void Run();
		
	private:
		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
	};
}
