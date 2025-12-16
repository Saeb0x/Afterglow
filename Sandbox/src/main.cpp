#include <iostream>

#include <Afterglow/Core/Logger.h>

int main(int ArgsCount, char** Args)
{
	Afterglow::Logger::GetInstance().SetMinVerbosity(Afterglow::LogVerbosity::Trace);
	Afterglow::Logger::GetInstance().SetLogFile("logs/Sandbox.log");

	AG_LOG_INFO("Afterglow Engine starting...");

	uint16_t width = 1920, height = 1080;
	AG_LOG_TRACE("Window size: {}x{}", width, height);
	
	AG_LOG_WARN("Using legacy OpenGL renderer");

	std::cin.get();
	return 0;
}