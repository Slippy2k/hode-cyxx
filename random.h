/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#ifndef RANDOM_H__
#define RANDOM_H__

#include "intern.h"

struct Random {
	uint32_t _rndSeed;
	uint8_t _rndRandomTable[100];
	int _rndRandomTableIndex;

	void initTable();
	void init();
	uint32_t update();
	uint8_t getNextNumber();
	uint8_t getSeed();
};

#endif // RANDOM_H__
