
#include "file.h"
#include "staticdata.h"

extern void savePNG(const char *filename, int w, int h, const uint8_t *src, const uint8_t *palette, int vga_colors);
extern int UnpackData(int type, const uint8_t *src, uint8_t *dst);

static uint8_t decodeBuffer[256 * 192 * 4];
static uint8_t fontBuffer[64 * 16 * 16];
static uint8_t defaultPalette[256 * 3];

static uint8_t _paf_buffer0x800[0x800];
static uint8_t *_res_setupDatLoadingPicture;
static int _res_setupDatFontSize;
static uint8_t *_res_setupDatFontData;
static uint8_t _datHdr_setupImageOffsetTable[0x2E * 4];
static uint8_t _datHdr_setupImageSizeTable[0x2E * 4];
static int _res_setupDatHeader0x04;
static int _res_setupDatHeader0x08;
static int _res_setupDatHeader0x0C;
static int _res_setupDatHeader0x10;
static int _res_setupDatHeader0x14;
static int _res_setupDatHeader0x18;
static int _res_setupDatHeader0x1C;
static int _res_setupDatHeader0x20;
static int _res_setupDatHeader0x24;
static int _res_setupDatHeader0x28;
static int _res_setupDatHeader0x2C;
static int _res_setupDatHeader0x30;
static int _res_setupDatHeader0x34;
static int _res_setupDatHeader0x38;
static int _res_setupDatHeader0x3C;
static int _res_setupDatHeader0x40;
static int _res_setupDatHeader0x44;
static int _res_setupDatHeader0x48;
static int _res_setupDatBaseOffset;

void DecodeSPR_FLAG0(const uint8_t *src, uint8_t *dst, int dstPitch) {
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
			dstLine += dstPitch * count;
			code = *src++;
			dst = dstLine + code;
			break;
		}
	}
}

static int res_roundTo2048(int pos) {
	return ((pos + 2043) / 2044) * 2048;
}

/*static void res_seekAndReadCurrentFile3(File *_ecx, uint8_t *_edx, int count, int offset) {
	int _util_currentFileCount = count;
	File *_util_currentFileHandle = _ecx;
	uint8_t *_util_currentFileBuffer = _edx;
	int _util_currentFileOffset = _eax;
	
	_ecx->seek(offset, SEEK_SET);
	int _ecx_i = _util_currentFileCount;
	while (1) {
		_ecx->read(_util_currentFileBuffer, _util_currentFileCount);
		_edx = _util_currentFileCount;
		_esi = _util_currentFileBuffer;
		_eax = 0;
		_ecx = _esi;
		_edx >>= 2;
		if (_edx == 0) break;
		do {
			_edi = *(uint32_t *)_ecx;
			_ecx += 4;
			_eax ^= _edi;
			--_edx;
		} while (_edx != 0);
		if (_eax == 0) break;
		_util_currentFileHandle->seek(_util_currentFileOffset);
	}
	_util_currentFileCount -= 0x800;
	_eax = _esi + 0x7FC; // 0x800 - 4
	_edx = _eax + 4;
	while (_util_currentFileCount != 0) {
		memcpy(_eax, _edx, 0x7FC);
		_eax += 0x7FC;
		_edx += 0x800;
		_util_currentFileCount -= 0x800;
	}
}*/

static uint32_t UpdateCRC(uint32_t &sum, const uint8_t *buf, uint32_t size) {
	assert((size & 3) == 0);
	size >>= 2;
	while (size--) {
		sum ^= READ_LE_UINT32(buf); buf += 4;
	}
	return sum;
}

static void res_seekAndReadCurrentFile3(File *_ecx, uint8_t *_edx, int size, int offset) {
	_ecx->seek(offset, SEEK_SET);
	uint8_t *fillPtr = _edx;
	assert((size & 3) == 0);
	uint32_t crc = 0;
	for (int i = 0; i < size / 0x800; ++i) {
		_ecx->read(fillPtr, 0x800);
		UpdateCRC(crc, fillPtr, 0x800);
		fillPtr += 0x800 - 4;
/*		printf("CRC %d = 0x%X\n", i, crc);*/
		assert(crc == 0);
	}
}

static int group_counter = 0;
static int sprite_counter = 0;

static uint8_t *menu_addToSpriteList(uint8_t *_ecx, uint8_t *_edx) {
	++group_counter;
	const int size = READ_LE_UINT32(_ecx + 8);
	const int count = READ_LE_UINT16(_ecx + 12);
	fprintf(stdout, "sprites size %d count %d\n", size, count);
	const uint8_t *p = _edx;
	for (int i = 0; i < count; ++i) {
		const int x = p[0];
		const int y = p[1];
		const int compressedSize = READ_LE_UINT16(p + 2);
		const int w = READ_LE_UINT16(p + 4);
		const int h = READ_LE_UINT16(p + 6);
		fprintf(stdout, "sprite #%d pos %d,%d compressedSize %d dimensions %d,%d\n", i, x, y, compressedSize, w, h);

		if (w != 0 && h != 0) {
			memset(decodeBuffer, 0, sizeof(decodeBuffer));
			DecodeSPR_FLAG0(p + 2, decodeBuffer, w);

			char name[32];
			snprintf(name, sizeof(name), "sprite_%02d_%03d.png", group_counter, sprite_counter);
			++sprite_counter;
			savePNG(name, w, h, decodeBuffer, defaultPalette, 1);
		}

		p += compressedSize + 2;
	}
	return _edx + size;
}

/* res_readSetupDat */
static void ReadSetupDat(File *f) {
	int size = res_roundTo2048(76);
	printf("size %d\n", size);
	
	res_seekAndReadCurrentFile3(f, _paf_buffer0x800, size, 0);
	const uint8_t *_esi = _paf_buffer0x800;

	int _ebx = READ_LE_UINT32(_esi);
	_res_setupDatHeader0x08 = READ_LE_UINT32(_esi + 0x08); // offset (from _res_setupDatBaseOffset)
	_res_setupDatHeader0x04 = READ_LE_UINT32(_esi + 0x04); // size
	_res_setupDatHeader0x0C = READ_LE_UINT32(_esi + 0x0C);
	_res_setupDatHeader0x10 = READ_LE_UINT32(_esi + 0x10);
	_res_setupDatHeader0x14 = READ_LE_UINT32(_esi + 0x14);
	_res_setupDatHeader0x18 = READ_LE_UINT32(_esi + 0x18);
	_res_setupDatHeader0x1C = READ_LE_UINT32(_esi + 0x1C);
	_res_setupDatHeader0x20 = READ_LE_UINT32(_esi + 0x20);
	_res_setupDatHeader0x24 = READ_LE_UINT32(_esi + 0x24);
	_res_setupDatHeader0x28 = READ_LE_UINT32(_esi + 0x28);
	_res_setupDatHeader0x2C = READ_LE_UINT32(_esi + 0x2C);
	_res_setupDatHeader0x30 = READ_LE_UINT32(_esi + 0x30);
	_res_setupDatHeader0x34 = READ_LE_UINT32(_esi + 0x34);
	_res_setupDatHeader0x38 = READ_LE_UINT32(_esi + 0x38);
	_res_setupDatHeader0x3C = READ_LE_UINT32(_esi + 0x3C);
	_res_setupDatHeader0x40 = READ_LE_UINT32(_esi + 0x40);
	printf("Quit Yes/No image index %d\n", _res_setupDatHeader0x40);
	_res_setupDatHeader0x44 = READ_LE_UINT32(_esi + 0x44);
	_res_setupDatHeader0x48 = READ_LE_UINT32(_esi + 0x48);
	printf("Loading image size %d\n", _res_setupDatHeader0x48);

	memcpy(_datHdr_setupImageOffsetTable, _paf_buffer0x800 + 0x4C, 0x2E * 4);
	memcpy(_datHdr_setupImageSizeTable, _paf_buffer0x800 + 0x4C + 0xB8, 0x2E * 4);
	assert(_ebx == 11);
	
	size = res_roundTo2048(_res_setupDatHeader0x48);
	printf("size %d/%d\n", size, _res_setupDatHeader0x48);
	uint8_t *_res_setupDatLoadingImageBuffer = (uint8_t *)malloc(size);
	int pos = res_roundTo2048(76);
	size = res_roundTo2048(_res_setupDatHeader0x48);
	res_seekAndReadCurrentFile3(f, _res_setupDatLoadingImageBuffer, size, pos);
	
//	_res_setupDatLoadingImageBuffer = mem_realloc(_res_setupDatLoadingImageBuffer, _res_setupDatHeader0x48);
	_res_setupDatLoadingPicture = _res_setupDatLoadingImageBuffer;
	uint8_t *_ecx = _res_setupDatLoadingPicture;

	// loading screen
	int _edx = READ_LE_UINT32(_res_setupDatLoadingPicture); // picture size
	_ecx += 8;
//	WRITE_LE_UINT32(_res_setupDatLoadingPicture, _ecx); // picture data
	_ecx += _edx;
//	WRITE_LE_UINT32(_res_setupDatLoadingPicture + 4, _ecx); // palette data
	_ecx += 768;
//	uint8_t *_menu_loadingSpritesList = _ecx;
	
	uint8_t *_edx_p = _ecx + 16;
	uint8_t *_eax = menu_addToSpriteList(_ecx, _edx_p);

	_res_setupDatFontSize = READ_LE_UINT32(_eax);
	_eax += 4;
	_res_setupDatFontData = _eax;
	_res_setupDatBaseOffset = READ_LE_UINT32(_datHdr_setupImageOffsetTable + 4 * (2 + _res_setupDatHeader0x40));
	printf("setupDatBaseOffset is 0x%x (index %d)\n", _res_setupDatBaseOffset, 2 + _res_setupDatHeader0x40);
}

static void res_loadFont() {
	const int size = UnpackData(9, _res_setupDatFontData, decodeBuffer);
	if (size == 16 * 16 * 64) {
		int offset = 0;
		for (int i = 0; i < 64; ++i) {
			const uint8_t chr = i;
			const uint8_t *p = decodeBuffer + ((chr & 15) + (chr >> 4) * 256) * 16;
			for (int y = 0; y < 16; ++y) {
				memcpy(fontBuffer + offset, p, 16);
				p += 16 * 16;
				offset += 16;
			}
		}
		savePNG("font.png", 16, 16 * 64, fontBuffer, defaultPalette, 1);
	}
}

static void DecodeLoadingScreen() {
	int src_size_1 = READ_LE_UINT32(_res_setupDatLoadingPicture + 0);
	int src_size_2 = READ_LE_UINT32(_res_setupDatLoadingPicture + 4);
	printf("_res_setupDatLoadingPicture LE32 %d LE32 %d\n", src_size_1, src_size_2);
	int size = UnpackData(9, _res_setupDatLoadingPicture + 8, decodeBuffer);
	printf("DecodeLoadingScreen size %d\n", size);
	if (size == 256 * 192) {
		savePNG("loading.png", 256, 192, decodeBuffer, _res_setupDatLoadingPicture + 8 + src_size_1, 1);
	}
}

static void DecodeHintScreen(File *f) {
	uint8_t pal[0x800];
	for (int i = 0; i < 0x2E; ++i) {
		uint32_t offs = READ_LE_UINT32(_datHdr_setupImageOffsetTable + i * 4);
		uint32_t size = READ_LE_UINT32(_datHdr_setupImageSizeTable + i * 4);
		printf("%02d offs 0x%X size %d\n", i, offs, size);
		if (size == 256 * 192) {
			int size_img = res_roundTo2048(size);
			res_seekAndReadCurrentFile3(f, decodeBuffer, size_img, offs);
			if (i == _res_setupDatHeader0x40 + 1) {
				// re-use palette from image _res_setupDatHeader0x40
			} else {
				res_seekAndReadCurrentFile3(f, pal, 0x800, offs + size_img);
			}
			char filename[32];
			snprintf(filename, sizeof(filename), "hint_%02d.png", i);
			savePNG(filename, 256, 192, decodeBuffer, pal, 1);
		}
	}
}

static void DecodeMenuBitmap256x192(const char *filename, int compressedSize, const uint8_t *p) {
	const int decompressedSize = UnpackData(9, p, decodeBuffer);
	fprintf(stdout, "bitmap %d %d\n", compressedSize, decompressedSize);
	if (decompressedSize == 256 * 192) {
		const uint8_t *palette = p + compressedSize;
		savePNG(filename, 256, 192, decodeBuffer, palette, 1);
	}
}

static uint8_t *DecodeMenuBitmap(const char *filename, const uint8_t *data, int count, uint8_t *p) {
	uint8_t paletteBuffer[256 * 3];
	for (int i = 0; i < 256; ++i) {
		const int color = i * 63 / 255;
		for (int j = 0; j < 3; ++j) {
			paletteBuffer[3 * i + j] = color;
		}
	}
	for (int i = 0; i < count; ++i) {
		const int w = data[0];
		const int h = data[1];
		const uint16_t unk = READ_LE_UINT16(data + 2);
		const uint32_t ptr1 = READ_LE_UINT32(data + 4);
		const uint32_t ptr2 = READ_LE_UINT32(data + 8);
		fprintf(stdout, "menuBitmap %d width %d height %d unk %d size 0x%x,0x%x\n", i, w, h, unk, ptr1, ptr2);
		data += 12;

		const uint8_t *src = p + w * h;

		uint8_t *palette = paletteBuffer;

		memcpy(paletteBuffer + (unk & 255) * 3, src, 150);

		char name[32];
		snprintf(name, sizeof(name), filename, i);
		savePNG(name, w, h, p, palette, 1);

		p += w * h + 768;
	}
	return p;
}

static void DecodeMenuData(File *f) {
	// bitmaps
	const int size = _res_setupDatHeader0x08;
	const int offset = _res_setupDatBaseOffset; //res_roundTo2048(_res_setupDatBaseOffset);
	fprintf(stdout, "menuBitmaps 0x%x 0x%x (offset 0x%x)\n", _res_setupDatHeader0x08, _res_setupDatBaseOffset, offset);
	uint8_t *menu_bitmapsBuffer = (uint8_t *)malloc(size);
	if (menu_bitmapsBuffer) {
		res_seekAndReadCurrentFile3(f, menu_bitmapsBuffer, size, offset);

		uint8_t *menu_titleBitmap = menu_bitmapsBuffer + 4;

		uint8_t *menu_playerBitmap = menu_bitmapsBuffer + 12;

		uint8_t *menu_optionBitmaps = menu_bitmapsBuffer + 20;

		uint8_t *p = menu_bitmapsBuffer + 172;

		uint8_t *menu_cutsceneBitmaps = p;
		p += _res_setupDatHeader0x18 * 12;

		uint8_t *menu_dataPtr2 = p; // checkpoints level1
		p += _res_setupDatHeader0x20 * 12;

		uint8_t *menu_dataPtr3 = p; // checkpoints level2
		p += _res_setupDatHeader0x24 * 12;

		uint8_t *menu_dataPtr4 = p; // checkpoints level3
		p += _res_setupDatHeader0x28 * 12;

		uint8_t *menu_dataPtr5 = p; // checkpoints level4
		p += _res_setupDatHeader0x2C * 12;

		uint8_t *menu_dataPtr6 = p; // checkpoints level5
		p += _res_setupDatHeader0x30 * 12;

		uint8_t *menu_dataPtr7 = p; // checkpoints level6
		p += _res_setupDatHeader0x34 * 12;

		uint8_t *menu_dataPtr8 = p; // checkpoints level7
		p += _res_setupDatHeader0x38 * 12;

		uint8_t *menu_dataPtr9 = p; // checkpoints level8
		p += _res_setupDatHeader0x3C * 12;

		uint8_t *menu_dataPtr10 = p; // levels
		p += _res_setupDatHeader0x1C * 12;

		int compressedSize = READ_LE_UINT32(menu_titleBitmap);
		if (compressedSize != 0) {
			DecodeMenuBitmap256x192("title.png", compressedSize, p);
			p += compressedSize + 768;
		}
		compressedSize = READ_LE_UINT32(menu_playerBitmap);
		if (compressedSize != 0) {
			DecodeMenuBitmap256x192("player.png", compressedSize, p);
			p += compressedSize + 768;
		}
		for (int i = 0; i < 152 / 8; ++i) {
			compressedSize = READ_LE_UINT32(menu_optionBitmaps + i * 8);
			if (compressedSize != 0) {
				char name[16];
				snprintf(name, sizeof(name), "options_%02d.png", i);
				DecodeMenuBitmap256x192(name, compressedSize, p);
				p += compressedSize + 768;
			}
		}

		p = DecodeMenuBitmap("cutscenes%02d.png", menu_cutsceneBitmaps, _res_setupDatHeader0x18, p);
		p = DecodeMenuBitmap("level1_checkpoints%02d.png", menu_dataPtr2, _res_setupDatHeader0x20, p);
		p = DecodeMenuBitmap("level2_checkpoints%02d.png", menu_dataPtr3, _res_setupDatHeader0x24, p);
		p = DecodeMenuBitmap("level3_checkpoints%02d.png", menu_dataPtr4, _res_setupDatHeader0x28, p);
		p = DecodeMenuBitmap("level4_checkpoints%02d.png", menu_dataPtr5, _res_setupDatHeader0x2C, p);
		p = DecodeMenuBitmap("level5_checkpoints%02d.png", menu_dataPtr6, _res_setupDatHeader0x30, p);
		p = DecodeMenuBitmap("level6_checkpoints%02d.png", menu_dataPtr7, _res_setupDatHeader0x34, p);
		p = DecodeMenuBitmap("level7_checkpoints%02d.png", menu_dataPtr8, _res_setupDatHeader0x38, p);
		p = DecodeMenuBitmap("level8_checkpoints%02d.png", menu_dataPtr9, _res_setupDatHeader0x3C, p);
		p = DecodeMenuBitmap("levels%02d.png", menu_dataPtr10, _res_setupDatHeader0x1C, p);

		free(menu_bitmapsBuffer);
	}

	// sprites
	const int sizeSpr = res_roundTo2048(_res_setupDatHeader0x04);
	const int offsetSpr = _res_setupDatBaseOffset + res_roundTo2048(_res_setupDatHeader0x08);
	fprintf(stdout, "menuSprites 0x%x (offset 0x%x)\n", _res_setupDatHeader0x04, offsetSpr);
	uint8_t *menu_spritesBuffer = (uint8_t *)malloc(sizeSpr);
	if (menu_spritesBuffer) {
		res_seekAndReadCurrentFile3(f, menu_spritesBuffer, sizeSpr, offsetSpr);

		uint8_t *_eax = menu_spritesBuffer;

//		uint8_t *menu_spritesList1 = _eax;
		_eax = menu_addToSpriteList(_eax, _eax + 16);
//		uint8_t *menu_spritesList2 = _eax;
		_eax = menu_addToSpriteList(_eax, _eax + 16);
//		uint8_t *menu_spritesList3 = _eax;

		_eax += _res_setupDatHeader0x14 * 8;

		int unkSize = READ_LE_UINT32(_eax); // null_var1
		_eax += 4;
//		uint8_t *menu_spritesPtr1 = _eax;

		_eax += unkSize;
//		uint8_t *menu_spritesPtr2 = _eax;

		_eax += _res_setupDatHeader0x44;
		uint8_t *menu_spritesPtr3 = _eax;
		_eax += _res_setupDatHeader0x10 * 16;

		for (int i = 0; i < _res_setupDatHeader0x10; ++i) {
			_eax = menu_addToSpriteList(menu_spritesPtr3 + i * 16, _eax);
		}

		unkSize = READ_LE_UINT32(_eax);
		_eax += 4;
		uint8_t *menu_spritesPtr4 = _eax;

		if (unkSize != 0) {
			_eax += unkSize * 20;
			for (int i = 0; i < unkSize; ++i) {
				_eax = menu_addToSpriteList(menu_spritesPtr4 + i * 20 + 4, _eax);
			}
		}

		free(menu_spritesBuffer);
	}
}

static void writeBenchmarkData(const char *filename, const uint8_t *data) {
	uint8_t paletteBuffer[256 * 3];
	for (int i = 0; i < 16; ++i) {
		const uint8_t color = i * 16;
		memset(paletteBuffer + 3 * i, color, 3);
	}
	memset(paletteBuffer + 16 * 3, 0, (256 - 240) * 3);
	const int sz = UnpackData(9, data, decodeBuffer);
	assert(sz == 256 * 192);
	savePNG(filename, 256, 192, decodeBuffer, paletteBuffer, 0);
}

static void DecodeBenchmarkData() {
	writeBenchmarkData("data_43D960.png", byte_43D960);
	writeBenchmarkData("data_43EA78.png", byte_43EA78);
}

int main(int argc, char *argv[]) {
	for (int i = 0; i < 256; ++i) {
		const int color = i * 63 / 255;
		for (int j = 0; j < 3; ++j) {
			defaultPalette[3 * i + j] = color;
		}
	}
	if (argc == 2) {
		File f;
		if (f.open(argv[1], "rb")) {
			ReadSetupDat(&f);
			DecodeLoadingScreen();
			DecodeHintScreen(&f);
			DecodeMenuData(&f);
			res_loadFont();
		}
	}
	DecodeBenchmarkData();
	return 0;
}
