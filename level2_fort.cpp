/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir (cyx@users.sourceforge.net)
 */

// fort_hod

#include "game.h"
#include "level.h"
#include "paf.h"
#include "util.h"
#include "video.h"

struct Level_fort: Level {
        //const CheckpointData *getCheckpointData() = 0;

	//virtual void initialize();
	//virtual void terminate();
	virtual void tick();
	virtual void preScreenUpdate(int screenNum);
	virtual void postScreenUpdate(int screenNum);
	virtual void setupLvlObjects(int screenNum);

	void postScreenUpdate_fort_screen1();
	void postScreenUpdate_fort_screen6();
	void postScreenUpdate_fort_screen7();
	void postScreenUpdate_fort_screen8();
	void postScreenUpdate_fort_screen16();
	void postScreenUpdate_fort_screen17();
	void postScreenUpdate_fort_screen21();

	void preScreenUpdate_fort_screen1();
	void preScreenUpdate_fort_screen2();
	void preScreenUpdate_fort_screen6();
	void preScreenUpdate_fort_screen9();
	void preScreenUpdate_fort_screen14();
	void preScreenUpdate_fort_screen16();
	void preScreenUpdate_fort_screen17();
	void preScreenUpdate_fort_screen21();

	void setupLvlObjects_fort_screen1();
};

Level *Level_fort_create() {
	return new Level_fort;
}

void Level_fort::postScreenUpdate_fort_screen1() {
	if (_res->_screensState[1].s0 == 2) {
		LvlObject *o = _g->findLvlObject(2, 0, 1);
		if (o) {
			o->actionKeyMask = 1;
		}
		++_g->_screenCounterTable[1];
		if (_g->_screenCounterTable[1] == 25) {
			_res->_resLvlScreenBackgroundDataTable[1].currentMaskId = 1;
			_g->setupScreenMask(1);
		} else if (_g->_screenCounterTable[1] == 59) {
			_res->_screensState[1].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[1].currentBackgroundId = 1;
		}
	}
}

void Level_fort::postScreenUpdate_fort_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		BoundingBox b = { 0, 0, 43, 191 };
		if (!_g->clipBoundingBox(&b, &data->boundingBox)) {
			_andyObject->actionKeyMask &= ~3;
			_andyObject->actionKeyMask |= 8;
			if (_andyObject->directionKeyMask & 4) {
				_andyObject->directionKeyMask &= ~10;
				++_g->_screenCounterTable[6];
				if (_g->_screenCounterTable[6] >= 188) {
					_andyObject->directionKeyMask &= ~4;
					_g->_screenCounterTable[7] = 0;
				}
			}
			if (_g->_screenCounterTable[7] < 25) {
				++_g->_screenCounterTable[7];
				_andyObject->directionKeyMask &= ~4;
			} else if ((_andyObject->directionKeyMask & 4) == 0) {
				_g->_screenCounterTable[6] = 0;
			}
		}
	}
}

void Level_fort::postScreenUpdate_fort_screen7() {
	if (_res->_currentScreenResourceNum == 7) {
		_andyObject->actionKeyMask &= ~3;
		_andyObject->actionKeyMask |= 8;
		if (_andyObject->directionKeyMask & 4) {
			_andyObject->directionKeyMask &= ~0xA;
			++_g->_screenCounterTable[6];
			if (_g->_screenCounterTable[6] >= 188) {
				_andyObject->directionKeyMask &= ~4;
				_g->_screenCounterTable[7] = 0;
			}
		}
		if (_g->_screenCounterTable[7] < 25) {
			++_g->_screenCounterTable[7];
			_andyObject->directionKeyMask &= ~4;
		} else if ((_andyObject->directionKeyMask & 4) == 0) {
			_g->_screenCounterTable[6] = 0;
		}
	}
}

void Level_fort::postScreenUpdate_fort_screen8() {
	if (_res->_currentScreenResourceNum == 8) {
		AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		BoundingBox b = { 138, 0, 255, 191 };
		if (!_g->clipBoundingBox(&b, &data->boundingBox)) {
			_andyObject->actionKeyMask &= ~3;
			_andyObject->actionKeyMask |= 8;
			if (_andyObject->directionKeyMask & 4) {
				_andyObject->directionKeyMask &= ~0xA;
				++_g->_screenCounterTable[6];
				if (_g->_screenCounterTable[6] >= 188) {
					_andyObject->directionKeyMask &= ~4;
					_g->_screenCounterTable[7] = 0;
				}
			}
			if (_g->_screenCounterTable[7] < 25) {
				++_g->_screenCounterTable[7];
				_andyObject->directionKeyMask &= ~4;
			} else if ((_andyObject->directionKeyMask & 4) == 0) {
				_g->_screenCounterTable[6] = 0;
			}
		}
	}
}

void Level_fort::postScreenUpdate_fort_screen16() {
	if (_res->_currentScreenResourceNum == 16) {
		if (_res->_screensState[16].s0 == 1) {
			AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			BoundingBox b = { 150, 0, 187, 60 };
                	if (!_g->clipBoundingBox(&b, &data->boundingBox)) {
				return;
			}
			if ((_andyObject->flags0 & 0x1F) == 0 && (_andyObject->flags0 & 0xE0) == 0xE0) {
				_res->_screensState[16].s0 = 1;
				_g->_screenCounterTable[16] = 0;
			} else {
				_g->setAndySpecialAnimation(3);
			}
		} else {
			++_g->_screenCounterTable[16];
			if (_g->_screenCounterTable[16] == 5) {
				if (_g->_levelCheckpoint == 3) {
					_g->_levelCheckpoint = 4;
				}
				if (!_paf->_skipCutscenes) {
					_paf->play(3);
					_paf->unload(3);
				}
				_video->clearPalette();
				_g->restartLevel();
			}
		}
	}
}

void Level_fort::postScreenUpdate_fort_screen17() {
	if (_res->_screensState[17].s0 == 1) {
		++_g->_screenCounterTable[17];
		if (_g->_screenCounterTable[17] == 68) {
			_g->_screenCounterTable[17] = 0;
		}
	}
}

void Level_fort::postScreenUpdate_fort_screen21() {
	if (_res->_currentScreenResourceNum == 21) {
		switch (_res->_screensState[21].s0) {
		case 1:
			++_g->_screenCounterTable[21];
			if (_g->_screenCounterTable[21] == 22) {
				_res->_screensState[21].s0 = 2;
			}
			break;
		case 2:
			if (!_paf->_skipCutscenes) {
				_paf->play(4);
				_paf->unload(4);
			}
			_video->clearPalette();
			_g->_quit = true;
			break;
		}
	}
}

void Level_fort::postScreenUpdate(int num) {
	switch (num) {
	case 1:
		postScreenUpdate_fort_screen1();
		break;
	case 6:
		postScreenUpdate_fort_screen6();
		break;
	case 7:
		postScreenUpdate_fort_screen7();
		break;
	case 8:
		postScreenUpdate_fort_screen8();
		break;
	case 16:
		postScreenUpdate_fort_screen16();
		break;
	case 17:
		postScreenUpdate_fort_screen17();
		break;
	case 21:
		postScreenUpdate_fort_screen21();
		break;
	}
}

void Level_fort::preScreenUpdate_fort_screen1() {
	if (_res->_currentScreenResourceNum == 1 && _g->_levelCheckpoint >= 1) {
		_res->_screensState[1].s0 = 1;
	}
	const int num = _res->_screensState[1].s0 == 0 ? 0 : 1;
	_res->_resLvlScreenBackgroundDataTable[1].currentBackgroundId = num;
	_res->_resLvlScreenBackgroundDataTable[1].currentMaskId = num;
}

void Level_fort::preScreenUpdate_fort_screen2() {
	if (_res->_currentScreenResourceNum == 2 && _g->_levelCheckpoint == 0) {
		_g->_levelCheckpoint = 1;
	}
}

void Level_fort::preScreenUpdate_fort_screen6() {
	if (_res->_currentScreenResourceNum == 6 && _g->_levelCheckpoint == 1) {
		_g->_levelCheckpoint = 2;
	}
}

void Level_fort::preScreenUpdate_fort_screen9() {
	if (_res->_currentScreenResourceNum == 9 && _g->_levelCheckpoint == 2) {
		_g->_levelCheckpoint = 3;
	}
}

void Level_fort::preScreenUpdate_fort_screen14() {
	if (_res->_currentScreenResourceNum == 14) {
		_res->_resLvlScreenBackgroundDataTable[14].currentBackgroundId = _res->_screensState[14].s0 != 0 ? 1 : 0;
	}
}

void Level_fort::preScreenUpdate_fort_screen16() {
	if (_res->_currentScreenResourceNum == 16) {
		_res->_screensState[16].s0 = 0;
		_andyObject->xPos += 9;
		if (!_paf->_skipCutscenes) {
			_paf->preload(3);
		}
	}
}

void Level_fort::preScreenUpdate_fort_screen17() {
	if (_res->_currentScreenResourceNum == 17) {
		_res->_screensState[17].s0 = _g->_screenCounterTable[17] == 0 ? 1 : 0;
	}
}

void Level_fort::preScreenUpdate_fort_screen21() {
	if (_res->_currentScreenResourceNum == 21) {
		_res->_screensState[21].s0 = 0;
		if (!_paf->_skipCutscenes) {
			_paf->preload(4);
		}
	}
}

void Level_fort::tick() {
	_g->_plasmaCannonFlags |= 2;
}

void Level_fort::preScreenUpdate(int num) {
	switch (num) {
	case 1:
		preScreenUpdate_fort_screen1();
		break;
	case 2:
		preScreenUpdate_fort_screen2();
		break;
	case 6:
		preScreenUpdate_fort_screen6();
		break;
	case 9:
		preScreenUpdate_fort_screen9();
		break;
	case 14:
		preScreenUpdate_fort_screen14();
		break;
	case 16:
		preScreenUpdate_fort_screen16();
		break;
	case 17:
		preScreenUpdate_fort_screen17();
		break;
	case 21:
		preScreenUpdate_fort_screen21();
		break;
	}
}

void Level_fort::setupLvlObjects_fort_screen1() {
	LvlObject *ptr = _g->findLvlObject(2, 0, 1);
	if (ptr) {
		ptr->xPos = 129;
		ptr->yPos = 97;
		ptr->anim = 0;
		ptr->frame = 0;
		ptr->directionKeyMask = 0;
		ptr->actionKeyMask = 0;
	}
}

void Level_fort::setupLvlObjects(int num) {
	switch (num) {
	case 1:
		setupLvlObjects_fort_screen1();
		break;
	}
}
