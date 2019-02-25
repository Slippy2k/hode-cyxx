
#include "game.h"
#include "paf.h"
#include "util.h"
#include "video.h"

static uint8_t byte_452610;
static uint8_t byte_452614;

void Game::preScreenUpdate_lar1_screen0() {
	if (_res->_currentScreenResourceNum == 0) {
		switch (_res->_screensState[0].s0) {
		case 0:
			_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 0;
			_res->_resLvlScreenBackgroundDataTable[0].unk3 = 0;
			_screenCounterTable[0] = 0;
			break;
		case 3:
			_res->_screensState[0].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 1;
			_res->_resLvlScreenBackgroundDataTable[0].unk3 = 1;
			_screenCounterTable[0] = 45;
			break;
		case 4:
			_screenCounterTable[0] = 64;
			_res->_screensState[0].s0 = 2;
			_res->_resLvlScreenBackgroundDataTable[0].currentBackgroundId = 2;
			_res->_resLvlScreenBackgroundDataTable[0].unk3 = 2;
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
			if ((byte_452610 & 0xF0) == 0) {
				_levelCheckpoint = 2;
				const uint8_t counter = ((byte_452610 & 0xF0) != 0) ? 2 : 0;
				_screenCounterTable[26] = counter; // +0x1A
			}
		}
	}
}

void Game::preScreenUpdate_lar1_screen11() {
	if (_res->_currentScreenResourceNum == 11) {
		if (_levelCheckpoint >= 2 && _levelCheckpoint <= 3) {
			if ((byte_452614 & 0xF) == 1 && (byte_452610 & 0xF) == 1) {
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
		_res->_resLvlScreenBackgroundDataTable[14].unk3 = 0;
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
	// TODO
	_screenCounterTable[26] = 0;
}

void Game::callLevel_tick_lar1() {
	// TODO
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
