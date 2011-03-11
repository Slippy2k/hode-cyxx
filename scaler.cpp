/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#include "scaler.h"

void point1x(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h) {
	while (h--) {
		for (int i = 0; i < w; ++i) {
			dst[i] = pal[src[i]];
		}
		dst += dstPitch;
		src += srcPitch;
	}
}

void point2x(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h) {
	while (h--) {
		uint16 *p = dst;
		for (int i = 0; i < w; ++i, p += 2) {
			uint16 c = pal[*(src + i)];
			*(p) = c;
			*(p + 1) = c;
			*(p + dstPitch) = c;
			*(p + dstPitch + 1) = c;
		}
		dst += dstPitch * 2;
		src += srcPitch;
	}
}

void point3x(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h) {
	while (h--) {
		uint16 *p = dst;
		for (int i = 0; i < w; ++i, p += 3) {
			uint16 c = pal[*(src + i)];
			*(p) = c;
			*(p + 1) = c;
			*(p + 2) = c;
			*(p + dstPitch) = c;
			*(p + dstPitch + 1) = c;
			*(p + dstPitch + 2) = c;
			*(p + 2 * dstPitch) = c;
			*(p + 2 * dstPitch + 1) = c;
			*(p + 2 * dstPitch + 2) = c;
		}
		dst += dstPitch * 3;
		src += srcPitch;
	}
}

void scale2x(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h) {
	while (h--) {
		uint16 *p = dst;
		for (int i = 0; i < w; ++i, p += 2) {
			uint16 B = pal[*(src + i - srcPitch)];
			uint16 D = pal[*(src + i - 1)];
			uint16 E = pal[*(src + i)];
			uint16 F = pal[*(src + i + 1)];
			uint16 H = pal[*(src + i + srcPitch)];
			if (B != H && D != F) {
				*(p) = D == B ? D : E;
				*(p + 1) = B == F ? F : E;
				*(p + dstPitch) = D == H ? D : E;
				*(p + dstPitch + 1) = H == F ? F : E;
			} else {
				*(p) = E;
				*(p + 1) = E;
				*(p + dstPitch) = E;
				*(p + dstPitch + 1) = E;
			}
		}
		dst += dstPitch * 2;
		src += srcPitch;
	}
}

void scale3x(uint16 *dst, int dstPitch, const uint16 *pal, const uint8 *src, int srcPitch, int w, int h) {
	while (h--) {
		uint16 *p = dst;
		for (int i = 0; i < w; ++i, p += 3) {
			uint16 A = pal[*(src + i - srcPitch - 1)];
			uint16 B = pal[*(src + i - srcPitch)];
			uint16 C = pal[*(src + i - srcPitch + 1)];
			uint16 D = pal[*(src + i - 1)];
			uint16 E = pal[*(src + i)];
			uint16 F = pal[*(src + i + 1)];
			uint16 G = pal[*(src + i + srcPitch - 1)];
			uint16 H = pal[*(src + i + srcPitch)];
			uint16 I = pal[*(src + i + srcPitch + 1)];
			if (B != H && D != F) {
				*(p) = D == B ? D : E;
				*(p + 1) = (D == B && E != C) || (B == F && E != A) ? B : E;
				*(p + 2) = B == F ? F : E;
				*(p + dstPitch) = (D == B && E != G) || (D == B && E != A) ? D : E;
				*(p + dstPitch + 1) = E;
				*(p + dstPitch + 2) = (B == F && E != I) || (H == F && E != C) ? F : E;
				*(p + 2 * dstPitch) = D == H ? D : E;
				*(p + 2 * dstPitch + 1) = (D == H && E != I) || (H == F && E != G) ? H : E;
				*(p + 2 * dstPitch + 2) = H == F ? F : E;
			} else {
				*(p) = E;
				*(p + 1) = E;
				*(p + 2) = E;
				*(p + dstPitch) = E;
				*(p + dstPitch + 1) = E;
				*(p + dstPitch + 2) = E;
				*(p + 2 * dstPitch) = E;
				*(p + 2 * dstPitch + 1) = E;
				*(p + 2 * dstPitch + 2) = E;
			}
		}
		dst += dstPitch * 3;
		src += srcPitch;
	}
}

void clear1x(uint16 *dst, int dstPitch, int x, int y, int w, int h) {
	uint16 *p = dst + y * dstPitch + x;
	for (; y < h; ++y) {
		memset(p, 0, w * sizeof(uint16));
		p += dstPitch;
	}
}

void clear2x(uint16 *dst, int dstPitch, int x, int y, int w, int h) {
	uint16 *p = dst + (y * dstPitch + x) * 2;
	for (y = 0; y < h; ++y) {
		for (int i = 0; i < 2; ++i) {
			memset(p, 0, w * sizeof(uint16) * 2);
			p += dstPitch;
		}
	}
}

void clear3x(uint16 *dst, int dstPitch, int x, int y, int w, int h) {
	uint16 *p = dst + (y * dstPitch + x) * 3;
	for (y = 0; y < h; ++y) {
		for (int i = 0; i < 3; ++i) {
			memset(p, 0, w * sizeof(uint16) * 3);
			p += dstPitch;
		}
	}
}

