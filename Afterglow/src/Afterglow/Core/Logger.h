#pragma once

#include <string>
#include <cstdint>
#include <fstream>
#include <mutex>
#include <unordered_set>

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

    namespace LogCategory
    {
        constexpr const char* Engine = "ENGINE";
        constexpr const char* Game = "GAME";

        constexpr const char* Renderer = "RENDERER";
        constexpr const char* Physics = "PHYSICS";
        constexpr const char* Audio = "AUDIO";
        constexpr const char* Asset = "ASSET";
        constexpr const char* Network = "NETWORK";
        constexpr const char* UI = "UI";
    }

    class Logger
    {
    public:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        static Logger& GetInstance();

        template<typename... Args>
        void Log(const char* category, LogVerbosity verbosity, const char* file, uint32_t line, fmt::format_string<Args...> fmtStr, Args&&... args)
        {
            if (verbosity < m_MinVerbosity) return;

            if (!IsCategoryEnabled(category)) return;

            std::string logMessage = fmt::format(fmtStr, std::forward<Args>(args)...);
            std::string logMessageFormatted = FormatLogMessage(category, verbosity, file, line, logMessage);

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

        // Category management.
        void EnableAllCategories();
        void DisableAllCategories();
        void EnableCategory(const char* category);
        void DisableCategory(const char* category);
        bool IsCategoryEnabled(const char* category) const;

    private:
        Logger() = default;
        ~Logger();

        std::string FormatLogMessage(const char* category, LogVerbosity verbosity, const char* file, uint32_t line, const std::string& logMessage);
        void PrintToConsole(LogVerbosity verbosity, const std::string& logMessageFormatted);
        const char* GetVerbosityString(LogVerbosity verbosity);
        fmt::color GetVerbosityColor(LogVerbosity verbosity);

    private:
        LogVerbosity m_MinVerbosity = LogVerbosity::Trace;
        std::unordered_set<std::string> m_EnabledCategories;
        bool b_AllCategoriesEnabled = true;

        std::ofstream m_OutputFileStream;
        std::mutex m_Mutex;
    };
}

#if defined(AG_DEBUG)
    #define AG_LOG_TRACE(category, ...)     ::Afterglow::Logger::GetInstance().Log(category, ::Afterglow::LogVerbosity::Trace, __FILE__, __LINE__, __VA_ARGS__)
    #define AG_LOG_INFO(category, ...)      ::Afterglow::Logger::GetInstance().Log(category, ::Afterglow::LogVerbosity::Info, __FILE__, __LINE__, __VA_ARGS__)
    #define AG_LOG_WARN(category, ...)      ::Afterglow::Logger::GetInstance().Log(category, ::Afterglow::LogVerbosity::Warning, __FILE__, __LINE__, __VA_ARGS__)
    #define AG_LOG_ERROR(category, ...)     ::Afterglow::Logger::GetInstance().Log(category, ::Afterglow::LogVerbosity::Error, __FILE__, __LINE__, __VA_ARGS__)
#elif defined(AG_RELEASE)
    #define AG_LOG_TRACE(category, ...)     (void)0
    #define AG_LOG_INFO(category, ...)      (void)0
    #define AG_LOG_WARN(category, ...)      ::Afterglow::Logger::GetInstance().Log(category, ::Afterglow::LogVerbosity::Warning, __FILE__, __LINE__, __VA_ARGS__)
    #define AG_LOG_ERROR(category, ...)     ::Afterglow::Logger::GetInstance().Log(category, ::Afterglow::LogVerbosity::Error, __FILE__, __LINE__, __VA_ARGS__)
#endif

#define AG_ENGINE_TRACE(...)    AG_LOG_TRACE(::Afterglow::LogCategory::Engine, __VA_ARGS__)
#define AG_ENGINE_INFO(...)     AG_LOG_INFO(::Afterglow::LogCategory::Engine, __VA_ARGS__)
#define AG_ENGINE_WARN(...)     AG_LOG_WARN(::Afterglow::LogCategory::Engine, __VA_ARGS__)
#define AG_ENGINE_ERROR(...)    AG_LOG_ERROR(::Afterglow::LogCategory::Engine, __VA_ARGS__)

#define AG_GAME_TRACE(...)      AG_LOG_TRACE(::Afterglow::LogCategory::Game, __VA_ARGS__)
#define AG_GAME_INFO(...)       AG_LOG_INFO(::Afterglow::LogCategory::Game, __VA_ARGS__)
#define AG_GAME_WARN(...)       AG_LOG_WARN(::Afterglow::LogCategory::Game, __VA_ARGS__)
#define AG_GAME_ERROR(...)      AG_LOG_ERROR(::Afterglow::LogCategory::Game, __VA_ARGS__)