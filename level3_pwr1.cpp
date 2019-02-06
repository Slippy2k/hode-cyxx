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
	case 10:
	case 12:
	case 14:
	case 16:
	case 18:
	case 23:
	case 27:
		warning("callLevel_postScreenUpdate_pwr1 %d unimplemented", num);
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

static const uint8_t byte_454520[] = {
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0,
	1, 2, 1, 0, 1, 3, 1, 0, 1, 0, 1, 4, 1, 0, 1, 0, 1, 0,
	1, 5, 1, 0, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0,
	0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void Game::callLevel_tick_pwr1() {
	// TODO:
	_video->_displayShadowLayer = byte_454520[_res->_currentScreenResourceNum * 2];
	warning("callLevel_tick_pwr1 unimplemented");
}
