
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mixer.h"

#define STS_MIXER_IMPLEMENTATION
#include "3p/sts_mixer.h"

static const float kGain = 1.f;
static const float kPitch = 1.f;
static const float kPan = 0.f;

static void nullMixerLock(int lock) {
}

struct MixerLock {
	void (*_lock)(int);
	MixerLock(void (*lock)(int))
		: _lock(lock) {
		_lock(1);
	}
	~MixerLock() {
		_lock(0);
	}
};

Mixer::Mixer()
	: _lock(nullMixerLock) {
}

Mixer::~Mixer() {
}

void Mixer::init(int rate) {
	MixerLock ml(_lock);
	sts_mixer_init(&_mixer, rate, STS_MIXER_SAMPLE_FORMAT_16);
	memset(_channels, 0, sizeof(_channels));
	for (int i = 0; i < kPcmChannels; ++i) {
		_channels[i].voice = -1;
	}
	memset(_mixingQueue, 0, sizeof(_mixingQueue));
	_mixingQueueSize = 0;
}

void Mixer::fini() {
	MixerLock ml(_lock);
	sts_mixer_shutdown(&_mixer);
}

void Mixer::playPcm(const uint8_t *data, int len, int frequency, int volume, int pan) {
	MixerLock ml(_lock);
	// update channels state
	for (int i = 0; i < kPcmChannels; ++i) {
		if (_channels[i].sample.data) {
			const int voice = _channels[i].voice;
			if (!(voice < 0) && _mixer.voices[voice].state == STS_MIXER_VOICE_STOPPED) {
				memset(&_channels[i], 0, sizeof(sts_mixer_sample_t));
				_channels[i].voice = -1;
			}			
		}
	}
	// find a free channel
	int channel = -1;
	for (int i = 0; i < kPcmChannels; ++i) {
		if (_channels[i].sample.data == 0) {
			channel = i;
			break;
		}
	}
	if (!(channel < 0)) {
		_channels[channel].sample.length = len / sizeof(int16_t);
		_channels[channel].sample.frequency = frequency;
		_channels[channel].sample.audio_format = STS_MIXER_SAMPLE_FORMAT_16;
		_channels[channel].sample.data = (void *)data;
		_channels[channel].voice = sts_mixer_play_sample(&_mixer, &_channels[channel].sample, kGain, kPitch, kPan);
	}
}

void Mixer::stopPcm(const uint8_t *data) {
	MixerLock ml(_lock);
	int channel = -1;
	for (int i = 0; i < kPcmChannels; ++i) {
		if (_channels[i].sample.data == data) {
			channel = i;
			break;
		}
	}
	if (!(channel < 0)) {
		sts_mixer_stop_sample(&_mixer, &_channels[channel].sample);
		// reset channel
		memset(&_channels[channel], 0, sizeof(sts_mixer_sample_t));
		_channels[channel].voice = -1;
	}
}

void Mixer::mix(int16_t *buf, int len) {
	// stereo s16
	assert((len & 1) == 0);
	sts_mixer_mix_audio(&_mixer, buf, len / 2);
}
