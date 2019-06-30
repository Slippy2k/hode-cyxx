
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

enum {
	MAX_FILESIZE = 0x10000,
};

static uint8_t _fileBuf[MAX_FILESIZE];

static FILE *_out = stdout;

static int16_t read16(const uint8_t *p) {
	return (p[1] << 8) | p[0];
}

static int32_t read32(const uint8_t *p) {
	const uint16_t lo = read16(p); p += 2;
	return lo | (read16(p) << 16);
}

enum {
	op00_end,
	op01_invalid,
	op02_addSound,
	op03_invalid,
	op04_removeSound,
	op05_seekForward,
	op06_repeatJge,
	op07_invalid,
	op08_seekBackward2,
	op09_modulatePanning,
	op0a_modulateVolume,
	op0b_setVolume,
	op0c_removeSounds2,
	op0d_initVolume,
	op0e_initPanning,
	op0f_invalid,
	op10_resumeSound,
	op11_pauseSound,
	op12_decrementRepeatCounter,
	op13_setPanning,
	op14_setPauseCounter,
	op15_decrementDelayCounter,
	op16_setDelayCounter,
	op17_decrementVolumeModulateSteps,
	op18_setVolumeModulateSteps,
	op19_decrementPanningModulateSteps,
	op1a_setPanningModulateSteps,
	op1b_seekBackward,
	op1c_jmp,
	op1d_terminate,

	op_count
};

static int _histogram[op_count];

static void printOpcode(uint16_t addr, uint8_t opcode, int args[16]) {
	fprintf(_out, "%04X: ", addr);
	switch (opcode) {
	case op00_end:
		fprintf(_out, "// end");
		break;
	case op02_addSound:
		fprintf(_out, "op02_add_sound %d num:%d", args[0], args[1]);
		break;
	case op04_removeSound:
		fprintf(_out, "op04_remove_sound %d num:%d", args[0], args[1]);
		break;
	case op05_seekForward:
		fprintf(_out, "op05_seek_forward frame:%d pos:%d", args[0], args[1]);
		break;
	case op06_repeatJge:
		fprintf(_out, "op06_repeat_jge offset:-%d", args[0]);
		break;
	case op08_seekBackward2:
		fprintf(_out, "op08_seek_backward2 pos:%d frame:%d frame:%d", args[0], args[1], args[2]);
		break;
	case op09_modulatePanning:
		fprintf(_out, "op09_modulate_panning");
		break;
	case op0a_modulateVolume:
		fprintf(_out, "op0a_modulate_volume");
		break;
	case op0b_setVolume:
		fprintf(_out, "op0b_set_volume %d", args[0]);
		break;
	case op0c_removeSounds2:
		fprintf(_out, "op0c_remove_sounds2 %d", args[0]);
		break;
	case op0d_initVolume:
		fprintf(_out, "op0d_init_volume value:%d steps:%d", args[0], args[1]);
		break;
	case op0e_initPanning:
		fprintf(_out, "op0e_init_panning value:%d steps:%d", args[0], args[1]);
		break;
	case op10_resumeSound:
		fprintf(_out, "op10_resume_sound %d", args[0]);
		break;
	case op11_pauseSound:
		fprintf(_out, "op11_pause_sound %d", args[0]);
		break;
	case op12_decrementRepeatCounter:
		fprintf(_out, "op12_decrement_repeat_counter %d", args[0]);
		break;
	case op13_setPanning:
		fprintf(_out, "op13_set_panning %d", args[0]);
		break;
	case op14_setPauseCounter:
		fprintf(_out, "op14_set_pause_counter %d", args[0]);
		break;
	case op15_decrementDelayCounter:
		fprintf(_out, "op15_decrement_delay_counter");
		break;
	case op16_setDelayCounter:
		fprintf(_out, "op16_setDelayCounter %d", args[0]);
		break;
	case op17_decrementVolumeModulateSteps:
		fprintf(_out, "op17_decrement_volume_modulate_steps");
		break;
	case op18_setVolumeModulateSteps:
		fprintf(_out, "op18_set_volume_modulate_steps %d", args[0]);
		break;
	case op19_decrementPanningModulateSteps:
		fprintf(_out, "op19_decrement_panning_modulates_steps");
		break;
	case op1a_setPanningModulateSteps:
		fprintf(_out, "op1a_set_panning_modulate_steps %d", args[0]);
		break;
	case op1b_seekBackward:
		fprintf(_out, "op08_seek_backward pos:%d frame:%d frame:%d", args[0], args[1], args[2]);
		break;
	case op1c_jmp:
		fprintf(_out, "op1c_jmp offset:-%d", args[0]);
		break;
	case op1d_terminate:
		fprintf(_out, "op1d_terminate");
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
		case op00_end:
			p += 4;
			break;
		case op02_addSound:
			a = p[1]; p += 2;
			b = read16(p); p += 2;
			break;
		case op04_removeSound:
			a = p[1]; p += 2;
			b = read16(p); p += 2;
			break;
		case op05_seekForward:
			p += 4;
			a = read32(p); p += 4;
			b = read32(p); p += 4;
			break;
		case op06_repeatJge:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op08_seekBackward2:
			p += 4;
			a = read32(p); p += 4;
			b = read32(p); p += 4;
			c = read32(p); p += 4;
			break;
		case op09_modulatePanning:
			p += 4;
			break;
		case op0a_modulateVolume:
			p += 4;
			break;
		case op0b_setVolume:
			a = p[1];
			p += 4;
			break;
		case op0c_removeSounds2:
			p += 2;
			a = read16(p); p += 2;
			break;
		case op0d_initVolume:
			p += 2;
			a = read16(p); p += 2;
			b = read32(p); p += 4;
			break;
		case op0e_initPanning:
			p += 2;
			a = read16(p); p += 2;
			b = read32(p); p += 4;
			c = read32(p); p += 4;
			break;
		case op10_resumeSound:
			p += 4;
			break;
		case op11_pauseSound:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op12_decrementRepeatCounter:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op13_setPanning:
			a = p[1]; p += 4;
			break;
		case op14_setPauseCounter:
			p += 2;
			a = read16(p); p += 2;
			break;
		case op15_decrementDelayCounter:
			p += 4;
			break;
		case op16_setDelayCounter:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op17_decrementVolumeModulateSteps:
			p += 4;
			break;
		case op18_setVolumeModulateSteps:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op19_decrementPanningModulateSteps:
			p += 4;
			break;
		case op1a_setPanningModulateSteps:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op1b_seekBackward:
			p += 4;
			a = read32(p); p += 4;
			b = read32(p); p += 4;
			c = read32(p); p += 4;
			break;
		case op1c_jmp:
			p += 4;
			a = read32(p); p += 4;
			break;
		case op1d_terminate:
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
	if (argc >= 2) {
		for (int i = 1; i < argc; ++i) {
			const int size = readFile(argv[i]);
			if (size != 0) {
				visitOpcode = printOpcode;
				parse(_fileBuf, size);
				for (int i = 0; i < op_count; ++i) {
					if (_histogram[i] == 0) {
						switch (i) {
						case op01_invalid:
						case op03_invalid:
						case op07_invalid:
						case op0f_invalid:
							break;
						default:
							fprintf(stdout, "Opcode %d not called\n", i);
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
