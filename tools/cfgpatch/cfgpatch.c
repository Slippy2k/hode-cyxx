
#include <stdio.h>
#include <stdlib.h>
#include "dik_constants.h"

static unsigned char _plyChecksum = 0;

static int READ_LE_UINT32(const unsigned char *p) {
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

static int PlyReadNextByte(const unsigned char *p) {
	_plyChecksum ^= *p;
	return *p;
}

static int PlyReadNextInt(const unsigned char *p) {	
	int i, val = 0;

	for (i = 0; i < 4; ++i) {
		val |= *p << (i * 8);
		_plyChecksum ^= *p;
		++p;
	}
	return val;
}

static void PlyPrintConfigData(const unsigned char *p) {
	int i;
	
	for (i = 0; i < 10; ++i) {
		printf("Level %d Screen %d\n", i, PlyReadNextByte(p)); p++;
	}
	printf("Current Level %d\n", PlyReadNextByte(p)); p++;
	printf("Current Screen %d\n", PlyReadNextByte(p)); p++;
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
	printf("Unk31 %d\n", PlyReadNextByte(p)); p++;
	printf("Sound Volume %d\n", PlyReadNextByte(p)); p++;
	printf("Maximum Level Reached %d\n", PlyReadNextByte(p)); p++;
}

int main(int argc, char *argv[]) {
	int i;
	FILE *fp;
	unsigned char buf[212];
	
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
			printf("Current Player Config %d\n", buf[209]);
			printf("Player Configuration Data Checksum 0x%X/0x%X\n", buf[211], _plyChecksum);
			fclose(fp);
		}
	}
	return 0;
}
