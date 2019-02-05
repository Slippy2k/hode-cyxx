
#include "game.h"
#include "paf.h"

void Game::preScreenUpdate_pwr2_screen2() {
	if (_levelCheckpoint == 1) {
		if (_res->_screensState[2].s0 == 0) {
			_res->_screensState[2].s0 = 2;
			_res->_resLvlScreenBackgroundDataTable[2].unk3 = 2;
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
