#include "Log.h"

#include <stdarg.h>
#include <stdio.h>

#define AG_LOG_MAX_CHARS_PER_LINE 1024

static LogOutputCallback OutputCallback;

static const char* LogVerbosityName(LogVerbosity verbosity)
{
	switch(verbosity)
	{
		case LOGVERBOSITY_DEBUG:
		{
			return ("Debug");
		}

		case LOGVERBOSITY_INFO:
		{
			return ("Info");
		}

		case LOGVERBOSITY_WARNING:
		{
			return ("Warning");
		}

		case LOGVERBOSITY_ERROR:
		{
			return ("Error");
		}
	}

	return ("Unknown");
}

void LogSetOutputCallback(LogOutputCallback callback)
{
	OutputCallback = callback;
}

void Log(LogVerbosity verbosity, const char* format, ...)
{
	if(!OutputCallback)
	{
		return;
	}

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

	OutputCallback(verbosity, line, length);
}