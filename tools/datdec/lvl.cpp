
#include "file.h"

extern void raw2png(FILE *fp, const uint8_t *src, int width, int height, const uint8_t *palette, int raw2png_6bits_color);
extern int UnpackData(int type, const uint8_t *src, uint8_t *dst);

static uint32_t UpdateCRC(uint32_t &sum, const uint8_t *buf, uint32_t size) {
	assert((size & 3) == 0);
	size >>= 2;
	while (size--) {
		sum ^= READ_LE_UINT32(buf); buf += 4;
	}
	return sum;
}

static int ResRoundTo2048(int pos) {
	return ((pos + 2043) / 2044) * 2048;
}

static char basePath[40];

static uint8_t _res_lvlFileHeader[0x7FC * 9];
static uint8_t _res_screensGrid[0xA0];
static uint8_t _res_lvlPalData[0x140];
static uint8_t _res_lvlLgcData[0xA0]; /* _res_levelData0x1E8 */
static uint8_t *_res_lvlBmpData; // _levelDescription / sizeof == 160
static int _res_screensCount;

static uint8_t tmpBuf[256 * 192 * 2];

static uint8_t currentPalette[768 + 2];
static uint8_t spritePalette[768];
int spritePaletteInitDone = 0;

static uint8_t *pLevelData0x2B88 = 0;

static void LoadLevelData0x2B88(File *f, const uint8_t *LevelData0x2B88) {
	uint32_t offs = READ_LE_UINT32(LevelData0x2B88);
	uint32_t size = READ_LE_UINT32(LevelData0x2B88 + 4);
	uint32_t alignedSize = READ_LE_UINT32(LevelData0x2B88 + 4);
	
	assert(size <= alignedSize);
	
	printf("LoadLevelData0x2B88 offs 0x%X %d %d\n", offs, size, alignedSize);
	free(pLevelData0x2B88);
	pLevelData0x2B88 = (uint8_t *)malloc(size);
//	assert((alignedSize & 3) == 0);
	f->seek(offs, SEEK_SET);
#if 0
	f->read(pLevelData0x2B88, alignedSize);
#else
	uint32_t crc = 0;
	uint8_t *fillPtr = pLevelData0x2B88;
	for (int i = 0; i < alignedSize / 0x800; ++i) {
		f->read(fillPtr, 0x800);
		UpdateCRC(crc, fillPtr, 0x800);
		fillPtr += 0x800 - 4;
/*		printf("CRC %d = 0x%X\n", i, crc);*/
		assert(crc == 0);
	}
#endif
}

/*const uint8_t *spr_src;
uint8_t *spr_dst;
uint8_t spr_cl;

static void loc_42F828() { // flags == 0
//	_ebx = _edi;
	spr_cl = 0x3F;
	spr_al = *spr_src++
	spr_cl &= spr_al;
	goto _paf_decode5JmpTable1[spr_al];
}*/
/*	_edi += _eax;
	_edx = 0;
	_edi += 320 * _ebx;
	
//	goto off_441F50[_cl];
}
*/

#if 0
void DecodeSPR_FLAG0(const uint8_t *src, uint8_t *dst, int MUL320) {
	uint8_t *_ebx = dst; // dstStart
	src += 6; // src
//	int count = 0x3F; // count
	while (1) { // goto off_440C50[_al];
		int code = *src++;
		int count = 0x3F & code;
		switch (code >> 6) {
		case 0:
			memcpy(dst, src, count); dst += count; src += count;
			break;
		case 1:
			code = *src++;
			memset(dst, code, count); dst += count;
			break;
		case 2:
			if (count == 0) { /* 0x80 */
				code = *src++;
				dst += code;
			} else { /* < 0xC0 */
				dst += count;
			}
			break;
		case 3:
			if (count == 0) { /* 0xC0 */
				count = *src++;
				if (count == 0) {
					return;
				}
				_ebx += MUL320 * count;
				code = *src++;
				dst = _ebx + code;				
			} else { /* <= 0xFF */
				code = *src++;
				_ebx += MUL320 * count;
				dst = _ebx + code;
			}
			break;
		}
	}
}
#endif

void DecodeSPR_FLAG0(const uint8_t *src, uint8_t *dst, int MUL320) {
	uint8_t *dstLine = dst;
	src += 6;
	while (1) {
		int code = *src++;
		int count = code & 0x3F;
		switch (code >> 6) {
		case 0:
			memcpy(dst, src, count); dst += count; src += count;
			break;
		case 1:
			code = *src++;
			memset(dst, code, count); dst += count;
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
			dstLine += MUL320 * count;
			code = *src++;
			dst = dstLine + code;				
			break;
		}
	}
}

static int sprite_counter = 0;

static void DecodeSprite(int i, const uint8_t *src, int x, int pitch, uint8_t *dst) {
	int size = READ_LE_UINT16(src);
	int w = READ_LE_UINT16(src + 2);
	int h = READ_LE_UINT16(src + 4);
	printf("bitmap %d size %d/%d w %d h %d\n", i, size, w * h, w, h);
	DecodeSPR_FLAG0(src, dst + x, pitch);
}

static uint8_t *pLevelData0x2988 = 0;

static void LoadLevelData0x2988(File *f, const uint8_t *LevelData0x2988) {
	uint32_t offs = READ_LE_UINT32(LevelData0x2988);
	uint32_t size = READ_LE_UINT32(LevelData0x2988 + 4);
	uint32_t alignedSize = READ_LE_UINT32(LevelData0x2988 + 4);

	pLevelData0x2988 = (uint8_t *)malloc(ResRoundTo2048(size));
	f->seek(offs, SEEK_SET);
	uint32_t crc = 0;
	uint8_t *fillPtr = pLevelData0x2988;
	for (int i = 0; i < ResRoundTo2048(alignedSize) / 0x800; ++i) {
		f->read(fillPtr, 0x800);
		UpdateCRC(crc, fillPtr, 0x800);
		fillPtr += 0x800 - 4;
		assert(crc == 0);
	}
	
	uint8_t *_ecx0x18 = pLevelData0x2988 + READ_LE_UINT32(pLevelData0x2988 + 0x1C);
	int count = READ_LE_UINT16(pLevelData0x2988 + 2);
	int img_h = 0;
	int img_w = 0;
	uint8_t *_eax0x18 = _ecx0x18;
	for (int i = 0; i < count; ++i) {
		int bitmapSize = READ_LE_UINT16(_eax0x18);
		int w = READ_LE_UINT16(_eax0x18 + 2);
		int h = READ_LE_UINT16(_eax0x18 + 4);
		img_w += w + 5;
		if (h > img_h) img_h = h;
		_eax0x18 += bitmapSize;
	}
	
	uint8_t *picBuf = (uint8_t *)malloc(img_w * img_h);
	assert(picBuf);
	memset(picBuf, 0, img_w * img_h);
	
	_eax0x18 = _ecx0x18;
	int x = 0;
	for (int i = 0; i < count; ++i) {
		int bitmapSize = READ_LE_UINT16(_eax0x18);
		int w = READ_LE_UINT16(_eax0x18 + 2);
		DecodeSprite(i, _eax0x18, x, img_w, picBuf);
		x += w + 5;
		_eax0x18 += bitmapSize;
	}
	
	spritePalette[0] = 0xFF;
	spritePalette[1] = 0;
	spritePalette[2] = 0xFF;
	
	char filename[64];
	sprintf(filename, "%s_SPR_%d.png", basePath, sprite_counter++);
	FILE *fp = fopen(filename, "wb");
	if (fp) {
		raw2png(fp, picBuf, img_w, img_h, spritePalette, 0);
		fclose(fp);
	}	
	
	free(picBuf);
	
}


static void DumpBitmap(File *f, int i, int j, uint32_t subOffs) {
/*	uint32_t baseOffs = READ_LE_UINT32(LevelData0x2B88);
	uint32_t baseSize = READ_LE_UINT32(LevelData0x2B88 + 4);
	printf("   %02d baseOffs 0x%X subOffs 0x%X (0x%X) size %d\n", i, baseOffs, subOffs, baseOffs + subOffs, baseSize);
	f->seek(baseOffs, SEEK_SET);
	
	uint8_t *p = (uint8_t *)malloc(baseSize);
	if (!p) return;
		
	f->read(p, baseSize);
	uint8_t *data = p + subOffs;*/
	uint8_t *data = pLevelData0x2B88 + subOffs;
	printf("   BMP %02x %02x %02x %02x %02x", data[2], data[3], data[4], data[5], data[6]);
	
	uint32_t outputSize = UnpackData(9, data + 2, tmpBuf);
	printf("outputSize %d\n", outputSize);

	if (outputSize == 49152) {
		char filename[64];
		sprintf(filename, "%s_%02d_%d.png", basePath, i, j);
		FILE *fp = fopen(filename, "wb");
		if (fp) {
			raw2png(fp, tmpBuf, 256, 192, currentPalette, 0);
/*			fwrite(tmpBuf, outputSize, 1, fp);*/
			fclose(fp);
		}		
	}
	
/*	free(p);*/
}

static void DumpPalette(File *f, int i, int j, const uint8_t *LevelData0x2B88, uint32_t subOffs) {
/*	uint32_t baseOffs = READ_LE_UINT32(LevelData0x2B88);
	f->seek(baseOffs + subOffs, SEEK_SET);
	f->read(palette, 768 + 2);*/
	
	uint8_t *data = pLevelData0x2B88 + subOffs;
	printf("   PAL LE16 %04X ", READ_LE_UINT16(data));
	memcpy(currentPalette, data + 2, 768);
	if (spritePaletteInitDone == 0) {
		spritePaletteInitDone = 1;
		memcpy(spritePalette, currentPalette, 768);
	}
	
/*	char filename[20];
	sprintf(filename, "pal_%d_%d.png", i, j);
	FILE *fp = fopen(filename, "wb");
	if (fp) {
		uint8_t *dat = (uint8_t *)malloc(128 * 256);
		if (dat) {
			for (int i = 0; i < 256; ++i) {
				memset(dat + i * 128, i, 128);
			}
			raw2png(fp, dat, 128, 256, palette + 2);
			free(dat);			
		}
		fclose(fp);
	}*/
}

static void ResLoadLevelData(File *f) {

	uint8_t *fillPtr = _res_lvlFileHeader;
	uint32_t crc = 0;
	for (int i = 0; i < 9; ++i) {
		f->read(fillPtr, 0x800);
		UpdateCRC(crc, fillPtr, 0x800);
		fillPtr += 0x800 - 4;
		printf("CRC %d = 0x%X\n", i, crc);
	}
	int _res_lvlUnk6 = _res_lvlFileHeader[6]; // _cl, _dl
	_res_screensCount = _res_lvlFileHeader[4]; // _al
	int _res_lvlUnk1 = _res_lvlFileHeader[5]; // _ah, _cl
	int _res_lvlUnk3 = _res_lvlUnk6 + _res_lvlUnk1; // _dl + _cl
	int _res_lvlUnk4 = _res_lvlFileHeader[7]; // _dl

	printf("%d %d %d %d\n", _res_screensCount, _res_lvlUnk1, _res_lvlUnk6, _res_lvlUnk4);

	memcpy(_res_screensGrid, _res_lvlFileHeader + 8, _res_screensCount * 4);
	for (int i = 0; i < _res_screensCount; ++i) printf("%2d - 0x%08X\n", i, READ_LE_UINT32(&_res_screensGrid[i * 4]));
	memcpy(_res_lvlPalData, _res_lvlFileHeader + 0xA8, _res_screensCount * 8);
	for (int i = 0; i < _res_screensCount; ++i) printf("%2d - 0x%X 0x%X\n", i, READ_LE_UINT32(&_res_lvlPalData[i * 8]), READ_LE_UINT32(&_res_lvlPalData[i * 8 + 4]));
	memcpy(_res_lvlLgcData, _res_lvlFileHeader + 0x1E8, _res_screensCount * 4);
	for (int i = 0; i < _res_screensCount; ++i) printf("%2d - 0x%X\n", i, READ_LE_UINT32(&_res_lvlLgcData[i * 4]));

/*	const uint8_t *p0 = _res_lvlFileHeader + 0x288; // sizeof == 96
	for (int i = 0; i < (0x2988 - 0x288) / 96; ++i) { // _res_lvlGraphicsDataTable
		printf("base 0x288 %d w %d h %d 0x2E08 %d 0x2988 %d layer %d\n", i, READ_LE_UINT16(p0 + 0x1C), READ_LE_UINT16(p0 + 0x1E), p0[8], p0[15], READ_LE_UINT16(p0 + 0x14));
		p0 += 96;
	}*/

	const uint8_t *q = _res_lvlFileHeader + 0x2B88;
	for (int i = 0; i < (0x2E08 - 0x2B88) / 16; ++i) {
		uint32_t a = READ_LE_UINT32(q); q += 4;
		uint32_t b = READ_LE_UINT32(q); q += 4;
		uint32_t c = READ_LE_UINT32(q); q += 4;
		uint32_t d = READ_LE_UINT32(q); q += 4;
		printf("base 0x2B88 %02d / offs 0x%X size 0x%X 0x%X ptr %d\n", i, a, b, c, d);
	}

	_res_lvlBmpData = _res_lvlFileHeader + 0x2E08; // sizeof == 160 * 40, end = 0x4708 // _res_levelData0x2E08
	for (int i = 0; i < 40; ++i) {
		printf("base 0x2E08 %02d count %d palette_table [0x%X,0x%X,0x%X,0x%X] bitmap_table [0x%X,0x%X,0x%X,0x%X]\n", i, _res_lvlBmpData[0],
		READ_LE_UINT32(_res_lvlBmpData + 0x10), READ_LE_UINT32(_res_lvlBmpData + 0x14), READ_LE_UINT32(_res_lvlBmpData + 0x18), READ_LE_UINT32(_res_lvlBmpData + 0x1C),
		READ_LE_UINT32(_res_lvlBmpData + 0x20), READ_LE_UINT32(_res_lvlBmpData + 0x24), READ_LE_UINT32(_res_lvlBmpData + 0x28), READ_LE_UINT32(_res_lvlBmpData + 0x2C));

#if 0
f->seek(0x2B88 + i * 16);
f->read(_res_lvlFileHeader + 0x2B88 + i * 16, 16);
printf("0x%X\n", READ_LE_UINT32(_res_lvlFileHeader + 0x2B88 + i * 16));
#endif
		LoadLevelData0x2B88(f, _res_lvlFileHeader + 0x2B88 + i * 16);

		for (int j = 0; j < 4; ++j) {
			uint32_t palOffs = READ_LE_UINT32(_res_lvlBmpData + 0x10 + j * 4);
			if (palOffs != 0) {
				DumpPalette(f, i, j, _res_lvlFileHeader + 0x2B88 + i * 16, palOffs);
			}
			uint32_t bitmapOffs = READ_LE_UINT32(_res_lvlBmpData + 0x20 + j * 4);
			if (bitmapOffs != 0) {
				DumpBitmap(f, i, j, bitmapOffs);
			}
/*			uint32_t unk1Offs = READ_LE_UINT32(_res_lvlBmpData + 0x40 + j * 4);
			if (unk1Offs != 0) {
				DumpData(f, i, 40 + j, _res_lvlFileHeader + 0x2B88 + i * 16, unk1Offs);
			}*/
		}
		
		_res_lvlBmpData += 160;
	}

	const uint8_t *q0 = _res_lvlFileHeader + 0x2988;
	for (int i = 0; i < (0x2B88 - 0x2988) / 16; ++i) {
		uint32_t a = READ_LE_UINT32(q0); q0 += 4;
		uint32_t b = READ_LE_UINT32(q0); q0 += 4;
		uint32_t c = READ_LE_UINT32(q0); q0 += 4;
		uint32_t d = READ_LE_UINT32(q0); q0 += 4;
		printf("base 0x2988 %02d / offs 0x%X size 0x%X 0x%X ptr %d\n", i, a, b, c, d);
		if (b != 0) {
			LoadLevelData0x2988(f, _res_lvlFileHeader + 0x2988 + i * 16);
		}
	}

//	0x4708
//	0x470C
	printf("shadow palettes offs 0x%X size %d\n", READ_LE_UINT32(_res_lvlFileHeader + 0x4708), READ_LE_UINT32(_res_lvlFileHeader + 0x470C));

	if (memcmp(_res_lvlFileHeader, "\0DOH", 4) == 0) {
		printf("signature ok\n");
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

static const int kLevelMapMaxW = 128;
static const int kLevelMapMaxH = 128;

static const int kMaxScreens = 40;

static void GenerateLevelMap() {
	assert(_res_screensCount <= kMaxScreens);
	bool visited[kMaxScreens];
	for (int i = 0; i < _res_screensCount; ++i) {
		visited[i] = false;
	}

	LevelScreen queue[kMaxScreens];
	int queueSize = 0;

	uint8_t levelMap[kLevelMapMaxH][kLevelMapMaxW];
	memset(levelMap, 255, sizeof(levelMap));

	int xmin, xmax;
	int ymin, ymax;

	queue[0].num = 0;
	queue[0].x = xmin = xmax = kLevelMapMaxW / 2;
	queue[0].y = ymin = ymax = kLevelMapMaxH / 2;
	queueSize = 1;

	while (queueSize > 0) {
		const int screenNum = queue[0].num;
		const int x = queue[0].x;
		const int y = queue[0].y;
		--queueSize;
		if (queueSize != 0) {
			memmove(&queue[0], &queue[1], queueSize * sizeof(LevelScreen));
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
		if (visited[screenNum]) {
			continue;
		}
		levelMap[y][x] = screenNum;
		visited[screenNum] = true;
		const int top    = _res_screensGrid[screenNum * 4 + kPosTopScreen];
		const int right  = _res_screensGrid[screenNum * 4 + kPosRightScreen];
		const int bottom = _res_screensGrid[screenNum * 4 + kPosBottomScreen];
		const int left   = _res_screensGrid[screenNum * 4 + kPosLeftScreen];
		if (top != 255) {
			assert(y >= 0);
			queue[queueSize].num = top;
			queue[queueSize].x = x;
			queue[queueSize].y = y - 1;
			++queueSize;
		}
		if (bottom != 255) {
			assert(y < kLevelMapMaxH);
			queue[queueSize].num = bottom;
			queue[queueSize].x = x;
			queue[queueSize].y = y + 1;
			++queueSize;
		}
		if (left != 255) {
			assert(x >= 0);
			queue[queueSize].num = left;
			queue[queueSize].x = x - 1;
			queue[queueSize].y = y;
			++queueSize;
		}
		if (right != 255) {
			assert(x < kLevelMapMaxW);
			queue[queueSize].num = right;
			queue[queueSize].x = x + 1;
			queue[queueSize].y = y;
			++queueSize;
		}
	}
	fprintf(stdout, "levelMap y %d,%d (%d) x %d,%d (%d)\n", ymin, ymax, (ymax - ymin + 1), xmin, xmax, (xmax - xmin + 1));
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

int main(int argc, char *argv[]) {
	if (argc == 2) {

		char *p = strrchr(argv[1], '/');
		if (p) ++p; else p = argv[1];
		strcpy(basePath, p);
		p = strchr(basePath, '.');
		if (p) *p = 0;

		File f;
		if (f.open(argv[1], "rb")) {
			ResLoadLevelData(&f);
			GenerateLevelMap();
		}
	}
	return 0;
}
