
#include "game.h"
#include "paf.h"
#include "util.h"
#include "video.h"

static uint8_t byte_4525F8[] = {  2,  0, 0, 0 };
static uint8_t byte_4525FC[] = { 18,  0, 0, 0 };
static uint8_t byte_452600[] = { 50,  9, 2, 0 };
static uint8_t byte_452604[] = { 50, 14, 2, 0 };
static uint8_t byte_452608[] = {  2,  0, 0, 0 };
static uint8_t byte_45260C[] = {  2,  3, 0, 0 };
static uint8_t byte_452610[] = {  2,  0, 0, 0 };
static uint8_t byte_452614[] = {  2,  0, 0, 0 };
static uint8_t byte_45261C[] = {  2,  0, 0, 0 };
static uint8_t byte_452620[] = {  2,  0, 0, 0 };

void Game::postScreenUpdate_lar1_helper(LvlObject *o, uint8_t *p, int num) {
	// TODO
}

void Game::postScreenUpdate_lar1_screen0() {
	switch (_res->_screensState[0].s0) {
	case 0:
		_res->_screensState[0].s0 = 3;
		break;
	case 1:
		if (_res->_currentScreenResourceNum == 0) {
			// TODO
		}
		break;
	case 2:
		if (_res->_currentScreenResourceNum == 0) {
			BoundingBox b = { 0, 0, 14, 74 };
			AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (clipBoundingBox(&b, &data->boundingBox)) {
				// TODO
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
			setupScreenMask(0);
		} else if (_screenCounterTable[0] >= 45) {
			_res->_screensState[0].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 1;
		} else if (_screenCounterTable[0] == 11) {
			setShakeScreen(3, 2);
		} else if (_screenCounterTable[0] == 13) {
			setShakeScreen(2, 4);
		} else if (_screenCounterTable[0] == 19) {
			setShakeScreen(2, 4);
		} else if (_screenCounterTable[0] == 25) {
			setShakeScreen(2, 6);
		}
		break;
	case 4:
		++_screenCounterTable[0];
		if (_screenCounterTable[0] >= 64) {
			_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 2;
			_res->_resLvlScreenBackgroundDataTable[0].currentMaskId = 2;
			setupScreenMask(0);
			_res->_screensState[0].s0 = 2;
		}
		break;
	}
}

void Game::postScreenUpdate_lar1_screen3() {
	if (_res->_currentScreenResourceNum == 3) {
		BoundingBox b = { 46, 0, 210, 106 };
		postScreenUpdate_rock_helper2(&b, 16);
	}
}

void Game::postScreenUpdate_lar1_screen4() {
	LvlObject *o = findLvlObject(2, 0, 4);
	postScreenUpdate_lar1_helper(o, byte_4525F8, 0);
}

void Game::postScreenUpdate_lar1_screen5() {
	LvlObject *o1 = findLvlObject(2, 0, 5);
	postScreenUpdate_lar1_helper(o1, byte_4525FC, 1);
	LvlObject *o2 = findLvlObject(2, 1, 5);
	postScreenUpdate_lar1_helper(o2, byte_452600, 2);
	LvlObject *o3 = findLvlObject(2, 2, 5);
	postScreenUpdate_lar1_helper(o3, byte_452604, 3);
	if (_res->_currentScreenResourceNum == 5) {
		if (_levelCheckpoint >= 1 && _levelCheckpoint <= 3) {
			_levelCheckpoint = 2;
			BoundingBox b = { 194, 0, 255, 88 };
			AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (clipBoundingBox(&b, &data->boundingBox) && (byte_452610[0] & 0xF0) == 0x10) {
				// TODO:
			}
		}
	}
}

void Game::postScreenUpdate_lar1_screen8() {
	LvlObject *o1 = findLvlObject(2, 0, 8);
	postScreenUpdate_lar1_helper(o1, byte_452608, 4);
	LvlObject *o2 = findLvlObject(2, 1, 8);
	postScreenUpdate_lar1_helper(o2, byte_45260C, 5);
	if (_res->_currentScreenResourceNum == 8) {
		if (_levelCheckpoint >= 1 && _levelCheckpoint <= 3) {
			BoundingBox b = { 104, 0, 255, 80 };
			AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (clipBoundingBox(&b, &data->boundingBox)) {
				_levelCheckpoint = 3;
				const int a = (byte_452610[0] & 0xF0) != 0 ? 3 : 4;
				_screenCounterTable[26] = a;
				if ((byte_452614[0] & 0xF0) == 0x10) {
					_screenCounterTable[26] = a + 2;
				}
			}
		}
	}
}

void Game::postScreenUpdate_lar1_screen9() {
	LvlObject *o = findLvlObject(2, 0, 9);
	postScreenUpdate_lar1_helper(o, byte_452610, 6);
}

void Game::postScreenUpdate_lar1_screen13() {
	LvlObject *o = findLvlObject(2, 0, 13);
	postScreenUpdate_lar1_helper(o, byte_452614, 7);
}

void Game::postScreenUpdate_lar1_screen15() {
	LvlObject *o = findLvlObject(2, 0, 15);
	postScreenUpdate_lar1_helper(o, byte_45261C, 9);
}

void Game::postScreenUpdate_lar1_screen16() {
	LvlObject *o = findLvlObject(2, 0, 16);
	postScreenUpdate_lar1_helper(o, byte_452620, 10);
}

void Game::postScreenUpdate_lar1_screen19() {
	if (_screenCounterTable[19] == 0) {
		if (_res->_currentScreenResourceNum == 19) {
			BoundingBox b = { 160, 0, 209, 71 };
			AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			if (clipBoundingBox(&b, &data->boundingBox)) {
				_plasmaCannonFlags |= 2;
				if (!_paf->_skipCutscenes) {
					_paf->play(13);
					_paf->unload(13);
					_video->clearPalette();
					++_screenCounterTable[19];
					updateScreen(_andyObject->screenNum);
					// byte_4525E1 = 0;
					// byte_4525E7 = 0;
				}
				_andyObject->xPos = 204;
				_andyObject->yPos = 25;
				_andyObject->anim = 232;
				_andyObject->frame = 0;
				_andyObject->flags1 = (_andyObject->flags1 & 0xFFDF) | 0x10;
				_andyObject->directionKeyMask = 0;
				_andyObject->actionKeyMask = 0;
				setupLvlObjectBitmap(_andyObject);
				removeLvlObject(_andyObject);
				// sub_40E3C0
			}
		}
	} else if (_res->_screensState[19].s0 == 2) {
		++_screenCounterTable[19];
		if (_screenCounterTable[19] == 2) {
			_res->_resLvlScreenBackgroundDataTable[19].currentMaskId = 1;
			setupScreenMask(19);
		} else if (_screenCounterTable[19] >= 14) {
			_res->_screensState[19].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[19].currentBackgroundId = 1;
		}
	}
}

void Game::postScreenUpdate_lar1_screen20() {
	if (_res->_currentScreenResourceNum == 20) {
		postScreenUpdate_lar1_screen19();
	}
}

void Game::postScreenUpdate_lar1_screen22() {
	if (_res->_currentScreenResourceNum == 22) {
		BoundingBox b = { 36, 0, 208, 82 };
		postScreenUpdate_rock_helper2(&b, 16);
	}
}

void Game::postScreenUpdate_lar1_screen24() {
	if (_res->_currentScreenResourceNum == 24) {
		if ((_andyObject->flags0 & 0x1F) == 5) {
			_plasmaCannonFlags |= 1;
		}
		BoundingBox b = { 50, 168, 113, 191 };
		AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		if (clipBoundingBox(&b, &data->boundingBox)) {
			if (!_paf->_skipCutscenes) {
				_paf->play(14);
				_paf->unload(14);
			}
			_video->clearPalette();
			_quit = true;
		}
	}
}

void Game::callLevel_postScreenUpdate_lar1(int num) {
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
	case 13:
		postScreenUpdate_lar1_screen13();
		break;
	case 15:
		postScreenUpdate_lar1_screen15();
		break;
	case 16:
		postScreenUpdate_lar1_screen16();
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
	case 12:
	case 14:
	case 18:
		warning("callLevel_postScreenUpdate_lar1 %d unimplemented", num);
		break;
	}
}

void Game::preScreenUpdate_lar1_screen0() {
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

void Game::preScreenUpdate_lar1_screen2() {
	if (_res->_currentScreenResourceNum == 2) {
		if (_levelCheckpoint == 0) {
			_levelCheckpoint = 1;
		}
	}
}

void Game::preScreenUpdate_lar1_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		if (_levelCheckpoint >= 1 && _levelCheckpoint <= 3) {
			if ((byte_452610[0] & 0xF0) == 0) {
				_levelCheckpoint = 2;
				const uint8_t counter = ((byte_452610[0] & 0xF0) != 0) ? 2 : 0;
				_screenCounterTable[26] = counter; // +0x1A
			}
		}
	}
}

void Game::preScreenUpdate_lar1_screen11() {
	if (_res->_currentScreenResourceNum == 11) {
		if (_levelCheckpoint >= 2 && _levelCheckpoint <= 3) {
			if ((byte_452614[0] & 0xF) == 1 && (byte_452610[0] & 0xF) == 1) {
				_levelCheckpoint = 4;
			}
		}
		if (_transformShadowBuffer) {
			free(_transformShadowBuffer);
			_transformShadowBuffer = 0;
		}
		_video->_displayShadowLayer = false;
	}
}

void Game::preScreenUpdate_lar1_screen13() {
	if (_res->_currentScreenResourceNum == 13) {
		if (_transformShadowBuffer) {
			free(_transformShadowBuffer);
			_transformShadowBuffer = 0;
		}
		_video->_displayShadowLayer = false;
	}
}

void Game::preScreenUpdate_lar1_screen14() {
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
		if (_res->_screensState[14].s0 == 0 && _currentLevelCheckpoint == 4) {
			if (!_paf->_skipCutscenes) {
				_paf->preload(11);
			}
		}
	}
}

void Game::preScreenUpdate_lar1_screen16() {
	if (_res->_currentScreenResourceNum == 16) {
		if (_levelCheckpoint == 5) {
			_levelCheckpoint = 6;
		}
	}
}

void Game::preScreenUpdate_lar1_screen20() {
	if (_res->_currentScreenResourceNum == 20) {
		if (_levelCheckpoint == 6) {
			if ((_andyObject->flags0 & 0x1F) == 0xB) {
				_levelCheckpoint = 7;
			}
		}
	}
}

void Game::preScreenUpdate_lar1_screen24() {
	if (_res->_currentScreenResourceNum == 24) {
		if (!_paf->_skipCutscenes) {
			_paf->preload(14);
		}
	}
}

void Game::callLevel_preScreenUpdate_lar1(int num) {
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
	case 11:
		preScreenUpdate_lar1_screen11();
		break;
	case 13:
		preScreenUpdate_lar1_screen13();
		break;
	case 14:
		preScreenUpdate_lar1_screen14();
		break;
	case 16:
		preScreenUpdate_lar1_screen16();
		break;
	case 20:
		preScreenUpdate_lar1_screen20();
		break;
	case 24:
		preScreenUpdate_lar1_screen24();
		break;
	}
}

void Game::callLevel_initialize_lar1() {
	resetLevelTickHelperData();
	_screenCounterTable[26] = 0;
}

void Game::callLevel_tick_lar1() {
	// TODO
	updateLevelTickHelper();
	if (_screenCounterTable[19] != 0) {
		_plasmaCannonFlags |= 2;
	}
	if (_res->_currentScreenResourceNum == 12) {
		_video->_displayShadowLayer = 1;
	}
}

void Game::callLevel_setupLvlObjects_lar1(int num) {
	switch (num) {
	case 24:
		warning("callLevel_setupLvlObjects_lar1 not implemented for screen %d", num);
		break;
	}
}
