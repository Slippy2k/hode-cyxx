/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

// fort_hod

#include "game.h"
#include "paf.h"
#include "video.h"

void Game::postScreenUpdate_fort_screen1() {
	if (_res->_screensState[1].s0 == 2) {
		LvlObject *o = _res->findLvlObject(2, 0, 1);
		if (o) {
			o->actionKeyMask = 1;
		}
		++_screenCounterTable[1];
		if (_screenCounterTable[1] == 25) {
			_res->_resLvlScreenBackgroundDataTable[1].unk3 = 1;
			setupScreenMask(1);
		} else if (_screenCounterTable[1] == 59) {
			_res->_screensState[1].s0 = 1;
			_res->_resLvlScreenBackgroundDataTable[1].currentBackgroundId = 1;
		}
	}
}

void Game::postScreenUpdate_fort_screen6() {
	if (_res->_currentScreenResourceNum == 6) {
		AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		BoundingBox b = { 0, 0, 43, 191 };
		if (!clipBoundingBox(&b, &data->boundingBox)) {
			_andyObject->actionKeyMask &= ~3;
			_andyObject->actionKeyMask |= 8;
			if (_andyObject->directionKeyMask & 4) {
				_andyObject->directionKeyMask &= ~10;
				++_screenCounterTable[6];
				if (_screenCounterTable[6] >= 188) {
					_andyObject->directionKeyMask &= ~4;
					_screenCounterTable[7] = 0;
				}
			}
			if (_screenCounterTable[7] < 25) {
				++_screenCounterTable[7];
				_andyObject->directionKeyMask &= ~4;
			} else if ((_andyObject->directionKeyMask & 4) == 0) {
				_screenCounterTable[6] = 0;
			}
		}
	}
}

void Game::postScreenUpdate_fort_screen7() {
	if (_res->_currentScreenResourceNum == 7) {
		_andyObject->actionKeyMask &= ~3;
		_andyObject->actionKeyMask |= 8;
		if (_andyObject->directionKeyMask & 4) {
			_andyObject->directionKeyMask &= ~0xA;
			++_screenCounterTable[6];
			if (_screenCounterTable[6] >= 188) {
				_andyObject->directionKeyMask &= ~4;
				_screenCounterTable[7] = 0;
			}
		}
		if (_screenCounterTable[7] < 25) {
			++_screenCounterTable[7];
			_andyObject->directionKeyMask &= ~4;
		} else if ((_andyObject->directionKeyMask & 4) == 0) {
			_screenCounterTable[6] = 0;
		}
	}
}

void Game::postScreenUpdate_fort_screen8() {
	if (_res->_currentScreenResourceNum == 8) {
		AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		BoundingBox b = { 138, 0, 255, 191 };
		if (!clipBoundingBox(&b, &data->boundingBox)) {
			_andyObject->actionKeyMask &= ~3;
			_andyObject->actionKeyMask |= 8;
			if (_andyObject->directionKeyMask & 4) {
				_andyObject->directionKeyMask &= ~0xA;
				++_screenCounterTable[6];
				if (_screenCounterTable[6] >= 188) {
					_andyObject->directionKeyMask &= ~4;
					_screenCounterTable[7] = 0;
				}
			}
			if (_screenCounterTable[7] < 25) {
				++_screenCounterTable[7];
				_andyObject->directionKeyMask &= ~4;
			} else if ((_andyObject->directionKeyMask & 4) == 0) {
				_screenCounterTable[6] = 0;
			}
		}
	}
}

void Game::postScreenUpdate_fort_screen16() {
	if (_res->_currentScreenResourceNum == 16) {
		if (_res->_screensState[16].s0 == 1) {
			AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			BoundingBox b = { 150, 0, 187, 60 };
                	if (!clipBoundingBox(&b, &data->boundingBox)) {
				return;
			}
			if ((_andyObject->flags0 & 0x1F) == 0 && (_andyObject->flags0 & 0xE0) == 0xE0) {
				_res->_screensState[16].s0 = 1;
				_screenCounterTable[16] = 0;
			} else {
				game_unk27(3);
			}
		} else {
			++_screenCounterTable[16];
			if (_screenCounterTable[16] == 5) {
				if (_levelCheckpoint == 3) {
					_levelCheckpoint = 4;
				}
				if (!_paf->_skipCutscenes) {
					_paf->play(3);
					_paf->unload(3);
				}
				_video->clearPalette();
				updateAndyLvlObject();
			}
		}
	}
}

void Game::postScreenUpdate_fort_screen17() {
	if (_res->_screensState[17].s0 == 1) {
		++_screenCounterTable[17];
		if (_screenCounterTable[17] == 68) {
			_screenCounterTable[17] = 0;
		}
	}
}

void Game::postScreenUpdate_fort_screen21() {
	if (_res->_currentScreenResourceNum == 21) {
		switch (_res->_screensState[21].s0) {
		case 1:
			++_screenCounterTable[21];
			if (_screenCounterTable[21] == 22) {
				_res->_screensState[21].s0 = 2;
			}
			break;
		case 2:
			if (!_paf->_skipCutscenes) {
				_paf->play(4);
				_paf->unload(4);
			}
			_video->clearPalette();
			_quit = true;
			break;
		}
	}
}

void Game::callLevel_postScreenUpdate_level2(int num) {
	switch (num) {
	case 1:
		postScreenUpdate_fort_screen1();
		break;
	case 6:
		postScreenUpdate_fort_screen6();
		break;
	case 7:
		postScreenUpdate_fort_screen7();
		break;
	case 8:
		postScreenUpdate_fort_screen8();
		break;
	case 16:
		postScreenUpdate_fort_screen16();
		break;
	case 17:
		postScreenUpdate_fort_screen17();
		break;
	case 21:
		postScreenUpdate_fort_screen21();
		break;
	}
}

int Game::level2OpStage1_case0(LvlObject *o) {
	return 1;
}

int Game::level2OpStage1_case1(LvlObject *o) {
	updateAndyObject(o);
	return 1;
}

int Game::callLevelOpStage1_level2(int num, LvlObject *o) {
	return (this->*_level2OpStage1[num])(o);
}

void Game::level2OpStage2_screen1() {
	if (_res->_currentScreenResourceNum == 1 && _levelCheckpoint >= 1) {
		_res->_screensState[1].s0 = 1;
	}
	const int num = _res->_screensState[1].s0 == 0 ? 0 : 1;
	_res->_resLvlScreenBackgroundDataTable[1].currentBackgroundId = num;
	_res->_resLvlScreenBackgroundDataTable[1].unk3 = num;
}

void Game::level2OpStage2_screen2() {
	if (_res->_currentScreenResourceNum == 2 && _levelCheckpoint == 0) {
		_levelCheckpoint = 1;
	}
}

void Game::level2OpStage2_screen6() {
	if (_res->_currentScreenResourceNum == 6 && _levelCheckpoint == 1) {
		_levelCheckpoint = 2;
	}
}

void Game::level2OpStage2_screen9() {
	if (_res->_currentScreenResourceNum == 9 && _levelCheckpoint == 2) {
		_levelCheckpoint = 3;
	}
}

void Game::level2OpStage2_screen14() {
	if (_res->_currentScreenResourceNum == 14) {
		_res->_resLvlScreenBackgroundDataTable[14].currentBackgroundId = _res->_screensState[14].s0 != 0 ? 1 : 0;
	}
}

void Game::level2OpStage2_screen16() {
	if (_res->_currentScreenResourceNum == 16) {
		_res->_screensState[16].s0 = 0;
		_andyObject->xPos += 9;
		if (!_paf->_skipCutscenes) {
			_paf->preload(3);
		}
	}
}

void Game::level2OpStage2_screen17() {
	if (_res->_currentScreenResourceNum == 17) {
		// TODO
		_res->_screensState[17].s0 = _screenCounterTable[17] == 0 ? 1 : 0;
	}
}

void Game::level2OpStage2_screen21() {
	if (_res->_currentScreenResourceNum == 21) {
		_res->_screensState[21].s0 = 0;
		if (!_paf->_skipCutscenes) {
			_paf->preload(4);
		}
	}
}

void Game::callLevelOpStage2_level2(int num) {
	switch (num) {
	case 1:
		level2OpStage2_screen1();
		break;
	case 2:
		level2OpStage2_screen2();
		break;
	case 6:
		level2OpStage2_screen6();
		break;
	case 9:
		level2OpStage2_screen9();
		break;
	case 14:
		level2OpStage2_screen14();
		break;
	case 16:
		level2OpStage2_screen16();
		break;
	case 17:
		level2OpStage2_screen17();
		break;
	case 21:
		level2OpStage2_screen21();
		break;
	}
}

