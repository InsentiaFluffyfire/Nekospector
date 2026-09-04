#pragma once

//Supports introspecting structs, unions and enums
//If there is a typedef then the struct name and its alias must be identical
//All members that are structs, unions or enums must be typedefed
//char arrays are assumed to be containers for null-terminated string
//char pointers are assumed to point at null terminated strings
//Possible params:
// mode:
// - "tupple": sets a flag on arrays in a struct/union to be displayed as a horizontal tupple rather than a vertical list
// - "only x": sets a flag such that only the x-th member of a union gets parsed
#define INTROSPECT(params)