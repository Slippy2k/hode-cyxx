/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include "game.h"
#include "resource.h"
#include "util.h"

static const uint8_t _mstLut1[] = {
	0x08, 0x00, 0x02, 0x01, 0x04, 0x00, 0x03, 0x01, 0x06, 0x07, 0x02, 0x01, 0x05, 0x07, 0x03, 0x01
};

static const uint8_t _mstLut4[] = {
	0x01, 0x02, 0x03, 0x05, 0x06, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13, 0x15,
	0x16, 0x17
};

static const uint8_t _mstLut5[] = {
	0x00, 0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0A, 0x0B,
	0x0C, 0x0C, 0x0D, 0x0E, 0x0F, 0x0F, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17
};

static const uint8_t _mstDefaultLutOp[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
	0x0B, 0x0C, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
	0x16, 0x17, 0x4F, 0x18, 0x19, 0x4F, 0x4F, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22,
	0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23,
	0x23, 0x23, 0x23, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x25, 0x25, 0x25,
	0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x25, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26, 0x26,
	0x26, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x27, 0x28, 0x28, 0x28, 0x28, 0x28,
	0x28, 0x28, 0x28, 0x28, 0x28, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x29, 0x2A,
	0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2A, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B,
	0x2B, 0x2B, 0x2B, 0x2C, 0x2C, 0x2C, 0x2C, 0x2C, 0x2C, 0x2C, 0x2C, 0x2C, 0x2C, 0x2D, 0x2D, 0x2D,
	0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2D, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E, 0x2E,
	0x2E, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x30, 0x30, 0x30, 0x30, 0x30,
	0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x4F, 0x4F, 0x4F,
	0x4F, 0x4F, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x43, 0x43, 0x43,
	0x43, 0x43, 0x44, 0x45, 0x46, 0x17, 0x4F, 0x47, 0x47, 0x48, 0x48, 0x4F, 0x49, 0x4A, 0x4B, 0x4C,
	0x4D, 0x4F, 0x4E
};

void Game::resetMstUnkData(MstUnkData *m) {
	m->o0 = 0;
	LvlObject *o = m->o16;
	if (o) {
		o->dataPtr = 0;
	}
	for (int i = 0; i < 64; ++i) {
		if (_mstObjectsTable[i].unk0 && _mstObjectsTable[i].unk8 == m) {
			_mstObjectsTable[i].unk8 = 0;
		}
	}
}

void Game::resetMstObject(MstObject *m) {
	m->unk0 = 0;
	LvlObject *o = m->o;
	if (o) {
		o->dataPtr = 0;
	}
}

void Game::setMstObjectDefaultPos(Task *t) {
	MstObject *m = t->mstObject;
	LvlObject *o = m->o;
	m->xPos = o->xPos + o->posTable[7].x;
	m->yPos = o->yPos + o->posTable[7].y;
	m->xMstPos = m->xPos + _res->_mstPointOffsets[o->screenNum].xOffset;
	m->yMstPos = m->yPos + _res->_mstPointOffsets[o->screenNum].yOffset;
}

void Game::shuffleMstUnk43(MstUnk43 *p) {
	// TODO
//	for (int i = 0; i < (int)p->count2; ++i) {
//		_resMstUnk43[p->offset2 + i] &= 0x7F;
//	}
//	shuffleArray(_resMstUnk43 + p->offset2, p->count2);
}

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
	for (int i = 0; i < 32; ++i) {
		resetMstUnkData(&_mstUnkDataTable[i]);
	}
	for (int i = 0; i < 64; ++i) {
		resetMstObject(&_mstObjectsTable[i]);
	}
	clearLvlObjectsList1();
	for (int i = 0; i < _res->_mstHdr.screenAreaCodesCount; ++i) {
		_res->_mstScreenAreaCodes[i].unk0x1D = 1;
	}
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 32; ++j) {
			_mstRandomLookupTable[i][j] = j;
		}
		for (int j = 0; j < 64; ++j) {
			const int index1 = _rnd.update() & 31;
			const int index2 = _rnd.update() & 31;
			SWAP(_mstRandomLookupTable[i][index1], _mstRandomLookupTable[i][index2]);
		}
	}
	_rnd.initTable();
	for (int i = 0; i < _res->_mstHdr.unk0x40; ++i) {
		const int count = _res->_mstUnk49[i].count2;
		if (count != 0) {
			// TODO
			for (int j = 0; j < count * 2; ++j) {
				// const int index1 = _rnd.update() % max;
				// const int index2 = _rnd.update() % max;
			}
		}
	}
	for (int i = 0; i < _res->_mstHdr.unk0x0C; ++i) { // var8
		const int count = _res->_mstUnk35[i].size;
		if (count != 0) {
			// TODO
			for (int j = 0; j < count * 2; ++j) {
				// const int index1 = _rnd.update() % max;
				// const int index2 = _rnd.update() % max;
			}
		}
	}

	// TODO
	memset(_mstVars, 0, sizeof(_mstVars));
	memset(_tasksTable, 0, sizeof(_tasksTable));
	// TODO
	_mstOp54Unk1 = _mstOp54Unk2 = 0;
	// TODO
	_executeMstLogicPrevCounter = _executeMstLogicCounter = 0;
	// TODO
	_tasksListTail = 0;
	// TODO
	if (_res->_mstTickCodeData != kNone) {
		_mstVars[31] = _mstTickDelay = _res->_mstTickDelay;
	} else {
		_mstVars[31] = -1;
	}
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
	int offset = 0;
	for (int i = 0; i < _res->_mstHdr.unk0x3C; ++i) {
		offset += 948;
		_res->_mstHeightMapData[offset - 0x20] = _mstPosX - _res->_mstHeightMapData[offset - 0x30];
		_res->_mstHeightMapData[offset - 0x1C] = _mstPosX + _res->_mstHeightMapData[offset - 0x30];
		_res->_mstHeightMapData[offset - 0x24] = _res->_mstHeightMapData[offset - 0x20] - _res->_mstHeightMapData[offset - 0x34];
		_res->_mstHeightMapData[offset - 0x18] = _res->_mstHeightMapData[offset - 0x1C] + _res->_mstHeightMapData[offset - 0x34];
		_res->_mstHeightMapData[offset - 0x10] = _mstPosY - _res->_mstHeightMapData[offset - 0x30];
		_res->_mstHeightMapData[offset - 0x0C] = _mstPosY + _res->_mstHeightMapData[offset - 0x30];
		_res->_mstHeightMapData[offset - 0x14] = _res->_mstHeightMapData[offset - 0x10] - _res->_mstHeightMapData[offset - 0x34];
		_res->_mstHeightMapData[offset - 0x08] = _res->_mstHeightMapData[offset - 0x0C] + _res->_mstHeightMapData[offset - 0x34];
	}
	if (_res->_currentScreenResourceNum < _res->_mstHdr.unk0x14) {
		uint32_t codeData = _res->_mstScreenInitCodeData[_res->_currentScreenResourceNum];
		if (codeData != kNone) {
			Task *t = createTask(_res->_mstCodeData + codeData * 4);
			if (t) {
				while ((this->*(t->run))(t) == 0);
			}
		}
	}
	_executeMstLogicPrevCounter = _executeMstLogicCounter;
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
	executeMstCodeHelper2();
	if (_mstVars[31] > 0) {
		--_mstVars[31];
		if (_mstVars[31] == 0) {
			uint32_t codeData = _res->_mstTickCodeData;
			assert(codeData != kNone);
			Task *t = createTask(_res->_mstCodeData + codeData * 4);
			if (t) {
				t->runningState = 1;
			}
		}
	}
	MstScreenAreaCode *msac;
	while ((msac = _res->findMstCodeForPos(_currentScreen, _mstPosX, _mstPosY)) != 0) {
		debug(kDebug_MONSTER, "trigger for %d,%d", _mstPosX, _mstPosY);
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

void Game::executeMstCodeHelper2() {
	updateMstMoveData();
	updateMstHeightMapData();
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

void Game::updateMstHeightMapData() {
	const int _mstPosDx = _mstPosX - _mstPrevPosX;
	const int _mstPosDy = _mstPosY - _mstPrevPosY;
	_mstPrevPosX = _mstPosX;
	_mstPrevPosY = _mstPosY;
	if (_mstPosDx == 0 && _mstPosDy == 0) {
		return;
	}
	int offset = 0;
	for (int i = 0; i < _res->_mstHdr.unk0x3C; ++i) {
		offset += 948;
		_res->_mstHeightMapData[offset - 0x20] = _mstPosX - _res->_mstHeightMapData[offset - 0x30];
		_res->_mstHeightMapData[offset - 0x1C] = _mstPosX + _res->_mstHeightMapData[offset - 0x30];
		_res->_mstHeightMapData[offset - 0x24] = _res->_mstHeightMapData[offset - 0x20] - _res->_mstHeightMapData[offset - 0x34];
		_res->_mstHeightMapData[offset - 0x18] = _res->_mstHeightMapData[offset - 0x1C] + _res->_mstHeightMapData[offset - 0x34];
		_res->_mstHeightMapData[offset - 0x10] = _mstPosY - _res->_mstHeightMapData[offset - 0x30];
		_res->_mstHeightMapData[offset - 0x0C] = _mstPosY + _res->_mstHeightMapData[offset - 0x30];
		_res->_mstHeightMapData[offset - 0x14] = _res->_mstHeightMapData[offset - 0x10] - _res->_mstHeightMapData[offset - 0x34];
		_res->_mstHeightMapData[offset - 0x08] = _res->_mstHeightMapData[offset - 0x0C] + _res->_mstHeightMapData[offset - 0x34];
	}
}

void Game::removeMstObjectTask(Task *t) {
	MstObject *m = t->mstObject;
	m->unk0 = 0;
	LvlObject *o = m->o;
	if (o) {
		o->dataPtr = 0;
		removeLvlObject2(o);
	}
	Task *child = t->child;
	if (child) {
		child->codeData = 0;
	}
	Task *next = t->next;
	t->codeData = 0;
	Task *prev = t->prev;
	if (prev) {
		prev->next = next;
	}
	if (next) {
		next->prev = prev;
	}
}

Task *Game::findFreeTask() {
	for (int i = 0; i < kMaxTasks; ++i) {
		Task *t = &_tasksTable[i];
		if (!t->codeData) {
			return t;
		}
	}
	return 0;
}

Task *Game::createTask(const uint8_t *codeData) {
	for (int i = 0; i < kMaxTasks; ++i) {
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

Task *Game::updateTask(Task *t, int num, const uint8_t *codeData) {
	Task *current = _tasksListTail;
	bool found = false;
	while (current) {
		if (current->localVars[7] == num) {
			found = true;
			if (current != t) {
				// TODO
				warning("updateTask, duplicated task id %d", num);
				break;
			}
		}
		current = current->prev;
	}
	if (found) {
		return current;
	}
	for (int i = 0; i < kMaxTasks; ++i) {
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
			t->localVars[7] = num;
			return t;
		}
	}
	return 0;
}

void Game::resetTask(Task *t, const uint8_t *codeData) {
	t->runningState |= 2;
	t->codeData = codeData;
	t->run = &Game::runTask_default;
	t->localVars[7] = 0;
	MstUnkData *m = t->dataPtr;
	if (m) {
		const uint8_t mask = m->flagsA5;
		if ((mask & 0x88) == 0 || (mask & 0xF0) == 0) {
			if ((mask & 8) != 0) {
				t->flags = (t->flags & ~0x40) | 0x20;
				m->flags48 &= ~0x1C;
			} else if ((mask & 2) != 0) {
// 414F2A
				m->flags48 |= 8;
				// TODO
			}
		}
	}
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

void Game::prependTask(Task **tasksList, Task *t) {
	Task *current = *tasksList;
	if (!current) {
		*tasksList = current;
		t->prev = t->next = 0;
	} else {
		Task *prev = current->prev;
		while (prev) {
			current = prev;
			prev = current->prev;
		}
		current->prev = t;
		t->prev = 0;
		t->next = current;
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
	case 5:
		{
			MstUnkData *m = 0;
			if (t->mstObject) {
				m = t->mstObject->unk8;
			} else {
				m = t->dataPtr;
			}
			if (m) {
				return m->localVars[index];
			}
		}
		break;
	default:
		error("getTaskVar unhandled index %d type %d", index, type);
		break;
	}
	return 0;
}

void Game::setTaskVar(Task *t, int index, int type, int value) {
	switch (type) {
	case 2:
		t->localVars[index] = value;
		break;
	case 3:
		_mstVars[index] = value;
		break;
	default:
		error("setTaskVar unhandled index %d type %d", index, type);
		break;
	}
}

int Game::getTaskAndyVar(int index, Task *t) const {
	switch (index) {
	case 0:
		return (_andyObject->flags1 >> 4) & 1;
	case 1:
		return (_andyObject->flags1 >> 5) & 1;
	case 5:
		return (_andyObject->flags0 & 0x1F) == 7;
	case 6:
		return (_andyObject->data0x2988 == 0);
	default:
		error("getTaskAndyVar unhandled index %d", index);
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
	case 6:
		return _difficulty;
	case 22:
		return _mstOp54Counter;
	case 23:
		return _andyObject->actionKeyMask;
	case 24:
		return _andyObject->directionKeyMask;
	case 30:
		return _mstOp56Counter;
	case 31:
		return _executeMstLogicCounter;
	case 32:
		return _executeMstLogicCounter - _executeMstLogicPrevCounter;
	case 34:
		return _levelCheckpoint;
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
		assert(p[0] <= 242);
		switch (p[0]) {
		case 0: // 0
			// TODO:
			// fall-through
		case 1: { // 1
				const int num = READ_LE_UINT16(p + 2);
				const int delay = getTaskVar(t, num, p[1]);
				t->delay = delay;
				if (delay > 0) {
					if (p[0] == 0) {
						t->run = &Game::runTask_waitResetInput;
						ret = 1;
					} else {
						t->run = &Game::runTask_wait;
						ret = 1;
					}
				}
			}
			break;
		case 3: // 3
			if (t->dataPtr) {
				// TODO
			}
			break;
		case 4: // 4
			if (t->dataPtr) {
				// TODO
			}
			break;
		case 23: // 13
			_mstFlags |= (1 << p[1]);
			break;
		case 24: // 14
			t->flags |= (1 << p[1]);
			break;
		case 25: // 15
			if (t->mstObject) {
				// TODO
			} else if (t->dataPtr) {
				// TODO
			}
			break;
		case 26: // 16
			_mstFlags &= ~(1 << p[1]);
			break;
		case 27: // 17
			t->flags &= ~(1 << p[1]);
			break;
		case 28: // 18
			if (t->mstObject) {
				// TODO
			} else if (t->dataPtr) {
				// TODO
			}
			break;
		case 29: { // 19
				t->delay = 4;
				t->mstFlags = p[1];
				if (getTaskAndyVar(p[1], t) == 0) {
					LvlObject *o = 0;
					if (t->dataPtr) {
						// TODO
					} else if (t->mstObject) {
						// TODO
					}
					if (o) {
						o->actionKeyMask = 0;
						o->directionKeyMask = 0;
					}
					t->run = &Game::runTask_waitFlags;
					ret = 1;
				}
			}
			break;
		case 30: { // 20
				t->delay = 3;
				t->mstFlags = 1 << p[1];
				if ((t->mstFlags & _mstFlags) == 0) {
					if (t->dataPtr) {
						// TODO
					} else if (t->mstObject) {
						// TODO
					}
					t->run = &Game::runTask_waitFlags;
					ret = 1;
				}
			}
			break;
		case 33:
		case 229: { // 23
				const int num = READ_LE_UINT16(p + 2);
				p = _res->_mstCodeData + (num - 1) * 4;
			}
			break;
		case 35: { // 24
				const int num = READ_LE_UINT16(p + 2);
				_res->flagMstCodeForPos(num, 1);
			}
			break;
		case 36: { // 25
				const int num = READ_LE_UINT16(p + 2);
				_res->flagMstCodeForPos(num, 0);
			}
			break;
		case 39: { // 26
				if (p[1] < _res->_mstHdr.pointsCount) {
					// TODO
					// executeMstOp26(_mstToLoad2Pri, p[1]);
					// executeMstOp26(_mstToLoad1Pri, p[1]);
					// executeMstOp26(_mstToLoad2Num, p[1]);
					// executeMstOp26(_mstToLoad1Num, p[1]);
				}
			}
			break;
		case 42: { // 29
				const int num = p[1];
				assert(num < kMaxVars);
				++_mstVars[num];
			}
			break;
		case 44: { // 31
				const int num = p[1];
				assert(num < kMaxLocals);
				--t->localVars[num];
			}
			break;
		case 45: { // 32
				const int num = p[1];
				assert(num < kMaxVars);
				--_mstVars[num];
			}
			break;
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
		case 56: { // 34
				const int src = p[2];
				const int dst = p[1];
				arithOp(p[0] - 47, &t->localVars[dst], t->localVars[src]);
			}
			break;
		case 57:
		case 58:
		case 59:
		case 60:
		case 61:
		case 62:
		case 63:
		case 64:
		case 65:
		case 66: { // 35
				const int src = p[2];
				const int dst = p[1];
				assert(dst < kMaxVars);
				arithOp(p[0] - 57, &_mstVars[dst], t->localVars[src]);
			}
			break;
		case 107:
		case 108:
		case 109:
		case 110:
		case 111:
		case 112:
		case 113:
		case 114:
		case 115:
		case 116: { // 40
				assert(p[1] < kMaxLocals);
				assert(p[2] < kMaxVars);
				arithOp(p[0] - 107, &t->localVars[p[1]], _mstVars[p[2]]);
			}
			break;
		case 117:
		case 118:
		case 119:
		case 120:
		case 121:
		case 122:
		case 123:
		case 124:
		case 125:
		case 126: { // 41
				assert(p[1] < kMaxLocals);
				assert(p[2] < kMaxVars);
				arithOp(p[0] - 117, &_mstVars[p[1]], _mstVars[p[2]]);
			}
			break;
		case 137:
		case 138:
		case 139:
		case 140:
		case 141:
		case 142:
		case 143:
		case 144:
		case 145:
		case 146: { // 43
				const int num = p[2];
				assert(p[1] < kMaxLocals);
				arithOp(p[0] - 137, &t->localVars[p[1]], getTaskOtherVar(num, t));
			}
			break;
		case 147:
		case 148:
		case 149:
		case 150:
		case 151:
		case 152:
		case 153:
		case 154:
		case 155:
		case 156: { // 44
				const int num = p[2];
				assert(p[1] < kMaxVars);
				arithOp(p[0] - 147, &_mstVars[p[1]], getTaskOtherVar(num, t));
			}
			break;
		case 167:
		case 168:
		case 169:
		case 170:
		case 171:
		case 172:
		case 173:
		case 174:
		case 175:
		case 176: { // 46
				const int num = READ_LE_UINT16(p + 2);
				assert(p[1] < kMaxLocals);
				arithOp(p[0] - 167, &t->localVars[p[1]], num);
			}
			break;
		case 177:
		case 178:
		case 179:
		case 180:
		case 181:
		case 182:
		case 183:
		case 184:
		case 185:
		case 186: { // 47
				const int16_t num = READ_LE_UINT16(p + 2);
				assert(p[1] < kMaxVars);
				arithOp(p[0] - 177, &_mstVars[p[1]], num);
			}
			break;
		case 187:
		case 188:
		case 189:
		case 190:
		case 191:
		case 192:
		case 193:
		case 194:
		case 195:
		case 196: { // 48
				MstUnkData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->unk8;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					const int16_t num = READ_LE_UINT16(p + 2);
					arithOp(p[0] < 187, &m->localVars[p[1]], num);
				}
			}
			break;
		case 198: { // 50
				Task *child = findFreeTask();
				if (child) {
					memset(child, 0, sizeof(Task));
					t->child = child;
					const int16_t num = READ_LE_UINT16(p + 2);
					const uint32_t codeData = _res->_mstUnk60[num];
					assert(codeData != kNone);
					p = _res->_mstCodeData + codeData * 4;
					t->codeData = p;
					t->runningState &= ~2;
					p -= 4;
				}
			}
			break;
		case 202: // 54
			executeMstOp54();
			break;
		case 204: // 56
			ret = executeMstOp56(t, p[1], READ_LE_UINT16(p + 2));
			break;
		case 215: { // 62
				if (_mstOp54Unk3 != 0xFFFF) {
					shuffleMstUnk43(&_res->_mstUnk43[_mstOp54Unk3]);
				}
				_mstOp54Counter = 0;
			}
			break;
		case 217: { // 64
				const int num = READ_LE_UINT16(p + 2);
				if (_mstOp54Unk3 != num) {
					_mstOp54Unk3 = num;
					shuffleMstUnk43(&_res->_mstUnk43[num]);
					_mstOp54Counter = 0;
				}
			}
			break;
		case 218: { // 65
				const int16_t num = READ_LE_UINT16(p + 2);
				if (num != _mstOp54Unk1) {
					_mstOp54Unk1 = num;
					_mstOp54Unk2 = num;
					shuffleMstUnk43(&_res->_mstUnk43[num]);
				}
			}
			break;
		case 223: { // 67
				const int num = READ_LE_UINT16(p + 2);
				MstUnk53 *m = &_res->_mstUnk53[num];
				const int mask = m->maskVars; // var8
				int a = getTaskVar(t, m->indexVar1, (mask >> 16) & 15); // var1C
				int b = getTaskVar(t, m->indexVar2, (mask >> 12) & 15); // var20
				int c = getTaskVar(t, m->indexVar3, (mask >>  8) & 15); // var14, _ebx
				int d = getTaskVar(t, m->indexVar4, (mask >>  4) & 15); // _edi
				int e = getTaskVar(t, m->indexVar5,  mask        & 15); // _eax
				if (a > b) {
					SWAP(a, b);
				}
				if (c > d) {
					SWAP(c, d);
				}
				LvlObject *o = 0;
				if (t->mstObject) {
					o = t->mstObject->o;
				} else if (t->dataPtr) {
					o = t->dataPtr->o16;
				}
				if (e <= -2 && o) {
					if (o->flags & 0x10) {

					} else {
// 41367F
						a += o->xPos;
						b += o->xPos;
					}

					c += o->yPos;
					d += o->yPos;
					if (e < -2) {
						a += o->posTable[6].x;
						b += o->posTable[6].x;
						c += o->posTable[6].y;
						d += o->posTable[6].y;
					} else {
						a += o->posTable[7].x;
						b += o->posTable[7].x;
						c += o->posTable[7].y;
						d += o->posTable[7].y;
					}
					e = o->screenNum;
				}
// 4136E8
				e = CLIP(e, -1, _res->_mstHdr.pointsCount - 1);
				if (p[0] == 224) {
					// TODO
					break;
				} else if (p[0] == 225) {
					// TODO
					break;
				} else {
					t->flags |= 0x80;
					if (p[0] == 222 || p[0] == 220) {
						if (e == -1) {
							if (a >= -_mstRefPosX && a <= 255 - _mstRefPosX) {
								break;
							}
						} else if (e == _currentScreen) {
							break;
						}
					}
				}
// 4137FA
				executeMstOp67(t, a, b, c, d, e, m->unk8, m->unk9, m->unkC, m->unkB, 0, m->unkE);
			}
			break;
		case 227: { // 69
				const int num = READ_LE_UINT16(p + 2);
				MstUnk54 *m = &_res->_mstUnk54[num];
				const int a = getTaskVar(t, m->unk0,  m->unk5       & 15);
				const int b = getTaskVar(t, m->unk2, (m->unk5 >> 4) & 15);
				if (compareOp(m->unk4, a, b)) {
					assert(m->codeData != kNone);
					t->codeData = _res->_mstCodeData + m->codeData * 4;
				}
			}
			break;
		case 239: { // 76
				const int i = READ_LE_UINT16(p + 2);
				const uint32_t codeData = _res->_mstUnk60[i];
				assert(codeData != kNone);
				createTask(_res->_mstCodeData + codeData * 4);
			}
			break;
		case 240: { // 77
				const int num = READ_LE_UINT16(p + 2);
				MstUnk59 *m = &_res->_mstUnk59[num];
				if (m->codeData == kNone) {
					warning("opcode 240 codeOffset is -1 for taskId %d", m->taskId);
				} else {
					updateTask(t, m->taskId, _res->_mstCodeData + m->codeData * 4);
				}
			}
			break;
		case 242: // 78
			if (t->child) {
				// TODO
			} else if (t->dataPtr) {
				// TODO
			} else if (t->mstObject) {
				// TODO
			} else {
				if ((t->runningState & 1) != 0 && _mstVars[31] == 0) {
					_mstVars[31] = _mstTickDelay;
				}
				removeTask(&_tasksListTail, t);
				ret = 1;
			}
			break;
		default:
			warning("Unhandled opcode %d (%d) in runTask_default", *p, _mstDefaultLutOp[*p]);
			break;
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

void Game::executeMstOp54() {
	// TODO
	const int x = MIN(_mstRefPosX, 255);
	if (_mstRefPosX < 0) {
		_mstPosXmin = x;
		_mstPosXmax = 255 + x;
	} else {
		_mstPosXmin = -x;
		_mstPosXmax = 255 - x;
	}
	const int y = MIN(_mstRefPosY, 191);
	if (_mstRefPosY < 0) {
		_mstPosYmin = y;
		_mstPosYmax = 191 + y;
	} else {
		_mstPosYmin = -y;
		_mstPosYmax = 191 - y;
	}
	// TODO
}

static uint8_t getLvlObjectFlag4(int type, const LvlObject *o, const LvlObject *andyObject) {
	switch (type) {
	case 1:
		return 1;
	case 2:
		return (o->flags1 >> 4) & 1;
	case 3:
		return ~(o->flags1 >> 4) & 1;
	case 4:
		return (andyObject->flags1 >> 4) & 1;
	case 5:
		return ~(andyObject->flags1 >> 4) & 1;
	}
	return 0;
}

int Game::executeMstOp56(Task *t, int code, int num) {
	assert(num < _res->_mstHdr.unk0x78);
	switch (code) {
	case 0:
		if (_mstCurrentUnkFlag == 0 && resetAndyLvlObjectPlasmaCannonKeyMask(0x71) != 0) {
			_plasmaCannonFlags |= 1;
			if (_andyObject->data0x2988 == 0) {
				_mstCurrentAnim = _res->_mstOp56Data[num].unk0 & 0xFFFF;
			} else {
				_mstCurrentAnim = _res->_mstOp56Data[num].unk0 >> 16;
			}
// 411AB4
			// TODO
// 411B72
			// TODO
		}
// 411BBA
		// TODO
		break;
	case 1:
		if (_mstCurrentUnkFlag != 0) {
			if (resetAndyLvlObjectPlasmaCannonKeyMask(0x61) != 0) {
				_plasmaCannonFlags &= ~1;
				if (_andyObject->data0x2988 == 0) {
					_mstCurrentAnim = _res->_mstOp56Data[num].unk0 & 0xFFFF;
				} else {
					_mstCurrentAnim = _res->_mstOp56Data[num].unk0 >> 16;
				}
// 4118ED
			}
// 4119F5
		}
		break;
	case 2: {
			LvlObject *o = t->dataPtr->o16;
			uint8_t flag = getLvlObjectFlag4(_res->_mstOp56Data[num].unk0 & 255, o, _andyObject);
			resetAndyLvlObjectPlasmaCannonKeyMask(flag | 0x10);
		}
		break;
	case 3:
		resetAndyLvlObjectPlasmaCannonKeyMask(0x12);
		break;
	case 4:
		resetAndyLvlObjectPlasmaCannonKeyMask(0x80);
		break;
	case 5:
		resetAndyLvlObjectPlasmaCannonKeyMask(0xA4);
		break;
	case 6:
		resetAndyLvlObjectPlasmaCannonKeyMask(0xA3);
		break;
	case 7:
		resetAndyLvlObjectPlasmaCannonKeyMask(0x05);
		break;
	case 8:
		resetAndyLvlObjectPlasmaCannonKeyMask(0xA1);
		break;
	case 9:
		resetAndyLvlObjectPlasmaCannonKeyMask(0xA2);
		break;
	case 10:
		if (_res->_mstOp56Data[num].unk0 == 1) {
			setShakeScreen(2, _res->_mstOp56Data[num].unk4 & 255);
		} else if (_res->_mstOp56Data[num].unk0 == 2) {
			setShakeScreen(1, _res->_mstOp56Data[num].unk4 & 255);
		} else {
			setShakeScreen(3, _res->_mstOp56Data[num].unk4 & 255);
		}
		break;
	case 11: {
			MstObject *m = t->mstObject;
			const int type = _res->_mstOp56Data[num].unkC;
			m->boundingBox.x1 = getTaskVar(t, _res->_mstOp56Data[num].unk0, (type >> 0xC) & 15);
			m->boundingBox.x2 = getTaskVar(t, _res->_mstOp56Data[num].unk4, (type >> 0x8) & 15);
			m->boundingBox.y1 = getTaskVar(t, _res->_mstOp56Data[num].unk8, (type >> 0x4) & 15);
			m->boundingBox.y2 = getTaskVar(t, type >> 16                  ,  type         & 15);
		}
		break;
	case 12: {
			const int type1 = ((_res->_mstOp56Data[num].unkC >> 4) & 15);
			const int hint  = getTaskVar(t, _res->_mstOp56Data[num].unk0, type1);
			const int type2 = (_res->_mstOp56Data[num].unkC & 15);
			const int pause = getTaskVar(t, _res->_mstOp56Data[num].unk4, type2);
			displayHintScreen(hint, pause);
		}
		break;
	case 15: {
			_andyObject->anim  = _res->_mstOp56Data[num].unk0;
			_andyObject->frame = _res->_mstOp56Data[num].unk4;
			LvlObject *o = 0;
			if (t->mstObject) {
				// TODO
			} else if (t->dataPtr) {
				// TODO
			} else {
				o = _andyObject;
			}
			uint8_t flag = getLvlObjectFlag4(_res->_mstOp56Data[num].unk8, o, _andyObject);
			_andyObject->flags1 = ((flag & 3) << 4) | (_andyObject->flags1 & 0xFFCF);
			setupLvlObjectBitmap(_andyObject);
			// TODO
		}
		break;
	case 16:
		if (t->mstObject) {
			// TODO
		} else if (t->dataPtr) {
			// TODO
		}
		// fall-through
	case 17: {
			LvlObject *o = _andyObject;
			const int pos = _res->_mstOp56Data[num].unk8;
			const int xPos = o->xPos + o->posTable[pos].x;
			const int yPos = o->yPos + o->posTable[pos].y;
			const int type1  = (_res->_mstOp56Data[num].unkC >> 4) & 15;
			const int index1 = _res->_mstOp56Data[num].unk0;
			setTaskVar(t, index1, type1, xPos);
			const int type2  = _res->_mstOp56Data[num].unkC & 15;
			const int index2 = _res->_mstOp56Data[num].unk4;
			setTaskVar(t, index2, type2, yPos);
		}
		break;
	case 18: {
			_mstUnk39 = _res->_mstOp56Data[num].unk0 & 255;
		}
		break;
	case 19:
		_andyActionKeyMaskAnd    = _res->_mstOp56Data[num].unk0 & 255;
		_andyActionKeyMaskOr     = _res->_mstOp56Data[num].unk4 & 255;
		_andyDirectionKeyMaskAnd = _res->_mstOp56Data[num].unk8 & 255;
		_andyDirectionKeyMaskOr  = _res->_mstOp56Data[num].unkC & 255;
		break;
	case 20: {
			_mstUnk39 = 0;
			t->dataPtr->flagsA6 |= 2;
			t->run = &Game::runTask_idle;
			t->dataPtr->o16->actionKeyMask = _mstUnk39;
			t->dataPtr->o16->directionKeyMask = _andyObject->directionKeyMask;
			return 1;
		}
		break;
	case 21: {
			t->dataPtr->flagsA6 &= ~2;
			t->dataPtr->o16->actionKeyMask = 0;
			t->dataPtr->o16->directionKeyMask = 0;
		}
		break;
	case 27: {
			int index1 =  _res->_mstOp56Data[num].unk0;
			int type1  = (_res->_mstOp56Data[num].unkC >> 0xC) & 15;
			int a      = getTaskVar(t, index1, type1);
			int index2 =  _res->_mstOp56Data[num].unk4;
			int type2  = (_res->_mstOp56Data[num].unkC >> 0x8) & 15;
			int b      = getTaskVar(t, index2, type2);
			int index3 = _res->_mstOp56Data[num].unk8;
			int type3  = (_res->_mstOp56Data[num].unkC >> 0x4) & 15;
			int c      = getTaskVar(t, index3, type3);
			int index4 =  _res->_mstOp56Data[num].unkC >> 16;
			int type4  =  _res->_mstOp56Data[num].unkC         & 15;
			int d      = getTaskVar(t, index4, type4);
			setScreenMaskRect(a - 16, b, a + 16, c, d);
		}
		break;
	case 28:
		break;
	case 29: {
			const uint8_t state  = _res->_mstOp56Data[num].unk4 & 255;
			const uint8_t screen = _res->_mstOp56Data[num].unk0 & 255;
			_res->_screensState[screen].s0 = state;
		}
		break;
	case 30:
		++_levelCheckpoint;
		break;
	default:
		warning("Unhandled opcode %d in executeMstOp56", code);
		break;
	}
	return 0;
}

void Game::executeMstOp67(Task *t, int y1, int y2, int x1, int x2, int screen, int arg10, int o_flags1, int o_flags2, int arg1C, int arg20, int arg24) {
	warning("executeMstOp67 pos %d,%d,%d,%d %d %d 0x%x 0x%x %d %d %d", y1, x1, y2, x2, screen, arg10, o_flags1, o_flags2, arg1C, arg20, arg24);
	if (o_flags2 == 0xFFFF) {
		LvlObject *o = 0;
		if (t->dataPtr) {
			o = t->dataPtr->o16;
		} else if (t->mstObject) {
			o = t->mstObject->o;
		}
		o_flags2 = o ? o->flags2 : 0x3001;
	}
	if (y1 != y2) {
		y1 += _rnd.update() % ABS(y2 - y1 + 1);
	}
	if (x1 != x2) {
		x1 += _rnd.update() % ABS(x2 - x1 + 1);
	}

	// TODO

	if (arg1C != -128) {
		if (_mstVars[30] > 32) {
			_mstVars[30] = 32;
		}
		int count = 0;
		for (int i = 0; i < 32; ++i) {
			if (_mstUnkDataTable[i].o0) {
				++count;
			}
		}
		if (count >= _mstVars[30]) {
			return;
		}
		if (arg1C < 0) {
			// TODO
			// _res->_mstUnk42[arg24];
		} else {
// 415510
		}
// 415518
		for (int i = 0; i < 32; ++i) {
			if (!_mstUnkDataTable[i].o0) {
// 415539
				MstUnkData *m = &_mstUnkDataTable[i];

				memset(m->localVars, 0, sizeof(m->localVars));
				m->flags48 = 0x1C;
				m->flagsA5 = 0;
				m->unkEC = -1;
				m->unkD0 = 0;
				m->flagsA6 = 0;
				m->flags48 &= ~4;

				// TODO

				for (int j = 0; j < 64; ++j) {
					if (_mstObjectsTable[j].unk0 && _mstObjectsTable[j].unk8 == m) {
						_mstObjectsTable[j].unk8 = 0;
					}
				}
				break;
			}
		}
		return;
	} else {
		for (int i = 0; i < 64; ++i) {
			if (!_mstObjectsTable[i].unk0) {
// 415743


				break;
			}
		}
		return;
	}
}

int Game::runTask_wait(Task *t) {
	--t->delay;
	if (t->delay == 0) {
		t->run = &Game::runTask_default;
		return 0;
	}
	return 1;
}

int Game::runTask_waitResetInput(Task *t) {
	--t->delay;
	if (t->delay == 0) {
		t->run = &Game::runTask_default;
		LvlObject *o = 0;
		if (o) {
			o->actionKeyMask = 0;
			o->directionKeyMask = 0;
		}
		return 0;
	}
	return 1;
}

int Game::runTask_waitFlags(Task *t) {
	switch (t->delay) {
	case 1:
		if (t->mstFlags == 0) {
			return 1;
		}
		break;
	case 2:
		if ((t->flags & (1 << t->mstFlags)) == 0) {
			return 1;
		}
		break;
	case 3:
		if ((_mstFlags & (1 << t->mstFlags)) == 0) {
			return 1;
		}
		break;
	case 4:
		if (getTaskAndyVar(t->mstFlags, t) == 0) {
			return 1;
		}
		break;
	case 5: {
			MstUnkData *m = 0;
			if (t->mstObject) {
				m = t->mstObject->unk8;
			} else {
				m = t->dataPtr;
			}
			if (!m || (m->flags48 & (1 << t->mstFlags)) == 0) {
				return 1;
			}
		}
		break;
	}
	t->run = &Game::runTask_default;
	LvlObject *o = 0;
	if (t->dataPtr) {
		o = t->dataPtr->o16;
	} else if (t->mstObject) {
		o = t->mstObject->o;
	}
	if (o) {
		o->actionKeyMask = 0;
		o->directionKeyMask = 0;
	}
	return 0;
}

int Game::runTask_idle(Task *t) {
	return 1;
}
