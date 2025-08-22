#pragma once

#include <memory>
#include <string>

#define FMT_UNICODE 0
#include <spdlog/spdlog.h>

namespace Afterglow
{
	class Log final
	{
	public:
		static void Init(const std::string& appLoggerName);

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	
	private:
		Log() = delete;
	
	private:
		inline static std::shared_ptr<spdlog::logger> s_CoreLogger;
		inline static std::shared_ptr<spdlog::logger> s_ClientLogger; 
	};
}

#ifdef AG_SHIPPING
	#define AG_TRACE(...)
	#define AG_INFO(...)
	#define AG_WARNING(...)
	#define AG_ERROR(...)
	
	#define AG_APP_LOG_TRACE(...)
	#define AG_APP_LOG_INFO(...)
	#define AG_APP_LOG_WARNING(...)
	#define AG_APP_LOG_ERROR(...)
#else
	#define AG_TRACE(...) ::Afterglow::Log::GetCoreLogger()->trace(__VA_ARGS__)
	#define AG_INFO(...) ::Afterglow::Log::GetCoreLogger()->info(__VA_ARGS__)
	#define AG_WARNING(...) ::Afterglow::Log::GetCoreLogger()->warn(__VA_ARGS__)
	#define AG_ERROR(...) ::Afterglow::Log::GetCoreLogger()->error(__VA_ARGS__)
	
	#define AG_APP_TRACE(...) ::Afterglow::Log::GetClientLogger()->trace(__VA_ARGS__)
	#define AG_APP_INFO(...) ::Afterglow::Log::GetClientLogger()->info(__VA_ARGS__)
	#define AG_APP_WARNING(...) ::Afterglow::Log::GetClientLogger()->warn(__VA_ARGS__)
	#define AG_APP_ERROR(...) ::Afterglow::Log::GetClientLogger()->error(__VA_ARGS__)
#endif
