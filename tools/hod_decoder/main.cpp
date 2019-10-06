
#include <queue>
#include <zlib.h>
#include "fileio.h"
#include "lzw.h"
#include "screenshot.h"
#include "wav.h"

#define SECTOR_FILE (1 << 0)
#define PSX         (1 << 1)

enum {
	kDat,
	kDax,
	kLvl,
	kSss,
	kMst,
};

static const struct {
	const char *name;
	int type;
	uint32_t crc;
	uint32_t flags;
} _files[] = {
	{ "setup.dat",    kDat, 0x9c99c981, SECTOR_FILE }, // hod_demo-webfr1_2.exe
	{ "setup.dat",    kDat, 0x9ebe2677, SECTOR_FILE }, // hod_demo-weben1_2.exe
	{ "setup.dat",    kDat, 0xb60fe7af, SECTOR_FILE }, // Coca Cola edition 1.4
	{ "rock_hod.lvl", kLvl, 0x7e50e77d, SECTOR_FILE },
	{ "rock_hod.lvl", kLvl, 0x7e37bbdd, SECTOR_FILE | PSX }, // SLED_013.51, SLUS_006.96
	{ "rock_hod.sss", kSss, 0x69682a22, SECTOR_FILE }, // demo v1.2
	{ "rock_hod.sss", kSss, 0xc50c13bb, SECTOR_FILE }, // demo v1.4
	{ "fort_hod.lvl", kLvl, 0x3e6aebec, SECTOR_FILE },
	{ "fort_hod.lvl", kLvl, 0xcaf0e23c, SECTOR_FILE | PSX },
	{ "fort_hod.sss", kSss, 0x6ad13bbb, SECTOR_FILE },
	{ "pwr1_hod.lvl", kLvl, 0x8add9bce, SECTOR_FILE | PSX },
	{ "isld_hod.lvl", kLvl, 0xcfe30753, SECTOR_FILE | PSX },
	{ "lava_hod.lvl", kLvl, 0x4e494f1c, SECTOR_FILE | PSX },
	{ "pwr2_hod.lvl", kLvl, 0xcd8652be, SECTOR_FILE | PSX },
	{ "lar1_hod.lvl", kLvl, 0x8968efb8, SECTOR_FILE | PSX },
	{ "lar2_hod.lvl", kLvl, 0x449c8dd4, SECTOR_FILE | PSX },
	{ "dark_hod.lvl", kLvl, 0x79c01b55, SECTOR_FILE | PSX },
	{ 0, -1, 0, 0 }
};

static const struct {
	const char *name;
	int type;
} _exts[] = {
	{ ".dat", kDat },
	{ ".dax", kDax }, // PSX is version 11 but with different binary layout and compression
	{ ".lvl", kLvl },
	{ ".sss", kSss },
	{ ".mst", kMst },
	{ 0, -1 }
};

static const struct {
	const int num;
	uint32_t crc;
} _saturn[] = {
	{ 0, 0x28e6813c }, // executable code
	{ 1, 0xb749d5c5 }, // level structures
	{ 2, 0xa97b41a8 }, // level graphics
	{ 3, 0xadf6d56c }, // sound
	{ -1, 0 }
};

static const char *_levels[] = {
	"rock_hod",
	"fort_hod",
	"pwr1_hod",
	"isld_hod",
	"lava_hod",
	"pwr2_hod",
	"lar1_hod",
	"lar2_hod",
	"dark_hod",
	0
};

static const int kMaxScreens = 40;

static bool _isPsx;

static uint8_t _bitmapBuffer[256 * 256]; // PSX compressed data can be larger than 256x192 bytes
static uint8_t _bitmapPalette[256 * 3];
static uint8_t _spritePalette[256 * 3];
static uint8_t _controlsPalette[256 * 3];

static void DecodeSss(File *fp, uint32_t baseOffset = 0);

static bool isMdecData(const uint8_t *p) {
	return READ_LE_UINT16(p + 2) == 0x3800 && (READ_LE_UINT16(p + 6) == 2 || READ_LE_UINT16(p + 6) == 3);
}

static void ConvertVgaPalette(uint8_t *p, int colors = 256) {
	// convert vga palette colors from 6 bits to 8 bits
	for (int i = 0; i < colors * 3; ++i) {
		p[i] = (p[i] << 2) | ((p[i] >> 4) & 3);
	}
}

enum {
	kPosTopScreen    = 0,
	kPosRightScreen  = 1,
	kPosBottomScreen = 2,
	kPosLeftScreen   = 3
};

struct LevelScreen {
	uint8_t num;
	int x, y;
};

static const int kLevelMapMaxW = kMaxScreens * 2;
static const int kLevelMapMaxH = kMaxScreens * 2;

static void GenerateLevelMap(const uint8_t *data, int count) {
	bool visited[kMaxScreens];
	for (int i = 0; i < count; ++i) {
		visited[i] = false;
	}

	std::queue<LevelScreen> q;

	uint8_t levelMap[kLevelMapMaxH][kLevelMapMaxW];
	memset(levelMap, 255, sizeof(levelMap));

	int xmin, xmax;
	int ymin, ymax;

	LevelScreen ls;
	ls.num = 0;
	ls.x = xmin = xmax = kLevelMapMaxW / 2;
	ls.y = ymin = ymax = kLevelMapMaxH / 2;
	q.push(ls);

	while (!q.empty()) {
		const int screenNum = q.front().num;
		const int x = q.front().x;
		const int y = q.front().y;
		q.pop();
		if (visited[screenNum]) {
			continue;
		}
		if (x < xmin) {
			xmin = x;
		} else if (x > xmax) {
			xmax = x;
		}
		if (y < ymin) {
			ymin = y;
		} else if (y > ymax) {
			ymax = y;
		}
		levelMap[y][x] = screenNum;
		visited[screenNum] = true;
		const int top    = data[screenNum * 4 + kPosTopScreen];
		const int right  = data[screenNum * 4 + kPosRightScreen];
		const int bottom = data[screenNum * 4 + kPosBottomScreen];
		const int left   = data[screenNum * 4 + kPosLeftScreen];
		if (top != 255) {
			assert(y >= 0);
			ls.num = top;
			ls.x = x;
			ls.y = y - 1;
			q.push(ls);
		}
		if (bottom != 255) {
			assert(y < kLevelMapMaxH);
			ls.num = bottom;
			ls.x = x;
			ls.y = y + 1;
			q.push(ls);
		}
		if (left != 255) {
			assert(x >= 0);
			ls.num = left;
			ls.x = x - 1;
			ls.y = y;
			q.push(ls);
		}
		if (right != 255) {
			assert(x < kLevelMapMaxW);
			ls.num = right;
			ls.x = x + 1;
			ls.y = y;
			q.push(ls);
		}
	}
	fprintf(stdout, "levelMap %d %d\n", (ymax - ymin + 1), (xmax - xmin + 1));
	for (int y = ymin; y <= ymax; ++y) {
		for (int x = xmin; x <= xmax; ++x) {
			if (levelMap[y][x] == 255) {
				fprintf(stdout, "    ");
			} else {
				fprintf(stdout, " %02x ", levelMap[y][x]);
			}
		}
		fprintf(stdout, "\n");
	}
}

static void DecodeRLE(const uint8_t *src, uint8_t *dst, int dstPitch) {
	uint8_t *start = dst;
	while (1) {
		int code = *src++;
		int count = code & 0x3F;
		switch (code >> 6) {
		case 0:
			memcpy(dst, src, count);
			dst += count;
			src += count;
			break;
		case 1:
			code = *src++;
			memset(dst, code, count);
			dst += count;
			break;
		case 2:
			if (count == 0) {
				count = *src++;
			}
			dst += count;
			break;
		case 3:
			if (count == 0) {
				count = *src++;
				if (count == 0) {
					return;
				}
			}
			start += dstPitch * count;
			code = *src++;
			dst = start + code;
			break;
		}
	}
}

static void DecodeLvlBackgroundBitmap(const uint8_t *header, const uint8_t *data, uint32_t dataSize, int num) {
	header += 16;
	const uint8_t *paletteData[4];
	for (int i = 0; i < 4; ++i) {
		const uint32_t offset = READ_LE_UINT32(header); header += 4;
		paletteData[i] = (offset != 0) ? data + offset : 0;
	}
	const uint8_t *bitmapData[4];
	for (int i = 0; i < 4; ++i) {
		const uint32_t offset = READ_LE_UINT32(header); header += 4;
		bitmapData[i] = (offset != 0) ? data + offset : 0;
	}

	char filename[64];

	for (int i = 0; i < 4; ++i) {
		if (_isPsx) {
			if (bitmapData[i] && isMdecData(bitmapData[i])) {
				snprintf(filename, sizeof(filename), "lvl_screen_%02d_state_%d.jpg", num, i);
				savePSX(filename, bitmapData[i], dataSize - (bitmapData[i] - data), 256, 192);
			}
			continue;
		}
		if (bitmapData[i] && paletteData[i]) {
			const int uncompressedSize = decodeLZW(bitmapData[i] + 2, _bitmapBuffer);
			assert(uncompressedSize == 256 * 192);
			snprintf(filename, sizeof(filename), "lvl_screen_%02d_state_%d.bmp", num, i);
			saveBMP(filename, _bitmapBuffer, paletteData[i] + 2, 256, 192);
		}
	}

	if (_isPsx) {
		return;
	}

	header += 4 * sizeof(uint32_t) * 2;
	for (int i = 0; i < 4; ++i) {
		const uint32_t offset = READ_LE_UINT32(header); header += 4;
		if (offset != 0) {
			const uint8_t *p = data + offset;
			p += 2; // sound
			const uint16_t size = READ_LE_UINT16(p + 2); // size
			if (size > 8) {
				const int x = p[0];
				const int y = p[1];
				const int w = READ_LE_UINT16(p + 4);
				const int h = READ_LE_UINT16(p + 6);
				memset(_bitmapBuffer, 0, sizeof(_bitmapBuffer));
				DecodeRLE(p + 8, _bitmapBuffer, w);

				snprintf(filename, sizeof(filename), "lvl_screen_%02d_static_%d_x_%03d_y_%03d.bmp", num, i, x, y);
				saveBMP(filename, _bitmapBuffer, paletteData[0] + 2, w, h);
			}
		}
	}

	// keep a copy of the first screen palette for sprites
	if (num == 0) {
		assert(paletteData[0]);
		memcpy(_spritePalette, paletteData[0] + 2, 256 * 3);
	}
}

static void DecodeLvlSprite(const uint8_t *data, int num) {
	const int spriteNum = data[1];
	const int framesCount = READ_LE_UINT16(data + 2);
	const uint32_t framesDataOffset = READ_LE_UINT32(data + 0x1C);

	int bitmapW = 0;
	int bitmapH = 0;
	const uint8_t *framesData = data + framesDataOffset;
	for (int i = 0; i < framesCount; ++i) {
		const uint16_t size = READ_LE_UINT16(framesData);
		const int w = READ_LE_UINT16(framesData + 2);
		const int h = READ_LE_UINT16(framesData + 4);
		bitmapW += w;
		if (bitmapH < h) {
			bitmapH = h;
		}
		framesData += size;
	}

	uint8_t *buffer = (uint8_t *)calloc(bitmapW * bitmapH, 1);
	if (!buffer) {
		fprintf(stderr, "Failed to allocate %d bytes\n", bitmapW * bitmapH);
		return;
	}

	framesData = data + framesDataOffset;
	int xOffset = 0;
	for (int i = 0; i < framesCount; ++i) {
		const uint16_t size = READ_LE_UINT16(framesData);
		const int w = READ_LE_UINT16(framesData + 2);
		DecodeRLE(framesData + 6, buffer + xOffset, bitmapW);
		xOffset += w;
		framesData += size;
	}

	_spritePalette[0] = 255;
	_spritePalette[1] = 0;
	_spritePalette[2] = 255;

	char name[64];
	snprintf(name, sizeof(name), "hod_spr_%02d_%02d.bmp", num, spriteNum);
	saveBMP(name, buffer, _spritePalette, bitmapW, bitmapH);

	free(buffer);
}

static void DecodeLvl(File *fp, int levelNum) {

	const uint32_t tag = fp->readUint32();
	assert(tag == 0x484F4400);

	const int screensCount = fp->readByte();
	fp->readByte();
	fp->readByte();
	const int spritesCount = fp->readByte();

	// level map
	uint8_t gridData[kMaxScreens * 4];
	assert(screensCount <= kMaxScreens);
	fp->read(gridData, screensCount * 4);
	GenerateLevelMap(gridData, screensCount);

	// screen masks (shadows, grids)
	static const uint32_t kMaskOffsets = 0x4708;
	fp->seekAlign(kMaskOffsets);
	const uint32_t masksOffset = fp->readUint32();
	const uint32_t masksSize = fp->readUint32();

	// .sss data is embedded in .lvl on PSX
	const uint32_t sssOffset = masksOffset + fioAlignSizeTo2048(masksSize);
	if (_isPsx) {
		assert((sssOffset & 0x7FF) == 0);
		fp->seek(sssOffset, SEEK_SET);
		DecodeSss(fp, sssOffset);
	}

	static const uint32_t kSpritesOffset = 0x2988;
	static const uint32_t kBackgroundsOffset = kSpritesOffset + 32 * 16;

	// background screens
	for (int i = 0; i < screensCount; ++i) {
		fp->seekAlign(kBackgroundsOffset + i * 16);
		const uint32_t offset = fp->readUint32();
		const uint32_t size = fp->readUint32();
		const uint32_t readSize = fp->readUint32();

		uint8_t *ptr = (uint8_t *)calloc(size, 1);
		if (!ptr) {
			fprintf(stderr, "Failed to allocate %d bytes", size);
			continue;
		}
		if (_isPsx) {
			fp->seek(offset + sssOffset, SEEK_SET);
			fp->readUint32();
		} else {
			fp->seek(offset, SEEK_SET);
		}
		fp->read(ptr, readSize);

		fp->seekAlign(kBackgroundsOffset + kMaxScreens * 16 + i * 160);
		uint8_t buffer[160];
		fp->read(buffer, sizeof(buffer));

		DecodeLvlBackgroundBitmap(buffer, ptr, readSize, i);

		free(ptr);
	}

	if (_isPsx) {
		return;
	}

	// sprites
	for (int i = 0; i < spritesCount; ++i) {
		fp->seekAlign(kSpritesOffset + i * 16);
		const uint32_t offset = fp->readUint32();
		const uint32_t size = fp->readUint32();
		const uint32_t readSize = fp->readUint32();
		uint8_t *ptr = (uint8_t *)calloc(size, 1);
		if (!ptr) {
			fprintf(stderr, "Failed to allocate %d bytes", size);
			continue;
		}
		fp->seek(offset, SEEK_SET);
		fp->read(ptr, readSize);

		DecodeLvlSprite(ptr, i);

		free(ptr);
	}
}

static void CheckSssCode(const uint8_t *p, int size) {
	static const uint8_t opcodesLength[] = {
		4, 0, 4, 0, 4, 12, 8, 0, 16, 4, 4, 4, 4, 8, 12, 0, 4, 8, 8, 4, 4, 4, 8, 4, 8, 4, 8, 16, 8, 4
	};
	int offset = 0;
	while (offset < size) {
		const int len = opcodesLength[p[offset]];
		if (len == 0) {
			fprintf(stderr, "Invalid .sss opcode %d\n", p[offset]);
			break;
		}
		offset += len;
	}
	assert(offset == size);
}

void DecodeSssCode(const uint8_t *codeData, uint32_t codeSize);

static void DecodeSssPcm(File *fp, uint32_t count, uint32_t stride, int16_t *ptr) {
	for (uint32_t i = 0; i < count; ++i) {
		int16_t lut[256];
		for (uint32_t j = 0; j < 256; ++j) {
			lut[j] = fp->readUint16();
		}
		for (uint32_t j = 256 * sizeof(int16_t); j < stride; ++j) {
			*ptr++ = lut[fp->readByte()];
		}
	}
}

static void DecodeSssAdpcmNibble(uint8_t nibble, int *pred) {
	const int sign  = nibble & 8;
	const int delta = nibble & 7;
	const int diff  = ((2 * delta + 1) * 511) >> 3;
	const int npred = *pred + (sign ? -diff : diff);
	if (npred < -32768) {
		*pred = -32768;
	} else if (npred > 32767) {
		*pred = 32767;
	} else {
		*pred = npred;
	}
}

static void DecodeSssAdpcm(const uint8_t *src, int size, int16_t *data) {
	int pred = READ_LE_UINT16(src); src += 2;
	*data++ = pred;
	for (int i = 2; i < size; ++i) {
		DecodeSssAdpcmNibble(src[i] >> 4, &pred);
		*data++ = pred;
		DecodeSssAdpcmNibble(src[i] & 15, &pred);
		*data++ = pred;
	}
}

static void DecodeSss(File *fp, uint32_t baseOffset) {

	const int version = fp->readUint32();
	assert(version == 6 || version == 10 || version == 12);

	fp->readUint32(); // 0x4
	fp->readUint32(); // 0x8

	const int data4Count = fp->readUint32(); // 0xC
	const int data1Count = fp->readUint32();
	const int data2Count = fp->readUint32();
	const int data3Count = fp->readUint32();
	const int infosCount = fp->readUint32();
	const int codeSize   = fp->readUint32();
	const int preload1Count = (version == 10 || version == 12) ? fp->readUint32() : 0; // pcm
	const int preload2Count = (version == 10 || version == 12) ? fp->readUint32() : 0; // sprites
	const int preload3Count = (version == 10 || version == 12) ? fp->readUint32() : 0; // mst
	const int pcmCount   = fp->readUint32();

	fp->seek(baseOffset + 2048, SEEK_SET);

	fp->seek(data1Count * 8,  SEEK_CUR);
	fp->seek(data2Count * 4,  SEEK_CUR);
	fp->seek(data3Count * 8,  SEEK_CUR);
	fp->seek(infosCount * 24, SEEK_CUR);

	uint8_t *codeData = (uint8_t *)malloc(codeSize);
	if (codeData) {
		fp->read(codeData, codeSize);
		CheckSssCode(codeData, codeSize);
		DecodeSssCode(codeData, codeSize);
		free(codeData);
	}

	if (version == 10 || version == 12) {

		fp->seek(preload1Count * 4, SEEK_CUR);
		fp->seek(preload2Count * 4, SEEK_CUR);
		fp->seek(preload3Count * 4, SEEK_CUR);
		for (int i = 0; i < preload1Count; ++i) {
			const int len = (version == 12) ? fp->readUint16() * 2 : fp->readByte();
			fp->seek(len, SEEK_CUR);
		}
		for (int i = 0; i < preload2Count; ++i) {
			const uint8_t len = fp->readByte();
			fp->seek(len, SEEK_CUR);
		}
		for (int i = 0; i < preload3Count; ++i) {
			const uint8_t len = fp->readByte();
			fp->seek(len, SEEK_CUR);
		}
		const uint8_t len = fp->readByte();
		fp->seek(len, SEEK_CUR);

		assert(data4Count < 48);
		uint32_t counts[48];
		for (int i = 0; i < data4Count; ++i) {
			counts[i] = fp->readUint32();
			fp->readUint32();
		}

		static const int kSizeOfData4 = 32;
		for (int i = 0; i < data4Count; ++i) {
			const int count = counts[i];
			assert(count < 32);
			uint8_t buffer[kSizeOfData4 * 32];
			fp->read(buffer, kSizeOfData4 * count);

			int total = 0;
			for (int j = 0; j < count; ++j) {
				const int len = READ_LE_UINT32(buffer + j * kSizeOfData4 + 0x1C) * 4;
				total += len;
			}
			fp->seek(total, SEEK_CUR);
		}

	} else if (version == 6) {

		assert(data4Count < 48);
		uint32_t counts[48];
		for (int i = 0; i < data4Count; ++i) {
			counts[i] = fp->readUint32();
			fp->readUint32();
		}

		static const int kSizeOfData4 = 68;
		for (int i = 0; i < data4Count; ++i) {
			const int count = counts[i];
			assert(count < 32);
			uint8_t buffer[kSizeOfData4 * 32];
			fp->read(buffer, kSizeOfData4 * count);

			int total = 0;
			for (int j = 0; j < count; ++j) {
				static const int8_t offsets[] = { 0x2C, 0x30, 0x34, 0x04, 0x08, 0x0C, 0x10, 0x14, -1 };
				static const int8_t lengths[] = {    2,    1,    1,    2,    2,    1,    1,    1, -1 };

				for (int k = 0; offsets[k] != -1; ++k) {
					int len = READ_LE_UINT32(buffer + j * kSizeOfData4 + offsets[k]);
					len *= lengths[k];
					total += (len + 3) & ~3;
				}
			}
			fp->seek(total, SEEK_CUR);
		}
	}

	static const int kSizeOfPcm = 20;
	assert(pcmCount < 256);
	uint8_t header[kSizeOfPcm * 256];
	fp->read(header, sizeof(header));

	for (int i = 0; i < pcmCount; ++i) {
		const uint32_t offset = READ_LE_UINT32(header + i * kSizeOfPcm + 4);
		const uint32_t size   = READ_LE_UINT32(header + i * kSizeOfPcm + 8);
		const uint32_t stride = READ_LE_UINT32(header + i * kSizeOfPcm + 12);
		const uint16_t count  = READ_LE_UINT16(header + i * kSizeOfPcm + 16);
		const uint16_t flags  = READ_LE_UINT16(header + i * kSizeOfPcm + 18);

		if (baseOffset != 0) {
			// offsets are all equal to '0x2800' in setup.dat .sss data
			// seek on first entry and read PCM data sequentially
			if (i == 0) {
				fp->seek(baseOffset + offset, SEEK_SET);
			}
		}

		if (size != 0) {

			const int channels = ((flags & 1) != 0) ? 2 : 1;

			assert(stride * count == size);
			assert(size % stride == 0);

			if (baseOffset == 0) {
				fp->seek(offset, SEEK_SET);
			}

			char filename[64];

			if (_isPsx) {
				assert(stride == 512);

				uint8_t *samples = (uint8_t *)malloc(size);
				if (samples) {
					fp->read(samples, size);

					snprintf(filename, sizeof(filename), "hod_%03d.raw", i);
					fioDumpData(filename, samples, size);

					free(samples);
				}
				continue;
			}

			assert(stride == 2276 || stride == 4040);
			const uint32_t decompressedSize = (stride - 256 * sizeof(int16_t)) * count * sizeof(int16_t);
			uint8_t *samples = (uint8_t *)malloc(decompressedSize);
			if (samples) {

				DecodeSssPcm(fp, count, stride, (int16_t *)samples);

				snprintf(filename, sizeof(filename), "hod_%03d.wav", i);
				saveWAV(filename, 22050, 16, channels, samples, decompressedSize);

				free(samples);
			}
		}
	}
}

void DecodeMstCode(const uint8_t *codeData, uint32_t codeSize);

static void DecodeMst(File *fp) {

	const int version  = fp->readUint32();
	assert(version == 160);

	const int dataSize = fp->readUint32();
	fp->seek(29 * sizeof(uint32_t), SEEK_CUR);
	const int codeSize = fp->readUint32() * 4;

	uint8_t *codeData = (uint8_t *)malloc(codeSize);
	if (codeData) {
		const uint32_t offset = 2048 + dataSize - codeSize;
		fp->seekAlign(offset);
		fp->read(codeData, codeSize);
		DecodeMstCode(codeData, codeSize);
		free(codeData);
	}
}

enum {
	kSprLoadingAnimation,
	kSprTitleButtons,
	kSprAssignPlayer,
	kSprControls,
	kSprMenuButtons
};

static uint32_t DecodeSetupDatSprite(const uint8_t *ptr, int spriteGroup, int spriteNum) {

	const int compressedSize = READ_LE_UINT16(ptr + 2);

	if (_isPsx) {
		assert(READ_LE_UINT16(ptr + 4) == 1);
		assert(READ_LE_UINT16(ptr + 6) == 0);
		assert(ptr[0] == ptr[8] && ptr[1] == ptr[9]);
		const int size = READ_LE_UINT16(ptr + 10);
		assert(size == compressedSize - 6);

		const int w = ptr[12] * 16;
		const int h = ptr[13] * 16;

		if (isMdecData(ptr + 16)) {
			char filename[64];
			snprintf(filename, sizeof(filename), "setup_spr_%02d_%02d.jpg", spriteGroup, spriteNum);
			savePSX(filename, ptr + 16, size - 4, w, h);
		} else {
			assert(READ_LE_UINT32(ptr + 16) == 0x3020100 && size == 1144);
		}

		return compressedSize + 2;
	}

	const int w = READ_LE_UINT16(ptr + 4);
	const int h = READ_LE_UINT16(ptr + 6);

	if (w != 0 && h != 0) {

		const int x = ptr[0];
		const int y = ptr[1];

		memset(_bitmapBuffer, 0, sizeof(_bitmapBuffer));
		DecodeRLE(ptr + 8, _bitmapBuffer, w);

		const uint8_t *palette = (spriteGroup == kSprControls) ? _controlsPalette : _spritePalette;

		char name[64];
		snprintf(name, sizeof(name), "setup_spr_%02d_%02d_x_%03d_y_%03d.bmp", spriteGroup, spriteNum, x, y);
		saveBMP(name, _bitmapBuffer, palette, w, h);
	}

	return compressedSize + 2;
}

static uint32_t DecodeSetupDatSpritesGroup(const uint8_t *ptr, int spriteGroup) {

	const int size  = READ_LE_UINT32(ptr + 8);
	const int count = READ_LE_UINT16(ptr + 12);
	ptr += 16;

	int ptrOffset = 0;
	for (int i = 0; i < count; ++i) {
		ptrOffset += DecodeSetupDatSprite(ptr + ptrOffset, spriteGroup, i);
	}

	assert(size == ptrOffset);

	return 16 + size;
}

static uint32_t DecodeSetupDatBitmap256x192(const char *name, const uint8_t *ptr, int compressedSize, uint8_t *palette) {

	char filename[64];

	if (isMdecData(ptr)) {
		snprintf(filename, sizeof(filename), "%s.jpg", name);
		savePSX(filename, ptr, compressedSize, 256, 192);
		return compressedSize;
	} else if (READ_LE_UINT32(ptr) == 0x06000003 && compressedSize == 300) {
		return compressedSize;
	}

	const int uncompressedSize = decodeLZW(ptr, _bitmapBuffer);
	assert(uncompressedSize == 256 * 192);
	memcpy(palette, ptr + compressedSize, 768);
	ConvertVgaPalette(palette);

	snprintf(filename, sizeof(filename), "%s.bmp", name);
	saveBMP(filename, _bitmapBuffer, palette, 256, 192);

	snprintf(filename, sizeof(filename), "%s.tiff", name);
	saveLZW(filename, ptr, compressedSize, palette, 256, 192);

	return compressedSize + 768;
}

static void DecodeSetupDatFont(uint8_t *src, int size, uint8_t *dst) {

	assert(size == 16 * 16 * 64);

	int offset = 0;
	for (int i = 0; i < 64; ++i) {
		const uint8_t chr = i;
		const uint8_t *p = src + ((chr & 15) + (chr >> 4) * 256) * 16;
		for (int y = 0; y < 16; ++y) {
			memcpy(dst + offset, p, 16);
			p += 16 * 16;
			offset += 16;
		}
	}
	for (int i = 0; i < 256; ++i) {
		memset(_bitmapPalette + 3 * i, i, 3);
	}
	saveBMP("font.bmp", dst, _bitmapPalette, 16, 16 * 64);
}

static uint32_t DecodeSetupDatBitmapGroup(const char *name, int count, const uint8_t *header, const uint8_t *p) {

	int offset = 0;

	for (int i = 0; i < count; ++i) {
		const int w      = header[0];
		const int h      = header[1];
		const int colors = header[2];
		const int size   = READ_LE_UINT32(header + 4);
		header += 12;

		char filename[64];
		snprintf(filename, sizeof(filename), name, i);

		if (isMdecData(p)) {
			strcat(filename, ".jpg");
			savePSX(filename, p + offset, size, w, h);
			offset += size;
			continue;
		}

		const uint8_t *palette = p + offset + w * h;
		memcpy(_bitmapPalette + colors * 3, palette, 50 * 3);
		ConvertVgaPalette(_bitmapPalette + colors * 3, 50);

		strcat(filename, ".bmp");
		saveBMP(filename, p + offset, _bitmapPalette, w, h);

		offset += w * h + 768;
	}

	return offset;
}

static void DecodeSetupDat(File *fp) {

	uint8_t buffer[2044];
	fp->read(buffer, sizeof(buffer));

	const int version = READ_LE_UINT32(buffer);
	assert(version == 10 || version == 11);

	const int bufferSize0  = READ_LE_UINT32(buffer + 0x04);
	const int bufferSize1  = READ_LE_UINT32(buffer + 0x08);
	const int sssOffset    = READ_LE_UINT32(buffer + 0x0C);
	const int icons        = READ_LE_UINT32(buffer + 0x10);
	const int menus        = READ_LE_UINT32(buffer + 0x14);
	const int cutscenes    = READ_LE_UINT32(buffer + 0x18);
	const int levels       = READ_LE_UINT32(buffer + 0x1C);
	// checkpoints
	const int yesNoQuit    = READ_LE_UINT32(buffer + 0x40);
	const int unk0x44      = READ_LE_UINT32(buffer + 0x44);
	const int bufferSize2  = READ_LE_UINT32(buffer + 0x48);

	// hint screens
	const int hintsCount = (version == 11) ? 46 : 20;
	for (int i = 0; i < hintsCount; ++i) {
		const uint32_t offset = READ_LE_UINT32(buffer + 0x4C + i * 4);
		const uint32_t size = READ_LE_UINT32(buffer + 0x4C + (hintsCount + i) * 4);
		if (size != 0) {
			fp->seek(offset, SEEK_SET);
			fp->read(_bitmapBuffer, size);
			fp->flush();

			char filename[64];

			if (_isPsx) {
				if (isMdecData(_bitmapBuffer)) {
					snprintf(filename, sizeof(filename), "hint_%02d.jpg", i);
					savePSX(filename, _bitmapBuffer, size, 256, 192);
				}
				continue;
			}

			// PC
			if (i <= yesNoQuit) {
				fp->read(_bitmapPalette, 256 * 3);
				ConvertVgaPalette(_bitmapPalette);
			}
			snprintf(filename, sizeof(filename), "hint_%02d.bmp", i);
			saveBMP(filename, _bitmapBuffer, _bitmapPalette, 256, 192);
		}
	}

	// sounds
	assert((sssOffset & 0x7FF) == 0);
	fp->seek(sssOffset, SEEK_SET);
	DecodeSss(fp, sssOffset);

	// graphics
	fp->seek(2048, SEEK_SET);
	uint8_t *ptr = (uint8_t *)malloc(bufferSize2);
	if (ptr) {
		fp->read(ptr, bufferSize2);

		int ptrOffset = 0;

		if (!_isPsx) {

			// loading screen
			const uint32_t compressedSize = READ_LE_UINT32(ptr); ptrOffset += 8;
			ptrOffset += DecodeSetupDatBitmap256x192("loading", ptr + ptrOffset, compressedSize, _spritePalette);

			// loading animation
			ptrOffset += DecodeSetupDatSpritesGroup(ptr + ptrOffset, kSprLoadingAnimation);
		}

		// font
		const uint32_t fontSize = READ_LE_UINT32(ptr + ptrOffset); ptrOffset += 4;
		if (version == 11) {
			const uint32_t uncompressedSize = decodeLZW(ptr + ptrOffset, _bitmapBuffer);
			assert(uncompressedSize == fontSize);
			DecodeSetupDatFont(_bitmapBuffer, uncompressedSize, _bitmapBuffer + uncompressedSize);
		} else {
			DecodeSetupDatFont(ptr + ptrOffset, fontSize, _bitmapBuffer);
		}

		free(ptr);
	}

	const int baseOffset = READ_LE_UINT32(buffer + 0x4C + (2 + yesNoQuit) * 4);
	fp->seek(baseOffset, SEEK_SET);

	static const int options = 19;

	ptr = (uint8_t *)malloc(bufferSize1);
	if (ptr) {
		fp->read(ptr, bufferSize1);

		int ptrOffset = 0;

		static const char *checkpoints[] = {
			"checkpoints_level1_%02d",
			"checkpoints_level2_%02d",
			"checkpoints_level3_%02d",
			"checkpoints_level4_%02d",
			"checkpoints_level5_%02d",
			"checkpoints_level6_%02d",
			"checkpoints_level7_%02d",
			"checkpoints_level8_%02d",
		};

		if (version == 10) {

			uint32_t compressedSize, hdrOffset;

			const uint32_t sprTitleButtonsOffset = ptrOffset;
			ptrOffset += READ_LE_UINT32(ptr + ptrOffset + 8) + 16;

			const uint32_t sprAssignPlayerOffset = ptrOffset;
			ptrOffset += READ_LE_UINT32(ptr + ptrOffset + 8) + 16;

			compressedSize = READ_LE_UINT32(ptr + ptrOffset); ptrOffset += 8;
			ptrOffset += DecodeSetupDatBitmap256x192("title", ptr + ptrOffset, compressedSize, _spritePalette);
			DecodeSetupDatSpritesGroup(ptr + sprTitleButtonsOffset, kSprTitleButtons);

			compressedSize = READ_LE_UINT32(ptr + ptrOffset); ptrOffset += 8;
			ptrOffset += DecodeSetupDatBitmap256x192("player", ptr + ptrOffset, compressedSize, _spritePalette);
			DecodeSetupDatSpritesGroup(ptr + sprAssignPlayerOffset, kSprAssignPlayer);

			const int size = READ_LE_UINT32(ptr + ptrOffset); ptrOffset += 4; // 16x10 tiles
			assert((size % (16 * 10)) == 0);
			ptrOffset += size;

			hdrOffset = ptrOffset;
			ptrOffset += cutscenes * 12;
			ptrOffset += DecodeSetupDatBitmapGroup("cutscene_%02d", cutscenes, ptr + hdrOffset, ptr + ptrOffset);

			for (int i = 0; i < 8; ++i) {
				const int count = READ_LE_UINT32(buffer + 0x20 + i * 4); // checkpoints
				hdrOffset = ptrOffset;
				ptrOffset += count * 12;
				ptrOffset += DecodeSetupDatBitmapGroup(checkpoints[i], count, ptr + hdrOffset, ptr + ptrOffset);
			}

		} else if (version == 11) {

			ptrOffset = 4 + (2 + options) * 8;
			ptrOffset += cutscenes * 12;
			for (int i = 0; i < 8; ++i) {
				const int count = READ_LE_UINT32(buffer + 0x20 + i * 4); // checkpoints
				ptrOffset += count * 12;
			}
			ptrOffset += levels * 12;

			static const char *names[] = { "title", "player", 0 };
			for (int i = 0; names[i]; ++i) {
				const int compressedSize = READ_LE_UINT32(ptr + 4 + i * 8);
				ptrOffset += DecodeSetupDatBitmap256x192(names[i], ptr + ptrOffset, compressedSize, _bitmapPalette);
			}

			for (int i = 0; i < options; ++i) {
				const int compressedSize = READ_LE_UINT32(ptr + 20 + i * 8);
				if (compressedSize != 0) {
					char name[64];
					snprintf(name, sizeof(name), "options_%02d", i);
					ptrOffset += DecodeSetupDatBitmap256x192(name, ptr + ptrOffset, compressedSize, (i == 11) ? _controlsPalette : _bitmapPalette);
				}
			}

			int hdrOffset = 20 + options * 8;
			ptrOffset += DecodeSetupDatBitmapGroup("cutscene_%02d", cutscenes, ptr + hdrOffset, ptr + ptrOffset);
			hdrOffset += cutscenes * 12;

			for (int i = 0; i < 8; ++i) {
				const int count = READ_LE_UINT32(buffer + 0x20 + i * 4); // checkpoints
				ptrOffset += DecodeSetupDatBitmapGroup(checkpoints[i], count, ptr + hdrOffset, ptr + ptrOffset);
				hdrOffset += count * 12;
			}

			ptrOffset += DecodeSetupDatBitmapGroup("levels_%d", levels, ptr + hdrOffset, ptr + ptrOffset);
		}

		free(ptr);
	}

	if (version == 11) {
		fp->seek(baseOffset + fioAlignSizeTo2048(bufferSize1), SEEK_SET); // align to the next 2048 sector
	}

	ptr = (uint8_t *)malloc(bufferSize0);
	if (ptr) {
		fp->read(ptr, bufferSize0);

		int ptrOffset = 0;
		int hdrOffset = 0;

		if (version == 11) {

			ptrOffset += DecodeSetupDatSpritesGroup(ptr + ptrOffset, kSprTitleButtons);

			ptrOffset += DecodeSetupDatSpritesGroup(ptr + ptrOffset, kSprAssignPlayer);

			ptrOffset += menus * 8; // menu data

			const int size = READ_LE_UINT32(ptr + ptrOffset); ptrOffset += 4; // 16x10 tiles
			assert((size % (16 * 10)) == 0);
			ptrOffset += size;

			ptrOffset += unk0x44;
		}

		if (_isPsx) {
			return;
		}

		hdrOffset = ptrOffset;
		ptrOffset += icons * 16;
		for (int i = 0; i < icons; ++i) {
			const int count = READ_LE_UINT16(ptr + hdrOffset + i * 16 + 12);
			for (int i = 0; i < count; ++i) {
				ptrOffset += DecodeSetupDatSprite(ptr + ptrOffset, kSprControls, i);
			}
		}

		const int size = READ_LE_UINT32(ptr + ptrOffset); ptrOffset += 4;
		if (size != 0) {
			hdrOffset = ptrOffset;
			ptrOffset += size * 20;
			for (int i = 0; i < size; ++i) {
				const int count = READ_LE_UINT16(ptr + hdrOffset + i * 20 + 4 + 12);
				for (int j = 0; j < count; ++j) {
					ptrOffset += DecodeSetupDatSprite(ptr + ptrOffset, kSprMenuButtons, j);
				}
			}
		}

		if (version == 10) {

			ptrOffset += menus * 8; // menu data

			hdrOffset = ptrOffset;
			ptrOffset += options * 8;
			for (int i = 0; i < options; ++i) {
				const int compressedSize = READ_LE_UINT32(ptr + hdrOffset + i * 8);
				if (compressedSize != 0) {
					char name[64];
					snprintf(name, sizeof(name), "options_%02d", i);
					ptrOffset += DecodeSetupDatBitmap256x192(name, ptr + ptrOffset, compressedSize, (i == 11) ? _controlsPalette : _bitmapPalette);
				}
			}

			hdrOffset = ptrOffset;
			ptrOffset += levels * 12;
			ptrOffset += DecodeSetupDatBitmapGroup("levels_%d", levels, ptr + hdrOffset, ptr + ptrOffset);
		}

		free(ptr);
	}
}

static int LookupLevelNum(const char *filename) {
	const char *sep = strrchr(filename, '/');
	const char *p = sep ? (sep + 1) : filename;
	for (int i = 0; _levels[i]; ++i) {
		if (strncasecmp(p, _levels[i], strlen(_levels[i])) == 0) {
			return i;
		}
	}
	return -1;
}

static void DecodePC(const char *filename, int type, uint32_t flags) {
	File *fp;
	if (flags & SECTOR_FILE) {
		fp = new SectorFile;
	} else {
		fp = new File;
	}
	_isPsx = ((flags & PSX) != 0) || (type == kDax);
	if (fp->open(filename)) {
		switch (type) {
		case kDat:
		case kDax:
			DecodeSetupDat(fp);
			break;
		case kLvl:
			DecodeLvl(fp, LookupLevelNum(filename));
			break;
		case kSss:
			DecodeSss(fp);
			break;
		case kMst:
			DecodeMst(fp);
			break;
		}
	}
	delete fp;
}

static void DecodeSaturn0000(FILE *fp, FILE *fp_offsets) {
	static const struct {
		uint32_t offset;
		int w, h;
	} _data[] = {
		{ 0x1ce88, 320, 224 },
		{ 0x51790, 104, 890 },
		{ 0, 0, 0 }
	};
	for (int i = 0; _data[i].offset; ++i) {
		fseek(fp, _data[i].offset, SEEK_SET);
		uint8_t *ptr = (uint8_t *)malloc(_data[i].w * _data[i].h * 3);
		if (ptr) {
			fread(ptr, 1, _data[i].w * _data[i].h * 3, fp);

			char name[64];
			snprintf(name, sizeof(name), "00000000_%08x.tga", _data[i].offset);
			saveTGA(name, ptr, _data[i].w, _data[i].h);

			free(ptr);
		}
	}
}

static void DecodeSaturn0001(FILE *fp, FILE *fp_offsets) {
	uint8_t tag[4];
	fread(tag, 1, 4, fp);
	assert(memcmp(tag, "DCD\x02", 4) == 0);

	char buffer[256];
	while (fgets(buffer, sizeof(buffer), fp_offsets)) {
		if (buffer[0] != '#') {
			int screen, state;
			uint32_t lzwOffset, paletteOffset;
			if (sscanf(buffer, "%d %d 0x%x 0x%x", &screen, &state, &lzwOffset, &paletteOffset) == 4) {

				fseek(fp, paletteOffset, SEEK_SET);
				fread(_bitmapPalette, 1, sizeof(_bitmapPalette), fp);
				ConvertVgaPalette(_bitmapPalette);

				uint8_t *ptr = (uint8_t *)malloc(256 * 192);
				if (ptr) {
					fseek(fp, lzwOffset, SEEK_SET);
					fread(ptr, 1, 256 * 192, fp);

					const int uncompressedSize = decodeLZW(ptr, _bitmapBuffer);
					assert(uncompressedSize == 256 * 192);
					char name[64];
					snprintf(name, sizeof(name), "00000001_%02d_%02d.bmp", screen, state);
					saveBMP(name, _bitmapBuffer, _bitmapPalette, 256, 192);

					free(ptr);
				}
			}
		}
	}
}

static void DecodeSaturn0002(FILE *fp) {
	uint8_t tag[4];
	fread(tag, 1, 4, fp);
	assert(memcmp(tag, "DHD\x02", 4) == 0);

	const int screensCount = fgetc(fp);

	// level map
	uint8_t gridData[kMaxScreens * 4];
	assert(screensCount <= kMaxScreens);
	fread(gridData, 4, screensCount, fp);
	GenerateLevelMap(gridData, screensCount);
}

static void DecodeSaturn0003(FILE *fp) { // .sss

	int data1Count = freadUint32LE(fp);
	int data2Count = freadUint32LE(fp);
	int data3Count = freadUint32LE(fp);
	int infosCount = freadUint32LE(fp);
	int codeSize   = freadUint32LE(fp);
	int pcmCount   = freadUint32LE(fp);

	fseek(fp, data1Count * 8,  SEEK_CUR);
	fseek(fp, data2Count * 4,  SEEK_CUR);
	fseek(fp, data3Count * 8,  SEEK_CUR);
	fseek(fp, infosCount * 24, SEEK_CUR);

	uint8_t *codeData = (uint8_t *)malloc(codeSize);
	if (codeData) {
		fread(codeData, 1, codeSize, fp);
		DecodeSssCode(codeData, codeSize);
		free(codeData);
	}

	long pos = ftell(fp);

	for (int i = 0; i < pcmCount; ++i) {
		fseek(fp, pos + i * 20, SEEK_SET);
		const uint32_t ptr    = freadUint32LE(fp);
		const uint32_t offset = freadUint32LE(fp);
		const uint32_t size   = freadUint32LE(fp);
		const uint32_t stride = freadUint32LE(fp);
		const uint16_t count  = freadUint16LE(fp);
		freadUint16LE(fp); // flags

		assert(ptr == 0);
		assert(size == stride * count);

		if (size != 0) {

			fseek(fp, offset, SEEK_SET);
			uint8_t *samples = (uint8_t *)malloc(size);
			if (samples) {
				fread(samples, 1, size, fp);

				// ffplay -acodec adpcm_ct -f u8 -ar 11025 -ac 1 -i $fn.pcm

				char name[64];
				snprintf(name, sizeof(name), "hod_%03d.pcm", i);
				FILE *out = fopen(name, "wb");
				if (out) {
					fwrite(samples, 1, size, out);
					fclose(out);
				}

				int decompressedSize = (size - 2) * 2 * sizeof(int16_t) + sizeof(int16_t);
				uint8_t *pcm = (uint8_t *)malloc(decompressedSize);
				if (decompressedSize) {

					DecodeSssAdpcm(samples, size, (int16_t *)pcm);

					snprintf(name, sizeof(name), "hod_%03d.wav", i);
					saveWAV(name, 11025, 16, 1, pcm, decompressedSize);

					free(pcm);
				}

				free(samples);
			}
		}
	}
}

static void Decode(const char *filename) {
	FILE *fp = fopen(filename, "rb");
	if (fp) {
		uint32_t crc, flags = 0;

		uint8_t buf[2048];
		crc = crc32(0, Z_NULL, 0);
		int count = fread(buf, 1, sizeof(buf), fp);
		if (count == sizeof(buf)) {
			if (fioUpdateCRC(0, buf, sizeof(buf)) == 0) {
				flags = SECTOR_FILE;
			}
			crc = crc32(crc, buf, count);
			while ((count = fread(buf, 1, sizeof(buf), fp)) > 0) {
				crc = crc32(crc, buf, count);
			}
		}
		fclose(fp);

		const char *ext = strrchr(filename, '.');
		if (ext) {
			for (int i = 0; _files[i].name; ++i) {
				if (_files[i].crc == crc) {
					DecodePC(filename, _files[i].type, _files[i].flags);
					return;
				}
			}
			for (int i = 0; _exts[i].name; ++i) {
				if (strcasecmp(_exts[i].name, ext) == 0) {
					DecodePC(filename, _exts[i].type, flags);
					return;
				}
			}
		} else { // 000000xy
			for (int i = 0; _saturn[i].num != -1; ++i) {
				if (_saturn[i].crc == crc) {
					FILE *fp = fopen(filename, "rb");
					if (fp) {
						switch (_saturn[i].num) {
						case 0:
						case 1: {
								char name[32];
								snprintf(name, sizeof(name), "offs_%08d.txt", _saturn[i].num);
								FILE *fp_offsets = fopen(name, "rb");
								if (!fp_offsets) {
									fprintf(stderr, "Failed to open '%s'\n", name);
								} else {
									if (_saturn[i].num == 0) {
										DecodeSaturn0000(fp, fp_offsets);
									} else {
										DecodeSaturn0001(fp, fp_offsets);
									}
									fclose(fp_offsets);
								}
							}
							break;
						case 2:
							DecodeSaturn0002(fp);
							break;
						case 3:
							DecodeSaturn0003(fp);
							break;
						}
						fclose(fp);
					}
					return;
				}
			}
		}
		fprintf(stderr, "Unrecognized file %s, crc 0x%x\n", filename, crc);
	}
}

int main(int argc, char *argv[]) {
	if (argc >= 2) {
		for (int i = 1; i < argc; ++i) {
			Decode(argv[i]);
		}
	} else {
		fprintf(stdout, "%s [.dat|.dax|.lvl|.sss|.mst]\n", argv[0]);
	}
	return 0;
}
