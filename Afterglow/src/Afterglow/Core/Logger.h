#pragma once

#include <string>
#include <cstdint>
#include <fstream>
#include <mutex>

#include <fmt/format.h>
#include <fmt/color.h>

namespace Afterglow
{
    enum class LogVerbosity : uint8_t
    {
        Trace = 0,
        Info,
        Warning,
        Error
    };

    class Logger
    {
    public:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        static Logger& GetInstance();

        template<typename... Args>
        void Log(LogVerbosity verbosity, const char* file, uint32_t line, fmt::format_string<Args...> fmtStr, Args&&... args)
        {
            if (verbosity < m_MinVerbosity) return;

            std::string logMessage = fmt::format(fmtStr, std::forward<Args>(args)...);
            std::string logMessageFormatted = FormatLogMessage(verbosity, file, line, logMessage);

            std::lock_guard<std::mutex> lock(m_Mutex);

            PrintToConsole(verbosity, logMessageFormatted);

            if (m_OutputFileStream.is_open()) 
            {
                m_OutputFileStream << logMessageFormatted << std::endl;

                if (verbosity == LogVerbosity::Error)
                    m_OutputFileStream.flush();
            }
        }

        void SetMinVerbosity(LogVerbosity verbosity) { m_MinVerbosity = verbosity; }
        void SetLogFile(const std::string& filepath);

    private:
        Logger() = default;
        ~Logger();

        std::string FormatLogMessage(LogVerbosity verbosity, const char* file, uint32_t line, const std::string& logMessage);
        void PrintToConsole(LogVerbosity verbosity, const std::string& logMessageFormatted);
        const char* GetVerbosityString(LogVerbosity verbosity);
        fmt::color GetVerbosityColor(LogVerbosity verbosity);

    private:
        LogVerbosity m_MinVerbosity = LogVerbosity::Trace;
        std::ofstream m_OutputFileStream;
        std::mutex m_Mutex;
    };
}

#if defined(AG_DEBUG)
    #define AG_LOG_TRACE(...)   ::Afterglow::Logger::GetInstance().Log(::Afterglow::LogVerbosity::Trace, __FILE__, __LINE__, __VA_ARGS__)
    #define AG_LOG_INFO(...)    ::Afterglow::Logger::GetInstance().Log(::Afterglow::LogVerbosity::Info, __FILE__, __LINE__, __VA_ARGS__)
    #define AG_LOG_WARN(...)    ::Afterglow::Logger::GetInstance().Log(::Afterglow::LogVerbosity::Warning, __FILE__, __LINE__, __VA_ARGS__)
    #define AG_LOG_ERROR(...)   ::Afterglow::Logger::GetInstance().Log(::Afterglow::LogVerbosity::Error, __FILE__, __LINE__, __VA_ARGS__)
#elif defined(AG_RELEASE)
    #define AG_LOG_TRACE(...)   (void)0
    #define AG_LOG_INFO(...)    (void)0
    #define AG_LOG_WARN(...)    ::Afterglow::Logger::GetInstance().Log(::Afterglow::LogVerbosity::Warning, __FILE__, __LINE__, __VA_ARGS__)
    #define AG_LOG_ERROR(...)   ::Afterglow::Logger::GetInstance().Log(::Afterglow::LogVerbosity::Error, __FILE__, __LINE__, __VA_ARGS__)
#endif