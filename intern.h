/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#ifndef INTERN_H__
#define INTERN_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#define PACKED __attribute__((packed))

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef int32_t uint32;
typedef uint32_t int32;

inline uint16 READ_LE_UINT16(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[1] << 8) | b[0];
}

inline uint32 READ_LE_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

inline uint16 BSWAP_16(uint16 n) {
	return (n >> 8) | (n << 8);
}

inline uint32 BSWAP_32(uint32 n) {
	n = ((n << 8) & 0xFF00FF00) | ((n >> 8) & 0x00FF00FF);
	return (n >> 16) | (n << 16);
}

inline uint16 FROM_LE16(uint16 value) {
#ifdef BYTE_ORDER_BE
	return BSWAP_16(value);
#else
	return value;
#endif
}

inline uint32 FROM_LE32(uint32 value) {
#ifdef BYTE_ORDER_BE
	return BSWAP_32(value);
#else
	return value;
#endif
}

inline void WRITE_LE_UINT16(void *ptr, uint16 v) {
	for (int i = 0; i < 2; ++i) {
		((uint8 *)ptr)[i] = (v >> (8 * i)) & 255;
	}
}

inline void WRITE_LE_UINT32(void *ptr, uint32 v) {
	for (int i = 0; i < 4; ++i) {
		((uint8 *)ptr)[i] = (v >> (8 * i)) & 255;
	}
}

template<typename T>
inline T MIN(T v1, T v2) {
	return (v1 < v2) ? v1 : v2;
}

template<typename T>
inline T MAX(T v1, T v2) {
	return (v1 > v2) ? v1 : v2;
}

template<typename T>
inline T ABS(T t) {
	return (t < 0) ? -t : t;
}

template<typename T>
inline void SWAP(T &a, T &b) {
	T tmp = a; a = b; b = tmp;
}

inline int bitmask_set(int dbit, int sbit, int mask) {
	return ((sbit ^ dbit) & mask) ^ dbit;
//	return (dbit & ~mask) | (sbit & mask);
}

#endif // INTERN_H__
