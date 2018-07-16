
extern "C" {
	#include <libavcodec/avcodec.h>
}
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "screenshot.h"
#include "libbs/bs.h"

// CD CD BC 00 00 00 20 33 00 38 01 00 02 00

/*
  0 . . . 2 . . little . Unknown^M
                           Number of run length codes in the frame?^M
                           Size of data (in bytes) following this header?^M
  2 . . . 2 . . little . Always 0x3800^M
  4 . . . 2 . . little . Frame's quantization scale^M
  6 . . . 2 . . little . Version of the frame^M
  8  . . . . . . . . . . Compressed macro blocks^M
                           Stream of 2 byte little-endian values^M
                           Number of macro blocks =^M
                             (width+15)/16 * (height+15)/16^M

*/

static uint16_t BSWAP_16(uint16_t n) {
	return (n >> 8) | (n << 8);
}

static uint16_t READ_LE_UINT16(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[1] << 8) | b[0];
}

inline uint32_t READ_LE_UINT32(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

static uint8_t _sectorBuf[2048];
static int _sectorBufPos;
static int _sectorBufLen;


static int fioAlignSizeTo2048(int size) {
	return ((size + 2043) / 2044) * 2048;
}

static uint32_t fioUpdateCRC(uint32_t sum, const uint8_t *buf, uint32_t size) {
	assert((size & 3) == 0);
	size >>= 2;
	while (size--) {
		sum ^= READ_LE_UINT32(buf); buf += 4;
	}
	return sum;
}

static void fioRefillBuffer(FILE *fp) {
	int size = fread(_sectorBuf, 1, 2048, fp);
	if (size == 2048) {
		uint32_t crc = fioUpdateCRC(0, _sectorBuf, 2048);
		assert(crc == 0);
		size -= 4;
	}
	_sectorBufPos = 0;
	_sectorBufLen = size;
}

static void fioSeekAlign(FILE *fp, int pos) {
	// pos += (pos / 2048) * 4;
	const int alignPos = (pos / 2048) * 2048;
	fseek(fp, alignPos, SEEK_SET);
	fioRefillBuffer(fp);
	const int skipCount = pos - alignPos;
	_sectorBufPos += skipCount;
	_sectorBufLen -= skipCount;
}

static int fioRead(FILE *fp, uint8_t *ptr, int size) {
	if (size >= _sectorBufLen) {
		const int count = fioAlignSizeTo2048(size) / 2048;
		for (int i = 0; i < count; ++i) {
			memcpy(ptr, _sectorBuf + _sectorBufPos, _sectorBufLen);
			ptr += _sectorBufLen;
			size -= _sectorBufLen;
			fioRefillBuffer(fp);
			if (_sectorBufLen == 0 || size < _sectorBufLen) {
				break;
			}
		}
	}
	if (_sectorBufLen != 0 && size != 0) {
		memcpy(ptr, _sectorBuf + _sectorBufPos, size);
		_sectorBufLen -= size;
		_sectorBufPos += size;
	}
	return 0;
}

static void fioFlush(FILE *fp) {
	const int currentPos = ftell(fp);
	assert((currentPos & 2047) == 0);
	_sectorBufLen = _sectorBufPos = 0;
}

static uint32_t yuv420_to_argb(int Y, int U, int V) {
        float R = Y + 1.402 * (V - 128);
        if (R < 0) {
                R = 0;
        } else if (R > 255) {
                R = 255;
        }
        float G = Y - 0.344 * (U - 128) - 0.714 * (V - 128);
        if (G < 0) {
                G = 0;
        } else if (G > 255) {
                G = 255;
        }
        float B = Y + 1.772 * (U - 128);
        if (B < 0) {
                B = 0;
        } else if (B > 255) {
                B = 255;
        }
        return 0xFF000000 | (((uint8_t)R) << 16) | (((uint8_t)G) << 8) | ((uint8_t)B);
}

static const int W = 256;
static const int H = 192;

static void decodeMdec(const uint8_t *src, int len, const char *fname) {
	fprintf(stdout, "MDEC len %d, VLC_ID 0x%x\n", READ_LE_UINT16(src), READ_LE_UINT16(src + 2));
	fprintf(stdout, "qscale %d version %d\n", READ_LE_UINT16(src + 4), READ_LE_UINT16(src + 6));

	const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_MDEC);

	AVCodecContext *ctx = avcodec_alloc_context3(codec);
	ctx->width  = W;
	ctx->height = H;
	avcodec_open2(ctx, codec, 0);

	AVFrame *frame = av_frame_alloc();

	AVPacket pkt;
	av_new_packet(&pkt, len);
	memcpy(pkt.data, src, len);

	int hasFrame = 0;
	int ret = avcodec_decode_video2(ctx, frame, &hasFrame, &pkt);
	if (ret < 0) {
		fprintf(stdout, "avcodec_decode_video2 ret %d\n", ret);
	} else {
		fprintf(stdout, "ret %d\n", ret);
		uint32_t *rgba = (uint32_t *)malloc(W * H * sizeof(uint32_t));
		if (rgba) {
			for (int y = 0; y < frame->height; ++y) {
				for (int x = 0; x < frame->width; ++x) {
					const int Y = *(frame->data[0] +  y * frame->linesize[0] + x);
					const int U = *(frame->data[1] + (y / 2 * frame->linesize[1] + x / 2));
					const int V = *(frame->data[2] + (y / 2 * frame->linesize[2] + x / 2));
					rgba[y * W + x] = yuv420_to_argb(Y, U, V);
				}
			}
			saveTGA(fname, (const uint8_t *)rgba, W, H);
			free(rgba);
		}
	}

	avcodec_free_context(&ctx);
	av_frame_free(&frame);
}

// rock_hod.lvl
static const uint32_t offsets[] = {
/*
	0x1a330c,
	0x29530c,
	0x3aab0c,

	0xd2230c,
	0xdecb0c,
	0xdf330c,
	0xe30b0c,
	0xe3b30c,
	0xe8f30c,
	0xe95b0c,
	0xef0610,
	0xef4b28,
*/
	0
};

static uint8_t tempBuffer[0x100000];

int main(int argc, char *argv[]) {
	avcodec_register_all();

	FILE *fp = fopen("a0.bss", "rb");
	if (fp) {
		const int count = fread(tempBuffer, 1, sizeof(tempBuffer), fp);
		fprintf(stdout, "Read %d bytes at 0x%x from 'a0'\n", count);
		decodeMdec(tempBuffer, count, "a0.tga");
		fclose(fp);
	}
	assert(argc == 2);
	fp = fopen(argv[1], "rb"); // rock_hod.lvl
	if (fp) {
		for (int i = 0; offsets[i] != 0; ++i) {
#if 0
			fseek(fp, offsets[i], SEEK_SET);
			const int count = fread(tempBuffer, 1, sizeof(tempBuffer), fp);
#else
			fioSeekAlign(fp, offsets[i]);
			const int count = W * H * 4;
			fioRead(fp, tempBuffer, count);
#endif
			fprintf(stdout, "Read %d bytes at 0x%x\n", count, offsets[i]);
			char name[16];
			snprintf(name, sizeof(name), "%08x.tga", offsets[i]);
			decodeMdec(tempBuffer, count, name);
			// int len = READ_LE_UINT16(tempBuffer) * sizeof(uint16_t) + 8;
			snprintf(name, sizeof(name), "%08x.bss", offsets[i]);
			FILE *out = fopen(name, "wb");
			if (out) {
				fwrite(tempBuffer, 1, count, out);
				fclose(out);
			}
/*
			uint8_t *rgb = (uint8_t *)malloc(W * H * 3);
			if (rgb) {
				unsigned char *iq = 0;
				bs_decode_rgb24(rgb, (bs_header_t *)tempBuffer, W, H, iq);
				snprintf(name, sizeof(name), "%08x.tga", offsets[i]);
				saveTGA(name, rgb, W, H);
				free(rgb);
			}
*/
		}

#define MAX_OFFSETS 4096
		uint32_t offsetsTable[MAX_OFFSETS];
		int offsetsCount = 0;

		// scan
		if (1) {
			fseek(fp, 0, SEEK_SET);
			do {
				const int count = fread(tempBuffer, 1, 4, fp);
				if (READ_LE_UINT16(tempBuffer + 2) == 0x3800) {
					const int size = READ_LE_UINT16(tempBuffer);
					fread(tempBuffer, 1, 4, fp);
					if (READ_LE_UINT16(tempBuffer + 2) == 2 || READ_LE_UINT16(tempBuffer + 2) == 3) {
						fprintf(stdout, "Found possible MDEC frame at 0x%lx qscale %d len %d\n", ftell(fp) - 8, READ_LE_UINT16(tempBuffer), size);
						if (size > 8192) {
							assert(offsetsCount < MAX_OFFSETS);
							offsetsTable[offsetsCount] = ftell(fp) - 8;
							++offsetsCount;
						}
					}
				}
			} while (!feof(fp));
			for (int i = 0; i < offsetsCount; ++i) {
#if 1
				fioSeekAlign(fp, offsetsTable[i]);
				const int count = W * H * 4;
				fioRead(fp, tempBuffer, count);
#else
				fseek(fp, offsetsTable[i], SEEK_SET);
				const int count = fread(tempBuffer, 1, sizeof(tempBuffer), fp);
#endif
				fprintf(stdout, "Read %d bytes at 0x%x\n", count, offsetsTable[i]);
				fioFlush(fp);
				char name[16];
				snprintf(name, sizeof(name), "%08x.tga", offsetsTable[i]);
				decodeMdec(tempBuffer, count, name);
				snprintf(name, sizeof(name), "%08x.bss", offsetsTable[i]);
				FILE *out = fopen(name, "wb");
				if (out) {
					fwrite(tempBuffer, 1, count, out);
					fclose(out);
				}
			}
		}

		fclose(fp);
	}
	return 0;
}
