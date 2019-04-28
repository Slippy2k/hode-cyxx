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

void Game::resetMstTaskData(MstTaskData *m) {
	m->unk0 = 0;
	LvlObject *o = m->o16;
	if (o) {
		o->dataPtr = 0;
	}
	for (int i = 0; i < 64; ++i) {
		if (_mstObjectsTable[i].unk0 && _mstObjectsTable[i].mstTaskData == m) {
			_mstObjectsTable[i].mstTaskData = 0;
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

void Game::initMstTaskData(MstTaskData *m) {
	shuffleDword(m->unkCC);
	shuffleDword(m->unkC8);

	const uint8_t *ptr = m->unk8;
	const int num = (~m->flagsA5) & 1;

	if (!m->unkC) {
		warning("initMstTaskData m %p unkC is NULL", m);
		return;
	}

	m->x1 = m->unkC->unk2C[num] - READ_LE_UINT32(ptr + 904);
	m->x2 = m->unkC->unk34[num] + READ_LE_UINT32(ptr + 904);
	m->y1 = m->unkC->unk3C[num] - READ_LE_UINT32(ptr + 908);
	m->y2 = m->unkC->unk44[num] + READ_LE_UINT32(ptr + 908);

	const uint32_t indexUnk35 = m->unkC->indexUnk35_0x24[num];
	m->unkD0 = (indexUnk35 == kNone) ? 0 : &_res->_mstUnk35[indexUnk35];
}

int Game::addMstTaskData(MstUnk48 *m48, uint8_t flag) {
	m48->unk5 = flag;
	if (m48->codeData != kNone) {
		Task *t = createTask(_res->_mstCodeData + m48->codeData * 4);
		if (!t) {
			return 0;
		}
		while ((this->*(t->run))(t) == 0);
	}
	_mstUnk6 = m48 - _res->_mstUnk48;
	_mstTaskDataCount = 0;
	for (int i = 0; i < m48->countUnk12; ++i) {
		MstUnk48Unk12Unk4 *unk4 = m48->unk12[i].data;
		uint8_t code = unk4->unk1B;
		if (code >= 32) {
			warning("Invalid mstTaskData index %d", code);
			continue;
		}
		if (code != 255) {
			unk4->unk19 = flag;
			MstTaskData *m = &_mstUnkDataTable[code];
			m->unk18 = unk4;
			m->flags48 |= 0x40;
			m->flagsA5 &= 0x8A;
			m->flagsA5 |= 0x0A;
			initMstTaskData(m);
			Task *current = _mstTasksList1; // _eax
			Task *t = m->task; // _esi
			if (!t) {
				warning("addMstTaskData m %p m->task is NULL", m);
				return 0;
			}
			while (current) {
				Task *next = current->nextPtr;
				if (current == t) {
					removeTask(&_mstTasksList1, t);
					appendTask(&_mstTasksList1, t);
					break;
				}
				current = next;
			}
			const uint32_t codeData = unk4->codeData;
			assert(codeData != kNone);
			resetTask(t, _res->_mstCodeData + codeData * 4);
			++_mstTasksList1;
		}
	}

	return 0;
}

void Game::disableMstTaskData(MstTaskData *m) {
	m->flags48 &= ~0x50;
	m->unk18->unk1B = 255;
	m->unk18 = 0;
	--_mstTaskDataCount;
	if (_mstTaskDataCount <= 0) {
		_mstUnk6 = -1;
	}
}

int Game::updateMstTaskDataPosition(MstTaskData *m) {
	// TODO
	return 0;
}

int Game::prepareMstTask(Task *t) {
	MstTaskData *m = t->dataPtr;
	assert(m);
	MstUnk35 *mu = m->unkD0;
	int num = 0;
	if (mu->count2 != 0) {
		const uint8_t code = shuffleFlags(m->unkCC);
		num = mu->data2[code];
	}
	const uint32_t codeData = mu->indexCodeData[num];
	assert(codeData != kNone);
	resetTask(t, _res->_mstCodeData + codeData * 4);
	const int counter = m->executeCounter;
	m->executeCounter = _executeMstLogicCounter;
	return m->executeCounter - counter;
}

void Game::setMstTaskDataDefaultPos(Task *t) {
	MstTaskData *m = t->dataPtr;
	assert(m);
	LvlObject *o = m->o20;
	if (!o) {
		o = m->o16;
	}
	assert(o);
	m->xPos = o->xPos + o->posTable[7].x;
	m->yPos = o->yPos + o->posTable[7].y;
	m->xMstPos = m->xPos + _res->_mstPointOffsets[o->screenNum].xOffset;
	m->yMstPos = m->yPos + _res->_mstPointOffsets[o->screenNum].yOffset;

	const uint8_t *ptr = m->unk8;
	if (ptr[946] & 4) {
		warning("setMstTaskDataDefaultPos ptr[946] 0x%x", ptr[946]);
		// TODO
	}
// 40ECBD
	m->xDelta = _mstPosX - m->xMstPos;
	if (m->xDelta < 0) {
		m->xDelta = -m->xDelta;
		m->flags49 = 8;
	} else {
		m->flags49 = 2;
	}
	m->yDelta = _mstPosY - m->yMstPos;
	if (m->yDelta < 0) {
		m->yDelta = -m->yDelta;
		m->flags49 |= 1;
	} else {
		m->flags49 |= 4;
	}
	m->unkEC = -1;
	m->unk1C = 0;
// 40ED0D
	// TODO
// 40F151
	if (m->flags49 & 8) {
		m->unk88 = READ_LE_UINT32(ptr + 920);
		m->unk84 = READ_LE_UINT32(ptr + 924);
	} else {
		m->unk88 = READ_LE_UINT32(ptr + 912);
		m->unk84 = READ_LE_UINT32(ptr + 916);
	}
	if (m->flags49 & 1) {
		m->unk90 = READ_LE_UINT32(ptr + 936);
		m->unk8C = READ_LE_UINT32(ptr + 940);
	} else {
		m->unk90 = READ_LE_UINT32(ptr + 928);
		m->unk8C = READ_LE_UINT32(ptr + 932);
	}
// 40F1B3
	// TODO

	m->unkFC = -1;
	m->flags49 &= ~1;
}

void Game::shuffleMstUnk43(MstUnk43 *p) {
	for (uint32_t i = 0; i < p->count2; ++i) {
		p->data2[i] &= 0x7F;
	}
	shuffleArray(p->data2, p->count2);
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
		resetMstTaskData(&_mstUnkDataTable[i]);
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
		const int count = _res->_mstUnk35[i].count2;
		if (count != 0) {
			uint8_t *ptr = _res->_mstUnk35[i].data2;
			for (int j = 0; j < count * 2; ++j) {
				const int index1 = _rnd.update() % count;
				const int index2 = _rnd.update() % count;
				SWAP(ptr[index1], ptr[index2]);
			}
		}
	}

	// TODO
	_mstOp67_x1 = -256;
	_mstOp67_x2 = -256;
	memset(_mstUnkDataTable, 0, sizeof(_mstUnkDataTable));
	memset(_mstObjectsTable, 0, sizeof(_mstObjectsTable));
	memset(_mstVars, 0, sizeof(_mstVars));
	memset(_tasksTable, 0, sizeof(_tasksTable));
	_mstOp54Unk3 = _mstOp54Unk1 = _mstOp54Unk2 = _mstUnk6 = -1;
	// TODO
	_executeMstLogicPrevCounter = _executeMstLogicCounter = 0;
	// TODO
	_mstCurrentUnkFlag = 0;
	// TODO
	_mstOp67_y1 = 0;
	_mstOp67_y2 = 0;
	// TODO
	_mstLogicHelper1TestValue = 0;
	_mstLogicHelper1TestMask = 0xFFFFFFFF;
	// TODO
	_tasksList = 0;
	_mstTasksList1 = 0;
	_mstTasksList2 = 0;
	_mstTasksList3 = 0;
	_mstTasksList4 = 0;
	if (_res->_mstTickCodeData != kNone) {
		_mstVars[31] = _mstTickDelay = _res->_mstTickDelay;
	} else {
		_mstVars[31] = -1;
	}
	_mstVars[30] = 0x20;
	for (int i = 0; i < 32; ++i) {
		_mstUnkDataTable[i].soundType = 0;
	}
	for (int i = 0; i < 64; ++i) {
		_mstObjectsTable[i].unk0x10 = 0;
	}
	updateMstMoveData();
	_mstPrevPosX = _mstPosX;
	_mstPrevPosY = _mstPosY;
	for (int i = 0; i < _res->_mstHdr.unk0x3C; ++i) {
		// TODO
	}
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
	if (_levelCheckpoint < _res->_mstHdr.unk0x14) {
		const uint32_t codeData = _res->_mstScreenInitCodeData[_levelCheckpoint];
		if (codeData != kNone) {
			Task *t = createTask(_res->_mstCodeData + codeData * 4);
			if (t) {
				while ((this->*(t->run))(t) == 0);
			}
		}
	}
}

static bool compareOp(int op, int num1, int num2) {
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

static void arithOp(int op, int *p, int num) {
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
	if ((_mstLogicHelper1TestValue & _mstLogicHelper1TestMask) != 0) {
		// TODO
		_mstLogicHelper1TestValue = 0;
	}
	for (int i = 0; i < 8; ++i) {
		_mstMovingState[i].unk28 = 0;
		_mstMovingState[i].unk30 = 0;
		_mstMovingState[i].unk3C = 0x1000000;
	}
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
	for (Task *t = _tasksList; t; t = t->nextPtr) {
		_runTaskOpcodesCount = 0;
		while ((this->*(t->run))(t) == 0);
	}
	// TODO
	for (Task *t = _mstTasksList1; t; t = t->nextPtr) {
		_runTaskOpcodesCount = 0;
		if (executeMstCodeHelper3(t) == 0) {
			while ((this->*(t->run))(t) == 0);
		}
	}
	for (Task *t = _mstTasksList2; t; t = t->nextPtr) {
		_runTaskOpcodesCount = 0;
		if (executeMstCodeHelper4(t) == 0) {
			while ((this->*(t->run))(t) == 0);
		}
	}
}

void Game::executeMstCodeHelper2() {
	updateMstMoveData();
	updateMstHeightMapData();
	for (Task *t = _mstTasksList1; t; t = t->nextPtr) {
		setMstTaskDataDefaultPos(t);
	}
}

int Game::executeMstCodeHelper3(Task *t) {
	if (!t->run) {
		warning("executeMstCodeHelper3 t.run is NULL");
		return 1;
	}
	warning("executeMstCodeHelper3 unimplemented");
	// TODO
	return 0;
}

int Game::executeMstCodeHelper4(Task *t) {
	warning("executeMstCodeHelper4 unimplemented");
	// TODO
	return 0;
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
	Task *prev = t->prevPtr;
	t->codeData = 0;
	Task *next = t->nextPtr;
	if (next) {
		next->prevPtr = prev;
	}
	if (prev) {
		prev->nextPtr = next;
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
			t->prevPtr = 0;
			t->nextPtr = _tasksList;
			if (_tasksList) {
				_tasksList->prevPtr = t;
			}
			_tasksList = t;
			return t;
		}
	}
	return 0;
}

int Game::changeTask(Task *t, int num, int value) {
	warning("changeTask %d %d unimplemented", num, value);
	// TODO
	return 0;
}

void Game::updateTask(Task *t, int num, const uint8_t *codeData) {
	Task *current = _tasksList;
	bool found = false;
	while (current) {
		if (current->localVars[7] == num) {
			found = true;
			if (current != t) {
				if (!codeData) {
					if (current->child) {
						current->child->codeData = 0;
						current->child = 0;
					}
					Task *prev = current->prevPtr;
					current->codeData = 0;
					Task *next = current->nextPtr;
					if (next) {
						next->prevPtr = prev;
					}
					if (prev) {
						prev->nextPtr = next;
					} else {
						_tasksList = next;
					}
				} else {
					t->codeData = codeData;
					t->run = &Game::runTask_default;
				}
			}
		}
		current = current->nextPtr;
	}
	if (found) {
		return;
	}
	if (!codeData) {
		return;
	}
	for (int i = 0; i < kMaxTasks; ++i) {
		Task *t = &_tasksTable[i];
		if (!t->codeData) {
			memset(t, 0, sizeof(Task));
			resetTask(t, codeData);
			t->prevPtr = 0;
			t->nextPtr = _tasksList;
			if (_tasksList) {
				_tasksList->prevPtr = t;
			}
			_tasksList = t;
			t->localVars[7] = num;
			break;
		}
	}
}

void Game::resetTask(Task *t, const uint8_t *codeData) {
	t->runningState |= 2;
	t->codeData = codeData;
	t->run = &Game::runTask_default;
	t->localVars[7] = 0;
	MstTaskData *m = t->dataPtr;
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
	Task *prev = t->prevPtr;
	t->codeData = 0;
	Task *next = t->nextPtr;
	if (next) {
		next->prevPtr = prev;
	}
	if (prev) {
		prev->nextPtr = next;
	} else {
		*tasksList = next;
	}
}

void Game::appendTask(Task **tasksList, Task *t) {
	Task *current = *tasksList;
	if (!current) {
		*tasksList = current;
		t->nextPtr = t->prevPtr = 0;
	} else {
		// go to last element
		Task *next = current->nextPtr;
		while (next) {
			current = next;
			next = current->nextPtr;
		}
		current->nextPtr = t;
		t->nextPtr = 0;
		t->prevPtr = current;
	}
}

int Game::getTaskVar(Task *t, int index, int type) const {
	switch (type) {
	case 1:
		return index;
	case 2:
		assert(index < kMaxLocals);
		return t->localVars[index];
	case 3:
		assert(index < kMaxVars);
		return _mstVars[index];
	case 4:
		return getTaskOtherVar(index, t);
	case 5:
		{
			MstTaskData *m = 0;
			if (t->mstObject) {
				m = t->mstObject->mstTaskData;
			} else {
				m = t->dataPtr;
			}
			if (m) {
				return m->localVars[index];
			}
		}
		break;
	default:
		warning("getTaskVar unhandled index %d type %d", index, type);
		break;
	}
	return 0;
}

void Game::setTaskVar(Task *t, int index, int type, int value) {
	switch (type) {
	case 2:
		assert(index < kMaxLocals);
		t->localVars[index] = value;
		break;
	case 3:
		assert(index < kMaxVars);
		_mstVars[index] = value;
		break;
	case 5: {
			MstTaskData *m = 0;
			if (t->mstObject) {
				m = t->mstObject->mstTaskData;
			} else {
				m = t->dataPtr;
			}
			if (m) {
				m->localVars[index] = value;
			}
		}
		break;
	default:
		warning("setTaskVar unhandled index %d type %d", index, type);
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
		return (_andyObject->spriteNum == 0);
	default:
		warning("getTaskAndyVar unhandled index %d", index);
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
	case 12:
		if (t->dataPtr) {
			return t->dataPtr->xPos;
		} else if (t->mstObject) {
			return t->mstObject->xPos;
		}
		return 0;
	case 13:
		if (t->dataPtr) {
			return t->dataPtr->yPos;
		} else if (t->mstObject) {
			return t->mstObject->yPos;
		}
		return 0;
	case 14:
		if (t->dataPtr) {
			return t->dataPtr->o16->screenNum;
		} else if (t->mstObject) {
			return t->mstObject->o->screenNum;
		}
		return 0;
	case 22:
		return _mstOp54Counter;
	case 23:
		return _andyObject->actionKeyMask;
	case 24:
		return _andyObject->directionKeyMask;
	case 25:
		if (t->dataPtr) {
			return t->dataPtr->xMstPos;
		} else if (t->mstObject) {
			return t->mstObject->xMstPos;
		}
		return 0;
	case 26:
		if (t->dataPtr) {
			return t->dataPtr->yMstPos;
		} else if (t->mstObject) {
			return t->mstObject->yMstPos;
		}
		return 0;
	case 27:
		if (t->dataPtr) {
			return t->dataPtr->o16->flags0;
		} else if (t->mstObject) {
			return t->mstObject->o->flags0;
		}
		return 0;
	case 28:
		if (t->dataPtr) {
			return t->dataPtr->o16->anim;
		} else if (t->mstObject) {
			return t->mstObject->o->anim;
		}
		return 0;
	case 29:
		if (t->dataPtr) {
			return t->dataPtr->o16->frame;
		} else if (t->mstObject) {
			return t->mstObject->o->frame;
		}
		return 0;
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
		warning("getTaskOtherVar unhandled index %d", index);
		break;
	}
	return 0;
}

int Game::getTaskFlag(Task *t, int num, int type) const {
	switch (type) {
	case 1:
		return num;
	case 2:
		return ((t->flags & (1 << num)) != 0) ? 1 : 0;
	case 3:
		return ((_mstFlags & (1 << num)) != 0) ? 1 : 0;
	case 4:
		return getTaskAndyVar(num, t);
	case 5: {
			MstTaskData *m = 0;
			if (t->mstObject) {
				m = t->mstObject->mstTaskData;
			} else {
				m = t->dataPtr;
			}
			if (m) {
				return ((m->flags48 & (1 << num)) != 0) ? 1 : 0;
			}
		}
	default:
		warning("getTaskFlag unhandled type %d num %d", type, num);
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
		debug(kDebug_MONSTER, "executeMstCode code %d", p[0]);
		assert(p[0] <= 242);
		switch (p[0]) {
		case 0: { // 0
				LvlObject *o = 0;
				if (t->dataPtr) {
					if ((t->dataPtr->flagsA6 & 2) == 0) {
						o = t->dataPtr->o16;
					}
				} else if (t->mstObject) {
					o = t->mstObject->o;
				}
				if (o) {
					o->actionKeyMask = 0;
					o->directionKeyMask = 0;
				}
			}
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
		case 2: { // 2 - set_var_random_range
				const int num = READ_LE_UINT16(p + 2);
				MstUnk56 *m = &_res->_mstUnk56[num];
				int a = getTaskVar(t, m->indexVar1, m->maskVars >> 4); // _ebx
				int b = getTaskVar(t, m->indexVar2, m->maskVars & 15); // _esi
				if (a > b) {
					SWAP(a, b);
				}
				a += _rnd.update() % (b - a + 1);
				setTaskVar(t, m->unkA, m->unk9, a);
			}
			break;
		case 3: // 3 - change_task_imm
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				ret = changeTask(t, num, (arg == 0xFF) ? -1 : arg);
			}
			break;
		case 4: // 4 - change_task_local_var
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				assert(arg < kMaxLocals);
				ret = changeTask(t, num, t->localVars[arg]);
			}
			break;
		case 5: // 5 - change_task_global_var
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				assert(arg < kMaxVars);
				ret = changeTask(t, num, _mstVars[arg]);
			}
			break;
		case 6: // 6 - change_task_other_var
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				ret = changeTask(t, num, getTaskOtherVar(arg, t));
			}
			break;
		case 7: // 7 - change_task_mst_var
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				assert(arg < kMaxLocals);
				ret = changeTask(t, num, t->dataPtr->localVars[arg]);
			}
			break;
		case 23: // 13 - set_flag_global
			_mstFlags |= (1 << p[1]);
			break;
		case 24: // 14 - set_flag_task
			t->flags |= (1 << p[1]);
			break;
		case 25: { // 15 - set_flag_mst
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					m->flags48 |= (1 << p[1]);
				}
			}
			break;
		case 26: // 16 - unset_flag_global
			_mstFlags &= ~(1 << p[1]);
			break;
		case 27: // 17 - unset_flag_task
			t->flags &= ~(1 << p[1]);
			break;
		case 28: { // 18 - unset_flag_mst
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					m->flags48 &= ~(1 << p[1]);
				}
			}
			break;
		case 29: { // 19
				t->delay = 4;
				t->tempVar = p[1];
				if (getTaskAndyVar(p[1], t) == 0) {
					LvlObject *o = 0;
					if (t->dataPtr) {
						if ((t->dataPtr->flagsA6 & 2) == 0) {
							o = t->dataPtr->o16;
						}
					} else if (t->mstObject) {
						o = t->mstObject->o;
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
				t->tempVar = 1 << p[1];
				if ((t->tempVar & _mstFlags) == 0) {
					LvlObject *o = 0;
					if (t->dataPtr) {
						if ((t->dataPtr->flagsA6 & 2) == 0) {
							o = t->dataPtr->o16;
						}
					} else if (t->mstObject) {
						o = t->mstObject->o;
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
		case 33:
		case 229: { // 23 - jmp_imm
				const int num = READ_LE_UINT16(p + 2);
				p = _res->_mstCodeData + (num - 1) * 4;
			}
			break;
		case 35: { // 24 - enable_trigger
				const int num = READ_LE_UINT16(p + 2);
				_res->flagMstCodeForPos(num, 1);
			}
			break;
		case 36: { // 25 - disable_trigger
				const int num = READ_LE_UINT16(p + 2);
				_res->flagMstCodeForPos(num, 0);
			}
			break;
		case 39: // 26
			if (p[1] < _res->_mstHdr.pointsCount) {
				executeMstOp26(&_mstTasksList1, p[1]);
				executeMstOp26(&_mstTasksList2, p[1]);
				executeMstOp26(&_mstTasksList3, p[1]);
				executeMstOp26(&_mstTasksList4, p[1]);
			}
			break;
		case 40: // 29
			if (p[1] < _res->_mstHdr.pointsCount) {
				executeMstOp27(&_mstTasksList1, p[1], p[2]);
				executeMstOp27(&_mstTasksList2, p[1], p[2]);
				executeMstOp27(&_mstTasksList3, p[1], p[2]);
				executeMstOp27(&_mstTasksList4, p[1], p[2]);
			}
			break;
		case 41: { // 28 - increment_local_var
				assert(p[1] < kMaxLocals);
				++t->localVars[p[1]];
			}
			break;
		case 42: { // 29 - increment_global_var
				const int num = p[1];
				assert(num < kMaxVars);
				++_mstVars[num];
			}
			break;
		case 44: { // 31 - decrement_task_var
				const int num = p[1];
				assert(num < kMaxLocals);
				--t->localVars[num];
			}
			break;
		case 45: { // 32 - decrement_global_var
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
		case 56: { // 34 - arith_local_var_local_var
				assert(p[1] < kMaxLocals);
				assert(p[2] < kMaxLocals);
				arithOp(p[0] - 47, &t->localVars[p[1]], t->localVars[p[2]]);
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
		case 66: { // 35 - arith_global_var_local_var
				assert(p[1] < kMaxVars);
				assert(p[2] < kMaxLocals);
				arithOp(p[0] - 57, &_mstVars[p[1]], t->localVars[p[2]]);
				if (p[1] == 31 && _mstVars[31] > 0) {
					_mstTickDelay = _mstVars[31];
				}
			}
			break;
		case 67:
		case 68:
		case 69:
		case 70:
		case 71:
		case 72:
		case 73:
		case 74:
		case 75:
		case 76: { // 36
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					assert(p[2] < kMaxLocals);
					arithOp(p[0] - 67, &m->localVars[p[1]], t->localVars[p[2]]);
				}
			}
			break;
		case 77:
		case 78:
		case 79:
		case 80:
		case 81:
		case 82:
		case 83:
		case 84:
		case 85:
		case 86: { // 37
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					assert(p[1] < kMaxLocals);
					arithOp(p[0] - 77, &t->localVars[p[1]], m->localVars[p[2]]);
				}
			}
			break;
		case 87:
		case 88:
		case 89:
		case 90:
		case 91:
		case 92:
		case 93:
		case 94:
		case 95:
		case 96: { // 38
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					assert(p[1] < kMaxVars);
					arithOp(p[0] - 87, &_mstVars[p[1]], m->localVars[p[2]]);
					if (p[1] == 31 && _mstVars[31] > 0) {
						_mstTickDelay = _mstVars[31];
					}
				}
			}
			break;
		case 97:
		case 98:
		case 99:
		case 100:
		case 101:
		case 102:
		case 103:
		case 104:
		case 105:
		case 106: { // 39
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					arithOp(p[0] - 97, &m->localVars[p[1]], m->localVars[p[2]]);
				}
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
				if (p[1] == 31 && _mstVars[31] > 0) {
					_mstTickDelay = _mstVars[31];
				}
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
				const int16_t num = READ_LE_UINT16(p + 2);
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
				if (p[1] == 31 && _mstVars[31] > 0) {
					_mstTickDelay = _mstVars[31];
				}
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
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
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
					memcpy(child, t, sizeof(Task));
					t->child = child;
					const uint16_t num = READ_LE_UINT16(p + 2);
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
		case 207: // 79
			break;
		case 211: // 58
			executeMstOp58(t, READ_LE_UINT16(p + 2));
			break;
		case 215: { // 62
				if (_mstOp54Unk3 != -1) {
					assert(_mstOp54Unk3 < _res->_mstHdr.unk0x24);
					shuffleMstUnk43(&_res->_mstUnk43[_mstOp54Unk3]);
				}
				_mstOp54Counter = 0;
			}
			break;
		case 216: { // 63
				if (_mstOp54Unk1 != -1) {
					assert(_mstOp54Unk1 < _res->_mstHdr.unk0x24);
					shuffleMstUnk43(&_res->_mstUnk43[_mstOp54Unk1]);
				}
			}
			break;
		case 217: { // 64
				const int16_t num = READ_LE_UINT16(p + 2);
				if (_mstOp54Unk3 != num) {
					_mstOp54Unk3 = num;
					assert(num >= 0 && num < _res->_mstHdr.unk0x24);
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
					assert(num >= 0 && num < _res->_mstHdr.unk0x24);
					shuffleMstUnk43(&_res->_mstUnk43[num]);
				}
			}
			break;
		case 219: { // 66
				const int16_t num = READ_LE_UINT16(p + 2);
				_mstOp54Unk2 = num;
			}
			break;
		case 220:
		case 221:
		case 222:
		case 223:
		case 224:
		case 225: { // 67
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
					warning(".mst opcode 224 unimplemented");
					// TODO
					break;
				} else if (p[0] == 225) {
					warning(".mst opcode 225 unimplemented");
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
				const MstUnk54 *m = &_res->_mstUnk54[num];
				const int a = getTaskVar(t, m->indexVar1, m->maskVars & 15);
				const int b = getTaskVar(t, m->indexVar2, m->maskVars >> 4);
				if (compareOp(m->compare, a, b)) {
					assert(m->codeData != kNone);
					p = _res->_mstCodeData + m->codeData * 4 - 4;
				}
			}
			break;
		case 228: { // 70
				const int num = READ_LE_UINT16(p + 2);
				const MstUnk54 *m = &_res->_mstUnk54[num];
				const int a = getTaskFlag(t, m->indexVar1, m->maskVars & 15);
				const int b = getTaskFlag(t, m->indexVar2, m->maskVars >> 4);
				if (compareOp(m->compare, a, b)) {
					assert(m->codeData != kNone);
					p = _res->_mstCodeData + m->codeData * 4 - 4;
				}
			}
			break;
		case 233:
		case 234: { // 72
				const int num = READ_LE_UINT16(p + 2);
				const MstUnk55 *m = &_res->_mstUnk55[num];
				const int a = getTaskVar(t, m->indexVar1, m->maskVars & 15);
				const int b = getTaskVar(t, m->indexVar2, m->maskVars >> 4);
				if (compareOp(m->compare, a, b)) {
					if (p[0] == 233) {
						LvlObject *o = 0;
						if (t->dataPtr) {
							if ((t->dataPtr->flagsA6 & 2) == 0) {
								o = t->dataPtr->o16;
							}
						} else if (t->mstObject) {
							o = t->mstObject->o;
						}
						if (o) {
							o->actionKeyMask = 0;
							o->directionKeyMask = 0;
						}
						t->tempVar = num;
						t->run = &Game::runTask_mstUnk55_233;
						ret = 1;
					}
				} else {
					if (p[0] == 234) {
						LvlObject *o = 0;
						if (t->dataPtr) {
							if ((t->dataPtr->flagsA6 & 2) == 0) {
								o = t->dataPtr->o16;
							}
						} else if (t->mstObject) {
							o = t->mstObject->o;
						}
						if (o) {
							o->actionKeyMask = 0;
							o->directionKeyMask = 0;
						}
						t->tempVar = num;
						t->run = &Game::runTask_mstUnk55_234;
						ret = 1;
					}
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
				const uint8_t *codeData = (m->codeData == kNone) ? 0 : _res->_mstCodeData + m->codeData * 4;
				updateTask(t, m->taskId, codeData);
			}
			break;
		case 242: // 78
			if (t->child) {
				Task *child = t->child;
				child->prevPtr = t->prevPtr;
				child->nextPtr = t->nextPtr;
				memcpy(t, child, sizeof(Task));
				t->child = 0;
				t->runningState &= ~2;
				child->codeData = 0;
				MstTaskData *m = t->dataPtr;
				if (m) {
					m->flagsA5 &= ~0x70;
					if ((m->flagsA5 & 8) != 0) {
						// TODO
					}
				}
				return 0;
			} else if (t->dataPtr) {
				MstTaskData *m = t->dataPtr;
				if (m->flagsA6 & 4) {
					return 1;
				}
				if ((m->flagsA5 & 0x80) == 0) {
					if ((m->flagsA5 & 8) == 0) {
						m->flags48 |= 8;
						warning(".mst opcode 242 mstTaskData is not NULL flagsA5 0x%x", m->flagsA5);
						if ((m->flagsA5 & 0x70) != 0) {
							m->flagsA5 &= ~0x70;
/*
							switch (m->flagsA5 & 7) {
							case 1:
							case 2:
// 413D74
								// TODO
								break;
							case 5:
								return executeMstOp67Type1(t);
							case 6:
								return executeMstOp67Type2(t, 1);
*/
						} else {
// 413DCA
						}
					} else {
// 413FE3
						if (m->unk18) {
							disableMstTaskData(m);
						}
						m->flagsA5 = (m->flagsA5 & ~0xF) | 6;
						executeMstOp67Type2(t, 1);
					}
				} else if ((m->flagsA5 & 8) != 0) {
// 413F8B
					m->flagsA5 &= ~8;
					const uint32_t codeData = m->unk4->codeData;
					assert(codeData != kNone);
					resetTask(t, _res->_mstCodeData + codeData * 4);
				} else {
					t->run = &Game::runTask_idle;
				}
			} else if (t->mstObject) {
				warning(".mst opcode 242 mstObject is not NULL");
				// TODO
			} else {
				if ((t->runningState & 1) != 0 && _mstVars[31] == 0) {
					_mstVars[31] = _mstTickDelay;
				}
				removeTask(&_tasksList, t);
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

// remove references to mst tasks
void Game::executeMstOp26(Task **tasksList, int screenNum) {
	Task *current = *tasksList; // _esi
	while (current) {
		MstTaskData *m = current->dataPtr; // _ecx
		Task *next = current->nextPtr; // _ebp
		if (m && m->o16->screenNum == screenNum) {
			if (_mstUnk6 != -1 && (m->flagsA5 & 8) != 0 && m->unk18 != 0) {
				disableMstTaskData(m);
			}
/*
			const uint8_t *ptr = m->unk8;
			if (ptr[946] & 4) {
				clearMstRectsTable(m, 0);
				clearMstRectsTable(m, 1);
			}
*/
			m->unk0 = 0;
			m->o16->dataPtr = 0;
			for (int i = 0; i < 64; ++i) {
				if (_mstObjectsTable[i].unk0 != 0 && _mstObjectsTable[i].mstTaskData == m) {
					_mstObjectsTable[i].unk0 = 0;
				}
			}
			removeLvlObject2(m->o16);
			Task *child = current->child;
			if (child) {
				child->codeData = 0;
				current->child = 0;
			}
			Task *prevPtr = current->prevPtr;
			current->codeData = 0;
			Task *nextPtr = current->nextPtr;
			if (nextPtr) {
				nextPtr->prevPtr = prevPtr;
			}
			if (prevPtr) {
				prevPtr->nextPtr = nextPtr;
			} else {
				*tasksList = nextPtr;
			}
		} else {
			MstObject *mo = current->mstObject;
			if (mo && mo->o->screenNum == screenNum) {
				mo->unk0 = 0;
				mo->o->dataPtr = 0;
				removeLvlObject2(mo->o);
				Task *child = current->child;
				if (child) {
					child->codeData = 0;
					current->child = 0;
				}
				Task *prevPtr = current->prevPtr;
				current->codeData = 0;
				Task *nextPtr = current->nextPtr;
				if (nextPtr) {
					nextPtr->prevPtr = prevPtr;
				}
				if (prevPtr) {
					prevPtr->nextPtr = nextPtr;
				} else {
					*tasksList = nextPtr;
				}
			}
		}
		current = next;
	}
}

void Game::executeMstOp27(Task **tasksList, int num, int arg) {
	warning("executeMstOp27 %d %d unimplemented", num, arg);
	// TODO
}

static int checkMstOp54Helper(MstUnk48 *m, uint8_t flag) {
	warning("checkMstOp54Helper %d unimplemented", flag);
	return 1;
}

void Game::executeMstOp54() {
	if (_mstUnk6 != -1) {
		return;
	}
	MstUnk43 *m43 = 0;
	if (_mstFlags & 0x20000000) {
		if (_mstOp54Unk2 == -1) {
			return;
		}
		m43 = &_res->_mstUnk43[_mstOp54Unk2];
	} else {
		if (_mstOp54Unk3 == -1) {
			return;
		}
		m43 = &_res->_mstUnk43[_mstOp54Unk3];
		_mstOp54Unk2 = _mstOp54Unk1;
	}
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
	warning("executeMstOp54 unimplemented m43->count2 %d", m43->count2);
	// game_unk47
	if (m43->count2 == 0) {
// TODO
	} else {
// 41E3CA
		memset(_mstOp54Table, 0, sizeof(_mstOp54Table));
		int var4 = 0;
		uint32_t i = 0;
		for (; i < m43->count2; ++i) {
			uint8_t *ptr = m43->data2;
			uint8_t code = ptr[i];
			if ((code & 0x80) == 0) {
				code &= 0x7F;
				var4 = 1;
				if (_mstOp54Table[code] == 0) {
					_mstOp54Table[code] = 1;
					uint32_t indexUnk48 = m43->indexUnk48[code];
					assert(indexUnk48 != kNone);
					MstUnk48 *m48 = &_res->_mstUnk48[indexUnk48];
					if (m48->unk4 == 0) {
						if (checkMstOp54Helper(m48, 0) && addMstTaskData(m48, 0)) {
							break; // goto 41E494;
						}
					} else {
						int flag = _rnd.update() & 1;
						if (checkMstOp54Helper(m48, flag) && addMstTaskData(m48, flag)) {
							break; // goto 41E494;
						}
						flag ^= 1;
						if (checkMstOp54Helper(m48, flag) && addMstTaskData(m48, flag)) {
							break; // goto 41E494;
						}
					}
				}
			}
		}
// 41E494
		if (_mstUnk6 != -1) {
			m43->data2[i] |= 0x80;
		} else {
// 41E4AC
			if (var4 != 0) {
				++_mstOp54Counter;
				if (_mstOp54Counter <= 16) {
					return;
				}
			}
			_mstOp54Counter = 0;
			if (m43->count2 != 0) {
				for (uint32_t i = 0; i < m43->count2; ++i) {
					m43->data2[i] &= 0x7F;
				}
				shuffleArray(m43->data2, m43->count2);
			}
		}
	}
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
	debug(kDebug_MONSTER, "executeMstOp56 code %d", code);
	switch (code) {
	case 0:
		if (_mstCurrentUnkFlag == 0 && resetAndyLvlObjectPlasmaCannonKeyMask(0x71) != 0) {
			_plasmaCannonFlags |= 1;
			if (_andyObject->spriteNum == 0) {
				_mstCurrentAnim = _res->_mstOp56Data[num].unk0 & 0xFFFF;
			} else {
				_mstCurrentAnim = _res->_mstOp56Data[num].unk0 >> 16;
			}
// 411AB4
			LvlObject *o = 0;
			if (t->mstObject) {
				o = t->mstObject->o;
			} else if (t->dataPtr) {
				o = t->dataPtr->o16;
			}
			if (_res->_mstOp56Data[num].unkC != 6 && o) {
				LvlObject *tmpObject = t->dataPtr->o16;
				const uint8_t flags = getLvlObjectFlag4(_res->_mstOp56Data[num].unkC, tmpObject, _andyObject);
				_mstCurrentFlags1 = ((flags & 3) << 4) | (_mstCurrentFlags1 & 0xFFCF);
				_mstCurrentScreenNum = tmpObject->screenNum;
				_currentMonsterObject = tmpObject;
				_mstOriginPosX = _res->_mstOp56Data[num].unk4 & 0xFFFF;
				_mstOriginPosY = _res->_mstOp56Data[num].unk8 & 0xFFFF;
			} else {
				_mstCurrentFlags1 = merge_bits(_mstCurrentFlags1, _andyObject->flags1, 0x30); // _mstCurrentFlags1 ^= (_mstCurrentFlags1 ^ _andyObject->flags1) & 0x30;
				_mstCurrentScreenNum = _andyObject->screenNum;
				_currentMonsterObject = _andyObject;
				_mstOriginPosX = _andyObject->posTable[3].x + _andyObject->posTable[6].x;
				_mstOriginPosY = _andyObject->posTable[3].y + _andyObject->posTable[6].y;
			}
			_mstCurrentUnkFlag = 1;
		}
// 411BBA
		// TODO
		break;
	case 1:
		if (_mstCurrentUnkFlag != 0) {
			warning("Op56 subopcode 1 unimplemented");
			if (resetAndyLvlObjectPlasmaCannonKeyMask(0x61) != 0) {
				_plasmaCannonFlags &= ~1;
				if (_andyObject->spriteNum == 0) {
					_mstCurrentAnim = _res->_mstOp56Data[num].unk0 & 0xFFFF;
				} else {
					_mstCurrentAnim = _res->_mstOp56Data[num].unk0 >> 16;
				}
// 4118ED
				// TODO
			}
// 4119F5
			// TODO
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
	case 13:
	case 14:
	case 22:
	case 23:
	case 24:
	case 25: {
			const MstOp56Data *dat = &_res->_mstOp56Data[num];
			const int mask = dat->unkC;
			int xPos = getTaskVar(t, dat->unk0, (mask >> 8) & 15); // _edi
			int yPos = getTaskVar(t, dat->unk4, (mask >> 4) & 15); // _esi
			int screenNum = getTaskVar(t, dat->unk8, mask & 15); // _eax
			LvlObject *o = 0;
			if (t->mstObject) {
				o = t->mstObject->o;
			} else if (t->dataPtr) {
				o = t->dataPtr->o16;
			}
			if (screenNum < 0) {
				if (screenNum == -2) {
					screenNum = o->screenNum;
					if (t->mstObject) {
						xPos += t->mstObject->xMstPos;
						yPos += t->mstObject->yMstPos;
					}
				} else if (screenNum == -1) {
					xPos += _mstPosX;
					yPos += _mstPosY;
					screenNum = _currentScreen;
				} else {
// 4114B3
					xPos += o->posTable[6].x - o->posTable[7].x;
					yPos += o->posTable[6].y - o->posTable[7].y;
				}
			} else {
// 411545
				if (screenNum >= _res->_mstHdr.pointsCount) {
					screenNum = _res->_mstHdr.pointsCount - 1;
				}
				xPos += _res->_mstPointOffsets[screenNum].xOffset;
				yPos += _res->_mstPointOffsets[screenNum].yOffset;
				if (code == 13) {
					if (o) {
						xPos -= _res->_mstPointOffsets[screenNum].xOffset;
						xPos -= o->posTable[7].x;
						yPos -= _res->_mstPointOffsets[screenNum].yOffset;
						yPos -= o->posTable[7].y;
						o->screenNum = screenNum;
						o->xPos = xPos;
						o->yPos = yPos;
						setLvlObjectPosInScreenGrid(o, 7);
						if (t->mstObject) {
							setMstObjectDefaultPos(t);
						} else {
							setMstTaskDataDefaultPos(t);
						}
					}
				} else if (code == 14) {
					if (_andyObject) {
						const int pos = dat->unkC >> 16;
						xPos -= _res->_mstPointOffsets[screenNum].xOffset;
						xPos -= _andyObject->posTable[pos].x;
						yPos -= _res->_mstPointOffsets[screenNum].yOffset;
						yPos -= _andyObject->posTable[pos].y;
						_andyObject->screenNum = screenNum;
						_andyObject->xPos = xPos;
						_andyObject->yPos = yPos;
						updateLvlObjectScreen(_andyObject);
						updateMstMoveData();
						updateMstHeightMapData();
					}
				} else if (code == 22) {
					updateScreenMaskBuffer(xPos, yPos, 1);
				} else if (code == 24) {
					updateScreenMaskBuffer(xPos, yPos, 2);
				} else if (code == 25) {
					updateScreenMaskBuffer(xPos, yPos, 3);
				} else {
					assert(code == 23);
					updateScreenMaskBuffer(xPos, yPos, 0);
				}
			}
		}
		break;
	case 15: {
			_andyObject->anim  = _res->_mstOp56Data[num].unk0;
			_andyObject->frame = _res->_mstOp56Data[num].unk4;
			LvlObject *o = 0;
			if (t->mstObject) {
				o = t->mstObject->o;
			} else if (t->dataPtr) {
				o = t->dataPtr->o16;
			} else {
				o = _andyObject;
			}
			uint8_t flag = getLvlObjectFlag4(_res->_mstOp56Data[num].unk8, o, _andyObject);
			_andyObject->flags1 = ((flag & 3) << 4) | (_andyObject->flags1 & 0xFFCF);
			const int x3 = _andyObject->posTable[3].x;
			const int y3 = _andyObject->posTable[3].y;
			setupLvlObjectBitmap(_andyObject);
			_andyObject->xPos += (x3 - _andyObject->posTable[3].x);
			_andyObject->yPos += (y3 - _andyObject->posTable[3].y);
			updateLvlObjectScreen(o);
			updateMstMoveData();
			updateMstHeightMapData();
		}
		break;
	case 16:
	case 17: {
			LvlObject *o = _andyObject;
			if (code == 16) {
				if (t->mstObject) {
					o = t->mstObject->o;
				} else if (t->dataPtr) {
					o = t->dataPtr->o16;
				}
			}
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
			_mstCurrentActionKeyMask = _res->_mstOp56Data[num].unk0 & 255;
		}
		break;
	case 19:
		_andyActionKeyMaskAnd    = _res->_mstOp56Data[num].unk0 & 255;
		_andyActionKeyMaskOr     = _res->_mstOp56Data[num].unk4 & 255;
		_andyDirectionKeyMaskAnd = _res->_mstOp56Data[num].unk8 & 255;
		_andyDirectionKeyMaskOr  = _res->_mstOp56Data[num].unkC & 255;
		break;
	case 20: {
			_mstCurrentActionKeyMask = 0;
			t->dataPtr->flagsA6 |= 2;
			t->run = &Game::runTask_idle;
			t->dataPtr->o16->actionKeyMask = _mstCurrentActionKeyMask;
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

void Game::executeMstOp58(Task *t, int num) {
	const MstOp58Data *dat = &_res->_mstOp58Data[num];
	const int mask = dat->unkE;
	int xPos = getTaskVar(t, dat->indexVar1, (mask >> 8) & 15); // _ebx
	int yPos = getTaskVar(t, dat->indexVar2, (mask >> 4) & 15); // _ebp
	const int type = getTaskVar(t, dat->unkC, mask & 15) & 255; // _eax
	LvlObject *o = 0;
	if (t->mstObject) {
		o = t->mstObject->o;
	} else if (t->dataPtr) {
		o = t->dataPtr->o16;
	}
	uint8_t screen = type;
	if (type == 0xFB) { // -5
		if (!o) {
			return;
		}
		xPos += o->xPos + o->posTable[6].x;
		yPos += o->yPos + o->posTable[6].y;
		screen = o->screenNum;
	} else if (type == 0xFE) { // -2
		if (!o) {
			return;
		}
		xPos += o->xPos + o->posTable[7].x;
		yPos += o->yPos + o->posTable[7].y;
		screen = o->screenNum;
	} else if (type == 0xFF) { // -1
		xPos += _mstRefPosX; // _ebx
		yPos += _mstRefPosY; // _ebp
		screen = _currentScreen;
	}
	uint16_t flags = (dat->unk6 == -1 && o) ? o->flags2 : 0x3001;
	o = addLvlObject(2, xPos, yPos, screen, dat->unk8, dat->unk4, dat->unkB, flags, dat->unk9, dat->unkA);
	if (o) {
		o->dataPtr = 0;
	}
}

void Game::executeMstUnk1(Task *t) {
	MstTaskData *m = t->dataPtr;
	t->run = &Game::runTask_default;
	m->o16->actionKeyMask = 0;
	m->o16->directionKeyMask = 0;
	if ((m->flagsA5 & 4) != 0 && (m->flagsA5 & 0x28) == 0) {
		switch (m->flagsA5 & 7) {
		case 5:
			m->flagsA5 = (m->flagsA5 & ~6) | 1;
			if (updateMstTaskDataPosition(m) == 0) {
				initMstTaskData(m);
			}
			prepareMstTask(t);
			break;
		case 4:
			m->flagsA5 &= ~7;
			if (executeMstUnk2(m, m->xMstPos, m->yMstPos) == 0) {
				m->flagsA5 |= 1;
				if (updateMstTaskDataPosition(m) == 0) {
					initMstTaskData(m);
				}
				// TODO
				// if (m->unkC->indexUnk35_20 != kNone) {
				//	m->unkD0 = m->unkC->indexUnk35_20;
				// }
				prepareMstTask(t);
			} else {
				m->flagsA5 |= 2;
				if (updateMstTaskDataPosition(m) == 0) {
					initMstTaskData(m);
				}
				prepareMstTask(t);
			}
			break;
		}
	} else {
		m->flagsA5 &= ~4;
		updateMstTaskDataPosition(m);
	}
}

int Game::executeMstUnk2(MstTaskData *m, int x, int y) {
	// TODO
	return 1;
}

void Game::executeMstOp67Type1(Task *t) {
	t->flags &= ~0x80;
	MstTaskData *m = t->dataPtr;
	m->flagsA5 = (m->flagsA5 & ~2) | 5;
	initMstTaskData(m);
	// TODO
}

void Game::executeMstOp67Type2(Task *t, int flag) {
	t->flags &= ~0x80;
	MstTaskData *m = t->dataPtr;
	m->flagsA5 = (m->flagsA5 & ~1) | 6;
	initMstTaskData(m);
	// TODO
}

void Game::executeMstOp67(Task *t, int x1, int x2, int y1, int y2, int screen, int arg10, int o_flags1, int o_flags2, int arg1C, int arg20, int arg24) {
	// warning("executeMstOp67 pos %d,%d,%d,%d %d %d 0x%x 0x%x %d %d %d", y1, x1, y2, x2, screen, arg10, o_flags1, o_flags2, arg1C, arg20, arg24);
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
	int objScreen = (screen < 0) ? _currentScreen : screen;

	LvlObject *o = 0; // _edi
	MstObject *mo = 0; // _ebp
	MstTaskData *m = 0; // _esi

	if (arg1C != -128) {
		if (_mstVars[30] > 32) {
			_mstVars[30] = 32;
		}
		int count = 0;
		for (int i = 0; i < 32; ++i) {
			if (_mstUnkDataTable[i].unk0) {
				++count;
			}
		}
		if (count >= _mstVars[30]) {
			return;
		}
		if (arg1C < 0) { // _edx
			const MstUnk42 *m42 = &_res->_mstUnk42[arg24];
			if (m42->count2 == 0) {
				arg1C = m42->data2[0];
			} else {
				arg1C = m42->data2[_rnd.update() % m42->count2];
			}
		}
// 415518
		for (int i = 0; i < 32; ++i) {
			if (!_mstUnkDataTable[i].unk0) {
				m = &_mstUnkDataTable[i];
				break;
			}
		}
		if (!m) {
			return;
		}
		memset(m->localVars, 0, sizeof(m->localVars));
		m->flags48 = 0x1C;
		m->flagsA5 = 0;
		m->unkEC = -1;
		m->unkD0 = 0;
		m->flagsA6 = 0;

		const int j = _res->_mstUnk42[arg24].indexUnk46[arg1C];
		assert(j >= 0 && j < _res->_mstHdr.unk0x30);
		m->unk0 = &_res->_mstUnk46[j];

		m->unk4 = &_res->_mstUnk46[j].data[arg20];
		MstUnk46Unk1 *m1 = &_res->_mstUnk46[j].data[arg20]; // _ecx
		m->unk8 = _res->_mstHeightMapData + m1->indexHeight * 948;

		m->localVars[7] = m1->unkC;

		if (m1->indexUnk51 == kNone) {
			m->flags48 &= ~4;
		}

		const uint8_t *ptr = m->unk8;
		int anim = m1->anim; // READ_LE_UINT16(_ecx + 4)
		o = addLvlObject(ptr[945], x1, y1, objScreen, ptr[944], anim, o_flags1, o_flags2, 0, 0);
		if (!o) {
			m->unk0 = 0;
			if (m->o16) {
				m->o16->dataPtr = 0;
			}
			for (int j = 0; j < 64; ++j) {
				if (_mstObjectsTable[j].unk0 && _mstObjectsTable[j].mstTaskData == m) {
					_mstObjectsTable[j].mstTaskData = 0;
				}
			}
			return;
		}
// 41562C
		m->o16 = o;
		if (_currentLevel == 7 && m->unk8[944] == 26) {
			m->o20 = addLvlObject(ptr[945], x1, y1, objScreen, ptr[944], m1->anim + 1, o_flags1, 0x3001, 0, 0);
			if (!m->o20) {
				// TODO
				return;
			}
			if (screen < 0) {
				m->o20->xPos += _mstRefPosX;
				m->o20->yPos += _mstRefPosY;
			}
			m->o20->dataPtr = 0;
			setLvlObjectPosRelativeToObject(m->o16, 6, m->o20, 6);
		}
// 4156FC
		m->unkE8 = o_flags2 & 0xFFFF;
		m->unkE6 = _mstLut5[o_flags2 & 0x1F];
		o->dataPtr = m;
	} else {
		for (int i = 0; i < 64; ++i) {
			if (!_mstObjectsTable[i].unk0) {
// 415743
				mo = &_mstObjectsTable[i];
				break;
			}
		}
		if (!mo) {
			return;
		}
		if (!o) {
			warning("executeMstOp67 o is NULL");
			return;
		}
	}
// 4157E8
	if (screen < 0) {
		o->xPos += _mstRefPosX;
		o->yPos += _mstRefPosY;
	}
	setLvlObjectPosInScreenGrid(o, 7);
	if (mo) {
		Task *t = 0;
		for (int i = 0; i < 64; ++i) {
			if (!_tasksTable[i].codeData) {
				t = &_tasksTable[i];
				break;
			}
		}
		if (!t) {
			mo->unk0 = 0;
			if (mo->o) {
				mo->o->dataPtr = 0;
			}
			removeLvlObject2(o);
			return;
		}
// 41584E
		memset(t, 0, sizeof(Task));
		// resetTask(t, 1234);
		t->prevPtr = 0;
		t->nextPtr = _mstTasksList2;
		if (_mstTasksList2) {
			_mstTasksList2->prevPtr = t;
		}
		// TODO
		setMstObjectDefaultPos(t);
		// TODO
		if (_currentLevel == kLvl_fort) {
			// TODO
		}
	} else {
// 41593C
		Task *t = 0;
		for (int i = 0; i < 64; ++i) {
			if (!_tasksTable[i].codeData) {
				t = &_tasksTable[i];
				break;
			}
		}
		if (!t) {
// 415952
			// TODO
			return;
		}
// 415989
		// TODO
		memset(t, 0, sizeof(Task));
		// resetTask(t, 1234);
		t->prevPtr = 0;
		t->nextPtr = _mstTasksList1;
		if (_mstTasksList1) {
			_mstTasksList1->prevPtr = t;
		}
		t->dataPtr = m;
		t->mstObject = 0;
		_mstTasksList1 = t;
		m->task = t;
		// _edi = _currentTask;
		// _currentTask = t;
		Task *child = t->child;
		if (child) {
			child->codeData = 0;
			t->child = 0;
		}
/*
		Task *next = t->nextPtr;
		Task *prev = t->prevPtr;
		t->codeData = 0;
		if (next) {
			next->prevPtr = prev;
		}
		if (prev) {
			prev->nextPtr = next;
		} else {
			_mstTasksList1 = next;
		}
		if (!_mstTasksList1) {
			_mstTasksList1 = t;
			t->nextPtr = 0;
			t->prevPtr = 0;
		} else {
			Task *current = _mstTasksList1; // _eax
			next = current->nextPtr; // _ecx
			while (next) {
				current = next;
				next = current->nextPtr;
			}
			current->nextPtr = t;
			t->nextPtr = 0;
			t->prevPtr = current;
		}
*/
// 415A3C
		// t->codeData = 1234;
		shuffleDword(m->unkCC);
		shuffleDword(m->unkC8);

		m->x2 = -1;
		m->unkC = _res->_mstUnk44[m->unk4->indexUnk44].data;

		if (m->unk8[946] & 4) {
			m->flagsA8 = 0xFF;
			m->flags49 = 0xFF;
		}

// 415A89
		setMstTaskDataDefaultPos(t);

		switch (arg10) {
		case 1:
		case 2:
			warning("executeMstOp67 unhandled type %d", arg10);
			_mstTasksList1 = _mstTasksList1->nextPtr; // TEMP
			break;
/*
		case 1:
			executeMstOp67Type1(t);
			break;
		case 2:
			if (m) {
				m->flagsA6 |= 1;
			}
			executeMstOp67Type2(t, 0);
			break;
*/
		default:
			m->flagsA5 = 1;
			if (updateMstTaskDataPosition(m) == 0) {
				initMstTaskData(m);
			}
			prepareMstTask(t);
			break;
		}
	}
// 415ADE
	// TODO
	t->flags &= ~0x80;
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
	return 1;
}

int Game::runTask_waitFlags(Task *t) {
	switch (t->delay) {
	case 1:
		if (t->tempVar == 0) {
			return 1;
		}
		break;
	case 2:
		if ((t->flags & (1 << t->tempVar)) == 0) {
			return 1;
		}
		break;
	case 3:
		if ((_mstFlags & (1 << t->tempVar)) == 0) {
			return 1;
		}
		break;
	case 4:
		if (getTaskAndyVar(t->tempVar, t) == 0) {
			return 1;
		}
		break;
	case 5: {
			MstTaskData *m = 0;
			if (t->mstObject) {
				m = t->mstObject->mstTaskData;
			} else {
				m = t->dataPtr;
			}
			if (!m || (m->flags48 & (1 << t->tempVar)) == 0) {
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

int Game::runTask_mstUnk55_233(Task *t) {
	const MstUnk55 *m = &_res->_mstUnk55[t->tempVar];
	const int a = getTaskVar(t, m->indexVar1, m->maskVars & 15);
	const int b = getTaskVar(t, m->indexVar2, m->maskVars >> 4);
	if (!compareOp(m->compare, a, b)) {
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
	return 1;
}

int Game::runTask_mstUnk55_234(Task *t) {
	const MstUnk55 *m = &_res->_mstUnk55[t->tempVar];
	const int a = getTaskVar(t, m->indexVar1, m->maskVars & 15);
	const int b = getTaskVar(t, m->indexVar2, m->maskVars >> 4);
	if (compareOp(m->compare, a, b)) {
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
	return 1;
}
