
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

template <bool stereo, int panning>
static void mixS16(int16_t *dst, const int16_t *src, int len, int panL, int panR) {

	static const int kPanBits = 14; // 0..16384

	if (stereo) {
		for (int j = 0; j < len; j += 2, dst += 2, src += 2) {
			if (panning != 1) {
				dst[0] = CLIP(dst[0] + ((panL * src[0]) >> kPanBits), -32768, 32767);
			}
			if (panning != 2) {
				dst[1] = CLIP(dst[1] + ((panR * src[1]) >> kPanBits), -32768, 32767);
			}
		}
	} else {
		for (int j = 0; j < len; j += 2, dst += 2, ++src) {
			if (panning != 1) {
				dst[0] = CLIP(dst[0] + ((panL * src[0]) >> kPanBits), -32768, 32767);
			}
			if (panning != 2) {
				dst[1] = CLIP(dst[1] + ((panR * src[0]) >> kPanBits), -32768, 32767);
			}
		}
	}
}

void Mixer::mix(int16_t *buf, int len) {
	// stereo s16
	assert((len & 1) == 0);
	assert(len == 1764 * 2);
	if (_mixingQueueSize == 0) {
		return;
	}
	for (int i = 0; i < _mixingQueueSize; ++i) {
		const int panL = _mixingQueue[i].panL;
		const int panR = _mixingQueue[i].panR;
		if (_mixingQueue[i].stereo) {
			assert(_mixingQueue[i].ptr + len <= _mixingQueue[i].end);
			switch (_mixingQueue[i].panType) {
			case 1:
				mixS16<true, 1>(buf, _mixingQueue[i].ptr, len, panL, panR);
				break;
			case 2:
				mixS16<true, 2>(buf, _mixingQueue[i].ptr, len, panL, panR);
				break;
			default:
				mixS16<true, 0>(buf, _mixingQueue[i].ptr, len, panL, panR);
				break;
			}
		} else {
			assert(_mixingQueue[i].ptr + len / 2 <= _mixingQueue[i].end);
			switch (_mixingQueue[i].panType) {
			case 1:
				mixS16<false, 1>(buf, _mixingQueue[i].ptr, len, panL, panR);
				break;
			case 2:
				mixS16<false, 2>(buf, _mixingQueue[i].ptr, len, panL, panR);
				break;
			default:
				mixS16<false, 0>(buf, _mixingQueue[i].ptr, len, panL, panR);
				break;
			}
		}
	}
}
