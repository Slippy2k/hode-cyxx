/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include "game.h"
#include "resource.h"
#include "util.h"

static const uint8_t _mstLookupTable1[] = {
	0x08, 0x00, 0x02, 0x01, 0x04, 0x00, 0x03, 0x01,
	0x06, 0x07, 0x02, 0x01, 0x05, 0x07, 0x03, 0x01
};

static const uint8_t _mstLookupTable4[] = {
	0x01, 0x02, 0x03, 0x05, 0x06, 0x07, 0x09, 0x0A,
	0x0B, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13, 0x15,
	0x16, 0x17
};

static const uint8_t _mstLookupTable5[] = {
	0x00, 0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05,
	0x06, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0A, 0x0B,
	0x0C, 0x0C, 0x0D, 0x0E, 0x0F, 0x0F, 0x10, 0x11,
	0x12, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17
};

void Game::initMstCode() {
	memset(_mstGlobalVars, 0, sizeof(_mstGlobalVars));
	if (_mstLogicDisabled) {
		return;
	}
	// TODO
	resetMstCode();
}

void Game::resetMstCode() {
	if (_mstLogicDisabled) {
		return;
	}
	_mstGlobalFlags = 0;
	clearLvlObjectsList1();
}

void Game::startMstCode() {
}

template <typename T>
static bool compareOp(int op, T num1, T num2) {
	switch (op) {
	case 0:
		return num1 != num2;
	case 1:
		return num1 == num2;
	case 2:
		return num1 >= num2;
	case 3:
		return num1 > num2;
	case 4:
		return num1 <= num2;
	case 5:
		return num1 < num2;
	case 6:
		return (num1 & num2) == 0;
	case 7:
		return (num1 | num2) == 0;
	case 8:
		return (num1 ^ num2) == 0;
	default:
		error("compareOp unhandled op %d", op);
		break;
	}
	return false;
}

template <typename T>
static void arithOp(int op, T *p, int num) {
	switch (op) {
	case 0:
		*p = num;
		break;
	case 1:
		*p += num;
		break;
	case 2:
		*p -= num;
		break;
	case 3:
		*p *= num;
		break;
	case 4:
		if (num != 0) {
			*p /= num;
		}
		break;
	case 5:
		*p <<= num;
		break;
	case 6:
		*p >>= num;
		break;
	case 7:
		*p &= num;
		break;
	case 8:
		*p |= num;
		break;
	case 9:
		*p ^= num;
		break;
	default:
		error("arithOp unhandled op %d", op);
		break;
	}
}

void Game::executeMstCode() {
	_andyActionKeyMaskAnd = 0xFF;
	_andyDirectionKeyMaskAnd = 0xFF;
	_andyActionKeyMaskOr = 0;
	_andyDirectionKeyMaskOr = 0;
	if (_mstLogicDisabled) {
		return;
	}
	++_executeMstLogicCounter;
}

