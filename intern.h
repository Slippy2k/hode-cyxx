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
#include <endian.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#define PACKED __attribute__((packed))

// not alignment safe
inline uint16_t READ_LE_UINT16(const void *ptr) {
	return le16toh(*(const uint16_t *)ptr);
}

inline uint32_t READ_LE_UINT32(const void *ptr) {
	return le32toh(*(const uint32_t *)ptr);
}

inline void WRITE_LE_UINT16(void *ptr, uint16_t v) {
	*((uint16_t *)ptr) = htole16(v);
}

inline void WRITE_LE_UINT32(void *ptr, uint32_t v) {
	*((uint32_t *)ptr) = htole32(v);
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
