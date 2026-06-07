#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

typedef signed char i8;
typedef short i16;
typedef int i32;
typedef __int64 i64;	/* MSVC extension */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned __int64 u64;	/* MSVC extension */

typedef float f32;
typedef double f64;

typedef int bool;
#define TRUE 1
#define FALSE 0

typedef size_t uptr;
typedef ptrdiff_t iptr;

/* Compile-time size validation. */
typedef char check_i8[sizeof(i8) == 1 ? 1 : -1];
typedef char check_i16[sizeof(i16) == 2 ? 1 : -1];
typedef char check_i32[sizeof(i32) == 4 ? 1 : -1];
typedef char check_i64[sizeof(i64) == 8 ? 1 : -1];
typedef char check_u8[sizeof(u8) == 1 ? 1 : -1];
typedef char check_u16[sizeof(u16) == 2 ? 1 : -1];
typedef char check_u32[sizeof(u32) == 4 ? 1 : -1];
typedef char check_u64[sizeof(u64) == 8 ? 1 : -1];
typedef char check_f32[sizeof(f32) == 4 ? 1 : -1];
typedef char check_f64[sizeof(f64) == 8 ? 1 : -1];
typedef char check_ptr[sizeof(uptr) == 8 ? 1 : -1];

#endif