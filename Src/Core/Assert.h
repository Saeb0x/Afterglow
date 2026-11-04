#if !defined(AG_ASSERT_H)
#define AG_ASSERT_H

#include "Config.h"

#if AG_COMPILER_MSVC
#define AG_DEBUGBREAK() __debugbreak()
#elif AG_COMPILER_CLANG
#define AG_DEBUGBREAK() __builtin_debugtrap()
#elif AG_COMPILER_GCC && (defined(__i386__) || defined(__x86_64__))
#define AG_DEBUGBREAK() __asm__ volatile("int3")
#elif AG_COMPILER_GCC
#define AG_DEBUGBREAK() __builtin_trap()
#endif

#if AG_DEBUG
#define AG_ASSERT(condition)                                                                       \
	do                                                                                             \
	{                                                                                              \
		if (!(condition))                                                                          \
		{                                                                                          \
			AG_DEBUGBREAK();                                                                       \
		}                                                                                          \
	} while (0)
#else
#define AG_ASSERT(condition) (void)sizeof(!(condition))
#endif

#endif