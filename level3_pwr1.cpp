/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

// pwr1_hod

#include "game.h"
#include "lzw.h"
#include "paf.h"
#include "util.h"
#include "video.h"

void Game::callLevel_postScreenUpdate_level3(int num) {
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
		warning("callLevel_postScreenUpdate_level3 %d unimplemented", num);
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
	switch (num) {
	case 4:
	case 6:
	case 9:
	case 15:
	case 21:
	case 23:
	case 24:
	case 26:
	case 27:
	case 29:
	case 31:
	case 35:
		warning("callLevelOpStage2_level3 %d unimplemented", num);
		break;
	}
}

void Game::level3OpStage3() {
	_shakeShadowBuffer = (uint8_t *)malloc(256 * 192 + 256);
	decodeLZW(_levelOpStage3ImageData1, _shakeShadowBuffer);
	memcpy(_shakeShadowBuffer + 256 * 192, _shakeShadowBuffer, 256);
	// TODO:
	warning("level3OpStage3 unimplemented");
}

