#if !defined(AG_LOG_H)
#define AG_LOG_H

#include "Types.h"

enum LogVerbosity
{
	LOGVERBOSITY_DEBUG = 0,
	LOGVERBOSITY_INFO,
	LOGVERBOSITY_WARNING,
	LOGVERBOSITY_ERROR
};

typedef void (*LogOutputCallback)(LogVerbosity verbosity, const char* text, usize length);

void LogSetOutputCallback(LogOutputCallback callback);
void Log(LogVerbosity verbosity, const char* format, ...);

#define AG_LOG_DEBUG(...) Log(LOGVERBOSITY_DEBUG, __VA_ARGS__)
#define AG_LOG_INFO(...) Log(LOGVERBOSITY_INFO, __VA_ARGS__)
#define AG_LOG_WARNING(...) Log(LOGVERBOSITY_WARNING, __VA_ARGS__)
#define AG_LOG_ERROR(...) Log(LOGVERBOSITY_ERROR, __VA_ARGS__)

#endif