
#ifndef MDEC_H__
#define MDEC_H__

#include <stdint.h>

struct MdecOutput {
	int w, h;
	struct {
		uint8_t *ptr;
		int pitch;
	} planes[3];
};

int decodeMDEC(const uint8_t *src, int len, int w, int h, const uint8_t *mborder, int mblen, const void *userdata, void (*output)(const MdecOutput *, const void *));

#endif // MDEC_H__
