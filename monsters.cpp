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

static uint8_t getBits23(uint8_t a) {
	uint8_t r = 0;
	if (a & 8) {
		r |= 1;
	}
	if (a & 4) {
		r |= 2;
	}
	return r;
}

static LvlObject *getTaskLvlObject(Task *t) {
	if (t->dataPtr) {
		return t->dataPtr->o16;
	} else if (t->mstObject) {
		return t->mstObject->o;
	}
	return 0;
}

void Game::resetMstTaskData(MstTaskData *m) {
	m->m46 = 0;
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

void Game::mstTaskSetScreenPosition(Task *t) {
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

	m->x1 = m->unkC->unk2C[num] - READ_LE_UINT32(ptr + 904); // x2 ?
	m->x2 = m->unkC->unk34[num] + READ_LE_UINT32(ptr + 904); // x1 ?
	m->y1 = m->unkC->unk3C[num] - READ_LE_UINT32(ptr + 908); // y2 ?
	m->y2 = m->unkC->unk44[num] + READ_LE_UINT32(ptr + 908); // y1 ?

	const uint32_t indexUnk35 = m->unkC->indexUnk35_0x24[num];
	m->m35 = (indexUnk35 == kNone) ? 0 : &_res->_mstUnk35[indexUnk35];
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
		const uint8_t code = unk4->unk1B;
		if (code != 255) {
			assert(code < 32);
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

void Game::copyMstTaskData(Task *t, MstTaskData *m, int num) {
	MstUnk46Unk1 *m46unk1 = &m->m46->data[num];
	m->unk4 = m46unk1;
	m->unk8 = _res->_mstHeightMapData + m46unk1->indexHeight * 948;
	if (m46unk1->indexUnk51 == kNone) {
		m->flags48 &= ~4;
	}
	m->unkC = _res->_mstUnk44[m46unk1->indexUnk44].data;
	mstTaskUpdateScreenPosition(t);
	if (!updateMstTaskDataPosition(m)) {
		initMstTaskData(m);
	}
	if (t->run == &Game::runTask_unk4) {
		t->run = &Game::runTask_default;
	}
	if ((m->flagsA5 & 8) == 0 && t->run == &Game::runTask_idle) {
		prepareMstTask(t);
	}
}

void Game::initMstTaskDataType2(Task *t) {
	warning("initMstTaskDataType2 %p unimplemented", t);
	// TODO

	if (_mstUnk6 == -1) {
		return;
	}
}

void Game::updateMstLvlObjectPos(MstTaskData *m) {
	LvlObject *o = m->o16;
	o->xPos = m->xPos - o->posTable[7].x;
	o->yPos = m->yPos - o->posTable[7].y;
	m->xMstPos = _res->_mstPointOffsets[o->screenNum].xOffset;
	m->yMstPos = _res->_mstPointOffsets[o->screenNum].yOffset;
}

bool Game::updateMstTaskDataPositionHelper(MstTaskData *m) {
	MstUnk46Unk1 *m46unk1 = m->unk4;
	const uint32_t indexUnk44 = m46unk1->indexUnk44;
	assert(indexUnk44 != kNone);
	MstUnk44 *m44 = &_res->_mstUnk44[indexUnk44];
	MstUnk44Unk1 *m44unk1 = m44->data;

	int x = m->xMstPos; // _ebp
	int y = m->yMstPos; // _edx

	if (m->x2 >= 0) {
		if (x < m->x2) {
			x = m->x2;
		} else if (x > m->x1) {
			x = m->x1;
		}
		if (y < m->y2) {
			y = m->y2;
		} else if (y > m->y1) {
			y = m->y1;
		}
	}
// 41E659

	uint32_t indexUnk34 = m44->data[0].indexUnk34_16;
	assert(indexUnk34 != kNone);
	MstUnk34 *m34 = &_res->_mstUnk34[indexUnk34]; // _esi
	int _ecx = (m34->x1 - m34->x2) / 2 + m34->x2;

	int _edi = 0x1000000;
	int var1C = y;

	uint32_t i = 0;

	for (i = 0; i < m44->count; ++i) {
		uint32_t indexUnk34 = m44->data[i].indexUnk34_16;
		assert(indexUnk34 != kNone);
		MstUnk34 *m34 = &_res->_mstUnk34[indexUnk34]; // _esi
		if (m34->x1 < x || m34->x2 > x || m34->y1 < y || m34->y2 > y) {
			const int d1 = ABS(x - m34->x2);
			if (d1 < _edi) {
				_edi = d1;
				m44unk1 = &m44->data[i];
				_ecx = m34->x2;
			}
			const int d2 = ABS(x - m34->x1);
			if (d2 < _edi) {
				_edi = d2;
				m44unk1 = &m44->data[i];
				_ecx = m34->x1;
			}
		} else {
// 41E6FD
			_ecx = x;
			var1C = y;
			m44unk1 = &m44->data[i];
			break;
		}
	}
// 41E70B
	int _edx = var1C;
	if (i == m44->count) {
		uint32_t indexUnk34 = m44unk1->indexUnk34_16;
		assert(indexUnk34 != kNone);
		MstUnk34 *m34 = &_res->_mstUnk34[indexUnk34]; // _esi
		if (y <= m34->y1) {
			y = (m34->y1 - m34->y2) / 2 + m34->y2;
		}
		_edx = var1C = y;
	}

// 41E737
	// find screenNum for level position
	int screenNum = -1;
	int xMst;
	int yMst;
	for (int i = 0; i < _res->_mstHdr.pointsCount; ++i) {
		xMst = _res->_mstPointOffsets[i].xOffset;
		if (xMst > _ecx || xMst + 255 < _ecx) {
			continue;
		}
		yMst = _res->_mstPointOffsets[i].yOffset;
		if (yMst > _edx || yMst + 191 < _edx) {
			continue;
		}
		screenNum = i;
		break;
	}
	if (screenNum == -1) {
		screenNum = 0;
		xMst = _res->_mstPointOffsets[0].xOffset + 256 / 2;
		yMst = _res->_mstPointOffsets[0].yOffset + 192 / 2;
	}
	LvlObject *o = m->o16;
	o->screenNum = screenNum;
	m->xPos = _ecx - xMst;
	m->yPos = _edx - yMst;
	updateMstLvlObjectPos(m);
	m->unkC = m44unk1;
	initMstTaskData(m);
	return true;
}

bool Game::updateMstTaskDataPosition(MstTaskData *m) {
	const uint8_t screenNum = m->o16->screenNum;
	MstUnk46Unk1 *m46 = m->unk4;
	const uint32_t indexUnk44 = m46->indexUnk44;
	assert(indexUnk44 != kNone);
	MstUnk44 *m44 = &_res->_mstUnk44[indexUnk44];
	// start from screen number
	uint32_t indexUnk44Unk1 = m44->indexUnk44Unk1[screenNum];
	if (indexUnk44Unk1 != kNone) {
		MstUnk44Unk1 *m44unk1 = &m44->data[indexUnk44Unk1];
		uint32_t indexUnk34 = m44unk1->indexUnk34_16;
		assert(indexUnk34 != kNone);
		MstUnk34 *m34 = &_res->_mstUnk34[indexUnk34];
		while (m34->x2 <= m->xMstPos) {
			if (m34->x1 >= m->xMstPos && m34->y2 <= m->yMstPos && m34->y1 >= m->yMstPos) {
				if (m->unkC == m44unk1) {
					return false;
				}
				m->unkC = m44unk1;
				initMstTaskData(m);
				return true;
			}

			indexUnk44Unk1 = m44unk1->indexUnk44_92;
			if (indexUnk44Unk1 == kNone) {
				break;
			}
			m44unk1 = &m44->data[indexUnk44Unk1];
			indexUnk34 = m44unk1->indexUnk34_16;
			assert(indexUnk34 != kNone);
			m34 = &_res->_mstUnk34[indexUnk34];
		}
	}
	return updateMstTaskDataPositionHelper(m);
}

int Game::prepareMstTask(Task *t) {
	MstTaskData *m = t->dataPtr;
	assert(m);
	MstUnk35 *mu = m->m35;
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

void Game::clearMstRectsTable(MstTaskData *m, int num) {
	int r = m->flagsA8[num];
	if (r < _mstRectsCount) {
		MstRect *p = &_mstRectsTable[r];
		int a = p->num;
		if (a == m->soundType) {
			p->num = 255;
			a = r;
		}
		do {
			if (p->num == 255) {
				++a;
				++p;
			}
		} while (a < _mstRectsCount);
		if (a == _mstRectsCount) {
			_mstRectsCount = r;
		}
	}
	m->flagsA8[num] = 255;
}

int Game::resetMstRectsTable(int num, int x1, int y1, int x2, int y2) {
	for (int i = 0; i < _mstRectsCount; ++i) {
		const MstRect *p = &_mstRectsTable[i];
		if (p->num != 255 && num != p->num) {
			if (p->x2 < x1) { // 16 - 8
				continue;
			}
			if (p->x1 > x2) { // 16 - 16
				continue;
			}
			if (p->y2 < y1) { // 16 - 4
				continue;
			}
			if (p->x1 > y2) { // 16 - 12
				continue;
			}
			return i + 1;
		}
	}
	return 0;
}

int Game::updateMstRectsTable(int num, int a, int x1, int y1, int x2, int y2) {
	if (num == 255) {
		MstRect *p = &_mstRectsTable[0];
		for (int i = 0; i < _mstRectsCount; ++i) {
			if (p->num == 255) {
				num = i;
				break;
			}
		}
		p->x1 = x1;
		p->y1 = y1;
		p->x2 = x2;
		p->y2 = y2;
		p->num = a;
		if (num != _mstRectsCount) {
			++_mstRectsCount;
		}
	} else if (num < _mstRectsCount) {
		MstRect *p = &_mstRectsTable[num];
		if (p->num == a) {
			p->x1 = x1;
			p->y1 = y1;
			p->x2 = x2;
			p->y2 = y2;
		}
	}
	return num;
}

int Game::checkMstRectsTable(int num, int x1, int y1, int x2, int y2) {
	for (int i = 0; i < _mstRectsCount; ++i) {
		MstRect *p = &_mstRectsTable[i];
		if (p->num == 255 || p->num == num) {
			continue;
		}
		if (p->num == 254) {
			if (_mstUnkDataTable[num].unk8[944] != 15) {
				continue;
			}
		}
		if (p->x2 < x1) {
			continue;
		}
		if (p->x1 > x2) {
			continue;
		}
		if (p->y2 < y1) {
			continue;
		}
		if (p->y1 > y2) {
			continue;
		}
		return i + 1;
	}
	return 0;
}

int Game::getMstDistance(int y, MovingOpcodeState *p) {
	switch (p->unk24) {
	case 0:
		if (p->boundingBox.x1 <= _mstTemp_x2 && y >= _mstTemp_y1 && y - _mstTemp_y2 <= 9) {
			const int dx = _mstTemp_x1 - p->boundingBox.x2;
			if (dx <= 0) {
				return 0;
			}
			return dx / p->unk1C;
		}
		break;
//	case 1:
//	case 2:
		// TODO
	case 3: {
			const int dx = _mstTemp_x2 - p->boundingBox.x1;
			if (dx >= 0) {
				const int dy = p->boundingBox.y1 - dx * 2 / 3;
				if (dy <= _mstTemp_y2) {
					const int dx2 = _mstTemp_x1 - p->boundingBox.x2;
					if (dx2 <= 0) {
						return (p->boundingBox.y2 >= _mstTemp_y1) ? 0 : -1;
					} else {
						const int dy2 =  p->boundingBox.y2 - dx2 * 2 / 3;
						if (dy2 >= _mstTemp_y1) {
							if (p->unk40 == 3) {
								return 0;
							}
							return dx2 / p->unk1C;
						}
					}
				}
			}
		}
		break;
//	case 4:
//	case 5:
		// TODO
	case 6:
		if (p->boundingBox.y2 >= _mstTemp_y1 && p->boundingBox.x2 >= _mstTemp_x1 && p->boundingBox.x1 <= _mstTemp_x2) {
			const int dy = p->boundingBox.y1 - _mstTemp_y2;
			if (dy <= 0 || p->unk40 == 3) {
				return 0;
			}
			return dy / p->unk20;
		}
		break;
	case 7:
	case 128: // 8
	case 133: // 9
		warning("getMstDistance unk24 %d y %d unimplemented", p->unk24, y);
		break;
	}
	return -1;
}

void Game::mstTaskUpdateScreenPosition(Task *t) {
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
		const uint8_t *ptr1 = ptr + (o->flags0 & 255) * 28; // _eax
		if (ptr1[14] != 0) {
			_mstTemp_x1 = m->xMstPos + (int8_t)ptr1[12];
			_mstTemp_y1 = m->yMstPos + (int8_t)ptr1[13];
			_mstTemp_x2 = _mstTemp_x1 + ptr1[14];
			_mstTemp_y2 = _mstTemp_y1 + ptr1[15];
			m->flagsA8[0] = updateMstRectsTable(m->flagsA8[0], m->soundType, _mstTemp_x1, _mstTemp_y1, _mstTemp_x2, _mstTemp_y2);
		} else {
			clearMstRectsTable(m, 0);
		}
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
	if (_mstMovingStateCount != 0 && !_specialAnimFlag && (o->flags1 & 6) != 6) {
		if (m->localVars[7] > 0 || m->localVars[7] < -1) {
			if ((m->flagsA5 & 0x80) == 0) {
				for (int i = 0; i < _mstMovingStateCount; ++i) {
					MovingOpcodeState *p = &_mstMovingState[i];
					if (m->xDelta > 256 || m->yDelta > 192) {
						continue;
					}
					_mstTemp_x1 = o->xPos;
					_mstTemp_y1 = o->yPos;
					_mstTemp_x2 = o->xPos + o->width - 1;
					_mstTemp_y2 = o->yPos + o->height - 1;
					uint8_t _al = p->unk40;
					if (_al == 1 || _al == 2) {
// 40EED8
						if (p->unk3C >= m->xDelta + m->yDelta) {
							if (o->screenNum != _currentScreen) {
								const int dx = _res->_mstPointOffsets[o->screenNum].xOffset - _res->_mstPointOffsets[_currentScreen].xOffset;
								const int dy = _res->_mstPointOffsets[o->screenNum].yOffset - _res->_mstPointOffsets[_currentScreen].yOffset;
								_mstTemp_x1 += dx;
								_mstTemp_x2 += dx;
								_mstTemp_y1 += dy;
								_mstTemp_y2 += dy;
							}
// 40EF72
							if (_mstTemp_x2 >= 0 && _mstTemp_x1 <= 255 && _mstTemp_y2 >= 0 && _mstTemp_y1 <= 191) {
								const uint8_t type = m->unk8[944];
								if (((type & 9) == 0 && clipLvlObjectsSmall(p->o, o, 132)) || ((type & 9) != 0 && clipLvlObjectsSmall(p->o, o, 20))) {
									p->m = m;
									p->unk3C = m->xDelta + m->yDelta;
									p->unk34 = _clipBoxOffsetX;
									p->unk38 = _clipBoxOffsetY;
								}
							}
// 40F009
							if (o->screenNum != _currentScreen) {
								const int dx = _res->_mstPointOffsets[_currentScreen].xOffset - _res->_mstPointOffsets[o->screenNum].xOffset;
								const int dy = _res->_mstPointOffsets[_currentScreen].yOffset - _res->_mstPointOffsets[o->screenNum].yOffset;
								_mstTemp_x1 += dx;
								_mstTemp_x2 += dx;
								_mstTemp_y1 += dy;
								_mstTemp_y2 += dy;
							}
						}
// 40F087
					} else if (_al == 3 && p->unk3C > m->xDelta + m->yDelta) {
						if (o->screenNum != _currentScreen) {
							const int dx = _res->_mstPointOffsets[o->screenNum].xOffset - _res->_mstPointOffsets[_currentScreen].xOffset;
							const int dy = _res->_mstPointOffsets[o->screenNum].yOffset - _res->_mstPointOffsets[_currentScreen].yOffset;
							_mstTemp_x1 += dx;
							_mstTemp_x2 += dx;
							_mstTemp_y1 += dy;
							_mstTemp_y2 += dy;
						}
// 40EE68
						if (_mstTemp_x2 >= 0 && _mstTemp_x1 <= 255 && _mstTemp_y2 >= 0 && _mstTemp_y1 <= 191) {
							if (testPlasmaCannonPointsDirection(_mstTemp_x1, _mstTemp_y1, _mstTemp_x2, _mstTemp_y2)) {
								p->unk3C = m->xDelta + m->yDelta;
								p->m = m;
								p->unk41 = _plasmaCannonLastIndex1;
							}
// 40F004
						}
// 40F009
						const int dx = _res->_mstPointOffsets[_currentScreen].xOffset - _res->_mstPointOffsets[o->screenNum].xOffset;
						const int dy = _res->_mstPointOffsets[_currentScreen].yOffset - _res->_mstPointOffsets[o->screenNum].yOffset;
						_mstTemp_x1 += dx;
						_mstTemp_x2 += dx;
						_mstTemp_y1 += dy;
						_mstTemp_y2 += dy;
					}
// 40F087
// 40F08C
					_mstTemp_x1 += _res->_mstPointOffsets[o->screenNum].xOffset;
					_mstTemp_y1 += _res->_mstPointOffsets[o->screenNum].yOffset;
					_mstTemp_x2 += _res->_mstPointOffsets[o->screenNum].xOffset;
					_mstTemp_y2 += _res->_mstPointOffsets[o->screenNum].yOffset;
					int res = getMstDistance((m->unk8[946] & 2) != 0 ? p->boundingBox.y2 : m->yMstPos, p);
					if (res < 0) {
						continue;
					}
					if (m->unkEC == -1 || m->unkEC > res || (m->unkEC == 0 && res == 0 && (m->unk1C->unk40 & 1) == 0 && p->unk40 == 2)) {
						m->unk1C = p;
						m->unkEC = res;
					}
				}
			}
		}
	}
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
	if (_mstMovingState[0].unk40 == 3 && m->unkF4 != 0 && _mstMovingState[0].unk24 == m->unkFC && m->directionKeyMask == _andyObject->directionKeyMask) {
		m->flags48 |= 0x80;
	} else {
		m->unkFC = -1;
		m->flags48 &= ~0x80;
	}
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
			uint8_t *ptr = _res->_mstUnk49[i].data2;
			for (int j = 0; j < count * 2; ++j) {
				const int index1 = _rnd.update() % count;
				const int index2 = _rnd.update() % count;
				SWAP(ptr[index1], ptr[index2]);
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
	_executeMstLogicPrevCounter = _executeMstLogicCounter = 0;
	// TODO
	_specialAnimFlag = false;
	_mstUnk10 = 255;
	_mstRectsCount = 0;
	_mstOp67_y1 = 0;
	_mstOp67_y2 = 0;
	_mstOp67_screenNum = 0xFF;
	_mstOp68_x1 = 256;
	_mstOp68_x2 = 256;
	_mstOp68_y1 = 0;
	_mstOp68_y2 = 0;
	_mstOp68_screenNum = 255;
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
	mstUpdateRefPos();
	_mstPrevPosX = _mstPosX;
	_mstPrevPosY = _mstPosY;
	for (int i = 0; i < _res->_mstHdr.unk0x3C; ++i) {
		// TODO
	}
}

void Game::startMstCode() {
	mstUpdateRefPos();
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
	for (int i = 0; i < kMaxMovingStates; ++i) {
		_mstMovingState[i].unk28 = 0;
		_mstMovingState[i].m = 0;
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
// 419C30
	for (int i = 0; i < _mstMovingStateCount; ++i) {
		MovingOpcodeState *p = &_mstMovingState[i];
		MstTaskData *m = p->m;
		if (!m) {
			continue;
		}
		const int energy = m->localVars[7];
		if (p->unk28) {
			warning("executeMstCode m->unk28 %p unimplemented", p->unk28);
			// TODO
			continue;
		}
// 419CC0
		if (energy > 0) {
			m->localVars[7] = energy - 1;
			_plasmaCannonLastIndex1 = p->unk41;
		} else if (energy == -2) {
			_plasmaCannonLastIndex1 = p->unk41;
		} else {
			_plasmaCannonLastIndex1 = 0;
		}
	}
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
	mstUpdateRefPos();
	updateMstHeightMapData();
	for (Task *t = _mstTasksList1; t; t = t->nextPtr) {
		mstTaskUpdateScreenPosition(t);
	}
}

void Game::mstLvlObjectSetActionDirection(LvlObject *o, const uint8_t *ptr, uint8_t mask1, uint8_t mask2) {
	o->actionKeyMask = ptr[1];
	uint8_t _al = mask1 & 15;
	o->directionKeyMask = _al;
	MstTaskData *m = (MstTaskData *)getLvlObjectDataPtr(o, kObjectDataTypeMonster);
	if ((mask1 & 0x10) == 0) {
		const int _edi = mask1 & 0xE0;
		switch (_edi) {
		case 32:
		case 96:
		case 160:
		case 192:
			if (_edi == 192) {
				o->directionKeyMask = _al | (m->flags49 & ~5);
			} else {
				uint8_t _bl = m->flags49 | _al;
				o->directionKeyMask = _bl;
				if ((m->unk8[946] & 2) != 0) {
					if (_edi == 160 && (_mstLut1[_al] & 1) != 0) {
						if (m->xDelta >= m->yDelta) {
							o->directionKeyMask = _bl & ~5;
						} else {
							o->directionKeyMask = _bl & ~0xA;
						}
					} else {
						if (m->xDelta >= m->yDelta * 2) {
							o->directionKeyMask = _bl & ~5;
						} else if (m->yDelta >= m->xDelta * 2) {
							o->directionKeyMask = _bl & ~0xA;
						}
					}
				}
			}
			break;
		case 128:
			_al |= mask2;
			o->directionKeyMask = _al;
			if ((m->unk8[946] & 2) != 0 && (_mstLut1[_al] & 1) != 0) {
				if (m->xDelta >= m->yDelta) {
					o->directionKeyMask = _al & ~5;
				} else {
					o->directionKeyMask = _al & ~0xA;
				}
			}
			break;
		case 224:
			o->directionKeyMask = m->unkF8 | _al;
			break;
		default:
			o->directionKeyMask = _al | mask2;
			break;
		}
	}
// 40E3B3
	o->directionKeyMask &= ptr[2];
	if ((mask1 & 0xE0) == 0x40) {
		o->directionKeyMask ^= 0xA;
	}
}

bool Game::executeMstUnk17(MstTaskData *m, int num) {
	LvlObject *o = m->o16;
	uint8_t _al = _res->_mstUnk52[num * 4];
	uint8_t _bl = _res->_mstUnk52[num * 4 + 2];
	const uint8_t *var4 = m->unk8 + _al * 28;
	uint8_t _dl = (o->flags1 >> 4) & 3;
	uint8_t _ecx = ((_dl & 1) != 0 ? 6 : 0) + 2;
	uint8_t var8 = _ecx;
	if (_dl & 2) {
		var8 |= 4;
	} else {
		var8 |= 1;
	}
	_ecx = _bl & 15;
	if ((_bl & 0x10) == 0) {
		uint32_t _ebp = _bl & 0xE0;
		switch (_ebp) {
		case 32:
		case 96:
		case 160:
		case 192: // 0
			if (_ebp == 192) {
				_ecx |= m->flags49 & ~5;
			} else {
				_ecx |= m->flags49;
				if (m->unk8[946] & 2) {
					if (_ebp == 160 && _mstLut1[_ecx] != 1) {
						if (m->xDelta >= m->yDelta) {
							_ecx &= ~5;
						} else {
							_ecx &= ~0xA;
						}
					} else {
						if (m->xDelta >= 2 * m->yDelta) {
							_ecx &= ~5;
						} else if (m->yDelta >= 2 * m->xDelta) {
							_ecx &= ~0xA;
						}
					}
				}
			}
			break;
		case 128: // 1
			_ecx |= var8;
			if ((m->unk8[946] & 2) != 0 && _mstLut1[_ecx] != 1) {
				if (m->xDelta >= m->yDelta) {
					_ecx &= ~5;
				} else {
					_ecx &= ~0xA;
				}
			}
			break;
		default: // 2
			_ecx |= var8;
			break;
		}
	}
// 40E5C0
	_bl &= 0xE0;
	_ecx &= var4[2];
	if (_bl == 0x40) {
		_ecx ^= 0xA;
	}
	return (var8 & _ecx) != 0 ? 0 : 1;
}

bool Game::executeMstUnk19(LvlObject *o, int type) {
	warning("executeMstUnk19 unimplemented");
	return false;
}

bool Game::executeMstUnk21(LvlObject *o, int type) {
	int _esi, _edi, _ebx, _ebp;
	if (type != 1 && type != 0x1000) {
		_esi = o->xPos;
		_edi = o->yPos;
		_ebx = _esi + o->width  - 1;
		_ebp = _edi + o->height - 1;
	} else {
		_esi = o->xPos + o->posTable[0].x;
		_ebx = o->xPos + o->posTable[1].x;
		_edi = o->yPos + o->posTable[0].y;
		_ebp = o->yPos + o->posTable[1].y;
		if (_esi > _ebx) {
			SWAP(_esi, _ebx);
		}
		if (_edi > _ebp) {
			SWAP(_edi, _ebp);
		}
		if (type == 0x1000 && _andyObject->screenNum != o->screenNum) {
			const int dx = _res->_mstPointOffsets[_andyObject->screenNum].xOffset - _res->_mstPointOffsets[o->screenNum].xOffset;
			_esi += dx;
			_ebx += dx;
			const int dy = _res->_mstPointOffsets[_andyObject->screenNum].yOffset - _res->_mstPointOffsets[o->screenNum].yOffset;
			_edi += dy;
			_ebp += dy;
		}
	}
// 417D2B
	return (_andyObject->xPos + _andyObject->width - 1 >= _esi && _andyObject->xPos <= _ebx && _andyObject->yPos + _andyObject->height - 1 >= _edi && _andyObject->yPos <= _ebp);
}

bool Game::executeMstUnk22(LvlObject *o, int type) {
	int x1, y1, x2, y2;
	if (type != 1) {
		x1 = o->xPos; // _eax
		y1 = o->yPos; // _edx
		x2 = o->xPos + o->width  - 1; // _esi
		y2 = o->yPos + o->height - 1; // _ecx
	} else {
		x1 = o->xPos + o->posTable[0].x; // _eax
		y1 = o->yPos + o->posTable[0].y; // _edx
		x2 = o->xPos + o->posTable[1].x; // _esi
		y2 = o->yPos + o->posTable[1].y; // _ecx
		if (x1 > x2) {
			SWAP(x1, x2);
		}
		if (y1 > y2) {
			SWAP(y1, y2);
		}
		const int xPos = _andyObject->xPos + _andyObject->posTable[3].x;
		const int yPos = _andyObject->yPos + _andyObject->posTable[3].y;
		if (xPos >= x1 && xPos <= x2 && yPos >= y1 && yPos <= y2) {
			return true;
		}
	}
	return false;
}

bool Game::executeMstUnk28(LvlObject *o, int type) const {
	int x1, y1, x2, y2;
	if (type != 1) {
		x1 = o->xPos; // _edx
		y1 = o->yPos; // _edi
		x2 = o->xPos + o->width  - 1; // _ebx
		y2 = o->yPos + o->height - 1;
	} else {
		x1 = o->xPos + o->posTable[0].x; // _edx
		y1 = o->yPos + o->posTable[0].y; // _edi
		x2 = o->xPos + o->posTable[1].x; // _ebx
		y2 = o->yPos + o->posTable[1].y;
		if (x1 > x2) {
			SWAP(x1, x2);
		}
		if (y1 > y2) {
			SWAP(y1, y2);
		}
		static const uint8_t indexes[] = { 1, 2, 4, 5 };
		for (int i = 0; i < 4; ++i) {
			const int xPos = _andyObject->xPos + _andyObject->posTable[indexes[i]].x;
			const int yPos = _andyObject->yPos + _andyObject->posTable[indexes[i]].y;
			if (xPos >= x1 && xPos <= x2 && yPos >= y1 && yPos <= y2) {
				return true;
			}
		}
	}
	return false;
}

bool Game::executeMstUnk20(MstTaskData *m, uint32_t flags) {
	if ((flags & 1) != 0 && (m->o16->flags0 & 0x200) == 0) {
		return false;
	} else if ((flags & 8) != 0 && (m->flags48 & 0x20) == 0) {
		return false;
	} else if ((flags & 0x100) != 0 && (_mstFlags & 0x80000000) != 0) {
		return false;
	} else if ((flags & 0x200) != 0 && (_mstFlags & 0x40000000) != 0) {
		return false;
	} else if ((flags & 0x40) != 0) {
		warning("executeMstUnk20 flags 0x%x", flags);
		// TODO
	} else if ((flags & 0x80) != 0) {
		warning("executeMstUnk20 flags 0x%x", flags);
		// TODO
	} else if ((flags & 0x400) != 0 && (m->o16->screenNum != _andyObject->screenNum || !executeMstUnk19(m->o16, 0))) {
		return false;
	} else if ((flags & 0x800) != 0 && (m->o16->screenNum != _andyObject->screenNum || !executeMstUnk19(m->o16, 1))) {
		return false;
	} else if ((flags & 0x100000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !executeMstUnk22(m->o16, 0))) {
		return false;
	} else if ((flags & 0x200000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !executeMstUnk22(m->o16, 1))) {
		return false;
	} else if ((flags & 4) != 0 && (m->o16->screenNum != _andyObject->screenNum || !executeMstUnk21(m->o16, 0))) {
		return false;
	} else if ((flags & 2) != 0 && (m->o16->screenNum != _andyObject->screenNum || !executeMstUnk21(m->o16, 1))) {
		return false;
	} else if ((flags & 0x4000) != 0 && !executeMstUnk19(m->o16, 0x4000)) {
		return false;
	} else if ((flags & 0x1000) != 0 && !executeMstUnk21(m->o16, 0x1000)) {
		return false;
	} else if ((flags & 0x20) != 0 && (m->o16->flags0 & 0x100) == 0) {
		return false;
	} else if ((flags & 0x10000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !executeMstUnk28(m->o16, 1))) {
		return false;
	} else if ((flags & 0x20000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !executeMstUnk28(m->o16, 0))) {
		return false;
	} else if ((flags & 0x40000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !clipLvlObjectsBoundingBox(m->o16, _andyObject, 36))) {
		return false;
	} else if ((flags & 0x80000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !clipLvlObjectsBoundingBox(m->o16, _andyObject, 20))) {
		return false;
	}
	return true;
}

bool Game::executeMstUnk27(MstTaskData *m, const uint8_t *p) {
	const uint32_t a = READ_LE_UINT32(p + 0x10);
	if (a == 0 || !executeMstUnk20(m, a)) {
		return false;
	}
	if ((a & 0x8000) != 0 && (m->flagsA6 & 4) == 0) {
		Task t;
		memcpy(&t, _mstCurrentTask, sizeof(Task));
		t.child = 0;
		const uint32_t codeData = READ_LE_UINT32(p + 0x18);
		assert(codeData != kNone);
		t.codeData = _res->_mstCodeData + codeData * 4;
		t.run = &Game::runTask_default;
		t.dataPtr->flagsA6 |= 4;
		runTask_default(_mstCurrentTask);
		t.dataPtr->flagsA6 &= ~4;
		t.nextPtr = _mstCurrentTask->nextPtr;
		t.prevPtr = _mstCurrentTask->prevPtr;
		memcpy(_mstCurrentTask, &t, sizeof(Task));
		_mstCurrentTask->run = &Game::runTask_idle;
		if ((_mstCurrentTask->dataPtr->flagsA6 & 2) == 0) {
			_mstCurrentTask->run = &Game::runTask_default;
		}
		return false;
	} else {
		resetMstTask(_mstCurrentTask, READ_LE_UINT32(p + 0x18), 0x10);
		return true;
	}
}

int Game::executeMstCodeHelper3(Task *t) {
	_mstCurrentTask = t;
	MstTaskData *m = t->dataPtr;
	MstTaskData *_mstCurrentTaskData = m;
	LvlObject *o = m->o16;
	int _mstCurrentFlags0 = o->flags0 & 255;
	const uint8_t *_mstCurrentDataPtr = m->unk8 + _mstCurrentFlags0 * 28; // _ebx
	int8_t a = _mstCurrentDataPtr[6];
	if (a != 0) {
		const int num = CLIP(m->unkE6 + a, 0, 17);
		o->flags2 = (o->flags2 & ~0x1F) | _mstLut4[num];
	} else {
		o->flags2 = m->unkE8;
	}
	if (_mstCurrentFlags0 == 31) {
		stopMstTaskData(_mstCurrentTask, &_mstTasksList1);
		return 1;
	}
	const uint32_t _edi = READ_LE_UINT32(_mstCurrentDataPtr + 20);
	if (_edi != kNone) {
		MstUnk46 *m46 = _mstCurrentTaskData->m46;
		for (uint32_t i = 0; i < m46->count; ++i) {
			if (m46->data[i].indexHeight == _edi) {
				copyMstTaskData(_mstCurrentTask, _mstCurrentTaskData, i);
				return 0;
			}
		}
	}
// 41824B
	if ((m->flagsA5 & 0x80) != 0) {
		return 0;
	}
	if (m->localVars[7] == 0 && !_specialAnimFlag) {
		m->flagsA5 |= 0x80;
		if (m->unk8[946] & 4) {
			clearMstRectsTable(m, 1);
		}
		if (t->child) {
			t->child->codeData = 0;
			t->child = 0;
		}
		if ((m->flagsA5 & 8) != 0 && m->unk18 && _mstUnk6 != -1) {
			initMstTaskDataType2(_mstCurrentTask);
			return 0;
		}
		const uint32_t codeData = m->unk4->codeData;
		if (codeData != kNone) {
			resetTask(t, _res->_mstCodeData + codeData * 4);
			return 0;
		}
		o->actionKeyMask = 7;
		o->directionKeyMask = 0;
		t->run = &Game::runTask_idle;
		return 0;
	}
// 41833A
	if (t->run == &Game::runTask_unk4) {
		return 0;
	}
	if (_mstCurrentDataPtr[0] != 0) {
		executeMstUnk27(_mstCurrentTaskData, _mstCurrentDataPtr);
		return 0;
	}
	if ((m->flagsA5 & 0x40) != 0) {
		return 0;
	}
// 41831
	if (_mstMovingStateCount > 0) {
		// TODO
	}
// 41882E
	if (o->screenNum == _currentScreen && (m->flagsA5 & 0x20) == 0 && (m->flags48 & 0x10) != 0) {
		MstUnk46Unk1 *m46 = m->unk4;
		if (m46->indexUnk47 != kNone) {
			MstUnk47 *m47 = &_res->_mstUnk47[m46->indexUnk47];
			if (m47->count > 0) {
				const uint8_t dir = (o->flags1 >> 4) & 3;
				const uint8_t *p = m47->data;
				for (uint32_t i = 0; i < m47->count; ++i) {
					int32_t a = READ_LE_UINT32(p);
					int32_t b = READ_LE_UINT32(p + 4);
					int32_t c = READ_LE_UINT32(p + 8);
					int32_t d = READ_LE_UINT32(p + 12);
					int x1, x2, y1, y2;
					switch (dir) {
					case 1:
						x1 = m->xMstPos - c; // _edx
						x2 = m->xMstPos - a; // _edi
						y1 = m->yMstPos + b; // _esi
						y2 = m->yMstPos + d; // _ebp;
						break;
					case 2:
						x1 = m->xMstPos + a; // _edx
						x2 = m->xMstPos + c; // _edi
						y1 = m->yMstPos - d; // _esi
						y2 = m->yMstPos - b; // _ebp
						break;
					case 3:
						x1 = m->xMstPos - c;
						x2 = m->xMstPos - a;
						y1 = m->yMstPos - d;
						y2 = m->yMstPos - b;
						break;
					default:
						x1 = m->xMstPos + a; // _edx
						x2 = m->xMstPos + c; // _edi
						y1 = m->yMstPos + b; // _esi
						y2 = m->yMstPos + d; // _ebp
						break;
					}

					if (_mstPosX >= x1 /*_edx*/ && _mstPosX <= x2 /*_edi*/ && _mstPosY >= y1 /*_esi*/ && _mstPosY <= y2 /*_ebp*/) {
						resetMstTask(_mstCurrentTask, READ_LE_UINT32(p + 16), 0x20);
						executeMstUnk27(_mstCurrentTaskData, _mstCurrentDataPtr);
						return 0;
					}

					p += 20;
				}
			}
		}
	}
// 418939
	if (executeMstUnk27(_mstCurrentTaskData, _mstCurrentDataPtr)) {
		return 0;
	}
	uint8_t _al = _mstCurrentTaskData->flagsA6;
	uint8_t _dl = _mstCurrentTaskData->flagsA5;
	if ((_al & 2) != 0 || (_dl & 0x30) != 0) {
		return 0;
	}
	uint8_t dir = _dl & 3;
	if (dir == 1) {
// 418AC6
		MstUnk44Unk1 *m44 = _mstCurrentTaskData->unkC;
		if (m44->indexUnk35_24 == kNone) {
			return 0;
		}
		int _ebp = 0;
		if (_mstPosY >= m->yMstPos - m44->y1 && _mstPosY < m->yMstPos + m44->y2) {
			if (m->x1 != -2 || m->x1 != m->x2) {
				if (m->xDelta <= m->x1) {
					if (_al & 1) {
						_ebp = 1;
					} else {
						_al = getBits23(m->flags49) & 1;
						_dl = (_mstCurrentTaskData->o16->flags1 >> 4) & 1;
						if (_dl == _al) {
							_ebp = 1;
						} else if (_mstCurrentTaskData->unk8[946] & 4) {
							_ebp = 1;
						} else if (m->xDelta <= _mstCurrentTaskData->unkC->x2) {
							_ebp = 2;
						}
					}
				}
// 418BAA
			} else if (o->screenNum == _currentScreen) {
				if (m->flagsA6 & 1) {
					_ebp = 1;
				} else {
					warning("executeMstCodeHelper3 mstPosY %d x1 %d x2 %d screen %d", _mstPosY, m->x1, m->x2, o->screenNum);
					// TODO
				}
			}
		}
// 418BAA
		if (_ebp == 0) {
			m->flagsA6 &= ~1;
			if ((m->flagsA5 & 4) == 0) {
				const uint32_t indexUnk35 = m->unkC->indexUnk35_24;
				assert(indexUnk35 != kNone);
				m->m35 = &_res->_mstUnk35[indexUnk35];
			}
			return 0;
		} else if (_ebp == 1) {
// 418C73
			m->flagsA6 |= 1;
			if (executeMstUnk2(_mstCurrentTaskData, _mstCurrentTaskData->xMstPos, _mstCurrentTaskData->yMstPos) == 0 && (m->unk8[946] & 2) == 0) {
				if (_mstCurrentPosX > m->xMstPos && (_mstCurrentPosX < m->unkC->unk2C[1] || _mstCurrentPosX < m->unkC->unk34[1])) {
					uint32_t indexUnk35 = m->unkC->indexUnk35_20;
					if (indexUnk35 != kNone) {
						m->m35 = &_res->_mstUnk35[indexUnk35];
					}
					if (m->flagsA5 & 4) {
						m->flagsA5 &= ~4;
						if (!updateMstTaskDataPosition(m)) {
							initMstTaskData(m);
						}
						indexUnk35 = m->unkC->indexUnk35_20;
						if (indexUnk35 != kNone) {
							m->m35 = &_res->_mstUnk35[indexUnk35];
						}
						prepareMstTask(_mstCurrentTask);
					}
					return 0;
				}
			}
// 418D41
			if ((m->unk8[946] & 2) == 0) {
				MstUnk44Unk1 *m44 = m->unkC;
				int _edi = READ_LE_UINT32(m->unk8 + 904);
				int _ebx = MAX(m->unk88, m44->unk34[1] + _edi);
				int _eax = MIN(m->unk84, m44->unk2C[1] - _edi);
				uint32_t indexUnk36 = m44->indexUnk36_32;
				assert(indexUnk36 != kNone);
				uint32_t indexUnk49 = _res->_mstUnk36[indexUnk36].indexUnk49;
				assert(indexUnk49 != kNone);
				uint8_t _bl = _res->_mstUnk49[indexUnk49].unk0x14 & 0xFF;
				if (ABS(_eax - _ebx) <= _bl) {
					uint32_t indexUnk35 = m44->indexUnk35_20;
					if (indexUnk35 != kNone) {
						m->m35 = &_res->_mstUnk35[indexUnk35];
					}
					if (m->flagsA5 & 4) {
						m->flagsA5 &= ~4;
						if (!updateMstTaskDataPosition(_mstCurrentTaskData)) {
							initMstTaskData(_mstCurrentTaskData);
						}
						m44 = _mstCurrentTaskData->unkC;
						indexUnk35 = m44->indexUnk35_20;
						if (indexUnk35 != kNone) {
							m->m35 = &_res->_mstUnk35[indexUnk35];
						}
						prepareMstTask(_mstCurrentTask);
					}
					return 0;
				}
			}
// 418DEA
			executeMstOp67Type2(_mstCurrentTask, 0);
			return 0;
		}
		assert(_ebp == 2);
		if (m->flagsA6 & 1) {
			return 0;
		}
		const uint32_t indexUnk35 = m->unkC->indexUnk35_24;
		assert(indexUnk35 != kNone);
		if (!m->m35) {
			_mstCurrentTaskData->m35 = &_res->_mstUnk35[indexUnk35];
			_mstCurrentTaskData->unkCC[0] = _rnd.update() & 7;
			_mstCurrentTaskData->unkCC[2] = 0x20;
			_mstCurrentTaskData->unkCC[1] = _rnd.update() & 31;
			prepareMstTask(_mstCurrentTask);
			return 0;
		}
// 418C1D
		if (m->flagsA5 & 4) {
			m->flagsA5 &= ~4;
			if (!updateMstTaskDataPosition(_mstCurrentTaskData)) {
				initMstTaskData(_mstCurrentTaskData);
			}
			_mstCurrentTaskData->m35 = &_res->_mstUnk35[indexUnk35];
			prepareMstTask(_mstCurrentTask);
		}
		return 0;
	} else if (dir != 2) {
		return 0;
	}
	if ((m->flagsA5 & 4) != 0 || (m->flags48 & 8) == 0) {
		return 0;
	}
	if ((m->flagsA5 & 8) == 0 && (m->unk8[946] & 2) == 0) {
		uint8_t _dl = m->flags49;
		if ((_dl & 2) != 0) {
			if (READ_LE_UINT32(m->unk8 + 916) <= m->unkC->unk34[1] || READ_LE_UINT32(m->unk8 + 912) >= m->unkC->unk2C[1]) {
				m->flagsA6 |= 1;
				assert(m == _mstCurrentTaskData);
				m->flagsA5 = 1;
				initMstTaskData(m);
				uint32_t indexUnk35 = m->unkC->indexUnk35_20;
				if (indexUnk35 != kNone) {
					m->m35 = &_res->_mstUnk35[indexUnk35];
				}
				return 0;
			}
		} else if (_dl & 8) {
// 418A37
			if (READ_LE_UINT32(m->unk8 + 920) >= m->unkC->unk2C[1] || READ_LE_UINT32(m->unk8 + 924) <= m->unkC->unk34[1]) {
				m->flagsA6 |= 1;
				assert(m == _mstCurrentTaskData);
				m->flagsA5 = 1;
				initMstTaskData(m);
				uint32_t indexUnk35 = m->unkC->indexUnk35_20;
				if (indexUnk35 != kNone) {
					m->m35 = &_res->_mstUnk35[indexUnk35];
				}
				return 0;
			}
		}
	}
// 418A9A
	if (executeMstUnk2(m, m->xMstPos, m->yMstPos) == 0) {
		executeMstOp67Type2(t, 1);
	}
	return 0;
}

int Game::executeMstCodeHelper4(Task *t) {
	warning("executeMstCodeHelper4 unimplemented");
	// TODO
	return 0;
}

void Game::mstUpdateRefPos() {
	if (_andyObject) {
		_mstRefPosX = _andyObject->xPos;
		_mstRefPosY = _andyObject->yPos;
		_mstPosX = _mstRefPosX + _res->_mstPointOffsets[_currentScreen].xOffset;
		_mstPosY = _mstRefPosY + _res->_mstPointOffsets[_currentScreen].yOffset;
		if (!_specialAnimFlag) {
			_mstUnk10 = updateMstRectsTable(_mstUnk10, 0xFE, _mstPosX, _mstPosY, _mstPosX + _andyObject->width - 1, _mstPosY + _andyObject->height - 1) & 0xFF;
		}
		_mstRefPosX += _andyObject->posTable[3].x;
		_mstRefPosY += _andyObject->posTable[3].y;
		_mstPosX += _andyObject->posTable[3].x;
		_mstPosY += _andyObject->posTable[3].y;
	} else {
		_mstRefPosX = 128;
		_mstRefPosY = 96;
		_mstPosX = _mstRefPosX + _res->_mstPointOffsets[0].xOffset;
		_mstPosY = _mstRefPosY + _res->_mstPointOffsets[0].yOffset;
        }
	_mstMovingStateCount = 0;
	_mstMovingState[0].unk40 = 0;
	if (!_lvlObjectsList0) {
		if (_plasmaCannonDirection == 0) {
			_executeMstLogicPrevCounter = _executeMstLogicCounter;
			return;
		}
		_mstMovingState[0].unk1C = 512;
		_mstMovingState[0].unk20 = 512;
		_mstMovingState[0].unk28 = 0;
		_mstMovingState[0].unk40 = 3;
		_mstMovingState[0].unk18 = 4;
		_mstMovingState[0].xPos = _gameXPosTable[_plasmaCannonFirstIndex] + _res->_mstPointOffsets[_currentScreen].xOffset;
		_mstMovingState[0].yPos = _gameYPosTable[_plasmaCannonFirstIndex] + _res->_mstPointOffsets[_currentScreen].yOffset;
		switch (_plasmaCannonDirection - 1) {
		case 0:
			_mstMovingState[0].unk24 = 6;
			_mstMovingStateCount = 1;
			break;
		case 2:
			_mstMovingState[0].unk24 = 3;
			_mstMovingStateCount = 1;
			break;
		case 1:
			_mstMovingState[0].unk24 = 0;
			_mstMovingStateCount = 1;
			break;
		case 5:
			_mstMovingState[0].unk24 = 4;
			_mstMovingStateCount = 1;
			break;
		case 3:
			_mstMovingState[0].unk24 = 7;
			_mstMovingStateCount = 1;
			break;
		case 11:
			_mstMovingState[0].unk24 = 2;
			_mstMovingStateCount = 1;
			break;
		case 7:
			_mstMovingState[0].unk24 = 5;
			_mstMovingStateCount = 1;
			break;
		case 8:
			_mstMovingState[0].unk24 = 1;
			_mstMovingStateCount = 1;
			break;
		default:
			_mstMovingStateCount = 1;
			break;
		}
	} else {
		MovingOpcodeState *p = _mstMovingState;
		for (LvlObject *o = _lvlObjectsList0; o; o = o->nextPtr) {
			p->o = o;
			assert(o->dataPtr);
			ShootLvlObjectData *ptr = (ShootLvlObjectData *)getLvlObjectDataPtr(o, kObjectDataTypeShoot);
			p->unk28 = ptr;
			if (ptr->unk3 == 0x80) {
				continue;
			}
			if (ptr->dxPos == 0 && ptr->dyPos == 0) {
				continue;
			}
			p->unk1C = ptr->dxPos;
			p->unk20 = ptr->dyPos;
			p->unk24 = ptr->unk1;
			switch (ptr->unk0) {
			case 0:
				p->unk40 = 1;
				p->xPos = o->xPos + _res->_mstPointOffsets[o->screenNum].xOffset + o->posTable[7].x;
				p->unk18 = 3;
				p->yPos = o->yPos + _res->_mstPointOffsets[o->screenNum].yOffset + o->posTable[7].y;
				break;
			case 5:
				p->unk24 |= 0x80;
				// fall-through
			case 4:
				p->unk40 = 2;
				p->xPos = o->xPos + _res->_mstPointOffsets[o->screenNum].xOffset + o->posTable[7].x;
				p->unk18 = 7;
				p->yPos = o->yPos + _res->_mstPointOffsets[o->screenNum].yOffset + o->posTable[7].y;
				break;
			default:
				--p;
				--_mstMovingStateCount;
				break;
			}
			++p;
			++_mstMovingStateCount;
			if (_mstMovingStateCount >= kMaxMovingStates) {
				break;
			}
		}
		if (_mstMovingStateCount == 0) {
			_executeMstLogicPrevCounter = _executeMstLogicCounter;
			return;
		}
	}
	for (int i = 0; i < _mstMovingStateCount; ++i) {
		MovingOpcodeState *p = &_mstMovingState[i];
		p->boundingBox.x2 = p->xPos + p->unk18;
		p->boundingBox.x1 = p->xPos - p->unk18;
		p->boundingBox.y2 = p->yPos + p->unk18;
		p->boundingBox.y1 = p->yPos - p->unk18;
	}
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

void Game::removeMstObjectTask(Task *t, Task **tasksList) {
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
	} else {
		*tasksList = next;
	}
}

void Game::stopMstTaskData(Task *t, Task **tasksList) {
	MstTaskData *m = t->dataPtr;
	if (_mstUnk6 != -1) {
		if ((m->flagsA5 & 8) != 0 && m->unk18) {
			disableMstTaskData(m);
		}
	}
	if (m->unk8[946] & 4) {
		clearMstRectsTable(m, 0);
		clearMstRectsTable(m, 1);
	}
	m->m46 = 0;
	LvlObject *o = m->o16;
	if (o) {
		o->dataPtr = 0;
	}
	for (int i = 0; i < 64; ++i) {
		if (_mstObjectsTable[i].unk0 != 0 && _mstObjectsTable[i].mstTaskData == m) {
			_mstObjectsTable[i].mstTaskData = 0;
		}
	}
	removeLvlObject2(o);
	if (t->child) {
		t->child->codeData = 0;
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

void Game::resetMstTask(Task *t, uint32_t codeData, uint8_t flags) {
	MstTaskData *m = t->dataPtr;
	m->flagsA5 = (m->flagsA5 & ~0x70) | flags;
	Task *c = t->child;
	if (m->flagsA5 & 8) {
		if (c) {
			t->child = 0;
			c->codeData = 0;
		}
		Task *n = findFreeTask();
		if (n) {
			memcpy(n, t, sizeof(Task));
			t->child = n;
			if (t->run != &Game::runTask_waitResetInput) {
				const uint8_t *p = n->codeData - 4;
				if ((t->flags & 0x40) != 0 || p[0] == 203 || ((flags & 0x10) != 0 && (t->run == &Game::runTask_unk1 || t->run == &Game::runTask_unk2 || t->run == &Game::runTask_unk3 || t->run == &Game::runTask_unk4))) {
					p += 4;
				}
				n->codeData = p;
				n->run = &Game::runTask_default;
				assert(codeData != kNone);
				resetTask(t, _res->_mstCodeData + codeData * 4);
			}
		}
	} else {
// 417A1A
		if (c) {
			t->child = 0;
			c->codeData = 0;
		}
	}
// 417A2B
	assert(codeData != kNone);
	resetTask(t, _res->_mstCodeData + codeData * 4);
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

int Game::mstTaskSetActionDirection(Task *t, int num, int delay) {
	uint8_t var4 = _res->_mstUnk52[num * 4];
	uint8_t var8 = _res->_mstUnk52[num * 4 + 2];
	MstTaskData *m = t->dataPtr;
	LvlObject *o = m->o16;
	const uint8_t *ptr = m->unk8 + var4 * 28;
	uint8_t _al = (o->flags1 >> 4) & 3;
	uint8_t _cl = ((_al & 1) != 0 ? 6 : 0) + 2;
	if (_al & 2) {
		_cl |= 4;
	} else {
		_cl |= 1;
	}
	mstLvlObjectSetActionDirection(o, ptr, var8, _cl);
	o->actionKeyMask |= _res->_mstUnk52[num * 4 + 1];
	debug(kDebug_MONSTER, "mstTaskSetActionDirection m %p action 0x%x direction 0x%x", m, o->actionKeyMask, o->directionKeyMask);
	t->flags &= ~0x80;
	int _edi = (int8_t)ptr[4];
	int _ebp = (int8_t)ptr[5];
	if (_edi != 0 || _ebp != 0) {
// 40E8E2
		uint8_t var11 = ptr[2];
		warning("mstTaskSetActionDirection %d unimplemented %d %d 0x%x", num, _edi, _ebp, var11);
		if ((var11 & 0xA) == 0xA) {
			// TODO
		}
		// TODO
		int _eax = 0;
		int _edx = 0;
// 40E9BC
		if ((m->unk8[946] & 4) != 0 && (_edx = ptr[14]) != 0) {
			if (_eax == 0) {
				_ebp = 0;
			} else if (_mstLut1[_eax] & 1) {
// 40E9F
				_ebp = (int8_t)ptr[10];
				_eax = (int8_t)ptr[11];
			} else {
// 40EA12
				_ebp = (int8_t)ptr[8];
				_eax = (int8_t)ptr[9];
			}
// 40EA1A
			if (o->directionKeyMask & 8) {
				_ebp = -_ebp;
			} else if ((o->directionKeyMask & 2) == 0) {
				_ebp = 0;
			}
			if (o->directionKeyMask & 1) {
				_eax = -_eax;
			} else if ((o->directionKeyMask & 4) == 0) {
				_eax = 0;
			}
// 40EA40
			_edi = m->xMstPos + (int8_t)ptr[12] + _ebp;
			_ebp = m->yMstPos + (int8_t)ptr[13] + _eax;
			if ((var8 & 0xE0) == 0x60 && checkMstRectsTable(m->soundType, _edi, _ebp, (_edx & 255) + _edi - 1, ptr[15] + _ebp - 1)) {
				t->flags |= 0x80;
				return 0;
			}
// 40EAA0
			m->flagsA8[0] = updateMstRectsTable(m->flagsA8[0], m->soundType, _edi, _ebp, ptr[14] + _edi - 1, ptr[15] + _ebp - 1);
		}
	} else {
		if ((m->unk8[946] & 4) != 0 && ptr[14] != 0) {
			warning("mstTaskSetActionDirection %d unimplemented ptr[14] %d", num, ptr[14]);
			// TODO
		}
	}
// 40EAD0
	m->flagsA4 = var4;
	if (delay == -1) {
		const uint32_t offset = m->unk8 - _res->_mstHeightMapData;
		assert((offset % 948) == 0);
		t->tempVar = offset / 948;
		t->run = &Game::runTask_unk4;
	} else {
		t->delay = delay;
		t->run = &Game::runTask_unk3;
	}
	return 1;
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
	assert(codeData);
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
				m->flags48 |= 8;
				const MstUnk46Unk1 *m46unk1 = m->unk4;
				if (m46unk1->indexUnk51 != kNone) {
					m->flags48 |= 4;
				}
				if (m46unk1->indexUnk47 != kNone) {
					m->flags48 |= 0x10;
				}
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
		*tasksList = t;
		t->nextPtr = t->prevPtr = 0;
	} else {
		// go to last element
		Task *next = current->nextPtr;
		while (next) {
			current = next;
			next = current->nextPtr;
		}
		assert(!current->nextPtr);
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
	case 2: {
			MstTaskData *m = t->dataPtr;
			if (m) {
				return ((m->o16->flags1 & 0x10) != 0) ? 1 : 0;
			} else if (t->mstObject) {
				return ((t->mstObject->o->flags1 & 0x10) != 0) ? 1 : 0;
			}
		}
		break;
	case 3: {
			MstTaskData *m = t->dataPtr;
			if (m) {
				return ((m->o16->flags1 & 0x20) != 0) ? 1 : 0;
			} else if (t->mstObject) {
				return ((t->mstObject->o->flags1 & 0x20) != 0) ? 1 : 0;
			}
		}
	case 4: {
			MstTaskData *m = t->dataPtr;
			if (m) {
				return ((m->o16->flags0 & 0x200) != 0) ? 1 : 0;
			} else if (t->mstObject) {
				return ((t->mstObject->o->flags0 & 0x200) != 0) ? 1 : 0;
			}
		}
		break;
	case 5:
		return (_andyObject->flags0 & 0x1F) == 7;
	case 6:
		return (_andyObject->spriteNum == 0);
	case 7:
		if ((_andyObject->flags0 & 0x1F) == 7) {
			AndyLvlObjectData *andyData = (AndyLvlObjectData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (andyData) {
				LvlObject *o = andyData->shootLvlObject;
				if (o) {
					ShootLvlObjectData *data = (ShootLvlObjectData *)getLvlObjectDataPtr(o, kObjectDataTypeShoot);
					if (data) {
						return (data->unk0 == 4) ? 1 : 0;
					}
				}
			}
		}
		break;
	case 8:
		if ((_andyObject->flags0 & 0x1F) == 7) {
			AndyLvlObjectData *andyData = (AndyLvlObjectData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (andyData) {
				LvlObject *o = andyData->shootLvlObject;
				if (o) {
					ShootLvlObjectData *data = (ShootLvlObjectData *)getLvlObjectDataPtr(o, kObjectDataTypeShoot);
					if (data) {
						return (data->unk0 == 0) ? 1 : 0;
					}
				}
			}
		}
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
	case 7:
		if (t->dataPtr && t->dataPtr->unk1C) {
			return t->dataPtr->unk1C->unk40;
		}
		return _mstMovingState[0].unk40;
	case 8:
		if (t->dataPtr && t->dataPtr->unk1C) {
			return t->dataPtr->unk1C->unk24 & 0x7F;
		}
		return _mstMovingState[0].unk24 & 0x7F;
	case 9:
		if (t->dataPtr && t->dataPtr->unk18) {
			return t->dataPtr->unk18->unk8;
		}
		break;
	case 10:
		if (t->dataPtr && t->dataPtr->unk18) {
			return t->dataPtr->unk18->unkC;
		}
		break;
	case 11:
		if (t->dataPtr) {
			return t->dataPtr->unkF4;
		}
		break;
	case 12:
		if (t->dataPtr) {
			return t->dataPtr->xPos;
		} else if (t->mstObject) {
			return t->mstObject->xPos;
		}
		break;
	case 13:
		if (t->dataPtr) {
			return t->dataPtr->yPos;
		} else if (t->mstObject) {
			return t->mstObject->yPos;
		}
		break;
	case 14:
		if (t->dataPtr) {
			return t->dataPtr->o16->screenNum;
		} else if (t->mstObject) {
			return t->mstObject->o->screenNum;
		}
		break;
	case 15:
		if (t->dataPtr) {
			return t->dataPtr->xDelta;
		} else if (t->mstObject) {
			return ABS(_mstPosX - t->mstObject->xMstPos);
		}
		break;
	case 16:
		if (t->dataPtr) {
			return t->dataPtr->yDelta;
		} else if (t->mstObject) {
			return ABS(_mstPosY - t->mstObject->yMstPos);
		}
		break;
	case 17:
		if (t->dataPtr) {
			return t->dataPtr->unkEC;
		}
		break;
	case 18:
		if (t->dataPtr) {
			return ABS(t->dataPtr->x2 - t->dataPtr->xMstPos);
		}
		break;
	case 19:
		if (t->dataPtr) {
			return ABS(t->dataPtr->x1 - t->dataPtr->xMstPos);
		}
		break;
	case 20:
		if (t->dataPtr) {
			return ABS(t->dataPtr->y2 - t->dataPtr->yMstPos);
		}
		break;
	case 21:
		if (t->dataPtr) {
			return ABS(t->dataPtr->y1 - t->dataPtr->yMstPos);
		}
		break;
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
		break;
	case 26:
		if (t->dataPtr) {
			return t->dataPtr->yMstPos;
		} else if (t->mstObject) {
			return t->mstObject->yMstPos;
		}
		break;
	case 27:
		if (t->dataPtr) {
			return t->dataPtr->o16->flags0 & 0xFF;
		} else if (t->mstObject) {
			return t->mstObject->o->flags0 & 0xFF;
		}
		break;
	case 28:
		if (t->dataPtr) {
			return t->dataPtr->o16->anim;
		} else if (t->mstObject) {
			return t->mstObject->o->anim;
		}
		break;
	case 29:
		if (t->dataPtr) {
			return t->dataPtr->o16->frame;
		} else if (t->mstObject) {
			return t->mstObject->o->frame;
		}
		break;
	case 30:
		return _mstOp56Counter;
	case 31:
		return _executeMstLogicCounter;
	case 32:
		return _executeMstLogicCounter - _executeMstLogicPrevCounter;
	case 33: {
			LvlObject *o = 0;
			if (t->dataPtr) {
				o = t->dataPtr->o16;
			} else if (t->mstObject) {
				o = t->mstObject->o;
			}
			if (o) {
				return _res->_screensState[o->screenNum].s0;
			}
		}
		break;
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
	if (!t->codeData) {
		warning("runTask_default has NULL codeData t %p", t);
		return 1;
	}
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
		case 3: // 3 - set_monster_action_direction_imm
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				ret = mstTaskSetActionDirection(t, num, (arg == 0xFF) ? -1 : arg);
			}
			break;
		case 4: // 4 - set_monster_action_direction_task_var
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				assert(arg < kMaxLocals);
				ret = mstTaskSetActionDirection(t, num, t->localVars[arg]);
			}
			break;
		case 5: // 5 - set_monster_action_direction_global_var
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				assert(arg < kMaxVars);
				ret = mstTaskSetActionDirection(t, num, _mstVars[arg]);
			}
			break;
		case 6: // 6 - set_monster_action_direction_other_var
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				ret = mstTaskSetActionDirection(t, num, getTaskOtherVar(arg, t));
			}
			break;
		case 7: // 7 - set_monster_action_direction_mst_var
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				assert(arg < kMaxLocals);
				ret = mstTaskSetActionDirection(t, num, t->dataPtr->localVars[arg]);
			}
			break;
		case 13: // 8
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				if (executeMstUnk17(t->dataPtr, num)) {
					const int arg = _res->_mstUnk52[num * 4 + 3];
					t->codeData = p;
					ret = mstTaskSetActionDirection(t, num, (arg == 0xFF) ? -1 : arg);
				}
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
				t->tempVar = p[1];
				if (((1 << p[1]) & _mstFlags) == 0) {
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
		case 32: { // 22
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					t->delay = 5;
					t->tempVar = p[1];
					if (((1 << p[1]) & m->flags48) == 0) {
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
		case 41: { // 28 - increment_task_var
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
		case 43: { // 30 - increment_monster_var
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					const int num = p[1];
					assert(num < kMaxLocals);
					++m->localVars[num];
				}
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
		case 46: { // 33 - decrement_monster_var
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					const int num = p[1];
					assert(num < kMaxLocals);
					--m->localVars[num];
				}
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
		case 56: { // 34 - arith_task_var_task_var
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
		case 66: { // 35 - arith_global_var_task_var
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
		case 157:
		case 158:
		case 159:
		case 160:
		case 161:
		case 162:
		case 163:
		case 164:
		case 165:
		case 166: { // 45
				MstTaskData *m = 0;
				if (t->mstObject) {
					m = t->mstObject->mstTaskData;
				} else {
					m = t->dataPtr;
				}
				if (m) {
					const int num = p[2];
					assert(p[1] < kMaxLocals);
					arithOp(p[0] - 157, &m->localVars[p[1]], getTaskOtherVar(num, t));
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
		case 197: // 49
			if (t->dataPtr) {
				const int num = READ_LE_UINT16(p + 2);
				MstOp49Data *m = &_res->_mstOp49Data[num];
				const uint32_t mask = m->unk8;
				int a = getTaskVar(t, m->unk0, (mask >> 16) & 15); // var1C
				int b = getTaskVar(t, m->unk2, (mask >> 12) & 15); // x2
				int c = getTaskVar(t, m->unk4, (mask >>  8) & 15); // var14
				int d = getTaskVar(t, m->unk6, (mask >>  4) & 15); // _esi
				int e = getTaskVar(t, m->unkE,  mask        & 15); // _eax
				if (e >= _res->_mstHdr.pointsCount) {
					e = _res->_mstHdr.pointsCount - 1;
				}
				ret = executeMstOp49(a, b, c, d, e, t, num);
			}
			break;
		case 198: { // 50
				Task *child = findFreeTask();
				if (child) {
					t->codeData = p + 4;
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
		case 200: // 52
			if (t->dataPtr && t->dataPtr->unk18) {
				executeMstOp52();
				return 1;
			}
			executeMstOp52();
			break;
		case 202: // 54
			executeMstOp54();
			break;
		case 204: // 56
			ret = executeMstOp56(t, p[1], READ_LE_UINT16(p + 2));
			break;
		case 207:
		case 208:
		case 209: // 79
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
						warning("Unhandled lvlObject.flags 0x%x screen %d\n", o->flags, e);
						// TODO

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
					_mstOp67_type = m->unk8;
					_mstOp67_flags1 = m->unk9;
					_mstOp67_unk = m->unkC;
					_mstOp67_x1 = a;
					_mstOp67_x2 = b;
					_mstOp67_y1 = c;
					_mstOp67_y2 = d;
					_mstOp67_screenNum = e;
					break;
				} else if (p[0] == 225) {
					_mstOp68_arg8 = m->unk8;
					_mstOp68_arg9 = m->unk9;
					_mstOp68_flags1 = m->unkC;
					_mstOp68_x1 = a;
					_mstOp68_x2 = b;
					_mstOp68_y1 = c;
					_mstOp68_y2 = d;
					_mstOp68_screenNum = e;
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
				mstOp67(t, a, b, c, d, e, m->unk8, m->unk9, m->unkC, m->unkB, 0, m->unkE);
			}
			break;
		case 226: { // 68
				const int num = READ_LE_UINT16(p + 2);
				const MstUnk63 *m63 = &_res->_mstUnk63[num];
				int _edi  = _res->_mstPointOffsets[_currentScreen].xOffset;
				int var14 = _res->_mstPointOffsets[_currentScreen].yOffset;
				int var1C = 0;
				int var8  = 0;
				int _ecx  = m63->unk4 * 256;
				int _edx  = _edi + 256 + _ecx;
				_edi -= _ecx;
				int var20 = _edx;
				for (int i = 0; i < 32; ++i) {
					MstTaskData *m = &_mstUnkDataTable[i];
					if (!m->m46) {
						continue;
					}
					if (m->unk8[944] != _res->_mstHeightMapData[m63->unk0 * 948 + 944]) {
						continue;
					}
					if (m->xMstPos < _edi || m->xMstPos > var20) {
						continue;
					}
					if (m->yMstPos < var14 || m->yMstPos > var14 + 192) {
						continue;
					}
					if (_mstPosX > m->xMstPos) {
						++var8;
					} else {
						++var1C;
					}
				}
				t->flags |= 0x80;
				_edi = var1C;
				_edx = var8;
				int _ebx = var1C + var8;
				if (_ebx >= m63->unk3) {
					break;
				}
				_edi += var8;
				_ecx = m63->unk3 - _edi;
				_edi = m63->unk1;
				if (var8 >= _edi) {
					_edi = 0;
				} else {
					_edi -= var8;
				}
				int _esi = m63->unk2;
				if (var1C >= _esi) {
					_esi = 0;
				} else {
					_esi -= var1C;
				}
				if (_edi != 0) {
					if (_mstOp67_screenNum < 0) {
						if (_mstOp67_x2 >= -_mstRefPosX && _mstOp67_x1 <= -_mstRefPosX * 2) {
							_edi = 0;
						}
					} else if (_mstOp67_screenNum == _currentScreen) {
						_edi = 0;
					}
				}
				if (_esi != 0) {
					if (_mstOp68_screenNum < 0) {
						if (_mstOp68_x2 >= -_mstRefPosX && _mstOp68_x1 <= -_mstRefPosX * 2) {
							_esi = 0;
						}
					} else if (_mstOp68_screenNum == _currentScreen) {
						_esi = 0;
					}
				}
				if (_edi != 0 || _esi != 0) {
					mstOp68(t, _res->_mstHeightMapData + m63->unk0 * 948, _edi, _esi, _ecx, m63->unk6);
				}
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
		case 231:
		case 232: { // 71
				const int num = READ_LE_UINT16(p + 2);
				const MstUnk55 *m = &_res->_mstUnk55[num];
				const int a = getTaskFlag(t, m->indexVar1, m->maskVars & 15);
				const int b = getTaskFlag(t, m->indexVar2, m->maskVars >> 4);
				// TODO: mstOp231 & mstOp232 are duplicating code
				// could we simply break and leave the codeData pointer to the same opcode ?
				if (compareOp(m->compare, a, b)) {
					if (p[0] == 231) {
						LvlObject *o = 0;
						MstTaskData *m = t->dataPtr;
						if (m) {
							if ((m->flagsA6 & 2) == 0) {
								o = m->o16;
							}
						} else if (t->mstObject) {
							o = t->mstObject->o;
						}
						if (o) {
							o->actionKeyMask = 0;
							o->directionKeyMask = 0;
						}
						t->tempVar = num;
						t->run = &Game::runTask_mstOp231;
						ret = 1;
					}
				} else {
					if (p[0] == 232) {
						LvlObject *o = 0;
						MstTaskData *m = t->dataPtr;
						if (m) {
							if ((m->flagsA6 & 2) == 0) {
								o = m->o16;
							}
						} else if (t->mstObject) {
							o = t->mstObject->o;
						}
						if (o) {
							o->actionKeyMask = 0;
							o->directionKeyMask = 0;
						}
						t->tempVar = num;
						t->run = &Game::runTask_mstOp232;
						ret = 1;
					}
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
						t->run = &Game::runTask_mstOp233;
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
						t->run = &Game::runTask_mstOp234;
						ret = 1;
					}
				}
			}
			break;
		case 237: // 74 - remove_monster_task
			if (t->dataPtr) {
				if (!t->mstObject) {
					stopMstTaskData(t, &_mstTasksList1);
					return 1;
				}
				removeMstObjectTask(t, &_mstTasksList2);
				return 1;
			} else {
				if (t->mstObject) {
					removeMstObjectTask(t, &_mstTasksList2);
					return 1;
				}
			}
			break;
		case 238: { // 75
				const int i = READ_LE_UINT16(p + 2);
				const uint32_t codeData = _res->_mstUnk60[i];
				assert(codeData != kNone);
				p = _res->_mstCodeData + codeData * 4;
				t->codeData = p;
				p -= 4;
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
						warning(".mst opcode 242 t->child flagsA5 0x%x", m->flagsA5);
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
						if ((m->flagsA5 & 0x70) != 0) {
							m->flagsA5 &= ~0x70;
							switch (m->flagsA5 & 7) {
							case 1:
							case 2: {
									MstUnk35 *m35 = m->m35;
									uint32_t num = 0;
									if (m35->count2 != 0) {
										const uint8_t i = shuffleFlags(m->unkCC);
										num = m35->data2[i];
									}
									const uint32_t codeData = m35->indexCodeData[num];
									assert(codeData != kNone);
									resetTask(t, _res->_mstCodeData + codeData * 4);
									t->runningState &= ~2;
									p = t->codeData - 4;
								}
								break;
							case 5:
								return executeMstOp67Type1(t);
							case 6:
								return executeMstOp67Type2(t, 1);
							}
						} else {
// 413DCA
							MstUnk35 *m35 = m->m35;
							uint32_t num = 0;
							if (m35->count2 != 0) {
								const uint8_t i = shuffleFlags(m->unkCC);
								num = m35->data2[i];
							}
							const uint32_t codeData = m35->indexCodeData[num];
							assert(codeData != kNone);
							resetTask(t, _res->_mstCodeData + codeData * 4);
							t->runningState &= ~2;
							const int counter = m->executeCounter;
							m->executeCounter = _executeMstLogicCounter;
							p = t->codeData - 4;
							if (m->executeCounter == counter) {
								if ((m->flagsA6 & 2) == 0) {
									if (m->o16) {
										m->o16->actionKeyMask = 0;
										m->o16->directionKeyMask = 0;
									}
								}
								ret = 1;
							}
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
			const uint8_t *ptr = m->unk8;
			if (ptr[946] & 4) {
				clearMstRectsTable(m, 0);
				clearMstRectsTable(m, 1);
			}
			m->m46 = 0;
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

int Game::executeMstOp49(int a, int b, int c, int d, int screen, Task *t, int num) {
	MstTaskData *m = t->dataPtr;
	const MstOp49Data *m49 = &_res->_mstOp49Data[num];
	m->m49 = &_res->_mstUnk49[m49->unkC];
	m->unkDC = m49->unkF;
	if (m49->unkF < 0) {
	}
// 41BAD4
	return 0;
}

void Game::executeMstOp52() {
	if (_mstUnk6 == -1) {
		return;
	}
	MstUnk48 *m48 = &_res->_mstUnk48[_mstUnk6];
	int j = 0;
	for (int i = 0; i < m48->countUnk12; ++i) {
		MstUnk48Unk12 *m48unk12 = &m48->unk12[j];
		const uint8_t num = m48unk12->data->unk1B;
		if (num != 255) {
			assert(num < 32);
			MstTaskData *m = &_mstUnkDataTable[num];
			m->flags48 &= ~0x50;
			m->unk18->unk1B = 255;
			m->unk18 = 0;
			--_mstTaskDataCount;
			if (_mstTaskDataCount <= 0) {
				_mstUnk6 = -1;
			}
			if (m->flagsA5 & 0x70) {
				const int a = (m->o16->flags0 & 255) * 7;
				if (m->unk8[a * 4] != 0) {
					// TODO
					warning("MstOp52 t->task %p unimplemented", m->task->run);
				} else {
// 41D7D8
					m->flagsA5 &= ~0xF;
					m->flagsA5 |= 6;
					executeMstOp67Type2(m->task, 1);
				}
			}
		}
		++j;
	}
	_mstUnk6 = -1;
}

bool Game::checkMstOp54Helper(MstUnk48 *m48, uint8_t flag) {
	for (int i = 0; i < 2; ++i) {
		for (uint32_t j = 0; j < m48->count[i]; ++j) {
			uint32_t a = (i ^ flag); // * 32; // _edx
			uint32_t n = m48->data1[i][j]; // _eax
			if (_mstCollisionTable[a][n].unk80 < m48->data2[i][j]) {
				return false;
			}
		}
	}
	uint8_t _op54Data[32];
	memset(_op54Data, 0, sizeof(_op54Data));
	int var24 = 0;
	int var28 = 0;
	int var18 = 0;
	int _edi = 0;
	for (int i = 0; i < m48->countUnk12; ++i) {
		MstUnk48Unk12 *m12 = &m48->unk12[i];
		MstUnk48Unk12Unk4 *m12u4 = m12->data;
		if (m12->unk0 != 0) {
			uint8_t var1C = m12u4->unk18;
			if (var1C != 2) {
				_edi = var1C;
			}
// 41DB85
l1:
			int var4C = _edi;
			int var8 = m12u4->unk8;
			int _ebx = var8;
			int var4 = m12u4->unkC;
			int _esi = var4;
			int _eax = _edi ^ flag;
			if (_eax == 1) {
				_ebx = -_ebx;
			}
			if (_ebx >= _mstPosXmin && _ebx <= _mstPosXmax) {
				uint8_t var4D = _res->_mstHeightMapData[m12u4->unk0 * 948 + 946] & 2;
				if (var4D != 0 && (_esi < _mstPosYmin || _esi > _mstPosYmax)) {
					if (var1C != 2 || _edi == 1) {
						return false;
					}
					_edi = 1;
					var4C = 1;
					goto l1; // goto 41DB85
				}
// 41DC19
				MstCollision *varC = &_mstCollisionTable[_eax][m12u4->unk0];
				_ebx += _mstPosX;
				int var44 =  _ebx;
				_esi += _mstPosY;
				int var38 = _esi;
				int var2C = 0x1000000;
				int var30 = 0x1000000;
				int var34 = -1;
				int var10 = varC->unk80;
				if (var10 > 0) {
					MstCollision *var20 = varC;
					assert(var10 == 1);
					for (int j = 0; j < var10; ++j, var20 += 4) {
						MstTaskData *m = var20->m;
						if (_op54Data[m->soundType] == 0 && (m12u4->unk1A < 0 || m->o16->screenNum == m12u4->unk1A)) {
							int _ebp = ABS(var38 - m->yMstPos);
							int _ecx = ABS(_ebx  - m->xMstPos);
							if (_ecx > m48->unk0 || _ebp > m48->unk2) {
								continue;
							}
							if ((var8 || var4) && m->unk8[944] != 10 && m->unk8[944] != 16 && m->unk8[944] != 9) {
								if (_esi <= 0) {
									if (m->x2 > _ebx) {
										continue;
									}
								} else {
									if (m->x1 < _ebx) {
										continue;
									}
								}
								if (var4D != 0) {
									if (_ebp <= 0) {
										if (m->y2 > var38) {
											continue;
										}
									} else {
										if (m->y1 < var38) {
											continue;
										}
									}
								}
							}
// 41DD5A
							if (_ecx <= var30 && _eax <= var2C) {
								var2C = _eax;
								var30 = _ecx;
								var34 = j;
							}
						}
					}
// 41DD80
				}
// 41DDA7
				if (var1C != 2 || var4C == 1) {
					return false;
				}
				var4C = _edi;
				goto l1; // goto 41DB85
			}
// 41DDD0
			if (var1C != 2 || var4C == 1) {
				return false;
			}
			_edi = 1;
			var4C = 1;
			goto l1; // goto 41DB85
		}
// 41DE1E
		var18 += 12;
		++var28;
	}
	var28 = _edi;
	int var20 = 0;
	for (int i = _edi; i < m48->countUnk12; ++i) {
		MstUnk48Unk12 *m12 = &m48->unk12[i]; // var20
		MstUnk48Unk12Unk4 *m12u4 = m12->data;
		if (m12->unk0 == 0) {
			uint8_t var1C = m12u4->unk18;
			m12u4->unk1B = 255;
			int var4C = (var1C == 2) ? 0 : var1C;
// 41DE98
l2:
			int var4 = m12u4->unk8;
			int _ebx = var4;
			int var8 = m12u4->unkC;
			int _esi = var8;
			if ((var4C ^ flag) == 1) {
				_ebx = -_ebx;
			}
			if (_ebx >= _mstPosXmin && _ebx <= _mstPosXmax) {
				uint8_t var4D = _res->_mstHeightMapData[m12u4->unk0 * 948 + 946] & 2;
				if (var4D == 0 && _esi >= _mstPosYmin && _esi <= _mstPosYmax) {
// 41DF10
					warning("checkMstOp54Helper 41DF10 unimplemented");
					// TODO

				}
			}
// 41E09E
			if (var1C == 2 && var4C != 1) {
				// _edx = 1;
				var4C = 1;
				goto l2; // goto 41DE98;
			}
// 41E0E4
		}
		var20 += 12;
		++var28;
	}
	return var24 != 0;
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
	executeMstUnk12();
	if (m43->count2 == 0) {
		uint32_t indexUnk48 = m43->indexUnk48[0];
		assert(indexUnk48 != kNone);
		MstUnk48 *m48 = &_res->_mstUnk48[indexUnk48];
		if (m48->unk4 == 0) {
			if (checkMstOp54Helper(m48, 0)) {
				addMstTaskData(m48, 0);
			}
		} else {
			uint8_t flag = _rnd.update() & 1;
			if (checkMstOp54Helper(m48, flag) && addMstTaskData(m48, flag)) {
			} else {
				flag ^= 1;
				if (checkMstOp54Helper(m48, flag)) {
					addMstTaskData(m48, flag);
				}
			}
		}
// 41E36E
		if (_mstUnk6 == -1) {
			++_mstOp54Counter;
		}
		if (_mstOp54Counter <= 16) {
			return;
		}
		_mstOp54Counter = 0;
		for (uint32_t i = 0; i < m43->count2; ++i) {
			m43->data2[i] &= ~0x80;
		}
		shuffleArray(m43->data2, m43->count2);

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

static uint8_t getLvlObjectFlag(uint8_t type, const LvlObject *o, const LvlObject *andyObject) {
	switch (type) {
	case 0:
		return 0;
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
	default:
		warning("getLvlObjectFlag unhandled type %d", type);
		break;
	}
	return 0;
}

int Game::executeMstOp56(Task *t, int code, int num) {
	assert(num < _res->_mstHdr.unk0x78);
	debug(kDebug_MONSTER, "executeMstOp56 code %d", code);
	switch (code) {
	case 0:
		if (!_specialAnimFlag && setAndySpecialAnimation(0x71) != 0) {
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
				const uint8_t flags = getLvlObjectFlag(_res->_mstOp56Data[num].unkC & 255, tmpObject, _andyObject);
				_specialAnimMask = ((flags & 3) << 4) | (_specialAnimMask & ~0x30);
				// _specialAnimScreenNum = tmpObject->screenNum;
				_specialAnimLvlObject = tmpObject;
				_mstOriginPosX = _res->_mstOp56Data[num].unk4 & 0xFFFF;
				_mstOriginPosY = _res->_mstOp56Data[num].unk8 & 0xFFFF;
			} else {
				_specialAnimMask = merge_bits(_specialAnimMask, _andyObject->flags1, 0x30); // _specialAnimMask ^= (_specialAnimMask ^ _andyObject->flags1) & 0x30;
				// _specialAnimScreenNum = _andyObject->screenNum;
				_specialAnimLvlObject = _andyObject;
				_mstOriginPosX = _andyObject->posTable[3].x - _andyObject->posTable[6].x;
				_mstOriginPosY = _andyObject->posTable[3].y - _andyObject->posTable[6].y;
			}
			_specialAnimFlag = true;
		}
// 411BBA
		if (_mstUnk10 != 255) {
			_mstRectsTable[_mstUnk10].num = 255;
		}
		break;
	case 1:
		if (!_specialAnimFlag) {
			break;
		}
		if (setAndySpecialAnimation(0x61) != 0) {
			_plasmaCannonFlags &= ~1;
			if (_andyObject->spriteNum == 0) {
				_mstCurrentAnim = _res->_mstOp56Data[num].unk0 & 0xFFFF;
			} else {
				_mstCurrentAnim = _res->_mstOp56Data[num].unk0 >> 16;
			}
// 4118ED
			LvlObject *o = 0;
			if (t->mstObject) {
				o = t->mstObject->o;
			} else if (t->dataPtr) {
				o = t->dataPtr->o16;
			}
			if (_res->_mstOp56Data[num].unkC != 6 && o) {
				LvlObject *tmpObject = t->dataPtr->o16;
				const uint8_t flags = getLvlObjectFlag(_res->_mstOp56Data[num].unkC & 255, tmpObject, _andyObject);
				_specialAnimMask = ((flags & 3) << 4) | (_specialAnimMask & 0xFFCF);
				// _specialAnimScreenNum = tmpObject->screenNum;
				_specialAnimLvlObject = tmpObject;
				_mstOriginPosX = _res->_mstOp56Data[num].unk4 & 0xFFFF;
				_mstOriginPosY = _res->_mstOp56Data[num].unk8 & 0xFFFF;
			} else {
				_specialAnimMask = merge_bits(_specialAnimMask, _andyObject->flags1, 0x30); // _specialAnimMask ^= (_specialAnimMask ^ _andyObject->flags1) & 0x30;
				// _specialAnimScreenNum = _andyObject->screenNum;
				_specialAnimLvlObject = _andyObject;
				_mstOriginPosX = _andyObject->posTable[3].x - _andyObject->posTable[6].x;
				_mstOriginPosY = _andyObject->posTable[3].y - _andyObject->posTable[6].y;
			}
			_specialAnimFlag = false;
		}
// 4119F5
		_mstUnk10 = updateMstRectsTable(_mstUnk10, 0xFE, _mstPosX, _mstPosY, _mstPosX + _andyObject->width - 1, _mstPosY + _andyObject->height - 1);
		break;
	case 2: {
			LvlObject *o = t->dataPtr->o16;
			const uint8_t flags = getLvlObjectFlag(_res->_mstOp56Data[num].unk0 & 255, o, _andyObject);
			setAndySpecialAnimation(flags | 0x10);
		}
		break;
	case 3:
		setAndySpecialAnimation(0x12);
		break;
	case 4:
		setAndySpecialAnimation(0x80);
		break;
	case 5: // game over, restart level
		setAndySpecialAnimation(0xA4);
		break;
	case 6:
		setAndySpecialAnimation(0xA3);
		break;
	case 7:
		setAndySpecialAnimation(0x05);
		break;
	case 8:
		setAndySpecialAnimation(0xA1);
		break;
	case 9:
		setAndySpecialAnimation(0xA2);
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
					} else if (t->dataPtr) {
						xPos += t->dataPtr->xMstPos;
						yPos += t->dataPtr->yMstPos;
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
			}
			if (code == 13) {
				assert(o);
				xPos -= _res->_mstPointOffsets[screenNum].xOffset;
				xPos -= o->posTable[7].x;
				yPos -= _res->_mstPointOffsets[screenNum].yOffset;
				yPos -= o->posTable[7].y;
				o->screenNum = screenNum;
				o->xPos = xPos;
				o->yPos = yPos;
				setLvlObjectPosInScreenGrid(o, 7);
				if (t->mstObject) {
					mstTaskSetScreenPosition(t);
				} else {
					mstTaskUpdateScreenPosition(t);
				}
			} else if (code == 14) {
				assert(_andyObject);
				const uint16_t pos = dat->unkC >> 16;
				assert(pos < 8);
				xPos -= _res->_mstPointOffsets[screenNum].xOffset;
				xPos -= _andyObject->posTable[pos].x;
				yPos -= _res->_mstPointOffsets[screenNum].yOffset;
				yPos -= _andyObject->posTable[pos].y;
				_andyObject->screenNum = screenNum;
				_andyObject->xPos = xPos;
				_andyObject->yPos = yPos;
				updateLvlObjectScreen(_andyObject);
				mstUpdateRefPos();
				updateMstHeightMapData();
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
			const uint8_t flags = getLvlObjectFlag(_res->_mstOp56Data[num].unk8 & 255, o, _andyObject);
			_andyObject->flags1 = ((flags & 3) << 4) | (_andyObject->flags1 & 0xFFCF);
			const int x3 = _andyObject->posTable[3].x;
			const int y3 = _andyObject->posTable[3].y;
			setupLvlObjectBitmap(_andyObject);
			_andyObject->xPos += (x3 - _andyObject->posTable[3].x);
			_andyObject->yPos += (y3 - _andyObject->posTable[3].y);
			updateLvlObjectScreen(o);
			mstUpdateRefPos();
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
			if (!updateMstTaskDataPosition(m)) {
				initMstTaskData(m);
			}
			prepareMstTask(t);
			break;
		case 6:
			m->flagsA5 &= ~7;
			if (executeMstUnk2(m, m->xMstPos, m->yMstPos) == 0) {
				m->flagsA5 |= 1;
				if (!updateMstTaskDataPosition(m)) {
					initMstTaskData(m);
				}
				uint32_t indexUnk35 = m->unkC->indexUnk35_20;
				if (indexUnk35 != kNone) {
					m->m35 = &_res->_mstUnk35[indexUnk35];
				}
				prepareMstTask(t);
			} else {
				m->flagsA5 |= 2;
				if (!updateMstTaskDataPosition(m)) {
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
	_mstCurrentPosX = x; // _esi
	_mstCurrentPosY = y;
	const uint8_t *ptr = m->unk8;
	const int32_t a = READ_LE_UINT32(ptr + 900);
	int _ecx = _mstPosX - a;
	int _edi = _mstPosX + a;
	if (ptr[946] & 2) {
		int _ebx = _mstPosY - a;
		int _ebp = _mstPosY + a;
		if (x > _ecx && x < _edi && y > _ebx && y < _ebp) {
			if (ABS(x - _mstPosX) > ABS(y - _mstPosY)) {
				if (x >= _mstPosX) {
					_mstCurrentPosX = _edi;
				} else {
					_mstCurrentPosX = _ecx;
				}
				return 0;
			}
// 419E1D
			if (y >= _mstPosY) {
				_mstCurrentPosY = _ebp;
			} else {
				_mstCurrentPosY = _ebx;
			}
			return 0;
		}
// 419E4F
		const int32_t b = READ_LE_UINT32(ptr + 896);
		_ecx -= b;
		_edi += b;
		_ebx -= b;
		_ebp += b;
		if (x < _ecx) {
			_mstCurrentPosX = _ecx;
		} else if (x > _edi) {
			_mstCurrentPosX = _edi;
		}
		if (y < _ebx) {
			_mstCurrentPosY = _ebx;
		} else if (y > _ebp) {
			_mstCurrentPosY = _ebp;
		}
		if (_mstCurrentPosX != x || _mstCurrentPosY != y) {
			return 0;
		}
		return 1;
	}
// 419EA7
	if (x > _ecx && x < _edi) {
		_mstCurrentPosX = _edi;
		if (x < _mstPosX) {
			_mstCurrentPosX = _ecx;
		}
		return 0;
	}
	const int32_t b = READ_LE_UINT32(ptr + 896);
	_ecx -= b;
	_edi += b;
	if (x < _ecx) {
		_mstCurrentPosX = _ecx;
		return 0;
	} else if (x > _edi) {
		_mstCurrentPosX = _edi;
		return 0;
	}
	return 1;
}

void Game::executeMstUnk7(MstTaskData *m) {
	Task *t = m->task;
	t->flags &= ~0x80;
	int x = m->xMstPos;
	if (x < m->unk74) {
		_xMstPos1 = x = m->unk7C;
		if ((m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0) {
			if (x > m->unk84) {
				t->flags |= 0x80;
				x = m->unk84;
			}
		}
		if (x > m->x1) {
			t->flags |= 0x80;
			x = m->x1;
		}
		_xMstPos2 = x - m->xMstPos;
		m->flags4A = 2;
	} else if (x > m->unk7C) {
// 41A2AA
		_xMstPos1 = x = m->unk74;
		if ((m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0) {
			if (x < m->unk88) {
				t->flags |= 0x80;
				x = m->unk88;
			}
		}
		if (x < m->x2) {
			t->flags |= 0x80;
			x = m->x2;
		}
		_xMstPos2 = x - m->xMstPos;
		m->flags4A = 8;
	} else {
// 41A2FC
		_xMstPos1 = x;
		_xMstPos2 = 0;
		m->flags4A = 0;
	}
}

void Game::executeMstUnk12() {
	const int count = MIN(_res->_mstHdr.unk0x3C, 32);
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < count; ++j) {
			_mstCollisionTable[i][j].unk80 = 0;
		}
	}

	for (int i = 0; i < 32; ++i) {
		MstTaskData *m = &_mstUnkDataTable[i];
		if (!m->m46) {
			continue;
		}
		uint8_t _bl = m->flagsA5;
		if ((_bl & 2) != 0 || ((_bl & 1) != 0 && m->m35 == &_res->_mstUnk35[m->unkC->indexUnk35_20])) {
			if ((_bl & 0xB0) != 0) {
				continue;
			}
			const uint8_t *p = m->unk8 + (m->o16->flags0 & 255) * 28;
			if (p[0] != 0) {
				continue;
			}
			if (m->task->run == &Game::runTask_unk4) {
				continue;
			}
			uint8_t _al = m->flagsA6;
			if (_al & 2) {
				continue;
			}
			assert(m->task->dataPtr == m);
			if ((_al & 8) == 0 || m->unk8[945] != 0) {
				const uint32_t offset = m->unk8 - _res->_mstHeightMapData;
				assert(offset % 948 == 0);
				const uint32_t _ecx = offset / 948;
				_al = m->xMstPos < _mstPosX;
				++_mstCollisionTable[_al][_ecx].unk80;
				_mstCollisionTable[_al][_ecx].m = m;
			}
		}
	}
}

void Game::executeMstUnk13(Task *t) {
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
}

int Game::executeMstOp67Type1(Task *t) {
	t->flags &= ~0x80;
	MstTaskData *m = t->dataPtr;
	m->flagsA5 = (m->flagsA5 & ~2) | 5;
	initMstTaskData(m);
	warning("executeMstOp67Type1 t %p", t);
	// TODO
	return 0;
}

int Game::executeMstOp67Type2(Task *t, int flag) {
	t->flags &= ~0x80;
	MstTaskData *m = t->dataPtr;
	m->flagsA5 = (m->flagsA5 & ~1) | 6;
	initMstTaskData(m);
	const int i = m->unkC->indexUnk36_32;
	assert(i >= 0 && i < _res->_mstHdr.unk0x10);
	MstUnk36 *mstUnk36 = &_res->_mstUnk36[i];
	const int j = mstUnk36->indexUnk49;
	assert(j >= 0 && j < _res->_mstHdr.unk0x40);
	m->m49 = &_res->_mstUnk49[j];
	if (flag != 0) {
		m->unkDC = mstUnk36->unk8 - 1;
	} else {
		m->unkDC = mstUnk36->unk4;
	}
	if (m->unkDC < 0) {
		if (m->m49->count2 == 0) {
			m->unkDC = 0;
		} else {
			uint8_t _al = shuffleFlags(m->unkC8);
			m->unkDC = m->m49->data2[_al];
		}
	}
// 41CC44
	m->unkD4 = m->m49->data1 + m->unkDC * 16;
	m->flags4B = 0xFD;
	m->unkC0 = -1;
	m->unkBC = -1;
	m->flagsA8[2] = 255;
	m->flagsA7 = 255;
	if (executeMstUnk2(m, m->xMstPos, m->yMstPos)) {
		executeMstUnk1(t);
		return 0;
	}
// 41CCA3
	const uint8_t *p = m->unk8;
	if (p[946] & 2) {
		m->unkE4 = 255;
		warning("executeMstOp67Type2 t %p flag %d p[946] %x", t, flag, p[946]);
		// TODO
	}
// 41CE2B
	int32_t _ebp = READ_LE_UINT32(p + 900);
	int32_t _ecx = READ_LE_UINT32(p + 896);
	int r = (_ecx + 3) / 4;
	m->unk64 = _ebp + r;
	m->unk68 = _ecx + _ebp - r;
	if (r != 0) {
		r = _rnd.update() % r;
	}
	m->unk74 = m->unk64 + r;
	m->unk64 += r;
	m->unk68 -= r;
	m->unk7C = m->unk68;
	if (m->flags4B == 0 && m->xMstPos < _mstPosX) {
		m->unk74 = _mstPosX - m->unk68;
		m->unk7C = _mstPosX - m->unk64;
	} else {
// 41CEA1
		m->unk74 = _mstPosX + m->unk64;
		m->unk7C += _mstPosX;
	}
// 41CEB4
	executeMstUnk7(m);
	int _edi = 1;
	if (_xMstPos2 >= m->unkD4[12]) {
		_edi = 0;
	} else {
		while (--m->unkDC >= 0) {
			m->unkD4 = m->m49->data1 + m->unkDC * 16;
			if (_xMstPos2 >= m->unkD4[12]) {
				_edi = 0;
				break;
			}
		}
	}
// 41CF17
	if (_xMstPos2 <= 0) {
		if ((m->unk8[946] & 2) == 0 || _mstUnk11 <= 0) {
			if (m->unk8[946] & 4) {
				uint8_t _dl = m->flagsA8[1];
				if (_dl < _mstRectsCount && _mstRectsTable[_dl].num == m->soundType) {
					_mstRectsTable[_dl].num = 255;
					int i = _dl;
					for (; i < _mstRectsCount; ++i) {
						if (_mstRectsTable[_dl].num != 255) {
							break;
						}
					}
					if (i != _mstRectsCount) {
						_mstRectsCount = _dl;
					}
				}
// 41CF7F
				m->flagsA8[1] = 255;
			}
// 41CF86
			executeMstUnk1(t);
			return 0;
		}
	}
// 41CF99
	warning("executeMstOp67Type2 t %p _edi %d", t, _edi);
	if (_edi) {

	}
// 41D2D2

	// TODO
	return 0;
}

void Game::mstOp67(Task *t, int x1, int x2, int y1, int y2, int screen, int type, int o_flags1, int o_flags2, int arg1C, int arg20, int arg24) {
	debug(kDebug_MONSTER, "mstOp67 pos %d,%d,%d,%d %d %d 0x%x 0x%x %d %d %d", y1, x1, y2, x2, screen, type, o_flags1, o_flags2, arg1C, arg20, arg24);
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
			if (_mstUnkDataTable[i].m46) {
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
			if (!_mstUnkDataTable[i].m46) {
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
		m->m35 = 0;
		m->flagsA6 = 0;

		const int j = _res->_mstUnk42[arg24].indexUnk46[arg1C];
		assert(j >= 0 && j < _res->_mstHdr.unk0x30);
		m->m46 = &_res->_mstUnk46[j];

		m->unk4 = &_res->_mstUnk46[j].data[arg20];
		MstUnk46Unk1 *m1 = &_res->_mstUnk46[j].data[arg20]; // _ecx
		m->unk8 = _res->_mstHeightMapData + m1->indexHeight * 948;

		m->localVars[7] = m1->unkC; // energy

		if (m1->indexUnk51 == kNone) {
			m->flags48 &= ~4;
		}

		const uint8_t *ptr = m->unk8;
		int anim = m1->anim; // READ_LE_UINT16(_ecx + 4)
		o = addLvlObject(ptr[945], x1, y1, objScreen, ptr[944], anim, o_flags1, o_flags2, 0, 0);
		if (!o) {
			m->m46 = 0;
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
		if (_currentLevel == kLvl_lar2 && m->unk8[944] == 26) {
			m->o20 = addLvlObject(ptr[945], x1, y1, objScreen, ptr[944], m1->anim + 1, o_flags1, 0x3001, 0, 0);
			if (!m->o20) {
				warning("mstOp67 addLvlObject kLvl_lar2 o is NULL");
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
			warning("mstOp67 unable to find a free mstObject");
			return;
		}
		if (!o) {
			warning("mstOp67 o is NULL");
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
		warning("mstOp67 mo %p unimplemented", mo);
		// TODO
		mstTaskSetScreenPosition(t);
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
			warning("mstOp67 fail to find a free task");
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
		// TODO
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
		MstUnk46Unk1 *m46unk1 = m->unk4;
		m->unkC = _res->_mstUnk44[m46unk1->indexUnk44].data;

		if (m->unk8[946] & 4) {
			m->flagsA8[0] = 0xFF;
			m->flagsA8[1] = 0xFF;
		}

// 415A89
		mstTaskUpdateScreenPosition(t);

		switch (type) {
		case 1:
			executeMstOp67Type1(t);
			if (!t->codeData) {
				warning("mstOp67 no bytecode for t %p type 1", t);
				removeTask(&_mstTasksList1, t);
				memset(m, 0, sizeof(MstTaskData));
			}
			break;
		case 2:
			if (m) {
				m->flagsA6 |= 1;
			}
			executeMstOp67Type2(t, 0);
			if (!t->codeData) {
				warning("mstOp67 no bytecode for t %p type 2", t);
				removeTask(&_mstTasksList1, t);
				memset(m, 0, sizeof(MstTaskData));
			}
			break;
		default:
			m->flagsA5 = 1;
			if (!updateMstTaskDataPosition(m)) {
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

void Game::mstOp68(Task *t, const uint8_t *p, int a, int b, int c, int d) {
	warning("mstOp68 unimplemented %d,%d,%d,%d", a, b, c, d);
}

int Game::runTask_wait(Task *t) {
	debug(kDebug_MONSTER, "runTask_wait t %p", t);
	--t->delay;
	if (t->delay == 0) {
		t->run = &Game::runTask_default;
		return 0;
	}
	return 1;
}

int Game::runTask_waitResetInput(Task *t) {
	debug(kDebug_MONSTER, "runTask_waitResetInput t %p", t);
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
	debug(kDebug_MONSTER, "runTask_waitFlags t %p", t);
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
	debug(kDebug_MONSTER, "runTask_idle t %p", t);
	return 1;
}

int Game::runTask_mstOp231(Task *t) {
	const MstUnk55 *m = &_res->_mstUnk55[t->tempVar];
	const int a = getTaskFlag(t, m->indexVar1, m->maskVars & 15);
	const int b = getTaskFlag(t, m->indexVar2, m->maskVars >> 4);
	if (!compareOp(m->compare, a, b)) {
		LvlObject *o = 0;
		MstTaskData *m = t->dataPtr;
		if (m) {
			if ((m->flagsA6 & 2) == 0) {
				o = m->o16;
			}
		} else if (t->mstObject) {
			o = t->mstObject->o;
		}
		if (o) {
			o->actionKeyMask = 0;
			o->directionKeyMask = 0;
		}
		t->run = &Game::runTask_default;
		return 0;
	}
	return 1;
}

int Game::runTask_mstOp232(Task *t) {
	warning("runTask_mstOp232 unimplemented");
	t->run = &Game::runTask_default;
	return 0;
}

int Game::runTask_mstOp233(Task *t) {
	debug(kDebug_MONSTER, "runTask_mstOp233 t %p", t);
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

int Game::runTask_mstOp234(Task *t) {
	debug(kDebug_MONSTER, "runTask_mstOp234 t %p", t);
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

int Game::runTask_unk1(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk1 t %p", t);
	if (t->delay == 0) {
		updateMstTaskDataPosition(t->dataPtr);
		executeMstUnk13(t);
		return 0;
	}
	--t->delay;
	return 1;
}

int Game::runTask_unk2(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk2 t %p", t);
	MstTaskData *m = t->dataPtr;
	const uint16_t flags0 = m->o16->flags0;
	if ((flags0 & 0x100) != 0 && (flags0 & 0xFF) == m->flagsA4) {
		updateMstTaskDataPosition(t->dataPtr);
		executeMstUnk13(t);
		return 0;
	}
	return 1;
}

int Game::runTask_unk3(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk3 t %p", t);
	MstTaskData *m = t->dataPtr;
	const uint16_t flags0 = m->o16->flags0;
	if ((flags0 & 0xFF) == m->flagsA4) {
		if (t->delay > 0) {
			t->run = &Game::runTask_unk1;
		} else {
			t->run = &Game::runTask_unk2;
		}
		return (this->*(t->run))(t);
	}
	return 1;
}

int Game::runTask_unk4(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk4 t %p", t);
	MstTaskData *m = t->dataPtr;
	const uint32_t offset = m->unk8 - _res->_mstHeightMapData;
	assert(offset % 948 == 0);
	const uint32_t num = offset / 948;
	if (t->tempVar != num) {
		updateMstTaskDataPosition(m);
		executeMstUnk13(t);
		return 0;
	}
	return 1;
}
