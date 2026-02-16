#pragma once

#include <string>
#include <fstream>
#include <mutex>

namespace Afterglow
{
	enum class LogLevel
	{
		Debug,
		Info,
		Warning,
		Error
	};

	struct LoggerConfig
	{
		std::string LogDirectory = "Logs";
		std::string ClientLoggerName = "CLIENT";
		std::string CoreLogFileName = "Afterglow-Core.log";
		std::string ClientLogFileName = "Afterglow-Client.log";
		bool EnableConsole = true;
		bool EnableFile = true;
		LogLevel MinConsoleLevel = LogLevel::Debug;
		LogLevel MinFileLevel = LogLevel::Debug;
	};

	class Logger
	{
	public:
		static void Init(const LoggerConfig& config = LoggerConfig());
		static void Shutdown();
		
		// NOTE(saeb): Internal logging functions (don't call directly, use macros)
		static void CoreLog(LogLevel verbosity, const char* file, int line, const char* message);
		static void ClientLog(LogLevel verbosity, const char* file, int line, const char* message);

	private:
		static void Log(bool isCore, LogLevel verbosity, const char* file, int line, const char* message);
		static const char* GetLogLevelString(LogLevel verbosity);
		static const char* GetLogLevelColor(LogLevel verbosity);

	private:
		static std::ofstream s_CoreFileStream;
		static std::ofstream s_ClientFileStream;
		static std::mutex s_Mutex;
		static LoggerConfig s_Config;
		static bool s_Initialized;
	};
}

// Core logging macros (for engine use)
#ifdef AG_DEBUG
	#define AG_CORE_DEBUG(msg)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Debug, __FILE__, __LINE__, msg)
	#define AG_CORE_INFO(msg)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Info, __FILE__, __LINE__, msg)
	#define AG_CORE_WARN(msg)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Warning, __FILE__, __LINE__, msg)
	#define AG_CORE_ERROR(msg)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Error, __FILE__, __LINE__, msg)
#else
	#define AG_CORE_DEBUG(msg)
	#define AG_CORE_INFO(msg)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Info, __FILE__, __LINE__, msg)
	#define AG_CORE_WARN(msg)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Warning, __FILE__, __LINE__, msg)
	#define AG_CORE_ERROR(msg)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Error, __FILE__, __LINE__, msg)
#endif

// Client logging macros (for client/game use)
#ifdef AG_DEBUG
	#define AG_LOG_DEBUG(msg)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Debug, __FILE__, __LINE__, msg)
	#define AG_LOG_INFO(msg)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Info, __FILE__, __LINE__, msg)
	#define AG_LOG_WARN(msg)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Warning, __FILE__, __LINE__, msg)
	#define AG_LOG_ERROR(msg)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Error, __FILE__, __LINE__, msg)
#else
	#define AG_LOG_DEBUG(msg)
	#define AG_LOG_INFO(msg)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Info, __FILE__, __LINE__, msg)
	#define AG_LOG_WARN(msg)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Warning, __FILE__, __LINE__, msg)
	#define AG_LOG_ERROR(msg)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Error, __FILE__, __LINE__, msg)
#endif