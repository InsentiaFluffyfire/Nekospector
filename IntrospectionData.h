#pragma once

#include "typedefs.h"
#include "utility.h"
#include "fstring.h"
#include "platformMemory.h"

#include "IntroDataType.h"

typedef enum MemberFlagBits {
	 MEMBER_FLAG_BIT_NONE    = 0x0,
	 MEMBER_FLAG_BIT_PTR     = 0x1,
	 MEMBER_FLAG_BIT_ENUM    = 0x2,
	 MEMBER_FLAG_BIT_TUPPLE  = 0x4,
} MemberFlagBits;

typedef struct MemberDefinition {
	 IntroDataType  type      ;
	 u32            count     ;
	 const char*    identifier;
	 u64            offset    ;
	 MemberFlagBits flags     ;
} MemberDefinition;

typedef struct EnumeratorDefinition {
	 const char*  identifier;
	 s32          value     ;
} EnumeratorDefinition;

/*===========================================================================================================================================================================
|
|       x64\Debug\testFile.h - Introspection Data
|
===========================================================================================================================================================================*/

MemberDefinition membersOf_vec2[] = {
	{IntroDataType_r32,      "values",   2,        offsetof(vec2, values),   0x4 },
	{IntroDataType_r32,      "x",        1,        offsetof(vec2, x),        0x0 },
	{IntroDataType_r32,      "r",        1,        offsetof(vec2, r),        0x0 },
	{IntroDataType_r32,      "u",        1,        offsetof(vec2, u),        0x0 },
	{IntroDataType_r32,      "s",        1,        offsetof(vec2, s),        0x0 },
	{IntroDataType_r32,      "width",    1,        offsetof(vec2, width),    0x0 },
	{IntroDataType_r32,      "right",    1,        offsetof(vec2, right),    0x0 },
	{IntroDataType_r32,      "y",        1,        offsetof(vec2, y),        0x0 },
	{IntroDataType_r32,      "g",        1,        offsetof(vec2, g),        0x0 },
	{IntroDataType_r32,      "v",        1,        offsetof(vec2, v),        0x0 },
	{IntroDataType_r32,      "t",        1,        offsetof(vec2, t),        0x0 },
	{IntroDataType_r32,      "height",   1,        offsetof(vec2, height),   0x0 },
	{IntroDataType_r32,      "down",     1,        offsetof(vec2, down),     0x0 },
};

MemberDefinition membersOf_quat[] = {
	{IntroDataType_r32,      "w",        1,        offsetof(quat, w),        0x0 },
	{IntroDataType_r32,      "i",        1,        offsetof(quat, i),        0x0 },
	{IntroDataType_r32,      "j",        1,        offsetof(quat, j),        0x0 },
	{IntroDataType_r32,      "k",        1,        offsetof(quat, k),        0x0 },
};

MemberDefinition membersOf_dquat[] = {
	{IntroDataType_r32,      "w",        1,        offsetof(dquat, w),        0x0 },
	{IntroDataType_r32,      "i",        1,        offsetof(dquat, i),        0x0 },
	{IntroDataType_r32,      "j",        1,        offsetof(dquat, j),        0x0 },
	{IntroDataType_r32,      "k",        1,        offsetof(dquat, k),        0x0 },
	{IntroDataType_r32,      "x",        1,        offsetof(dquat, x),        0x0 },
	{IntroDataType_r32,      "y",        1,        offsetof(dquat, y),        0x0 },
	{IntroDataType_r32,      "z",        1,        offsetof(dquat, z),        0x0 },
	{IntroDataType_r32,      "screw",    1,        offsetof(dquat, screw),    0x0 },
};

MemberDefinition membersOf_mat2[] = {
	{IntroDataType_r32,      "data",     4,        offsetof(mat2, data),     0x0 },
	{IntroDataType_r32,      "entry",    4,        offsetof(mat2, entry),    0x0 },
	{IntroDataType_vec2,     "columns",  2,        offsetof(mat2, columns),  0x0 },
};

MemberDefinition membersOf_mat3[] = {
	{IntroDataType_r32,      "data",     9,        offsetof(mat3, data),     0x0 },
	{IntroDataType_r32,      "entry",    9,        offsetof(mat3, entry),    0x0 },
	{IntroDataType_vec3,     "columns",  3,        offsetof(mat3, columns),  0x0 },
};

MemberDefinition membersOf_mat4[] = {
	{IntroDataType_r32,      "data",     16,       offsetof(mat4, data),     0x0 },
	{IntroDataType_r32,      "entry",    16,       offsetof(mat4, entry),    0x0 },
	{IntroDataType_vec4,     "columns",  4,        offsetof(mat4, columns),  0x0 },
};

EnumeratorDefinition enumeratorsOf_something[] = {
	{"someEnumValue",              0 },
	{"someEnumValueMaybe",         1 },
	{"someEnumValueForSure",       2 },
	{"someEnumValueNah",           3 },
	{"someEnumValueNya",           4 },
	{"SOME_ENUM_COUNT",            5 },
	{"someEnumInvalid",           -1 },
};

EnumeratorDefinition enumeratorsOf_testEnumTwo[] = {
	{"testSmall",               2 },
	{"testMedium",         125346 },
	{"testHuge",       2145678901 },
	{"someEnumInvalid",        -1 },
};

MemberDefinition membersOf_Transform[] = {
	{IntroDataType_vec3,     "position",    1,        offsetof(Transform, position),    0x0 },
	{IntroDataType_quat,     "orientation", 1,        offsetof(Transform, orientation), 0x0 },
	{IntroDataType_vec3,     "scale",       1,        offsetof(Transform, scale),       0x0 },
};

MemberDefinition membersOf_TestStruct[] = {
	{IntroDataType_bool,        "boolArray",   15,       offsetof(TestStruct, boolArray),   0x0 },
	{IntroDataType_u32,         "u32Array",    10,       offsetof(TestStruct, u32Array),    0x0 },
	{IntroDataType_u32,         "u32Array2D",  100,      offsetof(TestStruct, u32Array2D),  0x0 },
	{IntroDataType_vec3,        "vec3Array3D", 1000,     offsetof(TestStruct, vec3Array3D), 0x0 },
	{IntroDataType_vec3,        "vec3Ptr",     1,        offsetof(TestStruct, vec3Ptr),     0x1 },
	{IntroDataType_testEnumTwo, "testEnum",    1,        offsetof(TestStruct, testEnum),    0x2 },
};

/*===========================================================================================================================================================================
|
|       x64\Debug\testFile2.h - Introspection Data
|
===========================================================================================================================================================================*/

MemberDefinition membersOf_game_Key[] = {
	{IntroDataType_bool,     "endDown",             1,        offsetof(game_Key, endDown),             0x0 },
	{IntroDataType_u8,       "halfTransitionCount", 1,        offsetof(game_Key, halfTransitionCount), 0x0 },
};

MemberDefinition membersOf_game_Mouse[] = {
	{IntroDataType_game_Key, "left_Button",   1,        offsetof(game_Mouse, left_Button),   0x0 },
	{IntroDataType_game_Key, "middle_Button", 1,        offsetof(game_Mouse, middle_Button), 0x0 },
	{IntroDataType_game_Key, "right_Button",  1,        offsetof(game_Mouse, right_Button),  0x0 },
	{IntroDataType_r32,      "scrollTicks",   1,        offsetof(game_Mouse, scrollTicks),   0x0 },
	{IntroDataType_vec2,     "pos",           1,        offsetof(game_Mouse, pos),           0x0 },
	{IntroDataType_vec2,     "posDelta",      1,        offsetof(game_Mouse, posDelta),      0x0 },
};

MemberDefinition membersOf_fluffy_InputState[] = {
	{IntroDataType_game_Mouse,    "mouse",      1,        offsetof(fluffy_InputState, mouse),      0x0 },
	{IntroDataType_game_Keyboard, "keyboard",   1,        offsetof(fluffy_InputState, keyboard),   0x0 },
	{IntroDataType_u32,           "charCount",  1,        offsetof(fluffy_InputState, charCount),  0x0 },
	{IntroDataType_char,          "textBuffer", 1,        offsetof(fluffy_InputState, textBuffer), 0x0 },
};

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\fluffyCalc.h - Introspection Data
|
===========================================================================================================================================================================*/

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\fluffyInput.h - Introspection Data
|
===========================================================================================================================================================================*/

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\fluffyMath.h - Introspection Data
|
===========================================================================================================================================================================*/

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\fstring.h - Introspection Data
|
===========================================================================================================================================================================*/

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\memArena.h - Introspection Data
|
===========================================================================================================================================================================*/

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\platform.h - Introspection Data
|
===========================================================================================================================================================================*/

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\platformFileIO.h - Introspection Data
|
===========================================================================================================================================================================*/

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\platformMemory.h - Introspection Data
|
===========================================================================================================================================================================*/

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\RNG.h - Introspection Data
|
===========================================================================================================================================================================*/

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\typedefs.h - Introspection Data
|
===========================================================================================================================================================================*/

/*===========================================================================================================================================================================
|
|       C:\Development\Witch's Blade\codebase\core\source\utility.h - Introspection Data
|
===========================================================================================================================================================================*/

