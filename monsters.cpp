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
	0x4D, 0x4F, 0x4E, 0x90
};

void Game::resetMstUnkData(MstUnkData *m) {
	m->unk0 = 0;
	LvlObject *o = m->o;
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
	// TODO
	_rnd.initTable();
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
		_mstVars[31] = _res->_mstTickDelay;
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
	default:
		error("getTaskVar unhandled index %d type %d", index, type);
		break;
	}
	return 0;
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
//	case 22:
//		return _mstOp54Counter;
	case 23:
		return _andyObject->actionKeyMask;
	case 24:
		return _andyObject->directionKeyMask;
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
		const int code = _mstDefaultLutOp[p[0]];
		switch (code) {
		case 0: // 0
			// TODO:
			// fall-through
		case 1: {
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
		case 16: // 26
			_mstFlags &= ~(1 << p[1]);
			break;
		case 20: { // 30
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
		case 23: { // 33
				const int num = READ_LE_UINT16(p + 2);
				p = _res->_mstCodeData + (num - 1) * 4;
			}
			break;
		case 24: { // 35
				const int num = READ_LE_UINT16(p + 2);
				_res->flagMstCodeForPos(num, 1);
			}
			break;
		case 25: { // 36
				const int num = READ_LE_UINT16(p + 2);
				_res->flagMstCodeForPos(num, 0);
			}
			break;
		case 47: { // 177
				const int16_t num = READ_LE_UINT16(p + 2);
				assert(p[1] < 40);
				arithOp(p[0] - 177, &_mstVars[p[1]], num);
			}
			break;
		case 50: { // 198
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
		case 65: { // 218
				const int16_t val = READ_LE_UINT16(p + 2);
				if (val != _mstOp54Unk1) {
					_mstOp54Unk1 = val;
					_mstOp54Unk2 = val;
					// shuffleMstUnk43(_res->_mstUnk43 + val * 16);
				}
			}
			break;
		case 76: { // 239
				const int i = READ_LE_UINT16(p + 2);
				const uint32_t codeData = _res->_mstUnk60[i];
				assert(codeData != kNone);
				createTask(_res->_mstCodeData + codeData * 4);
			}
			break;
		case 78: // 242
			warning("Partial implementation for opcode 242 in runTask_default");
			removeTask(&_tasksListTail, t);
			ret = 1;
			break;
		default:
			warning("Unhandled opcode %d (%d) in runTask_default", *p, code);
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
	case 5:
		if (t->mstObject) {
			// TODO
		} else if (t->dataPtr) {
			// TODO
		} else {
			return 1;
		}
		break;
	}
	t->run = &Game::runTask_default;
	if (t->dataPtr) {
		// TODO
	} else if (t->mstObject) {
		// TODO
	}
	return 0;
}
