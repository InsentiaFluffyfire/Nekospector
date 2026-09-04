#pragma once

#include "typedefs.h"

#if DEBUG
#define Assert(cond) while (!(cond)) __debugbreak()
#else
#define Assert(cond) __assume(cond) 
#endif

#define ArrayCount(Array) (sizeof(Array)/sizeof(Array[0]))
#define KiloByte(x) (u64)((x)*1024)
#define MegaByte(x) (u64)(KiloByte(x)*1024)
#define GigaByte(x) (u64)(MegaByte(x)*1024)