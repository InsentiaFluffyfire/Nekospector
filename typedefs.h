#pragma once

#include <stdbool.h>
#include <stddef.h>

#define FLUFFY_EXPORT __declspec(dllexport)
#define nullptr ((void *)0)
#define null nullptr

#define VOID_FNC_POINTER(name) void name(void)
typedef	VOID_FNC_POINTER(voidFnc);


typedef signed char s8;
typedef short s16;
typedef int s32;
typedef long long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef unsigned long long memIndex;

typedef float r32;
typedef double r64;

typedef u32 b32;
typedef u64 b64;

