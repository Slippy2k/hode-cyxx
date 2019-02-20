
#ifndef MIXER_H__
#define MIXER_H__

#include "3p/sts_mixer.h"

struct Mixer {

	static const int kPcmChannels = 16;

	void (*_lock)(int);
	sts_mixer_t _mixer;
	struct {
		sts_mixer_sample_t sample;
		int voice;
	} _channels[kPcmChannels];

	Mixer();
	~Mixer();

	void init(int rate);
	void fini();

	void playPcm(const uint8_t *data, int len, int frequency, int volume, int pan);
	void stopPcm(const uint8_t *data);

	void mix(int16_t *buf, int len);
};

#endif
