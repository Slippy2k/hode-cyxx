
#include "game.h"
#include "lzw.h"
#include "systemstub.h"
#include "video.h"

uint32_t Game::benchmarkLoop(const uint8_t *p, int count) {
	uint32_t accum = 0;
	count >>= 2;
	if (count > 0) {
		count += 1023;
		count >>= 10;
		for (; count > 0; --count) {
			accum += READ_LE_UINT32(p);
			p += 4096;
		}
	}
	return accum;
}

uint32_t Game::benchmarkCpu() {
	const uint32_t t0 = _system->getTimeStamp();
	uint8_t *p = _video->_shadowLayer;
	int count = 32;
	do {
		decodeLZW(_benchmarkData1, p);
		benchmarkLoop(p, 65536);
		decodeLZW(_benchmarkData2, p);
		_video->updateGameDisplay(p);
	} while (--count != 0);
	// _util_cpuUsage = GetTickCount() - _ebp;
	// _cfg_slowCpu = !(_util_cpuUsage < 1100);
	const uint32_t t1 = _system->getTimeStamp();
	return t1 - t0;
}
