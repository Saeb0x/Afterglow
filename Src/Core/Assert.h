#if !defined(AG_ASSERT_H)
#define AG_ASSERT_H

#include "Config.h"

#if AG_COMPILER_MSVC
#define AG_DEBUGBREAK() __debugbreak()
#endif

#if AG_DEBUG
#define AG_ASSERT(condition)                                                                       \
	do                                                                                             \
	{                                                                                              \
		if(!(condition))                                                                           \
		{                                                                                          \
			AG_DEBUGBREAK();                                                                       \
		}                                                                                          \
	} while(0)
#else
#define AG_ASSERT(condition) (void)sizeof(!(condition))
#endif

#endif