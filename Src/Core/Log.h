#if !defined(AG_LOG_H)
#define AG_LOG_H

#include "Types.h"

enum class LogVerbosity : uint8
{
	Info = 0,
	Warning,
	Error
};

typedef void (*LogCallback)(LogVerbosity verbosity, const char* text, usize length);

void LogSetCallback(LogCallback callback);
void Log(LogVerbosity verbosity, const char* format, ...);

#if AG_DEBUG
#define AG_LOG_INFO(...) Log(LogVerbosity::Info, __VA_ARGS__)
#else
#define AG_LOG_INFO(...) (void)sizeof((Log(LogVerbosity::Info, __VA_ARGS__), 0))
#endif

#define AG_LOG_WARNING(...) Log(LogVerbosity::Warning, __VA_ARGS__)
#define AG_LOG_ERROR(...) Log(LogVerbosity::Error, __VA_ARGS__)

#endif