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

	Application* CreateApplication();
}

#ifdef AG_PLATFORMS_WINDOWS
	#define AFTERGLOW_ENTRY(appClass)							 \
	    Afterglow::Application* Afterglow::CreateApplication() { \
	        return new appClass;                                 \
	    }                                                        \
	    int main(int argc, char** argv) {                        \
	        Afterglow::Log::Init();                              \
	        AG_INFO("Afterglow Engine Initialized!");            \
	        auto app = Afterglow::CreateApplication();           \
	        app->Run();                                          \
	        delete app;                                          \
	        return 0;                                            \
	    }
#else
#error Afterglow supports Windows only (for now).
#endif
