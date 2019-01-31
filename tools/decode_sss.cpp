
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>

enum {
	MAX_FILESIZE = 0x10000,
};

static uint8_t _fileBuf[MAX_FILESIZE];

static FILE *_out = stdout;

static uint16_t read16(const uint8_t *p) {
	return (p[1] << 8) | p[0]; // LE
}

static uint32_t read32(const uint8_t *p) {
	const uint16_t lo = read16(p); p += 2;
	return lo | (read16(p) << 16);
}

enum {
	/* 0x00 */
	op_end,
	op_invalid0x01,
	op_startSound,
	op_invalid0x03,
	/* 0x04 */
	op_killSound,
	op_loopSound,
	op_jge,
	op_invalid0x07,
	/* 0x08 */
	op_unk0x08,
	op_adjustVolume,
	op_testVar0x54,
	op_unk0x0B,
	/* 0x0C */
	op_fadeInSound,
	op_unk0x0D,
	op_unk0x0E,
	op_unk0x0F,
	/* 0x10 */
	op_stopSound,
	op_fadeOutSound,
	op_setCounter,
	op_setVolume,
	/* 0x14 */
	op_unk0x14,
	op_decrementVar0x50,
	op_setVar0x50,
	op_decrementVar0x54,
	/* 0x18 */
	op_setVar0x54,
	op_decrementVar0x58,
	op_setVar0x58,
	op_unk0x1B,
	/* 0x1C */
	op_jmp,
	op_unk0x1D,

	op_count
};

static int _histogram[op_count];

static void printOpcode(uint16_t addr, uint8_t opcode, int args[16]) {
	fprintf(_out, "%04X: ", addr);
	switch (opcode) {
	case op_end:
		fprintf(_out, "// end");
		break;
	case op_startSound:
		fprintf(_out, "op_startSound %d", args[0]);
		break;
	case op_killSound:
		fprintf(_out, "op_killSound %d %d", args[0], args[1]);
		break;
	case op_loopSound:
		fprintf(_out, "op_loopSound %d %d", args[0], args[1]);
		break;
	case op_jge:
		fprintf(_out, "op_jge %d", args[0]);
		break;
	case op_unk0x08:
		fprintf(_out, "op_unk0x08 %d %d %d", args[0], args[1], args[2]);
		break;
	case op_adjustVolume:
		fprintf(_out, "op_adjustVolume");
		break;
	case op_testVar0x54:
		fprintf(_out, "op_testVar0x54");
		break;
	case op_unk0x0B:
		fprintf(_out, "op_unk0x0B %d %d", args[0], args[1]);
		break;
	case op_fadeInSound:
		fprintf(_out, "op_fadeInSound %d", args[0]);
		break;
	case op_unk0x0D:
		fprintf(_out, "op_unk0x0D");
		break;
	case op_unk0x0E:
		fprintf(_out, "op_unk0x0E");
		break;
	case op_stopSound:
		fprintf(_out, "op_startSound %d", args[0]);
		break;
	case op_fadeOutSound:
		fprintf(_out, "op_fadeOutSound %d", args[0]);
		break;
	case op_setCounter:
		fprintf(_out, "op_setCounter %d", args[0]);
		break;
	case op_setVolume:
		fprintf(_out, "op_adjustVolume %d", args[0]);
		break;
	case op_unk0x14:
		fprintf(_out, "op_unk0x14 %d", args[0]);
		break;
	case op_decrementVar0x50:
		fprintf(_out, "op_decrementVar0x50");
		break;
	case op_setVar0x50:
		fprintf(_out, "op_setVar0x50 %d", args[0]);
		break;
	case op_decrementVar0x54:
		fprintf(_out, "op_decrementVar0x54");
		break;
	case op_setVar0x54:
		fprintf(_out, "op_setVar0x54 %d", args[0]);
		break;
	case op_decrementVar0x58:
		fprintf(_out, "op_decrementVar0x58");
		break;
	case op_setVar0x58:
		fprintf(_out, "op_setVar0x58 %d", args[0]);
		break;
	case op_unk0x1B:
		fprintf(_out, "op_unk0x1B %d %d %d", args[0], args[1], args[2]);
		break;
	case op_jmp:
		fprintf(_out, "op_jmp %d", args[0]);
		break;
	case op_unk0x1D:
		fprintf(_out, "op_unk0x1D");
		break;
	}
	fprintf(_out, "\n");
}

static void (*visitOpcode)(uint16_t addr, uint8_t opcode, int args[4]);

static int parse(const uint8_t *buf, uint32_t size) {	
	const uint8_t *p = buf;
	while (p < buf + size) {
		const uint32_t addr = p - buf;
		int a, b, c, d;
		const uint8_t op = *p;
		++_histogram[op];
		a = b = c = d = 0;
		switch (op) {
		case op_end:
			p += 4;
			break;
		case op_startSound:
			p += 2;
			a = read16(p); p += 2;
			break;
		case op_killSound:
			a = p[1]; p += 2;
			b = read16(p); p += 2;
			break;
		case op_loopSound:
			p += 4;
			a = read32(p); p += 4;
			b = read32(p); p += 4;
			break;
		case op_jge:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op_unk0x08:
			p += 4;
			a = read32(p); p += 4;
			b = read32(p); p += 4;
			c = read32(p); p += 4;
			break;
		case op_adjustVolume:
			p += 4;
			break;
		case op_testVar0x54:
			p += 4;
			break;
		case op_unk0x0B:
			a = p[1]; p += 2;
			b = read16(p); p += 2;
			break;
		case op_fadeInSound:
			p += 2;
			a = read16(p); p += 2;
			break;
		case op_unk0x0D:
			p += 8;
			break;
		case op_unk0x0E:
			p += 12;
			break;
		case op_stopSound:
			p += 2;
			a = read16(p); p += 2;
			break;
		case op_fadeOutSound:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op_setCounter:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op_setVolume:
			p += 2;
			a = read16(p); p += 2;
			break;
		case op_unk0x14:
			p += 2;
			a = read16(p); p += 2;
			break;
		case op_decrementVar0x50:
			p += 4;
			break;
		case op_setVar0x50:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op_decrementVar0x54:
			p += 4;
			break;
		case op_setVar0x54:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op_decrementVar0x58:
			p += 4;
			break;
		case op_setVar0x58:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op_unk0x1B:
			p += 4;
			a = read32(p); p += 4;
			b = read32(p); p += 4;
			c = read32(p); p += 4;
			break;
		case op_jmp:
			p += 4;
			a = read32(p); p += 4;
			// p -= a;
			break;
		case op_unk0x1D:
			p += 4;
			break;
		default:
			fprintf(stdout, "Invalid opcode %d", op);
			return 0;
		}
		int args[4] = { a, b, c, d };
		visitOpcode(addr, op, args);
	}
	return 0;
}

static int readFile(const char *path) {
	int size = 0;
	FILE *fp = fopen(path, "rb");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		assert(size <= MAX_FILESIZE);
		const int count = fread(_fileBuf, 1, size, fp);
		if (count != size) {
			fprintf(stderr, "Failed to read %d bytes (%d)\n", size, count);
		}
		fclose(fp);
	}
	return size;
}

int main(int argc, char *argv[]) {
	if (argc == 2) {
		struct stat st;
		if (stat(argv[1], &st) == 0 && S_ISREG(st.st_mode)) {
			const int size = readFile(argv[1]);
			if (size != 0) {
				visitOpcode = printOpcode;
				parse(_fileBuf, size);
				for (int i = 0; i < op_count; ++i) {
					if (_histogram[i] == 0) {
						switch (i) {
						case op_invalid0x01:
						case op_invalid0x03:
						case op_invalid0x07:
							break;
						default:
							fprintf(stdout, "Opcode %d not referenced\n", i);
							break;
						}
					}
				}
			}
		}
	}
	return 0;
}
