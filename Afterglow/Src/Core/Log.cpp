#include "Log.h"

#include <iostream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace Afterglow
{
	std::ofstream Logger::s_FileStream;
	std::mutex Logger::s_Mutex;
	LoggerConfig Logger::s_Config;
	bool Logger::s_Initialized = false;

	void Logger::Init(const LoggerConfig& config)
	{
		if (s_Initialized)
			return;

		s_Config = config;

		if (s_Config.EnableFile)
		{
			std::filesystem::create_directories(s_Config.LogDirectory);
			std::string logPath = s_Config.LogDirectory + "/" + s_Config.LogFileName;
			s_FileStream.open(logPath, std::ios::out | std::ios::trunc);
		}

		s_Initialized = true;
	}

	void Logger::Shutdown()
	{
		if (!s_Initialized)
			return;

		if (s_FileStream.is_open())
			s_FileStream.close();

		s_Initialized = false;
	}

	void Logger::CoreLog(LogLevel level, const char* file, int line, const char* message)
	{
		Log(true, level, file, line, message);
	}

	void Logger::ClientLog(LogLevel level, const char* file, int line, const char* message)
	{
		Log(false, level, file, line, message);
	}

	void Logger::Log(bool isCore, LogLevel level, const char* file, int line, const char* message)
	{
		if (!s_Initialized || level < s_Config.MinLevel)
			return;

		std::lock_guard<std::mutex> lock(s_Mutex);

		// Timestamp
		auto now = std::chrono::system_clock::now();
		auto nowTimet = std::chrono::system_clock::to_time_t(now);
		std::tm localTm;
#ifdef _WIN32
		localtime_s(&localTm, &nowTimet);
#else
		localtime_r(&nowTimet, &localTm);
#endif
		std::ostringstream ts;
		ts << std::put_time(&localTm, "%I:%M:%S %p");

		// Filename
		std::string filename = file;
		size_t lastSlash = filename.find_last_of("/\\");
		if (lastSlash != std::string::npos)
			filename = filename.substr(lastSlash + 1);

		const char* levelStr = GetLogLevelString(level);
		std::string loggerName = isCore ? "CORE" : s_Config.ClientLoggerName;

		// Build the common header once: everything up to and including the opening paren of (LEVEL)
		std::string header = "[" + loggerName + "] [" + ts.str() + "] [" + filename + ":" + std::to_string(line) + "] (";
		std::string tail = std::string("): ") + message;

		if (s_Config.EnableConsole)
		{
			const char* color = GetLogLevelColor(level);
			std::cout << header << color << levelStr << "\033[0m" << tail << "\n";
		}

		if (s_Config.EnableFile && s_FileStream.is_open())
		{
			s_FileStream << header << levelStr << tail << "\n";
			s_FileStream.flush();
		}
	}

	const char* Logger::GetLogLevelString(LogLevel level)
	{
		switch (level)
		{
			case LogLevel::Debug:
				return "DEBUG";
			case LogLevel::Info:
				return "INFO";
			case LogLevel::Warning:
				return "WARN";
			case LogLevel::Error:
				return "ERROR";
			default:
				return "UNKNOWN";
		}
	}

	const char* Logger::GetLogLevelColor(LogLevel level)
	{
		switch (level)
		{
			case LogLevel::Debug:
				return "\033[96m";	// Bright Cyan
			case LogLevel::Info:
				return "\033[92m";	// Bright Green
			case LogLevel::Warning:
				return "\033[93m";	// Bright Yellow
			case LogLevel::Error:
				return "\033[91m";	// Bright Red
			default:
				return "\033[97m";	// Bright White
		}
	}
}