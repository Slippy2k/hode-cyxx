
#include <stdio.h>
#include <string.h>
#include "rnc.h"

static uint16_t READ_LE_UINT16(const uint8_t *p) {
	return p[0] | (p[1] << 8);
}

static uint16_t READ_BE_UINT16(const uint8_t *p) {
	return (p[0] << 8) | p[1];
}

static uint32_t READ_BE_UINT32(const uint8_t *p) {
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

//return codes
#define NOT_PACKED  0
#define PACKED_CRC  -1
#define UNPACKED_CRC    -2

//other defines
#define HEADER_LEN  18

RncDecoder::RncDecoder() {
	initCrc();

	_bitBuffl = 0;
	_bitBuffh = 0;
	_bitCount = 0;
	_srcPtr = 0;
	_dstPtr = 0;
	_inputByteLeft = 0;
}

void RncDecoder::initCrc() {

	uint16_t cnt = 0;
	uint16_t tmp1 = 0;
	uint16_t tmp2 = 0;

	for (tmp2 = 0; tmp2 < 0x100; tmp2++) {
		tmp1 = tmp2;
		for (cnt = 8; cnt > 0; cnt--) {
			if (tmp1 % 2) {
				tmp1 >>= 1;
				tmp1 ^= 0x0a001;
			} else
				tmp1 >>= 1;
		}
		_crcTable[tmp2] = tmp1;
	}
}

//calculate 16 bit crc of a block of memory
uint16_t RncDecoder::crcBlock(const uint8_t *block, uint32_t size) {

	uint16_t crc = 0;
	uint8_t *crcTable8 = (uint8_t *)_crcTable; //make a uint8* to crc_table
	uint8_t tmp;
	uint32_t i;

	for (i = 0; i < size; i++) {
		tmp = *block++;
		crc ^= tmp;
		tmp = (uint8_t)((crc >> 8) & 0x00FF);
		crc &= 0x00FF;
		crc = *(uint16_t *)&crcTable8[crc << 1];
		crc ^= tmp;
	}

	return crc;
}

uint16_t RncDecoder::inputBits(uint8_t amount) {

	uint16_t newBitBuffh = _bitBuffh;
	uint16_t newBitBuffl = _bitBuffl;
	int16_t newBitCount = _bitCount;
	uint16_t remBits, returnVal;

	returnVal = ((1 << amount) - 1) & newBitBuffl;
	newBitCount -= amount;

	if (newBitCount < 0) {
		newBitCount += amount;
		remBits = (newBitBuffh << (16 - newBitCount));
		newBitBuffh >>= newBitCount;
		newBitBuffl >>= newBitCount;
		newBitBuffl |= remBits;
		_srcPtr += 2;

		// added some more check here to prevent reading in the buffer
		// if there are no bytes anymore.
		_inputByteLeft -= 2;
		if (_inputByteLeft <= 0)
			newBitBuffh = 0;
		else if (_inputByteLeft == 1)
			newBitBuffh = *_srcPtr;
		else
			newBitBuffh = READ_LE_UINT16(_srcPtr);
		amount -= newBitCount;
		newBitCount = 16 - amount;
	}
	remBits = (newBitBuffh << (16 - amount));
	_bitBuffh = newBitBuffh >> amount;
	_bitBuffl = (newBitBuffl >> amount) | remBits;
	_bitCount = (uint8_t)newBitCount;

	return returnVal;
}

int RncDecoder::getbit() {

	if (_bitCount == 0) {
		_bitBuffl = *_srcPtr++;
		_bitCount = 8;
	}
	uint8_t temp = (_bitBuffl & 0x80) >> 7;
	_bitBuffl <<= 1;
	_bitCount--;
	return temp;
}

int32_t RncDecoder::unpackM2(const void *input, void *output) {

	const uint8_t *inputptr = (const uint8_t *)input;

	uint32_t unpackLen = 0;
	uint32_t packLen = 0;
	uint16_t crcUnpacked = 0;
	uint16_t crcPacked = 0;

	_bitBuffl = 0;
	_bitCount = 0;

	//Check for "RNC "
	if (READ_BE_UINT32(inputptr) != RNC2_SIGNATURE)
		return NOT_PACKED;

	inputptr += 4;

	// read unpacked/packed file length
	unpackLen = READ_BE_UINT32(inputptr);
	inputptr += 4;
	packLen = READ_BE_UINT32(inputptr);
	inputptr += 4;

	//read CRC's
	crcUnpacked = READ_BE_UINT16(inputptr);
	inputptr += 2;
	crcPacked = READ_BE_UINT16(inputptr);
	inputptr += 2;
	inputptr = (inputptr + HEADER_LEN - 16);

	if (crcBlock(inputptr, packLen) != crcPacked)
		return PACKED_CRC;

	inputptr = (((const uint8_t *)input) + HEADER_LEN);
	_srcPtr = inputptr;
	_dstPtr = (uint8_t *)output;

	uint16_t ofs, len;
	uint8_t ofs_hi, ofs_lo;

	len = 0;
	ofs_hi = 0;
	ofs_lo = 0;

	getbit();
	getbit();

	while (1) {

		bool loadVal = false;

		while (getbit() == 0)
			*_dstPtr++ = *_srcPtr++;

		len = 2;
		ofs_hi = 0;
		if (getbit() == 0) {
			len = (len << 1) | getbit();
			if (getbit() == 1) {
				len--;
				len = (len << 1) | getbit();
				if (len == 9) {
					len = 4;
					while (len--)
						ofs_hi = (ofs_hi << 1) | getbit();
					len = (ofs_hi + 3) * 4;
					while (len--)
						*_dstPtr++ = *_srcPtr++;
					continue;
				}
			}
			loadVal = true;
		} else {
			if (getbit() == 1) {
				len++;
				if (getbit() == 1) {
					len = *_srcPtr++;
					if (len == 0) {
						if (getbit() == 1)
							continue;
						else
							break;
					}
					len += 8;
				}
				loadVal = true;
			} else {
				loadVal = false;
			}
		}

		if (loadVal) {
			if (getbit() == 1) {
				ofs_hi = (ofs_hi << 1) | getbit();
				if (getbit() == 1) {
					ofs_hi = ((ofs_hi << 1) | getbit()) | 4;
					if (getbit() == 0)
						ofs_hi = (ofs_hi << 1) | getbit();
				} else if (ofs_hi == 0) {
					ofs_hi = 2 | getbit();
				}
			}
		}

		ofs_lo = *_srcPtr++;
		ofs = (ofs_hi << 8) | ofs_lo;
		while (len--) {
			*_dstPtr = *(uint8_t *)(_dstPtr - ofs - 1);
			_dstPtr++;
		}

	}

	if (crcBlock((uint8_t *)output, unpackLen) != crcUnpacked)
		return UNPACKED_CRC;

	// all is done..return the amount of unpacked bytes
	return unpackLen;
}

