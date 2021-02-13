
#include "scaler.h"

template <int N>
static void scale(uint32_t *dst, int dstPitch, const uint32_t *src, int srcPitch, int w, int h) {
	while (h--) {
		uint32_t *p = dst;
		for (int i = 0; i < w; ++i, p += N) {
			uint32_t c = *(src + i);
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

static void scale_nearest(int factor, uint32_t *dst, int dstPitch, const uint32_t *src, int srcPitch, int w, int h) {
	switch (factor) {
	case 2:
		scale<2>(dst, dstPitch, src, srcPitch, w, h);
		break;
	case 3:
		scale<3>(dst, dstPitch, src, srcPitch, w, h);
		break;
	case 4:
		scale<4>(dst, dstPitch, src, srcPitch, w, h);
		break;
	}
}

const Scaler scaler_nearest = {
	"nearest",
	2, 4,
	scale_nearest
};
