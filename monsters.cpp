/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include "game.h"
#include "resource.h"
#include "util.h"

static const uint8_t _mstLookupTable1[] = {
	0x08, 0x00, 0x02, 0x01, 0x04, 0x00, 0x03, 0x01, 0x06, 0x07, 0x02, 0x01, 0x05, 0x07, 0x03, 0x01
};

static const uint8_t _mstLookupTable4[] = {
	0x01, 0x02, 0x03, 0x05, 0x06, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13, 0x15,
	0x16, 0x17
};

static const uint8_t _mstLookupTable5[] = {
	0x00, 0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0A, 0x0B,
	0x0C, 0x0C, 0x0D, 0x0E, 0x0F, 0x0F, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17
};

void Game::initMstCode() {
	memset(_mstVars, 0, sizeof(_mstVars));
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
	_mstFlags = 0;
	// TODO
	clearLvlObjectsList1();
	for (int i = 0; i < _res->_mstHdr.screenAreaCodesCount; ++i) {
		_res->_mstScreenAreaCodes[i].unk0x1D = 1;
	}
	// TODO
	_rnd.initTable();
	// TODO
	memset(_mstVars, 0, sizeof(_mstVars));
	memset(_tasksTable, 0, sizeof(_tasksTable));
	// TODO
	_mstVars[30] = 0x20;
	// TODO
	updateMstMoveData();
	_mstPrevPosX = _mstPosX;
	_mstPrevPosY = _mstPosY;
}

void Game::startMstCode() {
	updateMstMoveData();
	_mstPrevPosX = _mstPosX;
	_mstPrevPosY = _mstPosY;
	// TODO
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
	// TODO

	if (_mstVars[31] > 0) {
		--_mstVars[31];
		if (_mstVars[31] == 0) {
			// TODO
		}
	}
	MstScreenAreaCode *msac;
	while ((msac = _res->findMstCodeForPos(_currentScreen, _mstPosX, _mstPosY)) != 0) {
		warning(".mst bytecode trigger for %d,%d", _mstPosX, _mstPosY);
		_res->flagMstCodeForPos(msac->unk0x1C, 0);
		assert(msac->codeData != kNone);
		createTask(_res->_mstCodeData + msac->codeData * 4);
	}
	if (_andyCurrentLevelScreenNum != _currentScreen) {
		_andyCurrentLevelScreenNum = _currentScreen;
	}
	for (Task *t = _tasksListTail; t; t = t->prev) {
		_runTaskOpcodesCount = 0;
		while ((this->*(t->run))(t) == 0);
	}
	// TODO
}

void Game::updateMstMoveData() {
	if (_andyObject) {
		_mstRefPosX = _andyObject->xPos + _andyObject->posTable[3].x;
		_mstRefPosY = _andyObject->yPos + _andyObject->posTable[3].y;
		_mstPosX = _mstRefPosX + _res->_mstPointOffsets[_currentScreen].xOffset;
		_mstPosY = _mstRefPosY + _res->_mstPointOffsets[_currentScreen].yOffset;
	} else {
		_mstRefPosX = 128;
		_mstRefPosY = 96;
		_mstPosX = _mstRefPosX + _res->_mstPointOffsets[0].xOffset;
		_mstPosY = _mstRefPosY + _res->_mstPointOffsets[0].yOffset;
        }
	// TODO
}

Task *Game::findFreeTask() {
	for (int i = 0; i < 128; ++i) {
		Task *t = &_tasksTable[i];
		if (!t->codeData) {
			return t;
		}
	}
	return 0;
}

Task *Game::createTask(const uint8_t *codeData) {
	for (int i = 0; i < 128; ++i) {
		Task *t = &_tasksTable[i];
		if (!t->codeData) {
			memset(t, 0, sizeof(Task));
			resetTask(t, codeData);
			t->next = 0;
			t->prev = _tasksListTail;
			if (_tasksListTail) {
				_tasksListTail->next = t;
			}
			_tasksListTail = t;
			return t;
		}
	}
	return 0;
}

void Game::resetTask(Task *t, const uint8_t *codeData) {
	t->runningState |= 2;
	t->codeData = codeData;
	t->run = &Game::runTask_default;
	t->localVars[12] = 0;
	// TODO:
}

void Game::removeTask(Task **tasksList, Task *t) {
	Task *c = t->child;
	if (c) {
		c->codeData = 0;
		t->child = 0;
	}
	Task *next = t->next;
	t->codeData = 0;
	t = t->prev;
	if (t) {
		t->next = next;
	}
	if (next) {
		next->prev = t;
	} else {
		*tasksList = t;
	}
}

int Game::getTaskVar(Task *t, int index, int type) const {
	switch (type) {
	case 1:
		return index;
	case 2:
		return t->localVars[index];
	case 3:
		return _mstVars[index];
	case 4:
		return getTaskOtherVar(index, t);
	default:
		error("getTaskVar unhandled index %d type %d", index, type);
		break;
	}
	return 0;
}

int Game::getTaskOtherVar(int index, Task *t) const {
	switch (index) {
	case 0:
		return _mstRefPosX;
	case 1:
		return _mstRefPosY;
	case 2:
		return _mstPosX;
	case 3:
		return _mstPosY;
	case 4:
		return _currentScreen;
	case 5:
		return _res->_screensState[_currentScreen].s0;
//	case 22:
//		return _mstOp54Counter;
	case 23:
		return _andyObject->actionKeyMask;
	case 24:
		return _andyObject->directionKeyMask;
//	case 31:
//		return _executeMstLogicCounter;
//	case 32:
//		return _executeMstLogicCounter - _executeMstLogicLastCounter; // PrevCounter
//	case 34:
//		return _res->_currentScreenResourceState;
	case 35:
		return _andyCurrentLevelScreenNum;
	default:
		error("getTaskOtherVar unhandled index %d", index);
		break;
	}
	return 0;
}

int Game::runTask_default(Task *t) {
	int ret = 0;
	t->runningState &= ~2;
	const uint8_t *p = t->codeData;
	do {
		assert(p >= _res->_mstCodeData && p < _res->_mstCodeData + _res->_mstHdr.codeSize * 4);
		assert(((p - t->codeData) & 3) == 0);
		switch (*p) {
		default:
			warning("Unhandled opcode %d in runTask_default", *p);
			return 1;
		}
		p += 4;
		if ((t->runningState & 2) != 0) {
			t->runningState &= ~2;
			p = t->codeData;
		}
		++_runTaskOpcodesCount;
	} while (_runTaskOpcodesCount <= 128 && ret == 0);
	if (t->codeData) {
		t->codeData = p;
	}
	return 1;
}
