
#include "menu.h"
#include "lzw.h"
#include "paf.h"
#include "resource.h"
#include "system.h"
#include "util.h"
#include "video.h"

Menu::Menu(PafPlayer *paf, Resource *res, Video *video)
	: _paf(paf), _res(res), _video(video) {
	_titleSprites = 0;
	_playerSprites = 0;
	_titleBitmapData = 0;
	_titleBitmapSize = 0;
	_playerBitmapData = 0;
	_titleBitmapSize = 0;
	_currentOption = 0;
}

void Menu::loadData() {
	_res->loadDatMenuBuffers();

	const int version = _res->_datHdr.version;
	const int options = 19;

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

	} else if (version == 11) {

		ptr = _res->_menuBuffer1;
		uint32_t hdrOffset = 4;

		ptrOffset = 4 + (2 + options) * 8; // pointers to bitmaps
		ptrOffset += _res->_datHdr.cutscenesCount * 12;
		for (int i = 0; i < 8; ++i) {
			ptrOffset += _res->_datHdr.levelCheckpointsCount[i] * 12;
		}
		ptrOffset += _res->_datHdr.levelsCount * 12;

		_titleBitmapSize = READ_LE_UINT32(ptr + hdrOffset);
		hdrOffset += 8;
		_titleBitmapData = ptr + ptrOffset;
		ptrOffset += _titleBitmapSize + 768;

		_playerBitmapSize = READ_LE_UINT32(ptr + hdrOffset);
		hdrOffset += 8;
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

void Menu::mainLoop() {
	loadData();
	while (!g_system->inp.quit) {
		if (g_system->inp.keyReleased(SYS_INP_UP)) {
			if (_currentOption > 0) {
				--_currentOption;
			}
		}
		if (g_system->inp.keyReleased(SYS_INP_DOWN)) {
			if (_currentOption < 3) {
				++_currentOption;
			}
		}
		if (g_system->inp.keyReleased(SYS_INP_SHOOT) || g_system->inp.keyReleased(SYS_INP_JUMP)) {
			if (_currentOption == 3) {
				break;
			}
		}
		drawBitmap(_titleBitmapData, _titleBitmapSize, _titleSprites);
		g_system->processEvents();
		g_system->sleep(15);
	}
}
