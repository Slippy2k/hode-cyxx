
#include "game.h"
#include "paf.h"

static const uint8_t byte_451DE8[] = {
	0, 1, 2, 3, 5, 7, 9, 10, 11, 12, 12, 12, 12, 11, 10, 9, 7, 5, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void Game::postScreenUpdate_isld_screen0() {
	if (_res->_currentScreenResourceNum == 0) {
		if (_screenCounterTable[0] < 5) {
			++_screenCounterTable[0];
			_andyObject->actionKeyMask |= 1;
			_andyObject->directionKeyMask |= 2;
		}
	}
}

void Game::postScreenUpdate_isld_screen1() {
	if (_res->_screensState[1].s0 != 2) {
		_screenCounterTable[1] = 0;
	} else {
		++_screenCounterTable[1];
		if (_screenCounterTable[1] > 21) {
			_res->_screensState[1].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[1].currentBackgroundId = 1;
		}
	}
}

void Game::postScreenUpdate_isld_screen3() {
	if (_res->_currentScreenResourceNum == 3) {
		if (_andyObject->xPos < 150) {
			LvlObject *o = findLvlObject2(0, 1, 3);
			if (o) {
				AnimBackgroundData *backgroundData = (AnimBackgroundData *)getLvlObjectDataPtr(o, kObjectDataTypeAnimBackgroundData);
				AndyObjectScreenData *andyData = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
				andyData->dxPos += byte_451DE8[backgroundData->currentFrame];
			}
		}
	}
}

void Game::postScreenUpdate_isld_screen4() {
	if (_res->_currentScreenResourceNum == 3) {
		if (_andyObject->xPos < 150) {
			LvlObject *o = findLvlObject2(0, 1, 4);
			if (o) {
				AnimBackgroundData *backgroundData = (AnimBackgroundData *)getLvlObjectDataPtr(o, kObjectDataTypeAnimBackgroundData);
				AndyObjectScreenData *andyData = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
				andyData->dxPos += byte_451DE8[backgroundData->currentFrame];
			}
		}
	}
}

void Game::postScreenUpdate_isld_screen8() {
	if (_res->_currentScreenResourceNum == 8) {
		const int xPos = _andyObject->xPos + _andyObject->posTable[3].x;
		if (xPos > 246) {
			_fallingAndyCounter = 2;
			_fallingAndyFlag = true;
			playAndyFallingCutscene(1);
		}
	}
}

void Game::postScreenUpdate_isld_screen9() {
	if (_res->_currentScreenResourceNum == 9) {
		const int xPos = _andyObject->xPos + _andyObject->posTable[3].x;
		if (xPos < 10 || xPos > 246) {
			_fallingAndyCounter = 2;
			_fallingAndyFlag = true;
			playAndyFallingCutscene(1);
		}
	}
}

void Game::callLevel_postScreenUpdate_isld(int num) {
	switch (num) {
	case 0:
		postScreenUpdate_isld_screen0();
		break;
	case 1:
		postScreenUpdate_isld_screen1();
		break;
	case 3:
		postScreenUpdate_isld_screen3();
		break;
	case 4:
		postScreenUpdate_isld_screen4();
		break;
	case 8:
		postScreenUpdate_isld_screen8();
		break;
	case 9:
		postScreenUpdate_isld_screen9();
		break;
	}
}

void Game::preScreenUpdate_isld_screen1() {
	if (_res->_currentScreenResourceNum == 1) {
		switch (_res->_screensState[1].s0) {
		case 1:
		case 2:
			_res->_resLvlScreenBackgroundDataTable[1].currentBackgroundId = 1;
			_res->_screensState[1].s0 = 1;
			break;
		default:
			_res->_resLvlScreenBackgroundDataTable[1].currentBackgroundId = 0;
			_res->_screensState[1].s0 = 0;
			break;
		}
	}
}

void Game::preScreenUpdate_isld_screen2() {
	if (_res->_currentScreenResourceNum == 2) {
		_res->_screensState[2].s0 = 0;
		_res->_resLvlScreenBackgroundDataTable[2].currentMaskId = 0;
		_screenCounterTable[2] = 0;
		LvlObject *o = findLvlObject(2, 0, 2);
		if (o) {
			o->actionKeyMask = 0;
			o->xPos = 30;
			o->yPos = 83;
			o->frame = 0;
			setupLvlObjectBitmap(o);
		}
	}
}

void Game::preScreenUpdate_isld_screen3() {
	if (_res->_currentScreenResourceNum == 3) {
		LvlObject *o = findLvlObject(2, 0, 3);
		if (o) {
			o->xPos = 70;
			o->yPos = 0;
			o->frame = 22;
			setupLvlObjectBitmap(o);
		}
	}
}

void Game::preScreenUpdate_isld_screen9() {
	if (_res->_currentScreenResourceNum == 9) {
		if (_levelCheckpoint == 1) {
			_levelCheckpoint = 2;
		}
	}
}

void Game::preScreenUpdate_isld_screen14() {
	if (_res->_screensState[14].s0 != 0) {
		_res->_resLvlScreenBackgroundDataTable[14].currentBackgroundId = 1;
		_res->_resLvlScreenBackgroundDataTable[14].currentMaskId = 1;
	} else {
		_res->_resLvlScreenBackgroundDataTable[14].currentBackgroundId = 0;
		_res->_resLvlScreenBackgroundDataTable[14].currentMaskId = 0;
	}
	if (_res->_currentScreenResourceNum == 14) {
		if (_levelCheckpoint == 2) {
			_levelCheckpoint = 3;
		}
	}
}

void Game::preScreenUpdate_isld_screen15() {
	if (_res->_screensState[15].s0 != 0) {
		_res->_resLvlScreenBackgroundDataTable[15].currentBackgroundId = 1;
		_res->_resLvlScreenBackgroundDataTable[15].unk7 = 1;
		_res->_screensState[14].s0 = 1;
	} else {
		_res->_resLvlScreenBackgroundDataTable[15].currentBackgroundId = 0;
		_res->_resLvlScreenBackgroundDataTable[15].unk7 = 0;
	}
}

void Game::preScreenUpdate_isld_screen16() {
	if (_res->_currentScreenResourceNum == 16) {
		if (_levelCheckpoint == 3) {
			_levelCheckpoint = 4;
		}
		if (_res->_screensState[14].s0 == 0) {
			_res->_screensState[14].s0 = 1;
			_res->_screensState[15].s0 = 1;
		}
	}
}

void Game::preScreenUpdate_isld_screen21() {
	if (_res->_currentScreenResourceNum == 21) {
		if (!_paf->_skipCutscenes) {
			_paf->preload(6);
		}
	}
}

void Game::callLevel_preScreenUpdate_isld(int num) {
	switch (num) {
	case 1:
		preScreenUpdate_isld_screen1();
		break;
	case 2:
		preScreenUpdate_isld_screen2();
		break;
	case 3:
		preScreenUpdate_isld_screen3();
		break;
	case 9:
		preScreenUpdate_isld_screen9();
		break;
	case 14:
		preScreenUpdate_isld_screen14();
		break;
	case 15:
		preScreenUpdate_isld_screen15();
		break;
	case 16:
		preScreenUpdate_isld_screen16();
		break;
	case 21:
		preScreenUpdate_isld_screen21();
		break;
	}
}

void Game::callLevel_initialize_isld() {
	if (!_paf->_skipCutscenes) {
		_paf->preload(24);
	}
	// TODO:
}

void Game::callLevel_tick_isld() {
	// TODO:
}

void Game::callLevel_terminate_isld() {
	if (!_paf->_skipCutscenes) {
		_paf->preload(24);
	}
}

