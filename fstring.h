#pragma once

#include "typedefs.h"
#include "fluffyMath.h"
#include "string.h"

#define DIGITS_ASCII_OFFSET '0'
#define MAX_FLOAT_DECIMALS 5u

static u32 AppendCString(char* dst, const char* src) {
	u32 length = 0;
	while (src[length]) {
		dst[length] = src[length];
		length++;
	}
	if (length) {
		dst[length] = 0;
	}
	return(length);
}

static u32 r32ToChars(char* dst, const r32 number, const u32 decimals) {
	static const r64 multiplier[] = {1,10,100,1000,10000,100000};

	const u32 tempDecimals = MinU32(decimals, MAX_FLOAT_DECIMALS);
	const u32 negative = (number < 0.0f);
	u64 tempNumber = (u64)(multiplier[tempDecimals] * AbsR32(number) + 0.5f);
	const u32 length = MaxU32(ceil(log10(tempNumber)), tempDecimals+1) + negative + (tempDecimals>0);
	s32 i = length-1;

	if (negative) {
		dst[0] = '-';
	}
	do {
		dst[i--] = DIGITS_ASCII_OFFSET + tempNumber%10;
		tempNumber /= 10;
		if (i == (length-tempDecimals-1)) {
			dst[i--] = '.';
		}
	} while (tempNumber || ((length-i) <= (tempDecimals ? tempDecimals+2 : 0)));
	dst[length] = 0;
	return(length);
}

static u32 s64ToChars(char* dst, const s64 number) {
	u64 tempNumber = number;
	const u32 negative = (number < 0);
	const u32 length = f_Log10(tempNumber)+negative;
	u32 index = length-1;
	if (negative) {
		dst[0] = '-';
	}
	do {
		dst[index] = tempNumber%10+DIGITS_ASCII_OFFSET;
		tempNumber /= 10;
		index--;
	} while (tempNumber);
	dst[length] = 0;
	return(length);
}

static u32 u64ToChars(char* dst, const u64 number) {
	u64 tempNumber = number;
	const u32 length = f_Log10(tempNumber);
	u32 index = length-1;
	do {
		dst[index] = tempNumber%10+DIGITS_ASCII_OFFSET;
		tempNumber /= 10;
		index--;
	} while (tempNumber);
	dst[length] = 0;
	return(length);
}