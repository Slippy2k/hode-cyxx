
#include "file.h"
#include "paf_decoder.h"

static PafDecoder d;

int main(int argc, char *argv[]) {
	if (argc >= 2) {
		int num = (argc >= 3) ? atoi(argv[2]) : -1;
		if (d.Open(argv[1], num)) {
			d.Decode();
			d.Close();
		} else if (num == -1) {
			File f;
			if (f.open(argv[1], "rb")) {
				const int offset = f.readUint32LE();
				if ((offset & 3) == 0 && (offset / 4) <= PafDecoder::kMaxVideosCount) {
					for (int i = 0; i < offset / 4; ++i) {
						fprintf(stdout, "Decoding PAF %d\n", i);
						PafDecoder d;
						if (d.Open(argv[1], i)) {
							d.Decode();
							d.Close();
						}
					}
				}
			}
		}
	} else {
		fprintf(stdout, "%s hod.paf [video num]\n", argv[0]);
	}
	return 0;
}
