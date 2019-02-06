
// isld_hod

#include "game.h"
#include "paf.h"

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
	case 8:
		postScreenUpdate_isld_screen8();
		break;
	case 9:
		postScreenUpdate_isld_screen9();
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

