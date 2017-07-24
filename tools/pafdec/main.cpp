
#include "paf_decoder.h"

#undef main
int main(int argc, char *argv[]) {
	if (argc >= 2) {
		int num = (argc >= 3) ? atoi(argv[2]) : -1;
		PafDecoder d;
		if (d.Open(argv[1], num)) {
			d.Decode();
			d.Close();
		}
	}
	return 0;
}
