
#include "file.h"


/*static uint32 ComputeChecksum(const uint8 *buf, uint32 size) {
	assert((size & 3) == 0);
	size >>= 2;
	uint32 crc = 0;
	while (size--) {
		crc ^= READ_LE_UINT32(buf); buf += 4;
	}
	return crc;
}*/

extern int raw2png_6bits_color;
extern void raw2png(FILE *fp, const uint8 *src, int width, int height, const uint8 *palette);
extern int UnpackData(int type, const uint8 *src, uint8 *dst);

static uint32 UpdateCRC(uint32 &sum, const uint8 *buf, uint32 size) {
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

static uint8 _res_lvlFileHeader[0x7FC * 9];
static uint8 _res_lvlLayerMoveTable[0xA0];
static uint8 _res_lvlPalData[0x140];
static uint8 _res_lvlLgcData[0xA0]; /* _res_levelData0x1E8 */
static uint8 *_res_lvlBmpData; // _levelDescription / sizeof == 160

static uint8 tmpBuf[256 * 192 * 2];

static uint8 currentPalette[768 + 2];
static uint8 spritePalette[768];
int spritePaletteInitDone = 0;

static uint8 *pLevelData0x2B88 = 0;

static void LoadLevelData0x2B88(File *f, const uint8 *LevelData0x2B88) {
	uint32 offs = READ_LE_UINT32(LevelData0x2B88);
	uint32 size = READ_LE_UINT32(LevelData0x2B88 + 4);
	uint32 alignedSize = READ_LE_UINT32(LevelData0x2B88 + 4);
	
	assert(size <= alignedSize);
	
	printf("LoadLevelData0x2B88 offs 0x%X %d %d\n", offs, size, alignedSize);
	free(pLevelData0x2B88);
	pLevelData0x2B88 = (uint8 *)malloc(size);
//	assert((alignedSize & 3) == 0);
	f->seek(offs, SEEK_SET);
#if 0
	f->read(pLevelData0x2B88, alignedSize);
#else
	uint32 crc = 0;
	uint8 *fillPtr = pLevelData0x2B88;
	for (int i = 0; i < alignedSize / 0x800; ++i) {
		f->read(fillPtr, 0x800);
		UpdateCRC(crc, fillPtr, 0x800);
		fillPtr += 0x800 - 4;
/*		printf("CRC %d = 0x%X\n", i, crc);*/
		assert(crc == 0);
	}
#endif
}

uint8 DecodeSprBuffer[256 * 192];

/*const uint8 *spr_src;
uint8 *spr_dst;
uint8 spr_cl;

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
void DecodeSPR_FLAG0(const uint8 *src, uint8 *dst, int MUL320) {
	uint8 *_ebx = dst; // dstStart
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

void DecodeSPR_FLAG0(const uint8 *src, uint8 *dst, int MUL320) {
	uint8 *dstLine = dst;
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

static void DecodeSprite(int i, const uint8 *src, int x, int pitch, uint8 *dst) {
	int size = READ_LE_UINT16(src);
	int w = READ_LE_UINT16(src + 2);
	int h = READ_LE_UINT16(src + 4);
//	printf("bitmap %d size %d/%d w %d h %d\n", i, size, w * h, w, h);
	memset(DecodeSprBuffer, 0, 256 * 192);
	DecodeSPR_FLAG0(src, dst + x, pitch);
}

static uint8 *pLevelData0x2988 = 0;

static void LoadLevelData0x2988(File *f, const uint8 *LevelData0x2988) {
	uint32 offs = READ_LE_UINT32(LevelData0x2988);
	uint32 size = READ_LE_UINT32(LevelData0x2988 + 4);
	uint32 alignedSize = READ_LE_UINT32(LevelData0x2988 + 4);

	pLevelData0x2988 = (uint8 *)malloc(ResRoundTo2048(size));
	f->seek(offs, SEEK_SET);
	uint32 crc = 0;
	uint8 *fillPtr = pLevelData0x2988;
	for (int i = 0; i < ResRoundTo2048(alignedSize) / 0x800; ++i) {
		f->read(fillPtr, 0x800);
		UpdateCRC(crc, fillPtr, 0x800);
		fillPtr += 0x800 - 4;
		assert(crc == 0);
	}
	
	uint8 *_ecx0x18 = pLevelData0x2988 + READ_LE_UINT32(pLevelData0x2988 + 0x1C);
	int count = READ_LE_UINT16(pLevelData0x2988 + 2);
	int img_h = 0;
	int img_w = 0;
	uint8 *_eax0x18 = _ecx0x18;
	for (int i = 0; i < count; ++i) {
		int bitmapSize = READ_LE_UINT16(_eax0x18);
		int w = READ_LE_UINT16(_eax0x18 + 2);
		int h = READ_LE_UINT16(_eax0x18 + 4);
		img_w += w + 5;
		if (h > img_h) img_h = h;
		_eax0x18 += bitmapSize;
	}
	
	uint8 *picBuf = (uint8 *)malloc(img_w * img_h);
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
		raw2png_6bits_color = 0;
		raw2png(fp, picBuf, img_w, img_h, spritePalette);
		fclose(fp);
	}	
	
	free(picBuf);
	
}


static void DumpBitmap(File *f, int i, int j, uint32 subOffs) {
/*	uint32 baseOffs = READ_LE_UINT32(LevelData0x2B88);
	uint32 baseSize = READ_LE_UINT32(LevelData0x2B88 + 4);
	printf("   %02d baseOffs 0x%X subOffs 0x%X (0x%X) size %d\n", i, baseOffs, subOffs, baseOffs + subOffs, baseSize);
	f->seek(baseOffs, SEEK_SET);
	
	uint8 *p = (uint8 *)malloc(baseSize);
	if (!p) return;
		
	f->read(p, baseSize);
	uint8 *data = p + subOffs;*/
	uint8 *data = pLevelData0x2B88 + subOffs;
	printf("   BMP LE16 %04X ", READ_LE_UINT16(data));
	
	uint32 outputSize = UnpackData(9, data + 2, tmpBuf);
	printf("outputSize %d\n", outputSize);

	if (outputSize == 49152) {
		char filename[64];
		sprintf(filename, "%s_%02d_%d.png", basePath, i, j);
		FILE *fp = fopen(filename, "wb");
		if (fp) {
			raw2png_6bits_color = 0;
			raw2png(fp, tmpBuf, 256, 192, currentPalette);
/*			fwrite(tmpBuf, outputSize, 1, fp);*/
			fclose(fp);
		}		
	}
	
/*	free(p);*/
}

static void DumpPalette(File *f, int i, int j, const uint8 *LevelData0x2B88, uint32 subOffs) {
/*	uint32 baseOffs = READ_LE_UINT32(LevelData0x2B88);
	f->seek(baseOffs + subOffs, SEEK_SET);
	f->read(palette, 768 + 2);*/
	
	uint8 *data = pLevelData0x2B88 + subOffs;
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
		uint8 *dat = (uint8 *)malloc(128 * 256);
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

	uint8 *fillPtr = _res_lvlFileHeader;
	uint32 crc = 0;
	for (int i = 0; i < 9; ++i) {
		f->read(fillPtr, 0x800);
		UpdateCRC(crc, fillPtr, 0x800);
		fillPtr += 0x800 - 4;
		printf("CRC %d = 0x%X\n", i, crc);
	}
	int _res_lvlUnk6 = _res_lvlFileHeader[6]; // _cl, _dl
	int _res_lvlUnk2 = _res_lvlFileHeader[4]; // _al
	int _res_lvlUnk1 = _res_lvlFileHeader[5]; // _ah, _cl
	int _res_lvlUnk3 = _res_lvlUnk6 + _res_lvlUnk1; // _dl + _cl
	int _res_lvlUnk4 = _res_lvlFileHeader[7]; // _dl

	printf("%d %d %d %d\n", _res_lvlUnk2, _res_lvlUnk1, _res_lvlUnk6, _res_lvlUnk4);

	memcpy(_res_lvlLayerMoveTable, _res_lvlFileHeader + 8, _res_lvlUnk2 * 4);
	for (int i = 0; i < _res_lvlUnk2; ++i) printf("%2d - 0x%08X\n", i, READ_LE_UINT32(&_res_lvlLayerMoveTable[i * 4]));
	memcpy(_res_lvlPalData, _res_lvlFileHeader + 0xA8, _res_lvlUnk2 * 8);
	for (int i = 0; i < _res_lvlUnk2; ++i) printf("%2d - 0x%X 0x%X\n", i, READ_LE_UINT32(&_res_lvlPalData[i * 8]), READ_LE_UINT32(&_res_lvlPalData[i * 8 + 4]));
	memcpy(_res_lvlLgcData, _res_lvlFileHeader + 0x1E8, _res_lvlUnk2 * 4);
	for (int i = 0; i < _res_lvlUnk2; ++i) printf("%2d - 0x%X\n", i, READ_LE_UINT32(&_res_lvlLgcData[i * 4]));

/*	const uint8 *p0 = _res_lvlFileHeader + 0x288; // sizeof == 96
	for (int i = 0; i < (0x2988 - 0x288) / 96; ++i) { // _res_lvlGraphicsDataTable
		printf("base 0x288 %d w %d h %d 0x2E08 %d 0x2988 %d layer %d\n", i, READ_LE_UINT16(p0 + 0x1C), READ_LE_UINT16(p0 + 0x1E), p0[8], p0[15], READ_LE_UINT16(p0 + 0x14));
		p0 += 96;
	}*/

	const uint8 *q = _res_lvlFileHeader + 0x2B88;
	for (int i = 0; i < (0x2E08 - 0x2B88) / 16; ++i) {
		uint32 a = READ_LE_UINT32(q); q += 4;
		uint32 b = READ_LE_UINT32(q); q += 4;
		uint32 c = READ_LE_UINT32(q); q += 4;
		uint32 d = READ_LE_UINT32(q); q += 4;
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
			uint32 palOffs = READ_LE_UINT32(_res_lvlBmpData + 0x10 + j * 4);
			if (palOffs != 0) {
				DumpPalette(f, i, j, _res_lvlFileHeader + 0x2B88 + i * 16, palOffs);
			}
			uint32 bitmapOffs = READ_LE_UINT32(_res_lvlBmpData + 0x20 + j * 4);
			if (bitmapOffs != 0) {
				DumpBitmap(f, i, j, bitmapOffs);
			}
/*			uint32 unk1Offs = READ_LE_UINT32(_res_lvlBmpData + 0x40 + j * 4);
			if (unk1Offs != 0) {
				DumpData(f, i, 40 + j, _res_lvlFileHeader + 0x2B88 + i * 16, unk1Offs);
			}*/
		}
		
		_res_lvlBmpData += 160;
	}

	const uint8 *q0 = _res_lvlFileHeader + 0x2988;
	for (int i = 0; i < (0x2B88 - 0x2988) / 16; ++i) {
		uint32 a = READ_LE_UINT32(q0); q0 += 4;
		uint32 b = READ_LE_UINT32(q0); q0 += 4;
		uint32 c = READ_LE_UINT32(q0); q0 += 4;
		uint32 d = READ_LE_UINT32(q0); q0 += 4;
		printf("base 0x2988 %02d / offs 0x%X size 0x%X 0x%X ptr %d\n", i, a, b, c, d);
		if (b != 0) {
			LoadLevelData0x2988(f, _res_lvlFileHeader + 0x2988 + i * 16);
		}
	}

//	0x4708
//	0x470C
	printf("shadow palettes offs 0x%X size %d\n", READ_LE_UINT32(_res_lvlFileHeader + 0x4708), READ_LE_UINT32(_res_lvlFileHeader + 0x470C));

	if (memcmp(_res_lvlFileHeader, "\0DOH", 4) == 0) {
		printf("signature ok");
	}
}

#undef main
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
		}
	}
	return 0;
}
