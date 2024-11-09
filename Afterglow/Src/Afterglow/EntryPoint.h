#pragma once
#include "Log.h"

#ifdef AG_PLATFORMS_WINDOWS

extern Afterglow::Application* Afterglow::CreateApplication();

int main(int argc, char** argv)
{
	Afterglow::Log::Init();
	AG_INFO("Afterglow Engine");
	auto app = Afterglow::CreateApplication();
	app->Run();
	delete app;
}

#endif
