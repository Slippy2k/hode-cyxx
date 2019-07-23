
#include "fileio.h"

void saveWAV(const char *filename, int rate, int bits, int channels, const uint8_t *samples, int size) {

	FILE *fp = fopen(filename, "wb");
	if (fp) {

		const uint32_t riffSize = 4 + 8 + 16 /* 'fmt ' */ + 8 + size; /* 'data' */

		fwrite("RIFF", 1, 4, fp);
		fwriteUint32LE(fp, riffSize);
		fwrite("WAVE", 1, 4, fp);

		fwrite("fmt ", 1, 4, fp);
		fwriteUint32LE(fp, 16);
		fwriteUint16LE(fp, 1); // AudioFormat PCM
		fwriteUint16LE(fp, channels);
		fwriteUint32LE(fp, rate);
		fwriteUint32LE(fp, rate * channels * bits / 8); // ByteRate
		fwriteUint16LE(fp, channels * bits / 8); // BlockAlign
		fwriteUint16LE(fp, bits);

		fwrite("data", 1, 4, fp);
		fwriteUint32LE(fp, size);
		if (bits == 16) {
			assert((size & 1) == 0);
			for (int i = 0; i < size; i += 2) {
				fwriteUint16LE(fp, *(uint16_t *)(samples + i));
			}
		} else {
			assert(bits == 8);
			fwrite(samples, 1, size, fp);
		}

		fclose(fp);
	}
}
