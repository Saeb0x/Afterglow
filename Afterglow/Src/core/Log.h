#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <sstream>

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

		// Template helper for formatting (no arguments)
		static std::string Format(const char* format);

		// Template helper for formatting (with arguments)
		template<typename... Args>
		static std::string Format(const char* format, Args&&... args);

	private:
		static void Log(bool isCore, LogLevel verbosity, const char* file, int line, const char* message);
		static const char* GetLogLevelString(LogLevel verbosity);
		static const char* GetLogLevelColor(LogLevel verbosity);

		// Helper to convert single argument to string
		template<typename T>
		static std::string ToString(T&& value);

	private:
		static std::ofstream s_CoreFileStream;
		static std::ofstream s_ClientFileStream;
		static std::mutex s_Mutex;
		static LoggerConfig s_Config;
		static bool s_Initialized;
	};

	inline std::string Logger::Format(const char* format)
	{
		return std::string(format);
	}

	template<typename T>
	inline std::string Logger::ToString(T&& value)
	{
		std::ostringstream oss;
		oss << std::forward<T>(value);
		return oss.str();
	}

	template<typename... Args>
	inline std::string Logger::Format(const char* format, Args&&... args)
	{
		std::ostringstream result;
		const char* current = format;

		// Convert all arguments to strings upfront
		std::string argStrings[] = { ToString(std::forward<Args>(args))... };
		size_t argIndex = 0;
		size_t argCount = sizeof...(args);

		while (*current)
		{
			if (*current == '{' && *(current + 1) == '}')
			{
				// Found {} placeholder
				if (argIndex < argCount)
				{
					result << argStrings[argIndex++];
				}
				current += 2;	// Skip {}
			}
			else
			{
				result << *current;
				++current;
			}
		}

		return result.str();
	}
}

// Core logging macros (for engine use)
#ifdef AG_DEBUG
	#define AG_CORE_DEBUG(msg, ...)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Debug, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
	#define AG_CORE_INFO(msg, ...)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Info, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
	#define AG_CORE_WARN(msg, ...)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Warning, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
	#define AG_CORE_ERROR(msg, ...)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Error, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
#else
	#define AG_CORE_DEBUG(msg, ...)
	#define AG_CORE_INFO(msg, ...)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Info, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
	#define AG_CORE_WARN(msg, ...)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Warning, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
	#define AG_CORE_ERROR(msg, ...)		::Afterglow::Logger::CoreLog(::Afterglow::LogLevel::Error, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
#endif

// Client logging macros (for client/game use)
#ifdef AG_DEBUG
	#define AG_LOG_DEBUG(msg, ...)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Debug, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
	#define AG_LOG_INFO(msg, ...)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Info, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
	#define AG_LOG_WARN(msg, ...)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Warning, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
	#define AG_LOG_ERROR(msg, ...)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Error, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
#else
	#define AG_LOG_DEBUG(msg, ...)
	#define AG_LOG_INFO(msg, ...)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Info, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
	#define AG_LOG_WARN(msg, ...)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Warning, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
	#define AG_LOG_ERROR(msg, ...)		::Afterglow::Logger::ClientLog(::Afterglow::LogLevel::Error, __FILE__, __LINE__, ::Afterglow::Logger::Format(msg, ##__VA_ARGS__).c_str())
#endif