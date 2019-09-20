
// dark_hod

#include "game.h"
#include "level.h"
#include "paf.h"
#include "video.h"

struct Level_dark: Level {
        //const CheckpointData *getCheckpointData() = 0;

	//virtual void initialize();
	//virtual void terminate();
	//virtual void tick();
	virtual void preScreenUpdate(int screenNum);
	virtual void postScreenUpdate(int screenNum);
	//virtual void setupLvlObjects(int screenNum);

	void postScreenUpdate_dark_screen0();

	void preScreenUpdate_dark_screen0();
};

Level *Level_dark_create() {
	return new Level_dark;
}

void Level_dark::postScreenUpdate_dark_screen0() {
	if (_res->_screensState[0].s0 != 0) {
		++_screenCounterTable[0];
		if (_screenCounterTable[0] == 29) {
			if (!_paf->_skipCutscenes) {
				_paf->play(20);
				_paf->unload(20);
				_g->displayHintScreen(_res->_datHdr.yesNoQuitImage - 1, 25);
				_paf->preload(21);
				_paf->play(21);
				_paf->unload(21);
			}
			_video->clearPalette();
			_video->fillBackBuffer();
			_g->_endLevel = true;
		}
	}
}

void Level_dark::postScreenUpdate(int num) {
	switch (num) {
	case 0:
		postScreenUpdate_dark_screen0();
		break;
	}
}

void Level_dark::preScreenUpdate_dark_screen0() {
	if (_res->_currentScreenResourceNum == 0) {
		if (!_paf->_skipCutscenes) {
			_paf->preload(20);
		}
		_screenCounterTable[0] = 0;
		_res->_screensState[0].s0 = 0;
	}
}

void Level_dark::preScreenUpdate(int num) {
	switch (num) {
	case 0:
		preScreenUpdate_dark_screen0();
		break;
	}
}
