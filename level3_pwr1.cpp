/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

// pwr1_hod

#include "game.h"
#include "lzw.h"
#include "paf.h"
#include "util.h"
#include "video.h"

void Game::postScreenUpdate_pwr1_helper(BoundingBox *b, int dx, int dy) {
	LvlObject *o = 0; //_unkLvlObject;
	while (o) {
		if (o->data0x2988 == 4 && o->anim <= 9) {
			BoundingBox b2;
			b2.x1 = o->posTable[3].x + o->xPos - 1;
			b2.x2 = o->posTable[3].x + o->xPos + 1;
			b2.y1 = o->posTable[3].y + o->yPos - 1;
			b2.y2 = o->posTable[3].y + o->yPos + 1;
			if (clipBoundingBox(b, &b2)) {
				o->xPos += dx;
				o->yPos += dy;
			}
		}
		o = o->nextPtr;
	}
	BoundingBox b2;
	b2.x1 = _andyObject->posTable[3].x + _andyObject->xPos - 1;
	b2.x2 = _andyObject->posTable[3].x + _andyObject->xPos + 1;
	b2.y1 = _andyObject->posTable[3].y + _andyObject->yPos - 1;
	b2.y2 = _andyObject->posTable[3].y + _andyObject->yPos + 1;
	if (clipBoundingBox(b, &b2)) {
		AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		data->dxPos += dx;
		data->dyPos += dy;
	}
}

void Game::postScreenUpdate_pwr1_screen10() {
	if (_res->_currentScreenResourceNum == 10) {
		BoundingBox b1 = { 50, 0, 100, 122 };
		postScreenUpdate_pwr1_helper(&b1, 0, 2);
		BoundingBox b2 = { 58, 50, 84, 132 };
		postScreenUpdate_pwr1_helper(&b2, 0, 2);
		BoundingBox b3 = { 42, 80, 72, 142 };
		postScreenUpdate_pwr1_helper(&b3, 4, 0);
		BoundingBox b4 = { 73, 100, 94, 142 };
		postScreenUpdate_pwr1_helper(&b4, -2, 0);
	}
}

void Game::postScreenUpdate_pwr1_screen14() {
	if (_res->_currentScreenResourceNum == 14) {
		BoundingBox b1 = { 0, 136, 104, 191 };
		postScreenUpdate_pwr1_helper(&b1, 2, 0);
		BoundingBox b2 = { 0, 152, 112, 178 };
		postScreenUpdate_pwr1_helper(&b2, 2, 0);
		BoundingBox b3 = { 76, 148, 116, 164 };
		postScreenUpdate_pwr1_helper(&b3, 0, 2);
		BoundingBox b4 = { 76, 164, 116, 186 };
		postScreenUpdate_pwr1_helper(&b4, 0, -2);
	}
}

void Game::postScreenUpdate_pwr1_screen16() {
	if (_res->_currentScreenResourceNum == 16) {
		BoundingBox b1 = { 100, 130, 160, 176 };
		postScreenUpdate_pwr1_helper(&b1, 2, 0);
		BoundingBox b2 = { 88, 140, 180, 167 };
		postScreenUpdate_pwr1_helper(&b2, 2, 0);
		BoundingBox b3 = { 88, 82, 185, 120 };
		postScreenUpdate_pwr1_helper(&b3, 2, 0);
		BoundingBox b4 = { 120, 92, 190, 114 };
		postScreenUpdate_pwr1_helper(&b4, 2, 0);
		BoundingBox b5 = { 104, 40, 147, 120 };
		postScreenUpdate_pwr1_helper(&b5, 0, -1);
		BoundingBox b6 = { 115, 24, 138, 90 };
		postScreenUpdate_pwr1_helper(&b6, 0, -1);
	}
}

void Game::postScreenUpdate_pwr1_screen18() {
	if (_res->_currentScreenResourceNum == 18) {
		if (_levelCheckpoint == 3) {
			BoundingBox b = { 0, 0, 123, 125 };
			AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (clipBoundingBox(&b, &data->boundingBox)) {
				++_levelCheckpoint;
			}
		}
		BoundingBox b1 = { 156, 80, 204, 144 };
		postScreenUpdate_pwr1_helper(&b1, 0, -2);
		BoundingBox b2 = { 166, 88, 194, 152 };
		postScreenUpdate_pwr1_helper(&b2, 0, -2);
	}
}

void Game::postScreenUpdate_pwr1_screen35() {
	if (_res->_currentScreenResourceNum == 35) {
		AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		BoundingBox b = { 0, 0, 193, 88 };
		if (clipBoundingBox(&b, &data->boundingBox)) {
			_andyObject->actionKeyMask &= ~3;
			_andyObject->directionKeyMask &= ~4;
			_andyObject->actionKeyMask |= 8;
		}
		if (_res->_screensState[35].s0 != 0) {
			++_screenCounterTable[35];
			if (_screenCounterTable[35] == 46) {
				if (!_paf->_skipCutscenes) {
					_paf->play(5);
					_paf->unload(5);
				}
				_video->clearPalette();
				_quit = true;
			}
		}
	}
}

void Game::callLevel_postScreenUpdate_pwr1(int num) {
	switch (num) {
	case 6:
	case 12:
	case 23:
	case 27:
		warning("callLevel_postScreenUpdate_pwr1 %d unimplemented", num);
		break;
	case 10:
		postScreenUpdate_pwr1_screen10();
		break;
	case 14:
		postScreenUpdate_pwr1_screen14();
		break;
	case 16:
		postScreenUpdate_pwr1_screen16();
		break;
	case 18:
		postScreenUpdate_pwr1_screen18();
		break;
	case 35:
		postScreenUpdate_pwr1_screen35();
		break;
	}
}

void Game::preScreenUpdate_pwr1_screen4() {
        if (_res->_currentScreenResourceNum == 4) {
		const uint8_t num = (_res->_screensState[4].s0 == 0) ? 0 : 1;
		_res->_resLvlScreenBackgroundDataTable[4].currentBackgroundId = num;
		_res->_resLvlScreenBackgroundDataTable[4].unk3 = num;
	}
}

void Game::preScreenUpdate_pwr1_screen6() {
        if (_res->_currentScreenResourceNum == 6 || _res->_currentScreenResourceNum == 5) {
		if (_res->_screensState[6].s0 == 0) {
			if (_levelCheckpoint != 1) {
				_screenCounterTable[6] = 0;
				_res->_resLvlScreenBackgroundDataTable[6].currentBackgroundId = 0;
				_res->_resLvlScreenBackgroundDataTable[6].unk3 = 0;
				_res->_screensState[6].s0 = 0;
			} else {
				_screenCounterTable[6] = 41;
				_res->_resLvlScreenBackgroundDataTable[6].currentBackgroundId = 1;
				_res->_resLvlScreenBackgroundDataTable[6].unk3 = 1;
				_res->_screensState[6].s0 = 1;
			}
		} else {
			_screenCounterTable[6] = 54;
			_res->_resLvlScreenBackgroundDataTable[6].currentBackgroundId = 2;
			_res->_resLvlScreenBackgroundDataTable[6].unk3 = 2;
			_res->_screensState[6].s0 = 2;
		}
        }
}

void Game::preScreenUpdate_pwr1_screen9() {
	if (_res->_currentScreenResourceNum == 9) {
		if (_levelCheckpoint == 1) {
			_levelCheckpoint = 2;
		}
	}
}

void Game::preScreenUpdate_pwr1_screen15() {
	if (_res->_currentScreenResourceNum == 15) {
		if (_levelCheckpoint == 2) {
			_levelCheckpoint = 3;
		}
	}
}

void Game::preScreenUpdate_pwr1_screen21() {
	if (_res->_currentScreenResourceNum == 15) {
		if (_levelCheckpoint == 4) {
			_levelCheckpoint = 5;
		}
	}
}

void Game::preScreenUpdate_pwr1_screen23() {
	if (_res->_currentScreenResourceNum == 23 || _res->_currentScreenResourceNum == 26) {
		const uint8_t num = _res->_screensState[23].s0 != 0 ? 1 : 0;
		_res->_resLvlScreenBackgroundDataTable[23].currentBackgroundId = num;
		_res->_resLvlScreenBackgroundDataTable[23].unk3 = num;
	}
}

void Game::preScreenUpdate_pwr1_screen24() {
	if (_res->_currentScreenResourceNum == 24) {
		if (_res->_screensState[27].s0 != 0) { // +0x6C
			if (_levelCheckpoint == 6) {
				_levelCheckpoint = 7;
			}
		}
	}
}

void Game::preScreenUpdate_pwr1_screen26() {
	if (_levelCheckpoint >= 7) {
		_res->_screensState[23].s0 = 1;
	}
	if (_res->_currentScreenResourceNum == 23 || _res->_currentScreenResourceNum == 26) {
		const uint8_t num = _res->_screensState[23].s0 != 0 ? 1 : 0;
		_res->_resLvlScreenBackgroundDataTable[23].currentBackgroundId = num;
		_res->_resLvlScreenBackgroundDataTable[23].unk3 = num;
	}
}

void Game::callLevel_preScreenUpdate_pwr1(int num) {
	switch (num) {
	case 4:
		preScreenUpdate_pwr1_screen4();
		break;
	case 6:
		preScreenUpdate_pwr1_screen6();
		break;
	case 9:
		preScreenUpdate_pwr1_screen9();
		break;
	case 15:
		preScreenUpdate_pwr1_screen15();
		break;
	case 21:
		preScreenUpdate_pwr1_screen21();
		break;
	case 23:
		preScreenUpdate_pwr1_screen23();
		break;
	case 24:
		preScreenUpdate_pwr1_screen24();
		break;
	case 26:
		preScreenUpdate_pwr1_screen26();
		break;
	case 27:
	case 29:
	case 31:
	case 35:
		warning("callLevel_preScreenUpdate_level3 %d unimplemented", num);
		break;
	}
}

void Game::callLevel_initialize_pwr1() {
	_transformShadowBuffer = (uint8_t *)malloc(256 * 192 + 256);
	const int size = decodeLZW(_transformBufferData1, _transformShadowBuffer);
	assert(size == 256 * 192);
	memcpy(_transformShadowBuffer + 256 * 192, _transformShadowBuffer, 256);
	// TODO:
	warning("callLevel_initialize_pwr1 unimplemented");
}

const uint8_t Game::_pwr1_screenTransformLut[] = {
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0,
	1, 2, 1, 0, 1, 3, 1, 0, 1, 0, 1, 4, 1, 0, 1, 0, 1, 0,
	1, 5, 1, 0, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0,
	0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void Game::callLevel_tick_pwr1() {
	// TODO:
	_video->_displayShadowLayer = _pwr1_screenTransformLut[_res->_currentScreenResourceNum * 2];
	warning("callLevel_tick_pwr1 unimplemented");
}
