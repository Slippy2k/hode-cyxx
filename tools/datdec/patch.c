
#include <stdio.h>
#include <stdlib.h>

#define FILE1 "L:/games/Heart Of Darkness Demo/_HODWin32.exe"
#define FILE2 "J:/prog/hode/DATA/rock_hod.lvl"
#define FILE3 "L:/games/Heart Of Darkness Demo/HODWin32.exe"

int main(int argc, char *argv[]) {
	FILE *fp1, *fp2, *fp3;
	int n, i, k;
	int flag = 0;
	char buf[2048];
	
	n = 6688; //0x6638 - 0x60C;
	
	fp1 = fopen(FILE1, "rb");	
	fp2 = fopen(FILE2, "rb");
	fp3 = fopen(FILE3, "wb");
	
	for (i = 0; i < 311296; ++i) {
		char c = fgetc(fp1);
		if (i == 0x3D960) {
			flag = 1;
			fseek(fp2, 0xE860C, SEEK_SET);
		}
		if (flag) {
			c = fgetc(fp2);
		}
		fputc(c, fp3);
		if (flag) {
			--n;
			if (n == 0) flag = 0;
		}
	}





/*	
	printf("size begin = 0x%X (%d ko)\n", 0x3D960 + n, n / 1024);
	while (n > 0) {311 296
		i = n > 2048 ? 2048 : n;
		fread(buf, i, 1, fp2);
//		fwrite(buf, i, 1, fp1);
		k = 0;
		while (k < 10) {
			fputc(buf[k], fp1);
			++k;
		}
		n -= i;
	}*/
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	return 0;	
}
