#pragma once

#include "Log.h"

#if AG_ENABLE_ASSERTIONS
    #define AG_ASSERT(expression, message) \
        do { \
            if (!(expression)) { \
                AG_LOG_ERROR("Assertion failed: {0} Check ({1}), file {2}, line {3}.", message, #expression, __FILE__, __LINE__); \
                __debugbreak(); \
                abort(); \
            } \
        } while (0)

    #define AG_ASSERT_STATIC(expression, message) \
        static_assert(expression, message)
#else
    #define AG_ASSERT(expression, message) 

    #define AG_ASSERT_STATIC(expression, message) 
#endif