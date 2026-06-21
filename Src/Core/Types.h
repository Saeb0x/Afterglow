#if !defined(TYPES_H)

#include <stdint.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#define Kilobytes(value) ((value) * 1024ULL)
#define Megabytes(value) (Kilobytes(value) * 1024ULL)
#define Gigabytes(value) (Megabytes(value) * 1024ULL)
#define Terabytes(value) (Gigabytes(value) * 1024ULL)

#if defined(AG_DEBUG)
    #define Assert(expression) if(!(expression)) { *(int32*)0 = 0; }
#else
    #define Assert(expression)
#endif

#define TYPES_H
#endif
