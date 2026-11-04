#if !defined(AG_CONFIG_H)
#define AG_CONFIG_H

#define AG_PLATFORM_WINDOWS 0
#define AG_PLATFORM_LINUX 0
#define AG_PLATFORM_MACOS 0

#if defined(_WIN32)
#undef AG_PLATFORM_WINDOWS
#define AG_PLATFORM_WINDOWS 1
#elif defined(__linux__)
#undef AG_PLATFORM_LINUX
#define AG_PLATFORM_LINUX 1
#elif defined(__APPLE__) && defined(__MACH__)
#undef AG_PLATFORM_MACOS
#define AG_PLATFORM_MACOS 1
#else
#error "Afterglow: Unsupported platform."
#endif

#define AG_COMPILER_MSVC 0
#define AG_COMPILER_CLANG 0
#define AG_COMPILER_GCC 0

#if defined(_MSC_VER)
#undef AG_COMPILER_MSVC
#define AG_COMPILER_MSVC 1
#elif defined(__clang__)
#undef AG_COMPILER_CLANG
#define AG_COMPILER_CLANG 1
#elif defined(__GNUC__)
#undef AG_COMPILER_GCC
#define AG_COMPILER_GCC 1
#else
#error "Afterglow: Unsupported compiler."
#endif

#if !defined(AG_DEBUG) || (AG_DEBUG != 0 && AG_DEBUG != 1)
#error "Afterglow: AG_DEBUG must be defined by the build (0 or 1)."
#endif

#if AG_DEBUG
#define AG_RELEASE 0
#else
#define AG_RELEASE 1
#endif

#endif