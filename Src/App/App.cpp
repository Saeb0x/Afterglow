#include "App.h"
#include "Core/Log.h"

WindowConfig AppGetWindowConfig()
{
	WindowConfig config = {};
	config.Title = "Afterglow";
	config.Width = 1280;
	config.Height = 720;

	return (config);
}

void AppMain(void* data)
{
	AG_LOG_DEBUG("App: Running on its own thread.");
}