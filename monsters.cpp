/*
 * Heart of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir (cyx@users.sourceforge.net)
 */

#include "game.h"
#include "level.h"
#include "resource.h"
#include "util.h"

// (lut[x] & 1) indicates a diagonal direction
static const uint8_t _mstLut1[] = {
	// 0
	0x08,
	0x00, // kDirectionKeyMaskUp
	0x02, // kDirectionKeyMaskRight
	0x01, // kDirectionKeyMaskUp | kDirectionKeyMaskRight
	// 4
	0x04, // kDirectionKeyMaskDown
	0x00, // kDirectionKeyMaskDown | kDirectionKeyMaskUp
	0x03, // kDirectionKeyMaskDown | kDirectionKeyMaskRight
	0x01, // kDirectionKeyMaskDown | kDirectionKeyMaskUp | kDirectionKeyMaskRight
	// 8
	0x06, // kDirectionKeyMaskLeft
	0x07, // kDirectionKeyMaskLeft | kDirectionKeyMaskUp
	0x02, // kDirectionKeyMaskLeft | kDirectionKeyMaskRight
	0x01, // kDirectionKeyMaskLeft | kDirectionKeyMaskUp | kDirectionKeyMaskRight
	// 12
	0x05, // kDirectionKeyMaskLeft | kDirectionKeyMaskDown
	0x07, // kDirectionKeyMaskLeft | kDirectionKeyMaskDown | kDirectionKeyMaskRight
	0x03, // kDirectionKeyMaskLeft | kDirectionKeyMaskDown | kDirectionKeyMaskRight
	0x01  // kDirectionKeyMaskLeft | kDirectionKeyMaskDown | kDirectionKeyMaskUp | kDirectionKeyMaskRight
};

// indexes _mstLut1
static const uint8_t _mstLut3[] = {
	0x01, 0x03, 0x09, 0x02, 0x08, 0x03, 0x02, 0x01, 0x06, 0x09, 0x02, 0x06, 0x03, 0x04, 0x01, 0x06,
	0x04, 0x02, 0x0C, 0x03, 0x04, 0x0C, 0x06, 0x08, 0x02, 0x0C, 0x08, 0x04, 0x09, 0x06, 0x08, 0x09,
	0x0C, 0x01, 0x04, 0x09, 0x01, 0x08, 0x03, 0x0C
};

// monster frames animation (minus #0, #4, #8, #12, #16, #20, eg. every 4th is skipped)
static const uint8_t _mstLut4[] = {
	0x01, 0x02, 0x03, 0x05, 0x06, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x11, 0x12, 0x13, 0x15,
	0x16, 0x17
};

// indexes _mstLut4 clipped to [0,17], repeat the frame preceding the skipped ones
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
	if (a & kDirectionKeyMaskLeft) { // Andy left facing monster
		r |= 1;
	}
	if (a & kDirectionKeyMaskDown) { // Andy below the monster
		r |= 2;
	}
	return r;
}

void Game::mstMonster1ResetData(MonsterObject1 *m) {
	m->m46 = 0;
	LvlObject *o = m->o16;
	if (o) {
		o->dataPtr = 0;
	}
	for (int i = 0; i < kMaxMonsterObjects2; ++i) {
		MonsterObject2 *mo = &_monsterObjects2Table[i];
		if (mo->monster2Info && mo->monster1 == m) {
			mo->monster1 = 0;
		}
	}
}

void Game::mstMonster2InitFirefly(MonsterObject2 *m) {
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

void Game::mstMonster2ResetData(MonsterObject2 *m) {
	m->monster2Info = 0;
	LvlObject *o = m->o;
	if (o) {
		o->dataPtr = 0;
	}
}

void Game::mstTaskSetMonster2ScreenPosition(Task *t) {
	MonsterObject2 *m = t->monster2;
	LvlObject *o = m->o;
	m->xPos = o->xPos + o->posTable[7].x;
	m->yPos = o->yPos + o->posTable[7].y;
	m->xMstPos = m->xPos + _res->_mstPointOffsets[o->screenNum].xOffset;
	m->yMstPos = m->yPos + _res->_mstPointOffsets[o->screenNum].yOffset;
}

void Game::mstMonster1ResetWalkPath(MonsterObject1 *m) {
	_rnd.resetMst(m->rnd_m35);
	_rnd.resetMst(m->rnd_m49);

	const uint8_t *ptr = m->monsterInfos;
	const int num = (~m->flagsA5) & 1;

	m->levelPosBounds_x2 = m->walkNode->coords[0][num] - (int32_t)READ_LE_UINT32(ptr + 904); // right x coordinate
	m->levelPosBounds_x1 = m->walkNode->coords[1][num] + (int32_t)READ_LE_UINT32(ptr + 904); // left x coordinate
	m->levelPosBounds_y2 = m->walkNode->coords[2][num] - (int32_t)READ_LE_UINT32(ptr + 908); // bottom y coordinate
	m->levelPosBounds_y1 = m->walkNode->coords[3][num] + (int32_t)READ_LE_UINT32(ptr + 908); // top y coordinate

	const uint32_t indexWalkCode = m->walkNode->walkCodeReset[num];
	m->walkCode = (indexWalkCode == kNone) ? 0 : &_res->_mstWalkCodeData[indexWalkCode];
}

bool Game::mstUpdateInRange(MstMonsterAction *m) {
	if (m->unk4 == 0) {
		if (mstHasMonsterInRange(m, 0) && addChasingMonster(m, 0)) {
			return true;
		}
	} else {
		int direction = _rnd.update() & 1;
		if (mstHasMonsterInRange(m, direction) && addChasingMonster(m, direction)) {
			return true;
		}
		direction ^= 1;
		if (mstHasMonsterInRange(m, direction) && addChasingMonster(m, direction)) {
			return true;
		}
	}
	return false;
}

bool Game::addChasingMonster(MstMonsterAction *m48, uint8_t direction) {
	debug(kDebug_MONSTER, "addChasingMonster %d", direction);
	m48->direction = direction;
	if (m48->codeData != kNone) {
		Task *t = createTask(_res->_mstCodeData + m48->codeData * 4);
		if (!t) {
			return false;
		}
		while ((this->*(t->run))(t) == 0);
	}
	_mstActionNum = m48 - &_res->_mstMonsterActionData[0];
	_mstChasingMonstersCount = 0;
	for (int i = 0; i < m48->areaCount; ++i) {
		MstMonsterAreaAction *unk4 = m48->area[i].data;
		const uint8_t num = unk4->monster1Index;
		if (num != 255) {
			assert(num < kMaxMonsterObjects1);
			unk4->direction = direction;
			MonsterObject1 *m = &_monsterObjects1Table[num];
			m->action = unk4;
			m->flags48 |= 0x40;
			m->flagsA5 &= 0x8A;
			m->flagsA5 |= 0x0A;
			mstMonster1ResetWalkPath(m);
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

void Game::mstMonster1ClearChasingMonster(MonsterObject1 *m) {
	m->flags48 &= ~0x50;
	m->action->monster1Index = 255;
	m->action = 0;
	--_mstChasingMonstersCount;
	if (_mstChasingMonstersCount <= 0) {
		_mstActionNum = -1;
	}
}

void Game::mstTaskSetMonster1BehaviorState(Task *t, MonsterObject1 *m, int num) {
	MstBehaviorState *behaviorState = &m->m46->data[num];
	m->behaviorState = behaviorState;
	m->monsterInfos = _res->_mstMonsterInfos + behaviorState->indexMonsterInfo * kMonsterInfoDataSize;
	if (behaviorState->indexUnk51 == kNone) {
		m->flags48 &= ~4;
	}
	const uint32_t indexWalkPath = behaviorState->walkPath;
	assert(indexWalkPath != kNone);
	m->walkNode = _res->_mstWalkPathData[indexWalkPath].data;
	mstTaskUpdateScreenPosition(t);
	if (!mstMonster1UpdateWalkPath(m)) {
		mstMonster1ResetWalkPath(m);
	}
	if (t->run == &Game::mstTask_monsterWait4) {
		t->run = &Game::mstTask_main;
	}
	if ((m->flagsA5 & 8) == 0 && t->run == &Game::mstTask_idle) {
		mstTaskSetNextWalkCode(t);
	}
}

int Game::mstTaskStopMonsterObject1(Task *t) {
	if (_mstActionNum == -1) {
		return 0;
	}
	MonsterObject1 *m = t->monster1;

	// bugfix: original code meant to check bit 3 directly ?

	// const uint8_t r = (m->flagsA5 == 0) ? 1 : 0;
	// if ((r & 8) != 0) {
	//   return 0;
	// }

	const MstMonsterAreaAction *m48 = m->action;
	if (!m48) {
		return 0;
	}
	const uint32_t codeData = m48->codeData2;
	if (codeData != kNone) {
		resetTask(t, _res->_mstCodeData + codeData * 4);
		mstMonster1ClearChasingMonster(m);
		return 0;
	}
	mstMonster1ClearChasingMonster(m);
	if (m->flagsA5 & 0x80) {
		m->flagsA5 &= ~8;
		const uint32_t codeData = m->behaviorState->codeData;
		if (codeData != kNone) {
			resetTask(t, _res->_mstCodeData + codeData * 4);
			return 0;
		}
		m->o16->actionKeyMask = 7;
		m->o16->directionKeyMask = 0;
		t->run = &Game::mstTask_idle;
		return 0;
	}
	m->flagsA5 = (m->flagsA5 & ~9) | 6;
	if (m->flagsA6 & 2) {
		return 0;
	}
	return mstTaskInitMonster1Type2(t, 1);
}

void Game::mstMonster1SetScreenPosition(MonsterObject1 *m) {
	LvlObject *o = m->o16;
	o->xPos = m->xPos - o->posTable[7].x;
	o->yPos = m->yPos - o->posTable[7].y;
	m->xMstPos = m->xPos + _res->_mstPointOffsets[o->screenNum].xOffset;
	m->yMstPos = m->yPos + _res->_mstPointOffsets[o->screenNum].yOffset;
}

bool Game::mstMonster1SetWalkingBounds(MonsterObject1 *m) {
	MstBehaviorState *behaviorState = m->behaviorState;
	const uint32_t indexWalkPath = behaviorState->walkPath;
	assert(indexWalkPath != kNone);
	MstWalkPath *walkPath = &_res->_mstWalkPathData[indexWalkPath];
	MstWalkNode *walkNode = walkPath->data;

	int x = m->xMstPos; // _ebp
	int y = m->yMstPos; // _edx
	if (m->levelPosBounds_x1 >= 0) {
		if (x < m->levelPosBounds_x1) {
			x = m->levelPosBounds_x1;
		} else if (x > m->levelPosBounds_x2) {
			x = m->levelPosBounds_x2;
		}
		if (y < m->levelPosBounds_y1) {
			y = m->levelPosBounds_y1;
		} else if (y > m->levelPosBounds_y2) {
			y = m->levelPosBounds_y2;
		}
	}
// 41E659

	const uint32_t indexWalkBox = walkPath->data[0].walkBox;
	const MstWalkBox *m34 = &_res->_mstWalkBoxData[indexWalkBox]; // _esi
	int xWalkBox = (m34->right - m34->left) / 2 + m34->left; // _ecx

	int minDistance = 0x1000000; // _edi
	int yWalkBox = y;

	uint32_t i = 0;
	for (; i < walkPath->count; ++i) {
		const uint32_t indexWalkBox = walkPath->data[i].walkBox;
		const MstWalkBox *m34 = &_res->_mstWalkBoxData[indexWalkBox]; // _esi
		if (!rect_contains(m34->left, m34->top, m34->right, m34->bottom, x, y)) {
			// find the closest box
			const int d1 = ABS(x - m34->left);
			if (d1 < minDistance) {
				minDistance = d1;
				walkNode = &walkPath->data[i];
				xWalkBox = m34->left;
			}
			const int d2 = ABS(x - m34->right);
			if (d2 < minDistance) {
				minDistance = d2;
				walkNode = &walkPath->data[i];
				xWalkBox = m34->right;
			}
		} else {
// 41E6FD
			// find match, point is in the box
			xWalkBox = x;
			yWalkBox = y;
			walkNode = &walkPath->data[i];
			break;
		}
	}
// 41E70B
	if (i == walkPath->count) {
		// calculate the yPos for the walkBox
		const uint32_t indexWalkBox = walkNode->walkBox;
		const MstWalkBox *m34 = &_res->_mstWalkBoxData[indexWalkBox]; // _esi
		if (y <= m34->bottom) {
			y = (m34->bottom - m34->top) / 2 + m34->top;
		}
		yWalkBox = y; // _edx
	}

// 41E737
	// find screenNum for level position
	int screenNum = -1;
	int xLevelPos;
	int yLevelPos;
	for (int i = 0; i < _res->_mstHdr.screensCount; ++i) {
		xLevelPos = _res->_mstPointOffsets[i].xOffset;
		yLevelPos = _res->_mstPointOffsets[i].yOffset;
		if (rect_contains(xLevelPos, yLevelPos, xLevelPos + 255, yLevelPos + 191, xWalkBox, yWalkBox)) {
			screenNum = i;
			break;
		}
	}
	if (screenNum == -1) {
		screenNum = 0;
		xLevelPos = _res->_mstPointOffsets[0].xOffset + 256 / 2;
		yLevelPos = _res->_mstPointOffsets[0].yOffset + 192 / 2;
	}
	LvlObject *o = m->o16;
	o->screenNum = screenNum;
	m->xPos = xWalkBox - xLevelPos;
	m->yPos = yWalkBox - yLevelPos;
	mstMonster1SetScreenPosition(m);
	m->walkNode = walkNode;
	mstMonster1ResetWalkPath(m);
	return true;
}

bool Game::mstMonster1UpdateWalkPath(MonsterObject1 *m) {
	debug(kDebug_MONSTER, "mstMonster1UpdateWalkPath m %p", m);
	const uint8_t screenNum = m->o16->screenNum;
	MstBehaviorState *behaviorState = m->behaviorState;
	const uint32_t indexWalkPath = behaviorState->walkPath;
	assert(indexWalkPath != kNone);
	MstWalkPath *walkPath = &_res->_mstWalkPathData[indexWalkPath];
	// start from screen number
	uint32_t indexWalkNode = walkPath->walkNodeData[screenNum];
	if (indexWalkNode != kNone) {
		MstWalkNode *walkNode = &walkPath->data[indexWalkNode];
		uint32_t indexWalkBox = walkNode->walkBox;
		const MstWalkBox *m34 = &_res->_mstWalkBoxData[indexWalkBox];
		while (m34->left <= m->xMstPos) {
			if (m34->right >= m->xMstPos && m34->top <= m->yMstPos && m34->bottom >= m->yMstPos) {
				if (m->walkNode == walkNode) {
					return false;
				}
				m->walkNode = walkNode;
				mstMonster1ResetWalkPath(m);
				return true;
			}

			indexWalkNode = walkNode->nextWalkNode;
			if (indexWalkNode == kNone) {
				break;
			}
			walkNode = &walkPath->data[indexWalkNode];
			indexWalkBox = walkNode->walkBox;
			assert(indexWalkBox != kNone);
			m34 = &_res->_mstWalkBoxData[indexWalkBox];
		}
	}
	return mstMonster1SetWalkingBounds(m);
}

uint32_t Game::mstMonster1GetNextWalkCode(MonsterObject1 *m) {
	MstWalkCode *walkCode = m->walkCode;
	int num = 0;
	if (walkCode->dataCount != 0) {
		num = _rnd.getMstNextNumber(m->rnd_m35);
		num = walkCode->data[num];
	}
	const uint32_t codeData = walkCode->codeData[num];
	return codeData;
}

int Game::mstTaskSetNextWalkCode(Task *t) {
	MonsterObject1 *m = t->monster1;
	assert(m);
	const uint32_t codeData = mstMonster1GetNextWalkCode(m);
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
			if (rect_intersects(x1, y1, x2, y2, p->x1, p->y1, p->x2, p->y2)) {
				return i + 1;
			}
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
			if (_monsterObjects1Table[num].monsterInfos[944] != 15) {
				continue;
			}
		}
		if (rect_intersects(x1, y1, x2, y2, p->x1, p->y1, p->x2, p->y2)) {
			return i + 1;
		}
	}
	return 0;
}

int Game::getMstDistance(int y, const AndyShootData *p) const {
	switch (p->directionMask) {
	case 0:
		if (p->boundingBox.x1 <= _mstTemp_x2 && p->boundingBox.y2 >= _mstTemp_y1 && p->boundingBox.y1 <= _mstTemp_y2) {
			const int dx = _mstTemp_x1 - p->boundingBox.x2;
			if (dx <= 0) {
				return 0;
			}
			return (p->type == 3) ? 0 : dx / p->width;
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
						return (p->type == 3) ? 0 : dx2 / p->width;
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
						return (p->type == 3) ? 0 : dx2 / p->width;
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
						return (p->type == 3) ? 0 : dx2 / p->width;
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
						return (p->type == 3) ? 0 : dx2 / p->width;
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
			return (p->type == 3) ? 0 : dx / p->width;
		}
		break;
	case 6:
		if (p->boundingBox.y2 >= _mstTemp_y1 && p->boundingBox.x2 >= _mstTemp_x1 && p->boundingBox.x1 <= _mstTemp_x2) {
			const int dy = p->boundingBox.y1 - _mstTemp_y2;
			if (dy <= 0) {
				return 0;
			}
			return (p->type == 3) ? 0 : dy / p->height;
		}
		break;
	case 7:
		if (p->boundingBox.y1 <= _mstTemp_y2 && p->boundingBox.x2 >= _mstTemp_x1 && p->boundingBox.x1 <= _mstTemp_x2) {
			const int dy = _mstTemp_y1 - p->boundingBox.y2;
			if (dy <= 0) {
				return 0;
			}
			return (p->type == 3) ? 0 : dy / p->height;
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

	const uint8_t *ptr = m->monsterInfos;
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
		m->facingDirectionMask = 8;
	} else {
		m->facingDirectionMask = 2;
	}
	m->yDelta = _mstAndyLevelPosY - m->yMstPos;
	if (m->yDelta < 0) {
		m->yDelta = -m->yDelta;
		m->facingDirectionMask |= 1;
	} else {
		m->facingDirectionMask |= 4;
	}
	m->collideDistance = -1;
	m->shootData = 0;
// 40ED0D
	if (_andyShootsCount != 0 && !_specialAnimFlag && (o->flags1 & 6) != 6 && (m->localVars[7] > 0 || m->localVars[7] < -1) && (m->flagsA5 & 0x80) == 0) {
		for (int i = 0; i < _andyShootsCount; ++i) {
			AndyShootData *p = &_andyShootsTable[i];
			if (m->xDelta > 256 || m->yDelta > 192) {
				continue;
			}
			_mstTemp_x1 = o->xPos;
			_mstTemp_y1 = o->yPos;
			_mstTemp_x2 = o->xPos + o->width - 1;
			_mstTemp_y2 = o->yPos + o->height - 1;
			uint8_t _al = p->type;
			if (_al == 1 || _al == 2) {
// 40EED8
				if (p->monsterDistance >= m->xDelta + m->yDelta) {
					if (o->screenNum != _currentScreen) {
						const int dx = _res->_mstPointOffsets[o->screenNum].xOffset - _res->_mstPointOffsets[_currentScreen].xOffset;
						const int dy = _res->_mstPointOffsets[o->screenNum].yOffset - _res->_mstPointOffsets[_currentScreen].yOffset;
						_mstTemp_x1 += dx;
						_mstTemp_x2 += dx;
						_mstTemp_y1 += dy;
						_mstTemp_y2 += dy;
					}
// 40EF72
					if (rect_intersects(0, 0, 255, 191, _mstTemp_x1, _mstTemp_y1, _mstTemp_x2, _mstTemp_y2)) {
						const uint8_t type = m->monsterInfos[944];
						if (((type & 9) == 0 && clipLvlObjectsSmall(p->o, o, 132)) || ((type & 9) != 0 && clipLvlObjectsSmall(p->o, o, 20))) {
							p->m = m;
							p->monsterDistance = m->xDelta + m->yDelta;
							p->clipX = _clipBoxOffsetX;
							p->clipY = _clipBoxOffsetY;
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
			} else if (_al == 3 && p->monsterDistance > m->xDelta + m->yDelta) {
				if (o->screenNum != _currentScreen) {
					const int dx = _res->_mstPointOffsets[o->screenNum].xOffset - _res->_mstPointOffsets[_currentScreen].xOffset;
					const int dy = _res->_mstPointOffsets[o->screenNum].yOffset - _res->_mstPointOffsets[_currentScreen].yOffset;
					_mstTemp_x1 += dx;
					_mstTemp_x2 += dx;
					_mstTemp_y1 += dy;
					_mstTemp_y2 += dy;
				}
// 40EE68
				if (rect_intersects(0, 0, 255, 191, _mstTemp_x1, _mstTemp_y1, _mstTemp_x2, _mstTemp_y2)) {
					if (testPlasmaCannonPointsDirection(_mstTemp_x1, _mstTemp_y1, _mstTemp_x2, _mstTemp_y2)) {
						p->monsterDistance = m->xDelta + m->yDelta;
						p->m = m;
						p->plasmaCannonPointsCount = _plasmaCannonLastIndex1;
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
			int res = getMstDistance((m->monsterInfos[946] & 2) != 0 ? p->boundingBox.y2 : m->yMstPos, p);
			if (res < 0) {
				continue;
			}
			if (m->collideDistance == -1 || m->collideDistance > res || (m->collideDistance == 0 && res == 0 && (m->shootData->type & 1) == 0 && p->type == 2)) {
				m->shootData = p;
				m->collideDistance = res;
			}
		}
	}
// 40F151
	if (m->facingDirectionMask & 8) {
		m->unk88 = READ_LE_UINT32(ptr + 920);
		m->unk84 = READ_LE_UINT32(ptr + 924);
	} else {
		m->unk88 = READ_LE_UINT32(ptr + 912);
		m->unk84 = READ_LE_UINT32(ptr + 916);
	}
	if (m->facingDirectionMask & 1) {
		m->unk90 = READ_LE_UINT32(ptr + 936);
		m->unk8C = READ_LE_UINT32(ptr + 940);
	} else {
		m->unk90 = READ_LE_UINT32(ptr + 928);
		m->unk8C = READ_LE_UINT32(ptr + 932);
	}
// 40F1B3
	if (_andyShootsTable[0].type == 3 && m->shootSource != 0 && _andyShootsTable[0].directionMask == m->shootDirection && m->directionKeyMask == _andyObject->directionKeyMask) {
		m->flags48 |= 0x80;
	} else {
		m->shootDirection = -1;
		m->flags48 &= ~0x80;
	}
}

void Game::shuffleMstMonsterActionIndex(MstMonsterActionIndex *p) {
	for (uint32_t i = 0; i < p->dataCount; ++i) {
		p->data[i] &= 0x7F;
	}
	shuffleArray(p->data, p->dataCount);
}

void Game::initMstCode() {
	memset(_mstVars, 0, sizeof(_mstVars));
	if (_mstDisabled) {
		return;
	}
	// _mstLut initialization
	resetMstCode();
}

void Game::resetMstCode() {
	if (_mstDisabled) {
		return;
	}
	_mstFlags = 0;
	for (int i = 0; i < kMaxMonsterObjects1; ++i) {
		mstMonster1ResetData(&_monsterObjects1Table[i]);
	}
	for (int i = 0; i < kMaxMonsterObjects2; ++i) {
		mstMonster2ResetData(&_monsterObjects2Table[i]);
	}
	clearLvlObjectsList1();
	for (int i = 0; i < _res->_mstHdr.screenAreaDataCount; ++i) {
		_res->_mstScreenAreaData[i].unk0x1D = 1;
	}
	_rnd.initMstTable();
	_rnd.initTable();
	for (int i = 0; i < _res->_mstHdr.movingBoundsDataCount; ++i) {
		const int count = _res->_mstMovingBoundsData[i].count2;
		if (count != 0) {
			shuffleArray(_res->_mstMovingBoundsData[i].data2, count);
		}
	}
	for (int i = 0; i < _res->_mstHdr.walkCodeDataCount; ++i) {
		const int count = _res->_mstWalkCodeData[i].dataCount;
		if (count != 0) {
			shuffleArray(_res->_mstWalkCodeData[i].data, count);
		}
	}
	for (int i = 0; i < _res->_mstHdr.monsterActionIndexDataCount; ++i) {
		shuffleMstMonsterActionIndex(&_res->_mstMonsterActionIndexData[i]);
	}
	_mstOp67_x1 = -256;
	_mstOp67_x2 = -256;
	memset(_monsterObjects1Table, 0, sizeof(_monsterObjects1Table));
	memset(_monsterObjects2Table, 0, sizeof(_monsterObjects2Table));
	memset(_mstVars, 0, sizeof(_mstVars));
	memset(_tasksTable, 0, sizeof(_tasksTable));
	_m43Num3 = _m43Num1 = _m43Num2 = _mstActionNum = -1;
	_mstOp54Counter = 0; // bugfix: not reset in the original, causes uninitialized reads at the beginning of 'fort'
	_executeMstLogicPrevCounter = _executeMstLogicCounter = 0;
	// _mstUnk8 = 0;
	_specialAnimFlag = false;
	_mstAndyRectNum = 255;
	_mstBoundingBoxesCount = 0;
	_mstOp67_y1 = 0;
	_mstOp67_y2 = 0;
	_mstOp67_screenNum = kNoScreen;
	_mstOp68_x1 = 256;
	_mstOp68_x2 = 256;
	_mstOp68_y1 = 0;
	_mstOp68_y2 = 0;
	_mstOp68_screenNum = kNoScreen;
	_mstLevelGatesMask = 0;
	// _mstLevelGatesTestMask = 0xFFFFFFFF;
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
	for (int i = 0; i < _res->_mstHdr.infoMonster1Count; ++i) {
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
	if (_level->_checkpoint < _res->_mstHdr.levelCheckpointCodeDataCount) {
		const uint32_t codeData = _res->_mstLevelCheckpointCodeData[_level->_checkpoint];
		if (codeData != kNone) {
			Task *t = createTask(_res->_mstCodeData + codeData * 4);
			if (t) {
				_runTaskOpcodesCount = 0;
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
	if (_mstDisabled) {
		return;
	}
	++_executeMstLogicCounter;
	if (_mstLevelGatesMask != 0) {
		_mstHelper1Count = 0;
		executeMstCodeHelper1();
		_mstLevelGatesMask = 0;
	}
	for (int i = 0; i < kMaxAndyShoots; ++i) {
		_andyShootsTable[i].shootObjectData = 0;
		_andyShootsTable[i].m = 0;
		_andyShootsTable[i].monsterDistance = 0x1000000;
	}
	mstUpdateMonster1ObjectsPosition();
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
	const MstScreenArea *msac;
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
	for (int i = 0; i < _andyShootsCount; ++i) {
		AndyShootData *p = &_andyShootsTable[i];
		MonsterObject1 *m = p->m;
		if (!m) {
			continue;
		}
		const int energy = m->localVars[7];
		ShootLvlObjectData *dat = p->shootObjectData;
		if (dat) {
			if (energy > 0) {
// 419C30
				if (p->type == 2) {
					m->localVars[7] -= 4;
					if (m->localVars[7] < 0) {
						m->localVars[7] = 0;
					}
				} else {
					--m->localVars[7];
				}
// 419C79
				dat->unk3 = 0x80;
				dat->xPosShoot = p->clipX;
				dat->yPosShoot = p->clipY;
				dat->o = m->o16;
			} else if (energy == -2) {
// 419CA1
				dat->unk3 = 0x80;
				dat->xPosShoot = p->clipX;
				dat->yPosShoot = p->clipY;
			}
			continue;
		}
// 419CC0
		if (energy > 0) {
			if (_cheats & kCheatOneHitPlasmaCannon) {
				m->localVars[7] = 0;
			} else {
				m->localVars[7] = energy - 1;
			}
			_plasmaCannonLastIndex1 = p->plasmaCannonPointsCount;
		} else if (energy == -2) {
			_plasmaCannonLastIndex1 = p->plasmaCannonPointsCount;
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

void Game::mstWalkPathUpdateIndex(MstWalkPath *walkPath, int index) {
	uint32_t _walkNodesTable[32];
	int _walkNodesFirstIndex, _walkNodesLastIndex;
	int32_t buffer[64];
	for (uint32_t i = 0; i < walkPath->count; ++i) {
		MstWalkNode *walkNode = &walkPath->data[i];
		memset(buffer, 0xFF, sizeof(buffer));
		memset(walkNode->unk60[index], 0, walkPath->count);
		_walkNodesTable[0] = i;
		_walkNodesLastIndex = 1;
		buffer[i] = 0;
		_walkNodesFirstIndex = 0;
// 41749C
		while (_walkNodesFirstIndex != _walkNodesLastIndex) {
			uint32_t _edi = _walkNodesTable[_walkNodesFirstIndex];
			++_walkNodesFirstIndex;
			if (_walkNodesFirstIndex >= 32) {
				_walkNodesFirstIndex = 0;
			}
			const uint32_t indexWalkBox = walkPath->data[_edi].walkBox;
			const MstWalkBox *m34 = &_res->_mstWalkBoxData[indexWalkBox];
// 4174DE
			for (int j = 0; j < 4; ++j) {
				const uint32_t indexWalkNode = walkPath->data[_edi].neighborWalkNode[j];
				if (indexWalkNode == kNone) {
					continue;
				}
				assert(indexWalkNode < walkPath->count);
				uint32_t mask;
				const uint8_t flags = m34->flags[j];
				if (flags & 0x80) {
					mask = _mstAndyVarMask & (1 << (flags & 0x7F));
				} else {
					mask = flags & (1 << index);
				}
				if (mask != 0) {
					continue;
				}
// 417525
				int _ecx;
				if (j == 0 || j == 1) {
					_ecx = m34->right - m34->left + buffer[_edi];
				} else {
					_ecx = m34->bottom - m34->top + buffer[_edi];
				}
				if (_ecx >= buffer[i]) {
					continue;
				}
				if (buffer[indexWalkNode] == -1) {
					_walkNodesTable[_walkNodesLastIndex] = indexWalkNode;
					++_walkNodesLastIndex;
					if (_walkNodesLastIndex >= 32) {
						_walkNodesLastIndex = 0;
					}
				}
				buffer[i] = _ecx;
// 417585
				uint8_t value;
				if (_edi == i) {
					static const uint8_t data[] = { 2, 8, 4, 1 };
					value = data[j];
				} else {
					value = walkNode->unk60[index][_edi];
				}
				walkNode->unk60[index][i] = value;
			}
		}
	}
}

int Game::mstWalkPathUpdateWalkNode(MstWalkPath *walkPath, MstWalkNode *walkNode, int num, int index) {
	if (walkNode->coords[num][index] < 0) {
		const uint32_t indexWalkBox = walkNode->walkBox;
		const MstWalkBox *m34 = &_res->_mstWalkBoxData[indexWalkBox];
		uint32_t mask;
		const uint8_t flags = m34->flags[num];
		if (flags & 0x80) {
			mask = _mstAndyVarMask & (1 << (flags & 0x7F));
		} else {
			mask = flags & (1 << index);
		}
		if (mask) {
			switch (num) { // ((int *)&m34)[num]
			case 0:
				walkNode->coords[num][index] = m34->right;
				break;
			case 1:
				walkNode->coords[num][index] = m34->left;
				break;
			case 2:
				walkNode->coords[num][index] = m34->bottom;
				break;
			case 3:
				walkNode->coords[num][index] = m34->top;
				break;
			}
		} else {
			const uint32_t indexWalkNode = walkNode->neighborWalkNode[num];
			assert(indexWalkNode != kNone && indexWalkNode < walkPath->count);
			walkNode->coords[num][index] = mstWalkPathUpdateWalkNode(walkPath, &walkPath->data[indexWalkNode], num, index);
		}
	}
	return walkNode->coords[num][index];
}

void Game::executeMstCodeHelper1() {
	for (int i = 0; i < _res->_mstHdr.walkPathDataCount; ++i) {
		MstWalkPath *walkPath = &_res->_mstWalkPathData[i];
		if (walkPath->mask & _mstLevelGatesMask) {
			++_mstHelper1Count;
			for (uint32_t j = 0; j < walkPath->count; ++j) {
				for (int k = 0; k < 2; ++k) {
					walkPath->data[j].coords[0][k] = -1;
					walkPath->data[j].coords[1][k] = -1;
					walkPath->data[j].coords[2][k] = -1;
					walkPath->data[j].coords[3][k] = -1;
				}
			}
			for (uint32_t j = 0; j < walkPath->count; ++j) {
				MstWalkNode *walkNode = &walkPath->data[j];
				for (int k = 0; k < 4; ++k) {
					mstWalkPathUpdateWalkNode(walkPath, walkNode, k, 0);
					mstWalkPathUpdateWalkNode(walkPath, walkNode, k, 1);
				}
			}
			mstWalkPathUpdateIndex(walkPath, 0);
			mstWalkPathUpdateIndex(walkPath, 1);
		}
	}
	if (_mstHelper1Count != 0) {
		for (int i = 0; i < kMaxMonsterObjects1; ++i) {
			MonsterObject1 *m = &_monsterObjects1Table[i];
			if (!m->m46) {
				continue;
			}
			MstWalkNode *walkNode = m->walkNode;
			const int num = (~m->flagsA5) & 1;
			m->levelPosBounds_x2 = walkNode->coords[0][num] - (int32_t)READ_LE_UINT32(m->monsterInfos + 904);
			m->levelPosBounds_x1 = walkNode->coords[1][num] + (int32_t)READ_LE_UINT32(m->monsterInfos + 904);
			m->levelPosBounds_y2 = walkNode->coords[2][num] - (int32_t)READ_LE_UINT32(m->monsterInfos + 908);
			m->levelPosBounds_y1 = walkNode->coords[3][num] + (int32_t)READ_LE_UINT32(m->monsterInfos + 908);
		}
	}
}

void Game::mstUpdateMonster1ObjectsPosition() {
	mstUpdateRefPos();
	mstUpdateMonstersRect();
	for (Task *t = _monsterObjects1TasksList; t; t = t->nextPtr) {
		mstTaskUpdateScreenPosition(t);
	}
}

void Game::mstLvlObjectSetActionDirection(LvlObject *o, const uint8_t *ptr, uint8_t mask1, uint8_t dirMask2) {
	o->actionKeyMask = ptr[1];
	o->directionKeyMask = mask1 & 15;
	MonsterObject1 *m = (MonsterObject1 *)getLvlObjectDataPtr(o, kObjectDataTypeMonster1);
	if ((mask1 & 0x10) == 0) {
		const int _edi = mask1 & 0xE0;
		switch (_edi) {
		case 32:
		case 96:
		case 160:
		case 192:
			if (_edi == 192) {
				o->directionKeyMask |= (m->facingDirectionMask & ~kDirectionKeyMaskVertical);
			} else {
				o->directionKeyMask |= m->facingDirectionMask;
				if ((m->monsterInfos[946] & 2) != 0) {
					if (_edi == 160 && (_mstLut1[o->directionKeyMask] & 1) != 0) {
						if (m->xDelta >= m->yDelta) {
							o->directionKeyMask &= ~kDirectionKeyMaskVertical;
						} else {
							o->directionKeyMask &= ~kDirectionKeyMaskHorizontal;
						}
					} else {
						if (m->xDelta >= m->yDelta * 2) {
							o->directionKeyMask &= ~kDirectionKeyMaskVertical;
						} else if (m->yDelta >= m->xDelta * 2) {
							o->directionKeyMask &= ~kDirectionKeyMaskHorizontal;
						}
					}
				}
			}
			break;
		case 128:
			o->directionKeyMask |= dirMask2;
			if ((m->monsterInfos[946] & 2) != 0 && (_mstLut1[o->directionKeyMask] & 1) != 0) {
				if (m->xDelta >= m->yDelta) {
					o->directionKeyMask &= ~kDirectionKeyMaskVertical;
				} else {
					o->directionKeyMask &= ~kDirectionKeyMaskHorizontal;
				}
			}
			break;
		case 224:
			o->directionKeyMask |= m->unkF8;
			break;
		default:
			o->directionKeyMask |= dirMask2;
			break;
		}
	}
// 40E3B3
	o->directionKeyMask &= ptr[2];
	if ((mask1 & 0xE0) == 0x40) {
		o->directionKeyMask ^= kDirectionKeyMaskHorizontal;
	}
}

void Game::mstMonster1UpdateGoalPosition(MonsterObject1 *m) {
	int var1C = 0;
	int var18 = 0;
	int _ebp, _edi, _esi, _eax;
	if (m->goalScreenNum == 0xFD) {
		MstMovingBounds *m49 = m->m49;
		if (m->levelPosBounds_x2 > _mstAndyLevelPosX + m49->unk14 - m->goalDistance_x2 && m->levelPosBounds_x1 < _mstAndyLevelPosX + m->goalDistance_x2 - m49->unk14 && m->levelPosBounds_y2 > _mstAndyLevelPosY + m49->unk15 - m->goalDistance_y2 && m->levelPosBounds_y1 < _mstAndyLevelPosY + m49->unk15 + m->goalDistance_y2) {
			var18 = _mstAndyLevelPosX + m49->unk14 + m->goalDistance_x1;
			if (m->levelPosBounds_x2 < var18) {
				if (m->levelPosBounds_x1 > _mstAndyLevelPosX - m->goalDistance_x1 - m49->unk14 && m->levelPosBounds_y2 < _mstAndyLevelPosY + m49->unk15 + m->goalDistance_y1 && m->levelPosBounds_y1 > _mstAndyLevelPosY - m->goalDistance_y1 - m49->unk15) {
					goto l41B2DC;
				}
			}
// 41B170
			int tmp = var18;
			var18 = 0;
			_ebp = 0x40000000;
			if (m->levelPosBounds_x2 >= tmp) {
				if (m->xMstPos > _mstAndyLevelPosX + m->goalDistance_x2) {
					_ebp = m->xMstPos - m->goalDistance_x1 - _mstAndyLevelPosX;
// 41B191
					if (_ebp >= 0 && _ebp < m49->unk14) {
						_ebp = 0x40000000;
					} else {
						var18 = 1;
					}
// 41B1A2
				} else if (m->xMstPos >= _mstAndyLevelPosX + m->goalDistance_x1) {
					_ebp = -1;
					var18 = 1;
				} else {
					_ebp = m->goalDistance_x2 - m->xMstPos + _mstAndyLevelPosX;
					if (_ebp >= 0 && _ebp < m49->unk14) {
						_ebp = 0x40000000;
					} else {
						var18 = 1;
					}
				}
			}
// 41B1BE
			_edi = 0x40000000;
			if (m->levelPosBounds_x1 <= _mstAndyLevelPosX - m->goalDistance_x1 - m49->unk14) {
				if (m->xMstPos > _mstAndyLevelPosX - m->goalDistance_x1) {
					_edi = m->xMstPos - _mstAndyLevelPosX  + m->goalDistance_x2;
					if (_edi >= 0 && _edi < m49->unk14) {
						_edi = 0x40000000;
					} else {
						var18 |= 2;
					}
// 41B1F8
				} else if (m->xMstPos >= _mstAndyLevelPosX - m->goalDistance_x2) {
					_edi = -1;
					var18 |= 2;
				} else {
					_edi = _mstAndyLevelPosX - m->goalDistance_x1 - m->xMstPos;
					if (_edi >= 0 && _edi < m49->unk14) {
						_edi = 0x40000000;
					} else {
						var18 |= 2;
					}
				}
			}
// 41B21A
			_esi = 0x40000000;
			if (m->levelPosBounds_y2 >= _mstAndyLevelPosY + m->goalDistance_y1 + m49->unk15) {
				if (m->yMstPos > _mstAndyLevelPosY + m->goalDistance_y2) {
					_esi = m->yMstPos - m->goalDistance_y1 - _mstAndyLevelPosY;
					if (_esi >= 0 && _esi < m49->unk15) {
						_esi = 0x40000000;
					} else {
						var18 |= 4;
					}
// 41B252
				} else if (m->yMstPos >= _mstAndyLevelPosY + m->goalDistance_y1) {
					_esi = -1;
					var18 |= 4;
				} else {
					_esi = m->goalDistance_y2 - m->yMstPos - _mstAndyLevelPosY;
					if (_esi >= 0 && _esi < m49->unk15) {
						_esi = 0x40000000;
					} else {
						var18 |= 4;
					}
				}
			}
// 41B276
			_eax = 0x40000000;
			if (m->levelPosBounds_y1 <= _mstAndyLevelPosY - m->goalDistance_y1 - m49->unk15) {
				if (m->yMstPos > _mstAndyLevelPosY - m->goalDistance_y1) {
					_eax = m->yMstPos - _mstAndyLevelPosY + m->goalDistance_y2;
					if (_eax >= 0 && _eax < m49->unk15) {
						_eax = 0x40000000;
					} else {
						var18 |= 8;
					}
				} else if (m->yMstPos >= _mstAndyLevelPosY - m->goalDistance_y2) {
// 41B2B0
					_eax = -1;
					var18 |= 8;
				} else {
					_eax = _mstAndyLevelPosY - m->goalDistance_y1 - m->yMstPos;
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
			_ebp = ABS(m->xMstPos - _mstAndyLevelPosX + m49->unk14 - m->goalDistance_x2);
			_edi = ABS(m->xMstPos - _mstAndyLevelPosX - m49->unk14 + m->goalDistance_x2);
			_esi = ABS(m->yMstPos - _mstAndyLevelPosY + m49->unk15 - m->goalDistance_y2);
			_eax = ABS(m->yMstPos - _mstAndyLevelPosX - m49->unk15 + m->goalDistance_y2);
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
				var1C = (_esi > _eax) ? 0x21 : 0x20;
				break;
			case 2:
				if (_edi <= _eax && _edi <= _esi) {
					var1C = 0x12;
				} else {
					var1C = (_esi >= _eax) ? 0x21 : 0x20;
				}
				break;
			case 3:
				if (_ebp > _edi) {
// 41B38E
					if (_edi <= _eax && _edi <= _esi) {
						var1C = 0x12;
					} else {
						var1C = (_esi >= _eax) ? 0x21 : 0x20;
					}
					break;
				}
				// fall-through
			case 1:
				if (_ebp <= _eax && _ebp <= _esi) {
					var1C = 2;
				} else {
					var1C = (_esi >= _eax) ? 0x21 : 0x20;
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
		m->goalPos_x1 = m->goalDistance_x1;
		m->goalPos_x2 = m->goalDistance_x2;
		break;
	case 0x10:
		m->goalPos_x1 = -m->goalDistance_x2;
		m->goalPos_x2 = -m->goalDistance_x1;
		break;
	case 0x20:
		m->goalPos_x1 = -m->goalDistance_x2;
		m->goalPos_x2 =  m->goalDistance_x2;
		break;
	}
// 41B404
	switch (var1C & 0xF) {
	case 0:
		m->goalPos_y1 = m->goalDistance_y1;
		m->goalPos_y2 = m->goalDistance_y2;
		break;
	case 1:
		m->goalPos_y1 = -m->goalDistance_y2;
		m->goalPos_y2 = -m->goalDistance_y1;
		break;
	case 2:
		m->goalPos_y2 =  m->goalDistance_y2;
		m->goalPos_y1 = -m->goalDistance_y2;
		break;
	}
// 41B442
	m->goalPos_x1 += _mstAndyLevelPosX;
	m->goalPos_x2 += _mstAndyLevelPosX;
	m->goalPos_y1 += _mstAndyLevelPosY;
	m->goalPos_y2 += _mstAndyLevelPosY;
	const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	if ((ptr[2] & kDirectionKeyMaskVertical) == 0) {
		m->goalDistance_y1 = m->goalPos_y1 = m->goalDistance_y2 = m->goalPos_y2 = m->yMstPos;
	}
	if ((ptr[2] & kDirectionKeyMaskHorizontal) == 0) {
		m->goalDistance_x1 = m->goalPos_x1 = m->goalDistance_x2 = m->goalPos_x2 = m->xMstPos;
	}
}

void Game::mstMonster1MoveTowardsGoal1(MonsterObject1 *m) {
	MstWalkNode *walkNode = m->walkNode;
	const uint8_t *p = m->monsterInfos;
	const uint32_t indexWalkBox = walkNode->walkBox;
	const MstWalkBox *m34 = &_res->_mstWalkBoxData[indexWalkBox];
	const int w = READ_LE_UINT32(p + 904);
	const int h = READ_LE_UINT32(p + 908);
	debug(kDebug_MONSTER, "mstMonster1MoveTowardsGoal1 m %p pos %d,%d [%d,%d,%d,%d]", m, m->xMstPos, m->yMstPos, m34->left,  m34->right, m34->top, m34->bottom);
	if (!rect_contains(m34->left - w, m34->top - h, m34->right + w, m34->bottom + h, m->xMstPos, m->yMstPos)) {
		mstMonster1UpdateWalkPath(m);
		m->unkC0 = -1;
		m->unkBC = -1;
	}
	m->task->flags &= ~0x80;
	if (m->xMstPos < m->goalPos_x1) {
		int x = m->goalPos_x2;
		_xMstPos1 = x;
		if (x > m->levelPosBounds_x2) {
			x = m->levelPosBounds_x2;
			m->task->flags |= 0x80;
		}
		m->goalDirectionMask = kDirectionKeyMaskRight;
		_xMstPos2 = x - m->xMstPos;
	} else if (m->xMstPos > m->goalPos_x2) {
		int x = m->goalPos_x1;
		_xMstPos1 = x;
		if (x < m->levelPosBounds_x1) {
			x = m->levelPosBounds_x1;
			m->task->flags |= 0x80;
		}
		m->goalDirectionMask = kDirectionKeyMaskLeft;
		_xMstPos2 = m->xMstPos - x;
	} else {
		_xMstPos1 = m->xMstPos;
		m->goalDirectionMask = 0;
		_xMstPos2 = 0;
	}
	if (m->yMstPos < m->goalPos_y1) {
		int y = m->goalPos_y2;
		_yMstPos1 = y;
		if (y > m->levelPosBounds_y2) {
			y = m->levelPosBounds_y2;
			m->task->flags |= 0x80;
		}
		_yMstPos2 = y - m->yMstPos;
		m->goalDirectionMask |= kDirectionKeyMaskDown;
	} else if (m->yMstPos > m->goalPos_y2) {
		int y = m->goalPos_y1;
		_yMstPos1 = y;
		if (y < m->levelPosBounds_y1) {
			y = m->levelPosBounds_y1;
			m->task->flags |= 0x80;
		}
		_yMstPos2 = m->yMstPos - y;
		m->goalDirectionMask |= kDirectionKeyMaskUp;
	} else {
		_yMstPos1 = m->yMstPos;
		_yMstPos2 = 0;
	}
	if (m->goalDirectionMask == 0) {
		return;
	}
// 41A759
	int _edi = 0;
	MstBehaviorState *behaviorState = m->behaviorState;
	assert(behaviorState->walkPath != kNone);
	MstWalkPath *walkPath = &_res->_mstWalkPathData[behaviorState->walkPath];
	uint8_t _cl = m->flagsA8[2];
	if (m->unkBC != _xMstPos1 || m->unkC0 != _yMstPos1) {
		if (_cl < walkPath->count) {
			MstWalkNode *walkNode = &walkPath->data[_cl];
			const MstWalkBox *m34 = &_res->_mstWalkBoxData[walkNode->walkBox];
			if (rect_contains(m34->left, m34->bottom, m34->right, m34->top, _xMstPos1, _yMstPos1)) {
				// goto 41A879
			} else {
				_edi = -1;
			}
		}
		if (_edi == 0) {
// 41A7AD
			const int _al = mstMonster1FindWalkPathRect(m, walkPath, _xMstPos1, _yMstPos1);
			if (_al < 0) {
				_xMstPos1 = _xMstPos3;
				_yMstPos1 = _yMstPos3;
				_cl = _edi - _al;
				m->goalPos_x1 = m->goalPos_x2 = _xMstPos1;
				m->goalPos_y1 = m->goalPos_y2 = _yMstPos1;
				_xMstPos2 = ABS(m->xMstPos - _xMstPos1);
				_yMstPos2 = ABS(m->yMstPos - _yMstPos1);
				if (m->xMstPos < m->goalPos_x1) {
					m->goalDirectionMask = kDirectionKeyMaskRight;
				} else if (m->xMstPos > m->goalPos_x2) {
					m->goalDirectionMask = kDirectionKeyMaskLeft;
				}
				if (m->yMstPos < m->goalPos_y1) {
					m->goalDirectionMask |= kDirectionKeyMaskDown;
				} else if (m->yMstPos > m->goalPos_y2) {
					m->goalDirectionMask |= kDirectionKeyMaskUp;
				}
			} else {
				_cl = _al;
			}
// 41A85C
			m->flagsA8[2] = _cl;
			m->unkBC = -1;
			m->unkC0 = -1;
		}
	}
// 41A879
	if (_cl >= walkPath->count || m->walkNode == &walkPath->data[_cl]) {
		m->flagsA7 = 0xFF;
		return;
	}
	_edi = (~m->flagsA5) & 1;
	if (m->unkBC == _xMstPos1 && m->unkC0 == _yMstPos1) {
		if (m->flagsA7 == 255) {
			return;
		}
		m->goalDirectionMask = m->flagsA7;
	} else {
// 41A8DD
		m->unkBC = _xMstPos1;
		m->unkC0 = _yMstPos1;
		const uint32_t indexWalkPath = m->behaviorState->walkPath;
		assert(indexWalkPath != kNone);
		MstWalkPath *walkPath = &_res->_mstWalkPathData[indexWalkPath];
		uint8_t var1D = m->walkNode->unk60[_edi][_cl];
		if (var1D != 0) {
			MstWalkNode *walkNode = m->walkNode;
			const uint8_t *p = m->monsterInfos;
			const int w = (int32_t)READ_LE_UINT32(p + 904);
			const int h = (int32_t)READ_LE_UINT32(p + 908);
			while (_xMstPos1 >= walkNode->coords[1][_edi] + w) {
				if (_xMstPos1 > walkNode->coords[0][_edi] - w) {
					break;
				}
				if (_yMstPos1 < walkNode->coords[3][_edi] + h) {
					break;
				}
				if (_yMstPos1 > walkNode->coords[2][_edi] - h) {
					break;
				}
				int var1C;
				switch (walkNode->unk60[_edi][_cl]) {
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
				const uint32_t indexWalkNode = walkNode->neighborWalkNode[var1C];
				assert(indexWalkNode != kNone);
				walkNode = &walkPath->data[indexWalkNode];
				if (walkNode == &walkPath->data[_cl]) {
					m->flagsA7 = 0xFF;
					return;
				}
			}
// 41A9E4
			m->goalDirectionMask = var1D;
		}
// 41A9EB
		m->flagsA7 = m->goalDirectionMask;
	}
// 41A9F4
	if (m->goalDirectionMask & kDirectionKeyMaskLeft) {
		_xMstPos2 = m->xMstPos - m->walkNode->coords[1][_edi] - (int32_t)READ_LE_UINT32(m->monsterInfos + 904);
	} else if (m->goalDirectionMask & kDirectionKeyMaskRight) {
		_xMstPos2 = m->walkNode->coords[0][_edi] - (int32_t)READ_LE_UINT32(m->monsterInfos + 904) - m->xMstPos;
	} else {
		_xMstPos2 = 0;
	}
	if (m->goalDirectionMask & kDirectionKeyMaskUp) {
		_yMstPos2 = m->yMstPos - m->walkNode->coords[3][_edi] - (int32_t)READ_LE_UINT32(m->monsterInfos + 908);
	} else if (m->goalDirectionMask & kDirectionKeyMaskDown) {
		_yMstPos2 = m->walkNode->coords[2][_edi] - (int32_t)READ_LE_UINT32(m->monsterInfos + 908) - m->yMstPos;
	} else {
		_yMstPos2 = 0;
	}
}

bool Game::mstMonster1TestGoalDirection(MonsterObject1 *m) {
	if (_mstLut1[m->goalDirectionMask] & 1) {
		if (_xMstPos2 < m->m49->unk14) {
			m->goalDirectionMask &= ~kDirectionKeyMaskHorizontal;
		}
		if (_yMstPos2 < m->m49->unk15) {
			m->goalDirectionMask &= ~kDirectionKeyMaskVertical;
		}
	}
	if (_mstLut1[m->goalDirectionMask] & 1) {
		while (--m->indexUnk49Unk1 >= 0) {
			m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
			if (_xMstPos2 >= m->m49Unk1->unkE && _yMstPos2 >= m->m49Unk1->unkF) {
				return true;
			}
		}
	} else {
		while (--m->indexUnk49Unk1 >= 0) {
			m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
			if (((m->goalDirectionMask & kDirectionKeyMaskHorizontal) == 0 || _xMstPos2 >= m->m49Unk1->unkC) && ((m->goalDirectionMask & kDirectionKeyMaskVertical) == 0 || _yMstPos2 >= m->m49Unk1->unkD)) {
				return true;
			}
		}
	}
	return false;
}

void Game::mstMonster1MoveTowardsGoal2(MonsterObject1 *m) {
	if (m->targetLevelPos_x != -1) {
		if (m->xMstPos != m->targetLevelPos_x || m->yMstPos != m->targetLevelPos_y) {
			_xMstPos2 = m->unkB4;
			_yMstPos2 = m->unkB8;
			return;
		}
		mstBoundingBoxClear(m, 1);
	}
// 41AB38
	mstMonster1MoveTowardsGoal1(m);
	if (m->goalDirectionMask == 0) {
		return;
	}
	m->unkB4 = _xMstPos2;
	m->unkB8 = _yMstPos2;
	MstMovingBoundsUnk1 *m49Unk1 = m->m49Unk1;
	int _ecx, _edx;
	if (_mstLut1[m->goalDirectionMask] & 1) {
		_ecx = m49Unk1->unkA;
		_edx = m49Unk1->unkB;
	} else {
		_ecx = m49Unk1->unk8;
		_edx = m49Unk1->unk9;
	}
	if (_yMstPos2 < _ecx && _yMstPos2 < _edx) {
		if (_xMstPos2 <= 0 && _yMstPos2 <= 0) {
			return;
		}
		if (!mstMonster1TestGoalDirection(m)) {
			return;
		}
	}
// 41ABED
	if (_mstLut1[m->goalDirectionMask] & 1) {
		if (_xMstPos2 < _yMstPos2) {
			if (_xMstPos2 < m->m49Unk1->unkA) {
				m->goalDirectionMask &= ~kDirectionKeyMaskHorizontal;
				if (m->flagsA7 != 255) {
					_xMstPos2 = 0;
				}
			}
		} else {
// 41AC2B
			if (_yMstPos2 < m->m49Unk1->unkB) {
				m->goalDirectionMask &= ~kDirectionKeyMaskVertical;
				if (m->flagsA7 != 255) {
					_yMstPos2 = 0;
				}
			}
		}
	}
// 41AC4F
	int var10 = (~m->flagsA5) & 1;
	const uint32_t indexWalkBox = m->walkNode->walkBox;
	const MstWalkBox *m34 = &_res->_mstWalkBoxData[indexWalkBox];
	int var20 = 0;
	int varC = 0;
	int var8 = _mstLut1[m->goalDirectionMask];
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
			_ecx = (int8_t)p[0xA];
			_eax = (int8_t)p[0xB];
		} else {
			_ecx = (int8_t)p[0x8];
			_eax = (int8_t)p[0x9];
		}
// 41ACF5
		int _edi = m->xMstPos;
		if (var4 & 8) {
			_edi -= _ecx;
			if (_edi < m->levelPosBounds_x1) {
				continue;
			}
		} else if (var4 & 2) {
			_edi += _ecx;
			if (_edi > m->levelPosBounds_x2) {
				continue;
			}
		}
// 41AD27
		int _ebp = m->yMstPos;
		if (var4 & 1) {
			_ebp -= _eax;
			if (_ebp < m->levelPosBounds_y1) {
				continue;
			}
		} else if (var4 & 4) {
			_ebp += _eax;
			if (_ebp > m->levelPosBounds_y2) {
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
		int w = READ_LE_UINT32(m->monsterInfos + 904);
		int h = READ_LE_UINT32(m->monsterInfos + 908);
		if (!rect_contains(m34->left - w, m34->top - h, m34->right + w, m34->bottom + h, _edi, _ebp)) {
			const uint32_t indexWalkPath = m->behaviorState->walkPath;
			assert(indexWalkPath != kNone);
			MstWalkPath *walkPath = &_res->_mstWalkPathData[indexWalkPath];
			const int num = mstMonster1FindWalkPathRect(m, walkPath, _edi, _ebp);
			if (num < 0) {
				continue;
			}
			if (m->walkNode->unk60[var10][num] == 0) {
				continue;
			}
// 41ADDF
		}
		m->targetLevelPos_x = _edi;
		m->targetLevelPos_y = _ebp;
		p = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
		if (p[0xE] != 0) {
			const int x1 = m->xMstPos + (int8_t)p[0xC];
			const int x2 = x1 + p[0xE] - 1;
			const int y1 = m->yMstPos + (int8_t)p[0xD];
			const int y2 = y1 + p[0xF] - 1;
			int r = mstBoundingBoxCollides2(m->monster1Index, x1, y1, x2, y2);
			if (r > 0) {
// 41AEA3
				const MstBoundingBox *b = &_mstBoundingBoxesTable[r - 1];
				if (!rect_intersects(b->x1, b->y1, b->x2, b->y2, m->goalPos_x1, m->goalPos_y1, m->goalPos_x2, m->goalPos_y2)) {
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
		m->goalDirectionMask = var4;
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
	m->goalDirectionMask = 0;
	_yMstPos2 = 0;
	_xMstPos2 = 0;
}

int Game::mstTaskStopMonster1(Task *t, MonsterObject1 *m) {
	if (m->monsterInfos[946] & 4) {
		mstBoundingBoxClear(m, 1);
	}
	if (m->goalScreenNum != 0xFC && (m->flagsA5 & 8) != 0 && (t->flags & 0x20) != 0 && m->action) {
		LvlObject *o = m->o16;
		const int bx = _res->_mstPointOffsets[_currentScreen].xOffset;
		const int by = _res->_mstPointOffsets[_currentScreen].yOffset;
		const int ox = o->xPos + _res->_mstPointOffsets[o->screenNum].xOffset;
		const int oy = o->yPos + _res->_mstPointOffsets[o->screenNum].yOffset;
		if (ox < bx || ox + o->width - 1 > bx + 255 || oy < by || oy + o->height - 1 > by + 191) {
			return mstTaskStopMonsterObject1(t);
		}
	}
	mstTaskResetMonster1WalkPath(t);
	return 0;
}

int Game::mstTaskUpdatePositionActionDirection(Task *t, MonsterObject1 *m) {
	if ((m->monsterInfos[946] & 4) == 0 || (_mstLut1[m->goalDirectionMask] & 1) != 0) {
		if (_xMstPos2 < m->m49->unk14) {
			m->goalDirectionMask &= ~kDirectionKeyMaskHorizontal;
		}
		if (_yMstPos2 < m->m49->unk15) {
			m->goalDirectionMask &= ~kDirectionKeyMaskVertical;
		}
	}
// 41B64F
	const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	if ((m->monsterInfos[946] & 4) == 0 && (m->flagsA5 & 4) == 0 && (m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0) {
		int _edi, _ebp;
		if (_mstLut1[m->goalDirectionMask] & 1) {
			_edi = (int8_t)ptr[0xA];
			_ebp = (int8_t)ptr[0xB];
		} else {
			_edi = (int8_t)ptr[0x8];
			_ebp = (int8_t)ptr[0x9];
		}
		int x = m->xMstPos;
		int y = m->yMstPos;
		if (m->goalDirectionMask & kDirectionKeyMaskLeft) {
			x -= _edi;
		} else if (m->goalDirectionMask & kDirectionKeyMaskRight) {
			x += _edi;
		}
		if (m->goalDirectionMask & kDirectionKeyMaskUp) {
			y -= _ebp;
		} else if (m->goalDirectionMask & kDirectionKeyMaskDown) {
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
	if (_mstLut1[m->goalDirectionMask] & 1) {
		_edx = (int8_t)ptr[0xA];
		_ecx = (int8_t)ptr[0xB];
	} else {
		_edx = (int8_t)ptr[0x8];
		_ecx = (int8_t)ptr[0x9];
	}
	if (m->goalDirectionMask == 0) {
		return mstTaskStopMonster1(t, m);
	}
	if (_xMstPos2 < _edx && _yMstPos2 < _ecx) {
		if ((_xMstPos2 <= 0 && _yMstPos2 <= 0) || !mstMonster1TestGoalDirection(m)) {
			return mstTaskStopMonster1(t, m);
		}
	}
// 41B79D
	if ((m->monsterInfos[946] & 4) == 0 && (_mstLut1[m->goalDirectionMask] & 1) != 0) {
		if (_xMstPos2 < _yMstPos2) {
			if (_xMstPos2 < m->m49Unk1->unkA) {
				const uint8_t _al = m->goalDirectionMask;
				m->goalDirectionMask &= ~kDirectionKeyMaskHorizontal;
				if (m->flagsA7 != _al) {
					_xMstPos2 = 0;
				}

			}
		} else {
// 41B7E9
			if (_yMstPos2 < m->m49Unk1->unkB) {
				const uint8_t _al = m->goalDirectionMask;
				m->goalDirectionMask &= ~kDirectionKeyMaskVertical;
				if (m->flagsA7 != _al) {
					_yMstPos2 = 0;
				}
			}
		}
	}
// 41B80E
	ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	mstLvlObjectSetActionDirection(m->o16, ptr, ptr[3], m->goalDirectionMask);
	return 1;
}

// ret >0: found a rect matching
// ret <0: return the closest match (setting _xMstPos3 and _yMstPos3)
int Game::mstMonster1FindWalkPathRect(MonsterObject1 *m, MstWalkPath *walkPath, int x, int y) {
	_xMstPos3 = x;
	_yMstPos3 = y;
	const int num = (~m->flagsA5) & 1;
	int _esi = 0x40000000;
	int var20 = -1;
	int var24 = -1;
	int _ebp = 0;
	for (uint32_t i = 0; i < walkPath->count; ++i, --var24) {
		int _edi;
		MstWalkNode *walkNode = &walkPath->data[i];
		if (walkNode->unk60[num][i] == 0 && m->walkNode != walkNode) {
			continue;
		}
		const uint32_t indexWalkBox = walkNode->walkBox;
		const MstWalkBox *m34 = &_res->_mstWalkBoxData[indexWalkBox];
		if (rect_contains(m34->left, m34->top, m34->right, m34->bottom, x, y)) {
			return i;
		}
// 41A3CE
		if (x >= m34->left && x <= m34->right) {
			int var4 = m34->bottom;
			int var8 = ABS(y - m34->bottom);
			int _ebx = m34->top;
			int _eax = ABS(y - m34->top);
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
		} else if (y >= m34->top && y <= m34->bottom) {
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
			int _eax = (y - m34->top);
			_eax *= _eax;
			_edx += _eax;
			if (_esi >= _edx) {
				_xMstPos3 = m34->left;
				_yMstPos3 = m34->top;
				var20 = var24;
				_esi = _edx;
			}
// 41A4C6
			_edx = x - m34->right;
			_edx *= _edx;
			_eax += _edx;
			if (_esi >= _edx) {
				_xMstPos3 = m34->right;
				_yMstPos3 = m34->top;
				var20 = var24;
			}
// 41A4FB
			_edi = y - m34->bottom;
			_edi *= _edi;
			_edx += _edi;
			if (_esi >= _edi) {
				_xMstPos3 = m34->right;
				_yMstPos3 = m34->bottom;
				var20 = var24;
			}
// 41A529
			_ebp = x - m34->left;
			_ebp *= _ebp;
			_eax = _edi + _ebp;
			if (_esi >= _eax) {
				_xMstPos3 = m34->left;
				_yMstPos3 = m34->bottom;
				var20 = var24;
			}

		}
// 41A560
		// This matches disassembly but looks like a copy-paste from the above condition
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

bool Game::mstTestActionDirection(MonsterObject1 *m, int num) {
	LvlObject *o = m->o16;
	const uint8_t _al = _res->_mstActionDirectionData[num].unk0;
	const uint8_t _bl = _res->_mstActionDirectionData[num].unk2;
	const uint8_t *var4 = m->monsterInfos + _al * 28;
	const uint8_t _dl = (o->flags1 >> 4) & 3;
	uint8_t var8 = ((_dl & 1) != 0) ? 8 : 2;
	if (_dl & 2) {
		var8 |= 4;
	} else {
		var8 |= 1;
	}
	uint8_t directionKeyMask = _bl & 15;
	if ((_bl & 0x10) == 0) {
		const uint32_t _ebp = _bl & 0xE0;
		switch (_ebp) {
		case 32:
		case 96:
		case 160:
		case 192: // 0
			if (_ebp == 192) {
				directionKeyMask |= m->facingDirectionMask & ~kDirectionKeyMaskVertical;
			} else {
				directionKeyMask |= m->facingDirectionMask;
				if (m->monsterInfos[946] & 2) {
					if (_ebp == 160 && (_mstLut1[directionKeyMask] & 1) != 0) {
						if (m->xDelta >= m->yDelta) {
							directionKeyMask &= ~kDirectionKeyMaskVertical;
						} else {
							directionKeyMask &= ~kDirectionKeyMaskHorizontal;
						}
					} else {
						if (m->xDelta >= 2 * m->yDelta) {
							directionKeyMask &= ~kDirectionKeyMaskVertical;
						} else if (m->yDelta >= 2 * m->xDelta) {
							directionKeyMask &= ~kDirectionKeyMaskHorizontal;
						}
					}
				}
			}
			break;
		case 128: // 1
			directionKeyMask |= var8;
			if ((m->monsterInfos[946] & 2) != 0 && (_mstLut1[directionKeyMask] & 1) != 0) {
				if (m->xDelta >= m->yDelta) {
					directionKeyMask &= ~kDirectionKeyMaskVertical;
				} else {
					directionKeyMask &= ~kDirectionKeyMaskHorizontal;
				}
			}
			break;
		default: // 2
			directionKeyMask |= var8;
			break;
		}
	}
// 40E5C0
	directionKeyMask &= var4[2];
	if ((_bl & 0xE0) == 0x40) {
		directionKeyMask ^= kDirectionKeyMaskHorizontal;
	}
	return ((var8 & directionKeyMask) != 0) ? 0 : 1;
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
	const int x = _andyObject->xPos + _andyObject->width / 2;
	const int y = _andyObject->yPos + _andyObject->height / 2;
	return rect_contains(x1, y1, x2, y2, x, y);
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
	return rect_intersects(x1, y1, x2, y2, _andyObject->xPos, _andyObject->yPos, _andyObject->xPos + _andyObject->width - 1, _andyObject->yPos + _andyObject->height - 1);
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
		if (rect_contains(x1, y1, x2, y2, xPos, yPos)) {
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
			if (rect_contains(x1, y1, x2, y2, xPos, yPos)) {
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
	} else if ((flags & 0x40) != 0 && (mstGetFacingDirectionMask(m->facingDirectionMask) & 1) != ((m->o16->flags1 >> 4) & 1) && (m->monsterInfos[946] & 4) == 0) {
		return false;
	} else if ((flags & 0x80) != 0 && (mstGetFacingDirectionMask(m->facingDirectionMask) & 1) != ((m->o16->flags1 >> 4) & 1) && (m->monsterInfos[946] & 4) != 0) {
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
	} else if ((flags & 0x40000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !clipLvlObjectsBoundingBox(_andyObject, m->o16, 36))) {
		return false;
	} else if ((flags & 0x80000) != 0 && (m->o16->screenNum != _andyObject->screenNum || !clipLvlObjectsBoundingBox(_andyObject, m->o16, 20))) {
		return false;
	}
	return true;
}

bool Game::mstMonster1Collide(MonsterObject1 *m, const uint8_t *p) {
	const uint32_t a = READ_LE_UINT32(p + 0x10);
	debug(kDebug_MONSTER, "mstMonster1Collide mask 0x%x flagsA6 0x%x", a, m->flagsA6);
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
		_mstCurrentTask->run = &Game::mstTask_main;
		_mstCurrentTask->monster1->flagsA6 |= 4;
		Task *currentTask = _mstCurrentTask;
		mstTask_main(_mstCurrentTask);
		_mstCurrentTask = currentTask;
		_mstCurrentTask->monster1->flagsA6 &= ~4;
		t.nextPtr = _mstCurrentTask->nextPtr;
		t.prevPtr = _mstCurrentTask->prevPtr;
		memcpy(_mstCurrentTask, &t, sizeof(Task));
		if (_mstCurrentTask->run == &Game::mstTask_idle && (_mstCurrentTask->monster1->flagsA6 & 2) == 0) {
			_mstCurrentTask->run = &Game::mstTask_main;
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
	const uint8_t *_mstCurrentDataPtr = m->monsterInfos + _mstCurrentFlags0 * 28; // _ebx
	int8_t a = _mstCurrentDataPtr[6];
	if (a != 0) {
		const int num = CLIP(m->lut4Index + a, 0, 17);
		o->flags2 = (o->flags2 & ~0x1F) | _mstLut4[num];
	} else {
		o->flags2 = m->o_flags2;
	}
	if (_mstCurrentFlags0 == 31) {
		mstRemoveMonsterObject1(_mstCurrentTask, &_monsterObjects1TasksList);
		return 1;
	}
	const uint32_t _edi = READ_LE_UINT32(_mstCurrentDataPtr + 20);
	if (_edi != kNone) {
		MstBehavior *m46 = _mstCurrentMonster1->m46;
		for (uint32_t i = 0; i < m46->count; ++i) {
			if (m46->data[i].indexMonsterInfo == _edi) {
				mstTaskSetMonster1BehaviorState(_mstCurrentTask, _mstCurrentMonster1, i);
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
		if (m->monsterInfos[946] & 4) {
			mstBoundingBoxClear(m, 1);
		}
		if (t->child) {
			t->child->codeData = 0;
			t->child = 0;
		}
		if ((m->flagsA5 & 8) != 0 && m->action && _mstActionNum != -1) {
			mstTaskStopMonsterObject1(_mstCurrentTask);
			return 0;
		}
		const uint32_t codeData = m->behaviorState->codeData;
		if (codeData != kNone) {
			resetTask(t, _res->_mstCodeData + codeData * 4);
			return 0;
		}
		o->actionKeyMask = 7;
		o->directionKeyMask = 0;
		t->run = &Game::mstTask_idle;
		return 0;
	}
// 41833A
	if (t->run == &Game::mstTask_monsterWait4) {
		return 0;
	}
	if (_mstCurrentDataPtr[0] != 0) {
		mstMonster1Collide(_mstCurrentMonster1, _mstCurrentDataPtr);
		return 0;
	}
	if ((m->flagsA5 & 0x40) != 0) {
		return 0;
	}
// 418384
	assert(_mstCurrentMonster1 == m);
	int dir = 0;
	for (int i = 0; i < _andyShootsCount; ++i) {
		AndyShootData *p = &_andyShootsTable[i];
		if (p->m && p->m != m) {
			continue;
		}
		if (m->collideDistance < 0) {
			continue;
		}
		if ((m->flags48 & 4) == 0) {
			continue;
		}
		if (m->behaviorState->indexUnk51 == kNone) {
			continue;
		}
		if (_rnd.getNextNumber() > m->behaviorState->unk10) {
			continue;
		}
		m->shootActionIndex = 8;
		int var28 = 0;
		AndyShootData *var14 = m->shootData;
		if (t->run != &Game::mstTask_monsterWait5 && t->run != &Game::mstTask_monsterWait6 && t->run != &Game::mstTask_monsterWait7 && t->run != &Game::mstTask_monsterWait8 && t->run != &Game::mstTask_monsterWait9 && t->run != &Game::mstTask_monsterWait10) {
			if (m->monsterInfos[946] & 2) {
				_mstCurrentMonster1->goalPos_x1 = _mstCurrentMonster1->goalPos_y1 = INT32_MIN;
				_mstCurrentMonster1->goalPos_x2 = _mstCurrentMonster1->goalPos_y2 = INT32_MAX;
				switch (var14->directionMask) {
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
				_mstCurrentMonster1->shootActionIndex = _mstLut1[var28];
			}
		} else {
// 4184EC
			m->shootActionIndex = _mstLut1[m->goalDirectionMask];
			var28 = 0;
		}
// 418508
		uint32_t var24 = 0;
		MstBehaviorState *_esi = m->behaviorState;
		if (_esi->count != 0) {
			var24 = _rnd.update() % (_esi->count + 1);
		}
// 418530
		int var20 = -1;
		int indexUnk51;
		if ((m->flagsA5 & 8) != 0 && m->action && m->action->indexUnk51 != kNone && m->monsterInfos == &_res->_mstMonsterInfos[m->action->unk0 * 948]) {
			indexUnk51 = m->action->indexUnk51;
		} else {
			indexUnk51 = _esi->indexUnk51;
		}
		int _ebx = -1;
		const MstShootIndex *var18 = &_res->_mstShootIndexData[indexUnk51]; // _esi
		for (; var24 < var18->count; ++var24) {
			assert(m->shootActionIndex >= 0 && m->shootActionIndex < 9);
			const uint32_t indexUnk50Unk1 = var18->indexUnk50Unk1[var24 * 9 + m->shootActionIndex];
			const MstShootAction *m50Unk1 = &_res->_mstShootData[var18->indexUnk50].data[indexUnk50Unk1];
			const int32_t hSize = m50Unk1->hSize;
			const int32_t vSize = m50Unk1->vSize;
			if (hSize != 0 || vSize != 0) {
				int dirMask = 0;
				int _ebp = 0;
				if ((m50Unk1->unk4 & kDirectionKeyMaskHorizontal) != kDirectionKeyMaskHorizontal && (m->o16->flags1 & 0x10) != 0) {
					_mstTemp_x1 = m->xMstPos - hSize;
				} else {
					_mstTemp_x1 = m->xMstPos + hSize;
				}
				if (_mstTemp_x1 < m->xMstPos) {
					if (_mstTemp_x1 < m->goalPos_x1) {
						dirMask = kDirectionKeyMaskLeft;
					}
					_ebp = kDirectionKeyMaskLeft;
				} else if (_mstTemp_x1 > m->xMstPos) {
					if (_mstTemp_x1 > m->goalPos_x2) {
						dirMask = kDirectionKeyMaskRight;
					}
					_ebp = kDirectionKeyMaskRight;
				} else {
					_ebp = 0;
				}
				_mstTemp_y1 = m->yMstPos + vSize;
				if (_mstTemp_y1 < m->yMstPos) {
					if (_mstTemp_y1 < m->goalPos_y1 && (m->monsterInfos[946] & 2) != 0) {
						dirMask |= kDirectionKeyMaskUp;
					}
					_ebp |= kDirectionKeyMaskUp;
				} else if (_mstTemp_y1 > m->yMstPos) {
					if (_mstTemp_y1 > m->goalPos_y2 && (m->monsterInfos[946] & 2) != 0) {
						dirMask |= kDirectionKeyMaskDown;
					}
					_ebp |= kDirectionKeyMaskDown;
				}
				if (var28 == dirMask) {
					continue;
				}
				if (mstMonster1CheckLevelBounds(m, _mstTemp_x1, _mstTemp_y1, dirMask)) {
					continue;
				}
			}
// 41866A
			_mstTemp_x1 = m->xMstPos;
			if ((m50Unk1->unk4 & kDirectionKeyMaskHorizontal) != kDirectionKeyMaskHorizontal && (m->o16->flags1 & 0x10) != 0) {
				_mstTemp_x1 -= m50Unk1->width;
				_mstTemp_x1 -= m50Unk1->xPos;
			} else {
				_mstTemp_x1 += m50Unk1->xPos;
			}
			_mstTemp_y1 = m->yMstPos + m50Unk1->yPos;
			_mstTemp_x2 = _mstTemp_x1 + m50Unk1->width - 1;
			_mstTemp_y2 = _mstTemp_y1 + m50Unk1->height - 1;
			if ((m->monsterInfos[946] & 4) != 0 && mstBoundingBoxCollides1(m->monster1Index, _mstTemp_x1, _mstTemp_y1, _mstTemp_x2, _mstTemp_y2)) {
				continue;
			}
			int _ecx;
			if (m->monsterInfos[946] & 2) {
				_ecx = var14->boundingBox.y2;
			} else {
				_ecx = m->yMstPos;
			}
			if (m50Unk1->width != 0 && getMstDistance(_ecx, var14) >= 0) {
				continue;
			}
			if (m->collideDistance >= m50Unk1->unk24) {
// 418766
				MstBehaviorState *behaviorState = m->behaviorState;
				_ebx = var24;
				int _edi = m50Unk1->unk24;
				if (behaviorState->unk18 != 0) {
					_edi += (_rnd.update() % (behaviorState->unk18 * 2 + 1)) - behaviorState->unk18;
					if (_edi < 0) {
						_edi = 0;
					}
				}
				dir = _edi;
				break;
			}
			if (var20 == -1) {
				dir = m50Unk1->unk24;
				var20 = var24;
			}
			_ebx = var20;
		}
		if (_ebx >= 0) {
// 4187BC
			const uint32_t indexUnk50Unk1 = var18->indexUnk50Unk1[_ebx * 9 + m->shootActionIndex];
			MstShootAction *m50Unk1 = &_res->_mstShootData[var18->indexUnk50].data[indexUnk50Unk1];
			mstTaskAttack(_mstCurrentTask, m50Unk1->codeData, 0x40);
			_mstCurrentMonster1->unkF8 = m50Unk1->unk8;
			_mstCurrentMonster1->shootSource = dir;
			_mstCurrentMonster1->shootDirection = var14->directionMask;
			_mstCurrentMonster1->directionKeyMask = _andyObject->directionKeyMask;
			return 0;
		}
	}
// 41882E
	if (o->screenNum == _currentScreen && (m->flagsA5 & 0x20) == 0 && (m->flags48 & 0x10) != 0) {
		MstBehaviorState *behaviorState = m->behaviorState;
		if (behaviorState->attackBox != kNone) {
			const MstAttackBox *m47 = &_res->_mstAttackBoxData[behaviorState->attackBox];
			if (m47->count > 0) {
				const uint8_t dir = (o->flags1 >> 4) & 3;
				const uint8_t *p = m47->data;
				for (uint32_t i = 0; i < m47->count; ++i) {
					int32_t a = READ_LE_UINT32(p); // x1
					int32_t b = READ_LE_UINT32(p + 4); // y1
					int32_t c = READ_LE_UINT32(p + 8); // x2
					int32_t d = READ_LE_UINT32(p + 12); // y2
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

					if (rect_contains(x1, y1, x2, y2, _mstAndyLevelPosX, _mstAndyLevelPosY)) {
						mstTaskAttack(_mstCurrentTask, READ_LE_UINT32(p + 16), 0x20);
						mstMonster1Collide(_mstCurrentMonster1, _mstCurrentDataPtr);
						return 0;
					}

					p += 20;
				}
			}
		}
	}
// 418939
	if (mstMonster1Collide(_mstCurrentMonster1, _mstCurrentDataPtr)) {
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
		MstWalkNode *walkNode = _mstCurrentMonster1->walkNode;
		if (walkNode->walkCodeStage2 == kNone) {
			return 0;
		}
		int _ebp = 0;
		if (_mstAndyLevelPosY >= m->yMstPos - walkNode->y1 && _mstAndyLevelPosY < m->yMstPos + walkNode->y2) {
			if (walkNode->x1 != -2 || walkNode->x1 != walkNode->x2) {
				if (m->xDelta <= walkNode->x1) {
					if (_al & 1) {
						_ebp = 1;
					} else {
						_al = mstGetFacingDirectionMask(_mstCurrentMonster1->facingDirectionMask) & 1;
						_dl = (_mstCurrentMonster1->o16->flags1 >> 4) & 1;
						if (_dl == _al || (_mstCurrentMonster1->monsterInfos[946] & 4) != 0) {
							_ebp = 1;
						} else if (m->xDelta <= walkNode->x2) {
							_ebp = 2;
						}
					}
				}
// 418B1B
			} else if (o->screenNum == _currentScreen) {
				if (_al & 1) {
					_ebp = 1;
				} else {
					_al = mstGetFacingDirectionMask(_mstCurrentMonster1->facingDirectionMask) & 1;
					_dl = (_mstCurrentMonster1->o16->flags1 >> 4) & 1;
					if (_al != _dl && (_mstCurrentMonster1->monsterInfos[946] & 4) == 0) {
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
				const uint32_t indexWalkCode = m->walkNode->walkCodeReset[0];
				assert(indexWalkCode != kNone);
				m->walkCode = &_res->_mstWalkCodeData[indexWalkCode];
			}
			return 0;
		} else if (_ebp == 1) {
// 418C73
			m->flagsA6 |= 1;
			assert(_mstCurrentMonster1 == m);
			if (mstSetCurrentPos(m, m->xMstPos, m->yMstPos) == 0 && (m->monsterInfos[946] & 2) == 0) {
				if ((_mstCurrentPosX > m->xMstPos && _mstCurrentPosX > m->walkNode->coords[0][1]) || (_mstCurrentPosX < m->xMstPos && _mstCurrentPosX < m->walkNode->coords[1][1])) {
					uint32_t indexWalkCode = m->walkNode->walkCodeStage1;
					if (indexWalkCode != kNone) {
						m->walkCode = &_res->_mstWalkCodeData[indexWalkCode];
					}
					if (m->flagsA5 & 4) {
						m->flagsA5 &= ~4;
						if (!mstMonster1UpdateWalkPath(m)) {
							mstMonster1ResetWalkPath(m);
						}
						indexWalkCode = m->walkNode->walkCodeStage1;
						if (indexWalkCode != kNone) {
							m->walkCode = &_res->_mstWalkCodeData[indexWalkCode];
						}
						mstTaskSetNextWalkCode(_mstCurrentTask);
					}
					return 0;
				}
			}
// 418D41
			if ((m->monsterInfos[946] & 2) == 0) {
				MstWalkNode *walkPath = m->walkNode;
				int _edi = READ_LE_UINT32(m->monsterInfos + 904);
				int _ebx = MAX(m->unk88, walkPath->coords[1][1] + _edi);
				int _eax = MIN(m->unk84, walkPath->coords[0][1] - _edi);
				const uint32_t indexUnk36 = walkPath->movingBoundsIndex2;
				assert(indexUnk36 != kNone);
				const uint32_t indexUnk49 = _res->_mstMovingBoundsIndexData[indexUnk36].indexUnk49;
				assert(indexUnk49 != kNone);
				uint8_t _bl = _res->_mstMovingBoundsData[indexUnk49].unk14;
				if (ABS(_eax - _ebx) <= _bl) {
					uint32_t indexWalkCode = walkPath->walkCodeStage1;
					if (indexWalkCode != kNone) {
						m->walkCode = &_res->_mstWalkCodeData[indexWalkCode];
					}
					if (m->flagsA5 & 4) {
						m->flagsA5 &= ~4;
						if (!mstMonster1UpdateWalkPath(_mstCurrentMonster1)) {
							mstMonster1ResetWalkPath(_mstCurrentMonster1);
						}
						indexWalkCode = _mstCurrentMonster1->walkNode->walkCodeStage1;
						if (indexWalkCode != kNone) {
							m->walkCode = &_res->_mstWalkCodeData[indexWalkCode];
						}
						mstTaskSetNextWalkCode(_mstCurrentTask);
					}
					return 0;
				}
			}
// 418DEA
			mstTaskInitMonster1Type2(_mstCurrentTask, 0);
			return 0;
		}
		assert(_ebp == 2);
		if (m->flagsA6 & 1) {
			return 0;
		}
		const uint32_t indexWalkCode = m->walkNode->walkCodeStage2;
		assert(indexWalkCode != kNone);
		MstWalkCode *m35 = &_res->_mstWalkCodeData[indexWalkCode];
		if (m->walkCode != m35) {
			_mstCurrentMonster1->walkCode = m35;
			_rnd.resetMst(_mstCurrentMonster1->rnd_m35);
			mstTaskSetNextWalkCode(_mstCurrentTask);
			return 0;
		}
// 418C1D
		if (m->flagsA5 & 4) {
			m->flagsA5 &= ~4;
			if (!mstMonster1UpdateWalkPath(_mstCurrentMonster1)) {
				mstMonster1ResetWalkPath(_mstCurrentMonster1);
			}
			const uint32_t indexWalkCode = m->walkNode->walkCodeStage2;
			assert(indexWalkCode != kNone);
			_mstCurrentMonster1->walkCode = &_res->_mstWalkCodeData[indexWalkCode];
			mstTaskSetNextWalkCode(_mstCurrentTask);
		}
		return 0;
	} else if (dir != 2) {
		return 0;
	}
	if ((m->flagsA5 & 4) != 0 || (m->flags48 & 8) == 0) {
		return 0;
	}
	if ((m->flagsA5 & 8) == 0 && (m->monsterInfos[946] & 2) == 0) {
		const uint8_t _dl = m->facingDirectionMask;
		if (_dl & 2) {
			if ((int32_t)READ_LE_UINT32(m->monsterInfos + 916) <= m->walkNode->coords[1][1] || (int32_t)READ_LE_UINT32(m->monsterInfos + 912) >= m->walkNode->coords[0][1]) {
				m->flagsA6 |= 1;
				assert(m == _mstCurrentMonster1);
				m->flagsA5 = 1;
				mstMonster1ResetWalkPath(m);
				const uint32_t indexWalkCode = m->walkNode->walkCodeStage1;
				if (indexWalkCode != kNone) {
					m->walkCode = &_res->_mstWalkCodeData[indexWalkCode];
				}
				return 0;
			}
		} else if (_dl & 8) {
// 418A37
			if ((int32_t)READ_LE_UINT32(m->monsterInfos + 920) >= m->walkNode->coords[0][1] || (int32_t)READ_LE_UINT32(m->monsterInfos + 924) <= m->walkNode->coords[1][1]) {
				m->flagsA6 |= 1;
				assert(m == _mstCurrentMonster1);
				m->flagsA5 = 1;
				mstMonster1ResetWalkPath(m);
				const uint32_t indexWalkCode = m->walkNode->walkCodeStage1;
				if (indexWalkCode != kNone) {
					m->walkCode = &_res->_mstWalkCodeData[indexWalkCode];
				}
				return 0;
			}
		}
	}
// 418A9A
	if (mstSetCurrentPos(m, m->xMstPos, m->yMstPos) == 0) {
		mstTaskInitMonster1Type2(t, 1);
	}
	return 0;
}

int Game::mstUpdateTaskMonsterObject2(Task *t) {
	debug(kDebug_MONSTER, "mstUpdateTaskMonsterObject2 t %p", t);
	mstTaskSetMonster2ScreenPosition(t);
	MonsterObject2 *m = t->monster2;
	if (_currentLevel == kLvl_fort && m->monster2Info->type == 27) {
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
	for (int i = 0; i < _andyShootsCount; ++i) {
		AndyShootData *p = &_andyShootsTable[i];
		if (p->type == 2) {
			_dl |= 1;
		} else if (p->type == 1) {
			_dl |= 2;
		}
	}
// 4191DC
	LvlObject *o = m->o;
	MstInfoMonster2 *monster2Info = m->monster2Info;
	const uint8_t _bl = monster2Info->shootMask;
	if ((_bl & _dl) != 0) {
		for (int i = 0; i < _andyShootsCount; ++i) {
			AndyShootData *p = &_andyShootsTable[i];
			if (p->type == 2 && (_bl & 1) == 0) {
				continue;
			} else if (p->type == 1 && (_bl & 2) == 0) {
				continue;
			}
			if (o->screenNum != _currentScreen || p->o->screenNum != _currentScreen) {
				continue;
			}
			if (!clipLvlObjectsBoundingBox(p->o, o, 20)) {
				continue;
			}
			ShootLvlObjectData *s = p->shootObjectData;
			s->unk3 = 0x80;
			s->xPosShoot = o->xPos + o->width / 2;
			s->yPosShoot = o->yPos + o->height / 2;
			if (p->type != 2 || (_bl & 4) != 0) {
				continue;
			}
			const uint32_t codeData = monster2Info->codeData2;
			if (codeData != kNone) {
				resetTask(t, _res->_mstCodeData + codeData * 4);
			} else {
				o->actionKeyMask = 7;
				o->directionKeyMask = 0;
				t->run = &Game::mstTask_idle;
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
	_andyShootsCount = 0;
	_andyShootsTable[0].type = 0;
	if (!_lvlObjectsList0) {
		if (_plasmaCannonDirection == 0) {
			_executeMstLogicPrevCounter = _executeMstLogicCounter;
			return;
		}
		_andyShootsTable[0].width  = 512;
		_andyShootsTable[0].height = 512;
		_andyShootsTable[0].shootObjectData = 0;
		_andyShootsTable[0].type = 3;
		_andyShootsTable[0].size = 4;
		_andyShootsTable[0].xPos = _plasmaCannonPosX[_plasmaCannonFirstIndex] + _res->_mstPointOffsets[_currentScreen].xOffset;
		_andyShootsTable[0].yPos = _plasmaCannonPosY[_plasmaCannonFirstIndex] + _res->_mstPointOffsets[_currentScreen].yOffset;
		switch (_plasmaCannonDirection - 1) {
		case 0:
			_andyShootsTable[0].directionMask = 6;
			_andyShootsCount = 1;
			break;
		case 2:
			_andyShootsTable[0].directionMask = 3;
			_andyShootsCount = 1;
			break;
		case 1:
			_andyShootsTable[0].directionMask = 0;
			_andyShootsCount = 1;
			break;
		case 5:
			_andyShootsTable[0].directionMask = 4;
			_andyShootsCount = 1;
			break;
		case 3:
			_andyShootsTable[0].directionMask = 7;
			_andyShootsCount = 1;
			break;
		case 11:
			_andyShootsTable[0].directionMask = 2;
			_andyShootsCount = 1;
			break;
		case 7:
			_andyShootsTable[0].directionMask = 5;
			_andyShootsCount = 1;
			break;
		case 8:
			_andyShootsTable[0].directionMask = 1;
			_andyShootsCount = 1;
			break;
		default:
			_andyShootsCount = 1;
			break;
		}
	} else {
		AndyShootData *p = _andyShootsTable;
		for (LvlObject *o = _lvlObjectsList0; o; o = o->nextPtr) {
			p->o = o;
			assert(o->dataPtr);
			ShootLvlObjectData *ptr = (ShootLvlObjectData *)getLvlObjectDataPtr(o, kObjectDataTypeShoot);
			p->shootObjectData = ptr;
			if (ptr->unk3 == 0x80) {
				continue;
			}
			if (ptr->dxPos == 0 && ptr->dyPos == 0) {
				continue;
			}
			p->width  = ptr->dxPos;
			p->height = ptr->dyPos;
			p->directionMask = ptr->state;
			switch (ptr->unk0) {
			case 0:
				p->type = 1;
				p->xPos = o->xPos + _res->_mstPointOffsets[o->screenNum].xOffset + o->posTable[7].x;
				p->size = 3;
				p->yPos = o->yPos + _res->_mstPointOffsets[o->screenNum].yOffset + o->posTable[7].y;
				break;
			case 5:
				p->directionMask |= 0x80;
				// fall-through
			case 4:
				p->type = 2;
				p->xPos = o->xPos + _res->_mstPointOffsets[o->screenNum].xOffset + o->posTable[7].x;
				p->size = 7;
				p->yPos = o->yPos + _res->_mstPointOffsets[o->screenNum].yOffset + o->posTable[7].y;
				break;
			default:
				--p;
				--_andyShootsCount;
				break;
			}
			++p;
			++_andyShootsCount;
			if (_andyShootsCount >= kMaxAndyShoots) {
				break;
			}
		}
		if (_andyShootsCount == 0) {
			_executeMstLogicPrevCounter = _executeMstLogicCounter;
			return;
		}
	}
	for (int i = 0; i < _andyShootsCount; ++i) {
		AndyShootData *p = &_andyShootsTable[i];
		p->boundingBox.x2 = p->xPos + p->size;
		p->boundingBox.x1 = p->xPos - p->size;
		p->boundingBox.y2 = p->yPos + p->size;
		p->boundingBox.y1 = p->yPos - p->size;
	}
}

void Game::mstUpdateMonstersRect() {
	const int _mstAndyLevelPosDx = _mstAndyLevelPosX - _mstAndyLevelPrevPosX;
	const int _mstAndyLevelPosDy = _mstAndyLevelPosY - _mstAndyLevelPrevPosY;
	_mstAndyLevelPrevPosX = _mstAndyLevelPosX;
	_mstAndyLevelPrevPosY = _mstAndyLevelPosY;
	if (_mstAndyLevelPosDx == 0 && _mstAndyLevelPosDy == 0) {
		return;
	}
	int offset = 0;
	for (int i = 0; i < _res->_mstHdr.infoMonster1Count; ++i) {
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
	m->monster2Info = 0;
	LvlObject *o = m->o;
	if (o) {
		o->dataPtr = 0;
		removeLvlObject2(o);
	}
	removeTask(tasksList, t);
}

void Game::mstRemoveMonsterObject1(Task *t, Task **tasksList) {
	MonsterObject1 *m = t->monster1;
	if (_mstActionNum != -1) {
		if ((m->flagsA5 & 8) != 0 && m->action) {
			mstMonster1ClearChasingMonster(m);
		}
	}
	if (m->monsterInfos[946] & 4) {
		mstBoundingBoxClear(m, 0);
		mstBoundingBoxClear(m, 1);
	}
	m->m46 = 0;
	LvlObject *o = m->o16;
	if (o) {
		o->dataPtr = 0;
	}
	for (int i = 0; i < kMaxMonsterObjects2; ++i) {
		if (_monsterObjects2Table[i].monster2Info != 0 && _monsterObjects2Table[i].monster1 == m) {
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
			if (t->run != &Game::mstTask_wait2) {
				const uint8_t *p = n->codeData - 4;
				if ((t->flags & 0x40) != 0 || p[0] == 203 || ((flags & 0x10) != 0 && (t->run == &Game::mstTask_monsterWait1 || t->run == &Game::mstTask_monsterWait2 || t->run == &Game::mstTask_monsterWait3 || t->run == &Game::mstTask_monsterWait4))) {
					p += 4;
				}
				n->codeData = p;
				n->run = &Game::mstTask_main;
			}
		}
	}
// 417A2B
	assert(codeData != kNone);
	resetTask(t, _res->_mstCodeData + codeData * 4);
}

int Game::mstTaskSetActionDirection(Task *t, int num, int delay) {
	MonsterObject1 *m = t->monster1;
	LvlObject *o = m->o16;
	uint8_t var4 = _res->_mstActionDirectionData[num].unk0;
	uint8_t var8 = _res->_mstActionDirectionData[num].unk2;
	const uint8_t *ptr = m->monsterInfos + var4 * 28;
	uint8_t _al = (o->flags1 >> 4) & 3;
	uint8_t _cl = ((_al & 1) != 0) ? 8 : 2;
	if (_al & 2) {
		_cl |= 4;
	} else {
		_cl |= 1;
	}
	mstLvlObjectSetActionDirection(o, ptr, var8, _cl);
	const uint8_t am = _res->_mstActionDirectionData[num].unk1;
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
		if (((var11 & kDirectionKeyMaskHorizontal) == kDirectionKeyMaskHorizontal && (o->directionKeyMask & 8) != 0) || ((var11 & kDirectionKeyMaskHorizontal) != kDirectionKeyMaskHorizontal && (o->flags1 & 0x10) != 0)) {
			_edi = m->xMstPos - _edi;
		} else {
			_edi = m->xMstPos + _edi;
		}
		if (_edi < m->xMstPos) {
			var10 = 8;
		} else {
			var10 = (_edi <= m->xMstPos) ? 0 : 2;
		}
		if ((var11 & kDirectionKeyMaskVertical) == kDirectionKeyMaskVertical && (o->directionKeyMask & 1) != 0) {
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
		if (((var10 & 8) != 0 && _edi < m->levelPosBounds_x1) || ((var10 & 2) != 0 && _edi > m->levelPosBounds_x2) || ((var10 & 1) != 0 && (_ebp < m->levelPosBounds_y1))) {
			t->flags |= 0x80;
			return 0;
		}
		_eax = var10;
// 40E9BC
	}
	if ((m->monsterInfos[946] & 4) != 0 && ptr[14] != 0) {
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
	m->o_flags0 = var4;
	if (delay == -1) {
		const uint32_t offset = m->monsterInfos - _res->_mstMonsterInfos;
		assert((offset % kMonsterInfoDataSize) == 0);
		t->arg2 = offset / kMonsterInfoDataSize;
		t->run = &Game::mstTask_monsterWait4;
		debug(kDebug_MONSTER, "mstTaskSetActionDirection arg2 %d", t->arg2);
	} else {
		t->arg1 = delay;
		t->run = &Game::mstTask_monsterWait3;
		debug(kDebug_MONSTER, "mstTaskSetActionDirection arg1 %d", t->arg1);
	}
	return 1;
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

void Game::updateTask(Task *t, int num, const uint8_t *codeData) {
	debug(kDebug_MONSTER, "updateTask t %p offset 0x%04x", t, codeData - _res->_mstCodeData);
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
					t->run = &Game::mstTask_main;
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
	t->run = &Game::mstTask_main;
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
				const MstBehaviorState *behaviorState = m->behaviorState;
				if (behaviorState->indexUnk51 != kNone) {
					m->flags48 |= 4;
				}
				if (behaviorState->attackBox != kNone) {
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
		if (t->monster1 && t->monster1->shootData) {
			return t->monster1->shootData->type;
		}
		return _andyShootsTable[0].type;
	case 8:
		if (t->monster1 && t->monster1->shootData) {
			return t->monster1->shootData->directionMask & 0x7F;
		}
		return _andyShootsTable[0].directionMask & 0x7F;
	case 9:
		if (t->monster1 && t->monster1->action) {
			return t->monster1->action->xPos;
		}
		break;
	case 10:
		if (t->monster1 && t->monster1->action) {
			return t->monster1->action->yPos;
		}
		break;
	case 11:
		if (t->monster1) {
			return t->monster1->shootSource;
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
			return ABS(t->monster1->levelPosBounds_x1 - t->monster1->xMstPos);
		}
		break;
	case 19:
		if (t->monster1) {
			return ABS(t->monster1->levelPosBounds_x2 - t->monster1->xMstPos);
		}
		break;
	case 20:
		if (t->monster1) {
			return ABS(t->monster1->levelPosBounds_y1 - t->monster1->yMstPos);
		}
		break;
	case 21:
		if (t->monster1) {
			return ABS(t->monster1->levelPosBounds_y2 - t->monster1->yMstPos);
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
		return _level->_checkpoint;
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

int Game::mstTask_main(Task *t) {
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
						t->run = &Game::mstTask_wait2;
						ret = 1;
					} else {
						t->run = &Game::mstTask_wait1;
						ret = 1;
					}
				}
			}
			break;
		case 2: { // 2 - set_var_random_range
				const int num = READ_LE_UINT16(p + 2);
				MstOp2Data *m = &_res->_mstOp2Data[num];
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
				const int arg = _res->_mstActionDirectionData[num].unk3;
				t->codeData = p;
				ret = mstTaskSetActionDirection(t, num, (arg == 0xFF) ? -1 : arg);
			}
			break;
		case 4: // 4 - set_monster_action_direction_task_var
			if (t->monster1) {
				const int num = READ_LE_UINT16(p + 2);
				const int arg = _res->_mstActionDirectionData[num].unk3;
				t->codeData = p;
				assert(arg < kMaxLocals);
				ret = mstTaskSetActionDirection(t, num, t->localVars[arg]);
			}
			break;
		case 13: // 8
			if (t->monster1) {
				const int num = READ_LE_UINT16(p + 2);
				if (mstTestActionDirection(t->monster1, num)) {
					const int arg = _res->_mstActionDirectionData[num].unk3;
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
					t->run = &Game::mstTask_wait3;
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
						t->run = &Game::mstTask_wait3;
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
			if (p[1] < _res->_mstHdr.screensCount) {
				mstOp26_removeMstTaskScreen(&_monsterObjects1TasksList, p[1]);
				mstOp26_removeMstTaskScreen(&_monsterObjects2TasksList, p[1]);
				// mstOp26_removeMstTaskScreen(&_mstTasksList3, p[1]);
				// mstOp26_removeMstTaskScreen(&_mstTasksList4, p[1]);
			}
			break;
		case 40: // 27 - remove_monsters_screen_type
			if (p[1] < _res->_mstHdr.screensCount) {
				mstOp27_removeMstTaskScreenType(&_monsterObjects1TasksList, p[1], p[2]);
				mstOp27_removeMstTaskScreenType(&_monsterObjects2TasksList, p[1], p[2]);
				// mstOp27_removeMstTaskScreenType(&_mstTasksList3, p[1], p[2]);
				// mstOp27_removeMstTaskScreenType(&_mstTasksList4, p[1], p[2]);
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
		case 55:
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
				const MstOp197Data *op197Data = &_res->_mstOp197Data[num];
				const uint32_t mask = op197Data->maskVars;
				int a = getTaskVar(t, op197Data->unk0, (mask >> 16) & 15); // var1C
				int b = getTaskVar(t, op197Data->unk2, (mask >> 12) & 15); // x2
				int c = getTaskVar(t, op197Data->unk4, (mask >>  8) & 15); // var14
				int d = getTaskVar(t, op197Data->unk6, (mask >>  4) & 15); // _esi
				int e = getTaskVar(t, op197Data->unkE,  mask        & 15); // _eax
				if (e >= _res->_mstHdr.screensCount) {
					e = _res->_mstHdr.screensCount - 1;
				}
				ret = mstOp49_setMovingBounds(a, b, c, d, e, t, num);
			}
			break;
		case 198: { // 50 - call_task
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
		case 199: // 51 - stop_monster
			mstTaskStopMonsterObject1(t);
			return 0;
		case 200: // 52 - stop_monster_action
			if (t->monster1 && t->monster1->action) {
				mstOp52();
				return 1;
			}
			mstOp52();
			break;
		case 201: { // 53 - start_monster_action
				const int num = READ_LE_UINT16(p + 2);
				mstOp53(&_res->_mstMonsterActionData[num]);
			}
			break;
		case 202: // 54 - continue_monster_action
			mstOp54();
			break;
		case 203: // 55 - monster_attack
			// _mstCurrentMonster1 = t->monster1;
			if (t->monster1) {
				const int num = READ_LE_UINT16(p + 2);
				if (mstCollidesByFlags(t->monster1, _res->_mstOp240Data[num].flags)) {
					t->codeData = p + 4;
					mstTaskAttack(t, _res->_mstOp240Data[num].codeData, 0x10);
					t->state &= ~2;
					p = t->codeData - 4;
				}
			}
			break;
		case 204: // 56 - special_action
			ret = mstOp56_specialAction(t, p[1], READ_LE_UINT16(p + 2));
			break;
		case 207:
		case 208:
		case 209: // 79
			break;
		case 210: // 57 - add_worm
			{
				MonsterObject1 *m = t->monster1;
				mstOp57_addWormHoleSprite(m->xPos + (int8_t)p[2], m->yPos + (int8_t)p[3], m->o16->screenNum);
			}
			break;
		case 211: // 58 - add_lvl_object
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
				if ((t->monster1 && (t->monster1->monsterInfos[944] == 10 || t->monster1->monsterInfos[944] == 25)) || (t->monster2 && (t->monster2->monster2Info->type == 10 || t->monster2->monster2Info->type == 25))) {
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
					mstOp59_addShootSpecialPowers(xPos, yPos, o->screenNum, type, (o->flags2 + 1) & 0xDFFF);
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
					m->localVars[7] = m->behaviorState->energy;
				}
			}
			break;
		case 215: { // 62
				if (_m43Num3 != -1) {
					assert(_m43Num3 < _res->_mstHdr.monsterActionIndexDataCount);
					shuffleMstMonsterActionIndex(&_res->_mstMonsterActionIndexData[_m43Num3]);
				}
				_mstOp54Counter = 0;
			}
			break;
		case 217: { // 64
				const int16_t num = READ_LE_UINT16(p + 2);
				if (_m43Num3 != num) {
					_m43Num3 = num;
					assert(num >= 0 && num < _res->_mstHdr.monsterActionIndexDataCount);
					shuffleMstMonsterActionIndex(&_res->_mstMonsterActionIndexData[num]);
					_mstOp54Counter = 0;
				}
			}
			break;
		case 218: { // 65
				const int16_t num = READ_LE_UINT16(p + 2);
				if (num != _m43Num1) {
					_m43Num1 = num;
					_m43Num2 = num;
					assert(num >= 0 && num < _res->_mstHdr.monsterActionIndexDataCount);
					shuffleMstMonsterActionIndex(&_res->_mstMonsterActionIndexData[num]);
				}
			}
			break;
		case 220:
		case 221:
		case 222:
		case 223:
		case 224:
		case 225: { // 67 - add_monster
				const int num = READ_LE_UINT16(p + 2);
				MstOp223Data *m = &_res->_mstOp223Data[num];
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
				e = CLIP(e, -1, _res->_mstHdr.screensCount - 1);
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
		case 226: { // 68 - add_monster_group
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
					if (m->monsterInfos[944] != _res->_mstMonsterInfos[m226Data->unk0 * kMonsterInfoDataSize + 944]) {
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
		case 227: { // 69 - compare_vars
				const int num = READ_LE_UINT16(p + 2);
				assert(num < _res->_mstHdr.op227DataCount);
				const MstOp227Data *m = &_res->_mstOp227Data[num];
				const int a = getTaskVar(t, m->indexVar1, m->maskVars & 15);
				const int b = getTaskVar(t, m->indexVar2, m->maskVars >> 4);
				if (compareOp(m->compare, a, b)) {
					assert(m->codeData != kNone);
					p = _res->_mstCodeData + m->codeData * 4 - 4;
				}
			}
			break;
		case 228: { // 70 - compare_flags
				const int num = READ_LE_UINT16(p + 2);
				assert(num < _res->_mstHdr.op227DataCount);
				const MstOp227Data *m = &_res->_mstOp227Data[num];
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
				const MstOp234Data *m = &_res->_mstOp234Data[num];
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
						t->run = &Game::mstTask_mstOp231;
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
						t->run = &Game::mstTask_mstOp232;
						ret = 1;
					}
				}
			}
			break;
		case 233:
		case 234: { // 72
				const int num = READ_LE_UINT16(p + 2);
				const MstOp234Data *m = &_res->_mstOp234Data[num];
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
						t->run = &Game::mstTask_mstOp233;
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
						t->run = &Game::mstTask_mstOp234;
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
		case 238: { // 75 - jmp
				const int i = READ_LE_UINT16(p + 2);
				const uint32_t codeData = _res->_mstUnk60[i];
				assert(codeData != kNone);
				p = _res->_mstCodeData + codeData * 4;
				t->codeData = p;
				p -= 4;
			}
			break;
		case 239: { // 76  - create_task
				const int i = READ_LE_UINT16(p + 2);
				const uint32_t codeData = _res->_mstUnk60[i];
				assert(codeData != kNone);
				createTask(_res->_mstCodeData + codeData * 4);
			}
			break;
		case 240: { // 77 - update_task
				const int num = READ_LE_UINT16(p + 2);
				MstOp240Data *m = &_res->_mstOp240Data[num];
				const uint8_t *codeData = (m->codeData == kNone) ? 0 : (_res->_mstCodeData + m->codeData * 4);
				updateTask(t, m->flags, codeData);
			}
			break;
		case 242: // 78 - terminate
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
					if ((m->flagsA5 & 8) != 0 && !m->action) {
						mstTaskResetMonster1Direction(t);
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
									const uint32_t codeData = mstMonster1GetNextWalkCode(m);
									assert(codeData != kNone);
									resetTask(t, _res->_mstCodeData + codeData * 4);
									t->state &= ~2;
									p = t->codeData - 4;
								}
								break;
							case 5:
								return mstTaskInitMonster1Type1(t);
							case 6:
								return mstTaskInitMonster1Type2(t, 1);
							}
						} else {
// 413DCA
							const uint32_t codeData = mstMonster1GetNextWalkCode(m);
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
						if (m->action) {
							mstMonster1ClearChasingMonster(m);
						}
						m->flagsA5 = (m->flagsA5 & ~0xF) | 6;
						return mstTaskInitMonster1Type2(t, 1);
					}
				} else if ((m->flagsA5 & 8) != 0) {
// 413F8B
					m->flagsA5 &= ~8;
					const uint32_t codeData = m->behaviorState->codeData;
					if (codeData != kNone) {
						resetTask(t, _res->_mstCodeData + codeData * 4);
						return 0;
					} else {
						m->o16->actionKeyMask = 7;
						m->o16->directionKeyMask = 0;
						t->run = &Game::mstTask_idle;
						return 1;
					}
				} else {
					t->run = &Game::mstTask_idle;
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
			warning("Unhandled opcode %d in mstTask_main", *p);
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
			if (_mstActionNum != -1 && (m->flagsA5 & 8) != 0 && m->action != 0) {
				mstMonster1ClearChasingMonster(m);
			}
			const uint8_t *ptr = m->monsterInfos;
			if (ptr[946] & 4) {
				mstBoundingBoxClear(m, 0);
				mstBoundingBoxClear(m, 1);
			}
			mstMonster1ResetData(m);
			removeLvlObject2(m->o16);
			removeTask(tasksList, current);
		} else {
			MonsterObject2 *mo = current->monster2;
			if (mo && mo->o->screenNum == screenNum) {
				mo->monster2Info = 0;
				mo->o->dataPtr = 0;
				removeLvlObject2(mo->o);
				removeTask(tasksList, current);
			}
		}
		current = next;
	}
}

void Game::mstOp27_removeMstTaskScreenType(Task **tasksList, int screenNum, int type) {
	Task *current = *tasksList;
	while (current) {
		MonsterObject1 *m = current->monster1;
		Task *next = current->nextPtr;
		if (m && m->o16->screenNum == screenNum && (m->monsterInfos[944] & 0x7F) == type) {
			if (_mstActionNum != -1 && (m->flagsA5 & 8) != 0 && m->action != 0) {
				mstMonster1ClearChasingMonster(m);
			}
			const uint8_t *ptr = m->monsterInfos;
			if (ptr[946] & 4) {
				mstBoundingBoxClear(m, 0);
				mstBoundingBoxClear(m, 1);
			}
			mstMonster1ResetData(m);
			removeLvlObject2(m->o16);
			removeTask(tasksList, current);
		} else {
			MonsterObject2 *mo = current->monster2;
			if (mo && mo->o->screenNum == screenNum && (mo->monster2Info->type & 0x7F) == type) {
				mo->monster2Info = 0;
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
	const MstOp197Data *op197Data = &_res->_mstOp197Data[num];
	MstMovingBounds *m49 = &_res->_mstMovingBoundsData[op197Data->indexUnk49];
	m->m49 = m49;
	m->indexUnk49Unk1 = op197Data->unkF;
	if (m->indexUnk49Unk1 < 0) {
		if (m49->count2 == 0) {
			m->indexUnk49Unk1 = 0;
		} else {
			m->indexUnk49Unk1 = m49->data2[_rnd.getMstNextNumber(m->rnd_m49)];
		}
	}
	assert((uint32_t)m->indexUnk49Unk1 < m49->count1);
	m->m49Unk1 = &m49->data1[m->indexUnk49Unk1];
	m->goalScreenNum = screen;
	if (a > b) {
		m->goalDistance_x1 = b;
		m->goalDistance_x2 = a;
	} else {
		m->goalDistance_x1 = a;
		m->goalDistance_x2 = b;
	}
	if (c > d) {
		m->goalDistance_y1 = d;
		m->goalDistance_y2 = c;
	} else {
		m->goalDistance_y1 = c;
		m->goalDistance_y2 = d;
	}
	switch (screen + 4) {
	case 1: {
			m->unkE4 = 255;
			const uint8_t _al = m->monsterInfos[946];
			if (_al & 4) {
				t->run = &Game::mstTask_monsterWait10;
			} else if (_al & 2) {
				t->run = &Game::mstTask_monsterWait8;
			} else {
				t->run = &Game::mstTask_monsterWait6;
			}
			if (m->goalDistance_x2 <= 0) {
				m->goalScreenNum = 255;
				if (m->xMstPos < _mstAndyLevelPosX) {
					m->goalDistance_x1 = -m->goalDistance_x1;
					m->goalDistance_x2 = -m->goalDistance_x2;
				}
			}
			if ((_al & 2) != 0 && m->goalDistance_y2 <= 0) {
				m->goalScreenNum = 255;
				if (m->yMstPos < _mstAndyLevelPosY) {
					m->goalDistance_y1 = -m->goalDistance_y2;
					m->goalDistance_y2 = -m->goalDistance_y1;
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
				m->goalDistance_x1 = b;
				m->goalDistance_x2 = a;
			} else if (_al & 2) {
				m->goalDistance_x1 = a;
				m->goalDistance_x2 = b;
			} else {
				m->goalDistance_x1 = 0;
				m->goalDistance_x2 = 0;
			}
			if (_al & 1) {
				c = -c;
				d = -d;
				m->goalDistance_y1 = d;
				m->goalDistance_y2 = c;
			} else if (_al & 4) {
				m->goalDistance_y1 = c;
				m->goalDistance_y2 = d;
			} else {
				m->goalDistance_y1 = 0;
				m->goalDistance_y2 = 0;
			}
		}
		// fall-through
	case 2:
		if (m->monsterInfos[946] & 4) {
			t->run = &Game::mstTask_monsterWait9;
		} else if (m->monsterInfos[946] & 2) {
			t->run = &Game::mstTask_monsterWait7;
		} else {
			t->run = &Game::mstTask_monsterWait5;
		}
		m->goalDistance_x1 += m->xMstPos;
		m->goalDistance_x2 += m->xMstPos;
		m->goalDistance_y1 += m->yMstPos;
		m->goalDistance_y2 += m->yMstPos;
		break;
	case 3:
		if (m->monsterInfos[946] & 4) {
			t->run = &Game::mstTask_monsterWait10;
		} else if (m->monsterInfos[946] & 2) {
			t->run = &Game::mstTask_monsterWait8;
		} else {
			t->run = &Game::mstTask_monsterWait6;
		}
		break;
	default:
		if (m->monsterInfos[946] & 4) {
			t->run = &Game::mstTask_monsterWait9;
		} else if (m->monsterInfos[946] & 2) {
			t->run = &Game::mstTask_monsterWait7;
		} else {
			t->run = &Game::mstTask_monsterWait5;
		}
		m->goalDistance_x1 += _res->_mstPointOffsets[screen].xOffset;
		m->goalDistance_x2 += _res->_mstPointOffsets[screen].xOffset;
		m->goalDistance_y1 += _res->_mstPointOffsets[screen].yOffset;
		m->goalDistance_y2 += _res->_mstPointOffsets[screen].yOffset;
		break;
	}
// 41BD34
	m->goalPos_x1 = m->goalDistance_x1;
	m->goalPos_x2 = m->goalDistance_x2;
	m->goalPos_y1 = m->goalDistance_y1;
	m->flagsA8[2] = 255;
	m->unkC0 = -1;
	m->unkBC = -1;
	m->flagsA7 = 255;
	m->goalPos_y2 = m->goalDistance_y2;
	const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	if ((ptr[2] & kDirectionKeyMaskVertical) == 0) {
		m->goalDistance_y1 = m->yMstPos;
		m->goalPos_y1 = m->yMstPos;
		m->goalDistance_y2 = m->yMstPos;
		m->goalPos_y2 = m->yMstPos;
	}
	if ((ptr[2] & kDirectionKeyMaskHorizontal) == 0) {
		m->goalDistance_x1 = m->xMstPos;
		m->goalPos_x1 = m->xMstPos;
		m->goalDistance_x2 = m->xMstPos;
		m->goalPos_x2 = m->xMstPos;
	}
// 41BDA9
	if (m->monsterInfos[946] & 4) {
		m->flagsA8[3] = 255;
		m->targetLevelPos_x = -1;
		m->targetLevelPos_y = -1;
		mstBoundingBoxClear(m, 1);
	}
// 41BE12
	uint8_t _dl = m->goalScreenNum;
	if (_dl != 0xFC && (m->flagsA5 & 8) != 0 && (t->flags & 0x20) != 0 && m->action) {
		if (t->run != &Game::mstTask_monsterWait6 && t->run != &Game::mstTask_monsterWait8 && t->run != &Game::mstTask_monsterWait10) {
			if ((_dl == 0xFE && m->o16->screenNum != _currentScreen) || (_dl != 0xFE && _dl != _currentScreen)) {
				if (m->monsterInfos[946] & 4) {
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
				_edx = -m->goalDistance_x2;
				_edi = -m->goalDistance_x1;
			} else {
				_edx = m->goalDistance_x1;
				_edi = m->goalDistance_x2;
			}
			uint8_t _bl = m->monsterInfos[946] & 2;
			int _eax, _ecx;
			if (_bl != 0 && _dl == 0xFD && m->yMstPos < _mstAndyLevelPosY) {
				_eax = -m->goalDistance_y2;
				_ecx = -m->goalDistance_y1;
			} else {
				_eax = m->goalDistance_y1;
				_ecx = m->goalDistance_y2;
			}
			if (_edx < a || _edi < b || (_bl != 0 && (_eax < _ebp || _ecx > var4))) {
				if ((m->monsterInfos[946] & 4) != 0) {
					mstBoundingBoxClear(m, 1);
				}
				return mstTaskStopMonsterObject1(t);
			}
		}
	}
// 41C038
	if (m->monsterInfos[946] & 4) {
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
			m->targetLevelPos_x = -1;
			m->targetLevelPos_y = -1;
			mstBoundingBoxClear(m, 1);
			if (p[0xE] != 0) {
				t->flags &= ~0x80;
				mstTaskResetMonster1WalkPath(t);
				return 0;
			}
		} else {
			m->flagsA8[0] = mstBoundingBoxUpdate(0xFF, m->monster1Index, x1, y1, x2, y2);
		}
	}
// 41C17B
	t->flags &= ~0x80;
	if (m->monsterInfos[946] & 2) {
		if (t->run == &Game::mstTask_monsterWait10) {
			mstMonster1UpdateGoalPosition(m);
			mstMonster1MoveTowardsGoal2(m);
		} else if (t->run == &Game::mstTask_monsterWait8) {
			mstMonster1UpdateGoalPosition(m);
			mstMonster1MoveTowardsGoal1(m);
		} else if (t->run == &Game::mstTask_monsterWait9) {
			mstMonster1MoveTowardsGoal2(m);
		} else {
			mstMonster1MoveTowardsGoal1(m);
		}
// 41C281
		uint8_t _cl, _dl;
		if (_mstLut1[m->goalDirectionMask] & 1) {
			_cl = m->m49Unk1->unkE;
			_dl = m->m49Unk1->unkF;
		} else {
			_cl = m->m49Unk1->unkC;
			_dl = m->m49Unk1->unkD;
		}
		if (_xMstPos2 < _cl && _yMstPos2 < _dl && !mstMonster1TestGoalDirection(m)) {
// 41C2E6
			// goto 41C2E6
		} else {
// 41C3FD
			if (m->goalDirectionMask) {
				return (this->*(t->run))(t);
			}
		}
	} else {
// 41C355
		if (t->run == &Game::mstTask_monsterWait6) {
			if (m->goalScreenNum == 0xFD && m->xMstPos < _mstAndyLevelPosX) {
				m->goalPos_x1 = _mstAndyLevelPosX - m->goalDistance_x2;
				m->goalPos_x2 = _mstAndyLevelPosX - m->goalDistance_x1;
			} else {
				m->goalPos_x1 = _mstAndyLevelPosX + m->goalDistance_x1;
				m->goalPos_x2 = _mstAndyLevelPosX + m->goalDistance_x2;
			}
		}
// 41C399
		mstMonster1SetGoalHorizontal(m);
		bool flag = false;
		while (_xMstPos2 < m->m49Unk1->unkC) {
			if (--m->indexUnk49Unk1 >= 0) {
				m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
			} else {
				flag = true;
				break;
				// goto 41C2E6
			}
		}
		if (!flag) {
// 41C3FD
			if (m->goalDirectionMask) {
				return (this->*(t->run))(t);
			}
		}
	}
// 41C2E6
	if (m->monsterInfos[946] & 4) {
		mstBoundingBoxClear(m, 1);
	}
// 41C33C
	t->flags |= 0x80;
	mstTaskResetMonster1WalkPath(t);
	return 0;
}

void Game::mstOp52() {
	if (_mstActionNum == -1) {
		return;
	}
	MstMonsterAction *m48 = &_res->_mstMonsterActionData[_mstActionNum];
	int j = 0;
	for (int i = 0; i < m48->areaCount; ++i) {
		MstMonsterArea *m48Area = &m48->area[j];
		const uint8_t num = m48Area->data->monster1Index;
		if (num != 255) {
			assert(num < kMaxMonsterObjects1);
			MonsterObject1 *m = &_monsterObjects1Table[num];
			mstMonster1ClearChasingMonster(m);
			if ((m->flagsA5 & 0x70) == 0) {
				assert(m->task->monster1 == m);
				Task *t = m->task;
				const int a = (m->o16->flags0 & 255) * 28;
				if (m->monsterInfos[a] != 0) {
					if (t->run != &Game::mstTask_monsterWait1 && t->run != &Game::mstTask_monsterWait4 && t->run != &Game::mstTask_monsterWait2 && t->run != &Game::mstTask_monsterWait3 && t->run != &Game::mstTask_monsterWait5 && t->run != &Game::mstTask_monsterWait6 && t->run != &Game::mstTask_monsterWait7 && t->run != &Game::mstTask_monsterWait8 && t->run != &Game::mstTask_monsterWait9 && t->run != &Game::mstTask_monsterWait10) {
						m->flagsA5 = (m->flagsA5 & ~0xF) | 6;
						mstTaskInitMonster1Type2(m->task, 1);
					} else {
						m->o16->actionKeyMask = 0;
						m->o16->directionKeyMask = 0;
						t->run = &Game::mstTask_monsterWait11;
					}
				} else {
// 41D7D8
					m->flagsA5 = (m->flagsA5 & ~0xF) | 6;
					mstTaskInitMonster1Type2(m->task, 1);
				}
			}
		}
		++j;
	}
	_mstActionNum = -1;
}

bool Game::mstHasMonsterInRange(const MstMonsterAction *m48, uint8_t flag) {
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
	for (int i = 0; i < m48->areaCount; ++i) {
		const MstMonsterArea *m12 = &m48->area[i];
		assert(m12->count == 1);
		MstMonsterAreaAction *m12u4 = m12->data;
		if (m12->unk0 != 0) {
			uint8_t var1C = m12u4->unk18;
			if (var1C != 2) {
				_edi = var1C;
			}
// 41DB81
l1:
			int var4C = _edi;

			int var8 = m12u4->xPos;
			int _ebx = var8; // xPos
			int var4 = m12u4->yPos;
			int _esi = var4; // yPos

			int _eax = _edi ^ flag;
			if (_eax == 1) {
				_ebx = -_ebx;
			}
			debug(kDebug_MONSTER, "mstHasMonsterInRange (unk0!=0) count:%d %d %d [%d,%d] screen:%d", m12->count, _ebx, _esi, _mstPosXmin, _mstPosXmax, m12u4->screenNum);
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
							if ((var8 || var4) && m->monsterInfos[944] != 10 && m->monsterInfos[944] != 16 && m->monsterInfos[944] != 9) {
								if (_esi <= 0) {
									if (m->levelPosBounds_x1 > _ebx) { // var44
										continue;
									}
								} else {
									if (m->levelPosBounds_x2 < _ebx) { // var44
										continue;
									}
								}
								if (var4D != 0) { // vertical move
									if (_ebp <= 0) {
										if (m->levelPosBounds_y1 > var38) {
											continue;
										}
									} else {
										if (m->levelPosBounds_y2 < var38) {
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
						debug(kDebug_MONSTER, "monster %d in range", num);
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
	for (int i = _edi; i < m48->areaCount; ++i) {
		MstMonsterArea *m12 = &m48->area[i]; // var20
		assert(m12->count == 1);
		MstMonsterAreaAction *m12u4 = m12->data;
		if (m12->unk0 == 0) {
			uint8_t var1C = m12u4->unk18;
			m12u4->monster1Index = 255;
			int var4C = (var1C == 2) ? 0 : var1C;
			int _edx = var4C;
// 41DE98
l2:
			int var4 = m12u4->xPos;
			int _ebx = var4;
			int var8 = m12u4->yPos;
			int _esi = var8;
			int _eax = _edx ^ flag;
			if (_eax == 1) {
				_ebx = -_ebx;
			}
			debug(kDebug_MONSTER, "mstHasMonsterInRange (unk0==0) count:%d %d %d [%d,%d] screen:%d", m12->count, _ebx, _esi, _mstPosXmin, _mstPosXmax, m12u4->screenNum);
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
							if ((var8 || var4) && m->monsterInfos[944] != 10 && m->monsterInfos[944] != 16 && m->monsterInfos[944] != 9) {
								if (_esi <= 0) {
									if (m->levelPosBounds_x1 > _ebx) { // var44
										continue;
									}
								} else {
									if (m->levelPosBounds_x2 < _ebx) { // var44
										continue;
									}
								}
								if (var4D != 0) { // vertical move
									if (_ebp <= 0) {
										if (m->levelPosBounds_y1 > var38) {
											continue;
										}
									} else {
										if (m->levelPosBounds_y2 < var38) {
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
						debug(kDebug_MONSTER, "monster %d in range", num);
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

void Game::mstOp53(MstMonsterAction *m) {
	if (_mstActionNum != -1) {
		return;
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
	mstUpdateInRange(m);
}

void Game::mstOp54() {
	debug(kDebug_MONSTER, "mstOp54 %d %d %d", _mstActionNum, _m43Num2, _m43Num3);
	if (_mstActionNum != -1) {
		return;
	}
	MstMonsterActionIndex *m43 = 0;
	if (_mstFlags & 0x20000000) {
		if (_m43Num2 == -1) {
			return;
		}
		m43 = &_res->_mstMonsterActionIndexData[_m43Num2];
	} else {
		if (_m43Num3 == -1) {
			return;
		}
		m43 = &_res->_mstMonsterActionIndexData[_m43Num3];
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
	if (m43->dataCount == 0) {
		const uint32_t indexUnk48 = m43->indexUnk48[0];
		MstMonsterAction *m48 = &_res->_mstMonsterActionData[indexUnk48];
		mstUpdateInRange(m48);
// 41E36E
		if (_mstActionNum == -1) {
			++_mstOp54Counter;
		}
		if (_mstOp54Counter <= 16) {
			return;
		}
		_mstOp54Counter = 0;
		shuffleMstMonsterActionIndex(m43);
	} else {
// 41E3CA
		memset(_mstOp54Table, 0, sizeof(_mstOp54Table));
		bool var4 = false;
		uint32_t i = 0;
		for (; i < m43->dataCount; ++i) {
			uint8_t num = m43->data[i];
			if ((num & 0x80) == 0) {
				var4 = true;
				if (_mstOp54Table[num] == 0) {
					_mstOp54Table[num] = 1;
					const uint32_t indexUnk48 = m43->indexUnk48[num];
					MstMonsterAction *m48 = &_res->_mstMonsterActionData[indexUnk48];
					if (mstUpdateInRange(m48)) {
						break; // goto 41E494;
					}
				}
			}
		}
// 41E494
		if (_mstActionNum != -1) {
			assert(i < m43->dataCount);
			m43->data[i] |= 0x80;
		} else {
// 41E4AC
			if (var4) {
				++_mstOp54Counter;
				if (_mstOp54Counter <= 16) {
					return;
				}
			}
			_mstOp54Counter = 0;
			if (m43->dataCount != 0) {
				shuffleMstMonsterActionIndex(m43);
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
	assert(num < _res->_mstHdr.op204DataCount);
	const MstOp204Data *op204Data = &_res->_mstOp204Data[num];
	debug(kDebug_MONSTER, "mstOp56_specialAction code %d", code);
	switch (code) {
	case 0:
		if (!_specialAnimFlag && setAndySpecialAnimation(0x71) != 0) {
			_plasmaCannonFlags |= 1;
			if (_andyObject->spriteNum == 0) {
				_mstCurrentAnim = op204Data->arg0 & 0xFFFF;
			} else {
				_mstCurrentAnim = op204Data->arg0 >> 16;
			}
// 411AB4
			LvlObject *o = 0;
			if (t->monster2) {
				o = t->monster2->o;
			} else if (t->monster1) {
				o = t->monster1->o16;
			}
			if (op204Data->arg3 != 6 && o) {
				LvlObject *tmpObject = t->monster1->o16;
				const uint8_t flags = getLvlObjectFlag(op204Data->arg3 & 255, tmpObject, _andyObject);
				_specialAnimMask = ((flags & 3) << 4) | (_specialAnimMask & ~0x30);
				// _specialAnimScreenNum = tmpObject->screenNum;
				_specialAnimLvlObject = tmpObject;
				_mstOriginPosX = op204Data->arg1 & 0xFFFF;
				_mstOriginPosY = op204Data->arg2 & 0xFFFF;
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
				_mstCurrentAnim = op204Data->arg0 & 0xFFFF;
			} else {
				_mstCurrentAnim = op204Data->arg0 >> 16;
			}
// 4118ED
			LvlObject *o = 0;
			if (t->monster2) {
				o = t->monster2->o;
			} else if (t->monster1) {
				o = t->monster1->o16;
			}
			if (op204Data->arg3 != 6 && o) {
				LvlObject *tmpObject = t->monster1->o16;
				const uint8_t flags = getLvlObjectFlag(op204Data->arg3 & 255, tmpObject, _andyObject);
				_specialAnimMask = ((flags & 3) << 4) | (_specialAnimMask & 0xFFCF);
				// _specialAnimScreenNum = tmpObject->screenNum;
				_specialAnimLvlObject = tmpObject;
				_mstOriginPosX = op204Data->arg1 & 0xFFFF;
				_mstOriginPosY = op204Data->arg2 & 0xFFFF;
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
			const uint8_t flags = getLvlObjectFlag(op204Data->arg0 & 255, o, _andyObject);
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
		if (op204Data->arg0 == 1) {
			setShakeScreen(2, op204Data->arg1 & 255);
		} else if (op204Data->arg0 == 2) {
			setShakeScreen(1, op204Data->arg1 & 255);
		} else {
			setShakeScreen(3, op204Data->arg1 & 255);
		}
		break;
	case 11: {
			MonsterObject2 *m = t->monster2;
			const int type = op204Data->arg3;
			m->x1 = getTaskVar(t, op204Data->arg0, (type >> 0xC) & 15);
			m->y1 = getTaskVar(t, op204Data->arg1, (type >> 0x8) & 15);
			m->x2 = getTaskVar(t, op204Data->arg2, (type >> 0x4) & 15);
			m->y2 = getTaskVar(t, type >> 16     ,  type         & 15);
		}
		break;
	case 12: {
			const int type1 = ((op204Data->arg3 >> 4) & 15);
			const int hint  = getTaskVar(t, op204Data->arg0, type1);
			const int type2 = (op204Data->arg3 & 15);
			const int pause = getTaskVar(t, op204Data->arg1, type2);
			displayHintScreen(hint, pause);
		}
		break;
	case 13:
	case 14:
	case 22:
	case 23:
	case 24:
	case 25: {
			const int mask = op204Data->arg3;
			int xPos = getTaskVar(t, op204Data->arg0, (mask >> 8) & 15);
			int yPos = getTaskVar(t, op204Data->arg1, (mask >> 4) & 15);
			int screenNum = getTaskVar(t, op204Data->arg2, mask & 15);
			LvlObject *o = 0;
			if (t->monster2) {
				o = t->monster2->o;
			} else if (t->monster1) {
				o = t->monster1->o16;
			}
			if (screenNum < 0) {
				if (screenNum == -2) {
					if (!o) {
						break;
					}
					screenNum = o->screenNum;
					if (t->monster2) {
						xPos += t->monster2->xMstPos;
						yPos += t->monster2->yMstPos;
					} else if (t->monster1) {
						xPos += t->monster1->xMstPos;
						yPos += t->monster1->yMstPos;
					} else {
						break;
					}
				} else if (screenNum == -1) {
					xPos += _mstAndyLevelPosX;
					yPos += _mstAndyLevelPosY;
					screenNum = _currentScreen;
				} else {
					if (!o) {
						break;
					}
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
				if (screenNum >= _res->_mstHdr.screensCount) {
					screenNum = _res->_mstHdr.screensCount - 1;
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
					mstTaskSetMonster2ScreenPosition(t);
				} else {
					assert(t->monster1);
					mstTaskUpdateScreenPosition(t);
				}
			} else if (code == 14) {
				const int pos = mask >> 16;
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
				mstUpdateMonstersRect();
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
			_andyObject->anim  = op204Data->arg0;
			_andyObject->frame = op204Data->arg1;
			LvlObject *o = 0;
			if (t->monster2) {
				o = t->monster2->o;
			} else if (t->monster1) {
				o = t->monster1->o16;
			} else {
				o = _andyObject;
			}
			const uint8_t flags = getLvlObjectFlag(op204Data->arg2 & 255, o, _andyObject);
			_andyObject->flags1 = ((flags & 3) << 4) | (_andyObject->flags1 & 0xFFCF);
			const int x3 = _andyObject->posTable[3].x;
			const int y3 = _andyObject->posTable[3].y;
			setupLvlObjectBitmap(_andyObject);
			_andyObject->xPos += (x3 - _andyObject->posTable[3].x);
			_andyObject->yPos += (y3 - _andyObject->posTable[3].y);
			updateLvlObjectScreen(o);
			mstUpdateRefPos();
			mstUpdateMonstersRect();
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
			const int pos = op204Data->arg2;
			assert(pos < 8);
			const int xPos = o->xPos + o->posTable[pos].x;
			const int yPos = o->yPos + o->posTable[pos].y;
			const int type1  = (op204Data->arg3 >> 4) & 15;
			const int index1 = op204Data->arg0;
			setTaskVar(t, index1, type1, xPos);
			const int type2  = op204Data->arg3 & 15;
			const int index2 = op204Data->arg1;
			setTaskVar(t, index2, type2, yPos);
		}
		break;
	case 18: {
			_mstCurrentActionKeyMask = op204Data->arg0 & 255;
		}
		break;
	case 19: {
			_andyActionKeyMaskAnd    = op204Data->arg0 & 255;
			_andyActionKeyMaskOr     = op204Data->arg1 & 255;
			_andyDirectionKeyMaskAnd = op204Data->arg2 & 255;
			_andyDirectionKeyMaskOr  = op204Data->arg3 & 255;
		}
		break;
	case 20: {
			_mstCurrentActionKeyMask = 0;
			t->monster1->flagsA6 |= 2;
			t->run = &Game::mstTask_idle;
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
			int screenNum = op204Data->arg2;
			if (screenNum < -1 && !t->monster1) {
				break;
			}
			if (screenNum == -1) {
				screenNum = _currentScreen;
			} else if (screenNum < -1) {
				screenNum = t->monster1->o16->screenNum;
			}
			if (screenNum >= _res->_mstHdr.screensCount) {
				screenNum = _res->_mstHdr.screensCount - 1;
			}
			int _ebp = _res->_mstPointOffsets[screenNum].xOffset;
			int _edx = _res->_mstPointOffsets[screenNum].yOffset;
			int _eax = op204Data->arg3 * 256;
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
				switch (op204Data->arg0) {
				case 0:
					_eax = op204Data->arg1;
					if (m->m46 == &_res->_mstBehaviorData[_eax]) {
						++count;
					}
					break;
				case 1:
					_eax = op204Data->arg1;
					if (m->monsterInfos == &_res->_mstMonsterInfos[_eax * kMonsterInfoDataSize]) {
						++count;
					}
					break;
				case 2:
					_eax = op204Data->arg1;
					if (m->monsterInfos[944] == _eax) {
						++count;
					}
					break;
				}
			}
			_mstOp56Counter = count;
		}
		break;
	case 27: {
			const int type = op204Data->arg3;
			int a = getTaskVar(t, op204Data->arg0, (type >> 0xC) & 15);
			int b = getTaskVar(t, op204Data->arg1, (type >> 0x8) & 15);
			int c = getTaskVar(t, op204Data->arg2, (type >> 0x4) & 15);
			int d = getTaskVar(t, type >> 16,       type         & 15);
			setScreenMaskRect(a - 16, b, a + 16, c, d);
		}
		break;
	case 28:
		// no-op
		break;
	case 29: {
			const uint8_t state  = op204Data->arg1 & 255;
			const uint8_t screen = op204Data->arg0 & 255;
			_res->_screensState[screen].s0 = state;
		}
		break;
	case 30:
		++_level->_checkpoint;
		break;
	default:
		warning("Unhandled opcode %d in mstOp56_specialAction", code);
		break;
	}
	return 0;
}

static void initWormHoleSprite(WormHoleSprite *s, const uint8_t *p) {
	s->screenNum = p[0];
	s->initData1 = p[1];
	s->xPos = p[2];
	s->yPos = p[3];
	s->initData4 = READ_LE_UINT32(p + 4);
	s->rect1_x1 = p[8];
	s->rect1_y1 = p[9];
	s->rect1_x2 = p[0xA];
	s->rect1_y2 = p[0xB];
	s->rect2_x1 = p[0xC];
	s->rect2_y1 = p[0xD];
	s->rect2_x2 = p[0xE];
	s->rect2_y2 = p[0xF];
}

void Game::mstOp57_addWormHoleSprite(int x, int y, int screenNum) {
	bool found = false;
	int spriteNum = 0;
	for (int i = 0; i < 6; ++i) {
		if (_wormHoleSpritesTable[i].screenNum == screenNum) {
			found = true;
			break;
		}
		if (_wormHoleSpritesTable[i].screenNum == 0xFF) {
			break;
		}
		++spriteNum;
	}
	if (!found) {
		found = true;
		if (spriteNum == 6) {
			++_wormHoleSpritesCount;
			if (_wormHoleSpritesCount >= spriteNum) {
				_wormHoleSpritesCount = 0;
				spriteNum = 0;
			} else {
				spriteNum = _wormHoleSpritesCount - 1;
			}
		} else {
			spriteNum = _wormHoleSpritesCount;
		}
// 40347C
		switch (_currentLevel) {
		case 2:
			initWormHoleSprite(&_wormHoleSpritesTable[spriteNum], _pwr1_spritesData + screenNum * 16);
			break;
		case 3:
			initWormHoleSprite(&_wormHoleSpritesTable[spriteNum], _isld_spritesData + screenNum * 16);
			break;
		case 4:
			initWormHoleSprite(&_wormHoleSpritesTable[spriteNum], _lava_spritesData + screenNum * 16);
			break;
		case 6:
			initWormHoleSprite(&_wormHoleSpritesTable[spriteNum], _lar1_spritesData + screenNum * 16);
			break;
		default:
			warning("mstOp57 unhandled level %d", _currentLevel);
			return;
		}
	}
// 4034D2
	WormHoleSprite *boulderWormSprite = &_wormHoleSpritesTable[spriteNum];
	const int dx = x - boulderWormSprite->xPos;
	const int dy = y + 15 - boulderWormSprite->yPos;
	spriteNum = _rnd._rndSeed & 3;
	if (spriteNum == 0) {
		spriteNum = 1;
	}
	static const uint8_t data[32] = {
		0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x06, 0x06, 0x06, 0x08, 0x08, 0x08, 0x0A,
		0x0A, 0x0A, 0x0C, 0x0C, 0x0C, 0x0E, 0x0E, 0x0E, 0x10, 0x10, 0x10, 0x12, 0x12, 0x12, 0x14, 0x14,
	};
// 403515
	const int pos = data[(dx >> 3) & 31];
	const int num = dy >> 5;
	if ((boulderWormSprite->flags[num] & (3 << pos)) == 0) {
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
		boulderWormSprite->flags[num] |= (spriteNum << pos);
	}
}

void Game::mstOp58_addLvlObject(Task *t, int num) {
	const MstOp211Data *dat = &_res->_mstOp211Data[num];
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

void Game::mstOp59_addShootSpecialPowers(int x, int y, int screenNum, int type, uint16_t flags) {
	LvlObject *o = addLvlObjectToList0(3);
	if (o) {
		o->dataPtr = _shootLvlObjectDataNextPtr;
		if (_shootLvlObjectDataNextPtr) {
			_shootLvlObjectDataNextPtr = _shootLvlObjectDataNextPtr->nextPtr;
			memset(o->dataPtr, 0, sizeof(ShootLvlObjectData));
		}
		ShootLvlObjectData *s = (ShootLvlObjectData *)o->dataPtr;
		assert(s);
		o->callbackFuncPtr = &Game::lvlObjectSpecialPowersCallback;
		s->state = type;
		s->unk0 = 0;
		s->counter = 17;
		s->dxPos = (int8_t)_specialPowersDxDyTable[type * 2];
		s->dyPos = (int8_t)_specialPowersDxDyTable[type * 2 + 1];
// 43E730
		static const uint8_t data[16] = {
			0x0D, 0x00, 0x0C, 0x01, 0x0C, 0x03, 0x0C, 0x00, 0x0C, 0x02, 0x0D, 0x01, 0x0B, 0x00, 0x0B, 0x02,
		};
		o->anim = data[type * 2];
		o->flags1 = ((data[type * 2 + 1] & 3) << 4) | (o->flags1 & ~0x0030);
		o->frame = 0;
		o->flags2 = o->flags1;
		o->screenNum = screenNum;
		setupLvlObjectBitmap(o);
		setLvlObjectPosRelativeToPoint(o, 6, x, y);
	}
}

void Game::mstOp59_addShootFireball(int x, int y, int screenNum, int type, int state, uint16_t flags) {
	LvlObject *o = addLvlObjectToList2(7);
	if (o) {
		o->dataPtr = _shootLvlObjectDataNextPtr;
		if (_shootLvlObjectDataNextPtr) {
			_shootLvlObjectDataNextPtr = _shootLvlObjectDataNextPtr->nextPtr;
			memset(o->dataPtr, 0, sizeof(ShootLvlObjectData));
		}
		ShootLvlObjectData *s = (ShootLvlObjectData *)o->dataPtr;
		assert(s);
		s->state = state;
// dx, dy
		static const uint8_t _byte_43E6E0[16] = {
			0x0A, 0x00, 0xF7, 0xFA, 0xF7, 0x06, 0x09, 0xFA, 0x09, 0x06, 0xF6, 0x00, 0x00, 0xF6, 0x00, 0x0A
		};
		static const uint8_t _byte_43E6F0[16] = {
			0x0D, 0x00, 0xF5, 0xF9, 0xF5, 0x07, 0x0B, 0xF9, 0x0B, 0x07, 0xF3, 0x00, 0x00, 0xF3, 0x00, 0x0D
		};
// 43E740
		static const uint8_t data1[16] = {
			0x02, 0x00, 0x01, 0x01, 0x01, 0x03, 0x01, 0x00, 0x01, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x02
		};
// 43E760
		static const uint8_t data2[16] = {
			0x0D, 0x00, 0xF5, 0xF9, 0xF5, 0x07, 0x0B, 0xF9, 0x0B, 0x07, 0xF3, 0x00, 0x00, 0xF3, 0x00, 0x0D
		};
		const uint8_t *_ecx;
		if (type >= 7) {
			s->dxPos = (int8_t)_byte_43E6F0[state * 2];
			s->dyPos = (int8_t)_byte_43E6F0[state * 2 + 1];
			s->counter = 33;
			_ecx = &data2[state * 2];
		} else {
			s->dxPos = (int8_t)_byte_43E6E0[state * 2];
			s->dyPos = (int8_t)_byte_43E6E0[state * 2 + 1];
			s->counter = 39;
			_ecx = &data1[state * 2];
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

void Game::mstTaskResetMonster1WalkPath(Task *t) {
	MonsterObject1 *m = t->monster1;
	t->run = &Game::mstTask_main;
	m->o16->actionKeyMask = 0;
	m->o16->directionKeyMask = 0;
	if ((m->flagsA5 & 4) != 0 && (m->flagsA5 & 0x28) == 0) {
		switch (m->flagsA5 & 7) {
		case 5:
			m->flagsA5 = (m->flagsA5 & ~6) | 1;
			if (!mstMonster1UpdateWalkPath(m)) {
				mstMonster1ResetWalkPath(m);
			}
			mstTaskSetNextWalkCode(t);
			break;
		case 6:
			m->flagsA5 &= ~7;
			if (mstSetCurrentPos(m, m->xMstPos, m->yMstPos) == 0) {
				m->flagsA5 |= 1;
				if (!mstMonster1UpdateWalkPath(m)) {
					mstMonster1ResetWalkPath(m);
				}
				const uint32_t indexWalkCode = m->walkNode->walkCodeStage1;
				if (indexWalkCode != kNone) {
					m->walkCode = &_res->_mstWalkCodeData[indexWalkCode];
				}
				mstTaskSetNextWalkCode(t);
			} else {
				m->flagsA5 |= 2;
				if (!mstMonster1UpdateWalkPath(m)) {
					mstMonster1ResetWalkPath(m);
				}
				mstTaskSetNextWalkCode(t);
			}
			break;
		}
	} else {
		m->flagsA5 &= ~4;
		mstMonster1UpdateWalkPath(m);
	}
}

bool Game::mstSetCurrentPos(MonsterObject1 *m, int x, int y) {
	_mstCurrentPosX = x; // _esi
	_mstCurrentPosY = y;
	const uint8_t *ptr = m->monsterInfos;
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

void Game::mstMonster1SetGoalHorizontal(MonsterObject1 *m) {
	Task *t = m->task;
	t->flags &= ~0x80;
	int x = m->xMstPos;
	if (x < m->goalPos_x1) {
		_xMstPos1 = x = m->goalPos_x2;
		if ((m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0 && x > m->unk84) {
			t->flags |= 0x80;
			x = m->unk84;
		}
		if (x > m->levelPosBounds_x2) {
			t->flags |= 0x80;
			x = m->levelPosBounds_x2;
		}
		_xMstPos2 = x - m->xMstPos;
		m->goalDirectionMask = kDirectionKeyMaskRight;
	} else if (x > m->goalPos_x2) {
// 41A2AA
		_xMstPos1 = x = m->goalPos_x1;
		if ((m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0 && x < m->unk88) {
			t->flags |= 0x80;
			x = m->unk88;
		}
		if (x < m->levelPosBounds_x1) {
			t->flags |= 0x80;
			x = m->levelPosBounds_x1;
		}
		_xMstPos2 = m->xMstPos - x;
		m->goalDirectionMask = kDirectionKeyMaskLeft;
	} else {
// 41A2FC
		_xMstPos1 = x;
		_xMstPos2 = 0;
		m->goalDirectionMask = 0;
	}
}

void Game::mstResetCollisionTable() {
	const int count = MIN(_res->_mstHdr.infoMonster1Count, 32);
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
		const uint8_t _bl = m->flagsA5;
		if ((_bl & 2) != 0 || ((_bl & 1) != 0 && ((m->walkNode->walkCodeStage1 == kNone && !m->walkCode) || (m->walkNode->walkCodeStage1 != kNone && m->walkCode == &_res->_mstWalkCodeData[m->walkNode->walkCodeStage1])))) {
			if ((_bl & 0xB0) != 0) {
				continue;
			}
			const uint8_t *p = m->monsterInfos + (m->o16->flags0 & 255) * 28;
			if (p[0] != 0) {
				continue;
			}
			if (m->task->run == &Game::mstTask_monsterWait4) {
				continue;
			}
			uint8_t _al = m->flagsA6;
			if (_al & 2) {
				continue;
			}
			assert(m->task->monster1 == m);
			if ((_al & 8) == 0 || m->monsterInfos[945] != 0) {
				const uint32_t offset = m->monsterInfos - _res->_mstMonsterInfos;
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

void Game::mstTaskRestart(Task *t) {
	t->run = &Game::mstTask_main;
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

bool Game::mstMonster1CheckLevelBounds(MonsterObject1 *m, int x, int y, uint8_t dir) {
	if ((m->flagsA5 & 2) != 0 && (m->flags48 & 8) != 0 && !mstSetCurrentPos(m, x, y)) {
		return true;
	}
	if ((dir & 8) != 0 && x < m->levelPosBounds_x1) {
		return true;
	}
	if ((dir & 2) != 0 && x > m->levelPosBounds_x2) {
		return true;
	}
	if ((dir & 1) != 0 && y < m->levelPosBounds_y1) {
		return true;
	}
	if ((dir & 4) != 0 && y > m->levelPosBounds_y2) {
		return true;
	}
	return false;
}

int Game::mstTaskResetMonster1Direction(Task *t) {
	MonsterObject1 *m = t->monster1;
	m->flagsA5 = (m->flagsA5 & ~0xF) | 6;
	return mstTaskInitMonster1Type2(t, 1);
}

int Game::mstTaskInitMonster1Type1(Task *t) {
	t->flags &= ~0x80;
	MonsterObject1 *m = t->monster1;
	m->flagsA5 = (m->flagsA5 & ~2) | 5;
	mstMonster1ResetWalkPath(m);
	const uint32_t indexWalkBox = m->walkNode->walkBox;
	const MstWalkBox *m34 = &_res->_mstWalkBoxData[indexWalkBox];
	int y = 0;
	int x = 0;
	bool flag = false;
	if (m->monsterInfos[946] & 2) {
		m->unkC0 = -1;
		m->unkBC = -1;
		m->flagsA8[2] = 255;
		m->flagsA7 = 255;
		y = m34->top;
		if (m->yMstPos < m34->top || m->yMstPos > m34->bottom) {
			flag = true;
		}
	}
// 41C4B6
	x = m34->left;
	if (m->monsterInfos[946] & 4) {
// 41C4C9
		m->flagsA8[3] = 255;
		m->targetLevelPos_x = -1;
		m->targetLevelPos_y = -1;
		mstBoundingBoxClear(m, 1);
	}
// 41C531
	if (!flag && m->xMstPos >= x && m->xMstPos <= m34->right) {
		mstTaskResetMonster1WalkPath(t);
		return 0;
	}
// 41C559
	const uint32_t indexUnk36 = m->walkNode->movingBoundsIndex1;
	assert(indexUnk36 != kNone);
	MstMovingBoundsIndex *m36 = &_res->_mstMovingBoundsIndexData[indexUnk36];
	MstMovingBounds *m49 = &_res->_mstMovingBoundsData[m36->indexUnk49];
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
	int _edi = (m34->right - x) / 4;
	m->goalDistance_x1 = x + _edi;
	m->goalDistance_x2 = m34->right - _edi;
	if (_edi != 0) {
		_edi = _rnd.update() % _edi;
	}
	m->goalDistance_x2 -= _edi;
	m->goalDistance_x1 += _edi;
	m->goalPos_x1 = m->goalDistance_x1;
	m->goalPos_x2 = m->goalDistance_x2;
	const uint8_t *ptr1 = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	if ((ptr1[2] & kDirectionKeyMaskHorizontal) == 0) {
		m->goalDistance_x1 = m->goalPos_x1 = m->goalDistance_x2 = m->goalPos_x2 = m->xMstPos;
	}
// 41C62E
	if (m->monsterInfos[946] & 2) {
		int _edi = (m34->bottom - y) / 4;
		m->goalDistance_y2 = m34->bottom - _edi;
		m->goalDistance_y1 = y + _edi;
		if (_edi != 0) {
			_edi = _rnd.update() % _edi;
		}
		m->goalDistance_y1 += _edi;
		m->goalPos_y1 = m->goalDistance_y1;
		m->goalDistance_y2 -= _edi;
		m->goalPos_y2 = m->goalDistance_y2;
		const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
		if ((ptr[2] & kDirectionKeyMaskVertical) == 0) {
			m->goalDistance_y1 = m->goalPos_y1 = m->goalDistance_y2 = m->goalPos_y2 = m->yMstPos;
		}
		if (m->monsterInfos[946] & 4) {
			mstMonster1MoveTowardsGoal2(m);
		} else {
			mstMonster1MoveTowardsGoal1(m);
		}
		_edi = 1;
		int _cl, _dl;
		if (_mstLut1[m->goalDirectionMask] & 1) {
			_cl = m->m49Unk1->unkE;
			_dl = m->m49Unk1->unkF;
		} else {
			_cl = m->m49Unk1->unkC;
			_dl = m->m49Unk1->unkD;
		}
		if (_xMstPos2 < _cl && _yMstPos2 < _dl) {
			if (!mstMonster1TestGoalDirection(m)) {
				_edi = 0;
			}
		}
	} else {
// 41C712
		mstMonster1SetGoalHorizontal(m);
		_edi = 1;
		while (_xMstPos2 < m->m49Unk1->unkC) {
			if (--m->indexUnk49Unk1 >= 0) {
				m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
			} else {
				_edi = 0;
				break;
			}
		}
	}
// 41C774
	if (_xMstPos2 <= 0 && ((m->monsterInfos[946] & 2) == 0 || _yMstPos2 <= 0)) {
		if (m->monsterInfos[946] & 4) {
			mstBoundingBoxClear(m, 1);
		}
		mstTaskResetMonster1WalkPath(t);
		return 0;
	}
// 41C7F6
	if (_edi != 0 && ((_xMstPos2 >= m->m49->unk14 || ((m->monsterInfos[946] & 2) != 0 && _yMstPos2 >= m->m49->unk15)))) {
// 41C833
		const uint8_t *p = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
		if ((m->monsterInfos[946] & 4) != 0 && p[0xE] != 0 && m->flagsA8[0] == 255) {
			const int x1 = m->xMstPos + (int8_t)p[0xC];
			const int x2 = x1 + p[0xE] - 1;
			const int y1 = m->yMstPos + (int8_t)p[0xD];
			const int y2 = y1 + p[0xF] - 1;
			if (mstBoundingBoxCollides2(m->monster1Index, x1, y1, x2, y2) > 1) {
// 41CA54
				m->indexUnk49Unk1 = 0;
				m->m49Unk1 = &m->m49->data1[0];
				m->flagsA8[3] = 255;
				m->targetLevelPos_x = -1;
				m->targetLevelPos_y = -1;
				mstBoundingBoxClear(m, 1);
				if (p[0xE] != 0) {
					t->flags &= ~0x80;
					mstTaskResetMonster1WalkPath(t);
					return 0;
				}
			} else {
				m->flagsA8[0] = mstBoundingBoxUpdate(0xFF, m->monster1Index, x1, y1, x2, y2);
			}
		}
// 41C976
		if (_xMstPos2 >= m36->unk8 || ((m->monsterInfos[946] & 2) != 0 && _yMstPos2 >= m36->unk8)) {
			m->indexUnk49Unk1 = m->m49->count1 - 1;
			m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
			if (m->monsterInfos[946] & 4) {
				m->flagsA8[3] = 255;
				m->targetLevelPos_x = -1;
				m->targetLevelPos_y = -1;
				mstBoundingBoxClear(m, 1);
			}
		}
// 41CA2D
		if (m->monsterInfos[946] & 4) {
			t->run = &Game::mstTask_monsterWait9;
		} else if (m->monsterInfos[946] & 2) {
			t->run = &Game::mstTask_monsterWait7;
		} else {
			t->run = &Game::mstTask_monsterWait5;
		}
		return (this->*(t->run))(t);
	} else if (m->monsterInfos[946] & 4) {
// 41CB2F
		mstBoundingBoxClear(m, 1);
	}
// 41CB85
	t->flags |= 0x80;
	mstTaskResetMonster1WalkPath(t);
	return -1;
}

int Game::mstTaskInitMonster1Type2(Task *t, int flag) {
	debug(kDebug_MONSTER, "mstTaskInitMonster1Type2 t %p flag %d", t, flag);
	t->flags &= ~0x80;
	MonsterObject1 *m = t->monster1;
	m->flagsA5 = (m->flagsA5 & ~1) | 6;
	mstMonster1ResetWalkPath(m);
	const uint32_t indexUnk36 = m->walkNode->movingBoundsIndex2;
	MstMovingBoundsIndex *m36 = &_res->_mstMovingBoundsIndexData[indexUnk36];
	MstMovingBounds *m49 = &_res->_mstMovingBoundsData[m36->indexUnk49];
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
	m->goalScreenNum = 0xFD;
	m->unkC0 = -1;
	m->unkBC = -1;
	m->flagsA8[2] = 255;
	m->flagsA7 = 255;
	if (mstSetCurrentPos(m, m->xMstPos, m->yMstPos)) {
		mstTaskResetMonster1WalkPath(t);
		return 0;
	}
// 41CCA3
	int _edi;
	const uint8_t *p = m->monsterInfos;
	if (p[946] & 2) {
		m->unkE4 = 255;
		if (p[946] & 4) {
			m->flagsA8[3] = 255;
			m->targetLevelPos_x = -1;
			m->targetLevelPos_y = -1;
			mstBoundingBoxClear(m, 1);
		}
// 41CD22
		m->goalDistance_x1 = READ_LE_UINT32(m->monsterInfos + 900);
		m->goalDistance_x2 = m->goalDistance_x1 + READ_LE_UINT32(m->monsterInfos + 896);
		m->goalDistance_y1 = READ_LE_UINT32(m->monsterInfos + 900);
		m->goalDistance_y2 = m->goalDistance_y1 + READ_LE_UINT32(m->monsterInfos + 896);
		m->goalPos_x1 = m->goalDistance_x1;
		m->goalPos_x2 = m->goalDistance_x2;
		m->goalPos_y1 = m->goalDistance_y1;
		m->goalPos_y2 = m->goalDistance_y2;
		const uint8_t *ptr1 = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
		if ((ptr1[2] & kDirectionKeyMaskVertical) == 0) {
			m->goalDistance_y1 = m->goalPos_y1 = m->goalDistance_y2 = m->goalPos_y2 = m->yMstPos;
		}
		if ((ptr1[2] & kDirectionKeyMaskHorizontal) == 0) {
			m->goalDistance_x1 = m->goalPos_x1 = m->goalDistance_x2 = m->goalPos_x2 = m->xMstPos;
		}
		if (p[946] & 4) {
			mstMonster1UpdateGoalPosition(m);
			mstMonster1MoveTowardsGoal2(m);
		} else {
			mstMonster1UpdateGoalPosition(m);
			mstMonster1MoveTowardsGoal1(m);
		}
		_edi = 1;
		int8_t _cl, _dl;
		if (_mstLut1[m->goalDirectionMask] & 1) {
			_cl = m->m49Unk1->unkE;
			_dl = m->m49Unk1->unkF;
		} else {
			_cl = m->m49Unk1->unkC;
			_dl = m->m49Unk1->unkD;
		}
		if (_xMstPos2 < _cl && _yMstPos2 < _dl && !mstMonster1TestGoalDirection(m)) {
			_edi = 0;
		}
	} else {
// 41CE2B
		int32_t _ebp = READ_LE_UINT32(p + 900);
		int32_t _ecx = READ_LE_UINT32(p + 896);
		int r = _ecx / 4;
		m->goalDistance_x1 = _ebp + r;
		m->goalDistance_x2 = _ecx + _ebp - r;
		if (r != 0) {
			r = _rnd.update() % r;
		}
		m->goalDistance_x1 += r;
		m->goalDistance_x2 -= r;
		if (m->goalScreenNum == 0xFD && m->xMstPos < _mstAndyLevelPosX) {
			m->goalPos_x1 = _mstAndyLevelPosX - m->goalDistance_x2;
			m->goalPos_x2 = _mstAndyLevelPosX - m->goalDistance_x1;
		} else {
// 41CEA1
			m->goalPos_x1 = _mstAndyLevelPosX + m->goalDistance_x1;
			m->goalPos_x2 = _mstAndyLevelPosX + m->goalDistance_x2;
		}
// 41CEB4
		mstMonster1SetGoalHorizontal(m);
		_edi = 1;
		while (_xMstPos2 < m->m49Unk1->unkC) {
			if (--m->indexUnk49Unk1 >= 0) {
				m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
			} else {
				_edi = 0;
				break;
			}
		}
	}
// 41CF17
	if (_xMstPos2 <= 0 && ((m->monsterInfos[946] & 2) == 0 || _yMstPos2 <= 0)) {
		if (m->monsterInfos[946] & 4) {
			mstBoundingBoxClear(m, 1);
		}
// 41CF86
		mstTaskResetMonster1WalkPath(t);
		return 0;
	}
// 41CF99
	if (_edi) {
		if (_xMstPos2 >= m->m49->unk14 || ((m->monsterInfos[946] & 2) != 0 && _yMstPos2 >= m->m49->unk15)) {
			const uint8_t *p = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
			if ((m->monsterInfos[946] & 4) != 0 && p[0xE] != 0 && m->flagsA8[0] == 0xFF) {
				const int x1 = m->xMstPos + (int8_t)p[0xC];
				const int x2 = x1 + p[0xE] - 1;
				const int y1 = m->yMstPos + (int8_t)p[0xD];
				const int y2 = y1 + p[0xF] - 1;
				if (mstBoundingBoxCollides2(m->monster1Index, x1, y1, x2, y2) > 1) {
// 41D1F5
					m->indexUnk49Unk1 = 0;
					m->m49Unk1 = &m->m49->data1[0];
					m->flagsA8[3] = 255;
					m->targetLevelPos_x = -1;
					m->targetLevelPos_y = -1;
					mstBoundingBoxClear(m, 1);
					if (p[0xE] != 0) {
						t->flags &= ~0x80;
						mstTaskResetMonster1WalkPath(t);
						return 0;
					}
				} else {
					m->flagsA8[0] = mstBoundingBoxUpdate(0xFF, m->monster1Index, x1, y1, x2, y2);
				}
			}
// 41D115
			if (_xMstPos2 <= m36->unk8 || ((m->monsterInfos[946] & 2) != 0 && _yMstPos2 >= m36->unk8)) {
				m->indexUnk49Unk1 = m->m49->count1 - 1;
				m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
				if (m->monsterInfos[946] & 4) {
					m->flagsA8[3] = 0xFF;
					m->targetLevelPos_x = -1;
					m->targetLevelPos_y = -1;
					mstBoundingBoxClear(m, 1);
				}
			}
// 41D1CE
			if (m->monsterInfos[946] & 4) {
				t->run = &Game::mstTask_monsterWait10;
			} else if (m->monsterInfos[946] & 2) {
				t->run = &Game::mstTask_monsterWait8;
			} else {
				t->run = &Game::mstTask_monsterWait6;
			}
			return (this->*(t->run))(t);
		}
	}
// 41D2D2
	if (m->monsterInfos[946] & 4) {
		mstBoundingBoxClear(m, 1);
	}
// 41D328
	t->flags |= 0x80;
	mstTaskResetMonster1WalkPath(t);
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
		if (_mstVars[30] > kMaxMonsterObjects1) {
			_mstVars[30] = kMaxMonsterObjects1;
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
			const MstBehaviorIndex *m42 = &_res->_mstBehaviorIndexData[arg24];
			if (m42->dataCount == 0) {
				arg1C = m42->data[0];
			} else {
				arg1C = m42->data[_rnd.update() % m42->dataCount];
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
		m->walkCode = 0;
		m->flagsA6 = 0;

		assert((uint32_t)arg1C < _res->_mstBehaviorIndexData[arg24].count1);
		const uint32_t indexBehavior = _res->_mstBehaviorIndexData[arg24].behavior[arg1C];
		MstBehavior *m46 = &_res->_mstBehaviorData[indexBehavior];
		m->m46 = m46;
		assert((uint32_t)arg20 < m46->count);
		MstBehaviorState *behaviorState = &m46->data[arg20]; // _ecx
		m->behaviorState = behaviorState;
		m->monsterInfos = _res->_mstMonsterInfos + behaviorState->indexMonsterInfo * kMonsterInfoDataSize;

		m->localVars[7] = behaviorState->energy;

		if (behaviorState->indexUnk51 == kNone) {
			m->flags48 &= ~4;
		}

		const uint8_t *ptr = m->monsterInfos;
		o = addLvlObject(ptr[945], x1, y1, objScreen, ptr[944], behaviorState->anim, o_flags1, o_flags2, 0, 0);
		if (!o) {
			mstMonster1ResetData(m);
			return;
		}
// 41562C
		m->o16 = o;
		if (_currentLevel == kLvl_lar2 && m->monsterInfos[944] == 26) {
			m->o20 = addLvlObject(ptr[945], x1, y1, objScreen, ptr[944], behaviorState->anim + 1, o_flags1, 0x3001, 0, 0);
			if (!m->o20) {
				warning("mstOp67 failed to addLvlObject in kLvl_lar2");
				mstMonster1ResetData(m);
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
		m->o_flags2 = o_flags2 & 0xFFFF;
		m->lut4Index = _mstLut5[o_flags2 & 0x1F];
		o->dataPtr = m;
	} else {
		for (int i = 0; i < kMaxMonsterObjects2; ++i) {
			if (!_monsterObjects2Table[i].monster2Info) {
				mo = &_monsterObjects2Table[i];
				break;
			}
		}
		if (!mo) {
			warning("mstOp67 no free monster2");
			return;
		}
// 415743
		assert(arg24 >= 0 && arg24 < _res->_mstHdr.infoMonster2Count);
		mo->monster2Info = &_res->_mstInfoMonster2Data[arg24];
		if (currentTask->monster1) {
			mo->monster1 = currentTask->monster1;
		} else if (currentTask->monster2) {
			mo->monster1 = currentTask->monster2->monster1;
		} else {
			mo->monster1 = 0;
		}

		mo->flags24 = 0;

		uint8_t _cl  = mo->monster2Info->type;
		uint16_t anim = mo->monster2Info->anim;

		o = addLvlObject((_cl >> 7) & 1, x1, y1, objScreen, (_cl & 0x7F), anim, o_flags1, o_flags2, 0, 0);
		if (!o) {
			mo->monster2Info = 0;
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
			mo->monster2Info = 0;
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
		mstTaskSetMonster2ScreenPosition(t);
		const uint32_t codeData = mo->monster2Info->codeData;
		assert(codeData != kNone);
		resetTask(t, _res->_mstCodeData + codeData * 4);
		if (_currentLevel == kLvl_fort && mo->monster2Info->type == 27) {
			mstMonster2InitFirefly(mo);
		}
	} else {
// 41593C
		Task *t = findFreeTask();
		if (!t) {
			warning("mstOp67_addMonster no free task found");
			mstMonster1ResetData(m);
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

		m->levelPosBounds_x1 = -1;
		MstBehaviorState *behaviorState = m->behaviorState;
		m->walkNode = _res->_mstWalkPathData[behaviorState->walkPath].data;

		if (m->monsterInfos[946] & 4) {
			m->flagsA8[0] = 0xFF;
			m->flagsA8[1] = 0xFF;
		}
// 415A89
		mstTaskUpdateScreenPosition(t);
		switch (type) {
		case 1:
			mstTaskInitMonster1Type1(t);
			assert(t->run != &Game::mstTask_main || (t->codeData && t->codeData != kUndefinedMonsterByteCode));
			break;
		case 2:
			if (m) {
				m->flagsA6 |= 1;
			}
			mstTaskInitMonster1Type2(t, 0);
			assert(t->run != &Game::mstTask_main || (t->codeData && t->codeData != kUndefinedMonsterByteCode));
			break;
		default:
			m->flagsA5 = 1;
			if (!mstMonster1UpdateWalkPath(m)) {
				mstMonster1ResetWalkPath(m);
			}
			mstTaskSetNextWalkCode(t);
			break;
		}
	}
// 415ADE
	currentTask->flags &= ~0x80;
}

void Game::mstOp68_addMonsterGroup(Task *t, const uint8_t *p, int a, int b, int c, int d) {
	const MstBehaviorIndex *m42 = &_res->_mstBehaviorIndexData[d];
	struct {
		int m42Index;
		int m46Index;
	} data[16];
	int count = 0;
	for (uint32_t i = 0; i < m42->count1; ++i) {
		MstBehavior *m46 = &_res->_mstBehaviorData[m42->behavior[i]];
		for (uint32_t j = 0; j < m46->count; ++j) {
			MstBehaviorState *behaviorState = &m46->data[j];
			uint32_t indexMonsterInfo = p - _res->_mstMonsterInfos;
			assert((indexMonsterInfo % kMonsterInfoDataSize) == 0);
			indexMonsterInfo /= kMonsterInfoDataSize;
			if (behaviorState->indexMonsterInfo == indexMonsterInfo) {
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

int Game::mstTask_wait1(Task *t) {
	debug(kDebug_MONSTER, "mstTask_wait1 t %p", t);
	--t->arg1;
	if (t->arg1 == 0) {
		t->run = &Game::mstTask_main;
		return 0;
	}
	return 1;
}

int Game::mstTask_wait2(Task *t) {
	debug(kDebug_MONSTER, "mstTask_wait2 t %p", t);
	--t->arg1;
	if (t->arg1 == 0) {
		t->run = &Game::mstTask_main;
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

int Game::mstTask_wait3(Task *t) {
	debug(kDebug_MONSTER, "mstTask_wait3 t %p", t);
	if (getTaskFlag(t, t->arg2, t->arg1) == 0) {
		return 1;
	}
	t->run = &Game::mstTask_main;
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

int Game::mstTask_idle(Task *t) {
	debug(kDebug_MONSTER, "mstTask_idle t %p", t);
	return 1;
}

int Game::mstTask_mstOp231(Task *t) {
	const MstOp234Data *m = &_res->_mstOp234Data[t->arg2];
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
		t->run = &Game::mstTask_main;
		return 0;
	}
	return 1;
}

int Game::mstTask_mstOp232(Task *t) {
	warning("mstTask_mstOp232 unimplemented");
	t->run = &Game::mstTask_main;
	return 0;
}

int Game::mstTask_mstOp233(Task *t) {
	debug(kDebug_MONSTER, "mstTask_mstOp233 t %p", t);
	const MstOp234Data *m = &_res->_mstOp234Data[t->arg2];
	const int a = getTaskVar(t, m->indexVar1, m->maskVars & 15);
	const int b = getTaskVar(t, m->indexVar2, m->maskVars >> 4);
	if (!compareOp(m->compare, a, b)) {
		t->run = &Game::mstTask_main;
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

int Game::mstTask_mstOp234(Task *t) {
	debug(kDebug_MONSTER, "mstTask_mstOp234 t %p", t);
	const MstOp234Data *m = &_res->_mstOp234Data[t->arg2];
	const int a = getTaskVar(t, m->indexVar1, m->maskVars & 15);
	const int b = getTaskVar(t, m->indexVar2, m->maskVars >> 4);
	if (compareOp(m->compare, a, b)) {
		t->run = &Game::mstTask_main;
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

int Game::mstTask_monsterWait1(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait1 t %p", t);
	if (t->arg1 == 0) {
		mstMonster1UpdateWalkPath(t->monster1);
		mstTaskRestart(t);
		return 0;
	}
	--t->arg1;
	return 1;
}

int Game::mstTask_monsterWait2(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait2 t %p", t);
	MonsterObject1 *m = t->monster1;
	const uint16_t flags0 = m->o16->flags0;
	if ((flags0 & 0x100) != 0 && (flags0 & 0xFF) == m->o_flags0) {
		mstMonster1UpdateWalkPath(t->monster1);
		mstTaskRestart(t);
		return 0;
	}
	return 1;
}

int Game::mstTask_monsterWait3(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait3 t %p", t);
	MonsterObject1 *m = t->monster1;
	const uint16_t flags0 = m->o16->flags0;
	if ((flags0 & 0xFF) == m->o_flags0) {
		if (t->arg1 > 0) {
			t->run = &Game::mstTask_monsterWait1;
		} else {
			t->run = &Game::mstTask_monsterWait2;
		}
		return (this->*(t->run))(t);
	}
	return 1;
}

int Game::mstTask_monsterWait4(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait4 t %p", t);
	MonsterObject1 *m = t->monster1;
	const uint32_t offset = m->monsterInfos - _res->_mstMonsterInfos;
	assert(offset % kMonsterInfoDataSize == 0);
	const uint32_t num = offset / kMonsterInfoDataSize;
	if (t->arg2 != num) {
		mstMonster1UpdateWalkPath(m);
		mstTaskRestart(t);
		return 0;
	}
	return 1;
}

int Game::mstTask_monsterWait5(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait5 t %p", t);
	// horizontal move
	MonsterObject1 *m = t->monster1;
	mstMonster1SetGoalHorizontal(m);
	if (_xMstPos2 < m->m49Unk1->unk8) {
		if (_xMstPos2 > 0) {
			while (--m->indexUnk49Unk1 >= 0) {
				m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
				if (_xMstPos2 >= m->m49Unk1->unkC) {
					goto set_am;
				}
			}
		}
		return mstTaskStopMonster1(t, m);
	}
set_am:
	const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	mstLvlObjectSetActionDirection(m->o16, ptr, ptr[3], m->goalDirectionMask);
	return 1;
}

int Game::mstTask_monsterWait6(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait6 t %p", t);
	MonsterObject1 *m = t->monster1;
	// horizontal move with goal
	if (m->goalScreenNum == 0xFD && m->xMstPos < _mstAndyLevelPosX) {
		m->goalPos_x1 = _mstAndyLevelPosX - m->goalDistance_x2;
		m->goalPos_x2 = _mstAndyLevelPosX - m->goalDistance_x1;
	} else {
		m->goalPos_x1 = _mstAndyLevelPosX + m->goalDistance_x1;
		m->goalPos_x2 = _mstAndyLevelPosX + m->goalDistance_x2;
	}
	mstMonster1SetGoalHorizontal(m);
	if (_xMstPos2 < m->m49Unk1->unk8) {
		if (_xMstPos2 > 0) {
			while (--m->indexUnk49Unk1 >= 0) {
				m->m49Unk1 = &m->m49->data1[m->indexUnk49Unk1];
				if (_xMstPos2 >= m->m49Unk1->unkC) {
					goto set_am;
				}
			}
		}
		return mstTaskStopMonster1(t, m);
	}
set_am:
	const uint8_t *ptr = _res->_mstMonsterInfos + m->m49Unk1->offsetMonsterInfo;
	mstLvlObjectSetActionDirection(m->o16, ptr, ptr[3], m->goalDirectionMask);
	return 1;
}

int Game::mstTask_monsterWait7(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait7 t %p", t);
	MonsterObject1 *m = t->monster1;
	mstMonster1MoveTowardsGoal1(m);
	return mstTaskUpdatePositionActionDirection(t, m);
}

int Game::mstTask_monsterWait8(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait8 t %p", t);
	MonsterObject1 *m = t->monster1;
	mstMonster1UpdateGoalPosition(m);
	mstMonster1MoveTowardsGoal1(m);
	return mstTaskUpdatePositionActionDirection(t, m);
}

int Game::mstTask_monsterWait9(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait9 t %p", t);
	MonsterObject1 *m = t->monster1;
	mstMonster1MoveTowardsGoal2(m);
	return mstTaskUpdatePositionActionDirection(t, m);
}

int Game::mstTask_monsterWait10(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait10 t %p", t);
	MonsterObject1 *m = t->monster1;
	mstMonster1UpdateGoalPosition(m);
	mstMonster1MoveTowardsGoal2(m);
	return mstTaskUpdatePositionActionDirection(t, m);
}

int Game::mstTask_monsterWait11(Task *t) {
	debug(kDebug_MONSTER, "mstTask_monsterWait11 t %p", t);
	MonsterObject1 *m = t->monster1;
	const int num = m->o16->flags0 & 0xFF;
	if (m->monsterInfos[num * 28] == 0) {
		mstTaskResetMonster1Direction(t);
	}
	return 1;
}
