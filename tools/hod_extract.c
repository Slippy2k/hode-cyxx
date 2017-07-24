
#include <stdio.h>
#include <stdint.h>

uint32_t READ_LE_UINT32(const void *ptr) {
	return *(const uint32_t *)ptr;
}

uint32_t freadUint32LE(FILE *fp) {
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
				uint32_t off = freadUint32LE(fp);
				uint32_t size1 = freadUint32LE(fp);
				uint32_t size2 = freadUint32LE(fp);
				uint32_t unk = freadUint32LE(fp);
				printf("i=%d off=0x%X size1=%d size2=%d unk=%d\n", i, off, size1, size2, unk);
				++i;
			}
			fclose(fp);
		}
	}
	return 0;
}
