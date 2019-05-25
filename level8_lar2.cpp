
#include "game.h"
#include "paf.h"
#include "util.h"
#include "video.h"

static uint8_t byte_4528D0[4] = {  2, 0, 0, 0 };
static uint8_t byte_4528D4[4] = {  2, 0, 0, 0 };
static uint8_t byte_4528D8[4] = {  2, 0, 0, 0 };
static uint8_t byte_4528DC[4] = { 18, 0, 0, 0 };
static uint8_t byte_4528E0[4] = {  2, 0, 0, 0 };
static uint8_t byte_4528EC[4] = {  2, 0, 0, 0 };

bool Game::postScreenUpdate_lar2_screen2_helper(BoundingBox *b) {
	bool ret = false;
	BoundingBox b1 = { 121, 158, 131, 162 };
	if (clipBoundingBox(&b1, b)) {
		ret = true;
		byte_4528D0[0] &= 0xF;
		LvlObject *o = findLvlObject(0, 0, 2);
		if (o) {
			o->objectUpdateType = 7;
		}
	}
	BoundingBox b2 = { 170, 158, 180, 162 };
	if (clipBoundingBox(&b2, b)) {
		ret = true;
		byte_4528D0[0] &= 0xF;
		LvlObject *o = findLvlObject(0, 0, 2);
		if (o) {
			o->objectUpdateType = 7;
		}
	}
	BoundingBox b3 = { 215, 158, 225, 162 };
	if (clipBoundingBox(&b3, b)) {
		ret = true;
		byte_4528D0[0] &= 0xF;
		LvlObject *o = findLvlObject(0, 0, 2);
		if (o) {
			o->objectUpdateType = 7;
		}
	}
	return ret;
}

void Game::postScreenUpdate_lar2_screen2() {
	LvlObject *o = findLvlObject(2, 0, 2);
	postScreenUpdate_lar1_helper(o, byte_4528D0, 0);
	if (_res->_currentScreenResourceNum == 2) {
		bool ret = false;
		for (LvlObject *o = _lvlObjectsList1; o; o = o->nextPtr) {
			if (o->spriteNum == 16 && o->screenNum == _res->_currentScreenResourceNum) {
				BoundingBox b;
				b.x1 = o->xPos;
				b.y1 = o->yPos;
				b.x2 = o->xPos + o->width  - 1;
				b.y2 = o->yPos + o->height - 1;
				if (postScreenUpdate_lar2_screen2_helper(&b)) {
					ret = true;
				}
			}
		}
		AndyLvlObjectData *data = (AndyLvlObjectData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		if (postScreenUpdate_lar2_screen2_helper(&data->boundingBox) == 0) {
			BoundingBox b = { 107, 77, 117, 81 };
			if (clipBoundingBox(&b, &data->boundingBox)) {
				LvlObject *o = findLvlObject2(0, 3, 2);
				if (o) {
					o->objectUpdateType = 7;
				}
				if (!ret) {
					byte_4528D0[0] = (byte_4528D0[0] & 0xF) | 0x10;
				}
			}
		}
	}
}

void Game::postScreenUpdate_lar2_screen3() {
	LvlObject *o = findLvlObject(2, 0, 3);
	postScreenUpdate_lar1_helper(o, byte_4528D4, 1);
}

void Game::postScreenUpdate_lar2_screen4() {
	if (_currentLevelCheckpoint == 8 && _levelCheckpoint == 9) {
		byte_4528D8[0] = (byte_4528D8[0] & 0xF) | 0x10;
		if (!_paf->_skipCutscenes) {
			_paf->play(18);
			_paf->unload(18);
			_video->clearPalette();
			_currentLevelCheckpoint = _levelCheckpoint;
			updateScreen(_andyObject->screenNum);
		}
		LvlObject *o = findLvlObject(2, 0, 4);
		postScreenUpdate_lar1_helper(o, byte_4528D8, 2);
	}
}

void Game::postScreenUpdate_lar2_screen5() {
	if (_currentLevelCheckpoint == 7 and _levelCheckpoint == 8) {
		byte_4528DC[0] &= 0xF;
	}
	LvlObject *o = findLvlObject(2, 0, 5);
	postScreenUpdate_lar1_helper(o, byte_4528DC, 3);
}

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

void Game::postScreenUpdate_lar2_screen10() {
	LvlObject *o = findLvlObject(2, 0, 10);
	postScreenUpdate_lar1_helper(o, byte_4528E0, 4);
}

void Game::postScreenUpdate_lar2_screen19() {
	if (_res->_currentScreenResourceNum == 19) {
		if (_currentLevelCheckpoint == 10 && _levelCheckpoint == 11) {
			if (!_paf->_skipCutscenes) {
				_paf->play(19);
				_paf->unload(19);
			}
			_video->clearPalette();
			_quit = true;
		}
	}
}

void Game::callLevel_postScreenUpdate_lar2(int num) {
	switch (num) {
	case 2:
		postScreenUpdate_lar2_screen2();
		break;
	case 3:
		postScreenUpdate_lar2_screen3();
		break;
	case 4:
		postScreenUpdate_lar2_screen4();
		break;
	case 5:
		postScreenUpdate_lar2_screen5();
		break;
	case 6:
		// TODO
		break;
	case 7:
		postScreenUpdate_lar2_screen7();
		break;
	case 8:
		// TODO
		break;
	case 10:
		postScreenUpdate_lar2_screen10();
		break;
	case 11:
		// TODO
		break;
	case 12:
		// TODO
		break;
	case 13:
		// TODO
		break;
	case 19:
		postScreenUpdate_lar2_screen19();
		break;
	}
}

void Game::preScreenUpdate_lar2_screen2() {
	LvlObject *o = findLvlObject(2, 0, 2);
	postScreenUpdate_lar1_helper(o, byte_4528D0, 1);
	if (_res->_currentScreenResourceNum == 2) {
		if (_levelCheckpoint == 0) {
			_levelCheckpoint = 1;
		}
	}
}

void Game::preScreenUpdate_lar2_screen4() {
	if (_res->_currentScreenResourceNum == 4) {
		if (_levelCheckpoint == 1) {
			_levelCheckpoint = 2;
		}
		if (_levelCheckpoint >= 2) {
			byte_4528D4[0] &= 0xF;
			if (_levelCheckpoint == 8) {
				byte_4528D8[0] &= 0xF;
				if (!_paf->_skipCutscenes) {
					_paf->preload(18);
				}
			}
		}
	}
}

void Game::preScreenUpdate_lar2_screen5() {
	if (_res->_currentScreenResourceNum == 5) {
		if (_levelCheckpoint == 7) {
			byte_4528DC[0] = (byte_4528D4[0] & 0xF) | 0x10;
		} else if (_levelCheckpoint >= 3) {
			byte_4528DC[0] &= 0xF;
		}
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
				byte_4528DC[0] &= 0xF;
			} else if (_levelCheckpoint == 6) {
				_paf->preload(17);
			}
		}
	}
}

void Game::preScreenUpdate_lar2_screen7() {
	if (_res->_currentScreenResourceNum == 7) {
		if (_levelCheckpoint >= 4 && _levelCheckpoint < 7) {
			_res->_screensState[7].s0 = 1;
		} else {
			_res->_screensState[7].s0 = 0;
		}
		if (_levelCheckpoint == 5) {
			if (!_paf->_skipCutscenes) {
				_paf->preload(16);
			}
			if (_res->_screensState[7].s0 != 0) {
				_res->_resLvlScreenBackgroundDataTable[7].currentBackgroundId = 1;
				_res->_resLvlScreenBackgroundDataTable[7].currentMaskId = 1;
			} else {
				_res->_resLvlScreenBackgroundDataTable[7].currentBackgroundId = 0;
				_res->_resLvlScreenBackgroundDataTable[7].currentMaskId = 0;
			}
		}
	}
}

void Game::preScreenUpdate_lar2_screen8() {
	if (_res->_currentScreenResourceNum == 8) {
		if (byte_4528EC[2] > 1) {
			byte_4528EC[2] = 1;
		}
		LvlObject *o = findLvlObject(2, 0, 8);
		postScreenUpdate_lar1_helper(o, byte_4528EC, 7);
	}
}

void Game::preScreenUpdate_lar2_screen9() {
	if (_res->_currentScreenResourceNum == 9) {
		byte_4528EC[2] = 0x24;
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
	case 2:
		preScreenUpdate_lar2_screen2();
		break;
	case 4:
		preScreenUpdate_lar2_screen4();
		break;
	case 5:
		preScreenUpdate_lar2_screen5();
		break;
	case 6:
		preScreenUpdate_lar2_screen6();
		break;
	case 7:
		preScreenUpdate_lar2_screen7();
		break;
	case 8:
	case 11:
		preScreenUpdate_lar2_screen8();
		break;
	case 9:
		preScreenUpdate_lar2_screen9();
		break;
	case 15:
		preScreenUpdate_lar2_screen15();
		break;
	case 19:
		preScreenUpdate_lar2_screen19();
		break;
	}
}

void Game::callLevel_tick_lar2() {
	// TODO
}

void Game::callLevel_setupLvlObjects_lar2(int num) {
	switch (num) {
	case 19:
		warning("callLevel_setupLvlObjects_lar2 not implemented for screen %d", num);
		break;
	}
}
