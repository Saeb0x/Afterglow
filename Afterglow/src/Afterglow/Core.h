#pragma once

#define AG_ASSERT(expression, message) \
    do { \
        if (!(expression)) { \
            fprintf(stderr, "Assertion failed: %s, function %s, file %s, line %d.\nMessage: %s\n", \
                    #expression, __func__, __FILE__, __LINE__, message); \
            abort(); \
        } \
    } while (0)

#define AG_ASSERT_STATIC(expression, msg) \
    static_assert(expression, msg)