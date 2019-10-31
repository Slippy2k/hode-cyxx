
#include <math.h>
#include "intern.h"
#include "mdec.h"

struct BitStream { // most significant 16 bits
	const uint8_t *_src;
	uint16_t _bits;
	int _len;
	const uint8_t *_end;
	bool _endOfStream;

	BitStream(const uint8_t *src, int size)
		: _src(src), _len(0), _end(src + size), _endOfStream(false) {
	}

	int getBits(int len) {
		int value = 0;
		while (len != 0) {
			if (_len == 0) {
				assert(_src < _end);
				_bits = READ_LE_UINT16(_src); _src += 2;
				if (_src >= _end) {
					_endOfStream = true;
				}
				_len = 16;
			}
			const int count = (len < _len) ? len : _len;
			value <<= count;
			value |= _bits >> (16 - count);
			_bits <<= count;
			_len -= count;
			len -= count;
		}
		return value;
	}
	int getSignedBits(int len) {
		const int shift = 32 - len;
		int32_t value = getBits(len);
		return (value << shift) >> shift;
	}
};

static int readDC(BitStream *bs, int version) {
	assert(version == 2);
	return bs->getSignedBits(10);
}

static void readAC(BitStream *bs, int *coefficients) {
	fprintf(stdout, "readAC unimplemented\n");
	// while (!bs->_endOfStream)
}

static const uint8_t _zigZagTable[8 * 8] = {
	 0,  1,  5,  6, 14, 15, 27, 28,
	 2,  4,  7, 13, 16, 26, 29, 42,
	 3,  8, 12, 17, 25, 30, 41, 43,
	 9, 11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
};

static const uint8_t _quantizationTable[8 * 8] = {
	 2, 16, 19, 22, 26, 27, 29, 34,
	16, 16, 22, 24, 27, 29, 34, 37,
	19, 22, 26, 27, 29, 34, 34, 38,
	22, 22, 26, 27, 29, 34, 37, 40,
	22, 26, 27, 29, 32, 35, 40, 48,
	26, 27, 29, 32, 35, 40, 48, 58,
	26, 27, 29, 34, 38, 46, 56, 69,
	27, 29, 35, 38, 46, 56, 69, 83
};

static void dequantizeBlock(int *coefficients, float *block, int scale) {
	block[0] = coefficients[0] * _quantizationTable[0]; // DC
	for (int i = 1; i < 8 * 8; i++) {
		block[i] = (float)coefficients[_zigZagTable[i]] * _quantizationTable[i] * scale / 8;
	}
}

// _idct8x8[x][y] = cos(((2 * x + 1) * y) * (M_PI / 16.0)) * 0.5;
// _idct8x8[x][y] /= sqrt(2.0) if y == 0
static const double _idct8x8[8][8] = {
	{ 0.353553390593274,  0.490392640201615,  0.461939766255643,  0.415734806151273,  0.353553390593274,  0.277785116509801,  0.191341716182545,  0.097545161008064 },
	{ 0.353553390593274,  0.415734806151273,  0.191341716182545, -0.097545161008064, -0.353553390593274, -0.490392640201615, -0.461939766255643, -0.277785116509801 },
	{ 0.353553390593274,  0.277785116509801, -0.191341716182545, -0.490392640201615, -0.353553390593274,  0.097545161008064,  0.461939766255643,  0.415734806151273 },
	{ 0.353553390593274,  0.097545161008064, -0.461939766255643, -0.277785116509801,  0.353553390593274,  0.415734806151273, -0.191341716182545, -0.490392640201615 },
	{ 0.353553390593274, -0.097545161008064, -0.461939766255643,  0.277785116509801,  0.353553390593274, -0.415734806151273, -0.191341716182545,  0.490392640201615 },
	{ 0.353553390593274, -0.277785116509801, -0.191341716182545,  0.490392640201615, -0.353553390593273, -0.097545161008064,  0.461939766255643, -0.415734806151273 },
	{ 0.353553390593274, -0.415734806151273,  0.191341716182545,  0.097545161008064, -0.353553390593274,  0.490392640201615, -0.461939766255643,  0.277785116509801 },
	{ 0.353553390593274, -0.490392640201615,  0.461939766255643, -0.415734806151273,  0.353553390593273, -0.277785116509801,  0.191341716182545, -0.097545161008064 }
};

static void idct(float *dequantData, float *result) {
	float tmp[8 * 8];
	// 1D IDCT rows
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			float p = 0;
			for (int i = 0; i < 8; ++i) {
				p += dequantData[i] * _idct8x8[x][i];
			}
			tmp[y + x * 8] = p;
		}
		dequantData += 8;
	}
	// 1D IDCT columns
	for (int x = 0; x < 8; x++) {
		const float *u = tmp + x * 8;
		for (int y = 0; y < 8; y++) {
			float p = 0;
			for (int i = 0; i < 8; ++i) {
				p += u[i] * _idct8x8[y][i];
			}
                        result[y * 8 + x] = p;
		}
	}
}

static void decodeBlock(BitStream *bs, int x8, int y8, uint8_t *dst, int dstPitch, int scale, int version) {
	int coefficients[8 * 8];
	memset(coefficients, 0, sizeof(coefficients));
	coefficients[0] = readDC(bs, version);
	readAC(bs, &coefficients[1]);

	float dequantData[8 * 8];
	dequantizeBlock(coefficients, dequantData, scale);

	float idctData[8 * 8];
	idct(dequantData, idctData);

	dst += (y8 * dstPitch + x8) * 8;
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			const int val = (int)round(idctData[y * 8 + x]); // (-128,127) range
			if (val < -128) {
				dst[x] = 0;
			} else if (val > 127) {
				dst[x] = 255;
			} else {
				dst[x] = 128 + val;
			}
		}
		dst += dstPitch;
	}
}

enum {
	kOutputPlaneY = 0,
	kOutputPlaneCb = 1,
	kOutputPlaneCr = 2
};

int decodeMDEC(const uint8_t *src, int len, int w, int h, const void *userdata, void (*output)(const MdecOutput *, const void *)) {
	BitStream bs(src, len);
	bs.getBits(16);
	const uint16_t vlc = bs.getBits(16);
	assert(vlc == 0x3800);
	const uint16_t qscale = bs.getBits(16);
	const uint16_t version = bs.getBits(16);
	fprintf(stdout, "mdec qscale %d version %d\n", qscale, version);
	assert(version == 2);

	const int blockW = (w + 15) / 16;
	const int blockH = (h + 15) / 16;

	MdecOutput mdecOut;
	mdecOut.format = kMdecOutputYuv;
	mdecOut.w = blockW * 16;
	mdecOut.h = blockH * 16;
	mdecOut.planes[kOutputPlaneY].ptr = (uint8_t *)malloc(blockW * 16 * blockH * 16);
	mdecOut.planes[kOutputPlaneY].pitch = blockW * 16;
	mdecOut.planes[kOutputPlaneCb].ptr = (uint8_t *)malloc(blockW * 8 * blockH * 8);
	mdecOut.planes[kOutputPlaneCb].pitch = blockW * 8;
	mdecOut.planes[kOutputPlaneCr].ptr = (uint8_t *)malloc(blockW * 8 * blockH * 8);
	mdecOut.planes[kOutputPlaneCr].pitch = blockW * 8;

	for (int y = 0; y < blockH; ++y) {
		for (int x = 0; x < blockW; ++x) {
			decodeBlock(&bs, x, y, mdecOut.planes[kOutputPlaneCr].ptr, mdecOut.planes[kOutputPlaneCr].pitch, qscale, version);
			decodeBlock(&bs, x, y, mdecOut.planes[kOutputPlaneCb].ptr, mdecOut.planes[kOutputPlaneCb].pitch, qscale, version);
			decodeBlock(&bs, x,     y,     mdecOut.planes[kOutputPlaneY].ptr, mdecOut.planes[kOutputPlaneY].pitch, qscale, version);
			decodeBlock(&bs, x + 1, y,     mdecOut.planes[kOutputPlaneY].ptr, mdecOut.planes[kOutputPlaneY].pitch, qscale, version);
			decodeBlock(&bs, x,     y + 1, mdecOut.planes[kOutputPlaneY].ptr, mdecOut.planes[kOutputPlaneY].pitch, qscale, version);
			decodeBlock(&bs, x + 1, y + 1, mdecOut.planes[kOutputPlaneY].ptr, mdecOut.planes[kOutputPlaneY].pitch, qscale, version);
		}
	}

	output(&mdecOut, userdata);

	free(mdecOut.planes[0].ptr);
	free(mdecOut.planes[1].ptr);
	free(mdecOut.planes[2].ptr);

	return 0;
}
