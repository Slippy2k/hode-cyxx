
#include <stdio.h>
#include <stdlib.h>

typedef unsigned int uint32;

static uint32 freadUint32LE(FILE *fp) {
	unsigned char b[4];
	fread(b, 4, 1, fp);
	return b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
}

static uint32 fileSize(FILE *fp) {
	int size, pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	return size;
}

int main(int argc, char *argv[]) {
	if (argc >= 2) {
		FILE *fp = fopen(argv[1], "rb");
		if (fp) {
			FILE *fp_o;
			uint32 offsTable[51];
			int i, j, offs;
			for (j = 0, i = 0; i < 50; ++i) {
				offs = freadUint32LE(fp);
				if (offs != 0) {
					offsTable[j++] = offs;
				}
			}
			offsTable[j] = fileSize(fp);
			for (i = 0; i < j; ++i) {
				char buf[4096];
				char filepath[256];
				int size = offsTable[i + 1] - offsTable[i];
				if (size <= 0) {
					printf("error size = %d\n", size);
				} else {
					sprintf(filepath, "hod_demo_%d.paf", i);
					fp_o = fopen(filepath, "wb");
					printf("dumping '%s' index %d offset 0x%X\n", filepath, i, offsTable[i]);
					if (fp_o) {
						fseek(fp, offsTable[i], SEEK_SET);
						while (size > 0) {
							if (size > 4096) {
								fread(buf, 4096, 1, fp);
								fwrite(buf, 4096, 1, fp_o);
								size -= 4096;
							} else {
								fread(buf, size, 1, fp);
								fwrite(buf, size, 1, fp_o);
								size = 0;
							}
						}
						fclose(fp_o);
					}
				}
			}
			fclose(fp);
		}
	}
	return 0;
}
