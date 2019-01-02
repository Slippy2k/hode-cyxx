/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

// pwr1_hod

#include "game.h"
#include "paf.h"
#include "util.h"
#include "video.h"

void Game::callLevelOpStage0_level3(int num) {
	switch (num) {
	case 6:
	case 10:
	case 12:
	case 14:
	case 16:
	case 18:
	case 23:
	case 27:
	case 35:
//		level3OpStage0_screenN();
		break;
	}
}

int Game::level3OpStage1_case0(LvlObject *o) {
	return 1;
}

int Game::level3OpStage1_case1(LvlObject *o) {
	updateAndyObject(o);
	return 1;
}

int Game::callLevelOpStage1_level3(int num, LvlObject *o) {
	return (this->*_level3OpStage1[num])(o);
}

void Game::callLevelOpStage2_level3(int num) {
}

void Game::level3OpStage3() {
	warning("level3OpStage3 unimplemented");
#if 0
	_levelOpStage3ImageBuf = malloc(256 * 192 + 256);
	decodeLZW(_levelOpStage3ImageData1, _levelOpStage3ImageBuf);
	memcpy(_levelOpStage3ImageBuf + 256 * 192, _levelOpStage3ImageBuf);
	// TODO:
#endif
}

