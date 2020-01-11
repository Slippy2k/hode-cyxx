
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
	_titleSprites = 0;
	_playerSprites = 0;
	_titleBitmapData = 0;
	_titleBitmapSize = 0;
	_playerBitmapData = 0;
	_playerBitmapSize = 0;
	_soundData = 0;
	_currentOption = 0;
}

void Menu::loadData() {
	_res->loadDatMenuBuffers();

	const int version = _res->_datHdr.version;
	const int optionsCount = 19;

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
		ptrOffset += size;

		const int cutscenesCount = _res->_datHdr.cutscenesCount;
		uint32_t hdrOffset = ptrOffset;
		ptrOffset += cutscenesCount * sizeof(DatBitmapsGroup);
		for (int i = 0; i < cutscenesCount; ++i) {
			DatBitmapsGroup *bitmapsGroup = (DatBitmapsGroup *)(ptr + hdrOffset);
			hdrOffset += sizeof(DatBitmapsGroup);
			ptrOffset += bitmapsGroup->w * bitmapsGroup->h + 768;
		}

		for (int i = 0; i < 8; ++i) {
			const int count = _res->_datHdr.levelCheckpointsCount[i];
			hdrOffset = ptrOffset;
			ptrOffset += count * sizeof(DatBitmapsGroup);
			for (int j = 0; j < count; ++j) {
				DatBitmapsGroup *bitmapsGroup = (DatBitmapsGroup *)(ptr + hdrOffset);
				hdrOffset += sizeof(DatBitmapsGroup);
				ptrOffset += bitmapsGroup->w * bitmapsGroup->h + 768;
			}
		}

		_soundData = ptr + ptrOffset;
		ptrOffset += _res->_datHdr.soundDataSize;

	} else if (version == 11) {

		ptr = _res->_menuBuffer1;
		uint32_t hdrOffset = 4;

		ptrOffset = 4 + (2 + optionsCount) * sizeof(DatBitmap);
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

		ptrOffset += _res->_datHdr.menusCount * 8;

		const int size = READ_LE_UINT32(ptr + ptrOffset); ptrOffset += 4;
		assert((size % (16 * 10)) == 0);
		ptrOffset += size;

		_soundData = ptr + ptrOffset;
		ptrOffset += _res->_datHdr.soundDataSize;
	}
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

void Menu::drawBitmap(const uint8_t *bitmapData, uint32_t bitmapSize, const DatSpritesGroup *spritesGroup) {
	const uint32_t uncompressedSize = decodeLZW(bitmapData, _video->_frontLayer);
	assert(uncompressedSize == Video::W * Video::H);
	const uint8_t *palette = bitmapData + bitmapSize;
	g_system->setPalette(palette, 256, 6);
	drawSprite(spritesGroup, _currentOption);
	g_system->copyRect(0, 0, Video::W, Video::H, _video->_frontLayer, Video::W);
	g_system->updateScreen(false);
}

bool Menu::mainLoop() {
	loadData();
	return handleTitleScreen();
}

bool Menu::handleTitleScreen() {
	while (!g_system->inp.quit) {
		if (g_system->inp.keyReleased(SYS_INP_UP)) {
			if (_currentOption > 0) {
				--_currentOption;
			}
			playSound(0x70);
		}
		if (g_system->inp.keyReleased(SYS_INP_DOWN)) {
			if (_currentOption < 3) {
				++_currentOption;
			}
			playSound(0x70);
		}
		if (g_system->inp.keyReleased(SYS_INP_SHOOT) || g_system->inp.keyReleased(SYS_INP_JUMP)) {
			if (_currentOption == 0) {
				// assign player
			} else if (_currentOption == 1) { // play
				break;
			} else if (_currentOption == 2) {
				// options
			} else if (_currentOption == 3) { // quit
				return false;
			}
			playSound(0x78);
		}
		drawBitmap(_titleBitmapData, _titleBitmapSize, _titleSprites);
		g_system->processEvents();
		g_system->sleep(15);
	}
	return true;
}
