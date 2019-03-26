/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
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

inline uint16_t READ_LE_UINT16(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[1] << 8) | b[0];
}

inline uint32_t READ_LE_UINT32(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

inline uint16_t BSWAP_16(uint16_t n) {
	return (n >> 8) | (n << 8);
}

inline uint32_t BSWAP_32(uint32_t n) {
	n = ((n << 8) & 0xFF00FF00) | ((n >> 8) & 0x00FF00FF);
	return (n >> 16) | (n << 16);
}

inline uint16_t FROM_LE16(uint16_t value) {
#ifdef BYTE_ORDER_BE
	return BSWAP_16(value);
#else
	return value;
#endif
}

inline uint32_t FROM_LE32(uint32_t value) {
#ifdef BYTE_ORDER_BE
	return BSWAP_32(value);
#else
	return value;
#endif
}

inline void WRITE_LE_UINT16(void *ptr, uint16_t v) {
	for (int i = 0; i < 2; ++i) {
		((uint8_t *)ptr)[i] = (v >> (8 * i)) & 255;
	}
}

inline void WRITE_LE_UINT32(void *ptr, uint32_t v) {
	for (int i = 0; i < 4; ++i) {
		((uint8_t *)ptr)[i] = (v >> (8 * i)) & 255;
	}
}

#undef MIN
template<typename T>
inline T MIN(T v1, T v2) {
	return (v1 < v2) ? v1 : v2;
}

#undef MAX
template<typename T>
inline T MAX(T v1, T v2) {
	return (v1 > v2) ? v1 : v2;
}

template<typename T>
inline T ABS(T t) {
	return (t < 0) ? -t : t;
}

template<typename T>
inline T CLIP(T t, T tmin, T tmax) {
	if (t < tmin) {
		return tmin;
	} else if (t > tmax) {
		return tmax;
	} else {
		return t;
	}
}

template<typename T>
inline void SWAP(T &a, T &b) {
	T tmp = a; a = b; b = tmp;
}

inline int merge_bits(int dbit, int sbit, int mask) {
	return ((sbit ^ dbit) & mask) ^ dbit;
//	return (dbit & ~mask) | (sbit & mask);
}

#endif // INTERN_H__
