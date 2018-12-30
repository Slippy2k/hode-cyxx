
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "lzw.h"
#include "screenshot.h"

static const int MAX_OFFSETS = 1024;

static uint8_t _buffer[256 * 224];
static uint8_t _buffer2[256 * 224 + 1024];
static uint8_t _palette[256 * 3];

static uint32_t offsets[MAX_OFFSETS];

static void decodeBitmap(FILE *fp, uint32_t offset) {
	int count;
	char name[32];

	fseek(fp, offset, SEEK_SET);
	fread(_buffer, 1, sizeof(_buffer), fp);
	count = decodeLZW(_buffer, _buffer2);
	fprintf(stdout, "bitmap size %d offset 0x%x\n", count, offset);
//	snprintf(name, sizeof(name), "%08x.bmp", offset);
	static int num = 0;
	snprintf(name, sizeof(name), "%02d.bmp", num++);
	saveBMP(name, _buffer2, _palette, 256, 224);
}

// ~/Data/heart_of_darkness/DATA_saturn/00000001
int main(int argc, char *argv[]) {
	int count = 0;
	FILE *fp = fopen(argv[1], "rb");
	if (fp) {
/*
		// matches DOS demo
		decodeBitmap(fp, 0x616);
		decodeBitmap(fp, 0x6646);
		decodeBitmap(fp, 0x9D9CA);
		decodeBitmap(fp, 0xAF514);
*/
		// generate default grayscale palette
		if (1) {
			fseek(fp, 10, SEEK_SET);
			fread(_palette, 1, sizeof(_palette), fp);
			for (int i = 0; i < 256 * 3; ++i) {
				_palette[i] = (_palette[i] << 2) | (_palette[i] & 3);
			}
		} else {
			for (int i = 0; i < 256; ++i) {
				_palette[3 * i] = _palette[3 * i + 1] = _palette[3 * i + 2] = i;
			}
		}

		// find 0x00 0xAD 0x00
		fseek(fp, 0, SEEK_SET);
		do {
			if (fgetc(fp) == 0) {
				if (fgetc(fp) == 0xAD && fgetc(fp) == 0) {
					assert(count < MAX_OFFSETS);
					offsets[count++] = ftell(fp) - 1;
				}
			}
		} while (!feof(fp));
		fprintf(stdout, "Found %d offsets\n", count);
		for (int i = 0; i < count; ++i) {
			decodeBitmap(fp, offsets[i]);
			if (i == 1) {
				for (int i = 0; i < 256; ++i) {
					_palette[3 * i] = _palette[3 * i + 1] = _palette[3 * i + 2] = i;
				}
			}
		}
		fclose(fp);
	}
	return 0;
}
