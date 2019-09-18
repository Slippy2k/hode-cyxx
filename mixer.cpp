
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mixer.h"
#include "util.h"

static void nullMixerLock(int lock) {
}

Mixer::Mixer()
	: _lock(nullMixerLock) {
}

Mixer::~Mixer() {
}

void Mixer::init(int rate) {
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
	if (_mixingQueueSize == 0) {
		return;
	}
	static const int kPanBits = 14; // 0..16384
	for (int j = 0; j < len; j += 2) {
		for (int i = 0; i < _mixingQueueSize; ++i) {
			const int panL = _mixingQueue[i].panL;
			const int panR = _mixingQueue[i].panR;
			if (_mixingQueue[i].stereo) {
				assert(_mixingQueue[i].ptr + j + 1 < _mixingQueue[i].end);
				buf[j]     = CLIP(buf[j]     + ((panL * _mixingQueue[i].ptr[j]    ) >> kPanBits), -32768, 32767);
				buf[j + 1] = CLIP(buf[j + 1] + ((panR * _mixingQueue[i].ptr[j + 1]) >> kPanBits), -32768, 32767);
			} else {
				assert(_mixingQueue[i].ptr + j / 2 < _mixingQueue[i].end);
				buf[j]     = CLIP(buf[j]     + ((panL * _mixingQueue[i].ptr[j / 2]) >> kPanBits), -32768, 32767);
				buf[j + 1] = CLIP(buf[j + 1] + ((panR * _mixingQueue[i].ptr[j / 2]) >> kPanBits), -32768, 32767);
			}
		}
	}
}
