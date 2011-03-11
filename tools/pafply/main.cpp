
#include "paf_player.h"
#include "paf_decoder.h"

#undef main
int main(int argc, char *argv[]) {
	if (argc >= 2) {
		int num = (argc >= 3) ? atoi(argv[2]) : -1;
#if 1
		PAF_Player p;
		if (p.open(argv[1], num)) {
			p.play();
			p.close();
		}
#else
		PafDecoder d;
		if (d.Open(argv[1], num)) {
			d.Decode();
			d.Close();
		}
#endif
	}
	return 0;
}
