
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mixer.h"
#include "util.h"

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
	_rate = rate;
	memset(_mixingQueue, 0, sizeof(_mixingQueue));
	_mixingQueueSize = 0;
}

void Mixer::fini() {
}

void Mixer::mix(int16_t *buf, int len) {
	// stereo s16
	assert((len & 1) == 0);
	assert(len == 1764 * 2);
	for (int j = 0; j < len; j += 2) {
		for (int i = 0; i < _mixingQueueSize; ++i) {
			if (_mixingQueue[i].stereo) {
				assert(_mixingQueue[i].ptr + j + 1 < _mixingQueue[i].end);
				buf[j]     = CLIP(buf[j]     + _mixingQueue[i].ptr[j],     -32768, 32767);
				buf[j + 1] = CLIP(buf[j + 1] + _mixingQueue[i].ptr[j + 1], -32768, 32767);
			} else {
				assert(_mixingQueue[i].ptr + j / 2 < _mixingQueue[i].end);
				buf[j]     = CLIP(buf[j]     + _mixingQueue[i].ptr[j / 2], -32768, 32767);
				buf[j + 1] = CLIP(buf[j + 1] + _mixingQueue[i].ptr[j / 2], -32768, 32767);
			}
		}
	}
}
