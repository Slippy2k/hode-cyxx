
#include "game.h"
#include "paf.h"
#include "video.h"

void Game::postScreenUpdate_pwr2_screen2() {
	uint8_t mask;
	switch (_res->_screensState[2].s0) {
	case 1:
		mask = 1;
		break;
	case 2:
		mask = 2;
		break;
	default:
		mask = 0;
		break;
	}
	_res->_resLvlScreenBackgroundDataTable[2].currentMaskId = mask;
	if (_res->_screensState[2].s3 != mask) {
		setupScreenMask(2);
	}
}

void Game::postScreenUpdate_pwr2_screen5() {
	if (_res->_screensState[5].s0 == 1) {
		_res->_screensState[5].s0 = 2;
		if (_levelCheckpoint == 1) {
			_levelCheckpoint = 2;
		}
		if (!_paf->_skipCutscenes) {
			_paf->play(9);
			_paf->unload(9);
		}
		_video->clearPalette();
		restartLevel();
	}
}

void Game::postScreenUpdate_pwr2_screen8() {
	if (_res->_screensState[8].s0 != 0) {
		if (!_paf->_skipCutscenes) {
			_paf->play(10);
			_paf->unload(10);
		}
		_video->clearPalette();
		_quit = true;
	}
}

void Game::callLevel_postScreenUpdate_pwr2(int num) {
	switch (num) {
	case 2:
		postScreenUpdate_pwr2_screen2();
		break;
	case 5:
		postScreenUpdate_pwr2_screen5();
		break;
	case 8:
	case 9:
	case 10:
	case 11:
		postScreenUpdate_pwr2_screen8();
		break;
	}
}

void Game::preScreenUpdate_pwr2_screen2() {
	if (_levelCheckpoint == 1) {
		if (_res->_screensState[2].s0 == 0) {
			_res->_screensState[2].s0 = 2;
			_res->_resLvlScreenBackgroundDataTable[2].currentMaskId = 2;
		}
	}
}

void Game::preScreenUpdate_pwr2_screen3() {
	if (_res->_currentScreenResourceNum == 3 && _levelCheckpoint != 0) {
		AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		BoundingBox b = { 0, 103, 122, 191 };
		if (clipBoundingBox(&b, &data->boundingBox)) {
			_levelCheckpoint = 1;
		}
	}
}

void Game::preScreenUpdate_pwr2_screen5() {
	if (_res->_currentScreenResourceNum == 5) {
		if (!_paf->_skipCutscenes) {
			_paf->preload(9);
		}
	}
}

void Game::preScreenUpdate_pwr2_screen7() {
	if (_res->_currentScreenResourceNum == 7) {
		_res->_screensState[5].s0 = 2; // +0x14
		if (!_paf->_skipCutscenes) {
			_paf->preload(10);
		}
	}
}

void Game::callLevel_preScreenUpdate_pwr2(int num) {
	switch (num) {
	case 2:
		preScreenUpdate_pwr2_screen2();
		break;
	case 3:
		preScreenUpdate_pwr2_screen3();
		break;
	case 5:
		preScreenUpdate_pwr2_screen5();
		break;
	case 7:
		preScreenUpdate_pwr2_screen7();
		break;
	}
}

void Game::callLevel_initialize_pwr2() {
}

const uint8_t Game::_pwr2_screenTransformLut[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void Game::callLevel_tick_pwr2() {
	_video->_displayShadowLayer = _pwr2_screenTransformLut[_res->_currentScreenResourceNum * 2];
}
