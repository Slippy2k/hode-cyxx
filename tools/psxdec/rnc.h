
#ifndef RNC_H__
#define RNC_H__

#include <stdint.h>

#define RNC2_SIGNATURE   0x524E4302 // "RNC\002"

struct RncDecoder {

	uint16_t _crcTable[256];

	uint16_t _bitBuffl;
	uint16_t _bitBuffh;
	uint8_t _bitCount;

	const uint8_t *_srcPtr;
	uint8_t *_dstPtr;

	int16_t _inputByteLeft;

	RncDecoder();
	int32_t unpackM2(const void *input, void *output);

	void initCrc();
	uint16_t crcBlock(const uint8_t *block, uint32_t size);
	uint16_t inputBits(uint8_t amount);
	int getbit();
};

#endif // RNC_H__
