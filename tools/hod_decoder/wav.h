
#ifndef WAV_H__
#define WAV_H__

#include <stdint.h>

void saveWAV(const char *filename, int rate, int bits, int channels, const uint8_t *samples, int size);

#endif
