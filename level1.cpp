/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include "game.h"
#include "paf.h"
#include "video.h"

void Game::level1OpStage0_screen0() {
	switch (_res->_screensState[0].s0) {
	case 0:
		if ((_andyObject->flags0 & 0x1F) == 0 && ((_andyObject->flags0 >> 5) & 7) == 6) {
			_res->_screensState[0].s0 = 2;
		}
		break;
	case 2:
		++_screenCounterTable[0];
		if (_screenCounterTable[0] > 25) {
			_res->_screensState[0].s0 = 1;
		}
		if (_screenCounterTable[0] == 2) {
			setShakeScreen(3, 12);
		}
		break;
	}
}

void Game::level1OpStage0_screen4() {
	switch (_res->_screensState[4].s0) {
	case 0:
		if (_plasmaCannonDirection != 0 && _res->_currentScreenResourceNum == 4) {
			if (testPlasmaCannonPointsDirection(162, 24, 224, 68) == 0) {
				if (testPlasmaCannonPointsDirection(202, 0, 255, 48) == 0) {
					if (testPlasmaCannonPointsDirection(173, 8, 201, 23) == 0) {
						_plasmaCannonLastIndex1 = 0;
						return;
					}
				}
			}
			++_screenCounterTable[4];
			if (_screenCounterTable[4] >= 20) {
				_res->_screensState[4].s0 = 2;
			}
			assert(_plasmaExplosionObject);
			_plasmaExplosionObject->screenNum = _res->_currentScreenResourceNum;
			_gameMainLoopFlag5 = 1;
			if (_shakeScreenDuration == 0) {
				setShakeScreen(3, 2);
			} else {
				++_shakeScreenDuration;
			}
		}
		break;
	case 2:
		++_screenCounterTable[4];
		if (_screenCounterTable[4] == 33) {
			_res->_resLvlScreenBackgroundDataTable[4].unk3 = 1;
			setupScreenMask(4);
		} else if (_screenCounterTable[4] > 46) {
			_res->_screensState[4].s0 = 1;
		}
		if (_screenCounterTable[4] == 31) {
			setShakeScreen(2, 12);
		}
		break;
	}
}

void Game::level1OpStage0_screen8() {
	if (_res->_currentScreenResourceNum == 8) {
		if ((_andyObject->flags0 & 0x1F) == 3) {
			_andyObject->flags2 = 0x3008;
		} else if (_andyObject->yPos + _andyObject->height / 2 < 89) {
			_andyObject->flags2 = 0x3004;
		} else {
			_andyObject->flags2 = 0x300C;
		}
	}
}

void Game::level1OpStage0_screen9Helper(int num) {
	switch (num) {
	case 0:
		removeLvlObject(_andyObject);
		setLvlObjectType8Resource(_andyObject, 8, 0);
		_andyObject->anim = 48;
		break;
	case 2:
		destroyLvlObjectUnk(_andyObject);
		_plasmaCannonDirection = 0;
		_plasmaCannonLastIndex1 = 0;
		_gameMainLoopFlag5 = 0;
		_gameMainLoopFlag6 = 0;
		_plasmaCannonObject = 0;
		setLvlObjectType8Resource(_andyObject, 8, 2);
		_andyObject->anim = 232;
		break;
	}
	_andyObject->frame = 0;
}

void Game::level1OpStage0_screen9() {
	int xPos;
	if (_res->_currentScreenResourceNum == 9) {
		switch (_res->_screensState[9].s0) {
		case 0:
			xPos = 68;
			if ((_andyObject->flags0 & 0xE0) != 0) {
				xPos -= 14;
			}
			if (_andyObject->xPos > xPos && _andyObject->yPos < 86) {
				if (!_paf->_skipCutscenes) {
					_paf->play(1);
					_res->_resLvlScreenBackgroundDataTable[9].currentBackgroundId = 1;
					_video->_paletteNeedRefresh = true;
				}
				if (_currentScreenResourceState == 4) {
					_currentScreenResourceState = 5;
				}
				_res->_screensState[9].s0 = 1;
				level1OpStage0_screen9Helper(2);
				_andyObject->xPos = 105;
				_andyObject->yPos = 52;
				_andyObject->anim = 232;
				_andyObject->frame = 0;
				setupLvlObjectBitmap(_andyObject);
				resetScreen(_andyObject->screenNum);
			}
			break;
		case 1:
			_plasmaCannonFlags |= 2;
			break;
		}
	}
}

void Game::level1OpHelper3(BoundingBox *box, int num) {
	static uint8_t byte_478C6C = 0;
	BoundingBox objBox;
	objBox.x1 = _andyObject->xPos;
	objBox.x2 = _andyObject->xPos + _andyObject->posTable[3].x;
	objBox.y1 = _andyObject->yPos;
	objBox.y2 = _andyObject->yPos + _andyObject->posTable[3].y;
	uint8_t _bl = _andyObject->flags0 & 0x1F;
	if (clipBoundingBox(box, &objBox)) {
		if ((_andyObject->actionKeyMask & 1) == 0) {
			_andyObject->actionKeyMask |= 8;
		}
		if (objBox.x2 < box->x1 + num || objBox.x2 > box->x2 - num) {
			goto ret;
		}
		uint8_t _al = 0;
		if (_currentLevel != 0) {
			if (_bl == 1) {
				if (((_andyObject->flags0 >> 5) & 7) == 3) {
					_al = 0x80;
				}
			} else {
				if (_bl == 2) {
					goto ret;
				}
			}
		} else {
			_andyObject->actionKeyMask &= ~1;
			_andyObject->directionKeyMask &= ~4;
			_andyObject->actionKeyMask |= 8;
			if (_bl == 2) {
				goto ret;
			}
		}
		if (byte_478C6C == 2) {
			_al = 0x80;
		}
		if (_al != 0 && _al > _gameKeyPressedMaskIndex) {
			_gameKeyPressedMaskIndex = _al;
			_plasmaCannonKeyMaskCounter = 0;
		}
	}
ret:
	byte_478C6C = _bl;
}

void Game::level1OpStage0_screen10() {
	if (_res->_currentScreenResourceNum == 10) {
		BoundingBox box = { 64, 0, 267, 191 };
		level1OpHelper3(&box, 12);
	}
}

void Game::level1OpStage0_screen11() {
	if (_res->_currentScreenResourceNum == 11) {
		BoundingBox box = { -12, 0, 162, 191 };
		level1OpHelper3(&box, 12);
	}
}

void Game::level1OpStage0_screen13() {
	if (_res->_currentScreenResourceNum == 13) {
		_plasmaCannonFlags &= ~1;
	}
}

void Game::level1OpStage0_screen15() {
	if (_res->_currentScreenResourceNum == 15) {
		_plasmaCannonFlags &= ~1;
		level1OpStage0_screen16();
	}
}

void Game::level1OpStage0_screen16() {
	switch (_res->_screensState[16].s0) {
	case 0:
		if (_screenCounterTable[16] < 2) {
			if ((_andyObject->flags0 & 0x1F) == 2) {
				break;
			}
			AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (data->unk4 != 2 || _andyObject->xPos <= 155 || _andyObject->yPos >= 87) {
				break;
			}
			++_screenCounterTable[16];
			setShakeScreen(3, 4);
		} else {
			_res->_screensState[16].s0 = 2;
			_plasmaCannonFlags |= 1;
			setShakeScreen(1, 18);
		}
		break;
	case 2:
		++_screenCounterTable[16];
		if (_screenCounterTable[16] == 5) {
			_res->_resLvlScreenBackgroundDataTable[16].unk3 = 1;
			setupScreenMask(16);
		} else if (_screenCounterTable[16] == 23) {
			_andyObject->flags1 &= ~0x30;
			_andyObject->xPos = 131;
			_andyObject->yPos = 177;
			_andyObject->anim = 4;
			_andyObject->frame = 0;
			_plasmaCannonFlags &= ~1;
		} else if (_screenCounterTable[16] == 37) {
			_res->_screensState[16].s0 = 3;
			_res->_resLvlScreenBackgroundDataTable[16].currentBackgroundId = 1;
		}
		break;
	case 3:
		++_screenCounterTable[16];
		if (_screenCounterTable[16] == 55) {
			_res->_screensState[16].s0 = 1;
		}
		break;
	}
}

void Game::level1OpStage0_screen18() {
	LvlObject *o;
	switch (_res->_screensState[18].s0) {
	case 0:
		if (_andyObject->yPos + _andyObject->height < 162) {
			if ((_andyObject->flags0 & 0x1F) == 0 && _screenCounterTable[18] == 0) {
				_screenCounterTable[18] = 1;
			} else {
				++_screenCounterTable[18];
				if (_screenCounterTable[18] == 24) {
					_res->_screensState[16].s0 = 2;
					_res->_resLvlScreenBackgroundDataTable[18].unk3 = 2;
					setupScreenMask(18);
				}
			}
		}
		break;
	case 2:
		o = _res->findLvlObject(2, 0, 18);
		assert(o);
		o->actionKeyMask = 1;
		++_screenCounterTable[18];
		if (_screenCounterTable[18] == 43) {
			setShakeScreen(2, 5);
			_res->_resLvlScreenBackgroundDataTable[18].unk3 = 1;
			setupScreenMask(18);
		} else if (_screenCounterTable[18] == 51) {
			_res->_screensState[18].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[18].currentBackgroundId = 1;
		} else {
			o = _res->findLvlObject(2, 0, 18);
			assert(o);
			if ((o->flags0 & 0x1F) == 11 || (o->flags0 & 0x1F) == 1) {
				break;
			}
			BoundingBox box = { 24, 98, 108, 165 };
			AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (!clipBoundingBox(&box, &data->boundingBox)) {
				return;
			}
			_andyObject->anim = 155;
			_andyObject->xPos = 59;
			_andyObject->yPos = 150;
			_andyObject->frame = 0;
			_andyObject->flags2 = 0x300C;
			setupLvlObjectBitmap(_andyObject);
		}
		break;
	}
}

void Game::level1OpStage0_screen19() {
	int fl;
	switch (_res->_screensState[19].s0) {
	case 0: {
			BoundingBox box = { 155, 69, 210, 88 };
			AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (!clipBoundingBox(&box, &data->boundingBox)) {
				return;
			}
			_res->_screensState[19].s0 = 3;
		}
		break;
	case 1:
		if (_andyObject->xPos <= 139) {
			_andyObject->actionKeyMask = 0x80;
			_andyObject->directionKeyMask = 0;
		}
		fl = _andyObject->flags0 & 0x1F;
		if (fl == 0 || fl == 2) {
			fl = (_andyObject->flags0 >> 5) & 7;
			if (fl == 7 || fl == 2) {
				_res->_screensState[19].s0 = 4;
			}
		}
		break;
	case 2:
		if (_andyObject->yPos < 1) {
			if (!_paf->_skipCutscenes) {
				_paf->play(2);
				_paf->unload(2);
				if (!_paf->_skipCutscenes) {
					_paf->play(21);
				}
			}
			_video->clearPalette();
			_quit = true;
		}
		break;
	case 3:
		++_screenCounterTable[19];
		if (_screenCounterTable[19] == 1) {
			_res->_resLvlScreenBackgroundDataTable[19].unk3 = 1;
			setupScreenMask(19);
			game_unk27(18);
		} else if (_screenCounterTable[19] > 12) {
			_res->_screensState[19].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[19].currentBackgroundId = 1;
		}
		break;
	case 4:
		++_screenCounterTable[19];
		if (_screenCounterTable[19] == 25) {
			setShakeScreen(2, 5);
			_res->_resLvlScreenBackgroundDataTable[19].unk3 = 2;
			setupScreenMask(19);
		} else if (_screenCounterTable[19] == 33) {
			_res->_screensState[19].s0 = 2;
			_res->_resLvlScreenBackgroundDataTable[19].currentBackgroundId = 2;
		}
		break;
	}
}

void Game::callLevelOpStage0_level1(int num) {
	switch (num) {
	case 0:
		level1OpStage0_screen0();
		break;
	case 4:
		level1OpStage0_screen4();
		break;
	case 8:
		level1OpStage0_screen8();
		break;
	case 9:
		level1OpStage0_screen9();
		break;
	case 10:
		level1OpStage0_screen10();
		break;
	case 11:
		level1OpStage0_screen11();
		break;
	case 13:
		level1OpStage0_screen13();
		break;
	case 15:
		level1OpStage0_screen15();
		break;
	case 16:
		level1OpStage0_screen16();
		break;
	case 18:
		level1OpStage0_screen18();
		break;
	case 19:
		level1OpStage0_screen19();
		break;
	}
}

int Game::level1OpStage1_case0(LvlObject *o) {
	return 1;
}

static const uint8_t _level1OpHelper1KeyMaskTable[112] = {
	8, 0, 8, 0, 8, 0, 8, 0, 8, 4, 9, 0, 9, 0, 9, 4,
	9, 0, 9, 0, 8, 4, 9, 0, 9, 4, 8, 0, 8, 4, 8, 4,
	8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 4,
	2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4, 2, 4,
	3, 0, 2, 4, 3, 0, 2, 4, 3, 4, 2, 0, 2, 4, 2, 4,
	8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 4, 8, 4, 4, 0,
	8, 4, 8, 4, 8, 4, 8, 4, 8, 4, 8, 0, 8, 0, 4, 0
};

void Game::level1OpHelper1(LvlObject *ptr, uint8_t *p) {
	const bool sameScreen = (_andyObject->screenNum == ptr->screenNum);
	int i = (_andyObject->width / 2 + _andyObject->xPos + (_andyObject->xPos & 7)) / 8;
	if (i < 0 || ptr->screenNum != _res->_currentScreenResourceNum) {
		i = 0;
	} else if (i > 31) {
		i = 31;
	}
	LvlObject *o = ptr->linkObjPtr;
	assert(o);
	assert(o);
	ptr->directionKeyMask = p[0x20 + i];
	if (ptr->directionKeyMask != 0x80) {
		p[0x43] = 0;
		ptr->actionKeyMask = p[i];
	} else {
		if (p[0x43] != 0) {
			--p[0x43];
		} else {
			p[0x43] = (_rnd.getNextNumber() >> 4) + 4;
			p[0x42] = _rnd.getNextNumber() & 7;
		}
		const int index = p[i] * 8 + p[0x42];
		ptr->directionKeyMask = _level1OpHelper1KeyMaskTable[index * 2];
		ptr->actionKeyMask = _level1OpHelper1KeyMaskTable[index * 2 + 1];
	}
	if ((ptr->actionKeyMask & 4) != 0 && sameScreen && (ptr->flags0 & 0x300) == 0x300) {
		if (clipLvlObjectsBoundingBox(_andyObject, ptr, 10)) {
			_mstGlobalFlags |= 0x80000000;
			game_unk27(0x80);
		}
	}
	if ((ptr->directionKeyMask & 4) != 0) {
		if (ptr->anim != 1 || ptr->frame != p[0x40]) {
			ptr->directionKeyMask &= ~4;
		}
	}
	if (_plasmaCannonDirection && (ptr->flags0 & 0x1F) != 0xB) {
		if (level1OpHelper2(ptr->linkObjPtr) != 0) {
			ptr->actionKeyMask |= 0x20;
			++ptr->stateCounter;
			if (ptr->stateCounter > p[0x41]) {
				ptr->stateCounter = 0;
				ptr->actionKeyMask |= 7;
			}
			_gameMainLoopFlag5 = 1;
			_plasmaCannonObject = ptr->linkObjPtr;
		}
	}
	o->directionKeyMask = ptr->directionKeyMask;
	o->actionKeyMask = ptr->actionKeyMask;
	updateAndyObject(ptr);
	updateAndyObject(o);
}

int Game::level1OpHelper2(LvlObject *ptr) {
	assert(ptr);
	if (ptr->bitmapBits) {
		int dx;
		if (ptr->screenNum == _currentLeftScreen) {
			dx = -256;
		} else if (ptr->screenNum == _currentRightScreen) {
			dx = 256;
		} else if (ptr->screenNum == _gameResData0x2E08) {
			dx = 0;
		} else {
			return 0;
		}
		if (testPlasmaCannonPointsDirection(ptr->xPos + dx, ptr->yPos, ptr->xPos + dx + ptr->width, ptr->yPos + ptr->height)) {
			return 1;
		}
		assert(_plasmaExplosionObject);
		_plasmaExplosionObject->screenNum = ptr->screenNum;
	}
	return 0;
}

int Game::level1OpStage1_case1(LvlObject *o) {
	static uint8_t data[68] = {
		0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
		0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
		0x0C, 0x07, 0x00, 0x00
	};
	if (_screenCounterTable[2] == 0) {
		level1OpHelper1(o, data);
		if ((o->flags0 & 0x3FF) == 0x4B) {
			_screenCounterTable[2] = 1;
		}
		return 1;
	}
	return 0;
}

int Game::level1OpStage1_case2(LvlObject *o) {
	static uint8_t data[68] = {
		0x00, 0x00, 0x00, 0x00, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
		0x06, 0x06, 0x06, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x04, 0x04, 0x04, 0x04, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04,
		0x06, 0x0B, 0x00, 0x00
	};
	if (_screenCounterTable[3] == 0) {
		level1OpHelper1(o, data);
		if ((o->flags0 & 0x3FF) == 0x4B) {
			_screenCounterTable[3] = 1;
		}
	} else {
		o->bitmapBits = 0;
		o = o->linkObjPtr;
		assert(o);
		o->anim = 0;
		o->frame = 0;
		o->xPos = 204;
		o->yPos = 0;
		setupLvlObjectBitmap(o);
	}
	return 1;
}

int Game::level1OpStage1_case3(LvlObject *o) {
	updateAndyObject(o);
	return 1;
}

int Game::level1OpStage1_case4(LvlObject *o) {
	updateAndyObject(o);
	updateAndyObject(o->linkObjPtr);
	return 1;
}

int Game::callLevelOpStage1_level1(int num, LvlObject *o) {
	return (this->*_level1OpStage1[num])(o);
}

void Game::level1OpStage2_screen0() {
	switch (_res->_screensState[0].s0) {
	case 0:
		_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 0;
		break;
	default:
		_res->_screensState[0].s0 = 1;
		_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 1;
		break;
	}
}

void Game::level1OpStage2_screen1() {
	if (_currentScreenResourceState != 0) {
		_res->_screensState[0].s0 = 1;
	}
}

void Game::level1OpStage2_screen2() {
	if (_res->_currentScreenResourceNum == 2 && _currentScreenResourceState == 0) {
		_currentScreenResourceState = 1;
	} else if (_currentScreenResourceState > 1) {
		LvlObject *ptr = _res->findLvlObject(2, 0, 2);
		if (ptr) {
			ptr->anim = 0;
			ptr->frame = 0;
		}
		setupLvlObjectBitmap(ptr);
		ptr = _res->findLvlObject(2, 1, 2);
		if (ptr) {
			ptr->anim = 0;
			ptr->frame = 0;
		}
		setupLvlObjectBitmap(ptr);
	}
}

void Game::level1OpStage2_screen3() {
	if (_currentScreenResourceState > 1) {
		LvlObject *ptr = _res->findLvlObject(2, 0, 3);
		if (ptr) {
			ptr->anim = 0;
			ptr->frame = 0;
		}
		setupLvlObjectBitmap(ptr);
		ptr = _res->findLvlObject(2, 1, 3);
		if (ptr) {
			ptr->anim = 0;
			ptr->frame = 0;
		}
		setupLvlObjectBitmap(ptr);
	}
}

void Game::level1OpStage2_screen4() {
	int num;
	switch (_res->_screensState[4].s0) {
	case 0:
		num = 0;
		break;
	case 1:
		num = 1;
		break;
	default:
		_res->_screensState[4].s0 = 1;
		num = 1;
		break;
	}
	_res->_resLvlScreenBackgroundDataTable[4].currentBackgroundId = num;
	_res->_resLvlScreenBackgroundDataTable[4].currentDataUnk1Id = num;
	_res->_resLvlScreenBackgroundDataTable[4].unk3 = num;
	if (_res->_currentScreenResourceNum == 4 && _currentScreenResourceState == 1) {
		_currentScreenResourceState = 2;
	}
}

void Game::level1OpStage2_screen5() {
	if (_res->_currentScreenResourceNum == 5 && _currentScreenResourceState == 2) {
		_currentScreenResourceState = 3;
	}
}

void Game::level1OpStage2_screen7() {
	if (_res->_currentScreenResourceNum == 7 && _currentScreenResourceState == 3) {
		_currentScreenResourceState = 4;
	}
}

void Game::level1OpStage2_screen9() {
	switch (_res->_screensState[9].s0) {
	case 0:
		if (!_paf->_skipCutscenes) {
			_paf->preload(1);
		}
		_res->_resLvlScreenBackgroundDataTable[9].currentBackgroundId = 0;
		break;
	default:
		_res->_screensState[9].s0 = 1;
		_res->_resLvlScreenBackgroundDataTable[9].currentBackgroundId = 1;
		break;
	}
}

void Game::level1OpStage2_screen10() {
	if (_res->_currentScreenResourceNum == 10) {
		if (_currentScreenResourceState == 4) {
			_currentScreenResourceState = 5;
		}
		if (!_paf->_skipCutscenes) {
			_paf->unload(22);
			_paf->preload(23);
		}
	}
}

void Game::level1OpStage2_screen13() {
	if (_res->_currentScreenResourceNum == 13) {
		if (_currentScreenResourceState == 5) {
			_currentScreenResourceState = 6;
		}
		if (!_paf->_skipCutscenes) {
			_paf->unload(1);
		}
	}
}

void Game::level1OpStage2_screen14() {
	if (_res->_currentScreenResourceNum == 14) {
		_res->_screensState[16].s0 = 0;
		_screenCounterTable[16] = 0;
	}
}

void Game::level1OpStage2_screen15() {
	if (_res->_currentScreenResourceNum == 15) {
		if (_res->_screensState[16].s0 != 0) {
			_gameResType0CallbackFlag2 = 2;
			_gameResType0CallbackFlag1 = 1;
		}
		playAndyFallingCutscene(1);
	}
}

void Game::level1OpStage2_screen16() {
	uint8_t _al;
	switch (_res->_screensState[16].s0) {
	case 0:
		_al = 0;
		break;
	default:
		_res->_screensState[16].s0 = 1;
		_plasmaCannonFlags &= ~1;
		_al = 1;
		break;
	}
	_res->_resLvlScreenBackgroundDataTable[16].currentBackgroundId = _al;
	_res->_resLvlScreenBackgroundDataTable[16].unk3 = _al;
	if (_res->_currentScreenResourceNum == 16) {
		playAndyFallingCutscene(1);
	}
}

void Game::level1OpStage2_screen17() {
	if (_res->_currentScreenResourceNum == 17) {
		if (_currentScreenResourceState == 6) {
			_currentScreenResourceState = 7;
		}
		playAndyFallingCutscene(1);
	}
}

void Game::level1OpStage2_screen18() {
	uint8_t _al;
	switch (_res->_screensState[18].s0) {
	case 0:
		_al = 0;
		break;
	default:
		_res->_screensState[18].s0 = 1;
		_al = 1;
		break;
	}
	_res->_resLvlScreenBackgroundDataTable[18].currentBackgroundId = _al;
	_res->_resLvlScreenBackgroundDataTable[18].unk3 = _al;
	if (_res->_currentScreenResourceNum == 18) {
		playAndyFallingCutscene(1);
	}
}

void Game::level1OpStage2_screen19() {
	uint8_t _al;
	switch (_res->_screensState[18].s0) {
	case 0:
		_al = 0;
		break;
	case 1:
		_al = 1;
		break;
	default:
		_res->_screensState[19].s0 = 2;
		_al = 2;
		break;
	}
	_res->_resLvlScreenBackgroundDataTable[19].currentBackgroundId = _al;
	_res->_resLvlScreenBackgroundDataTable[19].unk3 = _al;
	if (_res->_currentScreenResourceNum == 19) {
		if (!_paf->_skipCutscenes) {
			_paf->preload(2);
		}
	}
}

void Game::callLevelOpStage2_level1(int num) {
	switch (num) {
	case 0:
		level1OpStage2_screen0();
		break;
	case 1:
		level1OpStage2_screen1();
		break;
	case 2:
		level1OpStage2_screen2();
		break;
	case 3:
		level1OpStage2_screen3();
		break;
	case 4:
		level1OpStage2_screen4();
		break;
	case 5:
		level1OpStage2_screen5();
		break;
	case 7:
		level1OpStage2_screen7();
		break;
	case 9:
		level1OpStage2_screen9();
		break;
	case 10:
		level1OpStage2_screen10();
		break;
	case 13:
		level1OpStage2_screen13();
		break;
	case 14:
		level1OpStage2_screen14();
		break;
	case 15:
		level1OpStage2_screen15();
		break;
	case 16:
		level1OpStage2_screen16();
		break;
	case 17:
		level1OpStage2_screen17();
		break;
	case 18:
		level1OpStage2_screen18();
		break;
	case 19:
		level1OpStage2_screen19();
		break;
	}
}

void Game::level1OpStage3() {
	if (!_paf->_skipCutscenes) {
		if (_andyObject->data0x2988 == 0) {
			_paf->preload(22);
		} else {
			_paf->preload(23);
		}
	}
}

void Game::level1OpStage4() {
	if (_res->_currentScreenResourceNum > 9) {
		_plasmaCannonFlags |= 2;
	}
}

void Game::level1OpStage5() {
	if (!_paf->_skipCutscenes) {
		if (_andyObject->data0x2988 == 0) {
			_paf->unload(22);
		} else {
			_paf->unload(23);
		}
	}
}

void Game::level1SetupLvlObjects_screen2() {
	LvlObject *ptr = _res->findLvlObject(2, 0, 2);
	if (ptr) {
		ptr->xPos = 146;
		ptr->yPos = 0;
		ptr->anim = 1;
		ptr->frame = 0;
		ptr->directionKeyMask = 0;
		ptr->actionKeyMask = 0;
	}
	ptr = _res->findLvlObject(2, 1, 2);
	if (ptr) {
		ptr->xPos = 88;
		ptr->yPos = 0;
		ptr->anim = 1;
		ptr->frame = 0;
		ptr->directionKeyMask = 0;
		ptr->actionKeyMask = 0;
	}
}

void Game::level1SetupLvlObjects_screen3() {
	LvlObject *ptr = _res->findLvlObject(2, 0, 3);
	if (ptr) {
		ptr->xPos = 198;
		ptr->yPos = 0;
		ptr->anim = 1;
		ptr->frame = 0;
		ptr->directionKeyMask = 0;
		ptr->actionKeyMask = 0;
	}
	ptr = _res->findLvlObject(2, 1, 3);
	if (ptr) {
		ptr->xPos = 116;
		ptr->yPos = 0;
		ptr->anim = 1;
		ptr->frame = 0;
		ptr->directionKeyMask = 0;
		ptr->actionKeyMask = 0;
	}
}

void Game::level1SetupLvlObjects_screen18() {
	LvlObject *ptr = _res->findLvlObject(2, 0, 18);
	if (ptr) {
		ptr->xPos = 16;
		ptr->yPos = 78;
		ptr->anim = 0;
		ptr->frame = 0;
		ptr->directionKeyMask = 0;
		ptr->actionKeyMask = 0;
	}
}

void Game::level1SetupLvlObjects(int num) {
	switch (num) {
	case 2:
		level1SetupLvlObjects_screen2();
		break;
	case 3:
		level1SetupLvlObjects_screen3();
		break;
	case 18:
		level1SetupLvlObjects_screen18();
		break;
	}
}

