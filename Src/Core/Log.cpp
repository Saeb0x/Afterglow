#include "Log.h"

#include <cstdarg>
#include <cstdio>

#define AG_LOG_MAX_CHARS_PER_LINE 1024

static LogCallback LogOutput;

static const char* LogVerbosityName(LogVerbosity verbosity)
{
	switch(verbosity)
	{
		case LogVerbosity::Info:
		{
			return ("Info");
		}
		case LogVerbosity::Warning:
		{
			return ("Warning");
		}
		case LogVerbosity::Error:
		{
			return ("Error");
		}
	}

	return ("Unknown");
}

void LogSetCallback(LogCallback callback)
{
	LogOutput = callback;
}

void Log(LogVerbosity verbosity, const char* format, ...)
{
	if(!LogOutput)
	{
		return;
	}

	// NOTE(saeb): On the stack, so logging never allocates and threads can't overwrite each other.
	char line[AG_LOG_MAX_CHARS_PER_LINE];
	usize capacity = sizeof(line) - 1;

	int prefix = snprintf(line, capacity, "[%s] ", LogVerbosityName(verbosity));
	usize length = (prefix > 0) ? (usize)prefix : 0;

	va_list arguments;
	va_start(arguments, format);
	int message = vsnprintf(line + length, capacity - length, format, arguments);
	va_end(arguments);

	if(message > 0)
	{
		length += ((usize)message < capacity - length) ? (usize)message : capacity - length - 1;
	}

	line[length++] = '\n';
	line[length] = '\0';

	LogOutput(verbosity, line, length);
}