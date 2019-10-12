
#include "game.h"
#include "level.h"
#include "paf.h"
#include "util.h"
#include "video.h"

struct Level_lar1: Level {
        //const CheckpointData *getCheckpointData() = 0;

	virtual void initialize();
	virtual void terminate();
	virtual void tick();
	virtual void preScreenUpdate(int screenNum);
	virtual void postScreenUpdate(int screenNum);
	virtual void setupLvlObjects(int screenNum);

	void postScreenUpdate_lar1_screen0();
	void postScreenUpdate_lar1_screen3();
	void postScreenUpdate_lar1_screen4();
	void postScreenUpdate_lar1_screen5();
	void postScreenUpdate_lar1_screen8();
	void postScreenUpdate_lar1_screen9();
	void postScreenUpdate_lar1_screen12();
	void postScreenUpdate_lar1_screen13();
	void postScreenUpdate_lar1_screen14();
	void postScreenUpdate_lar1_screen15();
	void postScreenUpdate_lar1_screen16();
	void postScreenUpdate_lar1_screen18();
	void postScreenUpdate_lar1_screen19();
	void postScreenUpdate_lar1_screen20();
	void postScreenUpdate_lar1_screen22();
	void postScreenUpdate_lar1_screen24();

	void preScreenUpdate_lar1_screen0();
	void preScreenUpdate_lar1_screen2();
	void preScreenUpdate_lar1_screen6();
	void preScreenUpdate_lar1_screen8();
	void preScreenUpdate_lar1_screen10();
	void preScreenUpdate_lar1_screen11();
	void preScreenUpdate_lar1_screen12();
	void preScreenUpdate_lar1_screen13();
	void preScreenUpdate_lar1_screen14();
	void preScreenUpdate_lar1_screen15();
	void preScreenUpdate_lar1_screen16();
	void preScreenUpdate_lar1_screen17();
	void preScreenUpdate_lar1_screen18();
	void preScreenUpdate_lar1_screen19();
	void preScreenUpdate_lar1_screen20();
	void preScreenUpdate_lar1_screen23();
	void preScreenUpdate_lar1_screen24();

	void setupLvlObjects_lar1_screen24_helper1();
	void setupLvlObjects_lar1_screen24_helper2(int num);
	void setupLvlObjects_lar1_screen24();
};

Level *Level_lar1_create() {
	return new Level_lar1;
}

uint8_t Game::_lar1_unkData0[13 * 4] = {
        0x02, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x32, 0x09, 0x02, 0x00, 0x32, 0x0E, 0x02, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
};

BoundingBox Game::_lar1_bboxData[24] = {
	{ 203, 162, 213, 166 },
	{  68,  86,  78,  90 },
	{ 195,  58, 205,  62 },
	{ 111, 171, 125, 175 },
	{ 111, 171, 125, 175 },
	{ 202, 171, 212, 175 },
	{ 158,  29, 170,  57 },
	{ 158,  29, 170,  57 },
	{ 158,  29, 170,  57 },
	{ 158,  29, 170,  57 },
	{ 230, 139, 242, 167 },
	{ 230, 139, 242, 167 },
	{ 230, 139, 242, 167 },
	{  86,  37,  98,  65 },
	{  86,  37,  98,  65 },
	{  86,  37,  98,  65 },
	{ 238,  18, 250,  46 },
	{  14, 138,  26, 166 },
	{  14, 138,  26, 166 },
	{  51, 157,  61, 161 },
	{  51, 157,  61, 161 },
	{  51, 157,  61, 161 },
	{ 202, 157, 212, 161 },
	{  32, 145,  44, 173 }
};

static uint8_t _lar1_unkData3[96] = {
	0x04, 0x07, 0x01, 0x00, 0x05, 0x01, 0x01, 0x01, 0x08, 0x0F, 0x02, 0x00, 0x08, 0x07, 0x03, 0x04,
	0x08, 0x07, 0xFF, 0x05, 0x08, 0x07, 0x04, 0x04, 0x09, 0x27, 0xFF, 0x06, 0x09, 0x29, 0x03, 0x02,
	0x09, 0x29, 0xFF, 0x03, 0x09, 0x29, 0xFF, 0x04, 0x0A, 0x0B, 0x04, 0x02, 0x0A, 0x0B, 0xFF, 0x03,
	0x0A, 0x0B, 0xFF, 0x04, 0x0D, 0x07, 0xFF, 0x07, 0x0D, 0x0B, 0x00, 0x00, 0x0D, 0x0B, 0xFF, 0x01,
	0x0E, 0x07, 0x02, 0x08, 0x0F, 0x27, 0x01, 0x09, 0x0F, 0x2F, 0xFF, 0x0B, 0x12, 0x07, 0x01, 0x0B,
	0x12, 0x0B, 0xFF, 0x0D, 0x12, 0x0B, 0xFF, 0x0C, 0x12, 0x15, 0x02, 0x0B, 0x15, 0x2F, 0x00, 0x0E,
};

static const uint8_t byte_4526D8[32] = {
	0x05, 0x00, 0x00, 0x04, 0x06, 0x00, 0x00, 0x00, 0x07, 0xFB, 0x04, 0x05, 0x08, 0xFA, 0x04, 0x05,
	0x09, 0xF9, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x04, 0xF8, 0xF7, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04,
};

static const uint8_t _lar1_setupScreen24Data[24] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x03, 0x00, 0x02, 0x08, 0x02, 0x0B,
	0x08, 0x10, 0x0C, 0x09, 0x11, 0x0E, 0x0D, 0x17,
};

static void setLvlObjectUpdateType3_lar1(Game *g, int screenNum) {
	const uint8_t *p = Game::_lar1_unkData1;
	for (int i = 0; i < 15; ++i) {
		if (p[4] == screenNum && p[1] == 1) {
			LvlObject *o = g->findLvlObject2(0, p[5], p[4]);
			if (o) {
				o->objectUpdateType = 3;
			}
		}
		p += 6;
	}
}

void Game::postScreenUpdate_lar1_helper(LvlObject *o, uint8_t *p, int num) {
	uint32_t mask = 1 << num; // _ebp
	uint8_t _cl = p[0] & 15;
	if (_cl >= 3) {
		if ((o->flags0 & 0x1F) == 0) {
			if (p[3] == 0) {
				if (_cl == 3) {
					p[0] = (p[0] & ~0xB) | 4;
					p[3] = p[1];
					o->directionKeyMask = 1;
					o->actionKeyMask = 0;
				} else {
					p[0] = (p[0] & ~0xC) | 3;
					p[3] = p[2];
					o->directionKeyMask = 4;
					o->actionKeyMask = 0;
				}
			} else {
				--p[3];
				o->directionKeyMask = 0;
				o->actionKeyMask = 0;
			}
		}
	} else {
// 4062C7
		num = p[2] | p[1];
		if (num != p[0]) {
			uint8_t _dl = num >> 4;
			if (_cl != _dl) {
				uint8_t _al = (p[0] & 0xF0) | _dl;
				p[0] = _al;
				if (_al & 0xF0) {
					p[3] = p[1];
				} else {
					p[3] = p[2];
				}
			}
// 4062F3
			if (p[3] == 0) {
				if (p[0] & 0xF) {
					o->directionKeyMask = 1;
					_mstAndyVarMask &= ~mask;
				} else {
					o->directionKeyMask = 4;
					_mstAndyVarMask |= mask;
				}
				_mstHelper1TestValue |= mask;
			} else {
// 406333
				--p[3];
				o->actionKeyMask = 0;
				o->directionKeyMask = 0;
			}
		} else {
// 406343
			uint8_t _dl = p[0] >> 4;
			if (_cl != _dl) {
				if (p[3] != 0) {
					--p[3];
				} else {
					uint8_t _al = (p[0] & 0xF0) | _dl;
					p[0] = _al;
					if (p[0] & 0xF0) {
						o->directionKeyMask = 1;
						_mstAndyVarMask &= ~mask;
					} else {
						o->directionKeyMask = 4;
						_mstAndyVarMask |= mask;
					}
					_mstHelper1TestValue |= mask;
					if (o->screenNum != _currentScreen && o->screenNum != _currentLeftScreen && o->screenNum != _currentRightScreen) {
						o->actionKeyMask = 1;
					} else {
						o->actionKeyMask = 0;
					}
				}
			}
		}
	}
// 4063C4
	int _ebp = o->yPos + o->posTable[1].y;
	int _ecx = o->posTable[1].y - o->posTable[2].y - 7;
	int _edx = o->xPos + o->posTable[1].x;
	if (_edx < 0) {
		_edx = 0;
	}
	if (_ebp < 0) {
		_ebp = 0;
	}
	uint32_t offset = screenMaskOffset(_res->_screensBasePos[o->screenNum].u + _edx, _res->_screensBasePos[o->screenNum].v + _ebp);
	if (_ecx < 0) {
		_ecx = -_ecx;
		for (int i = 0; i < _ecx / 8; ++i) {
			memset(_screenMaskBuffer + offset, 0, 4);
			offset += 4;
		}
	} else {
// 406456
		for (int i = 0; i < _ecx / 8; ++i) {
			memset(_screenMaskBuffer + offset, 2, 4);
			offset += 512;
		}
	}
// 40647B
	if (o->screenNum == _currentScreen || (o->screenNum == _currentRightScreen && _res->_resLevelData0x2B88SizeTable[_currentRightScreen] != 0) || (o->screenNum == _currentLeftScreen && _res->_resLevelData0x2B88SizeTable[_currentLeftScreen] != 0)) {
		updateAndyObject(o);
	}
// 4064C1
	int _ebx = o->yPos + o->posTable[1].y;
	_ecx = o->posTable[2].y - o->posTable[1].y + 7;
	_edx = o->xPos + o->posTable[1].x;
	if (_edx < 0) {
		_edx = 0;
	}
	if (_ebx < 0) {
		_ebx = 0;
	}
	offset = screenMaskOffset(_res->_screensBasePos[o->screenNum].u + _ebx, _res->_screensBasePos[o->screenNum].v + _edx);
	if (_ecx < 0) {
		_ecx = -_ecx;
		for (int i = 0; i < _ecx / 8; ++i) {
			memset(_screenMaskBuffer + offset, 0, 4);
			offset += 512;
		}
	} else {
		for (int i = 0; i < _ecx / 8; ++i) {
			memset(_screenMaskBuffer + offset, 2, 4);
			offset += 512;
		}
	}
// 406576
	if (o->screenNum == _res->_currentScreenResourceNum && o->directionKeyMask == 4) {
		if ((o->flags0 & 0x1F) == 1 && (o->flags0 & 0xE0) == 0x40) {
			if (!_hideAndyObjectFlag && (_mstFlags & 0x80000000) == 0) {
				if (clipLvlObjectsBoundingBox(_andyObject, o, 132)) {
					setAndySpecialAnimation(0xA1);
				}
			}
		}
	}
}

void Level_lar1::postScreenUpdate_lar1_screen0() {
	switch (_res->_screensState[0].s0) {
	case 0:
		_res->_screensState[0].s0 = 3;
		break;
	case 1:
		if (_res->_currentScreenResourceNum == 0) {
			BoundingBox b = { 0, 0, 63, 78 };
			LvlObject *o = _g->findLvlObjectBoundingBox(&b);
			if (o) {
				if (((ShootLvlObjectData *)_g->getLvlObjectDataPtr(o, kObjectDataTypeShoot))->unk0 == 6) {
					_res->_screensState[0].s0 = 4;
				}
			}
		}
		break;
	case 2:
		if (_res->_currentScreenResourceNum == 0) {
			BoundingBox b = { 0, 0, 14, 74 };
			AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (_g->clipBoundingBox(&b, &data->boundingBox)) {
				if (!_g->_fadePalette) {
					_checkpoint = 1;
					_g->_levelRestartCounter = 6;
				} else {
					_andyObject->directionKeyMask = 0;
				}
			}
		}
		break;
	case 3:
		++_screenCounterTable[0];
		if (_screenCounterTable[0] < 7) {
			_andyObject->actionKeyMask = 1;
			_andyObject->directionKeyMask = 2;
		} else if (_screenCounterTable[0] == 7) {
			_res->_resLvlScreenBackgroundDataTable[0].currentMaskId = 1;
			_g->setupScreenMask(0);
		} else if (_screenCounterTable[0] >= 45) {
			_res->_screensState[0].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 1;
		} else if (_screenCounterTable[0] == 11) {
			_g->setShakeScreen(3, 2);
		} else if (_screenCounterTable[0] == 13) {
			_g->setShakeScreen(2, 4);
		} else if (_screenCounterTable[0] == 19) {
			_g->setShakeScreen(2, 4);
		} else if (_screenCounterTable[0] == 25) {
			_g->setShakeScreen(2, 6);
		}
		break;
	case 4:
		++_screenCounterTable[0];
		if (_screenCounterTable[0] >= 64) {
			_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 2;
			_res->_resLvlScreenBackgroundDataTable[0].currentMaskId = 2;
			_g->setupScreenMask(0);
			_res->_screensState[0].s0 = 2;
		}
		break;
	}
}

void Level_lar1::postScreenUpdate_lar1_screen3() {
	if (_res->_currentScreenResourceNum == 3) {
		BoundingBox b = { 46, 0, 210, 106 };
		_g->setAndyAnimationForArea(&b, 16);
	}
}

void Level_lar1::postScreenUpdate_lar1_screen4() {
	LvlObject *o = _g->findLvlObject(2, 0, 4);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar1_unkData0, 0);
}

void Level_lar1::postScreenUpdate_lar1_screen5() {
	LvlObject *o1 = _g->findLvlObject(2, 0, 5);
	_g->postScreenUpdate_lar1_helper(o1, &Game::_lar1_unkData0[4], 1);
	LvlObject *o2 = _g->findLvlObject(2, 1, 5);
	_g->postScreenUpdate_lar1_helper(o2, &Game::_lar1_unkData0[8], 2);
	LvlObject *o3 = _g->findLvlObject(2, 2, 5);
	_g->postScreenUpdate_lar1_helper(o3, &Game::_lar1_unkData0[12], 3);
	if (_res->_currentScreenResourceNum == 5) {
		if (_checkpoint >= 1 && _checkpoint <= 3) {
			_checkpoint = 2;
			BoundingBox b = { 194, 0, 255, 88 };
			AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (_g->clipBoundingBox(&b, &data->boundingBox) && (Game::_lar1_unkData0[0x18] & 0xF0) == 0x10) {
				_checkpoint = 2;
				_screenCounterTable[26] = (Game::_lar1_unkData0[0x1C] < 16) ? 1 : 3;
			}
		}
	}
}

void Level_lar1::postScreenUpdate_lar1_screen8() {
	LvlObject *o1 = _g->findLvlObject(2, 0, 8);
	_g->postScreenUpdate_lar1_helper(o1, &Game::_lar1_unkData0[16], 4);
	LvlObject *o2 = _g->findLvlObject(2, 1, 8);
	_g->postScreenUpdate_lar1_helper(o2, &Game::_lar1_unkData0[20], 5);
	if (_res->_currentScreenResourceNum == 8) {
		if (_checkpoint >= 1 && _checkpoint <= 3) {
			BoundingBox b = { 104, 0, 255, 80 };
			AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (_g->clipBoundingBox(&b, &data->boundingBox)) {
				_checkpoint = 3;
				const int a = (Game::_lar1_unkData0[0x18] & 0xF0) != 0 ? 3 : 4;
				_screenCounterTable[26] = a;
				if ((Game::_lar1_unkData0[0x1C] & 0xF0) == 0x10) {
					_screenCounterTable[26] = a + 2;
				}
			}
		}
	}
}

void Level_lar1::postScreenUpdate_lar1_screen9() {
	LvlObject *o = _g->findLvlObject(2, 0, 9);
	_g->postScreenUpdate_lar1_helper(o, &Game::_lar1_unkData0[0x18], 6);
}

void Level_lar1::postScreenUpdate_lar1_screen12() {
	if (_res->_currentScreenResourceNum == 12) {
		const int counter = _screenCounterTable[12];
		if (counter < 8 && (_andyObject->flags0 & 0x1F) == 3) {
			const uint8_t num = (_andyObject->flags0 >> 5) & 7;
			if (byte_4526D8[counter * 4 + 2] == num || byte_4526D8[counter * 4 + 3] == num) {
				BoundingBox b[] = {
					{ 205,   0, 227,  97 },
					{ 200,  16, 207,  55 },
					{ 128,  16, 159,  71 },
					{  56,  32,  87,  87 },
					{ 179, 112, 207, 167 },
					{ 179,  64, 207, 103 },
					{  32, 112,  87, 167 },
					{  32, 112,  87, 167 }
				};
				AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
				if (_g->clipBoundingBox(&b[counter], &data->boundingBox)) {
					++_screenCounterTable[12];
					uint8_t _bl;
					int _al = (int8_t)byte_4526D8[counter * 4];
					if (_al != 0) {
						if (_al < 0) {
							_al = -_al * 6;
							_g->updateScreenMaskLar(Game::_lar1_unkData1 + _al, false);
							_bl = 5;
						} else {
							_al *= 6;
							_g->updateScreenMaskLar(Game::_lar1_unkData1 + _al, true);
							_bl = 2;
						}
// 40751B
						LvlObject *o = _g->findLvlObject2(0, Game::_lar1_unkData1[_al + 5], Game::_lar1_unkData1[_al + 4]);
						if (o) {
							o->objectUpdateType = _bl;
						}
					}
// 407536
					_al = (int8_t)byte_4526D8[counter * 4 + 1];
					if (_al != 0) {
						if (_al < 0) {
							_al = -_al * 6;
							_g->updateScreenMaskLar(Game::_lar1_unkData1 + _al, false);
							_bl = 5;
						} else {
							_al *= 6;
							_g->updateScreenMaskLar(Game::_lar1_unkData1 + _al, true);
							_bl = 2;
						}
						LvlObject *o = _g->findLvlObject2(0, Game::_lar1_unkData1[_al + 5], Game::_lar1_unkData1[_al + 4]);
						if (o) {
							o->objectUpdateType = _bl;
						}
					}
				}
			}
		}
// 40768C
		_g->restoreAndyCollidesLava();
		_g->setLavaAndyAnimation(0xB6);
	}
}

void Level_lar1::postScreenUpdate_lar1_screen13() {
	LvlObject *o = _g->findLvlObject(2, 0, 13);
	_g->postScreenUpdate_lar1_helper(o, &Game::_lar1_unkData0[0x1C], 7);
}

void Level_lar1::postScreenUpdate_lar1_screen14() {
	if (_res->_currentScreenResourceNum == 14) {
		switch (_res->_screensState[14].s0) {
		case 0:
			if (_g->_currentLevelCheckpoint == 4) {
				if (_checkpoint == 5) {
					_g->_currentLevelCheckpoint = _checkpoint;
					if (!_paf->_skipCutscenes) {
						_paf->play(11);
						_paf->unload();
						_video->clearPalette();
					}
					_g->restartLevel();
				}
			} else {
				BoundingBox b = { 33, 60, 76, 89 };
				LvlObject *o = _g->findLvlObjectBoundingBox(&b);
				if (o) {
					if (((ShootLvlObjectData *)_g->getLvlObjectDataPtr(o, kObjectDataTypeShoot))->unk0 == 6) {
						_res->_screensState[14].s0 = 3;
					}
				}
			}
			break;
		case 1: {
				BoundingBox b = { 172, 23, 222, 53 };
				AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
				if (_g->clipBoundingBox(&b, &data->boundingBox) && (_andyObject->flags0 & 0x1F) == 0 && (_andyObject->flags0 & 0xE0) == 0xE0) {
					_res->_screensState[14].s0 = 4;
					if (!_paf->_skipCutscenes) {
						_paf->preload(12);
					}
				} else {
					_g->setAndySpecialAnimation(3);
				}
			}
			break;
		case 3:
			++_screenCounterTable[14];
			if (_screenCounterTable[14] == 1) {
				_res->_resLvlScreenBackgroundDataTable[14].currentMaskId = 1;
				_g->setupScreenMask(14);
			} else if (_screenCounterTable[14] >= 20) {
				_res->_screensState[14].s0 = 1;
			} else if (_screenCounterTable[14] == 7 || _screenCounterTable[14] == 9 || _screenCounterTable[14] == 11 || _screenCounterTable[14] == 13 || _screenCounterTable[14] == 15) {
				_g->setShakeScreen(3, 2);
			}
			break;
		case 4:
			++_screenCounterTable[14];
			if (_screenCounterTable[14] >= 37) {
				_res->_screensState[14].s0 = 2;
				_res->_resLvlScreenBackgroundDataTable[14].currentBackgroundId = 1;
				_res->_resLvlScreenBackgroundDataTable[14].currentMaskId = 2;
				if (!_paf->_skipCutscenes) {
					_paf->play(12);
					_paf->unload(12);
				}
				_video->clearPalette();
				_g->updateScreen(_andyObject->screenNum);
			}
			break;
		}
	}
	LvlObject *o = _g->findLvlObject(2, 0, 14);
	_g->postScreenUpdate_lar1_helper(o, &Game::_lar1_unkData0[0x20], 8);
}

void Level_lar1::postScreenUpdate_lar1_screen15() {
	LvlObject *o = _g->findLvlObject(2, 0, 15);
	_g->postScreenUpdate_lar1_helper(o, &Game::_lar1_unkData0[0x24], 9);
}

void Level_lar1::postScreenUpdate_lar1_screen16() {
	LvlObject *o = _g->findLvlObject(2, 0, 16);
	_g->postScreenUpdate_lar1_helper(o, &Game::_lar1_unkData0[0x28], 10);
}

void Level_lar1::postScreenUpdate_lar1_screen18() {
	LvlObject *o1 = _g->findLvlObject(2, 0, 18);
	_g->postScreenUpdate_lar1_helper(o1, &Game::_lar1_unkData0[0x2C], 11);
	LvlObject *o2 = _g->findLvlObject(2, 1, 18);
	_g->postScreenUpdate_lar1_helper(o2, &Game::_lar1_unkData0[0x30], 12);
	if ((_lar1_unkData3[0x59] & 0x40) == 0 && (_lar1_unkData3[0x59] & 0x80) != 0) {
		if ((_lar1_unkData3[0x4D] & 1) == 0) {
			_lar1_unkData3[0x4D] |= 1;
		}
	}
	if ((_lar1_unkData3[0x4D] & 0x40) == 0 && (_lar1_unkData3[0x4D] & 0x80) != 0) {
		if ((_lar1_unkData3[0x59] & 1) == 0) {
			_lar1_unkData3[0x59] |= 1;
		}
	}
}

void Level_lar1::postScreenUpdate_lar1_screen19() {
	if (_screenCounterTable[19] == 0) {
		if (_res->_currentScreenResourceNum == 19) {
			BoundingBox b = { 160, 0, 209, 71 };
			AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (_g->clipBoundingBox(&b, &data->boundingBox)) {
				_g->_plasmaCannonFlags |= 2;
				if (!_paf->_skipCutscenes) {
					_paf->play(13);
					_paf->unload(13);
					_video->clearPalette();
					++_screenCounterTable[19];
					_g->updateScreen(_andyObject->screenNum);
					Game::_lar1_unkData1[0x49] = 0;
					Game::_lar1_unkData1[0X4F] = 0;
				}
				_andyObject->xPos = 204;
				_andyObject->yPos = 25;
				_andyObject->anim = 232;
				_andyObject->frame = 0;
				_andyObject->flags1 = (_andyObject->flags1 & 0xFFDF) | 0x10;
				_andyObject->directionKeyMask = 0;
				_andyObject->actionKeyMask = 0;
				_g->setupLvlObjectBitmap(_andyObject);
				_g->removeLvlObject(_andyObject);
				_g->clearLvlObjectsList0();
			}
		}
	} else if (_res->_screensState[19].s0 == 2) {
		++_screenCounterTable[19];
		if (_screenCounterTable[19] == 2) {
			_res->_resLvlScreenBackgroundDataTable[19].currentMaskId = 1;
			_g->setupScreenMask(19);
		} else if (_screenCounterTable[19] >= 14) {
			_res->_screensState[19].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[19].currentBackgroundId = 1;
		}
	}
}

void Level_lar1::postScreenUpdate_lar1_screen20() {
	if (_res->_currentScreenResourceNum == 20) {
		postScreenUpdate_lar1_screen19();
	}
}

void Level_lar1::postScreenUpdate_lar1_screen22() {
	if (_res->_currentScreenResourceNum == 22) {
		BoundingBox b = { 36, 0, 208, 82 };
		_g->setAndyAnimationForArea(&b, 16);
	}
}

void Level_lar1::postScreenUpdate_lar1_screen24() {
	if (_res->_currentScreenResourceNum == 24) {
		if ((_andyObject->flags0 & 0x1F) == 5) {
			_g->_plasmaCannonFlags |= 1;
		}
		BoundingBox b = { 50, 168, 113, 191 };
		AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		if (_g->clipBoundingBox(&b, &data->boundingBox)) {
			if (!_paf->_skipCutscenes) {
				_paf->play(14);
				_paf->unload(14);
			}
			_video->clearPalette();
			_g->_endLevel = true;
		}
	}
}

void Level_lar1::postScreenUpdate(int num) {
	switch (num) {
	case 0:
		postScreenUpdate_lar1_screen0();
		break;
	case 3:
		postScreenUpdate_lar1_screen3();
		break;
	case 4:
		postScreenUpdate_lar1_screen4();
		break;
	case 5:
		postScreenUpdate_lar1_screen5();
		break;
	case 8:
		postScreenUpdate_lar1_screen8();
		break;
	case 9:
		postScreenUpdate_lar1_screen9();
		break;
	case 12:
		postScreenUpdate_lar1_screen12();
		break;
	case 13:
		postScreenUpdate_lar1_screen13();
		break;
	case 14:
		postScreenUpdate_lar1_screen14();
		break;
	case 15:
		postScreenUpdate_lar1_screen15();
		break;
	case 16:
		postScreenUpdate_lar1_screen16();
		break;
	case 18:
		postScreenUpdate_lar1_screen18();
		break;
	case 19:
		postScreenUpdate_lar1_screen19();
		break;
	case 20:
		postScreenUpdate_lar1_screen20();
		break;
	case 22:
		postScreenUpdate_lar1_screen22();
		break;
	case 24:
		postScreenUpdate_lar1_screen24();
		break;
	}
}

void Level_lar1::preScreenUpdate_lar1_screen0() {
	if (_res->_currentScreenResourceNum == 0) {
		switch (_res->_screensState[0].s0) {
		case 0:
			_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 0;
			_res->_resLvlScreenBackgroundDataTable[0].currentMaskId = 0;
			_screenCounterTable[0] = 0;
			break;
		case 3:
			_res->_screensState[0].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 1;
			_res->_resLvlScreenBackgroundDataTable[0].currentMaskId = 1;
			_screenCounterTable[0] = 45;
			break;
		case 4:
			_screenCounterTable[0] = 64;
			_res->_screensState[0].s0 = 2;
			_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 2;
			_res->_resLvlScreenBackgroundDataTable[0].currentMaskId = 2;
			break;
		}
	}
}

void Level_lar1::preScreenUpdate_lar1_screen2() {
	if (_res->_currentScreenResourceNum == 2) {
		if (_checkpoint == 0) {
			_checkpoint = 1;
		}
	}
}

void Level_lar1::preScreenUpdate_lar1_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		if (_checkpoint >= 1 && _checkpoint <= 3) {
			if ((Game::_lar1_unkData0[0x18] & 0xF0) == 0) {
				_checkpoint = 2;
				_screenCounterTable[26] = ((Game::_lar1_unkData0[0x1C] & 0xF0) != 0) ? 2 : 0;
			}
		}
	}
}

void Level_lar1::preScreenUpdate_lar1_screen8() {
	if (_res->_currentScreenResourceNum == 8) {
		setLvlObjectUpdateType3_lar1(_g, 8);
	}
}

void Level_lar1::preScreenUpdate_lar1_screen10() {
	if (_res->_currentScreenResourceNum == 10) {
		setLvlObjectUpdateType3_lar1(_g, 10);
	}
}

void Level_lar1::preScreenUpdate_lar1_screen11() {
	if (_res->_currentScreenResourceNum == 11) {
		if (_checkpoint >= 2 && _checkpoint <= 3) {
			if ((Game::_lar1_unkData0[0x1C] & 0xF) == 1 && (Game::_lar1_unkData0[0x18] & 0xF) == 1) {
				_checkpoint = 4;
			}
		}
		_g->unloadTransformLayerData();
		_video->_displayShadowLayer = false;
	}
}

void Level_lar1::preScreenUpdate_lar1_screen12() {
	if (_res->_currentScreenResourceNum == 12) {
		setLvlObjectUpdateType3_lar1(_g, 12);
		_g->loadTransformLayerData(Game::_pwr2_screenTransformData);
	}
}

void Level_lar1::preScreenUpdate_lar1_screen13() {
	if (_res->_currentScreenResourceNum == 13) {
		_g->unloadTransformLayerData();
		_video->_displayShadowLayer = false;
	}
}

void Level_lar1::preScreenUpdate_lar1_screen14() {
	switch (_res->_screensState[14].s0) {
	case 0:
		_res->_resLvlScreenBackgroundDataTable[14].currentBackgroundId = 0;
		_res->_resLvlScreenBackgroundDataTable[14].currentMaskId = 0;
		_screenCounterTable[14] = 0;
		break;
	case 3:
		_res->_screensState[14].s0 = 1;
		_screenCounterTable[14] = 20;
		break;
	case 4:
		_res->_screensState[14].s0 = 2;
		_screenCounterTable[14] = 37;
		break;
	}
	if (_res->_currentScreenResourceNum == 14) {
		if (_res->_screensState[14].s0 == 0 && _g->_currentLevelCheckpoint == 4) {
			if (!_paf->_skipCutscenes) {
				_paf->preload(11);
			}
		}
	}
}

void Level_lar1::preScreenUpdate_lar1_screen15() {
	if (_res->_currentScreenResourceNum == 15) {
		setLvlObjectUpdateType3_lar1(_g, 15);
	}
}

void Level_lar1::preScreenUpdate_lar1_screen16() {
	if (_res->_currentScreenResourceNum == 16) {
		if (_checkpoint == 5) {
			_checkpoint = 6;
		}
	}
}

void Level_lar1::preScreenUpdate_lar1_screen17() {
	if (_res->_currentScreenResourceNum == 17) {
		setLvlObjectUpdateType3_lar1(_g, 17);
	}
}

void Level_lar1::preScreenUpdate_lar1_screen18() {
	if (_checkpoint >= 7) {
		Game::_lar1_unkData0[0x30] &= 0xF;
		Game::_lar1_unkData0[0x30] |= 0x10;
	}
	if (_res->_currentScreenResourceNum == 18) {
		setLvlObjectUpdateType3_lar1(_g, 18);
	}
}

void Level_lar1::preScreenUpdate_lar1_screen19() {
	if (_res->_currentScreenResourceNum == 19) {
		if (_checkpoint == 6) {
			_res->_screensState[19].s0 = 0;
		}
		_res->_resLvlScreenBackgroundDataTable[19].currentBackgroundId = 0;
		_res->_resLvlScreenBackgroundDataTable[19].currentMaskId = 0;
		if (_res->_screensState[19].s0 != 0) {
			_res->_screensState[19].s0 = 1;
			_screenCounterTable[19] = 14;
			if (_g->_difficulty != 0) {
				_res->_resLvlScreenBackgroundDataTable[19].currentBackgroundId = 1;
				_res->_resLvlScreenBackgroundDataTable[19].currentMaskId = 1;
			}
		} else {
			_screenCounterTable[19] = (_g->_plasmaCannonFlags >> 1) & 1;
		}
		if (_screenCounterTable[19] == 0) {
			if (!_paf->_skipCutscenes) {
				_paf->preload(13);
			}
		}
	}
}

void Level_lar1::preScreenUpdate_lar1_screen20() {
	if (_res->_currentScreenResourceNum == 20) {
		if (_checkpoint == 6) {
			if ((_andyObject->flags0 & 0x1F) == 0xB) {
				_checkpoint = 7;
			}
		}
	}
}

void Level_lar1::preScreenUpdate_lar1_screen23() {
	if (_res->_currentScreenResourceNum == 23) {
		setLvlObjectUpdateType3_lar1(_g, 23);
		if (_g->_plasmaCannonFlags & 2) {
			Game::_lar1_unkData0[0x28] &= 0xF;
			Game::_lar1_unkData0[0x28] |= 0x10;
		}
	}
}

void Level_lar1::preScreenUpdate_lar1_screen24() {
	if (_res->_currentScreenResourceNum == 24) {
		if (!_paf->_skipCutscenes) {
			_paf->preload(14);
		}
	}
}

void Level_lar1::preScreenUpdate(int num) {
        switch (num) {
	case 0:
		preScreenUpdate_lar1_screen0();
		break;
	case 2:
		preScreenUpdate_lar1_screen2();
		break;
	case 6:
		preScreenUpdate_lar1_screen6();
		break;
	case 8:
		preScreenUpdate_lar1_screen8();
		break;
	case 10:
		preScreenUpdate_lar1_screen10();
		break;
	case 11:
		preScreenUpdate_lar1_screen11();
		break;
	case 12:
		preScreenUpdate_lar1_screen12();
		break;
	case 13:
		preScreenUpdate_lar1_screen13();
		break;
	case 14:
		preScreenUpdate_lar1_screen14();
		break;
	case 15:
		preScreenUpdate_lar1_screen15();
		break;
	case 16:
		preScreenUpdate_lar1_screen16();
		break;
	case 17:
		preScreenUpdate_lar1_screen17();
		break;
	case 18:
		preScreenUpdate_lar1_screen18();
		break;
	case 19:
		preScreenUpdate_lar1_screen19();
		break;
	case 20:
		preScreenUpdate_lar1_screen20();
		break;
	case 23:
		preScreenUpdate_lar1_screen23();
		break;
	case 24:
		preScreenUpdate_lar1_screen24();
		break;
	}
}

void Level_lar1::initialize() {
	_g->resetWormHoleSprites();
	_screenCounterTable[26] = 0;
}

void Level_lar1::terminate() {
	_g->unloadTransformLayerData();
}

void Level_lar1::tick() {
	_g->updateLevelTick_lar(24, _lar1_unkData3, Game::_lar1_bboxData);
	_g->updateWormHoleSprites();
	if (_screenCounterTable[19] != 0) {
		_g->_plasmaCannonFlags |= 2;
	}
	if (_res->_currentScreenResourceNum == 12) {
		_video->_displayShadowLayer = true;
	}
}

void Level_lar1::setupLvlObjects_lar1_screen24_helper1() {
	const int num = _checkpoint;
	for (int i = _lar1_setupScreen24Data[num * 3 + 2]; i < 24; ++i) {
		_lar1_unkData3[i * 4 + 1] &= ~0x40;
		_lar1_unkData3[i * 4 + 1] |= 1;
	}
	for (int i = _lar1_setupScreen24Data[num * 3 + 2]; i != 0; --i) {
		_lar1_unkData3[i * 4 + 1] &= ~1;
		_lar1_unkData3[i * 4 + 1] |= 0x40;
	}
// 452630
	static const uint8_t data[] = { 0, 1, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	for (int i = _lar1_setupScreen24Data[num * 3 + 1]; i < 13; ++i) {
		const uint8_t _al = (data[i] << 4) | 2;
		Game::_lar1_unkData0[i * 4] = _al;
		const uint32_t mask = 1 << i;
		if (_al & 0xF0) {
			_g->_mstAndyVarMask &= ~mask;
		} else {
			_g->_mstAndyVarMask |= mask;
		}
		_g->_mstHelper1TestValue |= mask;
	}
	for (int i = _lar1_setupScreen24Data[num * 3 + 1]; i != 0; --i) {
		const uint8_t _al = (((data[i] == 0) ? 1 : 0) << 4) | 2;
		Game::_lar1_unkData0[i * 4] = _al;
		const uint32_t mask = 1 << i;
		if (_al & 0xF0) {
			_g->_mstAndyVarMask &= ~mask;
		} else {
			_g->_mstAndyVarMask |= mask;
		}
		_g->_mstHelper1TestValue |= mask;
	}
	Game::_lar1_unkData0[2] &= 0xF;
	Game::_lar1_unkData0[2] |= 0x30;
	Game::_lar1_unkData0[3] &= 0xF;
	Game::_lar1_unkData0[3] |= 0x30;
}

void Level_lar1::setupLvlObjects_lar1_screen24_helper2(int num) {
// 45279
	static const uint8_t data1[48] = {
		0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x01,
		0x01, 0x01, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x01, 0x01, 0x03, 0x01, 0x01, 0x00, 0x01, 0x01,
	};
	const uint8_t *p = &data1[(num & 7) * 6];
	num = *p++;
	const CheckpointData *dat = &Game::_levelCheckpointData[kLvl_lar1][num];
	_andyObject->xPos = dat->xPos;
	_andyObject->yPos = dat->yPos;
	_andyObject->flags2 = dat->flags2;
	_andyObject->anim = dat->anim;
	_andyObject->flags1 &= 0xFFCF;
	_andyObject->flags1 |= (dat->flags2 >> 10) & 0x30;
// 452990
	static const uint8_t data2[5] = { 7, 6, 0, 4, 5 };
	for (int i = 0; i < 5; ++i) {
		uint8_t _al = *p++;
		num = data2[i];
		_al <<= 4;
		_al |= Game::_lar1_unkData0[num * 4] & 15;
		Game::_lar1_unkData0[num * 4] = _al;
		const uint32_t mask = 1 << num;
		if (_al & 0xF0) {
			_g->_mstAndyVarMask &= ~mask;
		} else {
			_g->_mstAndyVarMask |= mask;
		}
		_g->_mstHelper1TestValue |= mask;
	}
}

void Level_lar1::setupLvlObjects_lar1_screen24() {
	setupLvlObjects_lar1_screen24_helper1();
	const int num = _checkpoint;
	for (int b = _lar1_setupScreen24Data[num * 3]; b < 15; ++b) {
		_g->updateScreenMaskLar(Game::_lar1_unkData1 + b * 6, true);
		LvlObject *o = _g->findLvlObject2(0, Game::_lar1_unkData1[b * 6 + 5], Game::_lar1_unkData1[b * 6 + 4]);
		if (o) {
			o->objectUpdateType = 6;
		}
	}
// 408A53
	for (int b = _lar1_setupScreen24Data[num * 3]; b != 0; --b) {
		int _bl = -b;
		if (_bl < 0) {
			_g->updateScreenMaskLar(Game::_lar1_unkData1 + b * 6, false);
			_bl = 5;
		} else {
// 408B28
			_g->updateScreenMaskLar(Game::_lar1_unkData1 + b * 6, true);
			_bl = 2;
		}
// 408BAE
		LvlObject *o = _g->findLvlObject2(0, Game::_lar1_unkData1[b * 6 + 5], Game::_lar1_unkData1[b * 6 + 4]);
		if (o) {
			o->objectUpdateType = 6;
		}
	}
// 408BE5
	if (num == 2 || num == 3) {
		if (_screenCounterTable[26] == 0 && num == 3) {
			_screenCounterTable[26] = 4;
		}
		setupLvlObjects_lar1_screen24_helper2(_screenCounterTable[26]);
	}
}

void Level_lar1::setupLvlObjects(int num) {
	switch (num) {
	case 24:
		setupLvlObjects_lar1_screen24();
		break;
	}
}
