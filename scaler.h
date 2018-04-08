/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef SCALER_H__
#define SCALER_H__

#include "intern.h"

typedef void (*ScaleProc)(uint16_t *dst, int dstPitch, const uint16_t *pal, const uint8_t *src, int srcPitch, int w, int h);
typedef void (*ClearProc)(uint16_t *dst, int dstPitch, int x, int y, int w, int h);

void point1x(uint16_t *dst, int dstPitch, const uint16_t *pal, const uint8_t *src, int srcPitch, int w, int h);
void point2x(uint16_t *dst, int dstPitch, const uint16_t *pal, const uint8_t *src, int srcPitch, int w, int h);
void point3x(uint16_t *dst, int dstPitch, const uint16_t *pal, const uint8_t *src, int srcPitch, int w, int h);
void scale2x(uint16_t *dst, int dstPitch, const uint16_t *pal, const uint8_t *src, int srcPitch, int w, int h);
void scale3x(uint16_t *dst, int dstPitch, const uint16_t *pal, const uint8_t *src, int srcPitch, int w, int h);
void clear1x(uint16_t *dst, int dstPitch, int x, int y, int w, int h);
void clear2x(uint16_t *dst, int dstPitch, int x, int y, int w, int h);
void clear3x(uint16_t *dst, int dstPitch, int x, int y, int w, int h);

typedef void (*ScaleProc32)(int factor, uint32_t *dst, int dstPitch, const uint32_t *src, int srcPitch, int w, int h);

#define SCALER_TAG 1

struct Scaler {
	uint32_t tag;
	const char *name;
	int factorMin, factorMax;
	ScaleProc32 scale;
};

extern const Scaler scaler_nearest;
extern const Scaler scaler_xbrz;

#endif // SCALER_H__
