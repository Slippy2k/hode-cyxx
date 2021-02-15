
#include "scaler.h"

template <int N>
static void scaleImpl(uint32_t *dst, int dstPitch, const uint8_t *src, int srcPitch, int w, int h, const uint32_t *palette) {
	while (h--) {
		uint32_t *p = dst;
		for (int i = 0; i < w; ++i, p += N) {
			const uint32_t c = palette[src[i]];
			for (int j = 0; j < N; ++j) {
				for (int k = 0; k < N; ++k) {
					*(p + j * dstPitch + k) = c;
				}
			}
		}
		dst += dstPitch * N;
		src += srcPitch;
	}
}

static void scale_nearest(int factor, uint32_t *dst, int dstPitch, const uint8_t *src, int srcPitch, int w, int h, const uint32_t *palette) {
	switch (factor) {
	case 2:
		scaleImpl<2>(dst, dstPitch, src, srcPitch, w, h, palette);
		break;
	case 3:
		scaleImpl<3>(dst, dstPitch, src, srcPitch, w, h, palette);
		break;
	case 4:
		scaleImpl<4>(dst, dstPitch, src, srcPitch, w, h, palette);
		break;
	}
}

const Scaler scaler_nearest = {
	"nearest",
	2, 4,
	scale_nearest
};
