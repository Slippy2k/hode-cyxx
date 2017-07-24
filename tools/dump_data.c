
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

enum DataDumpType {
	UNSIGNED_8BITS  = 0,
	SIGNED_8BITS    = 1,
	UNSIGNED_16BITS = 2,
	SIGNED_16BITS   = 3,
	UNSIGNED_32BITS = 4,
	SIGNED_32BITS   = 5
};

static uint32_t freadUint32LE(FILE *fp) {
	uint8_t a = fgetc(fp);
	uint8_t b = fgetc(fp);
	uint8_t c = fgetc(fp);
	uint8_t d = fgetc(fp);
	return (d << 24) | (c << 16) | (b << 8) | a;
}

static uint16_t freadUint16LE(FILE *fp) {
	uint8_t a = fgetc(fp);
	uint8_t b = fgetc(fp);
	return (b << 8) | a;
}

static void dumpInt(FILE *fp, const char *tableName, uint32_t offset, int size, const char *fmt, enum DataDumpType type) {
	int i;

	printf("%s[%d] = {", tableName, size);
	fseek(fp, offset, SEEK_SET);
	for (i = 0; i < size; ++i) {
		int num = 0;
		switch (type) {
		case UNSIGNED_8BITS:
			num = fgetc(fp);
			break;
		case SIGNED_8BITS:
			num = (int8_t)fgetc(fp);
			break;
		case UNSIGNED_16BITS:
			num = freadUint16LE(fp);
			break;
		case SIGNED_16BITS:
			num = (int16_t)freadUint16LE(fp);
			break;
		default:
			num = freadUint32LE(fp);
			break;
		}
		if ((i % 16) == 0) {
			printf("\n\t");
		}
		printf(fmt, num);
		printf(", ");
	}
	printf("\n};\n");
}

static void fillInt(const char *tableName, int size, const char *fmt, int num) {
	int i;

	printf("%s[%d] = {", tableName, size);
	for (i = 0; i < size; ++i) {
		if ((i % 16) == 0) {
			printf("\n\t");
		}
		printf(fmt, num);
		printf(", ");
	}
	printf("\n};\n");
}

int main(int argc, char* argv[]) {
	FILE *fp;

#if 0
	fp = fopen("../DATA_demo/HODWin32_DEMO.exe", "rb");
	if (fp) {
//		dumpInt(fp, "_byte_43E8F8", 0x3E8F8, 68, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_byte_43E940", 0x3E940, 68, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level1OpHelper1KeyMaskTable", 0x3E888, 112, "%d", UNSIGNED_8BITS);
//		dumpInt(fp, "_andyMoveTable35", 0x40358, 48, "%3d", SIGNED_8BITS);
		dumpInt(fp, "_actionDirectionKeyMaskTable", 0x3E3D4, 352, "0x%02X", UNSIGNED_8BITS);
		fclose(fp);
	}
#endif
#if 1
	fp = fopen("../DATA_retail/HODWin32.exe", "rb");
	if (fp) {
//		dumpInt(fp, "_level1UpdateData1", 0x53060, 96, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level1UpdateData2", 0x530C0, 56, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level2UpdateData1", 0x4E940, 64, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level2UpdateData2", 0x58A50, 56, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level3UpdateData1", 0x517A8, 120, "0x%02X", UNSIGNED_8BITS);
//		fillInt("_level3UpdateData2", 80, "0x%02X", 0);
//		dumpInt(fp, "_level4UpdateData1", 0x4F408, 64, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level4UpdateData2", 0x58A88, 56, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level5UpdateData1", 0x50248, 72, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level5UpdateData2", 0x50290, 40, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level6UpdateData1", 0x51D68, 40, "0x%02X", UNSIGNED_8BITS);
//		fillInt("_level6UpdateData2", 32, "0x%02X", 0);
//		dumpInt(fp, "_level7UpdateData1", 0x4F6F8, 112, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level7UpdateData2", 0x4F768, 56, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level8UpdateData1", 0x4F8C0, 144, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level8UpdateData2", 0x58B28, 64, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level9UpdateData1", 0x4E850, 16, "0x%02X", UNSIGNED_8BITS);
//		dumpInt(fp, "_level9UpdateData2", 0x58A40, 8, "0x%02X", UNSIGNED_8BITS);
		dumpInt(fp, "_levelOpStage3ImageData1", 0x51DA8, 667, "0x%02X", UNSIGNED_8BITS);
		dumpInt(fp, "_levelOpStage3ImageData2", 0x50500, 2152, "0x%02X", UNSIGNED_8BITS); 
		fclose(fp);
	}
#endif
	return 0;
}
