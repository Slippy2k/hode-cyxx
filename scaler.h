/*
 * Heart of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir (cyx@users.sourceforge.net)
 */

#ifndef SCALER_H__
#define SCALER_H__

#include "intern.h"

typedef void (*ScaleProc)(int factor, uint32_t *dst, int dstPitch, const uint8_t *src, int srcPitch, int w, int h, const uint32_t *palette);
typedef void (*PaletteProc)(const uint32_t *palette);

struct Scaler {
	const char *name;
	int factorMin, factorMax;
	ScaleProc scale;
	PaletteProc palette;
};

extern const Scaler scaler_nearest;
extern const Scaler scaler_xbr;

#endif // SCALER_H__
