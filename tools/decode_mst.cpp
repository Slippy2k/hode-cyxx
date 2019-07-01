
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

enum {
	MAX_FILESIZE = 0x10000,
};

static FILE *_out = stdout;

static void (*visitOpcode)(uint32_t addr, const uint8_t *p);

static int16_t read16(const uint8_t *p) {
	return (p[1] << 8) | p[0];
}

static int32_t read32(const uint8_t *p) {
	const uint16_t lo = read16(p); p += 2;
	return lo | (read16(p) << 16);
}

enum {
	op_count = 243
};

static int _histogram[op_count];

static const char *_arithOp[] = {
	"=",
	"+=",
	"-=",
	"*=",
	"/=",
	"<<=",
	">>=",
	"&=",
	"|=",
	"^=",
	0
};

static const char *taskOtherVar(int num, char *buffer) {
	sprintf(buffer, "otherVar_%d", num);
	return buffer;
}

static const char *taskVar(int num, int type, char *buffer) {
	switch (type) {
	case 1:
		sprintf(buffer, "%d", num);
		break;
	case 2:
		sprintf(buffer, "task.localVars[%d]", num);
		break;
	case 3:
		sprintf(buffer, "mstVars[%d]", num);
		break;
	case 4:
		taskOtherVar(num, buffer);
		break;
	case 5:
		sprintf(buffer, "monster.localVars[%d]", num);
		break;
	}
	return buffer;
}

static void printMstOpcode56(uint8_t code, int num) {
	fprintf(_out, "special_action code:%d num:%d", code, num);
}

static void printMstOpcode(uint32_t addr, const uint8_t *p) {
	const uint8_t opcode = p[0];
	fprintf(_out, "%04X (%02X): ", addr, opcode);
	switch (opcode) {
	case 0:
	case 1: {
			char buffer[64];
			fprintf(_out, "sm_wait delay:%s", taskVar(read16(p + 2), p[1], buffer));
		}
		break;
	case 2:
		fprintf(_out, "set_var_random_range num:%d", read16(p + 2));
		break;
	case 3:
		fprintf(_out, "set_monster_action_direction_imm num:%d", read16(p + 2));
		break;
	case 13:
		fprintf(_out, "set_monster_action_direction_cond_imm num:%d", read16(p + 2));
		break;
	case 23:
		fprintf(_out, "set_flag_global bit:%d", p[1]);
		break;
	case 24:
		fprintf(_out, "set_flag_task bit:%d", p[1]);
		break;
	case 25:
		fprintf(_out, "set_flag_monster bit:%d", p[1]);
		break;
	case 26:
		fprintf(_out, "unset_flag_global bit:%d", p[1]);
		break;
	case 27:
		fprintf(_out, "unset_flag_task bit:%d", p[1]);
		break;
	case 28:
		fprintf(_out, "unset_flag_monster bit:%d", p[1]);
		break;
	case 33:
		fprintf(_out, "jmp_imm offset:0x%x", 4 * read16(p + 2));
		break;
	case 41:
		fprintf(_out, "++task.localVars[%d]", p[1]);
		break;
	case 42:
		fprintf(_out, "++mstVars[%d]", p[1]);
		break;
	case 47 ... 56:
		fprintf(_out, "task.localVars[%d] %s task.localVars[%d]", p[1], _arithOp[opcode - 47], p[2]);
		break;
	case 167 ... 176:
		fprintf(_out, "task.localVars[%d] %s %d", p[1], _arithOp[opcode - 167], read16(p + 2));
		break;
	case 177 ... 186:
		fprintf(_out, "mstVars[%d] %s %d", p[1], _arithOp[opcode - 177], read16(p + 2));
		break;
	case 187 ... 196:
		fprintf(_out, "monster.localVars[%d] %s %d", p[1], _arithOp[opcode - 187], read16(p + 2));
		break;
	case 202:
		fprintf(_out, "op54");
		break;
	case 204:
		printMstOpcode56(p[1], read16(p + 2));
		break;
	case 220 ... 225:
		fprintf(stdout, "add_monster num:%d", read16(p + 2));
		break;
	case 239:
		fprintf(_out, "create_task num:%d", read16(p + 2));
		break;
	case 242:
		fprintf(_out, "break");
		break;
	}
	fprintf(_out, "\n");
	++_histogram[opcode];
}

int main(int argc, char *argv[]) {
	if (argc == 2) {
		FILE *fp = fopen(argv[1], "rb");
		if (fp) {
			visitOpcode = printMstOpcode;
			uint32_t addr = 0;
			while (true) {
				uint8_t buf[4];
				if (fread(buf, 1, 4, fp) != 4) {
					break;
				}
				visitOpcode(addr, buf);
				addr += 4;
			}
			fclose(fp);
		}
		for (int i = 0; i < op_count; ++i) {
			if (_histogram[i] != 0) {
				fprintf(stdout, "opcode %d referenced %d times\n", i, _histogram[i]);
			}
		}
	}
	return 0;
}
