
#include "scaler.h"
#include "xbrz/xbrz.h"

static void scale_xbrz(int factor, uint32_t *dst, int dstPitch, const uint32_t *src, int srcPitch, int w, int h) {
	xbrz::scale(factor, src, dst, w, h, xbrz::ColorFormat::RGB);
}

const Scaler scaler_xbrz = {
	SCALER_TAG,
	"xbrz",
	2, 6,
	scale_xbrz
};
