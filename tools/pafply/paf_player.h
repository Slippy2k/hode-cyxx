
#ifndef __PAF_PLAYER_H__
#define __PAF_PLAYER_H__

#include "intern.h"
#include "file.h"

struct PAF_Player {
	File _fd;
	int _width;
	int _height;
	uint8 _paf_paletteBuffer[256 * 3];
	uint8 *_paf_decodeVideoPages;
	int _paf_currentDecodeVideoPage;
//	uint8 *_paf_unk47[256];
//	uint32 _paf_unk50[256];

	uint8 *getDecodePagePtr(uint8 a, uint8 b);
	int decode(const uint8 *src);
	void decode0(const uint8 *src, uint8 code, const uint8 *);
	bool open(const char *filename, int num);
	void close();
	void play();
//	void initLookupTables();
	void preload(int num);
};

#endif // __PAF_PLAYER_H__
