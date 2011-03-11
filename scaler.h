/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#ifndef SCALER_H__
#define SCALER_H__

#include "intern.h"

typedef void (*ScaleProc)(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h);
typedef void (*ClearProc)(uint16 *dst, int dstPitch, int x, int y, int w, int h);

void point1x(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h);
void point2x(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h);
void point3x(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h);
void scale2x(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h);
void scale3x(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h);
void clear1x(uint16 *dst, int dstPitch, int x, int y, int w, int h);
void clear2x(uint16 *dst, int dstPitch, int x, int y, int w, int h);
void clear3x(uint16 *dst, int dstPitch, int x, int y, int w, int h);

#endif // SCALER_H__
