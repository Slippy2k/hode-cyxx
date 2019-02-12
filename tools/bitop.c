
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

static int bit_op(int dst_bit, int src_bit, int mask) {
	return ((src_bit ^ dst_bit) & mask) ^ dst_bit;
}

static int bitmask_set(int dst_bit, int src_bit, int mask) {
//	int lomask = ((src_bit ^ dst_bit) ^ dst_bit) & mask;
	int lomask = src_bit & mask;
	int himask = dst_bit & ~mask;
	return lomask | himask;
}

// _ebx is (un)initialized with some stack variable
static uint32_t xor_uninit_42ACF9(uint32_t flags, uint32_t _ebx) {
	uint32_t _eax = flags & 0xFF00F000;
	uint32_t _edx = (flags ^ _ebx) & 0x00F00000;
	_ebx ^= _edx;
	_ebx &= 0xF00000;
	_eax ^= _ebx;
	return _eax;
}

int main(int argc, char *argv[]) {
	int i, j, m, t;
	for (m = 1; m <= 15; ++m) {
		for (j = 0; j <= 7; ++j) {
			for (i = 0; i <= 7; ++i) {
				const int v = bit_op(j, i, m);
				const int t = bitmask_set(j, i, m);
				// printf("bit_op(%d,%d,%d) = %d (%d)\n", j, i, m, v, t);
				assert(v == t);
			}
		}
	}
	static const uint32_t mask = 0xFFFFFFFF;
	for (uint32_t i = 0; i < 0x0FFFF000; ++i) {
		assert(xor_uninit_42ACF9(mask, i) == (mask & 0xFFF0F000));
	}
	return 0;
}

// bit_op(0,0,1) = 0
// bit_op(0,1,1) = 1
// bit_op(1,0,1) = 0
// bit_op(1,1,1) = 1
// bit_op(0,0,2) = 0
// bit_op(0,1,2) = 0
// bit_op(1,0,2) = 1
// bit_op(1,1,2) = 1
// bit_op(0,0,3) = 0
// bit_op(0,1,3) = 1
// bit_op(1,0,3) = 0
// bit_op(1,1,3) = 1
