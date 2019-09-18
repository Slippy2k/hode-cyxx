
// lava_hod

#include "game.h"
#include "level.h"
#include "paf.h"
#include "util.h"
#include "video.h"

struct Level_lava: Level {
	//const CheckpointData *getCheckpointData() = 0;

	virtual void initialize();
	virtual void terminate();
	virtual void tick();
	virtual void preScreenUpdate(int screenNum);
	virtual void postScreenUpdate(int screenNum);
	virtual void setupLvlObjects(int screenNum);

	void postScreenUpdate_lava_screen0();
	void postScreenUpdate_lava_screen4();
	void postScreenUpdate_lava_screen5();
	void postScreenUpdate_lava_screen6();
	void postScreenUpdate_lava_screen7();
	void postScreenUpdate_lava_screen8();
	void postScreenUpdate_lava_screen10();
	void postScreenUpdate_lava_screen11();
	void postScreenUpdate_lava_screen12();
	void postScreenUpdate_lava_screen13();
	void postScreenUpdate_lava_screen14();
	void postScreenUpdate_lava_screen15();

	void preScreenUpdate_lava_screen0();
	void preScreenUpdate_lava_screen3();
	void preScreenUpdate_lava_screen6();
	void preScreenUpdate_lava_screen10();
	void preScreenUpdate_lava_screen13();
	void preScreenUpdate_lava_screen15();

	void setupLvlObjects_lava_screen3();
};

Level *Level_lava_create() {
	return new Level_lava;
}

void Level_lava::postScreenUpdate_lava_screen0() {
	switch (_res->_screensState[0].s0) {
	case 2:
		++_g->_screenCounterTable[0];
		if (_g->_screenCounterTable[0] >= 11) {
			_res->_screensState[0].s0 = 1;
		}
		break;
	case 0:
		if (_andyObject->anim == 2 && _andyObject->frame > 2) {
			_res->_screensState[0].s0 = 2;
		}
		break;
	}
}

void Level_lava::postScreenUpdate_lava_screen4() {
	if (_res->_currentScreenResourceNum == 4) {
		_g->postScreenUpdate_lava_helper(175);
	}
}

void Level_lava::postScreenUpdate_lava_screen5() {
	if (_res->_currentScreenResourceNum == 5) {
		_g->postScreenUpdate_lava_helper(175);
	}
}

void Level_lava::postScreenUpdate_lava_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		_g->postScreenUpdate_lava_helper(175);
	}
}

void Level_lava::postScreenUpdate_lava_screen7() {
	if (_res->_currentScreenResourceNum == 7) {
		if (_g->_levelCheckpoint == 2) {
			BoundingBox b = { 104, 0, 239, 50 };
                        AndyLvlObjectData *data = (AndyLvlObjectData *)_g->getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
                        if (_g->clipBoundingBox(&b, &data->boundingBox)) {
				_g->_levelCheckpoint = 3;
			}
		}
		_g->postScreenUpdate_lava_helper(175);
	}
}

void Level_lava::postScreenUpdate_lava_screen8() {
	if (_res->_currentScreenResourceNum == 8) {
		if (_andyObject->xPos + _andyObject->posTable[5].x < 72 || _andyObject->xPos + _andyObject->posTable[4].x < 72) {
			const uint8_t flags = _andyObject->flags0 & 0x1F;
			if (flags != 3 && flags != 7 && flags != 4) {
				_g->postScreenUpdate_lava_helper(175);
			}
		}
	}
}

void Level_lava::postScreenUpdate_lava_screen10() {
	if (_res->_currentScreenResourceNum == 10) {
		if (_g->_screenCounterTable[10] < 37) {
			if (_andyObject->yPos + _andyObject->posTable[3].y < 142) {
				_andyObject->actionKeyMask = 0x40;
				_andyObject->directionKeyMask = 0;
				if (_g->_levelCheckpoint == 3) {
					_g->_levelCheckpoint = 4;
					_res->_screensState[10].s0 = 1;
					_res->_resLvlScreenBackgroundDataTable[10].currentMaskId = 1;
					_g->setupScreenMask(10);
				}
				++_g->_screenCounterTable[10];
				if (_g->_screenCounterTable[10] == 13) {
					_g->_levelRestartCounter = 12;
				} else {
					++_g->_screenCounterTable[10];
					if (_g->_screenCounterTable[10] == 37) {
						if (!_paf->_skipCutscenes) {
							_paf->play(7);
							_paf->unload(7);
							_video->clearPalette();
							_g->updateScreen(_andyObject->screenNum);
						}
					}
				}
			}
		}
	}
}

void Level_lava::postScreenUpdate_lava_screen11() {
	if (_res->_currentScreenResourceNum == 11) {
		_g->postScreenUpdate_lava_helper(175);
	}
}

void Level_lava::postScreenUpdate_lava_screen12() {
	if (_res->_currentScreenResourceNum == 12) {
		_g->postScreenUpdate_lava_helper(175);
	}
}

void Level_lava::postScreenUpdate_lava_screen13() {
	if (_res->_currentScreenResourceNum == 13) {
		_g->postScreenUpdate_lava_helper(175);
	}
}

void Level_lava::postScreenUpdate_lava_screen14() {
	if (_res->_currentScreenResourceNum == 14) {
		const int x = _andyObject->xPos;
		const Point16_t *pos = _andyObject->posTable;
		if (x + pos[5].x < 114 || x + pos[4].x < 114 || x + pos[3].x < 114 || x + pos[0].x < 114) {
			_g->postScreenUpdate_lava_helper(175);
		}
	}
}

void Level_lava::postScreenUpdate_lava_screen15() {
	if (_res->_screensState[0].s0 != 0) {
		if (!_paf->_skipCutscenes) {
			_paf->play(8);
			_paf->unload(8);
		}
		_video->clearPalette();
		_g->_endLevel = true;
	}
}

void Level_lava::postScreenUpdate(int num) {
	switch (num) {
	case 0:
		postScreenUpdate_lava_screen0();
		break;
	case 4:
		postScreenUpdate_lava_screen4();
		break;
	case 5:
		postScreenUpdate_lava_screen5();
		break;
	case 6:
		postScreenUpdate_lava_screen6();
		break;
	case 7:
		postScreenUpdate_lava_screen7();
		break;
	case 8:
		postScreenUpdate_lava_screen8();
		break;
	case 10:
		postScreenUpdate_lava_screen10();
		break;
	case 11:
		postScreenUpdate_lava_screen11();
		break;
	case 12:
		postScreenUpdate_lava_screen12();
		break;
	case 13:
		postScreenUpdate_lava_screen13();
		break;
	case 14:
		postScreenUpdate_lava_screen14();
		break;
	case 15:
		postScreenUpdate_lava_screen15();
		break;
	}
}

void Level_lava::preScreenUpdate_lava_screen0() {
	if (_res->_screensState[0].s0 != 0) {
		_res->_screensState[0].s0 = 1;
	}
}

void Level_lava::preScreenUpdate_lava_screen3() {
	if (_res->_currentScreenResourceNum == 3) {
		if (_g->_levelCheckpoint == 0) {
			_g->_levelCheckpoint = 1;
		}
	}
}

void Level_lava::preScreenUpdate_lava_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		if (_g->_levelCheckpoint == 1) {
			_g->_levelCheckpoint = 2;
		}
	}
}

void Level_lava::preScreenUpdate_lava_screen10() {
	const int num = (_res->_screensState[10].s0 == 0) ? 0 : 1;
	if (_res->_screensState[10].s0 != 0 && _res->_screensState[10].s0 != 1) {
		_res->_screensState[10].s0 = 1;
	}
	if (_res->_currentScreenResourceNum == 10) {
		_res->_resLvlScreenBackgroundDataTable[10].currentMaskId = num;
		if (!_paf->_skipCutscenes) {
			_paf->preload(7);
		}
	}
}

void Level_lava::preScreenUpdate_lava_screen13() {
	if (_res->_currentScreenResourceNum == 13) {
		if (_g->_levelCheckpoint == 4) {
			_g->_levelCheckpoint = 5;
		}
	}
}

void Level_lava::preScreenUpdate_lava_screen15() {
	if (_res->_screensState[15].s0 == 0) {
		if (!_paf->_skipCutscenes) {
			_paf->preload(8);
		}
	}
}

void Level_lava::preScreenUpdate(int num) {
	switch (num) {
	case 1:
		preScreenUpdate_lava_screen0();
		break;
	case 3:
		preScreenUpdate_lava_screen3();
		break;
	case 6:
		preScreenUpdate_lava_screen6();
		break;
	case 10:
		preScreenUpdate_lava_screen10();
		break;
	case 13:
		preScreenUpdate_lava_screen13();
		break;
	case 15:
		preScreenUpdate_lava_screen15();
		break;
	}
}

static LvlObject *findLvlObject_lava(LvlObject *o) {
	LvlObject *cur = o->nextPtr;
	while (cur) {
		if (o->type == cur->type && o->spriteNum == cur->spriteNum && o->screenNum == cur->screenNum) {
			return cur;
		}
		cur = cur->nextPtr;
	}
	return 0;
}

void Level_lava::initialize() {
	_g->loadTransformLayerData(Game::_pwr2_screenTransformData);
	_g->resetCrackSprites();
}

void Level_lava::terminate() {
	_g->unloadTransformLayerData();
}

const uint8_t Game::_lava_screenTransformLut[] = {
	0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void Level_lava::tick() {
	_video->_displayShadowLayer = Game::_lava_screenTransformLut[_res->_currentScreenResourceNum * 2] != 0;
	assert(Game::_lava_screenTransformLut[_res->_currentScreenResourceNum * 2 + 1] == 0);
	_g->restoreAndyCollidesLava();
	_g->updateCrackSprites();
}

void Level_lava::setupLvlObjects_lava_screen3() {
	LvlObject *ptr = _g->findLvlObject(2, 0, 3);
	assert(ptr);
	ptr->flags0 = 0xFC00;
	ptr->xPos = 138;
	ptr->yPos = 157;
	ptr->anim = 0;
	ptr->frame = 0;
	ptr->directionKeyMask = 0;
	ptr = findLvlObject_lava(ptr);
	assert(ptr);
	ptr->flags0 = 0xFC00;
	ptr->anim = 0;
	ptr->frame = 0;
	ptr->directionKeyMask = 0;
	ptr->xPos = 66;
	ptr->yPos = 157;
}

void Level_lava::setupLvlObjects(int num) {
	switch (num) {
	case 3:
		setupLvlObjects_lava_screen3();
		break;
	}
}
