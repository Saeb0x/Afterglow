#pragma once

#include "Application.h"
#include "Log.h"

int main(int argc, char** argv)
{
	auto* app = Afterglow::CreateApplication();

	auto config = app->GetLoggerConfig();
	Afterglow::Logger::Init(config);

	app->Run();
	delete app;

	Afterglow::Logger::Shutdown();
	return 0;
}