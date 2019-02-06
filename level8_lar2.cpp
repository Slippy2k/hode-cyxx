
#include "game.h"
#include "paf.h"
#include "video.h"

void Game::postScreenUpdate_lar2_screen7() {
	if (_res->_currentScreenResourceNum == 7) {
		if (!_paf->_skipCutscenes) {
			const uint8_t state = _res->_screensState[7].s0;
			if (state != 0 && _levelCheckpoint == 5 && state > 1) {
				_paf->play(16);
				_paf->unload(16);
				_video->clearPalette();
				_res->_screensState[7].s0 = 1;
				updateScreen(_andyObject->screenNum);
			}
		}
	}
}

void Game::callLevel_postScreenUpdate_lar2(int num) {
	switch (num) {
	case 7:
		postScreenUpdate_lar2_screen7();
		break;
	}
}

void Game::preScreenUpdate_lar2_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		if (_levelCheckpoint == 2) {
			_levelCheckpoint = 3;
		}
		if (!_paf->_skipCutscenes) {
			if (_levelCheckpoint == 3) {
				_paf->preload(15);
				// byte_4528DC &= 15
			} else if (_levelCheckpoint == 6) {
				_paf->preload(17);
			}
		}
	}
}

void Game::preScreenUpdate_lar2_screen15() {
	if (_res->_currentScreenResourceNum == 15) {
		if (_levelCheckpoint == 9) {
			_levelCheckpoint = 10;
		}
	}
}

void Game::preScreenUpdate_lar2_screen19() {
	if (_res->_currentScreenResourceNum == 19) {
		if (_levelCheckpoint == 10) {
			if (!_paf->_skipCutscenes) {
				_paf->preload(19);
			}
		}
	}
}

void Game::callLevel_preScreenUpdate_lar2(int num) {
	switch (num) {
	case 6:
		preScreenUpdate_lar2_screen6();
		break;
	case 15:
		preScreenUpdate_lar2_screen15();
		break;
	case 19:
		preScreenUpdate_lar2_screen19();
		break;
	}
}
