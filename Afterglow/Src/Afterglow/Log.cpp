#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Afterglow
{
	std::shared_ptr<spdlog::logger> Log::m_EngineLogger;
	std::shared_ptr<spdlog::logger> Log::m_AppLogger;

	void Log::Init()
	{
		m_EngineLogger = spdlog::stdout_color_mt("ENGINE");
		m_AppLogger = spdlog::stdout_color_mt("APP");

		spdlog::set_pattern("%^[%r] %n (%l): %v%$");
		m_EngineLogger->set_level(spdlog::level::trace);
		m_AppLogger->set_level(spdlog::level::trace);
	}
}
