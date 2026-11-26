#if !defined(AG_CONFIG_H)
#define AG_CONFIG_H

#define AG_PLATFORM_WINDOWS 0
#if defined(_WIN32)
#undef AG_PLATFORM_WINDOWS
#define AG_PLATFORM_WINDOWS 1
#else
#error "Afterglow: Unsupported platform."
#endif

#define AG_COMPILER_MSVC 0
#if defined(_MSC_VER)
#undef AG_COMPILER_MSVC
#define AG_COMPILER_MSVC 1
#else
#error "Afterglow: Unsupported compiler for assertion debug breaks."
#endif

#if !defined(AG_DEBUG) || (AG_DEBUG != 0 && AG_DEBUG != 1)
#error "Afterglow: AG_DEBUG must be defined by the build (0 or 1)."
#endif

#if AG_DEBUG == 1
#define AG_RELEASE 0
#else
#define AG_RELEASE 1
#endif

#endif