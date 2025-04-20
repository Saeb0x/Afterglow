#include "agpch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Afterglow
{
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		spdlog::set_pattern("%^%r [%n]: %l: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("AFTERGLOW");
		s_ClientLogger = spdlog::stdout_color_mt("APP");

		spdlog::set_level(spdlog::level::info);
	}
}