
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>

enum ExecutableVersion {
	DEMO_FR_12,
};

struct {
	uint32_t crc;
	int version;
} _versions[] = {
	{ 0x5429784f, DEMO_FR_12 },
	{ 0, -1 }
};

static enum ExecutableVersion g_exeVersion;

static uint8_t g_dseg[0x10000];

static uint16_t freadUint16LE(FILE *fp) {
	uint8_t buf[2];
	fread(buf, 1, sizeof(buf), fp);
	return buf[0] | (buf[1] << 8);
}

static uint32_t freadUint32LE(FILE *fp) {
	uint8_t buf[4];
	fread(buf, 1, sizeof(buf), fp);
	return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

static void dump_binary(FILE *fp, int offset, int count, const char *name, int size) {
	fprintf(stdout, "%s[] = {", name);
	fseek(fp, offset, SEEK_SET);
	for (int i = 0; i < count; ++i) {
		if (i != 0) {
			fprintf(stdout, ",");
		}
		if ((i % 16) == 0) {
			fprintf(stdout, "\n\t");
		} else {
			fprintf(stdout, " ");
		}
		switch (size) {
		case 1:
			fprintf(stdout, "0x%02X", fgetc(fp));
			break;
		case 2:
			fprintf(stdout, "0x%04X", freadUint16LE(fp));
			break;
		}
	}
	fprintf(stdout, "\n};\n");
}

static void dump_mst_opcodes_table(FILE *fp) {
	static const uint32_t DEFAULT_OPCODE_LUT_OFFS = 0x14188;
	static const uint32_t DEFAULT_OPCODE_LUT_SIZE = 244;

	dump_binary(fp, DEFAULT_OPCODE_LUT_OFFS, DEFAULT_OPCODE_LUT_SIZE, "_mstDefaultLutOp", 1);
}

static int detect_executable(FILE *fp) {
	int i, count;
	uint32_t crc;
	uint8_t buf[1024];

	count = fread(buf, 1, sizeof(buf), fp);
	if (count <= 0) {
		fprintf(stderr, "Failed to read %ld bytes, ret %d\n", sizeof(buf), count);
		return 0;
	} else if (memcmp(buf, "MZ", 2) != 0) {
		fprintf(stderr, "Invalid signature '%02x%02x'\n", buf[0], buf[1]);
		return 0;
	}
	crc = crc32(0, Z_NULL, 0);
	crc = crc32(crc, buf, count);
	while (1) {
		count = fread(buf, 1, sizeof(buf), fp);
		if (count == 0) {
			break;
		} else if (count < 0) {
			fprintf(stderr, "Failed to read %ld bytes, ret %d\n", sizeof(buf), count);
			return 0;
		}
		crc = crc32(crc, buf, count);
	}
	for (i = 0; _versions[i].crc != 0; ++i) {
		if (crc == _versions[i].crc) {
			g_exeVersion = _versions[i].version;
			return 1;
		}
	}
	fprintf(stderr, "Invalid CRC32 0x%08x\n", crc);
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc == 2) {
		FILE *fp = fopen(argv[1], "rb");
		if (fp) {
			if (detect_executable(fp)) {
				dump_mst_opcodes_table(fp);
			}
			fclose(fp);
		}
	}
	return 0;
}
