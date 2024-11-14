#pragma once

#include "Events/WindowEvents.h"
#include "Events/MouseEvents.h"
#include "Events/KeyEvents.h"
#include "Window.h"

namespace Afterglow
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);
	private:
		bool OnMouseMove(MouseMoveEvent& event);
		bool OnWindowsClose(WindowCloseEvent& event);
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running;
	};

	Application* CreateApplication();
}

#ifdef AG_PLATFORMS_WINDOWS
	#define AFTERGLOW_ENTRY(appClass)							 \
	    Afterglow::Application* Afterglow::CreateApplication() { \
	        return new appClass;                                 \
	    }                                                        \
	    int main(int argc, char** argv) {                        \
	        Afterglow::Log::Init();                              \
	        AG_INFO("Initialized Log!");						 \
	        auto app = Afterglow::CreateApplication();           \
	        app->Run();                                          \
	        delete app;                                          \
	        return 0;                                            \
	    }
#else
#error Afterglow supports Windows only (for now).
#endif
