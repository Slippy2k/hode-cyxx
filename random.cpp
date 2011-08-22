/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#include "random.h"

void Random::initTable() {
	for (int i = 0; i < 100; ++i) {
		_rndRandomTable[i] = i + 1;
	}
	for (int i = 0; i < 256; ++i) {
		const int a = update() % 100;
		const int b = update() % 100;
		SWAP(_rndRandomTable[a], _rndRandomTable[b]);
	}
	_rndRandomTableIndex = 0;
}

void Random::init() {
	_rndSeed = 0x101A1010;
	initTable();
}

uint32_t Random::update() {
	bool overflow = (_rndSeed & (1 << 31)) != 0;
	_rndSeed *= 2;
	if (!overflow) {
		_rndSeed ^= 0xDEADBEEF;
	}
	return _rndSeed;
}

uint8_t Random::getNextNumber() {
	uint8_t num = _rndRandomTable[_rndRandomTableIndex];
	++_rndRandomTableIndex;
	if (_rndRandomTableIndex == 100) {
		_rndRandomTableIndex = 0;
	}
	return num;
}

uint8_t Random::getSeed() {
	return _rndSeed & 0xFF;
}
