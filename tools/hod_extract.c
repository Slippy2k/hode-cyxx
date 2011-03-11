
#include <stdio.h>

typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned long uint32;
typedef signed long int32;

uint16 READ_LE_UINT16(const void *ptr) {
	return *(const uint16 *)ptr;
}

uint32 READ_LE_UINT32(const void *ptr) {
	return *(const uint32 *)ptr;
}

uint16 fread_uint16LE(FILE *fp) {
	char data[2];
	fread(data, 2, 1, fp);
	return READ_LE_UINT16(data);
}

uint32 fread_uint32LE(FILE *fp) {
	char data[4];
	fread(data, 4, 1, fp);
	return READ_LE_UINT32(data);
}

int main(int argc, char *argv[]) {
	if (argc >= 2) {
		FILE *fp = fopen(argv[1], "rb");
		if (fp) {
			int i = 0;
			fseek(fp, 0x4800, SEEK_SET);
			while (!feof(fp)) {
				uint32 off = fread_uint32LE(fp);
				uint32 size1 = fread_uint32LE(fp);
				uint32 size2 = fread_uint32LE(fp);
				uint32 unk = fread_uint32LE(fp);
				printf("i=%d off=0x%X size1=%d size2=%d unk=%d\n", i, off, size1, size2, unk);
				++i;
			}
			fclose(fp);
		}
	}
	return 0;
}
