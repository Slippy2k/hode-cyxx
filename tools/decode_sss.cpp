
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
	op_addSound,
	op_invalid0x03,
	/* 0x04 */
	op_removeSound,
	op_seekSound,
	op_jge,
	op_invalid0x07,
	/* 0x08 */
	op_seekSound2,
	op_modulatePan,
	op_modulateVolume,
	op_setVolume,
	/* 0x0C */
	op_removeSounds2,
	op_initVolume,
	op_initPan,
	op_invalid0x0F,
	/* 0x10 */
	op_resumeSound,
	op_pauseSound,
	op_setCounter,
	op_setPan,
	/* 0x14 */
	op_unk0x14,
	op_decrementVar0x50,
	op_setVar0x50,
	op_decrementVar0x54,
	/* 0x18 */
	op_setVar0x54,
	op_decrementVar0x58,
	op_setVar0x58,
	op_seekSound2_,
	/* 0x1C */
	op_jmp,
	op_terminate,

	op_count
};

static int _histogram[op_count];

static void printOpcode(uint16_t addr, uint8_t opcode, int args[16]) {
	fprintf(_out, "%04X: ", addr);
	switch (opcode) {
	case op_end:
		fprintf(_out, "// end");
		break;
	case op_addSound:
		fprintf(_out, "op_addSound num:%d", args[0]);
		break;
	case op_removeSound:
		fprintf(_out, "op_removeSound %d num:%d", args[0], args[1]);
		break;
	case op_seekSound:
		fprintf(_out, "op_seekSound %d pos:%d", args[0], args[1]);
		break;
	case op_jge:
		fprintf(_out, "op_jge offset:%d", args[0]);
		break;
	case op_seekSound2:
		fprintf(_out, "op_seekSound2 %d pos:%d %d", args[0], args[1], args[2]);
		break;
	case op_modulatePan:
		fprintf(_out, "op_modulatePan");
		break;
	case op_modulateVolume:
		fprintf(_out, "op_modulateVolume");
		break;
	case op_setVolume:
		fprintf(_out, "op_setVolume %d", args[0]);
		break;
	case op_removeSounds2:
		fprintf(_out, "op_removeSounds2 %d", args[0]);
		break;
	case op_initVolume:
		fprintf(_out, "op_initVolume value:%d steps:%d", args[0], args[1]);
		break;
	case op_initPan:
		fprintf(_out, "op_initPan value:%d steps:%d", args[0], args[1]);
		break;
	case op_resumeSound:
		fprintf(_out, "op_resumeSound %d", args[0]);
		break;
	case op_pauseSound:
		fprintf(_out, "op_pauseSound %d", args[0]);
		break;
	case op_setCounter:
		fprintf(_out, "op_setCounter %d", args[0]);
		break;
	case op_setPan:
		fprintf(_out, "op_setPan %d", args[0]);
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
	case op_seekSound2_:
		fprintf(_out, "op_seekSound2 %d %d %d", args[0], args[1], args[2]);
		break;
	case op_jmp:
		fprintf(_out, "op_jmp %d", args[0]);
		break;
	case op_terminate:
		fprintf(_out, "op_terminate");
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
		case op_addSound:
			p += 2;
			a = read16(p); p += 2;
			break;
		case op_removeSound:
			a = p[1]; p += 2;
			b = read16(p); p += 2;
			break;
		case op_seekSound:
			p += 4;
			a = read32(p); p += 4;
			b = read32(p); p += 4;
			break;
		case op_jge:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op_seekSound2:
			p += 4;
			a = read32(p); p += 4;
			b = read32(p); p += 4;
			c = read32(p); p += 4;
			break;
		case op_modulatePan:
			p += 4;
			break;
		case op_modulateVolume:
			p += 4;
			break;
		case op_setVolume:
			a = p[1];
			p += 4;
			break;
		case op_removeSounds2:
			p += 2;
			a = read16(p); p += 2;
			break;
		case op_initVolume:
			p += 2;
			a = read16(p); p += 2;
			b = read32(p); p += 4;
			break;
		case op_initPan:
			p += 2;
			a = read16(p); p += 2;
			b = read32(p); p += 4;
			c = read32(p); p += 4;
			break;
		case op_resumeSound:
			p += 2;
			a = read16(p); p += 2;
			break;
		case op_pauseSound:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op_setCounter:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op_setPan:
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
		case op_seekSound2_:
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
		case op_terminate:
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

static void opcodesLength() {
	uint8_t len[op_count];
	for (int i = 0; i < op_count; ++i) {
		switch (i) {
		case 0:
		case 2:
		case 4:
		case 9:
		case 10:
		case 11:
		case 12:
		case 16:
		case 19:
		case 20:
		case 21:
		case 23:
		case 25:
		case 29:
			len[i] = 4;
			break;
		case 6:
		case 13:
		case 17:
		case 18:
		case 22:
		case 24:
		case 26:
		case 28:
			len[i] = 8;
			break;
		case 5:
		case 14:
			len[i] = 12;
			break;
		case 8:
		case 27:
			len[i] = 16;
			break;
		default: // invalid opcode
			len[i] = 0;
			break;
		}
	}
	for (int i = 0; i < op_count; ++i) {
		fprintf(stdout, "%d, ", len[i]);
	}
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
						case op_invalid0x0F:
							break;
						default:
							fprintf(stdout, "Opcode %d not referenced\n", i);
							break;
						}
					}
				}
			}
		}
	} else {
		opcodesLength();
	}
	return 0;
}
