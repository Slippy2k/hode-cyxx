
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

extern int test_eq_15(int) __attribute__((stdcall));
extern int test_abs(int) __attribute__((stdcall));
extern int test_0x1485F0E1(int,int) __attribute__((stdcall));
extern int test_0xBA2E8BA3(int,int) __attribute__((stdcall));
extern int test_negsbb(int) __attribute__((stdcall));
extern int test_andnegsbb(int) __attribute__((stdcall));
extern int test_mullongintlongint(int,int,int,int) __attribute__((stdcall));
extern int test_cdq(int) __attribute__((stdcall));

static int test_eq_15__C(int i) {
	return i > 15 ? 1 : 0;
}

int main(int argc, char *argv[]) {
	int i;
	for (i = 0; i < 16; ++i) {
		assert(test_abs(-i) == i);
	}
	for (i = 0; i < 256; ++i) {
		const int test_15 = test_eq_15(i) ? 1 : 0;
		assert((test_eq_15__C(i) ^ test_15) == 0);
	}
	for (i = 0; i < 64 * 128; ++i) {
		const int base = 100;
		const int ptr = base + i * 948;
		assert(test_0x1485F0E1(ptr, base) == i);
	}
	for (i = 0; i < 64 * 256; ++i) {
		const int base = 100;
		const int ptr = base + i * 44;
		assert(test_0xBA2E8BA3(ptr, base) == i);
	}
	for (i = 0; i < 255; ++i) {
		const int res = (i != 0) ? -1 : 0;
		assert(test_negsbb(i) == res);
	}
	assert(test_andnegsbb(2) == -512);
	assert(test_andnegsbb(1) ==  512);
	i = test_mullongintlongint(1, 0, 0, 4);
	fprintf(stdout, "mul %d\n", i);
	for (int i = -512; i < 512; ++i) {
		assert(test_cdq(i) == i / 4);
	}
	return 0;
}
