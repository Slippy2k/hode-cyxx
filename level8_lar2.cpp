
#include "game.h"
#include "level.h"
#include "paf.h"
#include "util.h"
#include "video.h"

struct Level_lar2: Level {
        //const CheckpointData *getCheckpointData() = 0;

	//virtual void initialize();
	//virtual void terminate();
	virtual void tick();
	virtual void preScreenUpdate(int screenNum);
	virtual void postScreenUpdate(int screenNum);
	virtual void setupLvlObjects(int screenNum);

	bool postScreenUpdate_lar2_screen2_helper(BoundingBox *b);

	void postScreenUpdate_lar2_screen2();
	void postScreenUpdate_lar2_screen3();
	void postScreenUpdate_lar2_screen4();
	void postScreenUpdate_lar2_screen5();
	void postScreenUpdate_lar2_screen6();
	void postScreenUpdate_lar2_screen7();
	void postScreenUpdate_lar2_screen8();
	void postScreenUpdate_lar2_screen10();
	void postScreenUpdate_lar2_screen11();
	void postScreenUpdate_lar2_screen12();
	void postScreenUpdate_lar2_screen13();
	void postScreenUpdate_lar2_screen19();

	void preScreenUpdate_lar2_screen2();
	void preScreenUpdate_lar2_screen4();
	void preScreenUpdate_lar2_screen5();
	void preScreenUpdate_lar2_screen6();
	void preScreenUpdate_lar2_screen7();
	void preScreenUpdate_lar2_screen8();
	void preScreenUpdate_lar2_screen9();
	void preScreenUpdate_lar2_screen15();
	void preScreenUpdate_lar2_screen19();

	void setupLvlObjects_lar2_screen19();
};

Level *Level_lar2_create() {
	return new Level_lar2;
}

static uint8_t byte_4528BD[15] = { 5, 0, 9, 0xD, 7, 0xFF, 8, 0xD, 5, 1, 8, 0xD, 7, 0xFF, 9 };

uint8_t Game::_lar2_unkData0[40] = {
	0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x12, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00
};

static const uint8_t byte_4525F8[52] = {
	0x02, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x32, 0x09, 0x02, 0x00, 0x32, 0x0E, 0x02, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00,
};

static const uint8_t byte_4528F8[17] = {
	0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00
};

static const uint8_t _lar2_lutData[39] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x02,
	0x00, 0x00, 0x05, 0x02, 0x00, 0x02, 0x00, 0x00, 0x03, 0x00, 0x00, 0x09, 0x0C, 0x00, 0x0A, 0x0C,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

BoundingBox Game::_lar2_bboxData[13] = {
	{ 210, 155, 220, 159 },
	{ 224, 146, 234, 150 },
	{ 193,  84, 203,  88 },
	{ 209,  83, 219,  87 },
	{  65, 179,  75, 183 },
	{  65, 179,  75, 183 },
	{ 145, 179, 155, 183 },
	{ 145, 179, 155, 183 },
	{ 224, 179, 234, 183 },
	{  16,  84,  26,  88 },
	{  16,  84,  26,  88 },
	{  16, 164,  26, 168 },
	{  16, 164,  26, 168 }
};

static uint8_t _lar2_unkData3[52] = {
	0x03, 0x07, 0x00, 0x01, 0x0A, 0x07, 0x00, 0x04, 0x09, 0x03, 0x00, 0x07, 0x08, 0x03, 0x00, 0x07,
	0x0B, 0x17, 0x00, 0x06, 0x0B, 0x15, 0xFF, 0x05, 0x0B, 0x15, 0x01, 0x06, 0x0B, 0x17, 0xFF, 0x05,
	0x0B, 0x17, 0x02, 0x06, 0x0D, 0x05, 0x00, 0x09, 0x0D, 0x07, 0xFF, 0x08, 0x0D, 0x05, 0x01, 0x08,
	0x0D, 0x07, 0xFF, 0x09,
};

bool Level_lar2::postScreenUpdate_lar2_screen2_helper(BoundingBox *b) {
	bool ret = false;
	BoundingBox b1 = { 121, 158, 131, 162 };
	if (_g->clipBoundingBox(&b1, b)) {
		ret = true;
		Game::_lar2_unkData0[0] &= 0xF;
		LvlObject *o = _g->findLvlObject(0, 0, 2);
		if (o) {
			o->objectUpdateType = 7;
		}
	}
	BoundingBox b2 = { 170, 158, 180, 162 };
	if (_g->clipBoundingBox(&b2, b)) {
		ret = true;
		Game::_lar2_unkData0[0] &= 0xF;
		LvlObject *o = _g->findLvlObject(0, 0, 2);
		if (o) {
			o->objectUpdateType = 7;
		}
	}
	BoundingBox b3 = { 215, 158, 225, 162 };
	if (_g->clipBoundingBox(&b3, b)) {
		ret = true;
		Game::_lar2_unkData0[0] &= 0xF;
		LvlObject *o = _g->findLvlObject(0, 0, 2);
		if (o) {
			o->objectUpdateType = 7;
		}
	}
	return ret;
}

void Level_lar2::postScreenUpdate_lar2_screen2() {
	LvlObject *o = _g->findLvlObject(2, 0, 2);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0, 0);
	if (_res->_currentScreenResourceNum == 2) {
		bool ret = false;
		for (LvlObject *o = _g->_lvlObjectsList1; o; o = o->nextPtr) {
			if (o->spriteNum == 16 && o->screenNum == _res->_currentScreenResourceNum) {
				BoundingBox b;
				b.x1 = o->xPos;
				b.y1 = o->yPos;
				b.x2 = o->xPos + o->width  - 1;
				b.y2 = o->yPos + o->height - 1;
				if (postScreenUpdate_lar2_screen2_helper(&b)) {
					ret = true;
				}
			}
		}
		AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		if (postScreenUpdate_lar2_screen2_helper(&data->boundingBox) == 0) {
			BoundingBox b = { 107, 77, 117, 81 };
			if (_g->clipBoundingBox(&b, &data->boundingBox)) {
				LvlObject *o = _g->findLvlObject2(0, 3, 2);
				if (o) {
					o->objectUpdateType = 7;
				}
				if (!ret) {
					Game::_lar2_unkData0[0] = (Game::_lar2_unkData0[0] & 0xF) | 0x10;
				}
			}
		}
	}
}

void Level_lar2::postScreenUpdate_lar2_screen3() {
	LvlObject *o = _g->findLvlObject(2, 0, 3);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0 + 4, 1);
}

void Level_lar2::postScreenUpdate_lar2_screen4() {
	if (_g->_currentLevelCheckpoint == 8 && _g->_levelCheckpoint == 9) {
		Game::_lar2_unkData0[8] = (Game::_lar2_unkData0[8] & 0xF) | 0x10;
		if (!_paf->_skipCutscenes) {
			_paf->play(18);
			_paf->unload(18);
			_video->clearPalette();
			_g->_currentLevelCheckpoint = _g->_levelCheckpoint;
			_g->updateScreen(_andyObject->screenNum);
		}
		LvlObject *o = _g->findLvlObject(2, 0, 4);
		_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0 + 8, 2);
	}
}

void Level_lar2::postScreenUpdate_lar2_screen5() {
	if (_g->_currentLevelCheckpoint == 7 && _g->_levelCheckpoint == 8) {
		Game::_lar2_unkData0[0xC] &= 0xF;
	}
	LvlObject *o = _g->findLvlObject(2, 0, 5);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0 + 0xC, 3);
}

void Level_lar2::postScreenUpdate_lar2_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		if (_g->_levelCheckpoint != 3) {
			if (!_paf->_skipCutscenes) {
				if (_g->_currentLevelCheckpoint == 6) {
					_paf->play(17);
					_paf->unload(17);
					_video->clearPalette();
					_g->_currentLevelCheckpoint = _g->_levelCheckpoint;
					_g->updateScreen(_andyObject->screenNum);
				}
			}
		} else {
			if (!_paf->_skipCutscenes) {
				if (_res->_screensState[6].s0 != 0) {
					_paf->play(15);
					_paf->unload(15);
					_video->clearPalette();
					_res->_screensState[6].s0 = 0;
					_g->updateScreen(_andyObject->screenNum);
				}
			}

		}
	}
}

void Level_lar2::postScreenUpdate_lar2_screen7() {
	if (_res->_currentScreenResourceNum == 7) {
		if (!_paf->_skipCutscenes) {
			const uint8_t state = _res->_screensState[7].s0;
			if (state != 0 && _g->_levelCheckpoint == 5 && state > 1) {
				_paf->play(16);
				_paf->unload(16);
				_video->clearPalette();
				_res->_screensState[7].s0 = 1;
				_g->updateScreen(_andyObject->screenNum);
			}
		}
	}
}

void Level_lar2::postScreenUpdate_lar2_screen8() {
	LvlObject *o = _g->findLvlObject(2, 0, 8);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0 + 0x1C, 7);
}

void Level_lar2::postScreenUpdate_lar2_screen10() {
	LvlObject *o = _g->findLvlObject(2, 0, 10);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0 + 0x10, 4);
}

void Level_lar2::postScreenUpdate_lar2_screen11() {
	LvlObject *o = _g->findLvlObject(2, 0, 11);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0 + 0x14, 5);
	o = _g->findLvlObject(2, 1, 11);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0 + 0x18, 6);
	uint8_t *p = _lar2_unkData3 + 0x18;
	if ((_lar2_unkData3[0x11] & 1) == 0 && (_lar2_unkData3[0x11] & 0x40) != 0 && (_lar2_unkData3[0x19] & 1) == 0) {
		_lar2_unkData3[0x19] = (_lar2_unkData3[0x19] & ~0x40) | 1;
		p = _lar2_unkData3 + 0x1C;
		_lar2_unkData3[0x1D] = (_lar2_unkData3[0x1D] & ~0x40) | 1;
	}
	if ((p[1] & 1) == 0 && (p[1] & 0x40) != 0) {
		if ((_lar2_unkData3[0x21] & 1) != 0) {
			goto next;
		}
		_lar2_unkData3[0x21] |= 1;
		_lar2_unkData3[0x21] &= ~0x40;
	}
	if ((_lar2_unkData3[0x21] & 1) == 0 && (_lar2_unkData3[0x21] & 0x40) != 0 && (p[1] & 1) == 0) {
		p[1] &= ~0x40;
		p[1] |= 1;
		_lar2_unkData3[0x1D] |= 1;
		_lar2_unkData3[0x1D] &= ~0x40;
		p = _lar2_unkData3 + 0x1C;
	}
next:
	if ((p[1] & 1) == 0 && (p[1] & 0x40) != 0 && (_lar2_unkData3[0x11] & 1) == 0) {
		_lar2_unkData3[0x11] |= 1;
		_lar2_unkData3[0x11] &= ~0x40;
		_lar2_unkData3[0x15] |= 1;
		_lar2_unkData3[0x15] &= ~0x40;
	}
}

void Level_lar2::postScreenUpdate_lar2_screen12() {
	LvlObject *o = _g->findLvlObject(2, 0, 12);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0 + 0x20, 8);
	o = _g->findLvlObject(2, 1, 12);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0 + 0x24, 9);
	if (_res->_currentScreenResourceNum == 12) {
		BoundingBox b1 = { 65, 84, 75, 88 };
		AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		if (_g->clipBoundingBox(&b1, &data->boundingBox)) {
			Game::_lar2_unkData0[0x20] &= 0xF;
			o = _g->findLvlObject2(0, 0, 12);
			if (o) {
				o->objectUpdateType = 7;
			}
		} else {
			BoundingBox b2 = { 65, 163, 75, 167 };
			if (_g->clipBoundingBox(&b2, &data->boundingBox)) {
				Game::_lar2_unkData0[0x24] &= 0xF;
				o = _g->findLvlObject2(0, 1, 12);
				if (o) {
					o->objectUpdateType = 7;
				}
			}

		}
	}
}

void Level_lar2::postScreenUpdate_lar2_screen13() {
	if (_res->_currentScreenResourceNum == 13) {
		const uint8_t *p = &byte_4528BD[7];
		if ((byte_4528BD[0] & 1) == 0 && (byte_4528BD[0] & 0x40) != 0) {
			if ((byte_4528BD[8] & 1) == 0) {
				p = &byte_4528BD[11];
				byte_4528BD[8] &= ~0x40;
				byte_4528BD[12] = (byte_4528BD[12] | 1) & ~0x40;
			}
		}
		if ((p[1] & 1) == 0 && (p[1] & 0x40) != 0) {
			if ((byte_4528BD[0] & 1) == 0) {
				byte_4528BD[0] = (byte_4528BD[0] | 1) & ~0x40;
				byte_4528BD[4] = (byte_4528BD[4] | 1) & ~0x40;
			}
		}
	}
}

void Level_lar2::postScreenUpdate_lar2_screen19() {
	if (_res->_currentScreenResourceNum == 19) {
		if (_g->_currentLevelCheckpoint == 10 && _g->_levelCheckpoint == 11) {
			if (!_paf->_skipCutscenes) {
				_paf->play(19);
				_paf->unload(19);
			}
			_video->clearPalette();
			_g->_endLevel = true;
		}
	}
}

void Level_lar2::postScreenUpdate(int num) {
	switch (num) {
	case 2:
		postScreenUpdate_lar2_screen2();
		break;
	case 3:
		postScreenUpdate_lar2_screen3();
		break;
	case 4:
		postScreenUpdate_lar2_screen4();
		break;
	case 5:
		postScreenUpdate_lar2_screen5();
		break;
	case 6:
		postScreenUpdate_lar2_screen6();
		break;
	case 7:
		postScreenUpdate_lar2_screen7();
		break;
	case 8:
		postScreenUpdate_lar2_screen8();
		break;
	case 10:
		postScreenUpdate_lar2_screen10();
		break;
	case 11:
		postScreenUpdate_lar2_screen11();
		break;
	case 12:
		postScreenUpdate_lar2_screen12();
		break;
	case 13:
		postScreenUpdate_lar2_screen13();
		break;
	case 19:
		postScreenUpdate_lar2_screen19();
		break;
	}
}

void Level_lar2::preScreenUpdate_lar2_screen2() {
	LvlObject *o = _g->findLvlObject(2, 0, 2);
	_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0, 1);
	if (_res->_currentScreenResourceNum == 2) {
		if (_g->_levelCheckpoint == 0) {
			_g->_levelCheckpoint = 1;
		}
	}
}

void Level_lar2::preScreenUpdate_lar2_screen4() {
	if (_res->_currentScreenResourceNum == 4) {
		if (_g->_levelCheckpoint == 1) {
			_g->_levelCheckpoint = 2;
		}
		if (_g->_levelCheckpoint >= 2) {
			Game::_lar2_unkData0[4] &= 0xF;
			if (_g->_levelCheckpoint == 8) {
				Game::_lar2_unkData0[8] &= 0xF;
				if (!_paf->_skipCutscenes) {
					_paf->preload(18);
				}
			}
		}
	}
}

void Level_lar2::preScreenUpdate_lar2_screen5() {
	if (_res->_currentScreenResourceNum == 5) {
		if (_g->_levelCheckpoint == 7) {
			Game::_lar2_unkData0[0xC] = (Game::_lar2_unkData0[4] & 0xF) | 0x10;
		} else if (_g->_levelCheckpoint >= 3) {
			Game::_lar2_unkData0[0xC] &= 0xF;
		}
	}
}

void Level_lar2::preScreenUpdate_lar2_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		if (_g->_levelCheckpoint == 2) {
			_g->_levelCheckpoint = 3;
		}
		if (!_paf->_skipCutscenes) {
			if (_g->_levelCheckpoint == 3) {
				_paf->preload(15);
				Game::_lar2_unkData0[0xC] &= 0xF;
			} else if (_g->_levelCheckpoint == 6) {
				_paf->preload(17);
			}
		}
	}
}

void Level_lar2::preScreenUpdate_lar2_screen7() {
	if (_res->_currentScreenResourceNum == 7) {
		if (_g->_levelCheckpoint >= 4 && _g->_levelCheckpoint < 7) {
			_res->_screensState[7].s0 = 1;
		} else {
			_res->_screensState[7].s0 = 0;
		}
		if (_g->_levelCheckpoint == 5) {
			if (!_paf->_skipCutscenes) {
				_paf->preload(16);
			}
			if (_res->_screensState[7].s0 != 0) {
				_res->_resLvlScreenBackgroundDataTable[7].currentBackgroundId = 1;
				_res->_resLvlScreenBackgroundDataTable[7].currentMaskId = 1;
			} else {
				_res->_resLvlScreenBackgroundDataTable[7].currentBackgroundId = 0;
				_res->_resLvlScreenBackgroundDataTable[7].currentMaskId = 0;
			}
		}
	}
}

void Level_lar2::preScreenUpdate_lar2_screen8() {
	if (_res->_currentScreenResourceNum == 8) {
		if (Game::_lar2_unkData0[0x1E] > 1) {
			Game::_lar2_unkData0[0x1E] = 1;
		}
		LvlObject *o = _g->findLvlObject(2, 0, 8);
		_g->postScreenUpdate_lar1_helper(o, Game::_lar2_unkData0 + 0x1C, 7);
	}
}

void Level_lar2::preScreenUpdate_lar2_screen9() {
	if (_res->_currentScreenResourceNum == 9) {
		Game::_lar2_unkData0[0x1E] = 0x24;
	}
}

void Level_lar2::preScreenUpdate_lar2_screen15() {
	if (_res->_currentScreenResourceNum == 15) {
		if (_g->_levelCheckpoint == 9) {
			_g->_levelCheckpoint = 10;
		}
	}
}

void Level_lar2::preScreenUpdate_lar2_screen19() {
	if (_res->_currentScreenResourceNum == 19) {
		if (_g->_levelCheckpoint == 10) {
			if (!_paf->_skipCutscenes) {
				_paf->preload(19);
			}
		}
	}
}

void Level_lar2::preScreenUpdate(int num) {
	switch (num) {
	case 2:
		preScreenUpdate_lar2_screen2();
		break;
	case 4:
		preScreenUpdate_lar2_screen4();
		break;
	case 5:
		preScreenUpdate_lar2_screen5();
		break;
	case 6:
		preScreenUpdate_lar2_screen6();
		break;
	case 7:
		preScreenUpdate_lar2_screen7();
		break;
	case 8:
	case 11:
		preScreenUpdate_lar2_screen8();
		break;
	case 9:
		preScreenUpdate_lar2_screen9();
		break;
	case 15:
		preScreenUpdate_lar2_screen15();
		break;
	case 19:
		preScreenUpdate_lar2_screen19();
		break;
	}
}

void Level_lar2::tick() {
	_g->updateLevelTick_lar(13, _lar2_unkData3, Game::_lar2_bboxData);
}

void Level_lar2::setupLvlObjects_lar2_screen19() {
	int num1 = _lar2_lutData[_g->_levelCheckpoint * 3 + 1];
	for (int i = num1; i < 13; ++i) {
		_lar2_unkData3[i * 4 + 1] = (_lar2_unkData3[i * 4 + 1] & ~0x40) | 1;
	}
	for (int i = num1; i != 0; --i) {
		_lar2_unkData3[i * 4 + 1] = (_lar2_unkData3[i * 4 + 1] & ~1) | 0x40;
	}
	int num2 = _lar2_lutData[_g->_levelCheckpoint * 3];
	for (int i = num2; i < 10; ++i) {
// 408CBC
		const int num = i;
		Game::_lar2_unkData0[num * 4] = (byte_4528F8[num] << 4) | 2;
		const uint32_t mask = 1 << num;
		if (Game::_lar2_unkData0[num * 4] & 0xF0) {
			_g->_mstAndyVarMask &= ~mask;
		} else {
			_g->_mstAndyVarMask |= mask;
		}
		_g->_mstHelper1TestValue |= mask;
	}
	for (int i = num2; i != 0; --i) {
// 408D13
		const int num = i - 1;
		Game::_lar2_unkData0[num * 4] = ((byte_4528F8[num] == 0) ? 16 : 0) | 2;
		const uint32_t mask = 1 << num;
		if (byte_4525F8[num * 4] & 0xF0) {
			_g->_mstAndyVarMask &= ~mask;
		} else {
			_g->_mstAndyVarMask |= mask;
		}
		_g->_mstHelper1TestValue |= mask;
	}
}

void Level_lar2::setupLvlObjects(int num) {
	switch (num) {
	case 19:
		setupLvlObjects_lar2_screen19();
		break;
	}
}
