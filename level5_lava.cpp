
// lava_hod

#include "game.h"
#include "lzw.h"
#include "paf.h"
#include "video.h"

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
	case 15:
		postScreenUpdate_lava_screen15();
		break;
	}
}

static LvlObject *findLvlObject_lava(LvlObject *o) {
	LvlObject *cur = o->nextPtr;
	while (cur) {
		if (o->type == cur->type && o->data0x2988 == cur->data0x2988 && o->screenNum == cur->screenNum) {
			return cur;
		}
		cur = cur->nextPtr;
	}
	return 0;
}

void Game::callLevel_initialize_lava() {
	_shakeShadowBuffer = (uint8_t *)malloc(256 * 192 + 256);
	decodeLZW(_levelOpStage3ImageData1, _shakeShadowBuffer);
	memcpy(_shakeShadowBuffer + 256 * 192, _shakeShadowBuffer, 256);
}

void Game::setupLvlObjects_lava_screen3() {
	LvlObject *ptr = _res->findLvlObject(2, 0, 3);
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
