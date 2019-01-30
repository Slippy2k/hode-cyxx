
// isld_hod

#include "game.h"
#include "paf.h"

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

