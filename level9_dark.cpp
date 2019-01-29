
// dark_hod

#include "game.h"
#include "paf.h"
#include "video.h"

const Game::OpStage1Proc Game::_callLevel_objectUpdate_dark[] = {
	&Game::objectUpdate_rock_case0,
	&Game::objectUpdate_rock_case3
};

void Game::postScreenUpdate_dark_screen0() {
	if (_res->_screensState[0].s0 != 0) {
		++_screenCounterTable[0];
		if (_screenCounterTable[0] == 29) {
			if (!_paf->_skipCutscenes) {
				_paf->play(20);
				_paf->unload(20);
				// sub_427F32
				_paf->preload(21);
				_paf->play(21);
				_paf->unload(21);
			}
			_video->clearPalette();
			// sub_4375E0
			_quit = true;
		}
	}
}

void Game::preScreenUpdate_dark_screen0() {
	if (_res->_currentScreenResourceNum == 0) {
		if (!_paf->_skipCutscenes) {
			_paf->preload(20);
		}
		_screenCounterTable[0] = 0;
		_res->_screensState[0].s0 = 0;
	}
}
