
// isld_hod

#include "game.h"
#include "paf.h"

void Game::level4OpStage3() {
	if (!_paf->_skipCutscenes) {
		_paf->preload(24);
	}
	// TODO:
}

void Game::level4OpStage4() {
	// TODO:
}

void Game::callLevel_terminate_isld() {
	if (!_paf->_skipCutscenes) {
		_paf->preload(24);
	}
}

