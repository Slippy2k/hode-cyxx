
#include <cstdio>
#include <cstdlib>

static void DumpData(FILE *fp, int offset, int size, const char *name) {
	fseek(fp, offset, SEEK_SET);
	printf("static const uint8_t %s[] = {", name);
	for (int i = 0; i < size; ++i) {
		if (i % 16 == 0) {
			printf("\n\t");
		}
		printf("0x%02X", fgetc(fp));
		if (i != size - 1) {
			printf(", ");
		}
	}
	printf("\n};\n");
}

int main(int argc, char *argv[]) {
	if (argc == 2) {
		FILE *fp = fopen(argv[1], "rb");
		if (fp) {
//			DumpData(fp, 0x3D960, 2152, "byte_43D960");
//			DumpData(fp, 0x3EA78, 4536, "byte_43EA78");

DumpData(fp, 0x3FD40, 96, "_game_level0_updateData1");
DumpData(fp, 0x3FDA0, 56, "_game_level0_updateData2");

			fclose(fp);
		}
	}
	return 0;
}
