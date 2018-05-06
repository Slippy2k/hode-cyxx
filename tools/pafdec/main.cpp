
#include "paf_decoder.h"

int main(int argc, char *argv[]) {
	if (argc >= 2) {
		int num = (argc >= 3) ? atoi(argv[2]) : -1;
		PafDecoder d;
		if (d.Open(argv[1], num)) {
			d.Decode();
			d.Close();
		}
	} else {
		fprintf(stdout, "%s hod.paf [video num]\n", argv[0]);
	}
	return 0;
}
