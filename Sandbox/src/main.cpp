#include <iostream>

#include <Afterglow/Core/Logger.h>

int main(int ArgsCount, char** Args)
{
	Afterglow::Logger::GetInstance().SetMinVerbosity(Afterglow::LogVerbosity::Trace);
	Afterglow::Logger::GetInstance().SetLogFile("logs/Sandbox.log");

	Afterglow::Logger::GetInstance().EnableCategory(Afterglow::LogCategory::Engine);
	Afterglow::Logger::GetInstance().EnableCategory(Afterglow::LogCategory::Game);

	AG_LOG_INFO(Afterglow::LogCategory::Engine, "Afterglow Engine starting...");

	uint16_t width = 1920, height = 1080;
	AG_GAME_TRACE("Window size: {}x{}", width, height);
	
	AG_GAME_WARN("Using legacy OpenGL renderer");
	
	AG_ENGINE_ERROR("This is an error!");

	std::cin.get();
	return 0;
}