/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef RANDOM_H__
#define RANDOM_H__

#include "intern.h"

struct Random {
	uint32_t _rndSeed;
	uint8_t _rndRandomTable[100];
	int _rndRandomTableIndex;
	uint8_t _mstRandomTable[8][32];

	void initMstTable();
	void initTable();
	void init();
	uint32_t update();
	uint8_t getNextNumber();
	uint8_t getSeed();
	void resetMst(uint8_t *p);
	uint8_t getMstNextNumber(uint8_t *p);
};

#endif // RANDOM_H__
