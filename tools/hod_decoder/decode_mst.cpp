
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

static FILE *_out = stdout;

static void (*visitOpcode)(uint32_t addr, const uint8_t *p);

static int16_t read16(const uint8_t *p) {
	return (p[1] << 8) | p[0];
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

static const char *_compareOp[] = {
	"!=",
	"==",
	">=",
	">",
	"<=",
	"<",
	"&",
	"|"
	"^",
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
		sprintf(buffer, "task.vars[%d]", num);
		break;
	case 3:
		sprintf(buffer, "global.vars[%d]", num);
		break;
	case 4:
		taskOtherVar(num, buffer);
		break;
	case 5:
		sprintf(buffer, "monster.vars[%d]", num);
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
	case 8:
		fprintf(_out, "set_monster_action_direction_imm num:%d", read16(p + 2));
		break;
	case 4:
		fprintf(_out, "set_monster_action_direction_task_var num:%d", read16(p + 2));
		break;
	case 5:
		fprintf(_out, "set_monster_action_direction_global_var num:%d", read16(p + 2));
		break;
	case 6:
		fprintf(_out, "set_monster_action_direction_other_var num:%d", read16(p + 2));
		break;
	case 7:
		fprintf(_out, "set_monster_action_direction_monster_var num:%d", read16(p + 2));
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
	case 29:
		fprintf(_out, "sm_wait andy_var num:%d", p[1]);
		break;
	case 30:
		fprintf(_out, "sm_wait flag_global bit:%d", p[1]);
		break;
	case 32:
		fprintf(_out, "sm_wait flag_monster bit:%d", p[1]);
		break;
	case 33:
	case 229:
		fprintf(_out, "jmp_imm offset:0x%x", 4 * read16(p + 2));
		break;
	case 34:
		fprintf(_out, "nop");
		break;
	case 35:
		fprintf(_out, "enable_trigger num:%d", read16(p + 2));
		break;
	case 36:
		fprintf(_out, "disable_trigger num:%d", read16(p + 2));
		break;
	case 39:
		fprintf(_out, "remove_monsters_screen screen:%d", p[1]);
		break;
	case 40:
		fprintf(_out, "remove_monsters_screen_flags screen:%d", p[1]);
		break;
	case 41:
		fprintf(_out, "++task.vars[%d]", p[1]);
		break;
	case 42:
		fprintf(_out, "++global.vars[%d]", p[1]);
		break;
	case 43:
		fprintf(_out, "++monster.vars[%d]", p[1]);
		break;
	case 44:
		fprintf(_out, "--local.vars[%d]", p[1]);
		break;
	case 45:
		fprintf(_out, "--global.vars[%d]", p[1]);
		break;
	case 46:
		fprintf(_out, "--monster.vars[%d]", p[1]);
		break;
	case 47 ... 56:
		fprintf(_out, "task.vars[%d] %s task.vars[%d]", p[1], _arithOp[opcode - 47], p[2]);
		break;
	case 57 ... 66:
		fprintf(_out, "global.vars[%d] %s task.vars[%d]", p[1], _arithOp[opcode - 57], p[2]);
		break;
	case 67 ... 76:
		fprintf(_out, "monster.vars[%d] %s task.vars[%d]", p[1], _arithOp[opcode - 67], p[2]);
		break;
	case 77 ... 86:
		fprintf(_out, "task.vars[%d] %s monster.vars[%d]", p[1], _arithOp[opcode - 77], p[2]);
		break;
	case 87 ... 96:
		fprintf(_out, "global.vars[%d] %s monster.vars[%d]", p[1], _arithOp[opcode - 87], p[2]);
		break;
	case 97 ... 106:
		fprintf(_out, "monster.vars[%d] %s monster.vars[%d]", p[1], _arithOp[opcode - 97], p[2]);
		break;
	case 107 ... 116:
		fprintf(_out, "task.vars[%d] %s global.vars[%d]", p[1], _arithOp[opcode - 107], p[2]);
		break;
	case 117 ... 126:
		fprintf(_out, "global.vars[%d] %s global.vars[%d]", p[1], _arithOp[opcode - 117], p[2]);
		break;
	case 127 ... 136:
		fprintf(_out, "monster.vars[%d] %s global.vars[%d]", p[1], _arithOp[opcode - 127], p[2]);
		break;
	case 137 ... 146: {
			char buffer[64];
			fprintf(_out, "task.vars[%d] %s %s", p[1], _arithOp[opcode - 137], taskOtherVar(p[2], buffer));
		}
		break;
	case 147 ... 156: {
			char buffer[64];
			fprintf(_out, "global.vars[%d] %s %s", p[1], _arithOp[opcode - 147], taskOtherVar(p[2], buffer));
		}
		break;
	case 157 ... 166: {
			char buffer[64];
			fprintf(_out, "monster.vars[%d] %s %s", p[1], _arithOp[opcode - 157], taskOtherVar(p[2], buffer));
		}
		break;
	case 167 ... 176:
		fprintf(_out, "task.vars[%d] %s %d", p[1], _arithOp[opcode - 167], read16(p + 2));
		break;
	case 177 ... 186:
		fprintf(_out, "global.vars[%d] %s %d", p[1], _arithOp[opcode - 177], read16(p + 2));
		break;
	case 187 ... 196:
		fprintf(_out, "monster.vars[%d] %s %d", p[1], _arithOp[opcode - 187], read16(p + 2));
		break;
	case 197:
		fprintf(_out, "set_moving_bounds num:%d", read16(p + 2));
		break;
	case 198:
		fprintf(_out, "child_task num:%d", read16(p + 2));
		break;
	case 199:
		fprintf(_out, "stop_current_monster_object");
		break;
	case 200:
		fprintf(_out, "op52");
		break;
	case 201:
		fprintf(_out, "op51");
		break;
	case 202:
		fprintf(_out, "op54");
		break;
	case 203:
		fprintf(_out, "op55");
		break;
	case 204:
		printMstOpcode56(p[1], read16(p + 2));
		break;
	case 207:
	case 208:
	case 209:
		fprintf(_out, "nop");
		break;
	case 211:
		fprintf(_out, "add_lvl_object num:%d", read16(p + 2));
		break;
	case 212:
		fprintf(_out, "op59 type:%d dx:%d dy:%d", p[1], (int8_t)p[2], (int8_t)p[3]);
		break;
	case 213: {
			char buffer1[64];
			taskVar(p[2], p[1] >> 4, buffer1);
			char buffer2[64];
			taskVar(p[3], p[1] & 15, buffer2);
			fprintf(_out, "monster_set_action_direction am:%s dm:%s", buffer1, buffer2);
		}
		break;
	case 214:
		fprintf(_out, "reset_monster_energy");
		break;
	case 215:
		fprintf(_out, "shuffle m43 num:%d", read16(p + 2));
		break;
	case 217:
		fprintf(_out, "shuffle m43 num:%d", read16(p + 2));
		break;
	case 218:
		fprintf(_out, "shuffle m43 num:%d", read16(p + 2));
		break;
	case 220 ... 225:
		fprintf(_out, "add_monster num:%d", read16(p + 2));
		break;
	case 226:
		fprintf(_out, "add_monster_group num:%d", read16(p + 2));
		break;
	case 227:
		fprintf(_out, "compare_vars num:%d", read16(p + 2));
		break;
	case 228:
		fprintf(_out, "compare_flags num:%d", read16(p + 2));
		break;
	case 231:
		fprintf(_out, "sm_wait flag_task num:%d", read16(p + 2));
		break;
	case 232:
		fprintf(_out, "sm_not_wait flag_task num:%d", read16(p + 2));
		break;
	case 233:
		fprintf(_out, "sm_not_flags num:%d", read16(p + 2));
		break;
	case 234:
		fprintf(_out, "sm_flags num:%d", read16(p + 2));
		break;
	case 237:
		fprintf(_out, "remove_monster_task");
		break;
	case 238:
		fprintf(_out, "jmp num:%d", read16(p + 2));
		break;
	case 239:
		fprintf(_out, "create_task num:%d", read16(p + 2));
		break;
	case 240:
		fprintf(_out, "update_task num:%d", read16(p + 2));
		break;
	case 242:
		fprintf(_out, "break\n");
		break;
	}
	fprintf(_out, "\n");
	++_histogram[opcode];
}

void DecodeMstCode(const uint8_t *codeData, uint32_t codeSize) {
	visitOpcode = printMstOpcode;
	for (uint32_t addr = 0; addr < codeSize; addr += 4) {
		visitOpcode(addr, codeData + addr);
	}
}
