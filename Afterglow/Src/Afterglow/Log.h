#pragma once

#include <spdlog/spdlog.h>
#include <memory>

namespace Afterglow
{
	class Log
	{
	public:
		static void Init();

		static inline std::shared_ptr<spdlog::logger> GetEngineLogger() { return m_EngineLogger; }
		static inline std::shared_ptr<spdlog::logger> GetAppLogger() { return m_AppLogger; }
	private:
		static std::shared_ptr<spdlog::logger> m_EngineLogger;
		static std::shared_ptr<spdlog::logger> m_AppLogger;
	};
}

// Engine Logs
#define AG_TRACE(...) Afterglow::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define AG_DEBUG(...) Afterglow::Log::GetEngineLogger()->debug(__VA_ARGS__)
#define AG_INFO(...) Afterglow::Log::GetEngineLogger()->info(__VA_ARGS__)
#define AG_WARN(...) Afterglow::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define AG_ERROR(...) Afterglow::Log::GetEngineLogger()->error(__VA_ARGS__)
#define AG_CRITICAL(...) Afterglow::Log::GetEngineLogger()->critical(__VA_ARGS__)

// App Logs
#define AG_APP_TRACE(...) Afterglow::Log::GetAppLogger()->trace(__VA_ARGS__)
#define AG_APP_DEBUG(...) Afterglow::Log::GetAppLogger()->debug(__VA_ARGS__)
#define AG_APP_INFO(...) Afterglow::Log::GetAppLogger()->info(__VA_ARGS__)
#define AG_APP_WARN(...) Afterglow::Log::GetAppLogger()->warn(__VA_ARGS__)
#define AG_APP_ERROR(...) Afterglow::Log::GetAppLogger()->error(__VA_ARGS__)
#define AG_APP_CRITICAL(...) Afterglow::Log::GetAppLogger()->critical(__VA_ARGS__)