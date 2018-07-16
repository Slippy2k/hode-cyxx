
#include "file.h"
#include "staticdata.h"

extern int raw2png_6bits_color;
extern void raw2png(FILE *fp, const uint8_t *src, int width, int height, const uint8_t *palette);
extern int UnpackData(int type, const uint8_t *src, uint8_t *dst);

static uint8_t _paf_buffer0x800[0x800];
static uint8_t *_res_setupDatLoadingPicture;
static int _res_setupDatFontSize;
static uint8_t *_res_setupDatFontData;
static uint8_t dword_4622A0[0x2E * 4];
static uint8_t dword_462160[0x2E * 4];
static int _res_setupDatHeader0x40;


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

/*
0000 MenuList        struc ; (sizeof=0x10)   ; XREF: menu_addToUnkList
0000                                         ; menu_addToUnkList+3 ...
0000 ptr1            dd ? // picture data ?
0004 ptr2            dd ? // palette data ?
0008 size            dd ?
000C count           dw ?
000E num             dw ?
0010 MenuList        ends
*/
/*
.text:004221E0 menu_addToUnkList proc near             ; CODE XREF: menu_loadData+609
.text:004221E0                                         ; menu_loadData+618 ...
.text:004221E0                 mov     eax, [ecx+MenuList.size]
.text:004221E3                 and     [ecx+MenuList.num], 0
.text:004221E8                 mov     [ecx+MenuList.ptr1], edx
.text:004221EA                 mov     [ecx+MenuList.ptr2], edx
.text:004221ED                 add     eax, edx
.text:004221EF                 retn
.text:004221EF menu_addToUnkList endp
*/

static uint8_t *menu_addToUnkList(uint8_t *_ecx, uint8_t *_edx) {
	return _edx + READ_LE_UINT32(_ecx + 8);
}

/* res_readSetupDat */
static void ReadSetupDat(File *f) {
	int size = res_roundTo2048(76);
	printf("size %d\n", size);
	
	res_seekAndReadCurrentFile3(f, _paf_buffer0x800, size, 0);
	const uint8_t *_esi = _paf_buffer0x800;

	int _ebx = READ_LE_UINT32(_esi);
	int _res_setupDatHeader0x08 = READ_LE_UINT32(_esi + 0x08);
	int _res_setupDatHeader0x04 = READ_LE_UINT32(_esi + 0x04);
	int _res_setupDatHeader0x0C = READ_LE_UINT32(_esi + 0x0C);
	int _res_setupDatHeader0x10 = READ_LE_UINT32(_esi + 0x10);
	int _res_setupDatHeader0x14 = READ_LE_UINT32(_esi + 0x14);
	int _res_setupDatHeader0x18 = READ_LE_UINT32(_esi + 0x18);
	int _res_setupDatHeader0x1C = READ_LE_UINT32(_esi + 0x1C);
	int _res_setupDatHeader0x20 = READ_LE_UINT32(_esi + 0x20);
	int _res_setupDatHeader0x24 = READ_LE_UINT32(_esi + 0x24);
	int _res_setupDatHeader0x28 = READ_LE_UINT32(_esi + 0x28);
	int _res_setupDatHeader0x2C = READ_LE_UINT32(_esi + 0x2C);
	int _res_setupDatHeader0x30 = READ_LE_UINT32(_esi + 0x30);
	int _res_setupDatHeader0x34 = READ_LE_UINT32(_esi + 0x34);
	int _res_setupDatHeader0x38 = READ_LE_UINT32(_esi + 0x38);
	int _res_setupDatHeader0x3C = READ_LE_UINT32(_esi + 0x3C);
	_res_setupDatHeader0x40 = READ_LE_UINT32(_esi + 0x40);
	printf("Quit Yes/No image index %d\n", _res_setupDatHeader0x40);
	int _res_setupDatHeader0x44 = READ_LE_UINT32(_esi + 0x44);
	int _res_setupDatHeader0x48 = READ_LE_UINT32(_esi + 0x48);

	memcpy(dword_4622A0, _paf_buffer0x800 + 0x4C, 0x2E * 4);
	memcpy(dword_462160, _paf_buffer0x800 + 0x4C + 0xB8, 0x2E * 4);
	assert(_ebx == 11);
	
	size = res_roundTo2048(_res_setupDatHeader0x48);
	printf("size %d/%d\n", size, _res_setupDatHeader0x48);
	uint8_t *_res_setupDatUnk1 = (uint8_t *)malloc(size);
	int pos = res_roundTo2048(76);
	size = res_roundTo2048(_res_setupDatHeader0x48);
	res_seekAndReadCurrentFile3(f, _res_setupDatUnk1, size, pos);
	
//	_res_setupDatUnk1 = mem_realloc(_res_setupDatUnk1, _res_setupDatHeader0x48);
	_res_setupDatLoadingPicture = _res_setupDatUnk1;
	uint8_t *_ecx = _res_setupDatLoadingPicture;

	// loading screen
	int _edx = READ_LE_UINT32(_res_setupDatLoadingPicture); // picture size
	_ecx += 8;
//	WRITE_LE_UINT32(_res_setupDatLoadingPicture, _ecx); // picture data
	_ecx += _edx;
//	WRITE_LE_UINT32(_res_setupDatLoadingPicture + 4, _ecx); // palette data
	_ecx += 768;
//	_res_setupDatMenuList = _ecx;
	
	uint8_t *_edx_p = _ecx + 16;
	uint8_t *_eax = menu_addToUnkList(_ecx, _edx_p);

	_res_setupDatFontSize = READ_LE_UINT32(_eax);
	_eax += 4;
	_res_setupDatFontData = _eax;
//	_res_setupDatUnk5 = READ_LE_UINT32(dword_4622A0 + (2 + _res_setupDatHeader0x40) * 4);*/
}

static uint8_t decodeBuffer[256 * 192 * 4];

/* res_loadFont */
static void LoadFont() {
	int size = UnpackData(9, _res_setupDatFontData, decodeBuffer);
	printf("LoadFont size %d\n", size);
	if (size == 16 * 16 * 64) {
		FILE *fp = fopen("setupDatFont.png", "wb");
		if (fp) {
			uint8_t greyPal[256 * 3];
			for (int i = 0; i < 256; ++i) { greyPal[i * 3] = greyPal[i * 3 + 1] = greyPal[i * 3 + 2] = i << 4; }
			raw2png(fp, decodeBuffer, 16, 16 * 64, greyPal);
			fclose(fp);
		}
	}
}

/*menu_readSetupDat*/
/*menu_loadData*/

static void DecodeMainScreen() {
	int src_size_1 = READ_LE_UINT32(_res_setupDatLoadingPicture + 0);
	int src_size_2 = READ_LE_UINT32(_res_setupDatLoadingPicture + 4);
	printf("_res_setupDatLoadingPicture LE32 %d LE32 %d\n", src_size_1, src_size_2);
	int size = UnpackData(9, _res_setupDatLoadingPicture + 8, decodeBuffer);
	printf("DecodeMainScreen size %d\n", size);
	if (size == 256 * 192) {
		FILE *fp = fopen("setupDatUnk8.png", "wb");
		if (fp) {
			raw2png_6bits_color = 1;
			raw2png(fp, decodeBuffer, 256, 192, _res_setupDatLoadingPicture + 8 + src_size_1);
			fclose(fp);
		}
	}
}

#define PADDING 4096
static uint8_t tmpBuf[256 * 192 + PADDING];
static uint8_t pal[768 + PADDING];

static void DecodeHintScreen(File *f) {
	for (int i = 0; i < 0x2E; ++i) {
		uint32_t offs = READ_LE_UINT32(dword_4622A0 + i * 4);
		uint32_t size = READ_LE_UINT32(dword_462160 + i * 4);
		printf("%02d offs 0x%X size %d\n", i, offs, size);
		if (size == 256 * 192) {
			int size_img = res_roundTo2048(size);
			res_seekAndReadCurrentFile3(f, tmpBuf, size_img, offs);
			if (i == _res_setupDatHeader0x40 + 1) {
				// re-use palette from image _res_setupDatHeader0x40
			} else {
				res_seekAndReadCurrentFile3(f, pal, 0x800, offs + size_img);
			}
			char filename[30];
			sprintf(filename, "HOD_HINT_%02d.png", i);
			FILE *fp = fopen(filename, "wb");
			if (fp) {
				raw2png_6bits_color = 1;
				raw2png(fp, tmpBuf, 256, 192, pal);
				fclose(fp);
			}
		}
	}
}

static void writeStaticData(const char *filename, const uint8_t *buf) {
	for (int i = 0; i < 256; ++i) {
		const int j = i * 3;
		pal[j] = pal[j + 1] = pal[j + 2] = i;
	}
	FILE *fp = fopen(filename, "wb");
	if (fp) {
		raw2png_6bits_color = 1;
		raw2png(fp, buf, 256, 192, pal);
		fclose(fp);
	}
}

static void writeFile(const char *filename, const uint8_t *buf, int bufSize) {
	FILE *fp = fopen(filename, "wb");
	if (fp) {
		fwrite(buf, bufSize, 1, fp);
		fclose(fp);
	}
}

static void DecodeStaticData() {
	int sz1 = UnpackData(9, byte_43D960, decodeBuffer);
	assert(sz1 == 256 * 192);
	writeFile("data_43D960.bin", decodeBuffer, sz1);
	writeStaticData("data_43D960.png", decodeBuffer);

	int sz2 = UnpackData(9, byte_43EA78, decodeBuffer);
	assert(sz2 == 256 * 192);
	writeFile("data_43EA78.bin", decodeBuffer, sz2);
	writeStaticData("data_43EA78.png", decodeBuffer);
}

int main(int argc, char *argv[]) {
	if (argc == 2) {
		File f;
		if (f.open(argv[1], "rb")) {
			ReadSetupDat(&f);
			DecodeMainScreen();
			DecodeHintScreen(&f);
			LoadFont();
		}
	}
	DecodeStaticData();
	return 0;
}
