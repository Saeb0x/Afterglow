#include "Log.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace Afterglow
{
    std::ofstream Logger::s_CoreFileStream;
    std::ofstream Logger::s_ClientFileStream;
    std::mutex Logger::s_Mutex;
    LoggerConfig Logger::s_Config;
    bool Logger::s_Initialized = false;

    void Logger::Init(const LoggerConfig& config)
    {
        if (s_Initialized)
            return;

        s_Config = config;

        // Create logs directory if it doesn't exist
        if (s_Config.EnableFile && !std::filesystem::exists(s_Config.LogDirectory))
        {
            std::filesystem::create_directories(s_Config.LogDirectory);
        }

        auto now = std::chrono::system_clock::now();
        auto nowTimet = std::chrono::system_clock::to_time_t(now);
        std::tm localTm;

#ifdef _WIN32
        localtime_s(&localTm, &nowTimet);
#else
        localtime_r(&nowTimet, &localTm);
#endif

        // Filename timestamp format: YYYY-MM-DD_HH-MM-SS
        std::ostringstream timestampStr;
        timestampStr << std::put_time(&localTm, "%Y-%m-%d_%H-%M-%S");
        std::string timestamp = timestampStr.str();

        // Build configuration
#ifdef AG_DEBUG
        std::string buildConfig = "Debug";
#else
        std::string buildConfig = "Release";
#endif

        // Open core log file
        if (s_Config.EnableFile)
        {
            std::string coreBase = s_Config.CoreLogFileName;
            size_t dotPos = coreBase.find_last_of('.');
            std::string coreName = coreBase.substr(0, dotPos);
            std::string coreExt = (dotPos != std::string::npos) ? coreBase.substr(dotPos) : ".log";

            // Afterglow-Core_Debug_2025-02-17_20-30-45.log
            std::string coreLogPath = s_Config.LogDirectory + "/" + coreName + "_" + buildConfig + "_" + timestamp + coreExt;
            s_CoreFileStream.open(coreLogPath, std::ios::out | std::ios::trunc);
        }

        // Open client log file
        if (s_Config.EnableFile)
        {
            std::string clientBase = s_Config.ClientLogFileName;
            size_t dotPos = clientBase.find_last_of('.');
            std::string clientName = clientBase.substr(0, dotPos);
            std::string clientExt = (dotPos != std::string::npos) ? clientBase.substr(dotPos) : ".log";

            // Afterglow-Client_Release_2025-02-17_20-30-45.log
            std::string clientLogPath = s_Config.LogDirectory + "/" + clientName + "_" + buildConfig + "_" + timestamp + clientExt;
            s_ClientFileStream.open(clientLogPath, std::ios::out | std::ios::trunc);
        }

        s_Initialized = true;
    }

    void Logger::Shutdown()
    {
        if (!s_Initialized)
            return;

        if (s_CoreFileStream.is_open())
            s_CoreFileStream.close();

        if (s_ClientFileStream.is_open())
            s_ClientFileStream.close();

        s_Initialized = false;
    }

    void Logger::CoreLog(LogLevel verbosity, const char* file, int line, const char* message)
    {
        Log(true, verbosity, file, line, message);
    }

    void Logger::ClientLog(LogLevel verbosity, const char* file, int line, const char* message)
    {
        Log(false, verbosity, file, line, message);
    }

    void Logger::Log(bool isCore, LogLevel verbosity, const char* file, int line, const char* message)
    {
        if (!s_Initialized)
            return;

        std::lock_guard<std::mutex> lock(s_Mutex);

        auto now = std::chrono::system_clock::now();
        auto nowTimet = std::chrono::system_clock::to_time_t(now);
        std::tm localTm;

#ifdef _WIN32
        localtime_s(&localTm, &nowTimet);
#else
        localtime_r(&nowTimet, &localTm);
#endif

        // Timestamp (12-hour format with AM/PM)
        std::ostringstream timestamp;
        timestamp << std::put_time(&localTm, "%I:%M:%S %p");

        // Filename
        std::string filename = file;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos)
            filename = filename.substr(lastSlash + 1);

        // Thread ID
        std::ostringstream threadId;
        threadId << std::this_thread::get_id();

        // Log level string and color
        const char* levelStr = GetLogLevelString(verbosity);
        const char* levelColor = GetLogLevelColor(verbosity);
        const char* colorReset = "\033[0m";

        // Logger name
        std::string loggerName = isCore ? "CORE" : s_Config.ClientLoggerName;

        // Format: [Logger Name] [Thread ID: thread_id] [Timestamp] [Filename:line] (LOG LEVEL): Message
        std::ostringstream logMessage;
        logMessage << "[" << loggerName << "] " << "[Thread ID: " << threadId.str() << "] " << "[" << timestamp.str() << "] " << "[" << filename << ":" << line << "] " << "(" << levelStr << "): " << message;
        std::string logLine = logMessage.str();

        // Console output (with color for log level only)
        if (s_Config.EnableConsole && verbosity >= s_Config.MinConsoleLevel)
        {
            std::ostringstream coloredOutput;
            coloredOutput << "[" << loggerName << "] " << "[Thread ID: " << threadId.str() << "] " << "[" << timestamp.str() << "] " << "[" << filename << ":" << line << "] " << "(" << levelColor << levelStr << colorReset << "): " << message;
            std::cout << coloredOutput.str() << std::endl;
        }

        // File output (no colors)
        if (s_Config.EnableFile && verbosity >= s_Config.MinFileLevel)
        {
            std::ofstream& fileStream = isCore ? s_CoreFileStream : s_ClientFileStream;
            if (fileStream.is_open())
            {
                fileStream << logLine << std::endl;
                fileStream.flush();
            }
        }
    }

    const char* Logger::GetLogLevelString(LogLevel verbosity)
    {
        switch (verbosity)
        {
            case LogLevel::Debug:   return "DEBUG";
            case LogLevel::Info:    return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error:   return "ERROR";
            default:                return "UNKNOWN";
        }
    }

    const char* Logger::GetLogLevelColor(LogLevel verbosity)
    {
        switch (verbosity)
        {
            case LogLevel::Debug:   return "\033[96m";      // Bright Cyan
            case LogLevel::Info:    return "\033[92m";      // Bright Green
            case LogLevel::Warning: return "\033[93m";      // Bright Yellow
            case LogLevel::Error:   return "\033[91m";      // Bright Red
            default:                return "\033[97m";      // Bright White
        }
    }
}