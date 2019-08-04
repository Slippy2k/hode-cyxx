
#include <stdio.h>
#include <stdint.h>

extern void gen1(void);
extern unsigned char mstLookupTable1[];
extern unsigned char mstLookupTable2[];
extern void gen3(void);
extern unsigned char mstLookupTable3[];
extern void gen4(void);
extern unsigned char mstLookupTable4[];
extern void gen5(void);
extern unsigned char mstLookupTable5[];

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
		mstLookupTable1[i] = _eax & 255;
	}
}

int main(int argc, char *argv[]) {

	printf("LUT1\n");
	gen1();
	dump(mstLookupTable1, 16);
	printf("\n");
//	gen1_C();
//	dump(mstLookupTable1, 16);
//	printf("\n");

	printf("LUT4\n");
	gen4();
	dump(mstLookupTable4, 18);
	printf("\n");

	printf("LUT5\n");
	gen5();
	dump(mstLookupTable5, 32);
	printf("\n");

	mstLookupTable2[0] = 1;
	mstLookupTable2[1] = 3;
	mstLookupTable2[2] = 2;
	mstLookupTable2[3] = 6;
	mstLookupTable2[4] = 4;
	mstLookupTable2[5] = 12;
	mstLookupTable2[6] = 8;
	mstLookupTable2[7] = 9;
	mstLookupTable2[8] = 0;

	printf("LUT3\n");
	gen3();
	dump(mstLookupTable3, 40);
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
LUT3
0x01, 0x03, 0x09, 0x02, 0x08, 0x03, 0x02, 0x01, 0x06, 0x09, 0x02, 0x06, 0x03, 0x04, 0x01, 0x06, 0x04, 0x02, 0x0C, 0x03, 0x04, 0x0C, 0x06, 0x08, 0x02, 0x0C, 0x08, 0x04, 0x09, 0x06, 0x08, 0x09, 0x0C, 0x01, 0x04, 0x09, 0x01, 0x08, 0x03, 0x0C,
#endif
