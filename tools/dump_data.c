
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

enum {
	DEMO_FR_12,
	FULL_FR_10,
};

struct {
	uint32_t crc;
	int version;
} _versions[] = {
	{ 0x5429784f, DEMO_FR_12 },
	{ 0x3110fd89, FULL_FR_10 },
	{ 0, -1 }
};

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

static void dumpRect(FILE *fp, const char *name, uint32_t offset, int count) {
	fprintf(stdout, "static const BoundingBox %s[%d] = {", name, count);
	fseek(fp, offset, SEEK_SET);
	for (int i = 0; i < count; ++i) {
		int x1 = freadUint32LE(fp);
		int y1 = freadUint32LE(fp);
		int x2 = freadUint32LE(fp);
		int y2 = freadUint32LE(fp);
		fprintf(stdout, "\n\t{ %3d, %3d, %3d, %3d },", x1, y1, x2, y2);
	}
	fprintf(stdout, "\n");
}

static void dumpCheckpointData(FILE *fp, const char *name, uint32_t offset, int count) {
	assert((count % 12) == 0);
	fprintf(stdout, "static const CheckpointData %s[%d] = {", name, count / 12);
	fseek(fp, offset, SEEK_SET);
	for (int i = 0; i < count; i += 12) {
		int xPos = (int16_t)freadUint16LE(fp);
		int yPos = (int16_t)freadUint16LE(fp);
		int flags2 = freadUint16LE(fp);
		int anim = freadUint16LE(fp);
		int screenNum = fgetc(fp);
		int spriteNum = fgetc(fp);
		int unk = freadUint16LE(fp);
		fprintf(stdout, "\n\t{ %3d, %3d, 0x%04x, %3d, %2d, %2d },", xPos, yPos, flags2, anim, screenNum, spriteNum);
	}
	fprintf(stdout, "\n");
}

static void dumpFontCharMap(FILE *fp, const char *name, uint32_t offset, int count) {
	fprintf(stdout, "static const uint8_t %s[%d] = {", name, count * 2);
	fseek(fp, offset, SEEK_SET);
	for (int i = 0; i < count; ++i) {
		int chr = fgetc(fp);
		int num = fgetc(fp);
		int a = fgetc(fp);
		assert(a == 0);
		int b = fgetc(fp);
		assert(b == 0);
		if ((i % 8) == 0) {
			fprintf(stdout, "\n\t");
		} else {
			fprintf(stdout, " ");
		}
		fprintf(stdout, "0x%02x, 0x%02x,", chr, num);
	}
	fprintf(stdout, "\n};\n");
}

static void dumpBinary(FILE *fp, const char *name, uint32_t offset, int size, const char *fmt, enum DataDumpType type, uint32_t addr) {
	int i;

	printf("%s[%d] = {", name, size);
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
		case UNSIGNED_32BITS:
			num = freadUint32LE(fp);
			break;
		default:
			assert(0);
			break;
		}
		if ((i % 16) == 0) {
			printf("\n\t");
		} else {
			printf(" ");
		}
		printf(fmt, num);
		printf(",");
	}
	printf("\n};\n");
	if (addr) {
		char name[32];
		snprintf(name, sizeof(name), "%08x.bin", addr);

		FILE *out = fopen(name, "wb");
		if (out) {
			uint8_t buf[4096];
			int count;

			fseek(fp, offset, SEEK_SET);
			while (size > 0 && (count = fread(buf, 1, size > sizeof(buf) ? sizeof(buf) : size, fp)) > 0) {
				fwrite(buf, 1, count, out);
				size -= count;
			}

			fclose(out);
		}
	}
}

static void dumpZeroes(const char *name, int size, const char *fmt, int num) {
	int i;

	printf("%s[%d] = {", name, size);
	for (i = 0; i < size; ++i) {
		if ((i % 16) == 0) {
			printf("\n\t");
		}
		printf(fmt, num);
		printf(", ");
	}
	printf("\n};\n");
}

static int detectVersion(FILE *fp) {
	int i, count;
	uint32_t crc;
	uint8_t buf[1024];

	count = fread(buf, 1, sizeof(buf), fp);
	if (count <= 0) {
		fprintf(stderr, "Failed to read %ld bytes, ret %d\n", sizeof(buf), count);
		return -1;
	} else if (memcmp(buf, "MZ", 2) != 0) {
		fprintf(stderr, "Invalid signature '%02x%02x'\n", buf[0], buf[1]);
		return -1;
	}
	crc = crc32(0, Z_NULL, 0);
	crc = crc32(crc, buf, count);
	while (1) {
		count = fread(buf, 1, sizeof(buf), fp);
		if (count == 0) {
			break;
		} else if (count < 0) {
			fprintf(stderr, "Failed to read %ld bytes, ret %d\n", sizeof(buf), count);
			return -1;
		}
		crc = crc32(crc, buf, count);
	}
	for (i = 0; _versions[i].crc != 0; ++i) {
		if (crc == _versions[i].crc) {
			return _versions[i].version;
		}
	}
	fprintf(stderr, "Unknown version CRC32 0x%08x\n", crc);
	return -1;
}

int main(int argc, char* argv[]) {
	if (argc == 2) {
		FILE *fp = fopen(argv[1], "rb");
		if (fp) {

			const int version = detectVersion(fp);

			if (version == DEMO_FR_12) {

//				dumpBinary(fp, "Game::_transformBufferData1", 0x3EA78, 4536, "0x%02X", UNSIGNED_8BITS); // _benchmarkData1
//				dumpBinary(fp, "Game::_transformBufferData2", 0x3D960, 2152, "0x%02X", UNSIGNED_8BITS); // _benchmarkData2
				dumpBinary(fp, "_specialPowersDxDyTable", 0x3E660, 16, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_byte_43E670", 0x3E670, 16, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_byte_43E6E0", 0x3E6E0, 16, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_byte_43E6F0", 0x3E6F0, 16, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_byte_43E720", 0x3E720, 16, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_byte_43E730", 0x3E730, 16, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_byte_43E740", 0x3E740, 16, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_byte_43E750", 0x3E750, 16, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_byte_43E760", 0x3E6F0, 16, "0x%02X", UNSIGNED_8BITS, 0);
//				dumpBinary(fp, "_byte_43E8F8", 0x3E8F8, 68, "0x%02X", UNSIGNED_8BITS);
//				dumpBinary(fp, "_byte_43E940", 0x3E940, 68, "0x%02X", UNSIGNED_8BITS);
//				dumpBinary(fp, "_level1OpHelper1KeyMaskTable", 0x3E888, 112, "%d", UNSIGNED_8BITS);
//				dumpBinary(fp, "_andyMoveTable35", 0x40358, 48, "%3d", SIGNED_8BITS);
//				dumpBinary(fp, "_actionDirectionKeyMaskTable", 0x3E3D4, 352, "0x%02X", UNSIGNED_8BITS);
//				dumpBinary(fp, "_dbVolumeTable", 0x40010, 129, "0x%02X", UNSIGNED_8BITS);

//				dumpFontCharMap(fp, "_fontCharactersTable", 0x3FE88, 39);
			}

			if (version == FULL_FR_10) {

				dumpCheckpointData(fp, "_rock_checkpointData", 0x53060, 96);
//				dumpBinary(fp, "_level1UpdateData2", 0x530C0, 56, "0x%02X", UNSIGNED_8BITS);
				dumpCheckpointData(fp, "_fort_checkpointData", 0x4E940, 60);
//				dumpBinary(fp, "_level2UpdateData2", 0x58A50, 56, "0x%02X", UNSIGNED_8BITS);
				dumpCheckpointData(fp, "_pwr1_checkpointData", 0x517A8, 120);
//				dumpZeroes("_level3UpdateData2", 80, "0x%02X", 0);
				dumpCheckpointData(fp, "_isld_checkpointData", 0x4F408, 60);
//				dumpBinary(fp, "_level4UpdateData2", 0x58A88, 56, "0x%02X", UNSIGNED_8BITS);
				dumpCheckpointData(fp, "_lava_checkpointData", 0x50248, 72);
//				dumpBinary(fp, "_level5UpdateData2", 0x50290, 40, "0x%02X", UNSIGNED_8BITS);
				dumpCheckpointData(fp, "_pwr2_checkpointData", 0x51D68, 36);
//				dumpZeroes("_level6UpdateData2", 32, "0x%02X", 0);
				dumpCheckpointData(fp, "_lar1_checkpointData", 0x4F6F8, 108);
//				dumpBinary(fp, "_level7UpdateData2", 0x4F768, 56, "0x%02X", UNSIGNED_8BITS);
				dumpCheckpointData(fp, "_lar2_checkpointData", 0x4F8C0, 144);
//				dumpBinary(fp, "_level8UpdateData2", 0x58B28, 64, "0x%02X", UNSIGNED_8BITS);
				dumpCheckpointData(fp, "_dark_checkpointData", 0x4E850, 12);
//				dumpBinary(fp, "_level9UpdateData2", 0x58A40, 8, "0x%02X", UNSIGNED_8BITS);

				dumpBinary(fp, "byte_451248", 0x4EA48, 32, "0x%02X", UNSIGNED_8BITS, 0);

				dumpBinary(fp, "byte_452538", 0x4FD38,  4 / sizeof(uint16_t), "%d", SIGNED_16BITS, 0);
				dumpBinary(fp, "byte_452540", 0x4FD40,  6 / sizeof(uint16_t), "%d", SIGNED_16BITS, 0);
				dumpBinary(fp, "byte_452548", 0x4FD48,  8 / sizeof(uint16_t), "%d", SIGNED_16BITS, 0);
				dumpBinary(fp, "byte_452550", 0x4FD50, 10 / sizeof(uint16_t), "%d", SIGNED_16BITS, 0);
				dumpBinary(fp, "byte_452560", 0x4FD60, 10 / sizeof(uint16_t), "%d", SIGNED_16BITS, 0);
				dumpBinary(fp, "byte_452570", 0x4FD70, 14 / sizeof(uint16_t), "%d", SIGNED_16BITS, 0);

				dumpBinary(fp, "byte_4526D8", 0x4FED8, 32, "0x%02X", UNSIGNED_8BITS, 0);

				dumpBinary(fp, "Game::_transformBufferData1", 0x51DA8, 4536, "0x%02X", UNSIGNED_8BITS, 0x4545A8);
				dumpBinary(fp, "Game::_transformBufferData2", 0x50500, 2152, "0x%02X", UNSIGNED_8BITS, 0x452D00);
				dumpBinary(fp, "_lar1_unkData1", 0x4FD98, 90, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_lar1_unkData0", 0x4FDF8, 52, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "byte_452898", 0x50098, 52, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_lar2_unkData0", 0x500D0, 40, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "byte_4528F8", 0x500F8, 17, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_lar2_lutData", 0x50109, 39, "0x%02X", UNSIGNED_8BITS, 0);
				dumpRect(fp, "_lar1_bboxData", 0x4FB58, 24);
				dumpBinary(fp, "_lar1_unkData3", 0x4FCD8, 96, "0x%02X", UNSIGNED_8BITS, 0);
				dumpRect(fp, "_lar2_bboxData", 0x4FFC8, 13);
				dumpBinary(fp, "_lar2_unkData3", 0x50098, 52, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_pwr1_spritesData", 0x51820, 576, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_isld_spritesData", 0x4F448, 368, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_lava_spritesData", 0x50380, 288, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_lar1_spritesData", 0x4F9A8, 400, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "_lar1_setupScreen24Data", 0x4FE40, 24, "0x%02X", UNSIGNED_8BITS, 0);
				dumpBinary(fp, "byte_45279", 0x4FF98, 48, "0x%02X", UNSIGNED_8BITS, 0);
			}
			fclose(fp);
		}
	}
	return 0;
}
