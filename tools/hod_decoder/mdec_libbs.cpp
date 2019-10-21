
#include <stdlib.h>
#include "mdec.h"
#include "bs.h"

int decodeMDEC(const uint8_t *src, int len, int w, int h, const void *userdata, void (*output)(const MdecOutput *, const void *)) {
	static bool bs_inited = false;

	if (!bs_inited) {
		bs_init();
		bs_inited = true;
	}

	const int size = w * h * 3;
	uint8_t *rgb = (uint8_t *)malloc(size);
	if (rgb) {
		bs_decode_rgb24(rgb, (bs_header_t *)src, w, h, 0);

		MdecOutput mdecOutput;
		mdecOutput.format = kMdecOutputRgb;
		mdecOutput.w = w;
		mdecOutput.h = h;
		mdecOutput.planes[0].ptr = rgb;
		mdecOutput.planes[0].pitch = w * 3;
		output(&mdecOutput, userdata);

		free(rgb);
	}
	return size;
}
