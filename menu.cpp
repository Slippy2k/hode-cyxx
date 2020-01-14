
#include "game.h"
#include "menu.h"
#include "lzw.h"
#include "paf.h"
#include "resource.h"
#include "system.h"
#include "util.h"
#include "video.h"

Menu::Menu(Game *g, PafPlayer *paf, Resource *res, Video *video)
	: _g(g), _paf(paf), _res(res), _video(video) {

	assert(sizeof(SetupConfig) == Game::kSetupCfgSize);
	_config = (SetupConfig *)_g->_setupCfgBuffer;

	_titleSprites = 0;
	_playerSprites = 0;
	_titleBitmapData = 0;
	_titleBitmapSize = 0;
	_playerBitmapData = 0;
	_playerBitmapSize = 0;
	_digitTiles = 0;
	_optionData = 0;
	_soundData = 0;

	_palNum = 0;
}

void Menu::loadData() {
	_res->loadDatMenuBuffers();

	const int version = _res->_datHdr.version;

	const uint8_t *ptr = _res->_menuBuffer1;
	uint32_t ptrOffset = 0;

	if (version == 10) {

		_titleSprites = (DatSpritesGroup *)(ptr + ptrOffset);
		_titleSprites->size = le32toh(_titleSprites->size);
		ptrOffset += sizeof(DatSpritesGroup) + _titleSprites->size;

		_playerSprites = (DatSpritesGroup *)(ptr + ptrOffset);
		_playerSprites->size = le32toh(_playerSprites->size);
		ptrOffset += sizeof(DatSpritesGroup) + _playerSprites->size;

		_titleBitmapSize = READ_LE_UINT32(ptr + ptrOffset);
		_titleBitmapData = ptr + ptrOffset + 8;
		ptrOffset += 8 + _titleBitmapSize + 768;

		_playerBitmapSize = READ_LE_UINT32(ptr + ptrOffset);
		_playerBitmapData = ptr + ptrOffset + 8;
		ptrOffset += 8 + _playerBitmapSize + 768;

		const int size = READ_LE_UINT32(ptr + ptrOffset); ptrOffset += 4;
		assert((size % (16 * 10)) == 0);
		_digitTiles = ptr + ptrOffset;
		ptrOffset += size;

		const int cutscenesCount = _res->_datHdr.cutscenesCount;
		uint32_t hdrOffset = ptrOffset;
		ptrOffset += cutscenesCount * sizeof(DatBitmapsGroup);
		for (int i = 0; i < cutscenesCount; ++i) {
			DatBitmapsGroup *bitmapsGroup = (DatBitmapsGroup *)(ptr + hdrOffset);
			hdrOffset += sizeof(DatBitmapsGroup);
			ptrOffset += bitmapsGroup->w * bitmapsGroup->h + 768;
		}

		for (int i = 0; i < kCheckpointLevelsCount; ++i) {
			DatBitmapsGroup *bitmapsGroup = (DatBitmapsGroup *)(ptr + ptrOffset);
			_checkpointsBitmaps[i] = bitmapsGroup;
			const int count = _res->_datHdr.levelCheckpointsCount[i];
			ptrOffset += count * sizeof(DatBitmapsGroup);
			_checkpointsBitmapsData[i] = ptr + ptrOffset;
			for (int j = 0; j < count; ++j) {
				ptrOffset += bitmapsGroup[j].w * bitmapsGroup[j].h + 768;
			}
		}

		_soundData = ptr + ptrOffset;
		ptrOffset += _res->_datHdr.soundDataSize;

	} else if (version == 11) {

		ptr = _res->_menuBuffer1;
		uint32_t hdrOffset = 4;

		ptrOffset = 4 + (2 + kOptionsCount) * sizeof(DatBitmap);
		ptrOffset += _res->_datHdr.cutscenesCount * sizeof(DatBitmapsGroup);
		for (int i = 0; i < 8; ++i) {
			ptrOffset += _res->_datHdr.levelCheckpointsCount[i] * sizeof(DatBitmapsGroup);
		}
		ptrOffset += _res->_datHdr.levelsCount * sizeof(DatBitmapsGroup);

		_titleBitmapSize = READ_LE_UINT32(ptr + hdrOffset);
		hdrOffset += sizeof(DatBitmap);
		_titleBitmapData = ptr + ptrOffset;
		ptrOffset += _titleBitmapSize + 768;

		_playerBitmapSize = READ_LE_UINT32(ptr + hdrOffset);
		hdrOffset += sizeof(DatBitmap);
		_playerBitmapData = ptr + ptrOffset;
		ptrOffset += _playerBitmapSize + 768;

		for (int i = 0; i < kOptionsCount; ++i) {
			_optionsBitmapSize[i] = READ_LE_UINT32(ptr + hdrOffset);
			hdrOffset += sizeof(DatBitmap);
			_optionsBitmapData[i] = ptr + ptrOffset;
			ptrOffset += _optionsBitmapSize[i] + 768;
		}

		const int cutscenesCount = _res->_datHdr.cutscenesCount;
		for (int i = 0; i < cutscenesCount; ++i) {
			DatBitmapsGroup *bitmapsGroup = (DatBitmapsGroup *)(ptr + hdrOffset);
			hdrOffset += sizeof(DatBitmapsGroup);
			ptrOffset += bitmapsGroup->w * bitmapsGroup->h + 768;
		}

		for (int i = 0; i < kCheckpointLevelsCount; ++i) {
			const int count = _res->_datHdr.levelCheckpointsCount[i];
			_checkpointsBitmaps[i] = (DatBitmapsGroup *)(ptr + hdrOffset);
			_checkpointsBitmapsData[i] = ptr + ptrOffset;
			for (int j = 0; j < count; ++j) {
				DatBitmapsGroup *bitmapsGroup = (DatBitmapsGroup *)(ptr + hdrOffset);
				hdrOffset += sizeof(DatBitmapsGroup);
				ptrOffset += bitmapsGroup->w * bitmapsGroup->h + 768;
			}
		}

		const int levelsCount = _res->_datHdr.levelsCount;
		for (int i = 0; i < levelsCount; ++i) {
			DatBitmapsGroup *bitmapsGroup = (DatBitmapsGroup *)(ptr + hdrOffset);
			hdrOffset += sizeof(DatBitmapsGroup);
			ptrOffset += bitmapsGroup->w * bitmapsGroup->h + 768;
		}
	}

	ptr = _res->_menuBuffer0;
	ptrOffset = 0;

	if (version == 11) {

		_titleSprites = (DatSpritesGroup *)(ptr + ptrOffset);
		_titleSprites->size = le32toh(_titleSprites->size);
		ptrOffset += sizeof(DatSpritesGroup) + _titleSprites->size;

		_playerSprites = (DatSpritesGroup *)(ptr + ptrOffset);
		_playerSprites->size = le32toh(_playerSprites->size);
		ptrOffset += sizeof(DatSpritesGroup) + _playerSprites->size;

		_optionData = ptr + ptrOffset;
		ptrOffset += _res->_datHdr.menusCount * 8;

		const int size = READ_LE_UINT32(ptr + ptrOffset); ptrOffset += 4;
		assert((size % (16 * 10)) == 0);
		_digitTiles = ptr + ptrOffset;
		ptrOffset += size;

		_soundData = ptr + ptrOffset;
		ptrOffset += _res->_datHdr.soundDataSize;
	}

	uint32_t hdrOffset = ptrOffset;
	const int iconsCount = _res->_datHdr.iconsCount;
	ptrOffset += iconsCount * sizeof(DatSpritesGroup);
	for (int i = 0; i < iconsCount; ++i) {
		DatSpritesGroup *spritesGroup = (DatSpritesGroup *)(ptr + hdrOffset);
		hdrOffset += sizeof(DatSpritesGroup);
		ptrOffset += le32toh(spritesGroup->size);
	}

	const int size = READ_LE_UINT32(ptr + ptrOffset); ptrOffset += 4;
	if (size != 0) {
		hdrOffset = ptrOffset;
		ptrOffset += size * 20;
		for (int i = 0; i < size; ++i) {
			DatSpritesGroup *spritesGroup = (DatSpritesGroup *)(ptr + hdrOffset + 4);
			hdrOffset += 20;
			ptrOffset += le32toh(spritesGroup->size);
		}
	}

	if (version == 10) {

		_optionData = ptr + ptrOffset;
		ptrOffset += _res->_datHdr.menusCount * 8;

		hdrOffset = ptrOffset;
		ptrOffset += kOptionsCount * sizeof(DatBitmap);
		for (int i = 0; i < kOptionsCount; ++i) {
			_optionsBitmapSize[i] = READ_LE_UINT32(ptr + hdrOffset);
			hdrOffset += sizeof(DatBitmap);
			_optionsBitmapData[i] = ptr + ptrOffset;
			ptrOffset += _optionsBitmapSize[i] + 768;
		}

		const int levelsCount = _res->_datHdr.levelsCount;
		hdrOffset = ptrOffset;
		ptrOffset += levelsCount * sizeof(DatBitmapsGroup);
		for (int i = 0; i < levelsCount; ++i) {
			DatBitmapsGroup *bitmapsGroup = (DatBitmapsGroup *)(ptr + hdrOffset);
			hdrOffset += sizeof(DatBitmapsGroup);
			ptrOffset += bitmapsGroup->w * bitmapsGroup->h + 768;
		}
	}
}

static const uint8_t *getCheckpointBitmap(const DatBitmapsGroup *bitmapsGroup, const uint8_t *data, int num) {
	for (int i = 0; i < num; ++i) {
		data += bitmapsGroup[i].w * bitmapsGroup[i].h + 768;
	}
	return data;
}

int Menu::getSoundNum(int num) const {
	assert((num & 7) == 0);
	num /= 8;
	if (_soundData) {
		const int count = READ_LE_UINT32(_soundData + 4);
		const uint8_t *p = _soundData + 8 + count * 8;
		for (int i = 0; i < count; ++i) {
			const int count2 = READ_LE_UINT32(_soundData + 8 + i * 8 + 4);
			if (i == num) {
				assert(count2 != 0);
				return (int16_t)READ_LE_UINT16(p);
			}
			p += count2 * 2;
		}
		assert((p - _soundData) == _res->_datHdr.soundDataSize);
	}
	return -1;
}

void Menu::playSound(int num) {
	num = getSoundNum(num);
	if (num != -1) {
		_g->playSound(num, 0, 0, 5);
	}
}

void Menu::drawSprite(const DatSpritesGroup *spriteGroup, uint32_t num) {
	const uint8_t *ptr = (const uint8_t *)&spriteGroup[1];
	for (uint32_t i = 0; i < spriteGroup->count; ++i) {
		const uint16_t size = READ_LE_UINT16(ptr + 2);
		if (num == i) {
			_video->decodeSPR(ptr + 8, _video->_frontLayer, ptr[0], ptr[1], 0, READ_LE_UINT16(ptr + 4), READ_LE_UINT16(ptr + 6));
			break;
		}
		ptr += size + 2;
	}
}

void Menu::drawSpritePos(const DatSpritesGroup *spriteGroup, int x, int y, uint32_t num) {
	const uint8_t *ptr = (const uint8_t *)&spriteGroup[1];
	for (uint32_t i = 0; i < spriteGroup->count; ++i) {
		const uint16_t size = READ_LE_UINT16(ptr + 2);
		if (num == i) {
			if (x == 0 && y == 0) {
				x = ptr[0];
				y = ptr[1];
			}
			_video->decodeSPR(ptr + 8, _video->_frontLayer, x, y, 0, READ_LE_UINT16(ptr + 4), READ_LE_UINT16(ptr + 6));
			break;
		}
		ptr += size + 2;
	}
}

void Menu::refreshScreen(bool updatePalette) {
	if (updatePalette) {
		g_system->setPalette(_paletteBuffer, 256, 6);
	}
	g_system->copyRect(0, 0, Video::W, Video::H, _video->_frontLayer, Video::W);
	g_system->updateScreen(false);
}

bool Menu::mainLoop() {
	loadData();
	return handleTitleScreen();
}

void Menu::drawTitleScreen(int option) {
	const uint32_t uncompressedSize = decodeLZW(_titleBitmapData, _video->_frontLayer);
	assert(uncompressedSize == Video::W * Video::H);
	const uint8_t *palette = _titleBitmapData + _titleBitmapSize;
	g_system->setPalette(palette, 256, 6);
	drawSprite(_titleSprites, option);
	refreshScreen(false);
}

bool Menu::handleTitleScreen() {
	int currentOption = kTitleScreen_Play;
	while (!g_system->inp.quit) {
		if (g_system->inp.keyReleased(SYS_INP_UP)) {
			if (currentOption > kTitleScreen_AssignPlayer) {
				playSound(0x70);
				--currentOption;
			}
		}
		if (g_system->inp.keyReleased(SYS_INP_DOWN)) {
			if (currentOption < kTitleScreen_Quit) {
				playSound(0x70);
				++currentOption;
			}
		}
		if (g_system->inp.keyReleased(SYS_INP_SHOOT) || g_system->inp.keyReleased(SYS_INP_JUMP)) {
			playSound(0x78);
			if (currentOption == kTitleScreen_AssignPlayer) {
				handleAssignPlayer();
			} else if (currentOption == kTitleScreen_Play) {
				return true;
			} else if (currentOption == kTitleScreen_Options) {
				handleOptions(0);
			} else if (currentOption == kTitleScreen_Quit) {
				return false;
			}
		}
		drawTitleScreen(currentOption);
		g_system->processEvents();
		g_system->sleep(15);
	}
	return false;
}

void Menu::drawDigit(int x, int y, int num) {
	assert(x >= 0 && x + 16 < Video::W && y >= 0 && y + 10 < Video::H);
	if (num >= 16) {
		return;
	}
	uint8_t *dst = _video->_frontLayer + y * Video::W + x;
	const uint8_t *src = _digitTiles + num * 16;
	for (int j = 0; j < 10; ++j) {
		for (int i = 0; i < 16; ++i) {
			if (src[i] != 0) {
				dst[i] = src[i];
			}
		}
		dst += Video::W;
		src += Video::W;
	}
}

void Menu::drawBitmap(const DatBitmapsGroup *bitmapsGroup, const uint8_t *bitmapData, int x, int y, int w, int h, uint8_t baseColor) {
	assert(x >= 0 && x + w < Video::W && y >= 0 && y + h < Video::H);
	uint8_t *dst = _video->_frontLayer + y * Video::W + x;
	for (int j = 0; j < h; ++j) {
		for (int i = 0; i < w; ++i) {
			dst[i] = baseColor - bitmapsGroup->colors + bitmapData[i];
		}
		dst += Video::W;
		bitmapData += w;
	}
}

void Menu::setCurrentPlayer(int num) {
	_levelNum = _config->players[num].levelNum;
	if (_levelNum == 8) {
		_checkpointNum = 11;
	} else {
		_checkpointNum = _config->players[num].checkpointNum;
	}
// 422CE0
	// color remapping to 205 done in drawBitmap
	const DatBitmapsGroup *bitmapsGroup = &_checkpointsBitmaps[_levelNum][_checkpointNum];
	uint8_t *dst = _paletteBuffer + 205 * 3;
	const uint8_t *src = getCheckpointBitmap(_checkpointsBitmaps[_levelNum], _checkpointsBitmapsData[_levelNum], _checkpointNum) + bitmapsGroup->w * bitmapsGroup->h;
	memcpy(dst, src, 50 * 3);
// 422D26
	g_system->setPalette(_paletteBuffer, 256, 6);
}

void Menu::drawPlayerProgress(int num, int currentSlot) {
	const uint32_t uncompressedSize = decodeLZW(_playerBitmapData, _video->_frontLayer);
	assert(uncompressedSize == Video::W * Video::H);
	int player = 0;
	for (int y = 96; y < 164; y += 17) {
		if (_config->players[player].cutscenesMask == 0) {
			drawSpritePos(_playerSprites, 82, y - 3, 3); // empty
		} else {
			int levelNum = _config->players[player].levelNum;
			int checkpointNum;
			if (levelNum == 8) { // 'dark'
				levelNum = 7;
				checkpointNum = 11;
			} else {
				checkpointNum = _config->players[player].checkpointNum;
			}
			drawDigit(145, y, levelNum + 1);
			drawDigit(234, y, checkpointNum + 1);
		}
		++player;
	}
// 422DD7
	// TODO
	player = _config->currentPlayer;
// 422EC3
	if (_config->players[player].cutscenesMask != 0) {
		DatBitmapsGroup *bitmapsGroup = &_checkpointsBitmaps[_levelNum][_checkpointNum];
		const uint8_t *src = getCheckpointBitmap(_checkpointsBitmaps[_levelNum], _checkpointsBitmapsData[_levelNum], _checkpointNum);
		drawBitmap(bitmapsGroup, src, 132, 0, bitmapsGroup->w, bitmapsGroup->h, 205);
	}
// 422EFE
	if (currentSlot > 0) {
		if (currentSlot <= 4) {
			drawSpritePos(_playerSprites, 2, currentSlot * 17 + 74, 8);
			drawSpritePos(_playerSprites, 0, 0, 6);
		} else if (currentSlot == 5) {
			drawSpritePos(_playerSprites, 0, 0, 7);
		}
	}
// 422F49
	drawSpritePos(_playerSprites, 0, 0, num);
	if (num > 2) {
		drawSpritePos(_playerSprites, 0, 0, 2);
	}
	refreshScreen(false);
}

void Menu::handleAssignPlayer() {
	memcpy(_paletteBuffer, _playerBitmapData + _playerBitmapSize, 256 * 3);
	int _ebp = 1; // option
	int _ebx = 0; // currentPlayer
//	int _edx = 0;
	int _edi = 5;
	int _ecx;
	setCurrentPlayer(_config->currentPlayer);
	drawPlayerProgress(_config->currentPlayer, 0);
	while (!g_system->inp.quit) {
		if (g_system->inp.keyReleased(SYS_INP_SHOOT) || g_system->inp.keyReleased(SYS_INP_JUMP)) {
			if (_ebp != 0 && _ebx == _edi) {
				playSound(0x80);
			} else {
				playSound(0x78);
			}
// 42301D
			if (_ebp == 0) {
				// return to title screen
				return;
			}
			if (_ebx == 0) {
				_ebx = _config->currentPlayer + 1;
			} else {
				if (_ebx != _edi) {
					if (_ebp == 1) {
						--_ebx;
						_config->currentPlayer = _ebx;
						// _snd_masterVolume
					} else if (_ebp == 2) {
						_ebp = _edi;
					} else if (_ebp == 4) {
					}
				}
// 423123
				_ebx = 0;
			}
// 423125
		}
		if (_ebx != 0 && _ebp < 3) {
			if (g_system->inp.keyReleased(SYS_INP_UP)) {
				if (_ebx > 1) {
					playSound(0x70);
					--_ebx;
					_ecx = _ebx - 1;
					setCurrentPlayer(_ecx);
				}
			}
			if (g_system->inp.keyReleased(SYS_INP_DOWN)) {
				if (_ebx < _edi) {
					playSound(0x70);
					++_ebx;
					if (_ebx == _edi) {
						_ecx = _config->currentPlayer;
					} else {
						_ecx = _ebx - 1;
					}
					setCurrentPlayer(_ecx);
				}
			}
		}
		if (g_system->inp.keyReleased(SYS_INP_LEFT)) {
			if (_ebp == 1 || _ebp == 2 || _ebp == _edi) {
				playSound(0x70);
				--_ebp;
			}
		}
		if (g_system->inp.keyReleased(SYS_INP_RIGHT)) {
			if (_ebp < 2 || _ebp == 4) {
				playSound(0x70);
				++_ebp;
			}
		}
// 4231FF
		drawPlayerProgress(_ebp, _ebx);
		g_system->processEvents();
		g_system->sleep(15);
	}
}

void Menu::handleOptions(int num) { // handleOptionTransition
	const uint8_t *data = &_optionData[num * 8];
	if (data[6] != 0xFF) {
// 428004
	} else {
// 428045
	}
// 428053
	_paf->play(data[5]);
	if (_palNum == 1) {
		_config->players[_config->currentPlayer].levelNum = 0;
		_config->players[_config->currentPlayer].checkpointNum = 0;
	} else if (_palNum == 3) {
		_config->players[_config->currentPlayer].levelNum = _g->_currentLevel;
//		_config->players[_config->currentPlayer].checkpointNum = _levelCheckpoint;
	} else if (_palNum == 5) {
// 4281C3
	} else if (_palNum == 6) {
// 42816B
	} else if (_palNum == 9) {
// 428118
	} else if (_palNum == 18) {
// 4280EA
	} else {
		const uint32_t uncompressedSize = decodeLZW(_optionsBitmapData[_palNum], _video->_frontLayer);
		assert(uncompressedSize == Video::W * Video::H);
	}
}
