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
		static void Init(const std::string& clientLoggerName);

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
	#define AG_LOG_TRACE(...)
	#define AG_LOG_INFO(...)
	#define AG_LOG_WARNING(...)
	#define AG_LOG_ERROR(...)
	
	#define AG_CLIENT_LOG_TRACE(...)
	#define AG_CLIENT_LOG_INFO(...)
	#define AG_CLIENT_LOG_WARNING(...)
	#define AG_CLIENT_LOG_ERROR(...)
#else
	#define AG_LOG_TRACE(...) ::Afterglow::Log::GetCoreLogger()->trace(__VA_ARGS__)
	#define AG_LOG_INFO(...) ::Afterglow::Log::GetCoreLogger()->info(__VA_ARGS__)
	#define AG_LOG_WARNING(...) ::Afterglow::Log::GetCoreLogger()->warn(__VA_ARGS__)
	#define AG_LOG_ERROR(...) ::Afterglow::Log::GetCoreLogger()->error(__VA_ARGS__)
	
	#define AG_CLIENT_LOG_TRACE(...) ::Afterglow::Log::GetClientLogger()->trace(__VA_ARGS__)
	#define AG_CLIENT_LOG_INFO(...) ::Afterglow::Log::GetClientLogger()->info(__VA_ARGS__)
	#define AG_CLIENT_LOG_WARNING(...) ::Afterglow::Log::GetClientLogger()->warn(__VA_ARGS__)
	#define AG_CLIENT_LOG_ERROR(...) ::Afterglow::Log::GetClientLogger()->error(__VA_ARGS__)
#endif
