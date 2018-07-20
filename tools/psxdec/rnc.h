
#ifndef RNC_H__
#define RNC_H__

#include <stdint.h>

#define RNC2_SIGNATURE   0x524E4302 // "RNC\002"

struct RncDecoder {

	uint16_t _bits;
	uint8_t _bitCount;

	const uint8_t *_srcPtr;
	uint8_t *_dstPtr;

	RncDecoder();
	int unpackM2(const uint8_t *input, uint8_t *output);

	uint16_t crc16(const uint8_t *block, uint32_t size) const;
	int nextBit();
};

#endif // RNC_H__
