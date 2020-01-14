
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "dik_constants.h"

static uint8_t _plyChecksum = 0;

static int PlyReadNextByte(const uint8_t *p) {
	_plyChecksum ^= *p;
	return *p;
}

static int PlyReadNextInt(const uint8_t *p) {
	int i, val = 0;

	for (i = 0; i < 4; ++i) {
		val |= *p << (i * 8);
		_plyChecksum ^= *p;
		++p;
	}
	return val;
}

static void PlyPrintConfigData(const uint8_t *p) {
	int i;
	
	for (i = 0; i < 10; ++i) {
		printf("Level %d Checkpoint %d\n", i, PlyReadNextByte(p)); p++;
	}
	printf("Current Level %d\n", PlyReadNextByte(p)); p++;
	printf("Current Checkpoint %d\n", PlyReadNextByte(p)); p++;
	printf("Played Cutscenes Mask 0x%X\n", PlyReadNextInt(p)); p += 4;
	for (i = 0; i < 4; ++i) {
		printf("Joystick Keyboard Action %d 0x%X\n", i, PlyReadNextInt(p)); p += 4;
	}
	for (i = 0; i < 8; ++i) {
		printf("Keyboard Action %d 0x%X\n", i, PlyReadNextByte(p)); p++;
	}
	for (i = 0; i < 8; ++i) {
		printf("Keyboard Action Alt %d 0x%X\n", i, PlyReadNextByte(p)); p++;
	}
	printf("Difficulty %d\n", PlyReadNextByte(p)); p++;
	printf("Stereo Sound %d\n", PlyReadNextByte(p)); p++;
	printf("Sound Volume %d\n", PlyReadNextByte(p)); p++;
	printf("Last Level Reached %d\n", PlyReadNextByte(p)); p++;
}

int main(int argc, char *argv[]) {
	int i;
	FILE *fp;
	uint8_t buf[212];
	
	if (argc == 2) {
		fp = fopen(argv[1], "rb");
		if (fp) {
			fread(buf, 1, 212, fp);
			_plyChecksum = 0;
			for (i = 0; i < 4; ++i) {
				printf("=== PLAYER %d DATA ===\n", i);
				PlyPrintConfigData(&buf[i * 52]);
				printf("=== END OF PLAYER DATA === \n");
			}
			_plyChecksum ^= buf[208];
			printf("Unk208 %d (expected 8)\n", buf[208]);
			_plyChecksum ^= buf[209];
			printf("Current Player Config %d\n", buf[209]);
			_plyChecksum ^= buf[210];
			printf("Unk210 %d\n", buf[210]);
			printf("Player Configuration Data Checksum 0x%X (expected 0x%X)\n", buf[211], _plyChecksum);
			fclose(fp);
		}
	}
	return 0;
}
