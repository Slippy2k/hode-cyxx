/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#ifndef RANDOM_H__
#define RANDOM_H__

#include "intern.h"

struct Random {
	uint32 _rndSeed;
	uint8 _rndRandomTable[100];
	int _rndRandomTableIndex;

	void initTable();
	void init();
	uint32 update();
	uint8 getNextNumber();
	uint8 getSeed();
};

#endif // RANDOM_H__
