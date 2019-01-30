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

void Game::callLevel_postScreenUpdate_pwr1(int num) {
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
		warning("callLevel_postScreenUpdate_pwr1 %d unimplemented", num);
		break;
	}
}

void Game::callLevel_preScreenUpdate_pwr1(int num) {
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
		warning("callLevel_preScreenUpdate_level3 %d unimplemented", num);
		break;
	}
}

void Game::callLevel_initialize_pwr1() {
	_shakeShadowBuffer = (uint8_t *)malloc(256 * 192 + 256);
	decodeLZW(_levelOpStage3ImageData1, _shakeShadowBuffer);
	memcpy(_shakeShadowBuffer + 256 * 192, _shakeShadowBuffer, 256);
	// TODO:
	warning("level3OpStage3 unimplemented");
}

