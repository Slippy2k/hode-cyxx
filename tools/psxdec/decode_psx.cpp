
extern "C" {
	#include <libavcodec/avcodec.h>
}
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/param.h>

#include <cdio/cdio.h>
#include <cdio/iso9660.h>

#include "rnc.h"
#include "screenshot.h"

static uint16_t READ_LE_UINT16(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[1] << 8) | b[0];
}

inline uint32_t READ_LE_UINT32(const void *ptr) {
	const uint8_t *b = (const uint8_t *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

static uint32_t READ_BE_UINT32(const uint8_t *p) {
	return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

static const bool _fioReadCrc = true;

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

static int clipColorByte(float color) {
	if (color < 0) {
		return 0;
	} else if (color > 255) {
		return 255;
	} else {
		return (int)color;
	}
}

static uint32_t yuv420_to_argb(int Y, int U, int V) {
	const int r = clipColorByte(Y + 1.402 * (V - 128));
	const int g = clipColorByte(Y - 0.344 * (U - 128) - 0.714 * (V - 128));
	const int b = clipColorByte(Y + 1.772 * (U - 128));
	return (r << 16) | (g << 8) | b;
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

static void extractStr(CdIo_t *image, const char *directory = "") {
	CdioList_t *entries = iso9660_fs_readdir(image, directory, false);
	if (entries) {
		CdioListNode_t *node = _cdio_list_begin(entries);
		for (; node; node = _cdio_list_node_next(node)) {
			const iso9660_stat_t *st = (const iso9660_stat_t *)_cdio_list_node_data(node);
			if (st->type == iso9660_stat_t::_STAT_FILE) {
				const char *ext = strchr(st->filename, '.');
				if (ext && strcmp(ext, ".STR;1") == 0) {
					char name[MAXPATHLEN];
					snprintf(name, sizeof(name), "%s", st->filename);
					char *p = strrchr(name, ';');
					if (p) {
						*p = 0;
					}
					FILE *out = fopen(name, "wb");
					if (out) {
						uint8_t buffer[CDIO_CD_FRAMESIZE_RAW];
						for (lsn_t sector = 0; sector < st->secsize; ++sector) {
							driver_return_code_t r = cdio_read_audio_sector(image, buffer, st->lsn + sector);
							if (r != DRIVER_OP_SUCCESS) {
								fprintf(stderr, "Error reading sector %d of image file: %s\n", (st->lsn + sector), cdio_driver_errmsg(r));
								break;
							}
							fwrite(buffer, 1, sizeof(buffer), out);
						}
						fclose(out);
					}
				}
			} else if (st->type == iso9660_stat_t::_STAT_DIR) {
				if (st->filename[0] != '.') {
					extractStr(image, st->filename);
				}
			}
		}
		_cdio_list_free(entries, true);
	}
}

static uint8_t tempBuffer[0x100000];

#define TIM_HEADER_SIZE 20

static void rgb555_to_argb(const uint8_t *p, int size, uint32_t *argb) {
        assert((size & 1) == 0);
	int j = 0;
        for (int i = 0; i < size; i += 2) {
                uint16_t color = READ_LE_UINT16(p + i);

                //assert((color & 0x8000) == 0);
                int b = (color >> 10) & 31;
                int g = (color >>  5) & 31;
                int r =  color        & 31;

		b <<= 3;
		g <<= 3;
		r <<= 3;

		argb[j] = (r << 16) | (g << 8) | b;
		++j;
	}
}

static void decodeTim(FILE *fp) {
	int count = fread(tempBuffer, 1, sizeof(tempBuffer), fp);
	fprintf(stdout, "Read %d bytes from TIM\n", count);
	if (memcmp(tempBuffer, "RNC", 3) == 0) {
		const uint32_t uncompressedSize = READ_BE_UINT32(tempBuffer + 4);
		uint8_t *timBuffer = (uint8_t *)malloc(uncompressedSize);
		if (timBuffer) {
			RncDecoder decoder;
			count = decoder.unpackM2(tempBuffer, timBuffer);
			fprintf(stdout, "rnc %d %d\n", count, uncompressedSize);
			assert(READ_LE_UINT32(timBuffer) == 16);
			const int bpp = READ_LE_UINT32(timBuffer + 4);
			const int w = READ_LE_UINT16(timBuffer + 16);
			const int h = READ_LE_UINT16(timBuffer + 18);
			assert(bpp == 2);
			uint32_t *rgba = (uint32_t *)malloc(w * h * sizeof(uint32_t));
			if (rgba) {
				rgb555_to_argb(timBuffer + TIM_HEADER_SIZE, count - TIM_HEADER_SIZE, rgba);
				saveTGA("tim.tga", (const uint8_t *)rgba, w, h);
				free(rgba);
			}
			free(timBuffer);
		}
	}
}

int main(int argc, char *argv[]) {
	avcodec_register_all();

	for (int i = 1; i < argc; ++i) {
		const char *ext = strrchr(argv[i], '.');
		if (ext) {
			if (strcasecmp(ext, ".bin") == 0 || strcasecmp(ext, ".cue") == 0) {
				CdIo_t *image = cdio_open(argv[i], DRIVER_BINCUE);
				if (image) {
					extractStr(image);
					cdio_destroy(image);
				}
				continue;
			}
			if (strcasecmp(ext, ".lvl") == 0 || strcasecmp(ext, ".dax") == 0) {
				FILE *fp = fopen(argv[i], "rb");
				if (fp) {
#define MAX_OFFSETS 256
					uint32_t offsetsTable[MAX_OFFSETS];
					int offsetsCount = 0;
					// scan
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
						int count;
						if (_fioReadCrc) {
							fioSeekAlign(fp, offsetsTable[i]);
							count = W * H * 4;
							fioRead(fp, tempBuffer, count);
						} else {
							fseek(fp, offsetsTable[i], SEEK_SET);
							count = fread(tempBuffer, 1, sizeof(tempBuffer), fp);
						}
						fprintf(stdout, "Read %d bytes at 0x%x\n", count, offsetsTable[i]);
						if (_fioReadCrc) {
							fioFlush(fp);
						}
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
					fclose(fp);
				}
				continue;
			}
			if (strcasecmp(ext, ".tim") == 0) {
				FILE *fp = fopen(argv[i], "rb");
				if (fp) {
					decodeTim(fp);
					fclose(fp);
				}
				continue;
			}
		}
	}
	return 0;
}
