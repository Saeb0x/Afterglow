#include "agpch.h"
#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Afterglow
{
	void Log::Init(const std::string& clientLoggerName)
	{
		spdlog::set_pattern("%r [%n]: %^[%l]: %v%$");

		s_CoreLogger = spdlog::stdout_color_mt("Afterglow");
		s_ClientLogger = spdlog::stdout_color_mt(clientLoggerName);

		spdlog::set_level(spdlog::level::trace);
	}
}