#if !defined(AG_LOG_H)
#define AG_LOG_H

#include "Types.h"

enum class LogVerbosity : uint8
{
	Debug = 0,
	Info,
	Warning,
	Error
};

typedef void (*LogCallback)(LogVerbosity verbosity, const char* text, usize length);

void LogSetCallback(LogCallback callback);
void Log(LogVerbosity verbosity, const char* format, ...);

#define AG_LOG_DEBUG(...) Log(LogVerbosity::Debug, __VA_ARGS__)
#define AG_LOG_INFO(...) Log(LogVerbosity::Info, __VA_ARGS__)
#define AG_LOG_WARNING(...) Log(LogVerbosity::Warning, __VA_ARGS__)
#define AG_LOG_ERROR(...) Log(LogVerbosity::Error, __VA_ARGS__)

#endif