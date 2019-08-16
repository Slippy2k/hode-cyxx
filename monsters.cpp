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

static const uint8_t _mstLut3[] = {
	0x01, 0x03, 0x09, 0x02, 0x08, 0x03, 0x02, 0x01, 0x06, 0x09, 0x02, 0x06, 0x03, 0x04, 0x01, 0x06,
	0x04, 0x02, 0x0C, 0x03, 0x04, 0x0C, 0x06, 0x08, 0x02, 0x0C, 0x08, 0x04, 0x09, 0x06, 0x08, 0x09,
	0x0C, 0x01, 0x04, 0x09, 0x01, 0x08, 0x03, 0x0C
};

static const uint8_t _mstLut4[] = {
	0x01, 0x02, 0x03, 0x05, 0x06, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13, 0x15,
	0x16, 0x17
};

static const uint8_t _mstLut5[] = {
	0x00, 0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x09, 0x09, 0x0A, 0x0B,
	0x0C, 0x0C, 0x0D, 0x0E, 0x0F, 0x0F, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17
};

static const uint8_t _fireflyPosData[] = {
	0, 1, 2, 3, 4, 4, 4, 0xFF, 0, 1, 2, 2, 4, 4, 3, 0xFF,
	0, 1, 1, 2, 3, 2, 3, 0xFF, 0, 1, 1, 2, 3, 3, 3, 0xFF,
	0, 0, 1, 2, 2, 2, 2, 0xFF, 4, 4, 4, 3, 2, 1, 0, 0xFF,
	3, 4, 4, 2, 2, 1, 0, 0xFF, 3, 2, 3, 2, 1, 1, 0, 0xFF,
	3, 3, 3, 2, 1, 1, 0, 0xFF, 2, 2, 2, 2, 1, 0, 0, 0xFF
};

static const uint8_t kUndefinedMonsterByteCode[] = { 0x12, 0x34 };

static uint8_t mstGetFacingDirectionMask(uint8_t a) {
	uint8_t r = 0;
	if (a & 8) { // Andy left facing monster
		r |= 1;
	}
	if (a & 4) { // Andy below the monster
		r |= 2;
	}
	return r;
}

void Game::resetMonsterObject1(MonsterObject1 *m) {
	m->m46 = 0;
	LvlObject *o = m->o16;
	if (o) {
		o->dataPtr = 0;
	}
	for (int i = 0; i < kMaxMonsterObjects2; ++i) {
		MonsterObject2 *mo = &_monsterObjects2Table[i];
		if (mo->m45 && mo->monster1 == m) {
			mo->monster1 = 0;
		}
	}
}

void Game::initMonsterObject2_firefly(MonsterObject2 *m) {
	LvlObject *o = m->o;
	m->x1 = _res->_mstPointOffsets[o->screenNum].xOffset;
	m->y1 = _res->_mstPointOffsets[o->screenNum].yOffset;
	m->x2 = m->x1 + 255;
	m->y2 = m->y1 + 191;
	const uint32_t num = _rnd.update();
	m->hPosIndex = num % 40;
	if (_fireflyPosData[m->hPosIndex] != 0xFF) {
		m->hPosIndex &= ~7;
	}
	m->vPosIndex = m->hPosIndex;
	if (num & 0x80) {
		m->hPosIndex += 40;
	} else {
		m->vPosIndex += 40;
	}
	m->hDir = (num >> 16) & 1;
	m->vDir = (num >> 17) & 1;
}

void Game::resetMonsterObject2(MonsterObject2 *m) {
	m->m45 = 0;
	LvlObject *o = m->o;
	if (o) {
		o->dataPtr = 0;
	}
}

void Game::mstTaskSetScreenPosition(Task *t) {
	MonsterObject2 *m = t->monster2;
	LvlObject *o = m->o;
	m->xPos = o->xPos + o->posTable[7].x;
	m->yPos = o->yPos + o->posTable[7].y;
	m->xMstPos = m->xPos + _res->_mstPointOffsets[o->screenNum].xOffset;
	m->yMstPos = m->yPos + _res->_mstPointOffsets[o->screenNum].yOffset;
}

void Game::initMonsterObject1(MonsterObject1 *m) { // mstResetPosition
	_rnd.resetMst(m->rnd_m35);
	_rnd.resetMst(m->rnd_m49);

	const uint8_t *ptr = m->unk8;
	const int num = (~m->flagsA5) & 1;

	m->x1 = m->m44Unk1->unk2C[0][num] - READ_LE_UINT32(ptr + 904); // x2 ?
	m->x2 = m->m44Unk1->unk2C[1][num] + READ_LE_UINT32(ptr + 904); // x1 ?
	m->y1 = m->m44Unk1->unk2C[2][num] - READ_LE_UINT32(ptr + 908); // y2 ?
	m->y2 = m->m44Unk1->unk2C[3][num] + READ_LE_UINT32(ptr + 908); // y1 ?

	const uint32_t indexUnk35 = m->m44Unk1->indexUnk35_0x24[num];
	m->m35 = (indexUnk35 == kNone) ? 0 : &_res->_mstUnk35[indexUnk35];
}

bool Game::addChasingMonster(MstUnk48 *m48, uint8_t flag) {
	debug(kDebug_MONSTER, "addChasingMonster %d", flag);
	m48->unk5 = flag;
	if (m48->codeData != kNone) {
		Task *t = createTask(_res->_mstCodeData + m48->codeData * 4);
		if (!t) {
			return false;
		}
		while ((this->*(t->run))(t) == 0);
	}
	_m48Num = m48 - _res->_mstUnk48;
	_mstChasingMonstersCount = 0;
	for (int i = 0; i < m48->countUnk12; ++i) {
		MstUnk48Unk12Unk4 *unk4 = m48->unk12[i].data;
		const uint8_t num = unk4->monster1Index;
		if (num != 255) {
			assert(num < kMaxMonsterObjects1);
			unk4->unk19 = flag;
			MonsterObject1 *m = &_monsterObjects1Table[num];
			m->unk18 = unk4;
			m->flags48 |= 0x40;
			m->flagsA5 &= 0x8A;
			m->flagsA5 |= 0x0A;
			initMonsterObject1(m);
			Task *current = _monsterObjects1TasksList;
			Task *t = m->task;
			assert(t);
			while (current) {
				Task *next = current->nextPtr;
				if (current == t) {
					removeTask(&_monsterObjects1TasksList, t);
					appendTask(&_monsterObjects1TasksList, t);
					break;
				}
				current = next;
			}
			const uint32_t codeData = unk4->codeData;
			assert(codeData != kNone);
			resetTask(t, _res->_mstCodeData + codeData * 4);
			++_mstChasingMonstersCount;
		}
	}
	debug(kDebug_MONSTER, "_mstChasingMonstersCount %d", _mstChasingMonstersCount);
	return true;
}

void Game::disableMonsterObject1(MonsterObject1 *m) {
	m->flags48 &= ~0x50;
	m->unk18->monster1Index = 255;
	m->unk18 = 0;
	--_mstChasingMonstersCount;
	if (_mstChasingMonstersCount <= 0) {
		_m48Num = -1;
	}
}

void Game::copyMonsterObject1(Task *t, MonsterObject1 *m, int num) {
	MstUnk46Unk1 *m46unk1 = &m->m46->data[num];
	m->m46Unk1 = m46unk1;
	m->unk8 = _res->_mstMonsterInfos + m46unk1->indexMonsterInfo * kMonsterInfoDataSize;
	if (m46unk1->indexUnk51 == kNone) {
		m->flags48 &= ~4;
	}
	m->m44Unk1 = _res->_mstUnk44[m46unk1->indexUnk44].data;
	mstTaskUpdateScreenPosition(t);
	if (!updateMonsterObject1Position(m)) {
		initMonsterObject1(m);
	}
	if (t->run == &Game::runTask_unk4) {
		t->run = &Game::mstSm_main;
	}
	if ((m->flagsA5 & 8) == 0 && t->run == &Game::runTask_idle) {
		prepareMstTask(t);
	}
}

int Game::mstTaskStopMonsterObject1(Task *t) {
	if (_m48Num == -1) {
		return 0;
	}
	MonsterObject1 *m = t->monster1;

	// The condition matches the disassembly but looks like a bug
	// It was probably meant to check bit 3 instead

	//   mov  dl, [esi+MonsterObject1.flagsA5]
	//   test dl, dl
	//   setz al
	//   test al, 8
	//   jnz  short return_0

	// const uint8_t r = (m->flagsA5 == 0) ? 1 : 0;
	// if ((r & 8) != 0) {
	//   return 0;
	// }

	MstUnk48Unk12Unk4 *m48 = m->unk18;
	if (!m48) {
		return 0;
	}
	const uint32_t codeData = m48->codeData2;
	if (codeData != kNone) {
		resetTask(t, _res->_mstCodeData + codeData * 4);
		disableMonsterObject1(m);
		return 0;
	}
	disableMonsterObject1(m);
	if (m->flagsA5 & 0x80) {
		m->flagsA5 &= ~8;
		const uint32_t codeData = m->m46Unk1->codeData;
		if (codeData != kNone) {
			resetTask(t, _res->_mstCodeData + codeData * 4);
			return 0;
		}
		m->o16->actionKeyMask = 7;
		m->o16->directionKeyMask = 0;
		t->run = &Game::runTask_idle;
		return 0;
	}
	m->flagsA5 = (m->flagsA5 & ~9) | 6;
	if (m->flagsA6 & 2) {
		return 0;
	}
	return executeMstOp67Type2(t, 1);
}

// set lvlObject position from monster position
void Game::updateMstLvlObjectPos(MonsterObject1 *m) {
	LvlObject *o = m->o16;
	o->xPos = m->xPos - o->posTable[7].x;
	o->yPos = m->yPos - o->posTable[7].y;
	m->xMstPos = m->xPos + _res->_mstPointOffsets[o->screenNum].xOffset;
	m->yMstPos = m->yPos + _res->_mstPointOffsets[o->screenNum].yOffset;
}

bool Game::updateMonsterObject1PositionHelper(MonsterObject1 *m) {
	MstUnk46Unk1 *m46unk1 = m->m46Unk1;
	const uint32_t indexUnk44 = m46unk1->indexUnk44;
	assert(indexUnk44 != kNone);
	MstUnk44 *m44 = &_res->_mstUnk44[indexUnk44];
	MstUnk44Unk1 *m44Unk1 = m44->data;

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
	int _ecx = (m34->right - m34->left) / 2 + m34->left;

	int _edi = 0x1000000;
	int var1C = y;

	uint32_t i = 0;
	for (; i < m44->count; ++i) {
		uint32_t indexUnk34 = m44->data[i].indexUnk34_16;
		assert(indexUnk34 != kNone);
		MstUnk34 *m34 = &_res->_mstUnk34[indexUnk34]; // _esi
		if (m34->right < x || m34->left > x || m34->top < y || m34->bottom > y) {
			const int d1 = ABS(x - m34->left);
			if (d1 < _edi) {
				_edi = d1;
				m44Unk1 = &m44->data[i];
				_ecx = m34->left;
			}
			const int d2 = ABS(x - m34->right);
			if (d2 < _edi) {
				_edi = d2;
				m44Unk1 = &m44->data[i];
				_ecx = m34->right;
			}
		} else {
// 41E6FD
			_ecx = x;
			var1C = y;
			m44Unk1 = &m44->data[i];
			break;
		}
	}
// 41E70B
	int _edx = var1C;
	if (i == m44->count) {
		uint32_t indexUnk34 = m44Unk1->indexUnk34_16;
		assert(indexUnk34 != kNone);
		MstUnk34 *m34 = &_res->_mstUnk34[indexUnk34]; // _esi
		if (y <= m34->top) {
			y = (m34->top - m34->bottom) / 2 + m34->bottom;
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
	m->m44Unk1 = m44Unk1;
	initMonsterObject1(m);
	return true;
}

bool Game::updateMonsterObject1Position(MonsterObject1 *m) {
	debug(kDebug_MONSTER, "updateMonsterObject1Position m %p", m);
	const uint8_t screenNum = m->o16->screenNum;
	MstUnk46Unk1 *m46 = m->m46Unk1;
	const uint32_t indexUnk44 = m46->indexUnk44;
	assert(indexUnk44 != kNone);
	MstUnk44 *m44 = &_res->_mstUnk44[indexUnk44];
	// start from screen number
	uint32_t indexUnk44Unk1 = m44->indexUnk44Unk1[screenNum];
	if (indexUnk44Unk1 != kNone) {
		MstUnk44Unk1 *m44Unk1 = &m44->data[indexUnk44Unk1];
		uint32_t indexUnk34 = m44Unk1->indexUnk34_16;
		assert(indexUnk34 != kNone);
		MstUnk34 *m34 = &_res->_mstUnk34[indexUnk34];
		while (m34->left <= m->xMstPos) {
			if (m34->right >= m->xMstPos && m34->bottom <= m->yMstPos && m34->top >= m->yMstPos) {
				if (m->m44Unk1 == m44Unk1) {
					return false;
				}
				m->m44Unk1 = m44Unk1;
				initMonsterObject1(m);
				return true;
			}

			indexUnk44Unk1 = m44Unk1->indexUnk44Unk1_92;
			if (indexUnk44Unk1 == kNone) {
				break;
			}
			m44Unk1 = &m44->data[indexUnk44Unk1];
			indexUnk34 = m44Unk1->indexUnk34_16;
			assert(indexUnk34 != kNone);
			m34 = &_res->_mstUnk34[indexUnk34];
		}
	}
	return updateMonsterObject1PositionHelper(m);
}

int Game::prepareMstTask(Task *t) {
	MonsterObject1 *m = t->monster1;
	assert(m);
	MstUnk35 *mu = m->m35;
	int num = 0;
	if (mu->count2 != 0) {
		const uint8_t code = _rnd.getMstNextNumber(m->rnd_m35);
		num = mu->data2[code];
	}
	const uint32_t codeData = mu->indexCodeData[num];
	assert(codeData != kNone);
	resetTask(t, _res->_mstCodeData + codeData * 4);
	const int counter = m->executeCounter;
	m->executeCounter = _executeMstLogicCounter;
	return m->executeCounter - counter;
}

void Game::mstBoundingBoxClear(MonsterObject1 *m, int dir) {
	assert(dir == 0 || dir == 1);
	uint8_t r = m->flagsA8[dir];
	if (r < _mstBoundingBoxesCount && _mstBoundingBoxesTable[r].monster1Index == m->monster1Index) {
		_mstBoundingBoxesTable[r].monster1Index = 255;
		int i = r;
		for (; i < _mstBoundingBoxesCount; ++i) {
			if (_mstBoundingBoxesTable[i].monster1Index != 255) {
				break;
			}
		}
		if (i == _mstBoundingBoxesCount) {
			_mstBoundingBoxesCount = r;
		}
	}
	m->flagsA8[dir] = 255;
}

int Game::mstBoundingBoxCollides1(int num, int x1, int y1, int x2, int y2) {
	for (int i = 0; i < _mstBoundingBoxesCount; ++i) {
		const MstBoundingBox *p = &_mstBoundingBoxesTable[i];
		if (p->monster1Index != 0xFF && num != p->monster1Index) {
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

int Game::mstBoundingBoxUpdate(int num, int a, int x1, int y1, int x2, int y2) {
	if (num == 0xFF) {
		MstBoundingBox *p = &_mstBoundingBoxesTable[0];
		for (int i = 0; i < _mstBoundingBoxesCount; ++i) {
			if (p->monster1Index == 0xFF) {
				num = i;
				break;
			}
		}
		p->x1 = x1;
		p->y1 = y1;
		p->x2 = x2;
		p->y2 = y2;
		p->monster1Index = a;
		if (num != _mstBoundingBoxesCount) {
			++_mstBoundingBoxesCount;
		}
	} else if (num < _mstBoundingBoxesCount) {
		MstBoundingBox *p = &_mstBoundingBoxesTable[num];
		if (p->monster1Index == a) {
			p->x1 = x1;
			p->y1 = y1;
			p->x2 = x2;
			p->y2 = y2;
		}
	}
	return num;
}

int Game::mstBoundingBoxCollides2(int num, int x1, int y1, int x2, int y2) {
	for (int i = 0; i < _mstBoundingBoxesCount; ++i) {
		MstBoundingBox *p = &_mstBoundingBoxesTable[i];
		if (p->monster1Index == 0xFF || p->monster1Index == num) {
			continue;
		}
		if (p->monster1Index == 0xFE) {
			if (_monsterObjects1Table[num].unk8[944] != 15) {
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

int Game::getMstDistance(int y, const MovingOpcodeState *p) const {
	switch (p->unk24) {
	case 0:
		if (p->boundingBox.x1 <= _mstTemp_x2 && p->boundingBox.y2 >= _mstTemp_y1 && p->boundingBox.y1 <= _mstTemp_y2) {
			const int dx = _mstTemp_x1 - p->boundingBox.x2;
			if (dx <= 0) {
				return 0;
			}
			return (p->unk40 == 3) ? 0 : dx / p->width;
		}
		break;
	case 1:
		{
			const int dx = p->boundingBox.x2 - _mstTemp_x1;
			if (dx >= 0) {
				const int dy = p->boundingBox.y1 - dx * 2 / 3;
				if (dy <= _mstTemp_y2) {
					const int dx2 = p->boundingBox.x1 - _mstTemp_x2;
					if (dx2 <= 0) {
						return (p->boundingBox.y2 >= _mstTemp_y1) ? 0 : -1;
					}
					const int dy2 = p->boundingBox.y2 + dx2 * 2 / 3;
					if (dy2 > _mstTemp_y1) {
						return (p->unk40 == 3) ? 0 : dx2 / p->width;
					}
				}
			}
		}
		break;
	case 2: {
			const int dx = _mstTemp_x1 - p->boundingBox.x2;
			if (dx >= 0) {
				const int dy = p->boundingBox.y2 + dx * 2 / 3;
				if (dy >= _mstTemp_x1) {
					const int dx2 = p->boundingBox.x1 - _mstTemp_x2;
					if (dx2 <= 0) {
						return (p->boundingBox.y1 <= _mstTemp_y2) ? 0 : -1;
					}
					const int dy2 = p->boundingBox.y1 + dx2 * 2 / 3;
					if (dy2 <= _mstTemp_y2) {
						return (p->unk40 == 3) ? 0 : dx2 / p->width;
					}
				}
			}
		}
		break;
	case 3: {
			const int dx = _mstTemp_x2 - p->boundingBox.x1;
			if (dx >= 0) {
				const int dy = p->boundingBox.y1 - dx * 2 / 3;
				if (dy <= _mstTemp_y2) {
					const int dx2 = _mstTemp_x1 - p->boundingBox.x2;
					if (dx2 <= 0) {
						return (p->boundingBox.y2 >= _mstTemp_y1) ? 0 : -1;
					}
					const int dy2 =  p->boundingBox.y2 - dx2 * 2 / 3;
					if (dy2 >= _mstTemp_y1) {
						return (p->unk40 == 3) ? 0 : dx2 / p->width;
					}
				}
			}
		}
		break;
	case 4:
		{
			const int dx = _mstTemp_x2 - p->boundingBox.x1;
			if (dx >= 0) {
				const int dy = dx * 2 / 3 + p->boundingBox.y2;
				if (dy >= _mstTemp_y1) {
					const int dx2 = _mstTemp_x1 - p->boundingBox.x2;
					if (dx2 <= 0) {
						return (p->boundingBox.y1 <= _mstTemp_y2) ? 0 : -1;
					}
					const int dy2 = dx2 * 2 / 3 + + p->boundingBox.y1;
					if (dy2 <= _mstTemp_y2) {
						return (p->unk40 == 3) ? 0 : dx2 / p->width;
					}
				}
			}
		}
		break;
	case 5:
		if (p->boundingBox.x2 >= _mstTemp_x1 && p->boundingBox.y2 >= _mstTemp_y1 && p->boundingBox.y1 <= _mstTemp_y2) {
			const int dx = p->boundingBox.x1 - _mstTemp_x2;
			if (dx <= 0) {
				return 0;
			}
			return (p->unk40 == 3) ? 0 : dx / p->width;
		}
		break;
	case 6:
		if (p->boundingBox.y2 >= _mstTemp_y1 && p->boundingBox.x2 >= _mstTemp_x1 && p->boundingBox.x1 <= _mstTemp_x2) {
			const int dy = p->boundingBox.y1 - _mstTemp_y2;
			if (dy <= 0) {
				return 0;
			}
			return (p->unk40 == 3) ? 0 : dy / p->height;
		}
		break;
	case 7:
		if (p->boundingBox.y1 <= _mstTemp_y2 && p->boundingBox.x2 >= _mstTemp_x1 && p->boundingBox.x1 <= _mstTemp_x2) {
			const int dy = _mstTemp_y1 - p->boundingBox.y2;
			if (dy <= 0) {
				return 0;
			}
			return (p->unk40 == 3) ? 0 : dy / p->height;
		}
		break;
	case 128: // 8
		if (p->boundingBox.x1 <= _mstTemp_x2 && y >= _mstTemp_y1 && y - _mstTemp_y2 <= 9) {
			const int dx = _mstTemp_x1 - p->boundingBox.x2;
			if (dx <= 0) {
				return 0;
			}
			return dx / p->width;
		}
		break;
	case 133: // 9
		if (p->boundingBox.x2 >= _mstTemp_x1 && y >= _mstTemp_y1 && y - _mstTemp_y2 <= 9) {
			const int dx = _mstTemp_x2 - p->boundingBox.x1;
			if (dx <= 0) {
				return 0;
			}
			return dx / p->width;
		}
		break;
	}
	return -1;
}

void Game::mstTaskUpdateScreenPosition(Task *t) {
	MonsterObject1 *m = t->monster1;
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
			m->flagsA8[0] = mstBoundingBoxUpdate(m->flagsA8[0], m->monster1Index, _mstTemp_x1, _mstTemp_y1, _mstTemp_x2, _mstTemp_y2);
		} else {
			mstBoundingBoxClear(m, 0);
		}
	}
// 40ECBD
	m->xDelta = _mstAndyLevelPosX - m->xMstPos;
	if (m->xDelta < 0) {
		m->xDelta = -m->xDelta;
		m->flags49 = 8;
	} else {
		m->flags49 = 2;
	}
	m->yDelta = _mstAndyLevelPosY - m->yMstPos;
	if (m->yDelta < 0) {
		m->yDelta = -m->yDelta;
		m->flags49 |= 1;
	} else {
		m->flags49 |= 4;
	}
	m->collideDistance = -1;
	m->collidePtr = 0;
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
					if (m->collideDistance == -1 || m->collideDistance > res || (m->collideDistance == 0 && res == 0 && (m->collidePtr->unk40 & 1) == 0 && p->unk40 == 2)) {
						m->collidePtr = p;
						m->collideDistance = res;
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
	// _mstLut initialization
	resetMstCode();
}

void Game::resetMstCode() {
	if (_mstLogicDisabled) {
		return;
	}
	_mstFlags = 0;
	for (int i = 0; i < kMaxMonsterObjects1; ++i) {
		resetMonsterObject1(&_monsterObjects1Table[i]);
	}
	for (int i = 0; i < kMaxMonsterObjects2; ++i) {
		resetMonsterObject2(&_monsterObjects2Table[i]);
	}
	clearLvlObjectsList1();
	for (int i = 0; i < _res->_mstHdr.screenAreaCodesCount; ++i) {
		_res->_mstScreenAreaCodes[i].unk0x1D = 1;
	}
	_rnd.initMstTable();
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
	for (int i = 0; i < _res->_mstHdr.unk0x24; ++i) {
		const int count = _res->_mstUnk43[i].count2;
		if (count != 0) {
			uint8_t *ptr = _res->_mstUnk43[i].data2;
			for (int j = 0; j < count; ++j) {
				ptr[j] &= ~0x80;
			}
			for (int j = 0; j < count; ++j) {
				const int index1 = _rnd.update() % count;
				const int index2 = _rnd.update() % count;
				SWAP(ptr[index1], ptr[index2]);
			}
		}
	}
	_mstOp67_x1 = -256;
	_mstOp67_x2 = -256;
	memset(_monsterObjects1Table, 0, sizeof(_monsterObjects1Table));
	memset(_monsterObjects2Table, 0, sizeof(_monsterObjects2Table));
	memset(_mstVars, 0, sizeof(_mstVars));
	memset(_tasksTable, 0, sizeof(_tasksTable));
	_m43Num3 = _m43Num1 = _m43Num2 = _m48Num = -1;
	_executeMstLogicPrevCounter = _executeMstLogicCounter = 0;
	_mstUnk8 = 0;
	_specialAnimFlag = false;
	_mstAndyRectNum = 255;
	_mstBoundingBoxesCount = 0;
	_mstOp67_y1 = 0;
	_mstOp67_y2 = 0;
	_mstOp67_screenNum = 0xFF;
	_mstOp68_x1 = 256;
	_mstOp68_x2 = 256;
	_mstOp68_y1 = 0;
	_mstOp68_y2 = 0;
	_mstOp68_screenNum = 255;
	_mstHelper1TestValue = 0;
	_mstHelper1TestMask = 0xFFFFFFFF;
	_mstAndyVarMask = 0;
	_tasksList = 0;
	_monsterObjects1TasksList = 0;
	_monsterObjects2TasksList = 0;
	// _mstTasksList3 = 0;
	// _mstTasksList4 = 0;
	if (_res->_mstTickCodeData != kNone) {
		_mstVars[31] = _mstTickDelay = _res->_mstTickDelay;
	} else {
		_mstVars[31] = -1;
	}
	_mstVars[30] = 0x20;
	for (int i = 0; i < kMaxMonsterObjects1; ++i) {
		_monsterObjects1Table[i].monster1Index = i;
	}
	for (int i = 0; i < kMaxMonsterObjects2; ++i) {
		_monsterObjects2Table[i].monster2Index = i;
	}
	mstUpdateRefPos();
	_mstAndyLevelPrevPosX = _mstAndyLevelPosX;
	_mstAndyLevelPrevPosY = _mstAndyLevelPosY;
}

void Game::startMstCode() {
	mstUpdateRefPos();
	_mstAndyLevelPrevPosX = _mstAndyLevelPosX;
	_mstAndyLevelPrevPosY = _mstAndyLevelPosY;
	int offset = 0;
	for (int i = 0; i < _res->_mstHdr.unk0x3C; ++i) {
		offset += kMonsterInfoDataSize;
		const uint32_t unk30 = READ_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x30]); // 900
		const uint32_t unk34 = READ_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x34]); // 896

		const uint32_t unk20 = _mstAndyLevelPosX - unk30;
		const uint32_t unk1C = _mstAndyLevelPosX + unk30;
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x20], unk20);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x1C], unk1C);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x24], unk20 - unk34);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x18], unk1C + unk34);

		const uint32_t unk10 = _mstAndyLevelPosY - unk30;
		const uint32_t unk0C = _mstAndyLevelPosY + unk30;
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x10], unk10);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x0C], unk0C);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x14], unk10 - unk34);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x08], unk0C + unk34);
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
	if ((_mstHelper1TestValue & _mstHelper1TestMask) != 0) {
		_mstHelper1Count = 0;
		executeMstCodeHelper1();
		_mstHelper1TestValue = 0;
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
				t->state = 1;
			}
		}
	}
	MstScreenAreaCode *msac;
	while ((msac = _res->findMstCodeForPos(_currentScreen, _mstAndyLevelPosX, _mstAndyLevelPosY)) != 0) {
		debug(kDebug_MONSTER, "trigger for %d,%d", _mstAndyLevelPosX, _mstAndyLevelPosY);
		_res->flagMstCodeForPos(msac->unk0x1C, 0);
		assert(msac->codeData != kNone);
		createTask(_res->_mstCodeData + msac->codeData * 4);
	}
	if (_mstAndyCurrentScreenNum != _currentScreen) {
		_mstAndyCurrentScreenNum = _currentScreen;
	}
	for (Task *t = _tasksList; t; t = t->nextPtr) {
		_runTaskOpcodesCount = 0;
		while ((this->*(t->run))(t) == 0);
	}
// 419C30
	for (int i = 0; i < _mstMovingStateCount; ++i) {
		MovingOpcodeState *p = &_mstMovingState[i];
		MonsterObject1 *m = p->m;
		if (!m) {
			continue;
		}
		const int energy = m->localVars[7];
		ShootLvlObjectData *dat = p->unk28;
		if (dat) {
			if (energy > 0) {
				if ((p->unk40 & 2) == 0) {
					m->localVars[7] -= 4;
					if (m->localVars[7] < 0) {
						m->localVars[7] = 0;
					}
				} else {
// 419C75
					--m->localVars[7];
				}
// 419C79
				dat->unk3 = 0x80;
				dat->x2 = p->unk34;
				dat->y2 = p->unk38;
				dat->o = m->o16;
			} else if (energy == -2) {
// 419CA1
				dat->unk3 = 0x80;
				dat->x2 = p->unk34;
				dat->y2 = p->unk38;
			}
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
	for (Task *t = _monsterObjects1TasksList; t; t = t->nextPtr) {
		_runTaskOpcodesCount = 0;
		if (mstUpdateTaskMonsterObject1(t) == 0) {
			while ((this->*(t->run))(t) == 0);
		}
	}
	for (Task *t = _monsterObjects2TasksList; t; t = t->nextPtr) {
		_runTaskOpcodesCount = 0;
		if (mstUpdateTaskMonsterObject2(t) == 0) {
			while ((this->*(t->run))(t) == 0);
		}
	}
}

int Game::executeMstUnk18(MstUnk44 *m44, MstUnk44Unk1 *m44Unk1, int num, int index) {
	if (m44Unk1->unk2C[num][index] < 0) {
		const uint32_t indexUnk34 = m44Unk1->indexUnk34_16;
		assert(indexUnk34 != kNone && indexUnk34 < (uint32_t)_res->_mstHdr.unk0x08);
		MstUnk34 *m34 = &_res->_mstUnk34[indexUnk34];
		uint32_t mask;
		uint8_t flags = m34->unk10[num];
		if (flags) {
			flags &= 0x7F;
			mask = _mstAndyVarMask & (1 << flags);
		} else {
			mask = flags & (1 << index);
		}
		if (mask) {
			switch (num) { // ((int *)&m34)[num]
			case 0:
				m44Unk1->unk2C[num][index] = m34->right;
				break;
			case 1:
				m44Unk1->unk2C[num][index] = m34->left;
				break;
			case 2:
				m44Unk1->unk2C[num][index] = m34->top;
				break;
			case 3:
				m44Unk1->unk2C[num][index] = m34->bottom;
				break;
			}
		} else {
			const uint32_t indexUnk44Unk1 = m44Unk1->indexUnk44Unk1_76[num];
			assert(indexUnk44Unk1 != kNone && indexUnk44Unk1 < m44->count);
			m44Unk1->unk2C[num][index] = executeMstUnk18(m44, &m44->data[indexUnk44Unk1], num, index);
		}
	}
	return m44Unk1->unk2C[num][index];
}

void Game::executeMstCodeHelper1() {
	for (int i = 0; i < _res->_mstHdr.unk0x28; ++i) {
		MstUnk44 *m44 = &_res->_mstUnk44[i];
		if (m44->mask & _mstHelper1TestValue) {
			++_mstHelper1Count;
			for (uint32_t j = 0; j < m44->count; ++j) {
				for (int k = 0; k < 2; ++k) {
					m44->data[j].unk2C[0][k] = -1;
					m44->data[j].unk2C[1][k] = -1;
					m44->data[j].unk2C[2][k] = -1;
					m44->data[j].unk2C[3][k] = -1;
				}
			}
			warning("executeMstCodeHelper1 417691 unimplemented");
			// TODO
			for (uint32_t j = 0; j < m44->count; ++j) {
				// MstUnk44Unk1 *m44Unk1 = &m44->data[j];
				for (int k = 0; k < 4; ++k) { // unk2C[0] .. unk2C[3]
				}
			}
// 4177D7
			executeMstUnk16(m44, 0);
			executeMstUnk16(m44, 1);
		}
	}
	if (_mstHelper1Count != 0) {
		for (int i = 0; i < kMaxMonsterObjects1; ++i) {
			MonsterObject1 *m = &_monsterObjects1Table[i];
			if (!m->m46) {
				continue;
			}
			MstUnk44Unk1 *m44Unk1 = m->m44Unk1;
			const int num = (~m->flagsA5) & 1;
			m->x1 = m44Unk1->unk2C[0][num] - (int32_t)READ_LE_UINT32(m->unk8 + 904);
			m->x2 = m44Unk1->unk2C[1][num] + (int32_t)READ_LE_UINT32(m->unk8 + 904);
			m->y1 = m44Unk1->unk2C[2][num] - (int32_t)READ_LE_UINT32(m->unk8 + 908);
			m->y2 = m44Unk1->unk2C[3][num] + (int32_t)READ_LE_UINT32(m->unk8 + 908);
		}
	}
}

void Game::executeMstCodeHelper2() {
	mstUpdateRefPos();
	updateMstHeightMapData();
	for (Task *t = _monsterObjects1TasksList; t; t = t->nextPtr) {
		mstTaskUpdateScreenPosition(t);
	}
}

void Game::mstLvlObjectSetActionDirection(LvlObject *o, const uint8_t *ptr, uint8_t mask1, uint8_t dirMask2) {
	o->actionKeyMask = ptr[1];
	uint8_t _al = mask1 & 15;
	o->directionKeyMask = _al;
	MonsterObject1 *m = (MonsterObject1 *)getLvlObjectDataPtr(o, kObjectDataTypeMonster1);
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
					if (_edi == 160 && (_mstLut1[_bl] & 1) != 0) {
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
			o->directionKeyMask = _al | dirMask2;
			if ((m->unk8[946] & 2) != 0 && (_mstLut1[_al] & 1) != 0) {
				if (m->xDelta >= m->yDelta) {
					o->directionKeyMask &= ~5;
				} else {
					o->directionKeyMask &= ~0xA;
				}
			}
			break;
		case 224:
			o->directionKeyMask = m->unkF8 | _al;
			break;
		default:
			o->directionKeyMask = _al | dirMask2;
			break;
		}
	}
// 40E3B3
	o->directionKeyMask &= ptr[2];
	if ((mask1 & 0xE0) == 0x40) {
		o->directionKeyMask ^= 0xA;
	}
}

void Game::executeMstUnk4(MonsterObject1 *m) {
	int var1C = 0;
	int var18 = 0;
	int _ebp, _edi, _esi, _eax;
	if (m->flags4B == 0xFD) {
		MstUnk49 *m49 = m->m49;
		if (m->x1 > _mstAndyLevelPosX + m49->unk14 - m->unk68 && m->x2 < _mstAndyLevelPosX + m->unk68 - m49->unk14 && m->y1 > _mstAndyLevelPosY + m49->unk15 - m->unk70 && m->y2 < _mstAndyLevelPosY + m49->unk15 + m->unk70) {
			var18 = _mstAndyLevelPosX + m49->unk14 + m->unk64;
			if (m->x1 < var18) {
				if (m->x2 > _mstAndyLevelPosX - m->unk64 - m49->unk14 && m->y1 < _mstAndyLevelPosY + m49->unk15 + m->unk6C && m->y2 > _mstAndyLevelPosY - m->unk6C - m49->unk15) {
					goto l41B2DC;
				}
			}
// 41B170
			int tmp = var18;
			var18 = 0;
			_ebp = 0x40000000;
			if (m->x1 >= tmp) {
				if (m->xMstPos > _mstAndyLevelPosX + m->unk68) {
					_ebp = m->xMstPos - m->unk64 - _mstAndyLevelPosX;
// 41B191
					if (_ebp >= 0 && _ebp < m49->unk14) {
						_ebp = 0x40000000;
					} else {
						var18 = 1;
					}
// 41B1A2
				} else if (m->xMstPos >= _mstAndyLevelPosX + m->unk64) {
					_ebp = -1;
					var18 = 1;
				} else {
					_ebp = m->unk68 - m->xMstPos + _mstAndyLevelPosX;
					if (_ebp < 0 || _ebp >= m49->unk14) {
						var18 = 1;
					}
					if (_ebp >= 0 && _ebp < m49->unk14) {
						_ebp = 0x40000000;
					} else {
						var18 = 1;
					}
				}
			}
// 41B1BE
			_edi = 0x40000000;
			if (m->x2 <= _mstAndyLevelPosX - m->unk64 - m49->unk14) {
				if (m->xMstPos > _mstAndyLevelPosX - m->unk64) {
					_edi = m->xMstPos - _mstAndyLevelPosX  + m->unk68;
					if (_edi >= 0 && _edi < m49->unk14) {
						_edi = 0x40000000;
					} else {
						var18 |= 2;
					}
// 41B1F8
				} else if (m->xMstPos >= _mstAndyLevelPosX - m->unk68) {
					_edi = -1;
					var18 |= 2;
				} else {
					_edi = _mstAndyLevelPosX - m->unk64 - m->xMstPos;
					if (_edi >= 0 && _edi < m49->unk14) {
						_edi = 0x40000000;
					} else {
						var18 |= 2;
					}
				}
			}
// 41B21A
			_esi = 0x40000000;
			if (m->y1 >= _mstAndyLevelPosY + m->unk6C + m49->unk15) {
				if (m->yMstPos > _mstAndyLevelPosY + m->unk70) {
					_esi = m->yMstPos - m->unk6C - _mstAndyLevelPosY;
					if (_esi >= 0 && _esi < m49->unk15) {
						_esi = 0x40000000;
					} else {
						var18 |= 4;
					}
// 41B252
				} else if (m->yMstPos >= _mstAndyLevelPosY + m->unk6C) {
					_esi = -1;
					var18 |= 4;
				} else {
					_esi = m->unk70 - m->yMstPos - _mstAndyLevelPosY;
					if (_esi >= 0 && _esi < m49->unk15) {
						_esi = 0x40000000;
					} else {
						var18 |= 4;
					}
				}
			}
// 41B276
			_eax = 0x40000000;
			if (m->y2 <= _mstAndyLevelPosY - m->unk6C - m49->unk15) {
				if (m->yMstPos > _mstAndyLevelPosY - m->unk6C) {
					_eax = m->yMstPos - _mstAndyLevelPosY + m->unk70;
					if (_eax >= 0 && _eax < m49->unk15) {
						_eax = 0x40000000;
					} else {
						var18 |= 8;
					}
				} else if (m->yMstPos >= _mstAndyLevelPosY - m->unk70) {
// 41B2B0
					_eax = -1;
					var18 |= 8;
				} else {
					_eax = _mstAndyLevelPosY - m->unk6C - m->yMstPos;
					if (_eax >= 0 && _eax < m49->unk15) {
						_eax = 0x40000000;
					} else {
						var18 |= 8;
					}
				}
			}
// 41B2D2
			if (var18 == 0) {
				var18 = 15;
			}
		} else {
// 41B2DC
l41B2DC:
			_ebp = ABS(m->xMstPos - _mstAndyLevelPosX + m49->unk14 - m->unk68);
			_edi = ABS(m->xMstPos - _mstAndyLevelPosX - m49->unk14 + m->unk68);
			_esi = ABS(m->yMstPos - _mstAndyLevelPosY + m49->unk15 - m->unk70);
			_eax = ABS(m->yMstPos - _mstAndyLevelPosX - m49->unk15 + m->unk70);
// 41B338
			var18 = 15;
		}
// 41B33E
		if (var18 == m->unkE4) {
			var1C = m->unkE5;
		} else {
// 41B352
			switch (var18 & 3) {
			case 0:
				var1C = (_esi > _eax) ? 33 : 32;
				break;
			case 2:
				if (_edi <= _eax && _edi <= _esi) {
					var1C = 18;
				} else {
					var1C = (_esi >= _eax) ? 33 : 32;
				}
				break;
			case 3:
				if (_ebp > _edi) {
// 41B38E
					if (_edi <= _eax && _edi <= _esi) {
						var1C = 18;
					} else {
						var1C = (_esi >= _eax) ? 33 : 32;
					}
					break;
				}
				// fall-through
			case 1:
				if (_ebp <= _eax && _ebp <= _esi) {
					var1C = 2;
				} else {
					var1C = (_esi >= _eax) ? 33 : 32;
				}
				break;
			}
// 41B3A9
			m->unkE4 = var18;
			m->unkE5 = var1C;
		}
	} else {
// 41B3BB
		var1C = 0;
	}
// 41B3C0
	switch (var1C & 0xF0) {
	case 0x00:
		m->unk74 = m->unk64;
		m->unk7C = m->unk68;
		break;
	case 0x10:
		m->unk74 = -m->unk68;
		m->unk7C = -m->unk64;
		break;
	case 0x20:
		m->unk74 = -m->unk68;
		m->unk7C =  m->unk68;
		break;
	}
// 41B404
	switch (var1C & 0xF) {
	case 0:
		m->unk78 = m->unk6C;
		m->unk80 = m->unk70;
		break;
	case 1:
		m->unk78 = -m->unk70;
		m->unk80 = -m->unk6C;
		break;
	case 2:
		m->unk80 =  m->unk70;
		m->unk78 = -m->unk70;
		break;
	}
// 41B442
	m->unk74 += _mstAndyLevelPosX;
	m->unk7C += _mstAndyLevelPosX;
	m->unk78 += _mstAndyLevelPosY;
	m->unk80 += _mstAndyLevelPosY;
	const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	if ((ptr[2] & 0x5) == 0) {
		m->unk6C = m->unk78 = m->unk70 = m->unk80 = m->yMstPos;
	}
	if ((ptr[2] & 0xA) == 0) {
		m->unk64 = m->unk74 = m->unk68 = m->unk7C = m->xMstPos;
	}
}

void Game::mstSetVerticalHorizontalBounds(MonsterObject1 *m) {
	MstUnk44Unk1 *m44Unk1 = m->m44Unk1;
	const uint8_t *p = m->unk8;
	assert(m44Unk1->indexUnk34_16 != kNone);
	MstUnk34 *m34 = &_res->_mstUnk34[m44Unk1->indexUnk34_16];
	const int w = READ_LE_UINT32(p + 904);
	const int h = READ_LE_UINT32(p + 908);
	debug(kDebug_MONSTER, "mstSetVerticalHorizontalBounds m %p pos %d,%d [%d,%d,%d,%d]", m, m->xMstPos, m->yMstPos, m34->left,  m34->right, m34->bottom, m34->top);
	if (m->xMstPos < m34->left - w || m->xMstPos > m34->right + w || m->yMstPos < m34->bottom - h || m->yMstPos > m34->top + h) {
		updateMonsterObject1Position(m);
		m->unkC0 = -1;
		m->unkBC = -1;
	}
	m->task->flags &= ~0x80;
	if (m->xMstPos < m->unk74) {
		int x = m->unk7C;
		_xMstPos1 = x;
		if (x > m->x1) {
			x = m->x1;
			m->task->flags |= 0x80;
		}
		m->flags4A = 2;
		_xMstPos2 = x - m->xMstPos;
	} else if (m->xMstPos > m->unk7C) {
		int x = m->unk74;
		_xMstPos1 = x;
		if (x < m->x2) {
			x = m->x2;
			m->task->flags |= 0x80;
		}
		m->flags4A = 8;
		_xMstPos2 = m->xMstPos - x;
	} else {
		_xMstPos1 = m->xMstPos;
		m->flags4A = 0;
		_xMstPos2 = 0;
	}
	if (m->yMstPos < m->unk78) {
		int y = m->unk80;
		_yMstPos1 = y;
		if (y > m->y1) {
			y = m->y1;
			m->task->flags |= 0x80;
		}
		_yMstPos2 = y - m->yMstPos;
		m->flags4A |= 4;
	} else if (m->yMstPos > m->unk80) {
		int y = m->unk78;
		_yMstPos1 = y;
		if (y < m->y2) {
			y = m->y2;
			m->task->flags |= 0x80;
		}
		_yMstPos2 = m->yMstPos - y;
		m->flags4A |= 1;
	} else {
		_yMstPos1 = m->yMstPos;
		_yMstPos2 = 0;
	}
	if (m->flags4A == 0) {
		return;
	}
// 41A759
	int _edi = 0;
	MstUnk46Unk1 *m46u1 = m->m46Unk1;
	assert(m46u1->indexUnk44 != kNone);
	MstUnk44 *m44 = &_res->_mstUnk44[m46u1->indexUnk44];
	uint8_t _cl = m->flagsA8[2];
	if (m->unkBC != _xMstPos1 || m->unkC0 != _yMstPos1) {
		if (_cl < m44->count) {
			MstUnk44Unk1 *m44Unk1 = &m44->data[_cl];
			assert(m44Unk1->indexUnk34_16 != kNone);
			MstUnk34 *m34 = &_res->_mstUnk34[m44Unk1->indexUnk34_16];
			if (_xMstPos1 < m34->left || _xMstPos1 > m34->right || _yMstPos1 < m34->bottom || _yMstPos1 > m34->top) {
				_edi = -1;
			} else {
				goto l41A879;
			}
		}
// 41A7AD
		int _al = executeMstUnk15(m, m44, _xMstPos1, _yMstPos1);
		if (_al < 0) {
			_xMstPos1 = _xMstPos3;
			_yMstPos1 = _yMstPos3;
			_cl = _edi - _al;
			m->unk74 = m->unk7C = _xMstPos1;
			m->unk78 = m->unk80 = _yMstPos1;
			_xMstPos2 = ABS(m->xMstPos - _xMstPos1);
			_yMstPos2 = ABS(m->yMstPos - _yMstPos1);
			if (m->xMstPos < m->unk74) {
				m->flags4A = 2;
			} else if (m->xMstPos > m->unk7C) {
				m->flags4A = 8;
			}
			if (m->yMstPos < m->unk78) {
				m->flags4A |= 4;
			} else if (m->yMstPos > m->unk80) {
				m->flags4A |= 1;
			}
		} else {
			_cl = _al;
		}
// 41A85C
		m->flagsA8[2] = _cl;
		m->unkBC = -1;
		m->unkC0 = -1;
	}
// 41A879
l41A879:
	if (_cl >= m44->count || m->m44Unk1 == &m44->data[_cl]) {
		m->flagsA7 = 0xFF;
		return;
	}
	_edi = (~m->flagsA5) & 1;
	if (m->unkBC == _xMstPos1 && m->unkC0 == _yMstPos1) {
		if (m->flagsA7 == 255) {
			return;
		}
		m->flags4A = m->flagsA7;
	} else {
// 41A8DD
		m->unkBC = _xMstPos1;
		m->unkC0 = _yMstPos1;
		uint32_t indexUnk44 = m->m46Unk1->indexUnk44;
		assert(indexUnk44 != kNone && indexUnk44 < (uint32_t)_res->_mstHdr.unk0x28);
		MstUnk44 *m44 = &_res->_mstUnk44[indexUnk44];
		uint8_t var1D = m->m44Unk1->unk60[_edi][_cl];
		if (var1D != 0) {
			MstUnk44Unk1 *m44Unk1 = m->m44Unk1;
			const uint8_t *p = m->unk8;
			const int w = (int32_t)READ_LE_UINT32(p + 904);
			const int h = (int32_t)READ_LE_UINT32(p + 908);
			while (_xMstPos1 >= m44Unk1->unk2C[1][_edi] + w) {
				if (_xMstPos1 > m44Unk1->unk2C[0][_edi] - w) {
					break;
				}
				if (_yMstPos1 < m44Unk1->unk2C[3][_edi] + h) {
					break;
				}
				if (_yMstPos1 > m44Unk1->unk2C[2][_edi] - h) {
					break;
				}
				int var1C;
				switch (m44Unk1->unk60[_edi][_cl]) {
				case 2:
					var1C = 0;
					break;
				case 4:
					var1C = 2;
					break;
				case 8:
					var1C = 1;
					break;
				default:
					var1C = 3;
					break;
				}
				const uint32_t indexUnk44Unk1 = m44Unk1->indexUnk44Unk1_76[var1C];
				assert(indexUnk44Unk1 != kNone);
				m44Unk1 = &m44->data[indexUnk44Unk1];
				if (m44Unk1 == &m44->data[_cl]) {
					m->flagsA7 = 0xFF;
					return;
				}
			}
// 41A9E4
			m->flags4A = var1D;
		}
// 41A9EB
		m->flagsA7 = m->flags4A;
	}
// 41A9F4
	if (m->flags4A & 8) {
		_xMstPos2 = m->xMstPos - m->m44Unk1->unk2C[1][_edi] - (int32_t)READ_LE_UINT32(m->unk8 + 904);
	} else if (m->flags4A & 2) {
		_xMstPos2 = m->m44Unk1->unk2C[0][_edi] - (int32_t)READ_LE_UINT32(m->unk8 + 904) - m->xMstPos;
	} else {
		_xMstPos2 = 0;
	}
	if (m->flags4A & 1) {
		_yMstPos2 = m->yMstPos - m->m44Unk1->unk2C[3][_edi] - (int32_t)READ_LE_UINT32(m->unk8 + 908);
	} else if (m->flags4A & 4) {
		_yMstPos2 = m->m44Unk1->unk2C[2][_edi] - (int32_t)READ_LE_UINT32(m->unk8 + 908) - m->yMstPos;
	} else {
		_yMstPos2 = 0;
	}
}

bool Game::executeMstUnk6(MonsterObject1 *m) {
	if (_mstLut1[m->flags4A] & 1) {
		if (_xMstPos2 < m->m49->unk14) {
			m->flags4A &= ~0xA;
		}
		if (_yMstPos2 < m->m49->unk15) {
			m->flags4A &= ~0x5;
		}
	}
	if (_mstLut1[m->flags4A] & 1) {
		while (--m->indexUnk49Unk1 >= 0) {
			m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
			if (_xMstPos2 >= m->m49Unk1->unkE && _yMstPos2 >= m->m49Unk1->unkF) {
				return true;
			}
		}
	} else {
		while (--m->indexUnk49Unk1 >= 0) {
			m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
			if (((m->flags4A & 0xA) == 0 || _xMstPos2 >= m->m49Unk1->unkC) && ((m->flags4A & 0x5) == 0 || _yMstPos2 >= m->m49Unk1->unkD)) {
				return true;
			}
		}
	}
	return false;
}

void Game::executeMstUnk8(MonsterObject1 *m) {
	if (m->unkAC != -1) {
		if (m->xMstPos != m->unkAC || m->yMstPos != m->unkB0) {
			_xMstPos2 = m->unkB4;
			_yMstPos2 = m->unkB8;
			return;
		}
		mstBoundingBoxClear(m, 1);
	}
// 41AB38
	mstSetVerticalHorizontalBounds(m);
	if (m->flags4A == 0) {
		return;
	}
	m->unkB4 = _xMstPos2;
	m->unkB8 = _yMstPos2;
	MstUnk49Unk1 *m49u1 = m->m49Unk1;
	int _ecx, _edx;
	if (_mstLut1[m->flags4A] & 1) {
		_ecx = m49u1->unkA;
		_edx = m49u1->unkB;
	} else {
		_ecx = m49u1->unk8;
		_edx = m49u1->unk9;
	}
	if (_yMstPos2 < _ecx && _yMstPos2 < _edx) {
		if (_xMstPos2 <= 0 && _yMstPos2 <= 0) {
			return;
		}
		if (!executeMstUnk6(m)) {
			return;
		}
	}
// 41ABED
	if (_mstLut1[m->flags4A] & 1) {
		if (_xMstPos2 < _yMstPos2) {
			if (_xMstPos2 < m->m49Unk1->unkA) {
				m->flags4A &= ~0xA;
				if (m->flagsA7 != 255) {
					_xMstPos2 = 0;
				}
			}
		} else {
// 41AC2B
			if (_yMstPos2 < m->m49Unk1->unkB) {
				m->flags4A &= ~0x5;
				if (m->flagsA7 != 255) {
					_yMstPos2 = 0;
				}
			}
		}
	}
// 41AC4F
	int var10 = (~m->flagsA5) & 1;
	uint32_t indexUnk34 = m->m44Unk1->indexUnk34_16;
	assert(indexUnk34 != kNone);
	MstUnk34 *m34 = &_res->_mstUnk34[indexUnk34];
	int var20 = 0;
	int varC = 0;
	int var8 = _mstLut1[m->flags4A];
	for (; var20 < 5; ++var20) {
		if (var20 != 0) {
			const uint8_t *p = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
			if (p[0xE] == 0 || varC == 0) {
				break;
			}
		}
// 41ACAB
		int num = var20 + var8 * 5;
		int var4 = _mstLut3[num];
		if (_mstLut1[var4] == m->flagsA8[3]) {
			continue;
		}
		int _ecx, _eax;
		const uint8_t *p = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
		if (_mstLut1[var4] & 1) {
			_ecx = p[0xA];
			_eax = p[0xB];
		} else {
			_ecx = p[0x8];
			_eax = p[0x9];
		}
// 41ACF5
		int _edi = m->xMstPos;
		if (var4 & 8) {
			_edi -= _ecx;
			if (_edi < m->x2) {
				continue;
			}
		} else if (var4 & 2) {
			_edi += _ecx;
			if (_edi > m->x1) {
				continue;
			}
		}
// 41AD27
		int _ebp = m->yMstPos;
		if (var4 & 1) {
			_ebp -= _eax;
			if (_ebp < m->y2) {
				continue;
			}
		} else if (var4 & 4) {
			_ebp += _eax;
			if (_ebp > m->y1) {
				continue;
			}
		}
// 41AD53
		if (var10 == 1 && (m->flagsA5 & 4) == 0 && (m->flags48 & 8) != 0) {
			if (!mstSetCurrentPos(m, _edi, _ebp)) {
				continue;
			}
		}
// 41AD7B
		int w = READ_LE_UINT32(m->unk8 + 904);
		int h = READ_LE_UINT32(m->unk8 + 908);
		if (_edi < m34->left - w || _edi > m34->right + w || _ebp < m34->bottom - h || _ebp > m34->top + h) {
			const uint32_t indexUnk44 = m->m46Unk1->indexUnk44;
			assert(indexUnk44 != kNone);
			MstUnk44 *m44 = &_res->_mstUnk44[indexUnk44];
			const int num = executeMstUnk15(m, m44, _edi, _ebp);
			if (num < 0) {
				continue;
			}
			if (m->m44Unk1->unk60[var10][num] == 0) {
				continue;
			}
// 41ADDF
		}
		m->unkAC = _edi;
		m->unkB0 = _ebp;
		p = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
		if (p[0xE] != 0) {
			const int x1 = m->xMstPos + (int8_t)p[0xC];
			const int x2 = x1 + p[0xE] - 1;
			const int y1 = m->yMstPos + (int8_t)p[0xD];
			const int y2 = y1 + p[0xF] - 1;
			int r = mstBoundingBoxCollides2(m->monster1Index, x1, y1, x2, y2);
			if (r > 0) {
// 41AEA3
				MstBoundingBox *b = &_mstBoundingBoxesTable[r - 1];
				if (m->unk7C < b->x1 || m->unk74 > b->x2 || m->unk80 < b->y1 || m->unk78 > b->y2) {
					break;
				}
			} else {
// 41AF09
				m->flagsA8[1] = mstBoundingBoxUpdate(m->flagsA8[1], m->monster1Index, x1, y1, x2, y2);
			}
// 41AFB3
		}
		mstBoundingBoxClear(m, 1);
// 41B00C
		m->flags4A = var4;
		if (var20 == 0) {
			m->flagsA8[3] = 255;
			uint8_t n = _mstLut1[var4];
			if (n >= 4) {
				n -= 4;
			}
			m->flagsA8[3] = n;
		}
	}
// 41B030
	m->task->flags |= 0x80;
	m->flags4A = 0;
	_yMstPos2 = 0;
	_xMstPos2 = 0;
}

int Game::executeMstUnk9(Task *t, MonsterObject1 *m) {
	if (m->unk8[946] & 4) {
		mstBoundingBoxClear(m, 1);
	}
	if (m->flags4B != 0xFC && (m->flagsA5 & 8) != 0 && (t->flags & 0x20) != 0 && m->unk18) {
		LvlObject *o = m->o16;
		const int bx = _res->_mstPointOffsets[_currentScreen].xOffset;
		const int by = _res->_mstPointOffsets[_currentScreen].yOffset;
		const int ox = o->xPos + _res->_mstPointOffsets[o->screenNum].xOffset;
		const int oy = o->yPos + _res->_mstPointOffsets[o->screenNum].yOffset;
		if (ox < bx || ox + o->width - 1 > bx + 255 || oy < by || oy + o->height - 1 > by + 191) {
			return mstTaskStopMonsterObject1(t);
		}
	}
	executeMstUnk1(t);
	return 0;
}

int Game::executeMstUnk11(Task *t, MonsterObject1 *m) {
	if ((m->unk8[946] & 4) == 0 || (_mstLut1[m->flags4A] & 1) != 0) {
		if (_xMstPos2 < m->m49->unk14) {
			m->flags4A &= ~0xA;
		}
		if (_yMstPos2 < m->m49->unk15) {
			m->flags4A &= ~0x5;
		}
	}
// 41B64F
	const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	if ((m->unk8[946] & 4) == 0 && (m->flagsA5 & 4) == 0 && (m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0) {
		int _edi, _ebp;
		if (_mstLut1[m->flags4A] & 1) {
			_edi = (int8_t)ptr[0xA];
			_ebp = (int8_t)ptr[0xB];
		} else {
			_edi = (int8_t)ptr[0x8];
			_ebp = (int8_t)ptr[0x9];
		}
		int x = m->xMstPos;
		int y = m->yMstPos;
		if (m->flags4A & 8) {
			x -= _edi;
		} else if (m->flags4A & 2) {
			x += _edi;
		}
		if (m->flags4A & 1) {
			y -= _ebp;
		} else if (m->flags4A & 4) {
			y += _ebp;
		}
		if (!mstSetCurrentPos(m, x, y)) {
			_xMstPos2 = ABS(m->xMstPos - _mstCurrentPosX);
			_yMstPos2 = ABS(m->yMstPos - _mstCurrentPosY);
		}
	}
// 41B72A
	ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	int _edx, _ecx;
	if (_mstLut1[m->flags4A] & 1) {
		_edx = (int8_t)ptr[0xA];
		_ecx = (int8_t)ptr[0xB];
	} else {
		_edx = (int8_t)ptr[0x8];
		_ecx = (int8_t)ptr[0x9];
	}
	if (m->flags4A == 0) {
		return executeMstUnk9(t, m);
	}
	if (_xMstPos2 < _edx && _yMstPos2 < _ecx) {
		if ((_xMstPos2 <= 0 && _yMstPos2 <= 0) || !executeMstUnk6(m)) {
			return executeMstUnk9(t, m);
		}
	}
// 41B79D
	if ((m->unk8[946] & 4) == 0 && (_mstLut1[m->flags4A] & 1) != 0) {
		if (_xMstPos2 < _yMstPos2) {
			if (_xMstPos2 < m->m49Unk1->unkA) {
				const uint8_t _al = m->flags4A;
				m->flags4A &= ~0xA;
				if (m->flagsA7 != _al) {
					_xMstPos2 = 0;
				}

			}
		} else {
// 41B7E9
			if (_yMstPos2 < m->m49Unk1->unkB) {
				const uint8_t _al = m->flags4A;
				m->flags4A &= ~0x5;
				if (m->flagsA7 != _al) {
					_yMstPos2 = 0;
				}
			}
		}
	}
// 41B80E
	ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	mstLvlObjectSetActionDirection(m->o16, ptr, ptr[3], m->flags4A);
	return 1;
}

// ret >0: found a rect matching
// ret <0: return the closest match (setting _xMstPos3 and _yMstPos3)
int Game::executeMstUnk15(MonsterObject1 *m, MstUnk44 *m44, int x, int y) {
	_xMstPos3 = x;
	_yMstPos3 = y;
	const int num = (~m->flagsA5) & 1;
	int _esi = 0x40000000;
	int var20 = -1;
	int var24 = -1;
	int _ebp = 0;
	for (uint32_t i = 0; i < m44->count; ++i, --var24) {
		int _edi;
		MstUnk44Unk1 *m44Unk1 = &m44->data[i];
		if (m44Unk1->unk60[num][i] == 0 && m->m44Unk1 != m44Unk1) {
			continue;
		}
		const uint32_t indexUnk34 = m44Unk1->indexUnk34_16;
		assert(indexUnk34 != kNone);
		MstUnk34 *m34 = &_res->_mstUnk34[indexUnk34];
		if (x >= m34->left && x <= m34->right && y >= m34->bottom && y <= m34->top) {
			return i;
		}
// 41A3CE
		if (x >= m34->left && x <= m34->right) {
			int var4 = m34->top;
			int var8 = ABS(y - m34->top);
			int _ebx = m34->bottom;
			int _eax = ABS(y - m34->bottom);
			if (_eax >= var8) {
				_ebx = var4;
			}
			_edi = y - _ebx;
			_edi *= _edi;
			if (_esi >= _edi) {
				_esi = _edi;
				_xMstPos3 = x;
				_yMstPos3 = _ebx;
				var20 = var24;
			}
		} else if (y >= m34->bottom && y <= m34->top) {
// 41A435
			int var8 = m34->right;
			int var4 = ABS(x - m34->right);
			int _ecx = m34->left;
			int _eax = ABS(x - m34->left);
			if (_eax >= var4) {
				_ecx = var8;
			}
			_ebp = x - _ecx;
			_ebp *= _ebp;
			if (_esi >= _ebp) {
				_xMstPos3 = _ecx;
				_yMstPos3 = y;
				var20 = var24;
				_esi = _ebp;
			}

		} else {
// 41A49C
			int _edx = (x - m34->left);
			_edx *= _edx;
			int _eax = (y - m34->bottom);
			_eax *= _eax;
			_edx += _eax;
			if (_esi >= _edx) {
				_xMstPos3 = m34->left;
				_yMstPos3 = m34->bottom;
				var20 = var24;
				_esi = _edx;
			}
// 41A4C6
			_edx = x - m34->right;
			_edx *= _edx;
			_eax += _edx;
			if (_esi >= _edx) {
				_xMstPos3 = m34->right;
				_yMstPos3 = m34->bottom;
				var20 = var24;
			}
// 41A4FB
			_edi = y - m34->top;
			_edi *= _edi;
			_edx += _edi;
			if (_esi >= _edi) {
				_xMstPos3 = m34->right;
				_yMstPos3 = m34->top;
				var20 = var24;
			}
// 41A529
			_ebp = x - m34->left;
			_ebp *= _ebp;
			_eax = _edi + _ebp;
			if (_esi >= _eax) {
				_xMstPos3 = m34->left;
				_yMstPos3 = m34->top;
				var20 = var24;
			}

		}
// 41A560
		// This matches disassembly byt looks like a copy-paste from the above condition
/*
		if (_esi >= _edi + _ebp) {
			_xMstPos3 = x;
			_yMstPos3 = _ebx;
			var20 = var24;

		}
*/
	}
	return var20;
}

void Game::executeMstUnk16(MstUnk44 *m44, int flag) {
	warning("executeMstUnk16 unimplemented");
	// TODO
}

bool Game::mstTestActionDirection(MonsterObject1 *m, int num) {
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
					if (_ebp == 160 && (_mstLut1[_ecx] & 1) != 0) {
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
			if ((m->unk8[946] & 2) != 0 && (_mstLut1[_ecx] & 1) != 0) {
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
	_ecx &= var4[2];
	if ((_bl & 0xE0) == 0x40) {
		_ecx ^= 0xA;
	}
	return (var8 & _ecx) != 0 ? 0 : 1;
}

bool Game::lvlObjectCollidesAndy1(LvlObject *o, int flags) const {
	int x1, y1, x2, y2;
	if (flags != 1 && flags != 0x4000) {
		x1 = o->xPos;
		y1 = o->yPos;
		x2 = x1 + o->width  - 1;
		y2 = y1 + o->height - 1;
	} else {
		x1 = o->xPos + o->posTable[0].x;
		x2 = o->xPos + o->posTable[1].x;
		y1 = o->yPos + o->posTable[0].y;
		y2 = o->yPos + o->posTable[1].y;
		if (x1 > x2) {
			SWAP(x1, x2);
		}
		if (y1 > y2) {
			SWAP(y1, y2);
		}
		if (flags == 0x4000 && _andyObject->screenNum != o->screenNum) {
			const int dx = _res->_mstPointOffsets[o->screenNum].xOffset - _res->_mstPointOffsets[_andyObject->screenNum].xOffset;
			x1 += dx;
			x2 += dx;
			const int dy = _res->_mstPointOffsets[o->screenNum].yOffset - _res->_mstPointOffsets[_andyObject->screenNum].yOffset;
			y1 += dy;
			y2 += dy;
		}
	}
	int x = _andyObject->xPos + _andyObject->width / 2;
	int y = _andyObject->yPos + _andyObject->height / 2;
	return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
}

bool Game::lvlObjectCollidesAndy2(LvlObject *o, int type) const {
	int x1, y1, x2, y2;
	if (type != 1 && type != 0x1000) {
		x1 = o->xPos; // _esi
		y1 = o->yPos; // _edi
		x2 = x1 + o->width  - 1; // _ebx
		y2 = y1 + o->height - 1; // _ebp
	} else {
		x1 = o->xPos + o->posTable[0].x; // _esi
		x2 = o->xPos + o->posTable[1].x; // _ebx
		y1 = o->yPos + o->posTable[0].y; // _edi
		y2 = o->yPos + o->posTable[1].y; // _ebp
		if (x1 > x2) {
			SWAP(x1, x2);
		}
		if (y1 > y2) {
			SWAP(y1, y2);
		}
		if (type == 0x1000 && _andyObject->screenNum != o->screenNum) {
			const int dx = _res->_mstPointOffsets[_andyObject->screenNum].xOffset - _res->_mstPointOffsets[o->screenNum].xOffset;
			x1 += dx;
			x2 += dx;
			const int dy = _res->_mstPointOffsets[_andyObject->screenNum].yOffset - _res->_mstPointOffsets[o->screenNum].yOffset;
			y1 += dy;
			y2 += dy;
		}
	}
	return (_andyObject->xPos + _andyObject->width - 1 >= x1 && _andyObject->xPos <= x2 && _andyObject->yPos + _andyObject->height - 1 >= y1 && _andyObject->yPos <= y2);
}

bool Game::lvlObjectCollidesAndy3(LvlObject *o, int type) const {
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

bool Game::lvlObjectCollidesAndy4(LvlObject *o, int type) const {
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

bool Game::mstCollidesByFlags(MonsterObject1 *m, uint32_t flags) {
	if ((flags & 1) != 0 && (m->o16->flags0 & 0x200) == 0) {
		return false;
	} else if ((flags & 8) != 0 && (m->flags48 & 0x20) == 0) {
		return false;
	} else if ((flags & 0x100) != 0 && (_mstFlags & 0x80000000) != 0) {
		return false;
	} else if ((flags & 0x200) != 0 && (_mstFlags & 0x40000000) != 0) {
		return false;
	} else if ((flags & 0x40) != 0 && (mstGetFacingDirectionMask(m->flags49) & 1) != ((m->o16->flags1 >> 4) & 1) && (m->unk8[946] & 4) == 0) {
		return false;
	} else if ((flags & 0x80) != 0 && (mstGetFacingDirectionMask(m->flags49) & 1) != ((m->o16->flags1 >> 4) & 1) && (m->unk8[946] & 4) != 0) {
		return false;
	} else if ((flags & 0x400) != 0 && (m->o16->screenNum != _andyObject->screenNum || !lvlObjectCollidesAndy1(m->o16, 0))) {
		return false;
	} else if ((flags & 0x800) != 0 && (m->o16->screenNum != _andyObject->screenNum || !lvlObjectCollidesAndy1(m->o16, 1))) {
		return false;
	} else if ((flags & 0x100000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !lvlObjectCollidesAndy3(m->o16, 0))) {
		return false;
	} else if ((flags & 0x200000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !lvlObjectCollidesAndy3(m->o16, 1))) {
		return false;
	} else if ((flags & 4) != 0 && (m->o16->screenNum != _andyObject->screenNum || !lvlObjectCollidesAndy2(m->o16, 0))) {
		return false;
	} else if ((flags & 2) != 0 && (m->o16->screenNum != _andyObject->screenNum || !lvlObjectCollidesAndy2(m->o16, 1))) {
		return false;
	} else if ((flags & 0x4000) != 0 && !lvlObjectCollidesAndy1(m->o16, 0x4000)) {
		return false;
	} else if ((flags & 0x1000) != 0 && !lvlObjectCollidesAndy2(m->o16, 0x1000)) {
		return false;
	} else if ((flags & 0x20) != 0 && (m->o16->flags0 & 0x100) == 0) {
		return false;
	} else if ((flags & 0x10000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !lvlObjectCollidesAndy4(m->o16, 1))) {
		return false;
	} else if ((flags & 0x20000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !lvlObjectCollidesAndy4(m->o16, 0))) {
		return false;
	} else if ((flags & 0x40000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !clipLvlObjectsBoundingBox(m->o16, _andyObject, 36))) {
		return false;
	} else if ((flags & 0x80000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !clipLvlObjectsBoundingBox(m->o16, _andyObject, 20))) {
		return false;
	}
	return true;
}

bool Game::executeMstUnk27(MonsterObject1 *m, const uint8_t *p) {
	const uint32_t a = READ_LE_UINT32(p + 0x10);
	if (a == 0 || !mstCollidesByFlags(m, a)) {
		return false;
	}
	if ((a & 0x8000) != 0 && (m->flagsA6 & 4) == 0) {
		Task t;
		memcpy(&t, _mstCurrentTask, sizeof(Task));
		_mstCurrentTask->child = 0;
		const uint32_t codeData = READ_LE_UINT32(p + 0x18);
		assert(codeData != kNone);
		_mstCurrentTask->codeData = _res->_mstCodeData + codeData * 4;
		_mstCurrentTask->run = &Game::mstSm_main;
		_mstCurrentTask->monster1->flagsA6 |= 4;
		Task *currentTask = _mstCurrentTask;
		mstSm_main(_mstCurrentTask);
		_mstCurrentTask = currentTask;
		_mstCurrentTask->monster1->flagsA6 &= ~4;
		t.nextPtr = _mstCurrentTask->nextPtr;
		t.prevPtr = _mstCurrentTask->prevPtr;
		memcpy(_mstCurrentTask, &t, sizeof(Task));
		_mstCurrentTask->run = &Game::runTask_idle;
		if ((_mstCurrentTask->monster1->flagsA6 & 2) == 0) {
			_mstCurrentTask->run = &Game::mstSm_main;
		}
		return false;
	} else {
		mstTaskAttack(_mstCurrentTask, READ_LE_UINT32(p + 0x18), 0x10);
		return true;
	}
}

int Game::mstUpdateTaskMonsterObject1(Task *t) {
	debug(kDebug_MONSTER, "mstUpdateTaskMonsterObject1 t %p", t);
	_mstCurrentTask = t;
	MonsterObject1 *m = t->monster1;
	MonsterObject1 *_mstCurrentMonster1 = m;
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
		mstRemoveMonsterObject1(_mstCurrentTask, &_monsterObjects1TasksList);
		return 1;
	}
	const uint32_t _edi = READ_LE_UINT32(_mstCurrentDataPtr + 20);
	if (_edi != kNone) {
		MstUnk46 *m46 = _mstCurrentMonster1->m46;
		for (uint32_t i = 0; i < m46->count; ++i) {
			if (m46->data[i].indexMonsterInfo == _edi) {
				copyMonsterObject1(_mstCurrentTask, _mstCurrentMonster1, i);
				return 0;
			}
		}
	}
// 41824B
	if ((m->flagsA5 & 0x80) != 0) {
		return 0;
	}
	if (m->localVars[7] == 0 && !_specialAnimFlag) {
		// monster is dead
		m->flagsA5 |= 0x80;
		if (m->unk8[946] & 4) {
			mstBoundingBoxClear(m, 1);
		}
		if (t->child) {
			t->child->codeData = 0;
			t->child = 0;
		}
		if ((m->flagsA5 & 8) != 0 && m->unk18 && _m48Num != -1) {
			mstTaskStopMonsterObject1(_mstCurrentTask);
			return 0;
		}
		const uint32_t codeData = m->m46Unk1->codeData;
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
		executeMstUnk27(_mstCurrentMonster1, _mstCurrentDataPtr);
		return 0;
	}
	if ((m->flagsA5 & 0x40) != 0) {
		return 0;
	}
// 418384
	assert(_mstCurrentMonster1 == m);
	int dir = 0;
	for (int i = 0; i < _mstMovingStateCount; ++i) {
		MovingOpcodeState *p = &_mstMovingState[i];
		if (p->m && p->m != m) {
			continue;
		}
		if (m->collideDistance < 0) {
			continue;
		}
		if ((m->flags48 & 4) == 0) {
			continue;
		}
		if (m->m46Unk1->indexUnk51 == kNone) {
			continue;
		}
		if (_rnd.getNextNumber() > m->m46Unk1->unk10) {
			continue;
		}
		m->unkF0 = 8;
		int var28 = 0;
		MovingOpcodeState *var14 = m->collidePtr;
		if (t->run != &Game::runTask_unk5 && t->run != &Game::runTask_unk6 && t->run != &Game::runTask_unk7 && t->run != &Game::runTask_unk8 && t->run != &Game::runTask_unk9 && t->run != &Game::runTask_unk10) {
			if (m->unk8[946] & 2) {
				_mstCurrentMonster1->unk74 = _mstCurrentMonster1->unk78 = 0x80000000;
				_mstCurrentMonster1->unk7C = _mstCurrentMonster1->unk80 = 0x7FFFFFFF;
				switch (var14->unk24) {
				case 0:
					var28 = 2;
					break;
				case 1:
				case 133:
					var28 = 9;
					break;
				case 2:
					var28 = 12;
					break;
				case 3:
				case 128:
					var28 = 3;
					break;
				case 4:
					var28 = 6;
					break;
				case 5:
					var28 = 8;
					break;
				case 6:
					var28 = 1;
					break;
				case 7:
					var28 = 4;
					break;
				}
				_mstCurrentMonster1->unkF0 = _mstLut1[var28];
			}
		} else {
// 4184EC
			m->unkF0 = _mstLut1[m->flags4A];
			var28 = 0;
		}
// 418508
		uint32_t var24 = 0;
		MstUnk46Unk1 *_esi = m->m46Unk1;
		if (_esi->count != 0) {
			var24 = _rnd.update() % (_esi->count + 1);
		}
// 418530
		int var20 = -1;
		int indexUnk51;
		if ((m->flagsA5 & 8) != 0 && m->unk18 && m->unk18->indexUnk51 != kNone && m->unk8 == &_res->_mstMonsterInfos[m->unk18->unk0 * 948]) {
			indexUnk51 = m->unk18->indexUnk51;
		} else {
			indexUnk51 = _esi->indexUnk51;
		}
		int _ebx = -1;
		assert(indexUnk51 >= 0 && indexUnk51 < _res->_mstHdr.unk0x48);
		MstUnk51 *var18 = &_res->_mstUnk51[indexUnk51]; // _esi
		for (; var24 < var18->count; ++var24) {
			assert(m->unkF0 >= 0 && m->unkF0 < 9);
			uint32_t indexUnk50Unk1 = var18->indexUnk50Unk1[var24 * 9 + m->unkF0];
			MstUnk50Unk1 *m50u1 = &_res->_mstUnk50[var18->indexUnk50].data[indexUnk50Unk1];
			const int32_t x1 = m50u1->unk1C;
			const int32_t y1 = m50u1->unk20;
			if (x1 != 0 || y1 != 0) {
				int dirMask = 0;
				int _ebp = 0;
				if ((m50u1->unk4 & 0xA) != 0xA && (m->o16->flags1 & 0x10) != 0) {
					_mstTemp_x1 = m->xMstPos - x1;
				} else {
					_mstTemp_x1 = m->xMstPos + x1;
				}
				if (_mstTemp_x1 < m->xMstPos) {
					if (_mstTemp_x1 < m->unk74) {
						dirMask = 8;
					}
					_ebp = 8;
				} else if (_mstTemp_x1 > m->xMstPos) {
					if (_mstTemp_x1 > m->unk7C) {
						dirMask = 2;
					}
					_ebp = 2;
				} else {
					_ebp = 0;
				}
				_mstTemp_y1 = m->yMstPos + y1;
				if (_mstTemp_y1 < m->yMstPos) {
					if (_mstTemp_y1 < m->unk78 && (m->unk8[946] & 2) != 0) {
						dirMask |= 1;
					}
					_ebp |= 1;
				} else if (_mstTemp_y1 > m->yMstPos) {
					if (_mstTemp_y1 > m->unk80 && (m->unk8[946] & 2) != 0) {
						dirMask |= 4;
					}
					_ebp |= 4;
				}
				if (var28 == dirMask) {
					continue;
				}
				if (executeMstUnk14(m, _mstTemp_x1, _mstTemp_y1, dirMask)) {
					continue;
				}
			}
// 41866A
			_mstTemp_x1 = m->xMstPos;
			if ((m50u1->unk4 & 0xA) != 0xA && (m->o16->flags1 & 0x10) != 0) {
				_mstTemp_x1 -= m50u1->unk14;
				_mstTemp_x1 -= m50u1->unkC;
			} else {
				_mstTemp_x1 += m50u1->unkC;
			}
			_mstTemp_y1 = m->yMstPos + m50u1->unk10;
			_mstTemp_x2 = _mstTemp_x1 + m50u1->unk14 - 1;
			_mstTemp_y2 = _mstTemp_y1 + m50u1->unk18 - 1;
			if ((m->unk8[946] & 4) != 0 && mstBoundingBoxCollides1(m->monster1Index, _mstTemp_x1, _mstTemp_y1, _mstTemp_x2, _mstTemp_y2)) {
				continue;
			}
			int _ecx;
			if (m->unk8[946] & 2) {
				_ecx = var14->boundingBox.y2;
			} else {
				_ecx = m->yMstPos;
			}
			if (m50u1->unk14 != 0 && getMstDistance(_ecx, var14) >= 0) {
				continue;
			}
			if (m->collideDistance >= m50u1->unk24) {
// 418766
				MstUnk46Unk1 *m46u1 = m->m46Unk1;
				_ebx = var24;
				int _edi = m50u1->unk24;
				if (m46u1->unk18 != 0) {
					_edi += (_rnd.update() % (m46u1->unk18 * 2 + 1)) - m46u1->unk18;
					if (_edi < 0) {
						_edi = 0;
					}
				}
				dir = _edi;
				break;
			}
			if (var20 == -1) {
				dir = m50u1->unk24;
				var20 = var24;
			}
			_ebx = var20;
		}
		if (_ebx >= 0) {
// 4187BC
			uint32_t indexUnk50Unk1 = var18->indexUnk50Unk1[_ebx * 9 + m->unkF0];
			MstUnk50Unk1 *m50u1 = &_res->_mstUnk50[var18->indexUnk50].data[indexUnk50Unk1];

			mstTaskAttack(_mstCurrentTask, m50u1->codeData, 0x40);
			_mstCurrentMonster1->unkF8 = m50u1->unk8;
			_mstCurrentMonster1->unkF4 = dir;
			_mstCurrentMonster1->unkFC = var14->unk24;
			_mstCurrentMonster1->directionKeyMask = _andyObject->directionKeyMask;
			return 0;
		}
	}
// 41882E
	if (o->screenNum == _currentScreen && (m->flagsA5 & 0x20) == 0 && (m->flags48 & 0x10) != 0) {
		MstUnk46Unk1 *m46 = m->m46Unk1;
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

					if (_mstAndyLevelPosX >= x1 /*_edx*/ && _mstAndyLevelPosX <= x2 /*_edi*/ && _mstAndyLevelPosY >= y1 /*_esi*/ && _mstAndyLevelPosY <= y2 /*_ebp*/) {
						mstTaskAttack(_mstCurrentTask, READ_LE_UINT32(p + 16), 0x20);
						executeMstUnk27(_mstCurrentMonster1, _mstCurrentDataPtr);
						return 0;
					}

					p += 20;
				}
			}
		}
	}
// 418939
	if (executeMstUnk27(_mstCurrentMonster1, _mstCurrentDataPtr)) {
		return 0;
	}
	uint8_t _al = _mstCurrentMonster1->flagsA6;
	uint8_t _dl = _mstCurrentMonster1->flagsA5;
	if ((_al & 2) != 0 || (_dl & 0x30) != 0) {
		return 0;
	}
	dir = _dl & 3;
	if (dir == 1) {
// 418AC6
		MstUnk44Unk1 *m44Unk1 = _mstCurrentMonster1->m44Unk1;
		if (m44Unk1->indexUnk35_24 == kNone) {
			return 0;
		}
		int _ebp = 0;
		if (_mstAndyLevelPosY >= m->yMstPos - m44Unk1->y1 && _mstAndyLevelPosY < m->yMstPos + m44Unk1->y2) {
			if (m44Unk1->x1 != -2 || m44Unk1->x1 != m44Unk1->x2) {
				if (m->xDelta <= m44Unk1->x1) {
					if (_al & 1) {
						_ebp = 1;
					} else {
						_al = mstGetFacingDirectionMask(_mstCurrentMonster1->flags49) & 1;
						_dl = (_mstCurrentMonster1->o16->flags1 >> 4) & 1;
						if (_dl == _al || (_mstCurrentMonster1->unk8[946] & 4) != 0) {
							_ebp = 1;
						} else if (m->xDelta <= m44Unk1->x2) {
							_ebp = 2;
						}
					}
				}
// 418B1B
			} else if (o->screenNum == _currentScreen) {
				if (_al & 1) {
					_ebp = 1;
				} else {
					_al = mstGetFacingDirectionMask(_mstCurrentMonster1->flags49) & 1;
					_dl = (_mstCurrentMonster1->o16->flags1 >> 4) & 1;
					if (_al != _dl && (_mstCurrentMonster1->unk8[946] & 4) == 0) {
						_ebp = 2;
					} else {
						_ebp = 1;
					}
				}
			}
		}
// 418BAA
		if (_ebp == 0) {
			m->flagsA6 &= ~1;
			if ((m->flagsA5 & 4) == 0) {
				const uint32_t indexUnk35 = m->m44Unk1->indexUnk35_24;
				assert(indexUnk35 != kNone);
				m->m35 = &_res->_mstUnk35[indexUnk35];
			}
			return 0;
		} else if (_ebp == 1) {
// 418C73
			m->flagsA6 |= 1;
			assert(_mstCurrentMonster1 == m);
			if (mstSetCurrentPos(m, m->xMstPos, m->yMstPos) == 0 && (m->unk8[946] & 2) == 0) {
				if ((_mstCurrentPosX > m->xMstPos && _mstCurrentPosX > m->m44Unk1->unk2C[0][1]) || (_mstCurrentPosX < m->xMstPos && _mstCurrentPosX < m->m44Unk1->unk2C[1][1])) {
					uint32_t indexUnk35 = m->m44Unk1->indexUnk35_20;
					if (indexUnk35 != kNone) {
						m->m35 = &_res->_mstUnk35[indexUnk35];
					}
					if (m->flagsA5 & 4) {
						m->flagsA5 &= ~4;
						if (!updateMonsterObject1Position(m)) {
							initMonsterObject1(m);
						}
						indexUnk35 = m->m44Unk1->indexUnk35_20;
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
				MstUnk44Unk1 *m44 = m->m44Unk1;
				int _edi = READ_LE_UINT32(m->unk8 + 904);
				int _ebx = MAX(m->unk88, m44->unk2C[1][1] + _edi);
				int _eax = MIN(m->unk84, m44->unk2C[0][1] - _edi);
				uint32_t indexUnk36 = m44->indexUnk36_32;
				assert(indexUnk36 != kNone);
				uint32_t indexUnk49 = _res->_mstUnk36[indexUnk36].indexUnk49;
				assert(indexUnk49 != kNone);
				uint8_t _bl = _res->_mstUnk49[indexUnk49].unk14;
				if (ABS(_eax - _ebx) <= _bl) {
					uint32_t indexUnk35 = m44->indexUnk35_20;
					if (indexUnk35 != kNone) {
						m->m35 = &_res->_mstUnk35[indexUnk35];
					}
					if (m->flagsA5 & 4) {
						m->flagsA5 &= ~4;
						if (!updateMonsterObject1Position(_mstCurrentMonster1)) {
							initMonsterObject1(_mstCurrentMonster1);
						}
						indexUnk35 = _mstCurrentMonster1->m44Unk1->indexUnk35_20;
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
		const uint32_t indexUnk35 = m->m44Unk1->indexUnk35_24;
		assert(indexUnk35 != kNone);
		MstUnk35 *m35 = &_res->_mstUnk35[indexUnk35];
		if (m->m35 != m35) {
			_mstCurrentMonster1->m35 = m35;
			_rnd.resetMst(_mstCurrentMonster1->rnd_m35);
			prepareMstTask(_mstCurrentTask);
			return 0;
		}
// 418C1D
		if (m->flagsA5 & 4) {
			m->flagsA5 &= ~4;
			if (!updateMonsterObject1Position(_mstCurrentMonster1)) {
				initMonsterObject1(_mstCurrentMonster1);
			}
			const uint32_t indexUnk35 = m->m44Unk1->indexUnk35_24;
			assert(indexUnk35 != kNone);
			_mstCurrentMonster1->m35 = &_res->_mstUnk35[indexUnk35];
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
			if ((int32_t)READ_LE_UINT32(m->unk8 + 916) <= m->m44Unk1->unk2C[1][1] || (int32_t)READ_LE_UINT32(m->unk8 + 912) >= m->m44Unk1->unk2C[0][1]) {
				m->flagsA6 |= 1;
				assert(m == _mstCurrentMonster1);
				m->flagsA5 = 1;
				initMonsterObject1(m);
				uint32_t indexUnk35 = m->m44Unk1->indexUnk35_20;
				if (indexUnk35 != kNone) {
					m->m35 = &_res->_mstUnk35[indexUnk35];
				}
				return 0;
			}
		} else if (_dl & 8) {
// 418A37
			if ((int32_t)READ_LE_UINT32(m->unk8 + 920) >= m->m44Unk1->unk2C[0][1] || (int32_t)READ_LE_UINT32(m->unk8 + 924) <= m->m44Unk1->unk2C[1][1]) {
				m->flagsA6 |= 1;
				assert(m == _mstCurrentMonster1);
				m->flagsA5 = 1;
				initMonsterObject1(m);
				uint32_t indexUnk35 = m->m44Unk1->indexUnk35_20;
				if (indexUnk35 != kNone) {
					m->m35 = &_res->_mstUnk35[indexUnk35];
				}
				return 0;
			}
		}
	}
// 418A9A
	if (mstSetCurrentPos(m, m->xMstPos, m->yMstPos) == 0) {
		executeMstOp67Type2(t, 1);
	}
	return 0;
}

int Game::mstUpdateTaskMonsterObject2(Task *t) {
	debug(kDebug_MONSTER, "mstUpdateTaskMonsterObject2 t %p", t);
	mstTaskSetScreenPosition(t);
	MonsterObject2 *m = t->monster2;
	if (_currentLevel == 1 && m->m45->unk0 == 27) {
		if (_fireflyPosData[m->hPosIndex] == 0xFF) {
			uint32_t r = _rnd.update();
			uint8_t _dl = (r % 5) << 3;
			m->vPosIndex = _dl;
			if (m->hPosIndex >= 40) {
				m->hPosIndex = _dl;
				m->vPosIndex = _dl + 40;
				m->hDir = (r >> 16) & 1;
			} else {
				m->hPosIndex = m->vPosIndex + 40;
				m->vDir = (r >> 16) & 1;
			}
		}
// 41911A
		int dx = _fireflyPosData[m->hPosIndex];
		if (m->hDir == 0) {
			dx = -dx;
		}
		int dy = _fireflyPosData[m->vPosIndex];
		if (m->vDir == 0) {
			dy = -dy;
		}
		++m->vPosIndex;
		++m->hPosIndex;
		m->o->xPos += dx;
		m->o->yPos += dy;
		m->xMstPos += dx;
		m->yMstPos += dy;
		if (m->xMstPos > m->x2) {
			m->hDir = 0;
		} else if (m->xMstPos < m->x1) {
			m->hDir = 1;
		}
		if (m->yMstPos > m->y2) {
			m->vDir = 0;
		} else if (m->yMstPos < m->y1) {
			m->vDir = 1;
		}
	}
// 4191B1
	uint8_t _dl = 0;
	for (int i = 0; i < _mstMovingStateCount; ++i) {
		MovingOpcodeState *p = &_mstMovingState[i];
		if (p->unk40 == 2) {
			_dl |= 1;
		} else if (p->unk40 == 1) {
			_dl |= 2;
		}
	}
// 4191DC
	LvlObject *o = m->o;
	MstUnk45 *m45 = m->m45;
	uint8_t _bl = m45->unk1;
	if (_bl != _dl) {
		for (int i = 0; i < _mstMovingStateCount; ++i) {
			MovingOpcodeState *p = &_mstMovingState[i];
			if (p->unk40 == 2 && (_bl & 1) == 0) {
				continue;
			}
			if (p->unk40 == 1 && (_bl & 2) == 0) {
				continue;
			}
			if (o->screenNum != _currentScreen || p->o->screenNum != _currentScreen) {
				continue;
			}
			if (!clipLvlObjectsBoundingBox(p->o, o, 20)) {
				continue;
			}
			ShootLvlObjectData *s = p->unk28;
			s->unk3 = 0x80;
			s->x2 = o->xPos + o->width / 2;
			s->y2 = o->yPos + o->height / 2;
			if (p->unk40 != 2 || (_bl & 4) != 0) {
				continue;
			}
			const uint32_t codeData = m45->codeData;
			if (codeData != kNone) {
				resetTask(t, _res->_mstCodeData + codeData * 4);
			} else {
				o->actionKeyMask = 7;
				o->directionKeyMask = 0;
				t->run = &Game::runTask_idle;
			}
		}
	}
	if ((m->o->flags0 & 0xFF) == 0x1F) {
		mstRemoveMonsterObject2(t, &_monsterObjects2TasksList);
		return 1;
	}
	return 0;
}

void Game::mstUpdateRefPos() {
	if (_andyObject) {
		_mstAndyScreenPosX = _andyObject->xPos;
		_mstAndyScreenPosY = _andyObject->yPos;
		_mstAndyLevelPosX = _mstAndyScreenPosX + _res->_mstPointOffsets[_currentScreen].xOffset;
		_mstAndyLevelPosY = _mstAndyScreenPosY + _res->_mstPointOffsets[_currentScreen].yOffset;
		if (!_specialAnimFlag) {
			_mstAndyRectNum = mstBoundingBoxUpdate(_mstAndyRectNum, 0xFE, _mstAndyLevelPosX, _mstAndyLevelPosY, _mstAndyLevelPosX + _andyObject->width - 1, _mstAndyLevelPosY + _andyObject->height - 1) & 0xFF;
		}
		_mstAndyScreenPosX += _andyObject->posTable[3].x;
		_mstAndyScreenPosY += _andyObject->posTable[3].y;
		_mstAndyLevelPosX += _andyObject->posTable[3].x;
		_mstAndyLevelPosY += _andyObject->posTable[3].y;
	} else {
		_mstAndyScreenPosX = 128;
		_mstAndyScreenPosY = 96;
		_mstAndyLevelPosX = _mstAndyScreenPosX + _res->_mstPointOffsets[0].xOffset;
		_mstAndyLevelPosY = _mstAndyScreenPosY + _res->_mstPointOffsets[0].yOffset;
        }
	_mstMovingStateCount = 0;
	_mstMovingState[0].unk40 = 0;
	if (!_lvlObjectsList0) {
		if (_plasmaCannonDirection == 0) {
			_executeMstLogicPrevCounter = _executeMstLogicCounter;
			return;
		}
		_mstMovingState[0].width  = 512;
		_mstMovingState[0].height = 512;
		_mstMovingState[0].unk28 = 0;
		_mstMovingState[0].unk40 = 3;
		_mstMovingState[0].unk18 = 4;
		_mstMovingState[0].xPos = _plasmaCannonPosX[_plasmaCannonFirstIndex] + _res->_mstPointOffsets[_currentScreen].xOffset;
		_mstMovingState[0].yPos = _plasmaCannonPosY[_plasmaCannonFirstIndex] + _res->_mstPointOffsets[_currentScreen].yOffset;
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
			p->width  = ptr->dxPos;
			p->height = ptr->dyPos;
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

// update monster bounding box
void Game::updateMstHeightMapData() {
	const int _mstAndyLevelPosDx = _mstAndyLevelPosX - _mstAndyLevelPrevPosX;
	const int _mstAndyLevelPosDy = _mstAndyLevelPosY - _mstAndyLevelPrevPosY;
	_mstAndyLevelPrevPosX = _mstAndyLevelPosX;
	_mstAndyLevelPrevPosY = _mstAndyLevelPosY;
	if (_mstAndyLevelPosDx == 0 && _mstAndyLevelPosDy == 0) {
		return;
	}
	int offset = 0;
	for (int i = 0; i < _res->_mstHdr.unk0x3C; ++i) {
		offset += kMonsterInfoDataSize;
		const uint32_t unk30 = READ_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x30]); // 900
		const uint32_t unk34 = READ_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x34]); // 896

		const uint32_t unk20 = _mstAndyLevelPosX - unk30;
		const uint32_t unk1C = _mstAndyLevelPosX + unk30;
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x20], unk20);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x1C], unk1C);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x24], unk20 - unk34);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x18], unk1C + unk34);

		const uint32_t unk10 = _mstAndyLevelPosY - unk30;
		const uint32_t unk0C = _mstAndyLevelPosY + unk30;
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x10], unk10);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x0C], unk0C);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x14], unk10 - unk34);
		WRITE_LE_UINT32(&_res->_mstMonsterInfos[offset - 0x08], unk0C + unk34);
	}
}

void Game::mstRemoveMonsterObject2(Task *t, Task **tasksList) {
	MonsterObject2 *m = t->monster2;
	m->m45 = 0;
	LvlObject *o = m->o;
	if (o) {
		o->dataPtr = 0;
		removeLvlObject2(o);
	}
	removeTask(tasksList, t);
}

void Game::mstRemoveMonsterObject1(Task *t, Task **tasksList) {
	MonsterObject1 *m = t->monster1;
	if (_m48Num != -1) {
		if ((m->flagsA5 & 8) != 0 && m->unk18) {
			disableMonsterObject1(m);
		}
	}
	if (m->unk8[946] & 4) {
		mstBoundingBoxClear(m, 0);
		mstBoundingBoxClear(m, 1);
	}
	m->m46 = 0;
	LvlObject *o = m->o16;
	if (o) {
		o->dataPtr = 0;
	}
	for (int i = 0; i < kMaxMonsterObjects2; ++i) {
		if (_monsterObjects2Table[i].m45 != 0 && _monsterObjects2Table[i].monster1 == m) {
			_monsterObjects2Table[i].monster1 = 0;
		}
	}
	removeLvlObject2(o);
	removeTask(tasksList, t);
}

void Game::mstTaskAttack(Task *t, uint32_t codeData, uint8_t flags) {
	MonsterObject1 *m = t->monster1;
	m->flagsA5 = (m->flagsA5 & ~0x70) | flags;
	Task *c = t->child;
	if (c) {
		t->child = 0;
		c->codeData = 0;
	}
	if (m->flagsA5 & 8) {
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
				n->run = &Game::mstSm_main;
			}
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
			memset(t, 0, sizeof(Task));
			return t;
		}
	}
	return 0;
}

Task *Game::createTask(const uint8_t *codeData) {
	Task *t = findFreeTask();
	if (t) {
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
	return 0;
}

int Game::mstTaskSetActionDirection(Task *t, int num, int delay) {
	uint8_t var4 = _res->_mstUnk52[num * 4];
	uint8_t var8 = _res->_mstUnk52[num * 4 + 2];
	MonsterObject1 *m = t->monster1;
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
	const uint8_t am = _res->_mstUnk52[num * 4 + 1];
	o->actionKeyMask |= am;

	t->flags &= ~0x80;
	int _edi = (int8_t)ptr[4];
	int _ebp = (int8_t)ptr[5];
	debug(kDebug_MONSTER, "mstTaskSetActionDirection m %p action 0x%x direction 0x%x (%d,%d)", m, o->actionKeyMask, o->directionKeyMask, _edi, _ebp);
	int _eax = 0;
	int var10 = 0;
	if (_edi != 0 || _ebp != 0) {
// 40E8E2
		uint8_t var11 = ptr[2];
		if (((var11 & 0xA) == 0xA && (o->directionKeyMask & 8) != 0) || ((var11 & 0xA) != 0xA && (o->flags1 & 0x10) != 0)) {
			_edi = m->xMstPos - _edi;
		} else {
			_edi = m->xMstPos + _edi;
		}
		if (_edi < m->xMstPos) {
			var10 = 8;
		} else {
			var10 = (_edi <= m->xMstPos) ? 0 : 2;
		}
		if ((var11 & 0x5) == 0x5 && (o->directionKeyMask & 1) != 0) {
			_ebp = m->yMstPos - _ebp;
		} else {
			_ebp = m->yMstPos + _ebp;
		}
// 40E950
		if (_ebp < m->yMstPos) {
			var10 |= 1;
		} else if (_ebp > m->yMstPos) {
			var10 |= 4;
		}
// 40E96D
		if ((m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0 && mstSetCurrentPos(m, _edi, _ebp) == 0) {
			t->flags |= 0x80;
			return 0;
		}
		if (((var10 & 8) != 0 && _edi < m->x2) || ((var10 & 2) != 0 && _edi > m->x1) || ((var10 & 1) != 0 && (_ebp < m->y2))) {
			t->flags |= 0x80;
			return 0;
		}
		_eax = var10;
// 40E9BC
	}
	if ((m->unk8[946] & 4) != 0 && ptr[14] != 0) {
		if (_eax == 0) {
			_ebp = 0;
		} else if (_mstLut1[_eax] & 1) {
// 40E9FF
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
		if ((var8 & 0xE0) != 0x60 && mstBoundingBoxCollides2(m->monster1Index, _edi, _ebp, ptr[14] + _edi - 1, ptr[15] + _ebp - 1) != 0) {
			t->flags |= 0x80;
			return 0;
		}
// 40EAA0
		m->flagsA8[0] = mstBoundingBoxUpdate(m->flagsA8[0], m->monster1Index, _edi, _ebp, ptr[14] + _edi - 1, ptr[15] + _ebp - 1);
	}
// 40EAD0
	m->flagsA4 = var4;
	if (delay == -1) {
		const uint32_t offset = m->unk8 - _res->_mstMonsterInfos;
		assert((offset % kMonsterInfoDataSize) == 0);
		t->arg2 = offset / kMonsterInfoDataSize;
		t->run = &Game::runTask_unk4;
		debug(kDebug_MONSTER, "mstTaskSetActionDirection arg2 %d", t->arg2);
	} else {
		t->arg1 = delay;
		t->run = &Game::runTask_unk3;
		debug(kDebug_MONSTER, "mstTaskSetActionDirection arg1 %d", t->arg1);
	}
	return 1;
}

void Game::updateTask(Task *t, int num, const uint8_t *codeData) {
	Task *current = _tasksList;
	bool found = false;
	while (current) {
		Task *nextPtr = current->nextPtr;
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
					t->run = &Game::mstSm_main;
				}
			}
		}
		current = nextPtr;
	}
	if (found) {
		return;
	}
	if (codeData) {
		t = findFreeTask();
		if (t) {
			memset(t, 0, sizeof(Task));
			resetTask(t, codeData);
			t->prevPtr = 0;
			t->nextPtr = _tasksList;
			if (_tasksList) {
				_tasksList->prevPtr = t;
			}
			_tasksList = t;
			t->localVars[7] = num;
		}
	}
}

void Game::resetTask(Task *t, const uint8_t *codeData) {
	debug(kDebug_MONSTER, "resetTask t %p offset 0x%04x", t, codeData - _res->_mstCodeData);
	assert(codeData);
	t->state |= 2;
	t->codeData = codeData;
	t->run = &Game::mstSm_main;
	t->localVars[7] = 0;
	MonsterObject1 *m = t->monster1;
	if (m) {
		const uint8_t mask = m->flagsA5;
		if ((mask & 0x88) == 0 || (mask & 0xF0) == 0) {
			if ((mask & 8) != 0) {
				t->flags = (t->flags & ~0x40) | 0x20;
				m->flags48 &= ~0x1C;
			} else if ((mask & 2) != 0) {
				m->flags48 |= 8;
				const MstUnk46Unk1 *m46unk1 = m->m46Unk1;
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
			MonsterObject1 *m = 0;
			if (t->monster2) {
				m = t->monster2->monster1;
			} else {
				m = t->monster1;
			}
			if (m) {
				assert(index < kMaxLocals);
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
			MonsterObject1 *m = 0;
			if (t->monster2) {
				m = t->monster2->monster1;
			} else {
				m = t->monster1;
			}
			if (m) {
				assert(index < kMaxLocals);
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
	if (index & 0x80) {
		const int mask = 1 << (index & 0x7F);
		return ((mask & _mstAndyVarMask) != 0) ? 1 : 0;
	}
	switch (index) {
	case 0:
		return (_andyObject->flags1 >> 4) & 1;
	case 1:
		return (_andyObject->flags1 >> 5) & 1;
	case 2: {
			MonsterObject1 *m = t->monster1;
			if (m) {
				return ((m->o16->flags1 & 0x10) != 0) ? 1 : 0;
			} else if (t->monster2) {
				return ((t->monster2->o->flags1 & 0x10) != 0) ? 1 : 0;
			}
		}
		break;
	case 3: {
			MonsterObject1 *m = t->monster1;
			if (m) {
				return ((m->o16->flags1 & 0x20) != 0) ? 1 : 0;
			} else if (t->monster2) {
				return ((t->monster2->o->flags1 & 0x20) != 0) ? 1 : 0;
			}
		}
	case 4: {
			MonsterObject1 *m = t->monster1;
			if (m) {
				return ((m->o16->flags0 & 0x200) != 0) ? 1 : 0;
			} else if (t->monster2) {
				return ((t->monster2->o->flags0 & 0x200) != 0) ? 1 : 0;
			}
		}
		break;
	case 5:
		return ((_andyObject->flags0 & 0x1F) == 7) ? 1 : 0;
	case 6:
		return (_andyObject->spriteNum == 0) ? 1 : 0;
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
		return _mstAndyScreenPosX;
	case 1:
		return _mstAndyScreenPosY;
	case 2:
		return _mstAndyLevelPosX;
	case 3:
		return _mstAndyLevelPosY;
	case 4:
		return _currentScreen;
	case 5:
		return _res->_screensState[_currentScreen].s0;
	case 6:
		return _difficulty;
	case 7:
		if (t->monster1 && t->monster1->collidePtr) {
			return t->monster1->collidePtr->unk40;
		}
		return _mstMovingState[0].unk40;
	case 8:
		if (t->monster1 && t->monster1->collidePtr) {
			return t->monster1->collidePtr->unk24 & 0x7F;
		}
		return _mstMovingState[0].unk24 & 0x7F;
	case 9:
		if (t->monster1 && t->monster1->unk18) {
			return t->monster1->unk18->unk8;
		}
		break;
	case 10:
		if (t->monster1 && t->monster1->unk18) {
			return t->monster1->unk18->unkC;
		}
		break;
	case 11:
		if (t->monster1) {
			return t->monster1->unkF4;
		}
		break;
	case 12:
		if (t->monster1) {
			return t->monster1->xPos;
		} else if (t->monster2) {
			return t->monster2->xPos;
		}
		break;
	case 13:
		if (t->monster1) {
			return t->monster1->yPos;
		} else if (t->monster2) {
			return t->monster2->yPos;
		}
		break;
	case 14:
		if (t->monster1) {
			return t->monster1->o16->screenNum;
		} else if (t->monster2) {
			return t->monster2->o->screenNum;
		}
		break;
	case 15:
		if (t->monster1) {
			return t->monster1->xDelta;
		} else if (t->monster2) {
			return ABS(_mstAndyLevelPosX - t->monster2->xMstPos);
		}
		break;
	case 16:
		if (t->monster1) {
			return t->monster1->yDelta;
		} else if (t->monster2) {
			return ABS(_mstAndyLevelPosY - t->monster2->yMstPos);
		}
		break;
	case 17:
		if (t->monster1) {
			return t->monster1->collideDistance;
		}
		break;
	case 18:
		if (t->monster1) {
			return ABS(t->monster1->x2 - t->monster1->xMstPos);
		}
		break;
	case 19:
		if (t->monster1) {
			return ABS(t->monster1->x1 - t->monster1->xMstPos);
		}
		break;
	case 20:
		if (t->monster1) {
			return ABS(t->monster1->y2 - t->monster1->yMstPos);
		}
		break;
	case 21:
		if (t->monster1) {
			return ABS(t->monster1->y1 - t->monster1->yMstPos);
		}
		break;
	case 22:
		return _mstOp54Counter;
	case 23:
		return _andyObject->actionKeyMask;
	case 24:
		return _andyObject->directionKeyMask;
	case 25:
		if (t->monster1) {
			return t->monster1->xMstPos;
		} else if (t->monster2) {
			return t->monster2->xMstPos;
		}
		break;
	case 26:
		if (t->monster1) {
			return t->monster1->yMstPos;
		} else if (t->monster2) {
			return t->monster2->yMstPos;
		}
		break;
	case 27:
		if (t->monster1) {
			return t->monster1->o16->flags0 & 0xFF;
		} else if (t->monster2) {
			return t->monster2->o->flags0 & 0xFF;
		}
		break;
	case 28:
		if (t->monster1) {
			return t->monster1->o16->anim;
		} else if (t->monster2) {
			return t->monster2->o->anim;
		}
		break;
	case 29:
		if (t->monster1) {
			return t->monster1->o16->frame;
		} else if (t->monster2) {
			return t->monster2->o->frame;
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
			if (t->monster1) {
				o = t->monster1->o16;
			} else if (t->monster2) {
				o = t->monster2->o;
			}
			if (o) {
				return _res->_screensState[o->screenNum].s0;
			}
		}
		break;
	case 34:
		return _levelCheckpoint;
	case 35:
		return _mstAndyCurrentScreenNum;
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
			MonsterObject1 *m = 0;
			if (t->monster2) {
				m = t->monster2->monster1;
			} else {
				m = t->monster1;
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

int Game::mstSm_main(Task *t) {
	assert(t->codeData);
	const int taskNum = t - _tasksTable;
	int ret = 0;
	t->state &= ~2;
	const uint8_t *p = t->codeData;
	do {
		assert(p >= _res->_mstCodeData && p < _res->_mstCodeData + _res->_mstHdr.codeSize * 4);
		assert(((p - t->codeData) & 3) == 0);
		const uint32_t codeOffset = p - _res->_mstCodeData;
		debug(kDebug_MONSTER, "executeMstCode task %d %p code %d offset 0x%04x", taskNum, t, p[0], codeOffset);
		assert(p[0] <= 242);
		switch (p[0]) {
		case 0: { // 0
				LvlObject *o = 0;
				if (t->monster1) {
					if ((t->monster1->flagsA6 & 2) == 0) {
						o = t->monster1->o16;
					}
				} else if (t->monster2) {
					o = t->monster2->o;
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
				t->arg1 = delay;
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
		case 3:
		case 8: // 3 - set_monster_action_direction_imm
			if (t->monster1) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				ret = mstTaskSetActionDirection(t, num, (arg == 0xFF) ? -1 : arg);
			}
			break;
		case 4: // 4 - set_monster_action_direction_task_var
			if (t->monster1) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstUnk52[num * 4 + 3];
				t->codeData = p;
				assert(arg < kMaxLocals);
				ret = mstTaskSetActionDirection(t, num, t->localVars[arg]);
			}
			break;
		case 13: // 8
			if (t->monster1) {
				const int num = READ_LE_UINT16(p + 2);
				if (mstTestActionDirection(t->monster1, num)) {
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
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
				}
				if (m) {
					m->flags48 |= (1 << p[1]);
				}
			}
			break;
		case 26: // 16 - clear_flag_global
			_mstFlags &= ~(1 << p[1]);
			break;
		case 27: // 17 - clear_flag_task
			t->flags &= ~(1 << p[1]);
			break;
		case 28: { // 18 - clear_flag_mst
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
				}
				if (m) {
					m->flags48 &= ~(1 << p[1]);
				}
			}
			break;
		case 30: { // 20
				t->arg1 = 3;
				t->arg2 = p[1];
				if (((1 << p[1]) & _mstFlags) == 0) {
					LvlObject *o = 0;
					if (t->monster1) {
						if ((t->monster1->flagsA6 & 2) == 0) {
							o = t->monster1->o16;
						}
					} else if (t->monster2) {
						o = t->monster2->o;
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
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
				}
				if (m) {
					t->arg1 = 5;
					t->arg2 = p[1];
					if (((1 << p[1]) & m->flags48) == 0) {
						LvlObject *o = 0;
						if (t->monster1) {
							if ((t->monster1->flagsA6 & 2) == 0) {
								o = t->monster1->o16;
							}
						} else if (t->monster2) {
							o = t->monster2->o;
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
				p = _res->_mstCodeData + num * 4 - 4;
			}
			break;
		case 34:
			// no-op
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
		case 39: // 26 - remove_monsters_screen
			if (p[1] < _res->_mstHdr.pointsCount) {
				mstOp26_removeMstTaskScreen(&_monsterObjects1TasksList, p[1]);
				mstOp26_removeMstTaskScreen(&_monsterObjects2TasksList, p[1]);
				// mstOp26_removeMstTaskScreen(&_mstTasksList3, p[1]);
				// mstOp26_removeMstTaskScreen(&_mstTasksList4, p[1]);
			}
			break;
		case 40: // 27 - remove_monsters_screen_flags
			if (p[1] < _res->_mstHdr.pointsCount) {
				mstOp27_removeMstTaskScreenFlags(&_monsterObjects1TasksList, p[1], p[2]);
				mstOp27_removeMstTaskScreenFlags(&_monsterObjects2TasksList, p[1], p[2]);
				// mstOp27_removeMstTaskScreenFlags(&_mstTasksList3, p[1], p[2]);
				// mstOp27_removeMstTaskScreenFlags(&_mstTasksList4, p[1], p[2]);
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
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
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
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
				}
				if (m) {
					assert(p[1] < kMaxLocals);
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
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
				}
				if (m) {
					assert(p[1] < kMaxLocals);
					assert(p[2] < kMaxLocals);
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
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
				}
				if (m) {
					assert(p[1] < kMaxVars);
					assert(p[2] < kMaxLocals);
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
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
				}
				if (m) {
					assert(p[1] < kMaxLocals);
					assert(p[2] < kMaxLocals);
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
				assert(p[1] < kMaxVars);
				assert(p[2] < kMaxVars);
				arithOp(p[0] - 117, &_mstVars[p[1]], _mstVars[p[2]]);
				if (p[1] == 31 && _mstVars[31] > 0) {
					_mstTickDelay = _mstVars[31];
				}
			}
			break;
		case 127:
		case 128:
		case 129:
		case 130:
		case 131:
		case 132:
		case 133:
		case 134:
		case 135:
		case 136: { // 42
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
				}
				if (m) {
					assert(p[1] < kMaxLocals);
					assert(p[2] < kMaxVars);
					arithOp(p[0] - 127, &m->localVars[p[1]], _mstVars[p[2]]);
				}
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
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
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
		case 196: { // 48 - arith_monster_var_imm
				MonsterObject1 *m = 0;
				if (t->monster2) {
					m = t->monster2->monster1;
				} else {
					m = t->monster1;
				}
				if (m) {
					const int16_t num = READ_LE_UINT16(p + 2);
					assert(p[1] < kMaxLocals);
					arithOp(p[0] - 187, &m->localVars[p[1]], num);
				}
			}
			break;
		case 197: // 49
			if (t->monster1) {
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
				ret = mstOp49_setMovingBounds(a, b, c, d, e, t, num);
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
					t->state &= ~2;
					p -= 4;
				}
			}
			break;
		case 199: // 51
			mstTaskStopMonsterObject1(t);
			return 0;
		case 200: // 52
			if (t->monster1 && t->monster1->unk18) {
				mstOp52();
				return 1;
			}
			mstOp52();
			break;
		case 201: { // 53
				const int num = READ_LE_UINT16(p + 2);
				mstOp53(&_res->_mstUnk48[num]);
			}
			break;
		case 202: // 54
			mstOp54();
			break;
		case 203: // 55
			// _mstCurrentMonster1 = t->monster1;
			if (t->monster1) {
				const int num = READ_LE_UINT16(p + 2);
				if (mstCollidesByFlags(t->monster1, _res->_mstUnk59[num].flags)) {
					t->codeData = p + 4;
					mstTaskAttack(t, _res->_mstUnk59[num].codeData, 0x10);
					t->state &= ~2;
					p = t->codeData - 4;
				}
			}
			break;
		case 204: // 56
			ret = mstOp56_specialAction(t, p[1], READ_LE_UINT16(p + 2));
			break;
		case 207:
		case 208:
		case 209: // 79
			break;
		case 210: // 57
			{
				MonsterObject1 *m = t->monster1;
				mstOp57_addCrackSprite(m->xPos + (int8_t)p[2], m->yPos + (int8_t)p[3], m->o16->screenNum);
			}
			break;
		case 211: // 58
			mstOp58_addLvlObject(t, READ_LE_UINT16(p + 2));
			break;
		case 212: { // 59
				LvlObject *o = 0;
				if (t->monster2) {
					o = t->monster2->o;
				} else if (t->monster1) {
					o = t->monster1->o16;
				} else {
					break;
				}
				assert(o);
				int xPos = o->xPos + o->posTable[6].x; // _edi
				int yPos = o->yPos + o->posTable[6].y; // _edx
				const uint16_t flags1 = o->flags1;
				if (flags1 & 0x10) {
					xPos -= (int8_t)p[2];
				} else {
					xPos += (int8_t)p[2];
				}
				if (flags1 & 0x20) {
					yPos -= (int8_t)p[3];
				} else {
					yPos += (int8_t)p[3];
				}
				int dirMask = 0;
				if ((t->monster1 && (t->monster1->unk8[944] == 10 || t->monster1->unk8[944] == 25)) || (t->monster2 && (t->monster2->m45->unk0 == 10 || t->monster2->m45->unk0 == 25))) {
					int dx = o->posTable[6].x - o->posTable[7].x;
					int dy = o->posTable[6].y - o->posTable[7].y;
					if (dx >= 8) {
						dirMask = 2;
					} else if (dx < -8) {
						dirMask = 8;
					}
					if (dy >= 8) {
						dirMask |= 4;
					} else if (dy < -8) {
						dirMask |= 1;
					}
				} else {
					dirMask = ((flags1 & 0x10) != 0) ? 8 : 0;
				}
				if (p[1] == 255) {
					int type = 0;
					switch (dirMask) {
					case 1:
						type = 6;
						break;
					case 3:
						type = 3;
						break;
					case 4:
						type = 7;
						break;
					case 6:
						type = 4;
						break;
					case 8:
						type = 5;
						break;
					case 9:
						type = 1;
						break;
					case 12:
						type = 2;
						break;
					}
					mstOp59_1(xPos, yPos, o->screenNum, type, (o->flags2 + 1) & 0xDFFF);
				} else {
					int type = 0;
					switch (dirMask) {
					case 1:
						type = 6;
						break;
					case 3:
						type = 3;
						break;
					case 4:
						type = 7;
						break;
					case 6:
						type = 4;
						break;
					case 8:
						type = 5;
						break;
					case 9:
						type = 1;
						break;
					case 12:
						type = 2;
						break;
					}
					mstOp59_addShootFireball(xPos, yPos, o->screenNum, p[1], type, (o->flags2 + 1) & 0xDFFF);
				}
			}
			break;
		case 213: { // 60 - monster_set_action_direction
				LvlObject *o = 0;
				if (t->monster2) {
					o = t->monster2->o;
				} else if (t->monster1) {
					o = t->monster1->o16;
				}
				if (o) {
					o->actionKeyMask = getTaskVar(t, p[2], p[1] >> 4);
					o->directionKeyMask = getTaskVar(t, p[3], p[1] & 15);
				}
			}
			break;
		case 214: { // 61 - reset_monster_energy
				MonsterObject1 *m = t->monster1;
				if (m) {
					m->flagsA5 &= ~0xC0;
					m->localVars[7] = m->m46Unk1->energy;
				}
			}
			break;
		case 215: { // 62
				if (_m43Num3 != -1) {
					assert(_m43Num3 < _res->_mstHdr.unk0x24);
					shuffleMstUnk43(&_res->_mstUnk43[_m43Num3]);
				}
				_mstOp54Counter = 0;
			}
			break;
		case 217: { // 64
				const int16_t num = READ_LE_UINT16(p + 2);
				if (_m43Num3 != num) {
					_m43Num3 = num;
					assert(num >= 0 && num < _res->_mstHdr.unk0x24);
					shuffleMstUnk43(&_res->_mstUnk43[num]);
					_mstOp54Counter = 0;
				}
			}
			break;
		case 218: { // 65
				const int16_t num = READ_LE_UINT16(p + 2);
				if (num != _m43Num1) {
					_m43Num1 = num;
					_m43Num2 = num;
					assert(num >= 0 && num < _res->_mstHdr.unk0x24);
					shuffleMstUnk43(&_res->_mstUnk43[num]);
				}
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
				if (t->monster2) {
					o = t->monster2->o;
				} else if (t->monster1) {
					o = t->monster1->o16;
				}
				if (e <= -2 && o) {
					if (o->flags1 & 0x10) {
// 413657
						const int x1 = a;
						const int x2 = b;
						a = o->xPos - x2;
						b = o->xPos - x1;
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
					_mstOp68_type = m->unk8;
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
							if (a >= -_mstAndyScreenPosX && a <= 255 - _mstAndyScreenPosX) {
								break;
							}
						} else if (e == _currentScreen) {
							break;
						}
					}
				}
// 4137FA
				mstOp67_addMonster(t, a, b, c, d, e, m->unk8, m->unk9, m->unkC, m->unkB, 0, m->unkE);
			}
			break;
		case 226: { // 68
				const int num = READ_LE_UINT16(p + 2);
				const MstOp226Data *m226Data = &_res->_mstOp226Data[num];
				int _edi  = _res->_mstPointOffsets[_currentScreen].xOffset; // xOffset
				int var14 = _res->_mstPointOffsets[_currentScreen].yOffset; // yOffset
				int var1C = 0;
				int var8  = 0;
				int _ecx  = m226Data->unk4 * 256;
				int _edx  = _edi + 256;
				_edi -= _ecx;
				_edx += _ecx;
				int var20 = _edx;
				for (int i = 0; i < kMaxMonsterObjects1; ++i) {
					MonsterObject1 *m = &_monsterObjects1Table[i];
					if (!m->m46) {
						continue;
					}
					if (m->unk8[944] != _res->_mstMonsterInfos[m226Data->unk0 * kMonsterInfoDataSize + 944]) {
						continue;
					}
					if (m->xMstPos < _edi || m->xMstPos > var20) {
						continue;
					}
					if (m->yMstPos < var14 || m->yMstPos > var14 + 192) {
						continue;
					}
					if (_mstAndyLevelPosX > m->xMstPos) {
						++var8;
					} else {
						++var1C;
					}
				}
				t->flags |= 0x80;
				_edi = var1C;
				_edx = var8;
				int _ebx = var1C + var8;
				if (_ebx >= m226Data->unk3) {
					break;
				}
				_edi += var8;
				_ecx = m226Data->unk3 - _edi;
				_edi = m226Data->unk1;
				if (var8 >= _edi) {
					_edi = 0;
				} else {
					_edi -= var8;
				}
				int _esi = m226Data->unk2;
				if (var1C >= _esi) {
					_esi = 0;
				} else {
					_esi -= var1C;
				}
				if (_edi != 0) {
					if (_mstOp67_screenNum < 0) {
						if (_mstOp67_x2 >= -_mstAndyScreenPosX && _mstOp67_x1 <= -_mstAndyScreenPosX * 2) {
							_edi = 0;
						}
					} else if (_mstOp67_screenNum == _currentScreen) {
						_edi = 0;
					}
				}
				if (_esi != 0) {
					if (_mstOp68_screenNum < 0) {
						if (_mstOp68_x2 >= -_mstAndyScreenPosX && _mstOp68_x1 <= -_mstAndyScreenPosX * 2) {
							_esi = 0;
						}
					} else if (_mstOp68_screenNum == _currentScreen) {
						_esi = 0;
					}
				}
				if (_edi != 0 || _esi != 0) {
					mstOp68_addMonsterGroup(t, _res->_mstMonsterInfos + m226Data->unk0 * kMonsterInfoDataSize, _edi, _esi, _ecx, m226Data->unk6);
				}
			}
			break;
		case 227: { // 69
				const int num = READ_LE_UINT16(p + 2);
				assert(num < _res->_mstHdr.unk0x58);
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
				assert(num < _res->_mstHdr.unk0x58);
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
				if (compareOp(m->compare, a, b)) {
					if (p[0] == 231) {
						LvlObject *o = 0;
						MonsterObject1 *m = t->monster1;
						if (m) {
							if ((m->flagsA6 & 2) == 0) {
								o = m->o16;
							}
						} else if (t->monster2) {
							o = t->monster2->o;
						}
						if (o) {
							o->actionKeyMask = 0;
							o->directionKeyMask = 0;
						}
						t->arg2 = num;
						t->run = &Game::runTask_mstOp231;
						ret = 1;
					}
				} else {
					if (p[0] == 232) {
						LvlObject *o = 0;
						MonsterObject1 *m = t->monster1;
						if (m) {
							if ((m->flagsA6 & 2) == 0) {
								o = m->o16;
							}
						} else if (t->monster2) {
							o = t->monster2->o;
						}
						if (o) {
							o->actionKeyMask = 0;
							o->directionKeyMask = 0;
						}
						t->arg2 = num;
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
						if (t->monster1) {
							if ((t->monster1->flagsA6 & 2) == 0) {
								o = t->monster1->o16;
							}
						} else if (t->monster2) {
							o = t->monster2->o;
						}
						if (o) {
							o->actionKeyMask = 0;
							o->directionKeyMask = 0;
						}
						t->arg2 = num;
						t->run = &Game::runTask_mstOp233;
						ret = 1;
					}
				} else {
					if (p[0] == 234) {
						LvlObject *o = 0;
						if (t->monster1) {
							if ((t->monster1->flagsA6 & 2) == 0) {
								o = t->monster1->o16;
							}
						} else if (t->monster2) {
							o = t->monster2->o;
						}
						if (o) {
							o->actionKeyMask = 0;
							o->directionKeyMask = 0;
						}
						t->arg2 = num;
						t->run = &Game::runTask_mstOp234;
						ret = 1;
					}
				}
			}
			break;
		case 237: // 74 - remove_monster_task
			if (t->monster1) {
				if (!t->monster2) {
					mstRemoveMonsterObject1(t, &_monsterObjects1TasksList);
					return 1;
				}
				mstRemoveMonsterObject2(t, &_monsterObjects2TasksList);
				return 1;
			} else {
				if (t->monster2) {
					mstRemoveMonsterObject2(t, &_monsterObjects2TasksList);
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
				const uint8_t *codeData = (m->codeData == kNone) ? 0 : (_res->_mstCodeData + m->codeData * 4);
				updateTask(t, m->flags, codeData);
			}
			break;
		case 242: // 78
			debug(kDebug_MONSTER, "child %p monster1 %p monster2 %p", t->child, t->monster1, t->monster2);
			if (t->child) {
				Task *child = t->child;
				child->prevPtr = t->prevPtr;
				child->nextPtr = t->nextPtr;
				memcpy(t, child, sizeof(Task));
				t->child = 0;
				t->state &= ~2;
				child->codeData = 0;
				MonsterObject1 *m = t->monster1;
				if (m) {
					m->flagsA5 &= ~0x70;
					if ((m->flagsA5 & 8) != 0 && !m->unk18) {
						executeMstUnk23(t);
						return 1;
					}
				}
				return 0;
			} else if (t->monster1) {
				MonsterObject1 *m = t->monster1;
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
										const uint8_t i = _rnd.getMstNextNumber(m->rnd_m35);
										num = m35->data2[i];
									}
									const uint32_t codeData = m35->indexCodeData[num];
									assert(codeData != kNone);
									resetTask(t, _res->_mstCodeData + codeData * 4);
									t->state &= ~2;
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
								const uint8_t i = _rnd.getMstNextNumber(m->rnd_m35);
								num = m35->data2[i];
							}
							const uint32_t codeData = m35->indexCodeData[num];
							assert(codeData != kNone);
							resetTask(t, _res->_mstCodeData + codeData * 4);
							t->state &= ~2;
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
							disableMonsterObject1(m);
						}
						m->flagsA5 = (m->flagsA5 & ~0xF) | 6;
						return executeMstOp67Type2(t, 1);
					}
				} else if ((m->flagsA5 & 8) != 0) {
// 413F8B
					m->flagsA5 &= ~8;
					const uint32_t codeData = m->m46Unk1->codeData;
					if (codeData != kNone) {
						resetTask(t, _res->_mstCodeData + codeData * 4);
						return 0;
					} else {
						m->o16->actionKeyMask = 7;
						m->o16->directionKeyMask = 0;
						t->run = &Game::runTask_idle;
						return 1;
					}
				} else {
					t->run = &Game::runTask_idle;
					return 1;
				}
			} else if (t->monster2) {
				mstRemoveMonsterObject2(t, &_monsterObjects2TasksList);
				ret = 1;
			} else {
				if ((t->state & 1) != 0 && _mstVars[31] == 0) {
					_mstVars[31] = _mstTickDelay;
				}
				removeTask(&_tasksList, t);
				ret = 1;
			}
			break;
		default:
			warning("Unhandled opcode %d in mstSm_main", *p);
			break;
		}
		p += 4;
		if ((t->state & 2) != 0) {
			t->state &= ~2;
			p = t->codeData;
		}
		++_runTaskOpcodesCount;
	} while (_runTaskOpcodesCount <= 128 && ret == 0);
	if (t->codeData) {
		t->codeData = p;
	}
	return 1;
}

void Game::mstOp26_removeMstTaskScreen(Task **tasksList, int screenNum) {
	Task *current = *tasksList; // _esi
	while (current) {
		MonsterObject1 *m = current->monster1; // _ecx
		Task *next = current->nextPtr; // _ebp
		if (m && m->o16->screenNum == screenNum) {
			if (_m48Num != -1 && (m->flagsA5 & 8) != 0 && m->unk18 != 0) {
				disableMonsterObject1(m);
			}
			const uint8_t *ptr = m->unk8;
			if (ptr[946] & 4) {
				mstBoundingBoxClear(m, 0);
				mstBoundingBoxClear(m, 1);
			}
			resetMonsterObject1(m);
			removeLvlObject2(m->o16);
			removeTask(tasksList, current);
		} else {
			MonsterObject2 *mo = current->monster2;
			if (mo && mo->o->screenNum == screenNum) {
				mo->m45 = 0;
				mo->o->dataPtr = 0;
				removeLvlObject2(mo->o);
				removeTask(tasksList, current);
			}
		}
		current = next;
	}
}

void Game::mstOp27_removeMstTaskScreenFlags(Task **tasksList, int screenNum, int flags) {
	Task *current = *tasksList;
	while (current) {
		MonsterObject1 *m = current->monster1;
		Task *next = current->nextPtr;
		if (m && m->o16->screenNum == screenNum) {
			if ((m->unk8[944] & 0x7F) != flags) {
				continue;
			}
			if (_m48Num != -1 && (m->flagsA5 & 8) != 0 && m->unk18 != 0) {
				disableMonsterObject1(m);
			}
			const uint8_t *ptr = m->unk8;
			if (ptr[946] & 4) {
				mstBoundingBoxClear(m, 0);
				mstBoundingBoxClear(m, 1);
			}
			resetMonsterObject1(m);
			removeLvlObject2(m->o16);
			removeTask(tasksList, current);
		} else {
			MonsterObject2 *mo = current->monster2;
			if (mo && mo->o->screenNum == screenNum) {
				if ((mo->m45->unk0 & 0x7F) != flags) {
					continue;
				}
				mo->m45 = 0;
				mo->o->dataPtr = 0;
				removeLvlObject2(mo->o);
				removeTask(tasksList, current);
			}
		}
		current = next;
	}
}

int Game::mstOp49_setMovingBounds(int a, int b, int c, int d, int screen, Task *t, int num) {
	debug(kDebug_MONSTER, "mstOp49 %d %d %d %d %d %d", a, b, c, d, screen, num);
	MonsterObject1 *m = t->monster1;
	const MstOp49Data *op49data = &_res->_mstOp49Data[num];
	MstUnk49 *m49 = &_res->_mstUnk49[op49data->unkC];
	m->m49 = m49;
	m->indexUnk49Unk1 = op49data->unkF;
	if (m->indexUnk49Unk1 < 0) {
		if (m49->count2 == 0) {
			m->indexUnk49Unk1 = 0;
		} else {
			m->indexUnk49Unk1 = m49->data2[_rnd.getMstNextNumber(m->rnd_m49)];
		}
	}
	assert((uint32_t)m->indexUnk49Unk1 < m49->count1);
	m->m49Unk1 = &m49->data1[m->indexUnk49Unk1];
	m->flags4B = screen;
	if (a > b) {
		m->unk64 = b;
		m->unk68 = a;
	} else {
		m->unk64 = a;
		m->unk68 = b;
	}
	if (c > d) {
		m->unk6C = d;
		m->unk70 = c;
	} else {
		m->unk6C = c;
		m->unk70 = d;
	}
	switch (screen + 4) {
	case 1: {
			m->unkE4 = 255;
			const uint8_t _al = m->unk8[946];
			if (_al & 4) {
				t->run = &Game::runTask_unk10;
			} else if (_al & 2) {
				t->run = &Game::runTask_unk8;
			} else {
				t->run = &Game::runTask_unk6;
			}
			if (m->unk68 <= 0) {
				m->flags4B = 255;
				if (m->xMstPos < _mstAndyLevelPosX) {
					m->unk64 = -m->unk64;
					m->unk68 = -m->unk68;
				}
			}
			if ((_al & 2) != 0 && m->unk70 <= 0) {
				m->flags4B = 255;
				if (m->yMstPos < _mstAndyLevelPosY) {
					m->unk6C = -m->unk70;
					m->unk70 = -m->unk6C;
				}
			}
			// goto 41DB34
		}
		break;
	case 0: {
			const uint8_t _al = m->unkF8;
			if (_al & 8) {
				b = -b;
				a = -a;
				m->unk64 = b;
				m->unk68 = a;
			} else if (_al & 2) {
				m->unk64 = a;
				m->unk68 = b;
			} else {
				m->unk64 = 0;
				m->unk68 = 0;
			}
			if (_al & 1) {
				c = -c;
				d = -d;
				m->unk6C = d;
				m->unk70 = c;
			} else if (_al & 4) {
				m->unk6C = c;
				m->unk70 = d;
			} else {
				m->unk6C = 0;
				m->unk70 = 0;
			}
		}
		// fall-through
	case 2:
		if (m->unk8[946] & 4) {
			t->run = &Game::runTask_unk9;
		} else if (m->unk8[946] & 2) {
			t->run = &Game::runTask_unk7;
		} else {
			t->run = &Game::runTask_unk5;
		}
		m->unk64 += m->xMstPos;
		m->unk68 += m->xMstPos;
		m->unk6C += m->yMstPos;
		m->unk70 += m->yMstPos;
		break;
	case 3:
		if (m->unk8[946] & 4) {
			t->run = &Game::runTask_unk10;
		} else if (m->unk8[946] & 2) {
			t->run = &Game::runTask_unk8;
		} else {
			t->run = &Game::runTask_unk6;
		}
		break;
	default:
		if (m->unk8[946] & 4) {
			t->run = &Game::runTask_unk9;
		} else if (m->unk8[946] & 2) {
			t->run = &Game::runTask_unk7;
		} else {
			t->run = &Game::runTask_unk5;
		}
		m->unk64 += _res->_mstPointOffsets[screen].xOffset;
		m->unk68 += _res->_mstPointOffsets[screen].xOffset;
		m->unk6C += _res->_mstPointOffsets[screen].yOffset;
		m->unk70 += _res->_mstPointOffsets[screen].yOffset;
		break;
	}
// 41BD34
	m->unk74 = m->unk64;
	m->unk7C = m->unk68;
	m->unk78 = m->unk6C;
	m->flagsA8[2] = 255;
	m->unkC0 = -1;
	m->unkBC = -1;
	m->flagsA7 = 255;
	m->unk80 = m->unk70;
	const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	if ((ptr[2] & 5) == 0) {
		m->unk6C = m->yMstPos;
		m->unk78 = m->yMstPos;
		m->unk70 = m->yMstPos;
		m->unk80 = m->yMstPos;
	}
	if ((ptr[2] & 0xA) == 0) {
		m->unk64 = m->xMstPos;
		m->unk74 = m->xMstPos;
		m->unk68 = m->xMstPos;
		m->unk7C = m->xMstPos;
	}
// 41BDA9
	if (m->unk8[946] & 4) {
		m->flagsA8[3] = 255;
		m->unkAC = -1;
		m->unkB0 = -1;
		mstBoundingBoxClear(m, 1);
	}
// 41BE12
	uint8_t _dl = m->flags4B;
	if (_dl != 0xFC && (m->flagsA5 & 8) != 0 && (t->flags & 0x20) != 0 && m->unk18) {
		if (t->run != &Game::runTask_unk6 && t->run != &Game::runTask_unk8 && t->run != &Game::runTask_unk10) {
			if ((_dl == 0xFE && m->o16->screenNum != _currentScreen) || (_dl != 0xFE && _dl != _currentScreen)) {
				if (m->unk8[946] & 4) {
					mstBoundingBoxClear(m, 1);
				}
				return mstTaskStopMonsterObject1(t);
			}
		} else {
// 41BEF4
			const int x = MIN(_mstAndyScreenPosX, 255);
			if (x < 0) {
				c = x;
				d = x + 255;
			} else {
				c = -x;
				d = 255 - x;
			}
			const int y = MIN(_mstAndyScreenPosY, 191);
			int _ebp, var4;
			if (y < 0) {
				_ebp = y;
				var4 = y + 191;
			} else {
				_ebp = -y;
				var4 = 191 - y;
			}
			int _edx, _edi;
			if (_dl == 0xFD && m->xMstPos < _mstAndyLevelPosX) {
				_edx = -m->unk68;
				_edi = -m->unk64;
			} else {
				_edx = m->unk64;
				_edi = m->unk68;
			}
			uint8_t _bl = m->unk8[946] & 2;
			int _eax, _ecx;
			if (_bl != 0 && _dl == 0xFD && m->yMstPos < _mstAndyLevelPosY) {
				_eax = -m->unk70;
				_ecx = -m->unk6C;
			} else {
				_eax = m->unk6C;
				_ecx = m->unk70;
			}
			if (_edx < a || _edi < b || (_bl != 0 && (_eax < _ebp || _ecx > var4))) {
				if ((m->unk8[946] & 4) != 0) {
					mstBoundingBoxClear(m, 1);
				}
				return mstTaskStopMonsterObject1(t);
			}
		}
	}
// 41C038
	if (m->unk8[946] & 4) {
		const uint8_t *p = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
		const int x1 = m->xMstPos + (int8_t)p[0xC];
		const int x2 = x1 + p[0xE] - 1;
		const int y1 = m->yMstPos + (int8_t)p[0xD];
		const int y2 = y1 + p[0xF] - 1;
		if (mstBoundingBoxCollides2(m->monster1Index, x1, y1, x2, y2) > 1) {
// 41C1BB
			m->indexUnk49Unk1 = 0;
			m->m49Unk1 = &m->m49->data1[0];
			m->flagsA8[3] = 255;
			m->unkAC = -1;
			m->unkB0 = -1;
			mstBoundingBoxClear(m, 1);
			if (p[0xE] != 0) {
				t->flags &= ~0x80;
				executeMstUnk1(t);
				return 0;
			}
		} else {
			m->flagsA8[0] = mstBoundingBoxUpdate(0xFF, m->monster1Index, x1, y1, x2, y2);
		}
	}
// 41C17B
	t->flags &= ~0x80;
	if (m->unk8[946] & 2) {
		if (t->run == &Game::runTask_unk10) {
			executeMstUnk4(m);
			executeMstUnk8(m);
		} else if (t->run == &Game::runTask_unk8) {
			executeMstUnk4(m);
			mstSetVerticalHorizontalBounds(m);
		} else if (t->run == &Game::runTask_unk9) {
			executeMstUnk8(m);
		} else {
			mstSetVerticalHorizontalBounds(m);
		}
// 41C281
		uint8_t _cl, _dl;
		if (_mstLut1[m->flags4A] & 1) {
			_cl = m->m49Unk1->unkE;
			_dl = m->m49Unk1->unkF;
		} else {
			_cl = m->m49Unk1->unkC;
			_dl = m->m49Unk1->unkD;
		}
		if (_xMstPos2 < _cl && _yMstPos2 < _dl && !executeMstUnk6(m)) {
// 41C2E6
			goto l41C2E6;
		}
		// goto l41C3FD;
	} else {
// 41C355
		if (t->run == &Game::runTask_unk6) {
			if (m->flags4B == 0xFD && m->xMstPos < _mstAndyLevelPosX) {
				m->unk74 = _mstAndyLevelPosX - m->unk68;
				m->unk7C = _mstAndyLevelPosX - m->unk64;
			} else {
				m->unk74 = _mstAndyLevelPosX + m->unk64;
				m->unk7C = _mstAndyLevelPosX + m->unk68;
			}
		}
// 41C399
		mstSetHorizontalBounds(m);
		while (_xMstPos2 < m->m49Unk1->unkC) {
			if (--m->indexUnk49Unk1 < 0) {
				goto l41C2E6;
				break;
			}
			m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
		}
	}
// 41C3FD
	if (m->flags4A) {
		return (this->*(t->run))(t);
	}
// 41C2E6
l41C2E6:
	if (m->unk8[946] & 4) {
		mstBoundingBoxClear(m, 1);
	}
// 41C33C
	t->flags |= 0x80;
	executeMstUnk1(t);
	return 0;
}

void Game::mstOp52() {
	if (_m48Num == -1) {
		return;
	}
	MstUnk48 *m48 = &_res->_mstUnk48[_m48Num];
	int j = 0;
	for (int i = 0; i < m48->countUnk12; ++i) {
		MstUnk48Unk12 *m48unk12 = &m48->unk12[j];
		const uint8_t num = m48unk12->data->monster1Index;
		if (num != 255) {
			assert(num < kMaxMonsterObjects1);
			MonsterObject1 *m = &_monsterObjects1Table[num];
			disableMonsterObject1(m);
			if ((m->flagsA5 & 0x70) == 0) {
				assert(m->task->monster1 == m);
				Task *t = m->task;
				const int a = (m->o16->flags0 & 255) * 28;
				if (m->unk8[a] != 0) {
					if (t->run != &Game::runTask_unk1 && t->run != &Game::runTask_unk4 && t->run != &Game::runTask_unk2 && t->run != &Game::runTask_unk3 && t->run != &Game::runTask_unk5 && t->run != &Game::runTask_unk6 && t->run != &Game::runTask_unk7 && t->run != &Game::runTask_unk8 && t->run != &Game::runTask_unk9 && t->run != &Game::runTask_unk10) {
						m->flagsA5 = (m->flagsA5 & ~0xF) | 6;
						executeMstOp67Type2(m->task, 1);
					} else {
						m->o16->actionKeyMask = 0;
						m->o16->directionKeyMask = 0;
						t->run = &Game::runTask_unk11;
					}
				} else {
// 41D7D8
					m->flagsA5 = (m->flagsA5 & ~0xF) | 6;
					executeMstOp67Type2(m->task, 1);
				}
			}
		}
		++j;
	}
	_m48Num = -1;
}

bool Game::mstCollidesDirection(MstUnk48 *m48, uint8_t flag) {
	for (int i = 0; i < 2; ++i) {
		for (uint32_t j = 0; j < m48->count[i]; ++j) {
			uint32_t a = (i ^ flag); // * 32; // _edx
			uint32_t n = m48->data1[i][j]; // _eax
			if (_mstCollisionTable[a][n].count < m48->data2[i][j]) {
				return false;
			}
		}
	}

	uint8_t _op54Data[kMaxMonsterObjects1];
	memset(_op54Data, 0, sizeof(_op54Data));

	int var24 = 0;
	//int var28 = 0;
	//int var18 = 0;
	int _edi = 0;
	for (int i = 0; i < m48->countUnk12; ++i) {
		MstUnk48Unk12 *m12 = &m48->unk12[i];
		assert(m12->count == 1);
		MstUnk48Unk12Unk4 *m12u4 = m12->data;
		if (m12->unk0 != 0) {
			uint8_t var1C = m12u4->unk18;
			if (var1C != 2) {
				_edi = var1C;
			}
// 41DB81
l1:
			int var4C = _edi;

			int var8 = m12u4->unk8;
			int _ebx = var8; // xPos
			int var4 = m12u4->unkC;
			int _esi = var4; // yPos

			int _eax = _edi ^ flag;
			if (_eax == 1) {
				_ebx = -_ebx;
			}
			debug(kDebug_MONSTER, "mstCollidesDirection (unk0!=0) count:%d %d %d [%d,%d] screen:%d", m12->count, _ebx, _esi, _mstPosXmin, _mstPosXmax, m12u4->screenNum);
			if (_ebx >= _mstPosXmin && _ebx <= _mstPosXmax) {
				uint8_t var4D = _res->_mstMonsterInfos[m12u4->unk0 * kMonsterInfoDataSize + 946] & 2;
				if (var4D == 0 || (_esi >= _mstPosYmin && _esi <= _mstPosYmax)) {
// 41DC19
					MstCollision *varC = &_mstCollisionTable[_eax][m12u4->unk0];
					_ebx += _mstAndyLevelPosX;
					int var44 =  _ebx;
					_esi += _mstAndyLevelPosY;
					int var38 = _esi;
					int minDistY = 0x1000000;
					int minDistX = 0x1000000;
					int var34 = -1;
					int var10 = varC->count;
					//MstCollision *var20 = varC;
					for (int j = 0; j < var10; ++j) {
						MonsterObject1 *m = varC->monster1[j];
						if (_op54Data[m->monster1Index] == 0 && (m12u4->screenNum < 0 || m->o16->screenNum == m12u4->screenNum)) {
							int _ebp = var38 - m->yMstPos;
							int _eax = ABS(_ebp);
							int _esi = var44 - m->xMstPos;
							int _ecx = ABS(_esi);
							if (_ecx > m48->unk0 || _eax > m48->unk2) {
								continue;
							}
							if ((var8 || var4) && m->unk8[944] != 10 && m->unk8[944] != 16 && m->unk8[944] != 9) {
								if (_esi <= 0) {
									if (m->x2 > _ebx) { // var44
										continue;
									}
								} else {
									if (m->x1 < _ebx) { // var44
										continue;
									}
								}
								if (var4D != 0) { // vertical move
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
							if (_ecx <= minDistX && _eax <= minDistY) {
								minDistY = _eax;
								minDistX = _ecx;
								var34 = j;
							}
						}
					}
					if (var34 != -1) {
// 41DDEE
						const uint8_t num = varC->monster1[var34]->monster1Index;
						m12u4->monster1Index = num;
						_op54Data[num] = 1;
						++var24;
						continue;
					}
				}
			}
// 41DDA7
			if (var1C != 2 || var4C == 1) {
				return false;
			}
			_edi = 1;
			var4C = _edi;
			goto l1; // goto 41DB85
		}
// 41DE1E
		//var18 += 12;
		//++var28;
	}
	//var28 = _edi;
	//int var20 = 0;
	for (int i = _edi; i < m48->countUnk12; ++i) {
		MstUnk48Unk12 *m12 = &m48->unk12[i]; // var20
		assert(m12->count == 1);
		MstUnk48Unk12Unk4 *m12u4 = m12->data;
		if (m12->unk0 == 0) {
			uint8_t var1C = m12u4->unk18;
			m12u4->monster1Index = 255;
			int var4C = (var1C == 2) ? 0 : var1C;
			int _edx = var4C;
// 41DE98
l2:
			int var4 = m12u4->unk8;
			int _ebx = var4;
			int var8 = m12u4->unkC;
			int _esi = var8;
			int _eax = _edx ^ flag;
			if (_eax == 1) {
				_ebx = -_ebx;
			}
			debug(kDebug_MONSTER, "mstCollidesDirection (unk0==0) count:%d %d %d [%d,%d] screen:%d", m12->count, _ebx, _esi, _mstPosXmin, _mstPosXmax, m12u4->screenNum);
			if (_ebx >= _mstPosXmin && _ebx <= _mstPosXmax) {
				uint8_t var4D = _res->_mstMonsterInfos[m12u4->unk0 * kMonsterInfoDataSize + 946] & 2;
				if (var4D == 0 || (_esi >= _mstPosYmin && _esi <= _mstPosYmax)) {
// 41DF10
					MstCollision *varC = &_mstCollisionTable[_eax][m12u4->unk0];
					_ebx += _mstAndyLevelPosX;
					int var44 =  _ebx;
					_esi += _mstAndyLevelPosY;
					int var38 = _esi;
					int minDistY = 0x1000000;
					int minDistX = 0x1000000;
					int var34 = -1;
					int var10 = varC->count;
					for (int j = 0; j < var10; ++j) {
						MonsterObject1 *m = varC->monster1[j];
						if (_op54Data[m->monster1Index] == 0 && (m12u4->screenNum < 0 || m->o16->screenNum == m12u4->screenNum)) {
							int _ebp = var38 - m->yMstPos;
							int _eax = ABS(_ebp);
							int _esi = var44 - m->xMstPos;
							int _ecx = ABS(_esi);
							if (_ecx > m48->unk0 || _eax > m48->unk2) {
								continue;
							}
							if ((var8 || var4) && m->unk8[944] != 10 && m->unk8[944] != 16 && m->unk8[944] != 9) {
								if (_esi <= 0) {
									if (m->x2 > _ebx) { // var44
										continue;
									}
								} else {
									if (m->x1 < _ebx) { // var44
										continue;
									}
								}
								if (var4D != 0) { // vertical move
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
							if (_ecx <= minDistX && _eax <= minDistY) {
								minDistY = _eax;
								minDistX = _ecx;
								var34 = j;
							}
						}
					}
					if (var34 != -1) {
// 41E0BA
						const uint8_t num = varC->monster1[var34]->monster1Index;
						m12u4->monster1Index = num;
						_op54Data[num] = 1;
						++var24;
						continue;
					}
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
		//var20 += 12;
		//++var28;
	}
	return var24 != 0;
}

void Game::mstOp53(MstUnk48 *m) {
	if (_m48Num != -1) {
		return;
	}
	int x = MIN(_mstAndyScreenPosX, 255);
	if (_mstAndyScreenPosX < 0) {
		_mstPosXmin = x;
		_mstPosXmax = 255 + x;
	} else {
		_mstPosXmin = -x;
		_mstPosXmax = 255 - x;
	}
	int y = MIN(_mstAndyScreenPosY, 191);
	if (_mstAndyScreenPosY < 0) {
		_mstPosYmin = y;
		_mstPosYmax = 191 + y;
	} else {
		_mstPosYmin = -y;
		_mstPosYmax = 191 - y;
	}
	if (m->unk4 == 0) {
		if (mstCollidesDirection(m, 0)) {
			addChasingMonster(m, 0);
		}
		return;
	}
	int dir = _rnd.update() & 1;
	if (mstCollidesDirection(m, dir) && addChasingMonster(m, dir)) {
		return;
	}
	dir ^= 1;
	if (mstCollidesDirection(m, dir) && addChasingMonster(m, dir)) {
		return;
	}
}

void Game::mstOp54() {
	debug(kDebug_MONSTER, "mstOp54 %d %d %d", _m48Num, _m43Num2, _m43Num3);
	if (_m48Num != -1) {
		return;
	}
	MstUnk43 *m43 = 0;
	if (_mstFlags & 0x20000000) {
		if (_m43Num2 == -1) {
			return;
		}
		m43 = &_res->_mstUnk43[_m43Num2];
	} else {
		if (_m43Num3 == -1) {
			return;
		}
		m43 = &_res->_mstUnk43[_m43Num3];
		_m43Num2 = _m43Num1;
	}
	const int x = MIN(_mstAndyScreenPosX, 255);
	if (_mstAndyScreenPosX < 0) {
		_mstPosXmin = x;
		_mstPosXmax = 255 + x;
	} else {
		_mstPosXmin = -x;
		_mstPosXmax = 255 - x;
	}
	const int y = MIN(_mstAndyScreenPosY, 191);
	if (_mstAndyScreenPosY < 0) {
		_mstPosYmin = y;
		_mstPosYmax = 191 + y;
	} else {
		_mstPosYmin = -y;
		_mstPosYmax = 191 - y;
	}
	mstResetCollisionTable();
	if (m43->count2 == 0) {
		uint32_t indexUnk48 = m43->indexUnk48[0];
		assert(indexUnk48 != kNone);
		MstUnk48 *m48 = &_res->_mstUnk48[indexUnk48];
		if (m48->unk4 == 0) {
			if (mstCollidesDirection(m48, 0)) {
				addChasingMonster(m48, 0);
			}
		} else {
			uint8_t flag = _rnd.update() & 1;
			if (mstCollidesDirection(m48, flag) && addChasingMonster(m48, flag)) {
			} else {
				flag ^= 1;
				if (mstCollidesDirection(m48, flag)) {
					addChasingMonster(m48, flag);
				}
			}
		}
// 41E36E
		if (_m48Num == -1) {
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
			uint8_t code = m43->data2[i];
			if ((code & 0x80) == 0) {
				code &= 0x7F;
				var4 = 1;
				if (_mstOp54Table[code] == 0) {
					_mstOp54Table[code] = 1;
					uint32_t indexUnk48 = m43->indexUnk48[code];
					assert(indexUnk48 != kNone);
					MstUnk48 *m48 = &_res->_mstUnk48[indexUnk48];
					if (m48->unk4 == 0) {
						if (mstCollidesDirection(m48, 0) && addChasingMonster(m48, 0)) {
							break; // goto 41E494;
						}
					} else {
						int flag = _rnd.update() & 1;
						if (mstCollidesDirection(m48, flag) && addChasingMonster(m48, flag)) {
							break; // goto 41E494;
						}
						flag ^= 1;
						if (mstCollidesDirection(m48, flag) && addChasingMonster(m48, flag)) {
							break; // goto 41E494;
						}
					}
				}
			}
		}
// 41E494
		if (_m48Num != -1) {
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

int Game::mstOp56_specialAction(Task *t, int code, int num) {
	assert(num < _res->_mstHdr.unk0x78);
	debug(kDebug_MONSTER, "mstOp56_specialAction code %d", code);
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
			if (t->monster2) {
				o = t->monster2->o;
			} else if (t->monster1) {
				o = t->monster1->o16;
			}
			if (_res->_mstOp56Data[num].unkC != 6 && o) {
				LvlObject *tmpObject = t->monster1->o16;
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
		if (_mstAndyRectNum != 255) {
			_mstBoundingBoxesTable[_mstAndyRectNum].monster1Index = 255;
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
			if (t->monster2) {
				o = t->monster2->o;
			} else if (t->monster1) {
				o = t->monster1->o16;
			}
			if (_res->_mstOp56Data[num].unkC != 6 && o) {
				LvlObject *tmpObject = t->monster1->o16;
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
		_mstAndyRectNum = mstBoundingBoxUpdate(_mstAndyRectNum, 0xFE, _mstAndyLevelPosX, _mstAndyLevelPosY, _mstAndyLevelPosX + _andyObject->width - 1, _mstAndyLevelPosY + _andyObject->height - 1);
		break;
	case 2: {
			LvlObject *o = t->monster1->o16;
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
			MonsterObject2 *m = t->monster2;
			const int type = _res->_mstOp56Data[num].unkC;
			m->x1 = getTaskVar(t, _res->_mstOp56Data[num].unk0, (type >> 0xC) & 15);
			m->y1 = getTaskVar(t, _res->_mstOp56Data[num].unk4, (type >> 0x8) & 15);
			m->x2 = getTaskVar(t, _res->_mstOp56Data[num].unk8, (type >> 0x4) & 15);
			m->y2 = getTaskVar(t, type >> 16                  ,  type         & 15);
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
			if (t->monster2) {
				o = t->monster2->o;
			} else if (t->monster1) {
				o = t->monster1->o16;
			}
			if (screenNum < 0) {
				if (screenNum == -2) {
					screenNum = o->screenNum;
					if (t->monster2) {
						xPos += t->monster2->xMstPos;
						yPos += t->monster2->yMstPos;
					} else if (t->monster1) {
						xPos += t->monster1->xMstPos;
						yPos += t->monster1->yMstPos;
					}
				} else if (screenNum == -1) {
					xPos += _mstAndyLevelPosX;
					yPos += _mstAndyLevelPosY;
					screenNum = _currentScreen;
				} else {
// 4114B3
					xPos += o->posTable[6].x - o->posTable[7].x;
					yPos += o->posTable[6].y - o->posTable[7].y;
					if (t->monster2) {
						xPos += t->monster2->xMstPos;
						yPos += t->monster2->yMstPos;
					} else {
						assert(t->monster1);
						xPos += t->monster1->xMstPos;
						yPos += t->monster1->yMstPos;
					}
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
				if (t->monster2) {
					mstTaskSetScreenPosition(t);
				} else {
					mstTaskUpdateScreenPosition(t);
				}
			} else if (code == 14) {
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
			if (t->monster2) {
				o = t->monster2->o;
			} else if (t->monster1) {
				o = t->monster1->o16;
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
				if (t->monster2) {
					o = t->monster2->o;
				} else if (t->monster1) {
					o = t->monster1->o16;
				}
			}
			const int pos = _res->_mstOp56Data[num].unk8;
			assert(pos < 8);
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
	case 19: {
			_andyActionKeyMaskAnd    = _res->_mstOp56Data[num].unk0 & 255;
			_andyActionKeyMaskOr     = _res->_mstOp56Data[num].unk4 & 255;
			_andyDirectionKeyMaskAnd = _res->_mstOp56Data[num].unk8 & 255;
			_andyDirectionKeyMaskOr  = _res->_mstOp56Data[num].unkC & 255;
		}
		break;
	case 20: {
			_mstCurrentActionKeyMask = 0;
			t->monster1->flagsA6 |= 2;
			t->run = &Game::runTask_idle;
			t->monster1->o16->actionKeyMask = _mstCurrentActionKeyMask;
			t->monster1->o16->directionKeyMask = _andyObject->directionKeyMask;
			return 1;
		}
		break;
	case 21: {
			t->monster1->flagsA6 &= ~2;
			t->monster1->o16->actionKeyMask = 0;
			t->monster1->o16->directionKeyMask = 0;
		}
		break;
	case 26: {
			int screenNum = _res->_mstOp56Data[num].unk8;
			if (screenNum < -1 && !t->monster1) {
				break;
			}
			if (screenNum == -1) {
				screenNum = _currentScreen;
			} else if (screenNum < -1) {
				screenNum = t->monster1->o16->screenNum;
			}
			if (screenNum >= _res->_mstHdr.pointsCount) {
				screenNum = _res->_mstHdr.pointsCount - 1;
			}
			int _ebp = _res->_mstPointOffsets[screenNum].xOffset;
			int _edx = _res->_mstPointOffsets[screenNum].yOffset;
			int _eax = _res->_mstOp56Data[num].unkC * 256;
			int _edi = _ebp + 256;
			_ebp -= _eax;
			_edi += _eax;
			int count = 0;
			for (int i = 0; i < kMaxMonsterObjects1; ++i) {
				MonsterObject1 *m = &_monsterObjects1Table[i];
				if (!m->m46) {
					continue;
				}
				if (m->xMstPos < _ebp || m->xMstPos > _edi) {
					continue;
				}
				if (m->yMstPos < _edx || m->yMstPos > _edx + 192) {
					continue;
				}
				switch (_res->_mstOp56Data[num].unk0) {
				case 0:
					_eax = _res->_mstOp56Data[num].unk4;
					if (m->m46 == &_res->_mstUnk46[_eax]) {
						++count;
					}
					break;
				case 1:
					_eax = _res->_mstOp56Data[num].unk4;
					if (m->unk8 == &_res->_mstMonsterInfos[_eax * kMonsterInfoDataSize]) {
						++count;
					}
					break;
				case 2:
					_eax = _res->_mstOp56Data[num].unk4;
					if (m->unk8[944] == _eax) {
						++count;
					}
					break;
				}
			}
			_mstOp56Counter = count;
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
		// no-op
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
		warning("Unhandled opcode %d in mstOp56_specialAction", code);
		break;
	}
	return 0;
}

static void initCrackSprite(CrackSprite *s, const uint8_t *p) {
	s->screenNum = p[0];
	s->initData1 = p[1];
	s->initData2 = p[2];
	s->initData3 = p[3];
	s->initData4 = READ_LE_UINT32(p + 4);
	s->initData8 = p[8];
	s->initData9 = p[9];
	s->initDataA = p[0xA];
	s->initDataB = p[0xB];
	s->initDataC = p[0xC];
	s->initDataD = p[0xD];
	s->initDataE = p[0xE];
	s->initDataF = p[0xF];
}

void Game::mstOp57_addCrackSprite(int x, int y, int screenNum) {
	bool found = false;
	int spriteNum = 0;
	for (int i = 0; i < 6; ++i) {
		if (_crackSpritesTable[i].screenNum == screenNum) {
			found = true;
			break;
		}
		if (_crackSpritesTable[i].screenNum == 0xFF) {
			break;
		}
		++spriteNum;
	}
	if (!found) {
		found = true;
		if (spriteNum == 6) {
			++_crackSpritesCount;
			if (_crackSpritesCount >= spriteNum) {
				_crackSpritesCount = 0;
				spriteNum = 0;
			} else {
				spriteNum = _crackSpritesCount - 1;
			}
		} else {
			spriteNum = _crackSpritesCount;
		}
// 40347C
		switch (_currentLevel) {
		case 2:
			initCrackSprite(&_crackSpritesTable[spriteNum], _pwr1_spritesData + screenNum * 16);
			break;
		case 3:
			initCrackSprite(&_crackSpritesTable[spriteNum], _isld_spritesData + screenNum * 16);
			break;
		case 4:
			initCrackSprite(&_crackSpritesTable[spriteNum], _lava_spritesData + screenNum * 16);
			break;
		case 6:
			initCrackSprite(&_crackSpritesTable[spriteNum], _lar1_spritesData + screenNum * 16);
			break;
		default:
			warning("mstOp57 unhandled level %d", _currentLevel);
			return;
		}
	}
// 4034D2
	const int dx = x - _crackSpritesTable[spriteNum].initData2;
	const int dy = y + 15 - _crackSpritesTable[spriteNum].initData3;
	spriteNum = _rnd.getSeed() & 3;
	if (spriteNum == 0) {
		spriteNum = 1;
	}
	static const uint8_t byte_451248[32] = {
		0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x06, 0x06, 0x06, 0x08, 0x08, 0x08, 0x0A,
		0x0A, 0x0A, 0x0C, 0x0C, 0x0C, 0x0E, 0x0E, 0x0E, 0x10, 0x10, 0x10, 0x12, 0x12, 0x12, 0x14, 0x14,
	};
// 403515
	const int pos = byte_451248[(dx >> 3) & 31];
	const int num = dy >> 5;
	if ((_crackSpritesTable[spriteNum].flags[num] & (3 << pos)) == 0) {
		if (addLvlObjectToList3(20)) {
			LvlObject *o = _lvlObjectsList3;
			o->flags0 = _andyObject->flags0;
			o->flags1 = _andyObject->flags1;
			o->screenNum = screenNum;
			o->flags2 = 0x1007;
			o->anim = 0;
			o->frame = 0;
			setupLvlObjectBitmap(o);
			setLvlObjectPosRelativeToPoint(o, 7, x, y);
		}
	}
	_crackSpritesTable[spriteNum].flags[num] |= (spriteNum << pos);
}

void Game::mstOp58_addLvlObject(Task *t, int num) {
	const MstOp58Data *dat = &_res->_mstOp58Data[num];
	const int mask = dat->unkE;
	int xPos = getTaskVar(t, dat->indexVar1, (mask >> 8) & 15); // _ebx
	int yPos = getTaskVar(t, dat->indexVar2, (mask >> 4) & 15); // _ebp
	const int type = getTaskVar(t, dat->unkC, mask & 15) & 255; // _eax
	LvlObject *o = 0;
	if (t->monster2) {
		o = t->monster2->o;
	} else if (t->monster1) {
		o = t->monster1->o16;
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
		xPos += _mstAndyScreenPosX; // _ebx
		yPos += _mstAndyScreenPosY; // _ebp
		screen = _currentScreen;
	}
	uint16_t flags = (dat->unk6 == -1 && o) ? o->flags2 : 0x3001;
	o = addLvlObject(2, xPos, yPos, screen, dat->unk8, dat->unk4, dat->unkB, flags, dat->unk9, dat->unkA);
	if (o) {
		o->dataPtr = 0;
	}
}

// mstOp59_addShootSpecialPowers
void Game::mstOp59_1(int x, int y, int screenNum, int type, uint16_t flags) {
	LvlObject *o = addLvlObjectToList0(3);
	if (o) {
		o->dataPtr = _shootLvlObjectDataList;
		if (_shootLvlObjectDataList) {
			_shootLvlObjectDataList = _shootLvlObjectDataList->nextPtr;
			memset(o->dataPtr, 0, sizeof(ShootLvlObjectData));
		}
		ShootLvlObjectData *s = (ShootLvlObjectData *)o->dataPtr;
		assert(s);
		o->callbackFuncPtr = &Game::lvlObjectSpecialPowersCallback;
		s->unk1 = type;
		s->unk0 = 0;
		s->counter = 17;
		s->dxPos = (int8_t)_byte_43E660[type * 2];
		s->dyPos = (int8_t)_byte_43E660[type * 2 + 1];
		o->anim = _byte_43E730[type * 2];
		o->flags1 = ((_byte_43E730[type * 2 + 1] & 3) << 4) | (o->flags1 & ~0x0030);
		o->frame = 0;
		o->flags2 = o->flags1;
		o->screenNum = screenNum;
		setupLvlObjectBitmap(o);
		setLvlObjectPosRelativeToPoint(o, 6, x, y);
	}
}

void Game::mstOp59_addShootFireball(int x, int y, int screenNum, int pos, int type, uint16_t flags) {
	LvlObject *o = addLvlObjectToList2(7);
	if (o) {
		o->dataPtr = _shootLvlObjectDataList;
		if (_shootLvlObjectDataList) {
			_shootLvlObjectDataList = _shootLvlObjectDataList->nextPtr;
			memset(o->dataPtr, 0, sizeof(ShootLvlObjectData));
		}
		ShootLvlObjectData *s = (ShootLvlObjectData *)o->dataPtr;
		assert(s);
		s->unk1 = pos;
		const uint8_t *_ecx;
		if (type >= 7) {
			s->dxPos = (int8_t)_byte_43E6F0[type * 2];
			s->dyPos = (int8_t)_byte_43E6F0[type * 2 + 1];
			s->counter = 33;
			_ecx = &_byte_43E760[type * 2];
		} else {
			s->dxPos = (int8_t)_byte_43E6E0[type * 2];
			s->dyPos = (int8_t)_byte_43E6E0[type * 2 + 1];
			s->counter = 39;
			_ecx = &_byte_43E740[type * 2];
		}
		s->unk0 = type;
		o->anim = _ecx[0];
		o->screenNum = screenNum;
		o->flags1 = ((_ecx[1] & 3) << 4) | (o->flags1 & ~0x0030);
		o->flags2 = flags;
		o->frame = 0;
		setupLvlObjectBitmap(o);
		setLvlObjectPosRelativeToPoint(o, 6, x - s->dxPos, y - s->dyPos);
	}
}

void Game::executeMstUnk1(Task *t) {
	MonsterObject1 *m = t->monster1;
	t->run = &Game::mstSm_main;
	m->o16->actionKeyMask = 0;
	m->o16->directionKeyMask = 0;
	if ((m->flagsA5 & 4) != 0 && (m->flagsA5 & 0x28) == 0) {
		switch (m->flagsA5 & 7) {
		case 5:
			m->flagsA5 = (m->flagsA5 & ~6) | 1;
			if (!updateMonsterObject1Position(m)) {
				initMonsterObject1(m);
			}
			prepareMstTask(t);
			break;
		case 6:
			m->flagsA5 &= ~7;
			if (mstSetCurrentPos(m, m->xMstPos, m->yMstPos) == 0) {
				m->flagsA5 |= 1;
				if (!updateMonsterObject1Position(m)) {
					initMonsterObject1(m);
				}
				uint32_t indexUnk35 = m->m44Unk1->indexUnk35_20;
				if (indexUnk35 != kNone) {
					m->m35 = &_res->_mstUnk35[indexUnk35];
				}
				prepareMstTask(t);
			} else {
				m->flagsA5 |= 2;
				if (!updateMonsterObject1Position(m)) {
					initMonsterObject1(m);
				}
				prepareMstTask(t);
			}
			break;
		}
	} else {
		m->flagsA5 &= ~4;
		updateMonsterObject1Position(m);
	}
}

bool Game::mstSetCurrentPos(MonsterObject1 *m, int x, int y) {
	_mstCurrentPosX = x; // _esi
	_mstCurrentPosY = y;
	const uint8_t *ptr = m->unk8;
	const int32_t a = READ_LE_UINT32(ptr + 900);
	int _ecx = _mstAndyLevelPosX - a; // x1
	int _edi = _mstAndyLevelPosX + a; // x2
	if (ptr[946] & 2) {
		int _ebx = _mstAndyLevelPosY - a; // y1
		int _ebp = _mstAndyLevelPosY + a; // y2
		if (x > _ecx && x < _edi && y > _ebx && y < _ebp) {
			if (ABS(x - _mstAndyLevelPosX) > ABS(y - _mstAndyLevelPosY)) {
				if (x >= _mstAndyLevelPosX) {
					_mstCurrentPosX = _edi;
				} else {
					_mstCurrentPosX = _ecx;
				}
			} else {
				if (y >= _mstAndyLevelPosY) {
					_mstCurrentPosY = _ebp;
				} else {
					_mstCurrentPosY = _ebx;
				}
			}
			return false;
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
			return false;
		}
		return true;
	}
// 419EA7
	if (x > _ecx && x < _edi) {
		if (x >= _mstAndyLevelPosX) {
			_mstCurrentPosX = _edi;
		} else {
			_mstCurrentPosX = _ecx;
		}
		return false;
	}
	const int32_t b = READ_LE_UINT32(ptr + 896);
	_ecx -= b;
	_edi += b;
	if (x < _ecx) {
		_mstCurrentPosX = _ecx;
		return false;
	} else if (x > _edi) {
		_mstCurrentPosX = _edi;
		return false;
	}
	return true;
}

void Game::mstSetHorizontalBounds(MonsterObject1 *m) {
	Task *t = m->task;
	t->flags &= ~0x80;
	int x = m->xMstPos;
	if (x < m->unk74) {
		_xMstPos1 = x = m->unk7C;
		if ((m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0 && x > m->unk84) {
			t->flags |= 0x80;
			x = m->unk84;
		}
		if (x > m->x1) {
			t->flags |= 0x80;
			x = m->x1;
		}
		_xMstPos2 = x - m->xMstPos;
		m->flags4A = 2; // go right
	} else if (x > m->unk7C) {
// 41A2AA
		_xMstPos1 = x = m->unk74;
		if ((m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0 && x < m->unk88) {
			t->flags |= 0x80;
			x = m->unk88;
		}
		if (x < m->x2) {
			t->flags |= 0x80;
			x = m->x2;
		}
		_xMstPos2 = m->xMstPos - x;
		m->flags4A = 8; // go left
	} else {
// 41A2FC
		_xMstPos1 = x;
		_xMstPos2 = 0;
		m->flags4A = 0;
	}
}

void Game::mstResetCollisionTable() {
	const int count = MIN(_res->_mstHdr.unk0x3C, 32);
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < count; ++j) {
			_mstCollisionTable[i][j].count = 0;
		}
	}

	for (int i = 0; i < kMaxMonsterObjects1; ++i) {
		MonsterObject1 *m = &_monsterObjects1Table[i];
		if (!m->m46) {
			continue;
		}
		uint8_t _bl = m->flagsA5;
		if ((_bl & 2) != 0 || ((_bl & 1) != 0 && m->m35 == &_res->_mstUnk35[m->m44Unk1->indexUnk35_20])) {
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
			assert(m->task->monster1 == m);
			if ((_al & 8) == 0 || m->unk8[945] != 0) {
				const uint32_t offset = m->unk8 - _res->_mstMonsterInfos;
				assert(offset % kMonsterInfoDataSize == 0);
				const uint32_t _ecx = offset / kMonsterInfoDataSize;
				assert(_ecx < 32);
				_al = m->xMstPos < _mstAndyLevelPosX;
				const int count = _mstCollisionTable[_al][_ecx].count;
				_mstCollisionTable[_al][_ecx].monster1[count] = m;
				++_mstCollisionTable[_al][_ecx].count;
			}
		}
	}
}

// mstSetStateMain
void Game::executeMstUnk13(Task *t) {
	t->run = &Game::mstSm_main;
	LvlObject *o = 0;
	if (t->monster1) {
		o = t->monster1->o16;
	} else if (t->monster2) {
		o = t->monster2->o;
	}
	if (o) {
		o->actionKeyMask = 0;
		o->directionKeyMask = 0;
	}
}

bool Game::executeMstUnk14(MonsterObject1 *m, int x, int y, uint8_t dir) {
	if ((m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0 && !mstSetCurrentPos(m, x, y)) {
		return true;
	}
	if ((dir & 8) != 0 && x < m->x2) {
		return true;
	}
	if ((dir & 2) != 0 && x > m->x1) {
		return true;
	}
	if ((dir & 1) != 0 && y < m->y2) {
		return true;
	}
	if ((dir & 4) != 0 && y > m->y1) {
		return true;
	}
	return false;
}

int Game::executeMstUnk23(Task *t) {
	MonsterObject1 *m = t->monster1;
	m->flagsA5 = (m->flagsA5 & ~0xF) | 6;
	return executeMstOp67Type2(t, 1);
}

int Game::executeMstOp67Type1(Task *t) {
	t->flags &= ~0x80;
	MonsterObject1 *m = t->monster1;
	m->flagsA5 = (m->flagsA5 & ~2) | 5;
	initMonsterObject1(m);
	int y = 0;
	int x = 0;
	int var14 = 0;
	if (m->unk8[946] & 2) {
		m->unkC0 = -1;
		m->unkBC = -1;
		m->flagsA8[2] = 255;
		m->flagsA7 = 255;
		y = m->m44Unk1->y2;
		if (m->yMstPos < m->m44Unk1->y2 || m->yMstPos > m->m44Unk1->y1) {
			var14 = 1;
		}
	}
// 41C4B6
	x = m->m44Unk1->x2;
	if (m->unk8[946] & 4) {
// 41C4C9
		m->flagsA8[3] = 255;
		m->unkAC = -1;
		m->unkB0 = -1;
		mstBoundingBoxClear(m, 1);
	}
// 41C531
	if (!var14 && m->xMstPos >= x && m->xMstPos <= m->m44Unk1->x1) {
		executeMstUnk1(t);
		return 0;
	}
// 41C559
	const uint32_t indexUnk36 = m->m44Unk1->indexUnk36_28;
	assert(indexUnk36 != kNone);
	MstUnk36 *m36 = &_res->_mstUnk36[indexUnk36];
	const uint32_t indexUnk49 = m36->indexUnk49;
	assert(indexUnk49 != kNone);
	MstUnk49 *m49 = &_res->_mstUnk49[indexUnk49];
	m->m49 = m49;
	m->indexUnk49Unk1 = m36->unk4;
	if (m->indexUnk49Unk1 < 0) {
		if (m49->count2 == 0) {
			m->indexUnk49Unk1 = 0;
		} else {
			m->indexUnk49Unk1 = m49->data2[_rnd.getMstNextNumber(m->rnd_m49)];
		}
	}
// 41C5B1
	assert((uint32_t)m->indexUnk49Unk1 < m49->count1);
	m->m49Unk1 = &m49->data1[m->indexUnk49Unk1];
	int _edi = (m->m44Unk1->x1 - x) / 4;
	m->unk64 = x + _edi;
	m->unk68 = m->m44Unk1->x1 - _edi;
	if (_edi != 0) {
		_edi = _rnd.update() % _edi;
	}
	m->unk68 -= _edi;
	m->unk64 += _edi;
	m->unk74 = m->unk64;
	m->unk7C = m->unk68;
	const uint8_t *ptr1 = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	if ((ptr1[2] & 0xA) == 0) {
		m->unk64 = m->unk74 = m->unk68 = m->unk7C = m->xMstPos;
	}
// 41C62E
	if (m->unk8[946] & 2) {
		int _edi = (m->m44Unk1->y1 - y) / 4;
		m->unk70 = m->m44Unk1->y1 - _edi;
		m->unk6C = y + _edi;
		if (_edi != 0) {
			_edi = _rnd.update() % _edi;
		}
		m->unk6C += _edi;
		m->unk78 = m->unk6C;
		m->unk70 -= _edi;
		m->unk80 = m->unk70;
		const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
		if ((ptr[2] & 5) == 0) {
			m->unk6C = m->unk78 = m->unk70 = m->unk80 = m->yMstPos;
		}
		if (m->unk8[946] & 4) {
			executeMstUnk8(m);
		} else {
			mstSetVerticalHorizontalBounds(m);
		}
		warning("executeMstOp67Type1 41C6C0");
	}
// 41C712
	mstSetHorizontalBounds(m);
	_edi = 1;
	while (_xMstPos2 < m->m49Unk1->unkC) {
		if (--m->indexUnk49Unk1 < 0) {
			_edi = 0;
			break;
		}
		m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
	}
// 41C774
	if (_xMstPos2 <= 0 && ((m->unk8[946] & 2) == 0 || _yMstPos2 <= 0)) {
		if (m->unk8[946] & 4) {
			mstBoundingBoxClear(m, 1);
		}
		executeMstUnk1(t);
		return 0;
	} else {
// 41C7F6
		if (_edi != 0) {
			if (_xMstPos2 >= m->m49->unk14 || ((m->unk8[946] & 2) != 0 && _yMstPos2 >= m->m49->unk15)) {
// 41C833
				const uint8_t *p = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
				if ((m->unk8[946] & 4) != 0 && p[0xE] != 0 && m->flagsA8[0] == 255) {
					const int x1 = m->xMstPos + (int8_t)p[0xC];
					const int x2 = x1 + p[0xE] - 1;
					const int y1 = m->yMstPos + (int8_t)p[0xD];
					const int y2 = y1 + p[0xF] - 1;
					if (mstBoundingBoxCollides2(m->monster1Index, x1, y1, x2, y2) > 1) {
// 41CA54
						m->indexUnk49Unk1 = 0;
						m->m49Unk1 = &m->m49->data1[0];
						m->flagsA8[3] = 255;
						m->unkAC = -1;
						m->unkB0 = -1;
						mstBoundingBoxClear(m, 1);
						if (p[0xE] != 0) {
							t->flags &= ~0x80;
							executeMstUnk1(t);
							return 0;
						}
					} else {
						m->flagsA8[0] = mstBoundingBoxUpdate(0xFF, m->monster1Index, x1, y1, x2, y2);
					}
				}
// 41C976
				if (_xMstPos2 >= m36->unk8 || ((m->unk8[946] & 2) != 0 && _yMstPos2 >= m36->unk8)) {
					m->indexUnk49Unk1 = m->m49->count1 - 1;
					m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
					if (m->unk8[946] & 4) {
						m->flagsA8[3] = 255;
						m->unkAC = -1;
						m->unkB0 = -1;
						mstBoundingBoxClear(m, 1);
					}
				}
// 41CA2D
				if (m->unk8[946] & 4) {
					t->run = &Game::runTask_unk9;
				} else if (m->unk8[946] & 2) {
					t->run = &Game::runTask_unk7;
				} else {
					t->run = &Game::runTask_unk5;
				}
				return (this->*(t->run))(t);
			} else if (m->unk8[946] & 4) {
// 41CB2F
				mstBoundingBoxClear(m, 1);
			}
		}
// 41CB85
		t->flags |= 0x80;
		executeMstUnk1(t);
		return -1;
	}
}

int Game::executeMstOp67Type2(Task *t, int flag) {
	debug(kDebug_MONSTER, "executeMstOp67Type2 t %p flag %d", t, flag);
	t->flags &= ~0x80;
	MonsterObject1 *m = t->monster1;
	m->flagsA5 = (m->flagsA5 & ~1) | 6;
	initMonsterObject1(m);
	const int i = m->m44Unk1->indexUnk36_32;
	assert(i >= 0 && i < _res->_mstHdr.unk0x10);
	MstUnk36 *m36 = &_res->_mstUnk36[i];
	const int j = m36->indexUnk49;
	assert(j >= 0 && j < _res->_mstHdr.unk0x40);
	MstUnk49 *m49 = &_res->_mstUnk49[j];
	m->m49 = m49;
	if (flag != 0) {
		m->indexUnk49Unk1 = m49->count1 - 1;
	} else {
		m->indexUnk49Unk1 = m36->unk4;
	}
	if (m->indexUnk49Unk1 < 0) {
		if (m49->count2 == 0) {
			m->indexUnk49Unk1 = 0;
		} else {
			m->indexUnk49Unk1 = m49->data2[_rnd.getMstNextNumber(m->rnd_m49)];
		}
	}
// 41CC44
	assert((uint32_t)m->indexUnk49Unk1 < m49->count1);
	m->m49Unk1 = &m49->data1[m->indexUnk49Unk1];
	m->flags4B = 0xFD;
	m->unkC0 = -1;
	m->unkBC = -1;
	m->flagsA8[2] = 255;
	m->flagsA7 = 255;
	if (mstSetCurrentPos(m, m->xMstPos, m->yMstPos)) {
		executeMstUnk1(t);
		return 0;
	}
// 41CCA3
	int _edi;
	const uint8_t *p = m->unk8;
	if (p[946] & 2) {
		m->unkE4 = 255;
		if (p[946] & 4) {
// 41CCC5
			warning("executeMstOp67Type2 t %p flag %d p[946] %x", t, flag, p[946]);
		}
// 41CD22
		m->unk64 = READ_LE_UINT32(m->unk8 + 900);
		m->unk68 = m->unk64 + READ_LE_UINT32(m->unk8 + 896);
		m->unk6C = READ_LE_UINT32(m->unk8 + 900);
		m->unk70 = m->unk6C + READ_LE_UINT32(m->unk8 + 896);
		m->unk74 = m->unk64;
		m->unk7C = m->unk68;
		m->unk78 = m->unk6C;
		m->unk80 = m->unk70;
		const uint8_t *ptr1 = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
		if ((ptr1[2] & 0x5) == 0) {
			m->unk6C = m->unk78 = m->unk70 = m->unk80 = m->yMstPos;
		}
		if ((ptr1[2] & 0xA) == 0) {
			m->unk64 = m->unk74 = m->unk68 = m->unk7C = m->xMstPos;
		}
		if (p[946] & 4) {
			executeMstUnk4(m);
			executeMstUnk8(m);
		} else {
			executeMstUnk4(m);
			mstSetVerticalHorizontalBounds(m);
		}
		_edi = 1;
		int8_t _cl, _dl;
		if (_mstLut1[m->flags4A] & 1) {
			_cl = m->m49Unk1->unkE;
			_dl = m->m49Unk1->unkF;
		} else {
			_cl = m->m49Unk1->unkC;
			_dl = m->m49Unk1->unkD;
		}
		if (_xMstPos2 < _cl && _yMstPos2 < _dl && !executeMstUnk6(m)) {
			_edi = 0;
		}
	} else {
// 41CE2B
		int32_t _ebp = READ_LE_UINT32(p + 900);
		int32_t _ecx = READ_LE_UINT32(p + 896);
		int r = _ecx / 4;
		m->unk64 = _ebp + r;
		m->unk68 = _ecx + _ebp - r;
		if (r != 0) {
			r = _rnd.update() % r;
		}
		m->unk64 += r;
		m->unk68 -= r;
		if (m->flags4B == 0 && m->xMstPos < _mstAndyLevelPosX) {
			m->unk74 = _mstAndyLevelPosX - m->unk68;
			m->unk7C = _mstAndyLevelPosX - m->unk64;
		} else {
// 41CEA1
			m->unk74 = _mstAndyLevelPosX + m->unk64;
			m->unk7C = _mstAndyLevelPosX + m->unk68;
		}
// 41CEB4
		mstSetHorizontalBounds(m);
		_edi = 1;
		while (_xMstPos2 < m->m49Unk1->unkC) {
			if (--m->indexUnk49Unk1 < 0) {
				_edi = 0;
				break;
			}
			m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
		}
	}
// 41CF17
	if (_xMstPos2 <= 0 && ((m->unk8[946] & 2) == 0 || _yMstPos2 <= 0)) {
		if (m->unk8[946] & 4) {
			mstBoundingBoxClear(m, 1);
		}
// 41CF86
		executeMstUnk1(t);
		return 0;
	}
// 41CF99
	if (_edi) {
		if (_xMstPos2 >= m->m49->unk14 || ((m->unk8[946] & 2) != 0 && (_yMstPos2 >= m->m49->unk15))) {
			const uint8_t *p = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
			if ((m->unk8[946] & 4) != 0 && p[0xE] != 0 && m->flagsA8[0] == 0xFF) {
				const int x1 = m->xMstPos + (int8_t)p[0xC];
				const int x2 = x1 + p[0xE] - 1;
				const int y1 = m->yMstPos + (int8_t)p[0xD];
				const int y2 = y1 + p[0xF] - 1;
				if (mstBoundingBoxCollides2(m->monster1Index, x1, y1, x2, y2) > 1) {
// 41D1F5
					m->indexUnk49Unk1 = 0;
					m->m49Unk1 = &m->m49->data1[0];
					m->flagsA8[3] = 255;
					m->unkAC = -1;
					m->unkB0 = -1;
					mstBoundingBoxClear(m, 1);
					if (p[0xE] != 0) {
						t->flags &= ~0x80;
						executeMstUnk1(t);
						return 0;
					}
				} else {
					m->flagsA8[0] = mstBoundingBoxUpdate(0xFF, m->monster1Index, x1, y1, x2, y2);
				}
			}
// 41D115
			if (_xMstPos2 <= m36->unk8 || ((m->unk8[946] & 2) != 0 && _yMstPos2 >= m36->unk8)) {
				m->indexUnk49Unk1 = m->m49->count1 - 1;
				m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
				if (m->unk8[946] & 4) {
					m->flagsA8[3] = 0xFF;
					m->unkAC = -1;
					m->unkB0 = -1;
					mstBoundingBoxClear(m, 1);
				}
			}
// 41D1CE
			if (m->unk8[946] & 4) {
				t->run = &Game::runTask_unk10;
			} else if (m->unk8[946] & 2) {
				t->run = &Game::runTask_unk8;
			} else {
				t->run = &Game::runTask_unk6;
			}
			return (this->*(t->run))(t);
		}
	}
// 41D2D2
	if (m->unk8[946] & 4) {
		warning("executeMstOp67Type2 41D2DE");
	}
// 41D328
	t->flags |= 0x80;
	executeMstUnk1(t);
	return -1;
}

void Game::mstOp67_addMonster(Task *currentTask, int x1, int x2, int y1, int y2, int screen, int type, int o_flags1, int o_flags2, int arg1C, int arg20, int arg24) {
	debug(kDebug_MONSTER, "mstOp67_addMonster pos %d,%d,%d,%d %d %d 0x%x 0x%x %d %d %d", y1, x1, y2, x2, screen, type, o_flags1, o_flags2, arg1C, arg20, arg24);
	if (o_flags2 == 0xFFFF) {
		LvlObject *o = 0;
		if (currentTask->monster1) {
			o = currentTask->monster1->o16;
		} else if (currentTask->monster2) {
			o = currentTask->monster2->o;
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
	MonsterObject2 *mo = 0; // _ebp
	MonsterObject1 *m = 0; // _esi

	if (arg1C != -128) {
		if (_mstVars[30] > 32) {
			_mstVars[30] = 32;
		}
		int count = 0;
		for (int i = 0; i < kMaxMonsterObjects1; ++i) {
			if (_monsterObjects1Table[i].m46) {
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
		for (int i = 0; i < kMaxMonsterObjects1; ++i) {
			if (!_monsterObjects1Table[i].m46) {
				m = &_monsterObjects1Table[i];
				break;
			}
		}
		if (!m) {
			warning("mstOp67 unable to find a free MonsterObject1");
			return;
		}
		memset(m->localVars, 0, sizeof(m->localVars));
		m->flags48 = 0x1C;
		m->flagsA5 = 0;
		m->collideDistance = -1;
		m->m35 = 0;
		m->flagsA6 = 0;

		assert((uint32_t)arg1C < _res->_mstUnk42[arg24].count1);
		const int j = _res->_mstUnk42[arg24].indexUnk46[arg1C];
		assert(j >= 0 && j < _res->_mstHdr.unk0x30);
		MstUnk46 *m46 = &_res->_mstUnk46[j];
		m->m46 = m46;
		assert((uint32_t)arg20 < m46->count);
		MstUnk46Unk1 *m1 = &m46->data[arg20]; // _ecx
		m->m46Unk1 = m1;
		m->unk8 = _res->_mstMonsterInfos + m1->indexMonsterInfo * kMonsterInfoDataSize;

		m->localVars[7] = m1->energy;

		if (m1->indexUnk51 == kNone) {
			m->flags48 &= ~4;
		}

		const uint8_t *ptr = m->unk8;
		int anim = m1->anim; // READ_LE_UINT16(_ecx + 4)
		o = addLvlObject(ptr[945], x1, y1, objScreen, ptr[944], anim, o_flags1, o_flags2, 0, 0);
		if (!o) {
			resetMonsterObject1(m);
			return;
		}
// 41562C
		m->o16 = o;
		if (_currentLevel == kLvl_lar2 && m->unk8[944] == 26) {
			m->o20 = addLvlObject(ptr[945], x1, y1, objScreen, ptr[944], m1->anim + 1, o_flags1, 0x3001, 0, 0);
			if (!m->o20) {
				warning("mstOp67 failed to addLvlObject in kLvl_lar2");
				resetMonsterObject1(m);
				return;
			}
			if (screen < 0) {
				m->o20->xPos += _mstAndyScreenPosX;
				m->o20->yPos += _mstAndyScreenPosY;
			}
			m->o20->dataPtr = 0;
			setLvlObjectPosRelativeToObject(m->o16, 6, m->o20, 6);
		}
// 4156FC
		m->unkE8 = o_flags2 & 0xFFFF;
		m->unkE6 = _mstLut5[o_flags2 & 0x1F];
		o->dataPtr = m;
	} else {
		for (int i = 0; i < kMaxMonsterObjects2; ++i) {
			if (!_monsterObjects2Table[i].m45) {
				mo = &_monsterObjects2Table[i];
				break;
			}
		}
		if (!mo) {
			warning("mstOp67 no free monster2");
			return;
		}
// 415743
		assert(arg24 >= 0 && arg24 < _res->_mstHdr.unk0x2C);
		mo->m45 = &_res->_mstUnk45[arg24];
		if (currentTask->monster1) {
			mo->monster1 = currentTask->monster1;
		} else if (currentTask->monster2) {
			mo->monster1 = currentTask->monster2->monster1;
		} else {
			mo->monster1 = 0;
		}

		mo->flags24 = 0;

		uint8_t _cl  = mo->m45->unk0;
		uint16_t _ax = mo->m45->unk2; // anim

		o = addLvlObject((_cl >> 7) & 1, x1, y1, objScreen, (_cl & 0x7F), _ax, o_flags1, o_flags2, 0, 0);
		if (!o) {
			mo->m45 = 0;
			if (mo->o) {
				mo->o->dataPtr = 0;
			}
			return;
		}
		mo->o = o;
		o->dataPtr = mo;
	}
// 4157E8
	if (screen < 0) {
		o->xPos += _mstAndyScreenPosX;
		o->yPos += _mstAndyScreenPosY;
	}
	setLvlObjectPosInScreenGrid(o, 7);
	if (mo) {
		Task *t = findFreeTask();
		if (!t) {
			warning("mstOp67 mo %p no free task", mo);
			mo->m45 = 0;
			if (mo->o) {
				mo->o->dataPtr = 0;
			}
			removeLvlObject2(o);
			return;
		}
// 41584E
		memset(t, 0, sizeof(Task));
		resetTask(t, kUndefinedMonsterByteCode);
		t->monster2 = mo;
		t->monster1 = 0;
		mo->task = t;
		t->codeData = 0;
		appendTask(&_monsterObjects2TasksList, t);
		t->codeData = kUndefinedMonsterByteCode;
		mstTaskSetScreenPosition(t);
		const uint32_t codeData = mo->m45->codeData;
		assert(codeData != kNone);
		resetTask(t, _res->_mstCodeData + codeData * 4);
		if (_currentLevel == kLvl_fort && mo->m45->unk0 == 27) {
			initMonsterObject2_firefly(mo);
		}
	} else {
// 41593C
		Task *t = findFreeTask();
		if (!t) {
			warning("mstOp67_addMonster no free task found");
			resetMonsterObject1(m);
			removeLvlObject2(o);
			return;
		}
// 415989
		memset(t, 0, sizeof(Task));
		resetTask(t, kUndefinedMonsterByteCode);
		t->monster1 = m;
		t->monster2 = 0;
		m->task = t;
		t->codeData = 0;
		appendTask(&_monsterObjects1TasksList, t);
// 415A3C
		t->codeData = kUndefinedMonsterByteCode;
		_rnd.resetMst(m->rnd_m35);
		_rnd.resetMst(m->rnd_m49);

		m->x2 = -1;
		MstUnk46Unk1 *m46unk1 = m->m46Unk1;
		m->m44Unk1 = _res->_mstUnk44[m46unk1->indexUnk44].data;

		if (m->unk8[946] & 4) {
			m->flagsA8[0] = 0xFF;
			m->flagsA8[1] = 0xFF;
		}
// 415A89
		mstTaskUpdateScreenPosition(t);
		switch (type) {
		case 1:
			executeMstOp67Type1(t);
			assert(t->run != &Game::mstSm_main || (t->codeData && t->codeData != kUndefinedMonsterByteCode));
			break;
		case 2:
			if (m) {
				m->flagsA6 |= 1;
			}
			executeMstOp67Type2(t, 0);
			assert(t->run != &Game::mstSm_main || (t->codeData && t->codeData != kUndefinedMonsterByteCode));
			break;
		default:
			m->flagsA5 = 1;
			if (!updateMonsterObject1Position(m)) {
				initMonsterObject1(m);
			}
			prepareMstTask(t);
			break;
		}
	}
// 415ADE
	currentTask->flags &= ~0x80;
}

void Game::mstOp68_addMonsterGroup(Task *t, const uint8_t *p, int a, int b, int c, int d) {
	MstUnk42 *m42 = &_res->_mstUnk42[d];
	struct {
		int m42Index;
		int m46Index;
	} data[16];
	int count = 0;
	for (uint32_t i = 0; i < m42->count1; ++i) {
		MstUnk46 *m46 = &_res->_mstUnk46[m42->indexUnk46[i]];
		for (uint32_t j = 0; j < m46->count; ++j) {
			MstUnk46Unk1 *m46u1 = &m46->data[j];
			uint32_t indexMonsterInfo = p - _res->_mstMonsterInfos;
			assert((indexMonsterInfo % kMonsterInfoDataSize) == 0);
			indexMonsterInfo /= kMonsterInfoDataSize;
			if (m46u1->indexMonsterInfo == indexMonsterInfo) {
				assert(count < 16);
				data[count].m42Index = i;
				data[count].m46Index = j;
				++count;
			}
		}
	}
	if (count == 0) {
		return;
	}
	int j = 0;
	for (int i = 0; i < a; ++i) {
		mstOp67_addMonster(t, _mstOp67_x1, _mstOp67_x2, _mstOp67_y1, _mstOp67_y2, _mstOp67_screenNum, _mstOp67_type, _mstOp67_flags1, _mstOp68_flags1, data[j].m42Index, data[j].m46Index, d);
		if (--c == 0) {
			return;
		}
		if (++j >= count) {
			j = 0;
		}
	}
	for (int i = 0; i < b; ++i) {
		mstOp67_addMonster(t, _mstOp68_x1, _mstOp68_x2, _mstOp68_y1, _mstOp68_y2, _mstOp68_screenNum, _mstOp68_type, _mstOp68_arg9, _mstOp68_flags1, data[j].m42Index, data[j].m46Index, d);
		if (--c == 0) {
			return;
		}
		if (++j >= count) {
			j = 0;
		}
	}
}

int Game::runTask_wait(Task *t) {
	debug(kDebug_MONSTER, "runTask_wait t %p", t);
	--t->arg1;
	if (t->arg1 == 0) {
		t->run = &Game::mstSm_main;
		return 0;
	}
	return 1;
}

int Game::runTask_waitResetInput(Task *t) {
	debug(kDebug_MONSTER, "runTask_waitResetInput t %p", t);
	--t->arg1;
	if (t->arg1 == 0) {
		t->run = &Game::mstSm_main;
		LvlObject *o = 0;
		if (t->monster1) {
			o = t->monster1->o16;
		} else if (t->monster2) {
			o = t->monster2->o;
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
	if (getTaskFlag(t, t->arg2, t->arg1) == 0) {
		return 1;
	}
	t->run = &Game::mstSm_main;
	LvlObject *o = 0;
	if (t->monster1) {
		o = t->monster1->o16;
	} else if (t->monster2) {
		o = t->monster2->o;
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
	const MstUnk55 *m = &_res->_mstUnk55[t->arg2];
	const int a = getTaskFlag(t, m->indexVar1, m->maskVars & 15);
	const int b = getTaskFlag(t, m->indexVar2, m->maskVars >> 4);
	if (!compareOp(m->compare, a, b)) {
		LvlObject *o = 0;
		MonsterObject1 *m = t->monster1;
		if (m) {
			if ((m->flagsA6 & 2) == 0) {
				o = m->o16;
			}
		} else if (t->monster2) {
			o = t->monster2->o;
		}
		if (o) {
			o->actionKeyMask = 0;
			o->directionKeyMask = 0;
		}
		t->run = &Game::mstSm_main;
		return 0;
	}
	return 1;
}

int Game::runTask_mstOp232(Task *t) {
	warning("runTask_mstOp232 unimplemented");
	t->run = &Game::mstSm_main;
	return 0;
}

int Game::runTask_mstOp233(Task *t) {
	debug(kDebug_MONSTER, "runTask_mstOp233 t %p", t);
	const MstUnk55 *m = &_res->_mstUnk55[t->arg2];
	const int a = getTaskVar(t, m->indexVar1, m->maskVars & 15);
	const int b = getTaskVar(t, m->indexVar2, m->maskVars >> 4);
	if (!compareOp(m->compare, a, b)) {
		t->run = &Game::mstSm_main;
		LvlObject *o = 0;
		if (t->monster1) {
			o = t->monster1->o16;
		} else if (t->monster2) {
			o = t->monster2->o;
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
	const MstUnk55 *m = &_res->_mstUnk55[t->arg2];
	const int a = getTaskVar(t, m->indexVar1, m->maskVars & 15);
	const int b = getTaskVar(t, m->indexVar2, m->maskVars >> 4);
	if (compareOp(m->compare, a, b)) {
		t->run = &Game::mstSm_main;
		LvlObject *o = 0;
		if (t->monster1) {
			o = t->monster1->o16;
		} else if (t->monster2) {
			o = t->monster2->o;
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
	if (t->arg1 == 0) {
		updateMonsterObject1Position(t->monster1);
		executeMstUnk13(t);
		return 0;
	}
	--t->arg1;
	return 1;
}

int Game::runTask_unk2(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk2 t %p", t);
	MonsterObject1 *m = t->monster1;
	const uint16_t flags0 = m->o16->flags0;
	if ((flags0 & 0x100) != 0 && (flags0 & 0xFF) == m->flagsA4) {
		updateMonsterObject1Position(t->monster1);
		executeMstUnk13(t);
		return 0;
	}
	return 1;
}

int Game::runTask_unk3(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk3 t %p", t);
	MonsterObject1 *m = t->monster1;
	const uint16_t flags0 = m->o16->flags0;
	if ((flags0 & 0xFF) == m->flagsA4) {
		if (t->arg1 > 0) {
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
	MonsterObject1 *m = t->monster1;
	const uint32_t offset = m->unk8 - _res->_mstMonsterInfos;
	assert(offset % kMonsterInfoDataSize == 0);
	const uint32_t num = offset / kMonsterInfoDataSize;
	if (t->arg2 != num) {
		updateMonsterObject1Position(m);
		executeMstUnk13(t);
		return 0;
	}
	return 1;
}

int Game::runTask_unk5(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk5 t %p", t);
	MonsterObject1 *m = t->monster1;
	mstSetHorizontalBounds(m);
	if (_xMstPos2 < m->m49Unk1->unk8) {
		if (_xMstPos2 > 0) {
			while (--m->indexUnk49Unk1 >= 0) {
				m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
				if (_xMstPos2 >= m->m49Unk1->unkC) {
					goto set_am;
				}
			}
		}
		return executeMstUnk9(t, m);
	}
set_am:
	const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	mstLvlObjectSetActionDirection(m->o16, ptr, ptr[3], m->flags4A);
	return 1;
}

int Game::runTask_unk6(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk6 t %p", t);
	MonsterObject1 *m = t->monster1;
	if (m->flags4B == 0xFD && m->xMstPos < _mstAndyLevelPosX) {
		m->unk74 = _mstAndyLevelPosX - m->unk68;
		m->unk7C = _mstAndyLevelPosX - m->unk64;
	} else {
		m->unk74 = _mstAndyLevelPosX + m->unk64;
		m->unk7C = _mstAndyLevelPosX + m->unk68;
	}
	mstSetHorizontalBounds(m);
	if (_xMstPos2 < m->m49Unk1->unk8) {
		if (_xMstPos2 > 0) {
			while (--m->indexUnk49Unk1 >= 0) {
				m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
				if (_xMstPos2 >= m->m49Unk1->unkC) {
					goto set_am;
				}
			}
		}
		return executeMstUnk9(t, m);
	}
set_am:
	const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	mstLvlObjectSetActionDirection(m->o16, ptr, ptr[3], m->flags4A);
	return 1;
}

int Game::runTask_unk7(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk7 t %p", t);
	MonsterObject1 *m = t->monster1;
	executeMstUnk6(m);
	return executeMstUnk11(t, m);
}

int Game::runTask_unk8(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk8 t %p", t);
	MonsterObject1 *m = t->monster1;
	executeMstUnk4(m);
	mstSetVerticalHorizontalBounds(m);
	return executeMstUnk11(t, m);
}

int Game::runTask_unk9(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk9 t %p", t);
	MonsterObject1 *m = t->monster1;
	executeMstUnk8(m);
	return executeMstUnk11(t, m);
}

int Game::runTask_unk10(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk10 t %p", t);
	MonsterObject1 *m = t->monster1;
	executeMstUnk4(m);
	executeMstUnk8(m);
	return executeMstUnk11(t, m);
}

int Game::runTask_unk11(Task *t) {
	debug(kDebug_MONSTER, "runTask_unk11 t %p", t);
	MonsterObject1 *m = t->monster1;
	const int num = m->o16->flags0 & 0xFF;
	if (m->unk8[num * 28] == 0) {
		executeMstUnk23(t);
	}
	return 1;
}
