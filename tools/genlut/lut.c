
#include <stdio.h>

extern void gen1(void);
extern unsigned char unkLookupTable1[];
extern void gen4(void);
extern unsigned char unkLookupTable4[];
extern void gen5(void);
extern unsigned char unkLookupTable5[];

static void dump(const unsigned char *p, int sz) {
	int i;
	for (i = 0; i < sz; ++i) {
		printf("0x%02X, ", p[i]);
	}
}

void gen1_C() {
	int i, _eax;
	for (i = 0; i <= 15; ++i) {
		_eax = 8;
		if (i & 1) {
			if (i & 2) {
				_eax = 1;
			} else {
//				_eax &= 8;
//				neg al
//				sbb eax, eax
//				eax &= 7;
				_eax = 0xFF;
			}
		} else {
			if (i & 4) {
				if (i & 2) {
					_eax = 3;
				} else {
					_eax &= 8;
					_eax |= 0x20;
					_eax >>= 3;
				}
			} else {
				if (i & 2) {
					_eax = 2;
				} else if (i & 8) {
					_eax = 6;
				}
			}
		}
		unkLookupTable1[i] = _eax & 255;
	}
}


int main(int argc, char *argv[]) {

	printf("LUT1\n");
	gen1();
	dump(unkLookupTable1, 16);
	printf("\n");
//	gen1_C();
//	dump(unkLookupTable1, 16);
//	printf("\n");

	// LUT2&3 not used

	printf("LUT4\n");
	gen4();
	dump(unkLookupTable4, 18);
	printf("\n");

	printf("LUT5\n");
	gen5();
	dump(unkLookupTable5, 32);
	printf("\n");
	return 0;
}

#if 0
LUT1
0x08, 0x00, 0x02, 0x01, 0x04, 0x00, 0x03, 0x01, 0x06, 0x07, 0x02, 0x01, 0x05, 0x07, 0x03, 0x01,
LUT4
0x01, 0x02, 0x03, 0x05, 0x06, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13, 0x15, 0x16, 0x17,
LUT5
0x00, 0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0A, 0x0B, 0x0C, 0x0C, 0x0D, 0x0E, 0x0F, 0x0F, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17,
#endif
