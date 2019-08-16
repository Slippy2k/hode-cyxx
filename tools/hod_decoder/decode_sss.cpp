
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

static FILE *_out = stdout;

static uint32_t (*visitOpcode)(uint32_t addr, const uint8_t *p);

static int16_t read16(const uint8_t *p) {
	return (p[1] << 8) | p[0];
}

static int32_t read32(const uint8_t *p) {
	const uint16_t lo = read16(p); p += 2;
	return lo | (read16(p) << 16);
}

enum {
	op_count = 30
};

static int _histogram[op_count];

static uint32_t printSssOpcode(uint32_t offset, const uint8_t *p) {
	int opcode = p[offset];
	fprintf(_out, "%04X (%02X): ", offset, opcode);
	switch (opcode) {
	case 0x00:
		fprintf(_out, "// end\n");
		offset += 4;
		break;
	case 0x02:
		fprintf(_out, "add_sound %d num:%d", read16(p + offset + 2), (int8_t)p[offset + 1]);
		offset += 4;
		break;
	case 0x04:
		fprintf(_out, "remove_sound %d num:%d", read16(p + offset + 2), p[offset + 1]);
		offset += 4;
		break;
	case 0x05:
		fprintf(_out, "seek_forward keyframe:%d pos:%d", read32(p + offset + 4), read32(p + offset + 8));
		offset += 12;
		break;
	case 0x06:
		fprintf(_out, "repeat_jge offset:-%d", read32(p + offset + 4));
		offset += 8;
		break;
	case 0x08:
		fprintf(_out, "seek_backward_delay pos:%d frame:%d keyframe:%d", read32(p + offset + 4), read32(p + offset + 8), read32(p + offset + 12));
		offset += 16;
		break;
	case 0x09:
		fprintf(_out, "modulate_panning");
		offset += 4;
		break;
	case 0x0A:
		fprintf(_out, "modulate_volume");
		offset += 4;
		break;
	case 0x0B:
		fprintf(_out, "set_volume %d", p[offset + 1]);
		offset += 4;
		break;
	case 0x0C:
		fprintf(_out, "remove_sounds2 flags:0x%x", read16(p + offset + 2));
		offset += 4;
		break;
	case 0x0D:
		fprintf(_out, "init_volume_modulation value:%d steps:%d", read16(p + offset + 2), read32(p + offset + 4));
		offset += 8;
		break;
	case 0x0E:
		fprintf(_out, "init_panning_modulation value:%d steps:%d", read16(p + offset + 2), read32(p + offset + 8));
		offset += 12;
		break;
	case 0x10:
		fprintf(_out, "resume_sound");
		offset += 4;
		break;
	case 0x11:
		fprintf(_out, "pause_sound counter:%d", read32(p + offset + 4));
		offset += 8;
		break;
	case 0x12:
		fprintf(_out, "decrement_repeat_counter %d", read32(p + offset + 4));
		offset += 8;
		break;
	case 0x13:
		fprintf(_out, "set_panning %d", p[offset + 1]);
		offset += 4;
		break;
	case 0x14:
		fprintf(_out, "set_pause_counter %d", read16(p + offset + 2));
		offset += 4;
		break;
	case 0x15:
		fprintf(_out, "decrement_delay_counter");
		offset += 4;
		break;
	case 0x16:
		fprintf(_out, "set_delay_counter %d", read32(p + offset + 4));
		offset += 8;
		break;
	case 0x17:
		fprintf(_out, "decrement_volume_modulate_steps");
		offset += 4;
		break;
	case 0x18:
		fprintf(_out, "set_volume_modulate_steps %d", read32(p + offset + 4));
		offset += 8;
		break;
	case 0x19:
		fprintf(_out, "decrement_panning_modulate_steps");
		offset += 4;
		break;
	case 0x1A:
		fprintf(_out, "set_panning_modulate_steps %d", read32(p + offset + 4));
		offset += 8;
		break;
	case 0x1B:
		fprintf(_out, "seek_backward pos:%d frame:%d keyframe:%d", read32(p + offset + 4), read32(p + offset + 8), read32(p + offset + 12));
		offset += 16;
		break;
	case 0x1C:
		fprintf(_out, "jmp offset:-%d", read32(p + offset + 4));
		offset += 8;
		break;
	case 0x1D:
		fprintf(_out, "terminate");
		offset += 4;
		break;
	}
	fprintf(_out, "\n");
	++_histogram[opcode];
	return offset;
}

void DecodeSssCode(const uint8_t *codeData, uint32_t codeSize) {
	visitOpcode = printSssOpcode;
	uint32_t offset = 0;
	while (offset < codeSize) {
		offset = visitOpcode(offset, codeData);
	}
}
