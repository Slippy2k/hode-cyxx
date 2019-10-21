
#include <stdlib.h>
#include "mdec.h"
#include "PSXMDECDecoder.h"

int decodeMDEC(const uint8_t *src, int len, int w, int h, const void *userdata, void (*output)(const MdecOutput *, const void *)) {

	const int size = w * h * 4;
	uint8_t *rgba = (uint8_t *)malloc(size);
	if (rgba) {
		PSXMDECDecoder().DecodeFrameToABGR32((uint16_t *)rgba, (uint16_t *)src, w, h);

		uint8_t *rgb = (uint8_t *)malloc(w * h * 3);
		if (rgb) {
			for (int i = 0; i < w * h; ++i) {
				uint32_t color = *((uint32_t *)rgba + i);
				rgb[3 * i] = color & 255;
				rgb[3 * i + 1] = (color >> 8) & 255;
				rgb[3 * i + 2] = (color >> 16) & 255;
			}
		}

		MdecOutput mdecOutput;
		mdecOutput.format = kMdecOutputRgb;
		mdecOutput.w = w;
		mdecOutput.h = h;
		mdecOutput.planes[0].ptr = rgb;
		mdecOutput.planes[0].pitch = w * 3;
		output(&mdecOutput, userdata);

		free(rgba);
	}
	return size;
}
