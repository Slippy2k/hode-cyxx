
// lava_hod

#include "game.h"
#include "lzw.h"
#include "paf.h"
#include "util.h"
#include "video.h"

void Game::postScreenUpdate_lava_helper(int yPos) {
	const uint8_t flags = (_andyObject->flags0) & 0x1F;
	if (!_hideAndyObjectSprite) {
		if ((_mstFlags & 0x80000000) == 0) {
			uint8_t mask = 0;
			const int y = _andyObject->yPos;
			if (_andyObject->posTable[5].y + y >= yPos || _andyObject->posTable[4].y + y >= yPos) {
				mask = 0xA3;
			}
			if (flags == 2 || _andyObject->posTable[7].y + y >= yPos) {
				mask = 0xA3;
			}
			if (mask != 0 && _actionDirectionKeyMaskIndex > 0) {
				_actionDirectionKeyMaskIndex = mask;
				_actionDirectionKeyMaskCounter = 0;
			}
		} else if (flags == 0xB) {
			_mstFlags &= 0x7FFFFFFF;
		}
	}
}

void Game::postScreenUpdate_lava_screen0() {
	switch (_res->_screensState[0].s0) {
	case 2:
		++_screenCounterTable[0];
		if (_screenCounterTable[0] >= 11) {
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

void Game::postScreenUpdate_lava_screen4() {
	if (_res->_currentScreenResourceNum == 4) {
		postScreenUpdate_lava_helper(175);
	}
}

void Game::postScreenUpdate_lava_screen5() {
	if (_res->_currentScreenResourceNum == 5) {
		postScreenUpdate_lava_helper(175);
	}
}

void Game::postScreenUpdate_lava_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		postScreenUpdate_lava_helper(175);
	}
}

void Game::postScreenUpdate_lava_screen7() {
	if (_res->_currentScreenResourceNum == 7) {
		if (_levelCheckpoint == 2) {
			BoundingBox b = { 104, 0, 239, 50 };
                        AndyLvlObjectData *data = (AndyLvlObjectData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
                        if (clipBoundingBox(&b, &data->boundingBox)) {
				_levelCheckpoint = 3;
			}
		}
		postScreenUpdate_lava_helper(175);
	}
}

void Game::postScreenUpdate_lava_screen8() {
	if (_res->_currentScreenResourceNum == 8) {
		if (_andyObject->xPos + _andyObject->posTable[5].x < 72 || _andyObject->xPos + _andyObject->posTable[4].x < 72) {
			const uint8_t flags = _andyObject->flags0 & 0x1F;
			if (flags != 3 && flags != 7 && flags != 4) {
				postScreenUpdate_lava_helper(175);
			}
		}
	}
}

void Game::postScreenUpdate_lava_screen10() {
	if (_res->_currentScreenResourceNum == 10) {
		if (_screenCounterTable[10] < 37) {
			if (_andyObject->yPos + _andyObject->posTable[3].y < 142) {
				_andyObject->actionKeyMask = 0x40;
				_andyObject->directionKeyMask = 0;
				if (_levelCheckpoint == 3) {
					_levelCheckpoint = 4;
					_res->_screensState[10].s0 = 1;
					_res->_resLvlScreenBackgroundDataTable[10].currentMaskId = 1;
					setupScreenMask(10);
				}
				++_screenCounterTable[10];
				if (_screenCounterTable[10] == 13) {
					_fadePaletteCounter = 12;
				} else {
					++_screenCounterTable[10];
					if (_screenCounterTable[10] == 37) {
						if (!_paf->_skipCutscenes) {
							_paf->play(7);
							_paf->unload(7);
							_video->clearPalette();
							updateScreen(_andyObject->screenNum);
						}
					}
				}
			}
		}
	}
}

void Game::postScreenUpdate_lava_screen11() {
	if (_res->_currentScreenResourceNum == 11) {
		postScreenUpdate_lava_helper(175);
	}
}

void Game::postScreenUpdate_lava_screen12() {
	if (_res->_currentScreenResourceNum == 12) {
		postScreenUpdate_lava_helper(175);
	}
}

void Game::postScreenUpdate_lava_screen13() {
	if (_res->_currentScreenResourceNum == 13) {
		postScreenUpdate_lava_helper(175);
	}
}

void Game::postScreenUpdate_lava_screen14() {
	if (_res->_currentScreenResourceNum == 14) {
		const int x = _andyObject->xPos;
		const Point16_t *pos = _andyObject->posTable;
		if (x + pos[5].x < 114 || x + pos[4].x < 114 || x + pos[3].x < 114 || x + pos[0].x < 114) {
			postScreenUpdate_lava_helper(175);
		}
	}
}

void Game::postScreenUpdate_lava_screen15() {
	if (_res->_screensState[0].s0 != 0) {
		if (!_paf->_skipCutscenes) {
			_paf->play(8);
			_paf->unload(8);
		}
		_video->clearPalette();
		_quit = true;
	}
}

void Game::callLevel_postScreenUpdate_lava(int num) {
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

void Game::preScreenUpdate_lava_screen0() {
	if (_res->_screensState[0].s0 != 0) {
		_res->_screensState[0].s0 = 1;
	}
}

void Game::preScreenUpdate_lava_screen3() {
	if (_res->_currentScreenResourceNum == 3) {
		if (_levelCheckpoint == 0) {
			_levelCheckpoint = 1;
		}
	}
}

void Game::preScreenUpdate_lava_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		if (_levelCheckpoint == 1) {
			_levelCheckpoint = 2;
		}
	}
}

void Game::preScreenUpdate_lava_screen10() {
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

void Game::preScreenUpdate_lava_screen13() {
	if (_res->_currentScreenResourceNum == 13) {
		if (_levelCheckpoint == 4) {
			_levelCheckpoint = 5;
		}
	}
}

void Game::preScreenUpdate_lava_screen15() {
	if (_res->_screensState[15].s0 == 0) {
		if (!_paf->_skipCutscenes) {
			_paf->preload(8);
		}
	}
}

void Game::callLevel_preScreenUpdate_lava(int num) {
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

void Game::callLevel_initialize_lava() {
	_transformShadowBuffer = (uint8_t *)malloc(256 * 192 + 256);
	const int size = decodeLZW(_pwr2_screenTransformData, _transformShadowBuffer);
	assert(size == 256 * 192);
	memcpy(_transformShadowBuffer + 256 * 192, _transformShadowBuffer, 256);
	resetLevelTickHelperData();
}

static const uint8_t byte_452CD8[] = {
	0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void Game::callLevel_tick_lava() {
	_video->_displayShadowLayer = byte_452CD8[_res->_currentScreenResourceNum * 2];
	// TODO
	updateLevelTickHelper();
}

void Game::setupLvlObjects_lava_screen3() {
	LvlObject *ptr = findLvlObject(2, 0, 3);
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

void Game::callLevel_setupLvlObjects_lava(int num) {
	switch (num) {
	case 3:
		setupLvlObjects_lava_screen3();
		break;
	}
}
