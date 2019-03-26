/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include "game.h"
#include "fileio.h"
#include "lzw.h"
#include "paf.h"
#include "screenshot.h"
#include "systemstub.h"
#include "util.h"
#include "video.h"

Game::Game(SystemStub *system, const char *dataPath) {
	memset(this, 0, sizeof(Game)); // TODO: proper init
	_difficulty = 1;
	_system = system;
	_res = new Resource(dataPath);
	_paf = new PafPlayer(system, &_res->_fs);
	_video = new Video(system);
	_mstOriginPosX = Video::kScreenWidth / 2;
	_mstOriginPosY = Video::kScreenHeight / 2;
	_shadowScreenMaskBuffer = (uint8_t *)malloc(99328);
	_transformShadowBuffer = 0;
	_transformShadowLayerDelta = 0;
	_mstLogicDisabled = true;
	_playingSssObjectsMax = 16; // 10 if (lowMemory || slowCPU)
	_snd_volumeMax = 64;
	_snd_masterVolume = 128;
}

Game::~Game() {
	delete _paf;
	delete _res;
	delete _video;
	free(_shadowScreenMaskBuffer);
}

void Game::clearObjectScreenDataList() {
	_otherObjectScreenDataList = &_otherObjectScreenDataTable[0];
	for (int i = 0; i < 31; ++i) {
		_otherObjectScreenDataTable[i].nextPtr = &_otherObjectScreenDataTable[i + 1];
	}
	_otherObjectScreenDataTable[31].nextPtr = 0;
}

void Game::prependObjectScreenDataList(LvlObject *ptr) {
	warning("prependObjectScreenDataList unimplemented");
/*
	OtherObjectScreenData *dat = (OtherObjectScreenData *)getLvlObjectDataPtr(ptr, kObjectDataTypeOther);
	dat->nextPtr = _otherObjectScreenDataList;
	_otherObjectScreenDataList = dat;
	ptr->dataPtr = 0;
*/
}

void Game::setShakeScreen(int type, int counter) {
	static const uint8_t table1[] = { 1, 2, 3, 4 };
	static const uint8_t table2[] = { 2, 3, 4, 5 };
	static const uint8_t table3[] = { 3, 4, 5, 8 };
	switch (type) {
	case 1:
		_shakeScreenTable = table1;
		break;
	case 2:
		_shakeScreenTable = table2;
		break;
	case 3:
		_shakeScreenTable = table3;
		break;
	default:
		return;
	}
	_shakeScreenDuration = counter;
}

void Game::fadeScreenPalette() {
	if (_fadePalette == 0) {
		assert(_fadePaletteCounter != 0);
		for (int i = 0; i < 256 * 3; ++i) {
			_fadePaletteBuffer[i] = _video->_displayPaletteBuffer[i] / _fadePaletteCounter;
		}
		_fadePalette = 1;
	} else {
		if (_fadePaletteCounter != 0) {
			_snd_masterVolume -= _snd_masterVolume / _fadePaletteCounter;
			if (_snd_masterVolume < 0) {
				_snd_masterVolume = 0;
			}
		}
		--_fadePaletteCounter;
	}
	for (int i = 0; i < 256 * 3; ++i) {
		int16_t color = _video->_displayPaletteBuffer[i] - _fadePaletteBuffer[i];
		if (color < 0) {
			 color = 0;
		}
		_video->_displayPaletteBuffer[i] = color;
	}
	_video->_paletteNeedRefresh = true;
}

void Game::shakeScreen() {
	if (_video->_displayShadowLayer) {
		const int num = (_currentLevel == 4 || _currentLevel == 6) ? 1 : 4;
		transformShadowLayer(num);
	}
	if (_shakeScreenDuration != 0) {
		--_shakeScreenDuration;
		if (_shakeScreenDuration != 0) {
			const int r1 = _rnd.update() & 3;
			const int r2 = _rnd.update() & 3;
			int dx = _shakeScreenTable[r2] & ~3;
			if (r1 & 1) {
				dx = -dx;
			}
			int dy = _shakeScreenTable[r1];
			if (_shakeScreenDuration & 1) {
				dy = -dy;
			}
			_system->shakeScreen(dx, dy);
		}
	}
	if (_fadePaletteCounter != 0) {
		fadeScreenPalette();
	}
}

static BoundingBox _screenTransformRects[] = {
	{  0,  0,   0,   0 },
	{  0,  0, 255, 128 },
	{  0, 10, 154, 126 },
	{  0,  0, 107,  36 },
	{  0,  1,  78,  29 },
	{ 14,  7, 249,  72 },
	{ 14,  0, 255,  72 },
	{  0,  0, 255, 144 },
	{  0,  0, 255, 144 },
	{  0, 69, 255, 191 }
};

void Game::transformShadowLayer(int delta) {
	const uint8_t *src = _transformShadowBuffer + _transformShadowLayerDelta; // _esi
	uint8_t *dst = _video->_shadowLayer; // _eax
	_transformShadowLayerDelta += delta; // overflow/wrap at 255
	for (int y = 0; y < 192; ++y) {
		for (int x = 0; x < 250; ++x) {
			const int offset = x + *src++;
			*dst++ = _video->_frontLayer[y * 256 + offset];
		}
		memset(dst, 0xC4, 6);
		dst += 6;
		src += 6;
	}
	int r = -1;
	if (_currentLevel == kLvl_pwr1) {
		r = _pwr1_screenTransformLut[_res->_currentScreenResourceNum * 2 + 1];
	} else if (_currentLevel == kLvl_pwr2) {
		r = _pwr2_screenTransformLut[_res->_currentScreenResourceNum * 2 + 1];
	}
	if (!(r < 0)) {
		const BoundingBox *b = &_screenTransformRects[r];
		const int offset = b->y1 * 256 + b->x1;
		src = _video->_frontLayer + offset;
		dst = _video->_shadowLayer + offset;
		for (int y = b->y1; y < b->y2; ++y) {
			for (int x = b->x1; x < b->x2; ++x) {
				dst[x] = src[x];
			}
			dst += 256;
			src += 256;
		}
	}
}

void Game::decodeShadowScreenMask(LvlBackgroundData *lvl) {
	uint8_t *dst = _shadowScreenMaskBuffer;
	for (int i = lvl->currentDataUnk1Id; i < lvl->dataUnk1Count; ++i) {
		uint8_t *src = lvl->backgroundMaskTable[i];
		if (src) {
			const int decodedSize = decodeLZW(src + 2, dst);

			_shadowScreenMasksTable[i].dataSize = READ_LE_UINT32(dst);

			// header : 20 bytes
			// projectionData : w * h * sizeof(uint16_t) - for a given (x, y) returns the casted (x, y)
			// paletteData : 256 (only the first 144 bytes are read)

			_shadowScreenMasksTable[i].projectionDataPtr = dst + 0x14 + READ_LE_UINT32(dst + 4);
			_shadowScreenMasksTable[i].shadowPalettePtr = dst + 0x14 + READ_LE_UINT32(dst + 8);
			const int x = _shadowScreenMasksTable[i].x = READ_LE_UINT16(dst + 0xC);
			const int y = _shadowScreenMasksTable[i].y = READ_LE_UINT16(dst + 0xE);
			const int w = _shadowScreenMasksTable[i].w = READ_LE_UINT16(dst + 0x10);
			const int h = _shadowScreenMasksTable[i].h = READ_LE_UINT16(dst + 0x12);

			debug(kDebug_GAME, "shadow screen mask #%d pos %d,%d dim %d,%d size %d", i, x, y, w, h, decodedSize);

			const int size = w * h;
			src = _shadowScreenMasksTable[i].projectionDataPtr + 2;
			for (int j = 1; j < size; ++j) {
				const int16_t offset = (int16_t)READ_LE_UINT16(src - 2) + (int16_t)READ_LE_UINT16(src);
				// fprintf(stdout, "shadow #%d offset #%d 0x%x 0x%x\n", i, j, READ_LE_UINT16(src), offset);
				WRITE_LE_UINT16(src, offset);
				src += 2;
			}

			const int shadowPaletteSize = decodedSize - 20 - w * h * sizeof(uint16_t);
			assert(shadowPaletteSize >= 144);

			_video->buildShadowColorLookupTable(_shadowScreenMasksTable[i].shadowPalettePtr, _video->_shadowColorLookupTable);
			dst += decodedSize;
		}
	}
}

void Game::playSound(int num, LvlObject *ptr, int a, int b) {
	if (num < _res->_sssHdr.dataUnk1Count) {
		debug(kDebug_GAME, "playSound num %d/%d a=%d b=%d", num, _res->_sssHdr.dataUnk1Count, a, b);
		_currentSoundLvlObject = ptr;
		playSoundObject(&_res->_sssDataUnk1[num], a, b);
		_currentSoundLvlObject = 0;
	}
}

void Game::removeSound(LvlObject *ptr) {
	for (int i = 0; i < _sssObjectsCount; ++i) {
		if (_sssObjectsTable[i].lvlObject == ptr) {
			_sssObjectsTable[i].lvlObject = 0;
		}
	}
	ptr->sssObj = 0;
}

void Game::setupBackgroundBitmap() {
	LvlBackgroundData *lvl = &_res->_resLvlScreenBackgroundDataTable[_res->_currentScreenResourceNum];
	const int num = lvl->currentBackgroundId;
	const uint8_t *pal = lvl->backgroundPaletteTable[num];
	lvl->backgroundPaletteId = READ_LE_UINT16(pal); pal += 2;
	const uint8_t *pic = lvl->backgroundBitmapTable[num];
	lvl->backgroundBitmapId = READ_LE_UINT16(pic); pic += 2;
	if (lvl->backgroundPaletteId != 0xFFFF) {
		playSound(lvl->backgroundPaletteId, 0, 0, 3);
	}
	if (lvl->backgroundBitmapId != 0xFFFF) {
		playSound(lvl->backgroundBitmapId, 0, 0, 3);
	}
	decodeLZW(pic, _video->_backgroundLayer);
	if (lvl->dataUnk1Count != 0) {
		decodeShadowScreenMask(lvl);
	}
	for (int i = 0; i < 256 * 3; ++i) {
		_video->_displayPaletteBuffer[i] = (*pal++) << 8;
	}
	_video->_paletteNeedRefresh = true;
}

void Game::addToSpriteList(LvlObject *ptr) {
	Sprite *spr = _gameSpriteListHead;
	if (spr) {
		uint8_t rightScreenId  = _res->_screensGrid[_res->_currentScreenResourceNum * 4 + kPosRightScreen];
		uint8_t topScreenId    = _res->_screensGrid[_res->_currentScreenResourceNum * 4 + kPosTopScreen];
		uint8_t bottomScreenId = _res->_screensGrid[_res->_currentScreenResourceNum * 4 + kPosBottomScreen];
		uint8_t leftScreenId   = _res->_screensGrid[_res->_currentScreenResourceNum * 4 + kPosLeftScreen];

		LvlObjectData *dat = ptr->levelData0x2988;
		LvlAnimHeader *ah = (LvlAnimHeader *)(dat->animsInfoData + kLvlAnimHdrOffset) + ptr->anim;
		LvlAnimSeqHeader *ash = (LvlAnimSeqHeader *)(dat->animsInfoData + ah->seqOffset) + ptr->frame;

		spr->num = (((ash->flags1 ^ ptr->flags1) & 0xFFF0) << 10) | (ptr->flags2 & 0x3FFF);

		int index = ptr->screenNum;
		spr->xPos = ptr->xPos;
		spr->yPos = ptr->yPos;
		if (index == topScreenId) {
			spr->yPos -= 192;
		} else if (index == bottomScreenId) {
			spr->yPos += 192;
		} else if (index == rightScreenId) {
			spr->xPos += 256;
		} else if (index == leftScreenId) {
			spr->xPos -= 256;
		} else if (index != _res->_currentScreenResourceNum) {
			return;
		}
		if (spr->xPos >= 256 || spr->xPos + ptr->width < 0) {
			return;
		}
		if (spr->yPos >= 192 || spr->yPos + ptr->height < 0) {
			return;
		}
		if (_currentLevel == kLvl_isld && ptr->data0x2988 == 2) {
			AndyObjectScreenData *dataPtr = (AndyObjectScreenData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
			spr->xPos += dataPtr->dxPos;
		}
		if (READ_LE_UINT16(ptr->bitmapBits) > 8) {
			spr->bitmapBits = ptr->bitmapBits;
			_gameSpriteListHead = spr->nextPtr;
			index = (ptr->flags2 & 31);
			spr->nextPtr = _gameSpriteListPtrTable[index];
			_gameSpriteListPtrTable[index] = spr;
		}
	}
}

int16_t Game::calcScreenMaskDy(int16_t xPos, int16_t yPos, int num) {
	if (xPos < 0) {
		xPos += 256;
		num = _res->_screensGrid[num * 4 + kPosLeftScreen];
	} else if (xPos >= 256) {
		xPos -= 256;
		num = _res->_screensGrid[num * 4 + kPosRightScreen];
	}
	if (num != 0xFF && yPos < 0) {
		yPos += 192;
		num = _res->_screensGrid[num * 4 + kPosTopScreen];
	} else if (yPos >= 192) {
		assert(num != 0xFF);
		yPos -= 192;
		num = _res->_screensGrid[num * 4 + kPosBottomScreen];
	}
	uint8_t var1 = 0xFF - (yPos & 7);
	if (num == 0xFF) {
		return var1;
	}
	int __esi = _res->_screensBasePos[num].v + yPos; // y
	int __eax = _res->_screensBasePos[num].u + xPos; // x
	int _esi = ((__esi << 6) & ~511) + (__eax >> 3); // screenMaskPos (8x8)
	int _edi = ((yPos & ~7) << 2) + (xPos >> 3); // screenPos (8x8)
	if (_screenMaskBuffer[_esi - 512] & 1) {
		_edi -= 32;
		var1 -= 8;
	} else if (_screenMaskBuffer[_esi] & 1) {
		/* nothing to do */
	} else if (_screenMaskBuffer[_esi + 512] & 1) {
		_edi += 32;
		var1 += 8;
	} else if (_screenMaskBuffer[_esi + 1024] & 1) {
		_edi += 64;
		var1 += 16;
	} else {
		return 0;
	}
	int _dl = 1; // screen
	while (_res->_screensGrid[_res->_currentScreenResourceNum * 4 + _dl - 1] != num) {
		++_dl;
		if (_dl >= 4) {
			if (num == _res->_currentScreenResourceNum) {
				break;
			}
			return (int8_t)(var1 + 4);
		}
	}
	const uint8_t *p = _res->_resLevelData0x470CTablePtrData + (xPos & 7);
	return (int8_t)(var1 + p[_screenPosTable[_dl][_edi] * 8]);
}

void Game::setupScreenPosTable(uint8_t num) {
	const uint8_t *src = &_res->_screensGrid[num * 4];
	for (int i = 0; i < 4; ++i) {
		if (src[i] != 0xFF) {
			int index = _res->_resLvlScreenBackgroundDataTable[src[i]].currentMaskId;
			const uint8_t *p = _res->getLevelData0x470CPtr4(src[i] * 4 + index);
			if (p) {
				Video::decodeRLE(p, _screenPosTable[i], 768);
				continue;
			}
		}
		memset(_screenPosTable[i], 0, 768);
	}
	int index = _res->_resLvlScreenBackgroundDataTable[num].currentMaskId;
	const uint8_t *p = _res->getLevelData0x470CPtr4(num * 4 + index);
	if (p) {
		Video::decodeRLE(p, _screenPosTable[4], 768);
	} else {
		memset(_screenPosTable[4], 0, 768);
	}
}

void Game::setupScreenMask(uint8_t num) {
	if (num == 0xFF) {
		return;
	}
	int mask = _res->_resLvlScreenBackgroundDataTable[num].currentMaskId;
	if (_res->_screensState[num].s3 != mask) {
		debug(kDebug_GAME, "setupScreenMask num %d mask %d", num, mask);
		_res->_screensState[num].s3 = mask;
		uint8_t *p = _res->getLevelData0x470CPtr0(num * 4 + mask);
		if (p) {
			Video::decodeRLE(p, _screenTempMaskBuffer, 768);
		} else {
			memset(_screenTempMaskBuffer, 0, 768);
		}
		const int offs = ((_res->_screensBasePos[num].v & ~7) << 6) + (_res->_screensBasePos[num].u >> 3);
		p = _screenMaskBuffer + offs;
		for (int i = 0; i < 24; ++i) {
			memcpy(p, _screenTempMaskBuffer + i * 32, 32);
			p += 512;
		}
	}
	if (_res->_currentScreenResourceNum == num) {
		setupScreenPosTable(num);
	}
}

void Game::resetScreenMask() {
	memset(_screenMaskBuffer, 0, sizeof(_screenMaskBuffer));
	for (int i = 0; i < _res->_lvlHdr.screensCount; ++i) {
		_res->_screensState[i].s3 = 0xFF;
		setupScreenMask(i);
	}
}

void Game::setScreenMaskRect(int x1, int y1, int x2, int y2, int pos) {
	const int u = _res->_screensBasePos[pos].u;
	const int v = _res->_screensBasePos[pos].v;
	const int screen = _res->_screensGrid[pos * 4 + kPosTopScreen];
	if (x1 < u || y1 < v || y2 >= v + 192) {
		if (screen != 255) {
			if (x1 >= u && y1 >= v) {
				if (y2 < v + 192) {
					// (x1, y1, x2, v + 192, screen);
				}
			}
		}
	}
// 40C111
	// (x1, v, x2, u, screen);
}

void Game::setupLvlObjectBitmap(LvlObject *ptr) {
	LvlObjectData *dat = ptr->levelData0x2988;
	if (!dat) {
		return;
	}
	LvlAnimHeader *ah = (LvlAnimHeader *)(dat->animsInfoData + kLvlAnimHdrOffset) + ptr->anim;
	LvlAnimSeqHeader *ash = (LvlAnimSeqHeader *)(dat->animsInfoData + ah->seqOffset) + ptr->frame;

	ptr->soundToPlay = ash->sound;
	ptr->flags0 = bitmask_set(ptr->flags0, ash->flags0, 0x3FF);
	ptr->flags1 = bitmask_set(ptr->flags1, ash->flags1, 6);
	ptr->flags1 = bitmask_set(ptr->flags1, ash->flags1, 8);
	ptr->unk22 = ash->firstFrame;

	ptr->bitmapBits = _res->getLvlSpriteFramePtr(dat, ash->firstFrame);

	ptr->width = READ_LE_UINT16(ptr->bitmapBits + 2);
	ptr->height = READ_LE_UINT16(ptr->bitmapBits + 4);

	const int w = ptr->width - 1;
	const int h = ptr->height - 1;

	if (ptr->type == 8 && (ptr->data0x2988 == 2 || ptr->data0x2988 == 0)) {
		AndyObjectScreenData *dataPtr = (AndyObjectScreenData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
		dataPtr->boundingBox.x1 = ptr->xPos;
		dataPtr->boundingBox.y1 = ptr->yPos;
		dataPtr->boundingBox.x2 = ptr->xPos + w;
		dataPtr->boundingBox.y2 = ptr->yPos + h;
	}

	const LvlSprHotspotData *hotspot = ((LvlSprHotspotData *)dat->hotspotsData) + ash->firstFrame;
	const int type = (ptr->flags1 >> 4) & 3;
	for (int i = 0; i < 8; ++i) {
		switch (type) {
		case 0:
			ptr->posTable[i].x = hotspot->pts[i].x;
			ptr->posTable[i].y = hotspot->pts[i].y;
			break;
		case 1:
			ptr->posTable[i].x = w - hotspot->pts[i].x;
			ptr->posTable[i].y = hotspot->pts[i].y;
			break;
		case 2:
			ptr->posTable[i].x = hotspot->pts[i].x;
			ptr->posTable[i].y = h - hotspot->pts[i].y;
			break;
		case 3:
			ptr->posTable[i].x = w - hotspot->pts[i].x;
			ptr->posTable[i].y = h - hotspot->pts[i].y;
			break;
		}
	}
}

void Game::randomizeInterpolatePoints(int32_t *pts, int count) {
	int32_t rnd = _rnd.update();
	for (int i = 0; i < count; ++i) {
		const int index = _pointDstIndexTable[i];
		const int c1 = pts[_pointSrcIndex1Table[index]];
		const int c2 = pts[_pointSrcIndex2Table[index]];
		pts[index] = (c1 + c2 + (rnd >> _pointRandomizeShiftTable[i])) / 2;
		rnd *= 2;
	}
}

int Game::fixPlasmaCannonPointsScreenMask(int num) {
	uint8_t _dl = ((~_plasmaCannonDirection) & 1) | 6;
	int var1 = _plasmaCannonFirstIndex;
	int _edi = _res->_screensBasePos[num].u;
	int _ebp = _res->_screensBasePos[num].v;
	uint8_t _al = 0;
	if (_andyObject->anim == 84) {
		int yPos, xPos;
		yPos = _plasmaCannonYPointsTable1[var1];
		if (yPos < 0) {
			yPos = 0;
		}
		yPos += _ebp;
		yPos <<= 6;
		yPos &= ~511;
		while ((xPos = _plasmaCannonXPointsTable1[var1]) >= 0) {
			xPos += _edi;
			xPos >>= 3;
			_al = _screenMaskBuffer[xPos + yPos];
			if ((_al & _dl) != 0) {
				_plasmaCannonLastIndex1 = var1;
				break;
			}
			var1 += 4;
			if (var1 >= _plasmaCannonLastIndex2) {
				break;
			}
		}
	} else {
		int xPos, yPos;
		while ((xPos = _plasmaCannonXPointsTable1[var1]) >= 0 && (yPos = _plasmaCannonYPointsTable1[var1]) >= 0) {
			yPos += _ebp;
			yPos <<= 6;
			yPos &= ~511;
			xPos += _edi;
			xPos >>= 3;
			_al = _screenMaskBuffer[yPos + xPos];
			if ((_al & _dl) != 0) {
				_plasmaCannonLastIndex1 = var1;
				break;
			}
			var1 += 4;
			if (var1 >= _plasmaCannonLastIndex2) {
				break;
			}
		}
	}
	return _al;
}

void Game::setupPlasmaCannonPointsHelper() {
	if (_plasmaCannonPointsSetupCounter == 0) {
		int xR = _rnd.update();
		for (int i = 0; i < 64; ++i) {
			const int index = _pointDstIndexTable[i];
			const int x1 = _gameXPosTable[_pointSrcIndex1Table[index]];
			const int x2 = _gameXPosTable[_pointSrcIndex2Table[index]];
			_gameXPosTable[index] = (x1 + x2 + (xR >> _pointRandomizeShiftTable[i])) / 2;
			xR *= 2;
		}
		int yR = _rnd.update();
		for (int i = 0; i < 64; ++i) {
			const int index = _pointDstIndexTable[i];
			const int y1 = _gameYPosTable[_pointSrcIndex1Table[index]];
			const int y2 = _gameYPosTable[_pointSrcIndex2Table[index]];
			_gameYPosTable[index] = (y1 + y2 + (yR >> _pointRandomizeShiftTable[i])) / 2;
			yR *= 2;
		}
		if (_andyObject->anim == 84) {
			for (int i = 0; i <= 496; i += 8) {
				const int index = i / 4;
				_plasmaCannonXPointsTable2[2 + index] = (_gameXPosTable[4 + index] - _plasmaCannonXPointsTable1[4 + index]) / 2;
				_plasmaCannonYPointsTable2[2 + index] = (_gameYPosTable[4 + index] - _plasmaCannonYPointsTable1[4 + index]) / 8;
			}
		} else {
			for (int i = 0; i <= 496; i += 8) {
				const int index = i / 4;
				_plasmaCannonXPointsTable2[2 + index] = (_gameXPosTable[4 + index] - _plasmaCannonXPointsTable1[4 + index]) / 2;
				_plasmaCannonYPointsTable2[2 + index] = (_gameYPosTable[4 + index] - _plasmaCannonYPointsTable1[4 + index]) / 2;
			}
		}
		for (int i = 0; i <= 504; i += 8) {
			const int index = i / 4;
			_plasmaCannonXPointsTable1[2 + index] = _gameXPosTable[2 + index];
			_plasmaCannonYPointsTable1[2 + index] = _gameYPosTable[2 + index];
		}
	} else {
		for (int i = 0; i <= 496; i += 8) {
			const int index = i / 4;
			_plasmaCannonXPointsTable1[4 + index] += _plasmaCannonXPointsTable2[2 + index];
			_plasmaCannonYPointsTable1[4 + index] += _plasmaCannonYPointsTable2[2 + index];
		}
	}
	++_plasmaCannonPointsSetupCounter;
	if (_plasmaCannonPointsSetupCounter >= 2) {
		_plasmaCannonPointsSetupCounter = 0;
	}
	_plasmaCannonLastIndex2 = 128;
	if (_plasmaCannonDirection == 0) {
		_plasmaCannonLastIndex1 = _plasmaCannonPointsMask = 0;
		_plasmaCannonFirstIndex = 128;
	} else {
		_plasmaCannonFirstIndex = 0;
		_plasmaCannonPointsMask = fixPlasmaCannonPointsScreenMask(_res->_currentScreenResourceNum);
	}
}

void Game::destroyLvlObjectPlasmaExplosion(LvlObject *o) {
	AndyObjectScreenData *l = (AndyObjectScreenData *)getLvlObjectDataPtr(o, kObjectDataTypeAndy);
	if (l->nextPtr) {
		l->nextPtr = 0;
		assert(_plasmaExplosionObject);
		removeLvlObjectFromList(&_plasmaExplosionObject, _plasmaExplosionObject->nextPtr);
		destroyLvlObject(_plasmaExplosionObject->nextPtr);
		removeLvlObjectFromList(&_plasmaExplosionObject, _plasmaExplosionObject);
		destroyLvlObject(_plasmaExplosionObject);
	}
}

void Game::shuffleArray(uint8_t *p, int count) {
	for (int i = 0; i < count * 2; ++i) {
		const int index1 = _rnd.update() % count;
		const int index2 = _rnd.update() % count;
		SWAP(p[index1], p[index2]);
	}
}

void Game::shuffleDword(uint8_t *p) {
	p[0] = _rnd.update() & 7;
	p[1] = _rnd.update() & 31;
	p[2] = 0x20;
}

uint8_t Game::shuffleFlags(uint8_t *p) {
	const uint8_t code = _mstRandomLookupTable[p[0]][p[1]];
	++p[1];
	if (p[1] >= 32) {
		p[1] = 0;
	}
	--p[2];
	if (p[2] == 0) {
		++p[0];
		if (p[0] >= 8) {
			p[0] = 0;
		}
		p[2] = 0x20;
		p[1] = _rnd.update() & 31;
	}
	return code;
}

void Game::destroyLvlObject(LvlObject *o) {
	if (!o) {
		warning("destroyLvlObject called with NULL lvlObject");
		return;
	}
	if (o->type == 8) {
		_res->decLevelData0x2988RefCounter(o);
		o->nextPtr = _declaredLvlObjectsListHead;
		--_declaredLvlObjectsListCount;
		_declaredLvlObjectsListHead = o;
		switch (o->data0x2988) {
		case 0:
		case 2:
			o->dataPtr = 0;
			break;
		case 3:
		case 7:
			if (o->dataPtr) {
				prependObjectScreenDataList(o);
			}
			break;
		}
	}
	if (o->sssObj) {
		removeSound(o);
	}
	o->sssObj = 0;
	o->bitmapBits = 0;
}

void Game::setupPlasmaCannonPoints(LvlObject *ptr) {
	_plasmaCannonDirection = 0;
	if ((ptr->flags0 & 0x1F) == 4) {
		if ((ptr->actionKeyMask & 4) == 0) { // not using plasma cannon
			destroyLvlObjectPlasmaExplosion(ptr);
		} else {
			_gameXPosTable[0] = _gameXPosTable[128] = ptr->xPos + ptr->posTable[6].x;
			_gameYPosTable[0] = _gameYPosTable[128] = ptr->yPos + ptr->posTable[6].y;
			const int num = ((ptr->flags0 >> 5) & 7) - 3;
			switch (num) {
			case 0:
				_gameYPosTable[128] -= 176; // 192 - 16
				_plasmaCannonDirection = 3;
				break;
			case 1:
				_gameYPosTable[128] += 176;
				_plasmaCannonDirection = 6;
				break;
			case 3:
				_gameYPosTable[128] -= 176;
				_plasmaCannonDirection = 1;
				break;
			case 4:
				_gameYPosTable[128] += 176;
				_plasmaCannonDirection = 4;
				break;
			default:
				_plasmaCannonDirection = 2;
				break;
			}
			if (ptr->flags1 & 0x10) {
				if (_plasmaCannonDirection != 1) {
					_plasmaCannonDirection = (_plasmaCannonDirection & ~2) | 8;
					_gameXPosTable[128] -= 264; // 256 + 8
				}
			} else {
				if (_plasmaCannonDirection != 1) {
					_gameXPosTable[128] += 264;
				}
			}
			if (_plasmaCannonPrevDirection != _plasmaCannonDirection) {
				_plasmaCannonXPointsTable1[0] = _gameXPosTable[0];
				_plasmaCannonXPointsTable1[128] = _gameXPosTable[128];
				randomizeInterpolatePoints(_plasmaCannonXPointsTable1, 64);
				_plasmaCannonYPointsTable1[0] = _gameYPosTable[0];
				_plasmaCannonYPointsTable1[128] = _gameYPosTable[128];
				randomizeInterpolatePoints(_plasmaCannonYPointsTable1, 64);
				_plasmaCannonPrevDirection = _plasmaCannonDirection;
			}
		}
	}
	if (_plasmaCannonPrevDirection != 0) {
		setupPlasmaCannonPointsHelper();
		if (_plasmaCannonFirstIndex >= _plasmaCannonLastIndex2) {
			_plasmaCannonPrevDirection = 0;
			_plasmaCannonLastIndex2 = 16;
		}
	}
}

int Game::testPlasmaCannonPointsDirection(int x1, int y1, int x2, int y2) {
	int index1 = _plasmaCannonFirstIndex;
	int _esi = _gameXPosTable[index1];
	int _ebp = _gameYPosTable[index1];
	int index2 = _plasmaCannonLastIndex1;
	if (index2 == 0) {
		index2 = _plasmaCannonLastIndex2;
	}
	int _eax = _plasmaCannonXPointsTable1[index2];
	int _edi = _plasmaCannonYPointsTable1[index2];
	if (_esi > _eax) {
		if (_ebp > _edi) {
			if (x1 > _esi || x2 < _eax || y1 > _ebp || y2 < _edi) {
				return 0;
			}
			index1 += 4;
			do {
				_eax = _plasmaCannonXPointsTable1[index1];
				_edi = _plasmaCannonYPointsTable1[index1];
				if (x1 <= _esi && x2 >= _eax && y1 <= _ebp && y2 >= _edi) {
					goto endDir;
				}
				_esi = _eax;
				_ebp = _edi;
				index1 += 4;
			} while (index1 < index2);
			return 0;
		} else {
			if (x1 > _esi || x2 < _eax || y1 > _edi || y2 < _ebp) {
				return 0;
			}
			index1 += 4;
			do {
				_eax = _plasmaCannonXPointsTable1[index1];
				_edi = _plasmaCannonYPointsTable1[index1];
				if (x1 <= _esi && x2 >= _eax && y1 <= _edi && y2 >= _ebp) {
					goto endDir;
				}
				_esi = _eax;
				_ebp = _edi;
				index1 += 4;
			} while (index1 < index2);
			return 0;
		}
	} else {
		if (_ebp > _edi) {
			if (x1 > _eax || x2 < _esi || y1 > _ebp || y2 < _edi) {
				return 0;
			}
			index1 += 4;
			do {
				_eax = _plasmaCannonXPointsTable1[index1];
				_edi = _plasmaCannonYPointsTable1[index1];
				if (x1 <= _eax && x2 >= _esi && y1 <= _ebp && y2 >= _edi) {
					goto endDir;
				}
				_esi = _eax;
				_ebp = _edi;
				index1 += 4;
			} while (index1 < index2);
			return 0;
		} else {
			if (x1 > _eax || x2 < _esi || y1 > _edi || y2 < _ebp) {
				return 0;
			}
			index1 += 4;
			do {
				_eax = _plasmaCannonXPointsTable1[index1];
				_edi = _plasmaCannonYPointsTable1[index1];
				if (x1 <= _eax && x2 >= _esi && y1 <= _edi && y2 >= _ebp) {
					goto endDir;
				}
				_esi = _eax;
				_ebp = _edi;
				index1 += 4;
			} while (index1 < index2);
			return 0;
		}
	}
endDir:
	_plasmaCannonLastIndex1 = index1;
	_plasmaCannonPointsMask = 0;
	return 1;
}

void Game::preloadLevelScreenData(int num, int prev) {
	_res->loadLvlScreenBackgroundData(num);
	loadLevelScreenSounds(num);
}

void Game::loadLevelScreenSounds(int num) {
	if (_res->_sssHdr.pcmCount > 0 && _res->_sssPreloadData1) {
		for (size_t i = 0; i < _res->_sssPreloadData1[num].count; ++i) {
			const int j = _res->_sssPreloadData1[num].ptr[i];
			debug(kDebug_GAME, "levelScreen preloadData1 #%d res %d", i, j);
		}
	}
	if (_res->_lvlHdr.spritesCount > 0 && _res->_sssPreloadData2) {
		for (size_t i = 0; i < _res->_sssPreloadData2[num].count; ++i) {
			const int j = _res->_sssPreloadData2[num].ptr[i];
			debug(kDebug_GAME, "levelScreen preloadData2 #%d res %d", i, j);
		}
	}
	if (_res->_sssHdr.preloadInfoCount > 0 && _res->_sssPreloadData3) {
		for (size_t i = 0; i < _res->_sssPreloadData3[num].count; ++i) {
			const int j = _res->_sssPreloadData3[num].ptr[i];
			debug(kDebug_GAME, "levelScreen preloadData3 #%d res %d", i, j);
		}
	}
}

void Game::setLvlObjectPosRelativeToObject(LvlObject *ptr1, int num1, LvlObject *ptr2, int num2) {
	ptr1->xPos = ptr2->posTable[num2].x - ptr1->posTable[num1].x + ptr2->xPos;
	ptr1->yPos = ptr2->posTable[num2].y - ptr1->posTable[num1].y + ptr2->yPos;
}

void Game::setLvlObjectPosRelativeToPoint(LvlObject *ptr, int num, int x, int y) {
	assert(num >= 0 && num < 8);
	ptr->xPos = x - ptr->posTable[num].x;
	ptr->yPos = y - ptr->posTable[num].y;
}

void Game::clearLvlObjectsList1() {
	if (!_lvlObjectsList1) {
		return;
	}
	for (int i = 0; i < 32; ++i) {
		resetMstUnkData(&_mstUnkDataTable[i]);
	}
	for (int i = 0; i < 64; ++i) {
		resetMstObject(&_mstObjectsTable[i]);
	}
	LvlObject *ptr = _lvlObjectsList1;
	while (ptr) {
		LvlObject *next = ptr->nextPtr;
		if (ptr->type == 8) {
			_res->decLevelData0x2988RefCounter(ptr);
			ptr->nextPtr = _declaredLvlObjectsListHead;
			--_declaredLvlObjectsListCount;
			_declaredLvlObjectsListHead = ptr;
			switch (ptr->data0x2988) {
			case 0:
			case 2:
				ptr->dataPtr = 0;
				break;
			case 3:
			case 7:
				if (ptr->dataPtr) {
					prependObjectScreenDataList(ptr);
				}
				break;
			}
			if (ptr->sssObj) {
				removeSound(ptr);
			}
			ptr->sssObj = 0;
			ptr->bitmapBits = 0;
		}
		ptr = next;
	}
	_lvlObjectsList1 = 0;
}

void Game::clearLvlObjectsList2() {
	LvlObject *ptr = _lvlObjectsList2;
	while (ptr) {
		LvlObject *next = ptr->nextPtr;
		if (ptr->type == 8) {
			_res->decLevelData0x2988RefCounter(ptr);
			ptr->nextPtr = _declaredLvlObjectsListHead;
			--_declaredLvlObjectsListCount;
			_declaredLvlObjectsListHead = ptr;
			switch (ptr->data0x2988) {
			case 0:
			case 2:
				ptr->dataPtr = 0;
				break;
			case 3:
			case 7:
				if (ptr->dataPtr) {
					prependObjectScreenDataList(ptr);
				}
				break;
			}
			if (ptr->sssObj) {
				removeSound(ptr);
			}
			ptr->sssObj = 0;
			ptr->bitmapBits = 0;
		}
		ptr = next;
	}
	_lvlObjectsList2 = 0;
}

void Game::clearLvlObjectsList3() {
	LvlObject *ptr = _lvlObjectsList3;
	while (ptr != 0) {
		LvlObject *next = ptr->nextPtr;
		if (ptr->type == 8) {
			_res->decLevelData0x2988RefCounter(ptr);
			ptr->nextPtr = _declaredLvlObjectsListHead;
			--_declaredLvlObjectsListCount;
			_declaredLvlObjectsListHead = ptr;
			switch (ptr->data0x2988) {
			case 0:
			case 2:
				ptr->dataPtr = 0;
				break;
			case 3:
			case 7:
				if (ptr->dataPtr) {
					prependObjectScreenDataList(ptr);
				}
				break;
			}
			if (ptr->sssObj) {
				removeSound(ptr);
			}
			ptr->sssObj = 0;
			ptr->bitmapBits = 0;
		}
		ptr = next;
	}
	_lvlObjectsList3 = 0;
}

LvlObject *Game::addLvlObjectToList1(int type, int num) {
	if ((type != 8 || _res->_resLevelData0x2988PtrTable[num] != 0) && _declaredLvlObjectsListCount < 160) {
		LvlObject *ptr = _declaredLvlObjectsListHead;
		_declaredLvlObjectsListHead = _declaredLvlObjectsListHead->nextPtr;
		++_declaredLvlObjectsListCount;
		ptr->data0x2988 = num;
		ptr->type = type;
		if (type == 8) {
			_res->incLevelData0x2988RefCounter(ptr);
			lvlObjectTypeCallback(ptr);
		}
		ptr->unk22 = 0;
		ptr->sssObj = 0;
		ptr->nextPtr = 0;
		ptr->bitmapBits = 0;
		ptr->nextPtr = _lvlObjectsList1;
		_lvlObjectsList1 = ptr;
		return ptr;
	}
	return 0;
}

int Game::addLvlObjectToList3(int num) {
	if (_res->_resLevelData0x2988PtrTable[num] == 0 && _declaredLvlObjectsListCount < 160) {
		assert(_declaredLvlObjectsListHead);
		LvlObject *ptr = _declaredLvlObjectsListHead;
		_declaredLvlObjectsListHead = _declaredLvlObjectsListHead->nextPtr;
		assert(ptr);
		++_declaredLvlObjectsListCount;
		ptr->data0x2988 = 1;
		ptr->type = 8;
		_res->incLevelData0x2988RefCounter(ptr);
		ptr->unk22 = 0;
		ptr->sssObj = 0;
		ptr->nextPtr = 0;
		ptr->bitmapBits = 0;
		ptr->nextPtr = _lvlObjectsList3;
		_lvlObjectsList3 = ptr;
		ptr->callbackFuncPtr = &Game::lvlObjectList3Callback;
		return 1;
	}
	return 0;
}

void Game::removeLvlObject(LvlObject *ptr) {
	AndyObjectScreenData *dataPtr = (AndyObjectScreenData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
	LvlObject *o = dataPtr->nextPtr;
	if (o) {
		dataPtr->nextPtr = 0;
		removeLvlObjectFromList(&_lvlObjectsList0, o);
		destroyLvlObject(o);
	}
}

void Game::removeLvlObject2(LvlObject *o) {
	if (o->type != 2) {
		LvlObject *ptr = _lvlObjectsList1;
		if (ptr) {
			if (ptr == o) {
				_lvlObjectsList1 = o->nextPtr;
			} else {
				LvlObject *prev = 0;
				do {
					prev = ptr;
					ptr = ptr->nextPtr;
				} while (ptr && ptr != o);
				assert(ptr);
				prev->nextPtr = ptr->nextPtr;
			}
		}
	}
	o->dataPtr = 0;
	if (o->type == 8) {
		_res->decLevelData0x2988RefCounter(o);
		o->nextPtr = _declaredLvlObjectsListHead;
		_declaredLvlObjectsListHead = o;
		--_declaredLvlObjectsListCount;
	} else {
		switch (o->data0x2988) {
		case 0:
		case 2:
			o->dataPtr = 0;
			break;
		case 3:
		case 7:
			if (o->dataPtr) {
				prependObjectScreenDataList(o);
			}
			break;
		}
	}
	if (o->sssObj) {
		removeSound(o);
		o->sssObj = 0;
	}
	o->bitmapBits = 0;
}

void Game::setupCurrentScreen() {
	LvlObject *ptr = _andyObject;
	_fallingAndyFlag = false;
	_andyActionKeysFlags = 0;
	_hideAndyObjectSprite = false;
	const uint8_t *dat = &_levelCheckpointData[_currentLevel][_levelCheckpoint * 12];
	_plasmaCannonFlags = 0;
	_actionDirectionKeyMaskIndex = 0;
	_andyCurrentLevelScreenNum = ptr->screenNum;
	if (dat[9] != ptr->data0x2988) {
		switch (dat[9]) {
		case 0:
			removeLvlObject(ptr);
			setLvlObjectType8Resource(ptr, 8, 0);
			ptr->anim = 48;
			break;
		case 2:
			destroyLvlObjectPlasmaExplosion(_andyObject);
			setLvlObjectType8Resource(ptr, 8, 2);
			_plasmaCannonDirection = 0;
			_plasmaCannonLastIndex1 = 0;
			_plasmaCannonExplodeFlag = 0;
			_plasmaCannonPointsMask = 0;
			_plasmaCannonObject = 0;
			ptr->anim = 232;
			break;
		}
		ptr->frame = 0;
	}
	ptr->linkObjPtr = 0;
	ptr->xPos = (int16_t)READ_LE_UINT16(dat + 0);
	ptr->yPos = (int16_t)READ_LE_UINT16(dat + 2);
	ptr->flags2 = READ_LE_UINT16(dat + 4);
	ptr->anim = READ_LE_UINT16(dat + 6);
	ptr->flags1 = ((ptr->flags2 >> 10) & 0x30) | (ptr->flags1 & ~0x30);
	ptr->screenNum = dat[8];
	ptr->directionKeyMask = 0;
	ptr->actionKeyMask = 0;
	_currentScreen = dat[8];
	_currentLeftScreen = _res->_screensGrid[_currentScreen * 4 + kPosLeftScreen];
	_currentRightScreen = _res->_screensGrid[_currentScreen * 4 + kPosRightScreen];
	ptr->frame = 0;
	setupLvlObjectBitmap(ptr);
	AndyObjectScreenData *dataPtr = (AndyObjectScreenData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
	dataPtr->unk6 = 0;
	if (ptr->data0x2988 == 2) {
		removeLvlObject(ptr);
	} else {
		destroyLvlObjectPlasmaExplosion(ptr);
	}
}

void Game::updateScreenHelper(int num) {
	_res->_screensState[num].s2 = 1;
	for (LvlObject *ptr = _screenLvlObjectsList[num]; ptr; ptr = ptr->nextPtr) {
		switch (ptr->type) {
		case 0: {
				AnimBackgroundData *p = (AnimBackgroundData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAnimBackgroundData);
				uint8_t *data = _res->_resLvlScreenBackgroundDataTable[num].backgroundAnimationTable[ptr->flags & 0xFF];
				assert(data);
				p->framesCount = READ_LE_UINT16(data); data += 2;
				ptr->soundToPlay = READ_LE_UINT16(data); data += 2;
				p->currentSpriteData = p->otherSpriteData = data;
				p->currentFrame = 0;
				p->firstSpriteData = READ_LE_UINT16(data + 4) + data + 4;
			}
			break;
		case 1: {
				uint8_t *data =  _res->_resLvlScreenBackgroundDataTable[num].backgroundSoundTable[ptr->flags & 0xFF];
				ptr->soundToPlay = READ_LE_UINT16(data); data += 2;
				ptr->dataPtr = data;
			}
			break;
		case 2:
			ptr->levelData0x2988 = _res->_resLvlScreenBackgroundDataTable[num].dataUnk5Table[ptr->flags & 0xFF];
			if (_currentLevel == kLvl_rock) {
				ptr->callbackFuncPtr = _callLevel_objectUpdate_rock[ptr->stateValue];
			} else {
				// other levels use two callbacks
				switch (ptr->stateValue) {
				case 0:
					ptr->callbackFuncPtr = &Game::objectUpdate_rock_case0;
					break;
				case 1:
					ptr->callbackFuncPtr = &Game::objectUpdate_rock_case3;
					break;
				default:
					warning("updateScreenHelper unimplemented for level %d, state %d", _currentLevel, ptr->stateValue);
					break;
				}
			}
			setupLvlObjectBitmap(ptr);
			break;
		}
	}
}

void Game::resetDisplay() {
//	_video_blitSrcPtr = _video_blitSrcPtr2;
	_video->_displayShadowLayer = false;
	_shakeScreenDuration = 0;
	_fadePaletteCounter = 0;
	_fadePalette = 0;
//	_snd_masterVolume = _plyConfigTable[_plyConfigNumber].soundVolume;
}

void Game::updateScreen(uint8_t num) {
	uint8_t i, prev;

	if (num == 0xFF) {
		return;
	}
	prev = _res->_currentScreenResourceNum;
	_res->_currentScreenResourceNum = num;
	updateScreenHelper(num);
	callLevel_preScreenUpdate(num);
	if (_res->_screensState[num].s0 >= _res->_screensState[num].s1) {
		--_res->_screensState[num].s1;
	}
	callLevel_postScreenUpdate(num);
	i = _res->_screensGrid[num * 4 + kPosTopScreen];
	if (i != 0xFF && prev != i) {
		callLevel_preScreenUpdate(i);
		setupScreenMask(i);
		callLevel_postScreenUpdate(i);
	}
	i = _res->_screensGrid[num * 4 + kPosRightScreen];
	if (i != 0xFF && _res->_resLevelData0x2B88SizeTable[i] != 0 && prev != i) {
		updateScreenHelper(i);
		callLevel_preScreenUpdate(i);
		setupScreenMask(i);
		callLevel_postScreenUpdate(i);
	}
	i = _res->_screensGrid[num * 4 + kPosBottomScreen];
	if (i != 0xFF && prev != i) {
		callLevel_preScreenUpdate(i);
		setupScreenMask(i);
		callLevel_postScreenUpdate(i);
	}
	i = _res->_screensGrid[num * 4 + kPosLeftScreen];
	if (i != 0xFF && _res->_resLevelData0x2B88SizeTable[i] != 0 && prev != i) {
		updateScreenHelper(i);
		callLevel_preScreenUpdate(i);
		setupScreenMask(i);
		callLevel_postScreenUpdate(i);
	}
	callLevel_postScreenUpdate(num);
	setupBackgroundBitmap();
	setupScreenMask(num);
	resetDisplay();
//	_time_counter1 = GetTickCount();
}

void Game::resetScreen() {
	for (int i = 0; i < _res->_lvlHdr.screensCount; ++i) {
		_res->_screensState[i].s0 = 0;
		_screenCounterTable[i] = 0;
	}
	const uint8_t *dat2 = _levelScreenStartData[_currentLevel];
	const int n = _levelCheckpointData[_currentLevel][_levelCheckpoint * 12 + 8];
	for (int i = 0; i < n; ++i) {
		_res->_screensState[i].s0 = *dat2++;
		_screenCounterTable[i] = *dat2++;
	}
	resetScreenMask();
	for (int i = n; i < _res->_lvlHdr.screensCount; ++i) {
		switch (_currentLevel) {
		case 0:
			callLevel_setupLvlObjects_rock(i);
			break;
		case 1:
			callLevel_setupLvlObjects_fort(i);
			break;
		case 2:
			// none for 'pwr1'
			break;
		case 3:
			// none for 'isld'
			break;
		case 4:
			callLevel_setupLvlObjects_lava(i);
			break;
		case 5:
			// none for 'pwr2'
			break;
		case 6:
			callLevel_setupLvlObjects_lar1(i);
			break;
		case 7:
			callLevel_setupLvlObjects_lar2(i);
			break;
		case 8:
			// none for 'dark'
			break;
		}
	}
//	memset(byte_472400, 0, 66 * 4);
//	byte_47309C = 0;
}

void Game::restartLevel() {
	setupCurrentScreen();
	clearLvlObjectsList2();
	clearLvlObjectsList3();
	if (!_mstLogicDisabled) {
		resetMstCode();
		startMstCode();
	} else {
		_mstFlags = 0;
	}
	if (_res->_sssHdr.dataUnk1Count) {
		resetSound();
	}
	const int num = _levelCheckpointData[_currentLevel][_levelCheckpoint * 12 + 8];
	preloadLevelScreenData(num, 0xFF);
	_andyObject->levelData0x2988 = _res->_resLevelData0x2988PtrTable[_andyObject->data0x2988];
	resetScreen();
	if (_andyObject->screenNum != num) {
		preloadLevelScreenData(_andyObject->screenNum, 0xFF);
	}
	updateScreen(_andyObject->screenNum);
}

void Game::playAndyFallingCutscene(int type) {
	bool play = false;
	if (type == 0) {
		play = true;
	} else if (_fallingAndyFlag) {
		++_fallingAndyCounter;
		if (_fallingAndyCounter >= 2) {
			play = true;
		}
	}
	if (!_paf->_skipCutscenes && play) {
		switch (_currentLevel) {
		case 0:
			if (_andyObject->data0x2988 == 0) {
				_paf->play(22); // Andy falls with cannon
			} else {
				_paf->play(23); // Andy falls without cannon
			}
			break;
		case 1:
			if (_res->_currentScreenResourceNum == 0) {
				_paf->play(23);
			}
			break;
		case 3:
			_paf->play(24);
			break;
		}
	}
	if (type != 0 && play) {
		restartLevel();
	}
}

int8_t Game::updateLvlObjectScreen(LvlObject *ptr) {
	int8_t ret = 0;

	if ((_plasmaCannonFlags & 1) == 0 && _plasmaCannonDirection == 0) {
		int xPosPrev = ptr->xPos;
		int xPos = ptr->xPos + ptr->posTable[3].x;
		int yPosPrev = ptr->yPos;
		int yPos = ptr->yPos + ptr->posTable[3].y;
		uint8_t num = ptr->screenNum;
		if (xPos < 0) {
			ptr->screenNum = _res->_screensGrid[num * 4 + kPosLeftScreen];
			ptr->xPos += 256;
		} else if (xPos > 256) {
			ptr->screenNum = _res->_screensGrid[num * 4 + kPosRightScreen];
			ptr->xPos -= 256;
		}
		if (yPos < 0 && ptr->screenNum != 0xFF) {
			ptr->screenNum = _res->_screensGrid[ptr->screenNum * 4 + kPosTopScreen];
			ptr->yPos += 192;
		} else if (yPos > 192) {
			assert(ptr->screenNum != 0xFF);
			ptr->screenNum = _res->_screensGrid[ptr->screenNum * 4 + kPosBottomScreen];
			ptr->yPos -= 192;
		}
		if (ptr->screenNum == 0xFF) {
			debug(kDebug_GAME, "Changing screen from -1 to %d, pos=%d,%d (%d,%d)", num, xPos, yPos, xPosPrev, yPosPrev);
			ptr->screenNum = num;
			ptr->xPos = xPosPrev;
			ptr->yPos = yPosPrev;
			ret = -1;
		} else if (ptr->screenNum != num) {
			debug(kDebug_GAME, "Changing screen from %d to %d, pos=%d,%d", num, ptr->screenNum, xPos, yPos);
			ret = 1;
			AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
			data->boundingBox.x1 = ptr->xPos;
			data->boundingBox.x2 = ptr->xPos + ptr->width - 1;
			data->boundingBox.y1 = ptr->yPos;
			data->boundingBox.y2 = ptr->yPos + ptr->height - 1;
		}
	}
	_currentScreen = ptr->screenNum;
	_currentLeftScreen = _res->_screensGrid[_currentScreen * 4 + kPosLeftScreen];
	_currentRightScreen = _res->_screensGrid[_currentScreen * 4 + kPosRightScreen];
	return ret;
}

void Game::setAndyLvlObjectPlasmaCannonKeyMask() {
	if (_plasmaCannonKeyMaskCounter == 0) {
		switch (_actionDirectionKeyMaskIndex >> 4) {
		case 0:
			_plasmaCannonKeyMaskCounter = 6;
			break;
		case 1:
		case 8:
		case 10:
			_plasmaCannonKeyMaskCounter = 2;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			_plasmaCannonKeyMaskCounter = 1;
			break;
		}
	}
	if (_actionDirectionKeyMaskIndex != 0) {
		if (_actionDirectionKeyMaskIndex == 164 && _fadePalette == 0) {
			_fadePaletteCounter = 10;
			_plasmaCannonFlags |= 1;
		} else {
			if (_andyObject->data0x2988 == 2 && _actionDirectionKeyMaskIndex >= 16) {
				removeLvlObject(_andyObject);
			}
			_andyActionKeysFlags = 0;
		}
		_andyObject->actionKeyMask = _actionDirectionKeyMaskTable[_actionDirectionKeyMaskIndex * 2];
		_andyObject->directionKeyMask = _actionDirectionKeyMaskTable[_actionDirectionKeyMaskIndex * 2 + 1];
	}
	--_plasmaCannonKeyMaskCounter;
	if (_plasmaCannonKeyMaskCounter == 0) {
		_actionDirectionKeyMaskIndex = 0;
	}
}

int Game::resetAndyLvlObjectPlasmaCannonKeyMask(uint8_t mask) {
	if (mask > _actionDirectionKeyMaskIndex) {
		_actionDirectionKeyMaskIndex = mask;
		_plasmaCannonKeyMaskCounter = 0;
		return 1;
	}
	return 0;
}

int Game::clipBoundingBox(BoundingBox *coords, BoundingBox *box) {
	if (coords->x1 > coords->x2) {
		SWAP(coords->x1, coords->x2);
	}
	if (coords->y1 > coords->y2) {
		SWAP(coords->y1, coords->y2);
	}
	if (box->x1 > box->x2) {
		SWAP(box->x1, box->x2);
	}
	if (box->y1 > box->y2) {
		SWAP(box->y1, box->y2);
	}
	if (coords->x1 > box->x2 || coords->x2 < box->x1 || coords->y1 > box->y2 || coords->y2 < box->y1) {
		return 0;
	}
	_clipBoxOffsetX = (box->x2 - coords->x1) / 2 + coords->x1;
	_clipBoxOffsetY = (box->y2 - coords->y1) / 2 + coords->y1;
	return 1;
}

int Game::updateBoundingBoxClippingOffset(BoundingBox *_ecx, BoundingBox *_ebp, const uint8_t *coords, int direction) {
	int ret = 0;
	int count = *coords++;
	if (count == 0) {
		return clipBoundingBox(_ecx, _ebp);
	}
	switch (direction) {
	case 1:
		for (; --count != 0; coords += 4) {
			if (_ecx->x1 > _ebp->x2 - coords[0] || _ecx->x2 < _ebp->x2 - coords[2]) {
				continue;
			}
			if (_ecx->y1 > _ebp->y1 + coords[3] || _ecx->y2 < _ebp->y1 + coords[1]) {
				continue;
			}
		}
		break;
	case 2:
		for (; --count != 0; coords += 4) {
			if (_ecx->x1 > coords[2] + _ebp->x1 || _ecx->x2 < coords[0] + _ebp->x1) {
				continue;
			}
			if (_ecx->y1 > _ebp->y2 - coords[1] || _ecx->y2 < _ebp->y2 - coords[3]) {
				continue;
			}
		}
		break;
	case 3:
		for (; --count != 0; coords += 4) {
			if (_ecx->x1 > _ebp->x2 - coords[0] || _ecx->x2 < _ebp->x2 - coords[2]) {
				continue;
			}
			if (_ecx->y1 > _ebp->y2 - coords[1] || _ecx->y2 < _ebp->y2 - coords[3]) {
				continue;
			}
		}
		break;
	default:
		for (; --count != 0; coords += 4) {
			if (_ecx->x1 > coords[2] + _ebp->x1 || _ecx->x2 < coords[0] + _ebp->x1) {
				continue;
			}
			if (_ecx->y1 > coords[3] + _ebp->y1 || _ecx->y2 < coords[1] + _ebp->y1) {
				continue;
			}
		}
		break;
	}
	if (count != 0) {
		_clipBoxOffsetX = (_ecx->x2 - _ecx->x1) / 2 + _ecx->x1;
		_clipBoxOffsetY = (_ecx->y2 - _ecx->y1) / 2 + _ecx->y1;
		ret = 1;
	}
	return ret;
}

int Game::game_unk16(LvlObject *o1, BoundingBox *box1, LvlObject *o2, BoundingBox *box2) {
	warning("game_unk16() unimplemented");
	return 0;
}

int Game::clipLvlObjectsBoundingBox(LvlObject *o, LvlObject *ptr, int type) {
	BoundingBox obj1, obj2;

	obj1.x1 = o->xPos + _res->_screensBasePos[o->screenNum].u;
	obj1.y1 = obj1.y2 = o->yPos + _res->_screensBasePos[o->screenNum].v;
	obj2.x1 = ptr->xPos + _res->_screensBasePos[ptr->screenNum].u;
	obj2.y1 = obj2.y2 = ptr->yPos + _res->_screensBasePos[ptr->screenNum].v;

	switch (type - 17) {
	case 1:
		obj1.x2 = obj1.x1 + o->posTable[1].x;
		obj1.x1 += o->posTable[0].x;
		obj1.y2 += o->posTable[1].y;
		obj1.y1 += o->posTable[0].y;
		obj2.x2 += ptr->width - 1;
		obj2.y2 += ptr->height - 1;
		return clipBoundingBox(&obj1, &obj2);
	case 17:
		obj1.x2 = obj1.x1 + o->posTable[1].x;
		obj1.x1 += o->posTable[0].x;
		obj1.y2 += o->posTable[1].y;
		obj1.y1 += o->posTable[0].y;
		obj2.x2 = obj2.x1 + ptr->posTable[1].x;
		obj2.x1 += ptr->posTable[0].x;
		obj2.y2 += ptr->posTable[1].y;
		obj2.y1 += ptr->posTable[0].y;
		return clipBoundingBox(&obj1, &obj2);
	case 49:
		obj1.x2 = obj1.x1 + o->posTable[1].x;
		obj1.x1 += o->posTable[0].x;
		obj1.y2 += o->posTable[1].y;
		obj1.y1 += o->posTable[0].y;
		obj2.x2 = obj2.x1 + ptr->width - 1;
		obj2.y2 += ptr->height - 1;
		if (clipBoundingBox(&obj1, &obj2)) {
			updateBoundingBoxClippingOffset(&obj1, &obj2, _res->getLvlSpriteCoordPtr(ptr->levelData0x2988, ptr->unk22), (ptr->flags1 >> 4) & 3);
		}
		break;
	case 16:
		obj1.x2 = obj1.x1 + o->width - 1;
		obj1.y2 += o->height - 1;
		obj2.y1 += ptr->posTable[0].y;
		obj2.x2 = obj2.x1 + ptr->posTable[1].x;
		obj2.x1 += ptr->posTable[0].x;
		obj2.y2 += ptr->posTable[1].y;
		return clipBoundingBox(&obj1, &obj2);
	case 0:
		obj1.x2 = obj1.x1 + o->width - 1;
		obj1.y2 += o->height - 1;
		obj2.x2 += ptr->width - 1;
		obj2.y1 = obj2.y2 + ptr->height - 1;
		return clipBoundingBox(&obj1, &obj2);
	case 48:
		obj1.x2 += o->width - 1;
		obj1.y2 += o->height - 1;
		obj2.x2 += ptr->width - 1;
		obj2.y2 += ptr->height - 1;
		if (clipBoundingBox(&obj1, &obj2)) {
			return updateBoundingBoxClippingOffset(&obj1, &obj2, _res->getLvlSpriteCoordPtr(ptr->levelData0x2988, ptr->unk22), (ptr->flags1 >> 4) & 3);
		}
		break;
	case 19:
		obj1.x2 = obj1.x1 + o->width - 1;
		obj1.y2 += o->height - 1;
		obj2.x2 = obj2.x1 + ptr->posTable[1].x;
		obj2.x1 += ptr->posTable[0].x;
		obj2.y1 += ptr->posTable[0].y;
		obj2.y2 += ptr->posTable[1].y;
		if (clipBoundingBox(&obj2, &obj1)) {
			return updateBoundingBoxClippingOffset(&obj2, &obj1, _res->getLvlSpriteCoordPtr(ptr->levelData0x2988, ptr->unk22), (ptr->flags1 >> 4) & 3);
		}
		break;
	case 3:
		obj1.x2 = obj1.x2 + o->width - 1;
		obj1.y2 += o->height - 1;
		obj2.x2 = obj2.x1 + ptr->width - 1;
		obj2.y2 += ptr->height - 1;
		if (clipBoundingBox(&obj2, &obj1)) {
			return updateBoundingBoxClippingOffset(&obj2, &obj1, _res->getLvlSpriteCoordPtr(ptr->levelData0x2988, ptr->unk22), (ptr->flags1 >> 4) & 3);
		}
		break;
	case 51:
		obj1.x2 = obj1.x2 + o->width - 1;
		obj1.y2 += o->height - 1;
		obj2.x2 = obj2.x1 + ptr->width - 1;
		obj2.y2 += ptr->height - 1;
		return game_unk16(o, &obj1, ptr, &obj2);
	case 115:
		if (o->width == 3) {
			obj1.y2 += 7;
			obj1.x2 = obj1.x1 + 7;
		} else {
			obj1.x2 = obj1.x1 + o->width - 1;
			obj1.y2 += o->height - 1;
		}
		obj2.x2 = obj2.x1 + ptr->width - 9;
		obj2.x1 += 4;
		obj2.y2 += ptr->height - 13;
		obj2.y1 += 6;
		if (clipBoundingBox(&obj2, &obj1)) {
			return updateBoundingBoxClippingOffset(&obj2, &obj1, _res->getLvlSpriteCoordPtr(ptr->levelData0x2988, ptr->unk22), (ptr->flags1 >> 4) & 3);
		}
	}
	return 0;
}

int Game::updateAndyLvlObject() {
	if (!_andyObject) {
		return 0;
	}
	if (_actionDirectionKeyMaskIndex != 0) {
		setAndyLvlObjectPlasmaCannonKeyMask();
	}
	assert(_andyObject->callbackFuncPtr);
	(this->*_andyObject->callbackFuncPtr)(_andyObject);
	if (_currentLevel != kLvl_isld) {
		AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		_andyObject->xPos += data->dxPos;
		_andyObject->yPos += data->dyPos;
	}
	if ((_andyObject->flags0 & 0x1F) == 0xB) {
		if (_andyObject->data0x2988 == 2) {
			removeLvlObject(_andyObject);
		}
		if ((_andyObject->flags0 & ~0x1F) == 0x40) {
			resetAndyLvlObjectPlasmaCannonKeyMask(0xA4);
		}
	}
	int8_t _al = updateLvlObjectScreen(_andyObject);
	if (_al > 0) {
		return 1;
	} else if (_al == 0) {
		if (_currentLevel != kLvl_rock && _currentLevel != kLvl_lar2 && _currentLevel != kLvl_test) {
			return 0;
		}
		if (_plasmaExplosionObject) {
			_plasmaExplosionObject->screenNum = _andyObject->screenNum;
			lvlObjectType1Callback(_plasmaExplosionObject);
			if (_andyObject->actionKeyMask & 4) {
				addToSpriteList(_plasmaExplosionObject);
			}
		} else if (_andyObject->data0x2988 == 0) {
			lvlObjectType1Init(_andyObject);
		}
		return 0;
	}
	if ((_andyObject->flags0 & 0x1F) != 0xB) {
		playAndyFallingCutscene(0);
	}
	setupCurrentScreen();
	clearLvlObjectsList2();
	clearLvlObjectsList3();
	if (!_mstLogicDisabled) {
		resetMstCode();
		startMstCode();
	} else {
		_mstFlags = 0;
	}
	if (_res->_sssHdr.dataUnk1Count) {
		resetSound();
	}
	const int num = _levelCheckpointData[_currentLevel][_levelCheckpoint * 12 + 8];
	preloadLevelScreenData(num, 0xFF);
	_andyObject->levelData0x2988 = _res->_resLevelData0x2988PtrTable[_andyObject->data0x2988];
	resetScreen();
	if (_andyObject->screenNum != num) {
		preloadLevelScreenData(_andyObject->screenNum, 0xFF);
	}
	updateScreen(_andyObject->screenNum);
	return 1;
}

void Game::drawPlasmaCannon() {
	int index = _plasmaCannonFirstIndex;
	int lastIndex = _plasmaCannonLastIndex1;
	if (lastIndex == 0) {
		lastIndex = _plasmaCannonLastIndex2;
	}
	int x1 = _gameXPosTable[index];
	int y1 = _gameYPosTable[index];
	index += 4;
	do {
		_video->_drawLine.color = 0xA9;
		int x2 = _plasmaCannonXPointsTable1[index];
		int y2 = _plasmaCannonYPointsTable1[index];
		if (_plasmaCannonDirection == 1) {
			_video->drawLine(x1 - 1, y1, x2 - 1, y2);
			_video->drawLine(x1 + 1, y1, x2 + 1, y2);
		} else {
			_video->drawLine(x1, y1 - 1, x2, y2 - 1);
			_video->drawLine(x1, y1 + 1, x2, y2 + 1);
		}
		_video->_drawLine.color = 0xA6;
		_video->drawLine(x1, y1, x2, y2);
		x1 = x2;
		y1 = y2;
		index += 4;
	} while (index <= lastIndex);
	_plasmaCannonLastIndex1 = 0;
	_plasmaCannonPointsMask = 0;
	_plasmaCannonExplodeFlag = 0;
	_plasmaCannonObject = 0;
}

void Game::redrawObjects() {
	memcpy(_video->_frontLayer, _video->_backgroundLayer, 256 * 192);

	// redraw background animation sprites
	LvlBackgroundData *dat = &_res->_resLvlScreenBackgroundDataTable[_res->_currentScreenResourceNum];
	for (Sprite *spr = _gameSpriteListPtrTable[0]; spr; spr = spr->nextPtr) {
		if ((spr->num & 0x1F) == 0) {
			_video->decodeSPR(spr->bitmapBits, _video->_backgroundLayer, spr->xPos, spr->yPos, 0);
		}
	}
	memset(_video->_shadowLayer, 0, 256 * 192);
	for (int i = 1; i < 8; ++i) {
		for (Sprite *spr = _gameSpriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x2000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_shadowLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
	for (int i = 1; i < 4; ++i) {
		for (Sprite *spr = _gameSpriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x1000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_frontLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
	if (_andyObject->data0x2988 == 0 && (_andyObject->flags2 & 0x1F) == 4) {
		if (_plasmaCannonFirstIndex < _plasmaCannonLastIndex2) {
			drawPlasmaCannon();
		}
	}
	for (int i = 4; i < 8; ++i) {
		for (Sprite *spr = _gameSpriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x1000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_frontLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
	for (int i = 0; i < 24; ++i) {
		for (Sprite *spr = _gameSpriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x2000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_shadowLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
	for (int i = 0; i < dat->dataUnk1Count; ++i) {
		_video->applyShadowColors(_shadowScreenMasksTable[i].x,
			_shadowScreenMasksTable[i].y,
			_shadowScreenMasksTable[i].w,
			_shadowScreenMasksTable[i].h,
			256,
			_shadowScreenMasksTable[i].w,
			_video->_shadowLayer,
			_video->_frontLayer,
			_shadowScreenMasksTable[i].projectionDataPtr,
			_shadowScreenMasksTable[i].shadowPalettePtr);
	}
	for (int i = 1; i < 12; ++i) {
		for (Sprite *spr = _gameSpriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x1000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_frontLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
	if (_andyObject->data0x2988 == 0 && (_andyObject->flags2 & 0x1F) == 0xC) {
		if (_plasmaCannonFirstIndex < _plasmaCannonLastIndex2) {
			drawPlasmaCannon();
		}
	}
	for (int i = 12; i <= 24; ++i) {
		for (Sprite *spr = _gameSpriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x1000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_frontLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
}

static const char *_levels[] = {
	"rock_hod",
	"fort_hod",
	"pwr1_hod",
	"isld_hod",
	"lava_hod",
	"pwr2_hod",
	"lar1_hod",
	"lar2_hod",
	"dark_hod",
	"test_hod"
};

void Game::mainLoop(int level, int checkpoint) {
	if (level >= kLvl_test) {
		return;
	}
	_currentLevel = level;
	_levelCheckpoint = checkpoint;
	benchmarkCpu();
	_res->loadSetupDat();
	_res->loadLevelData(_levels[_currentLevel]);
	levelMainLoop();
}

void Game::mixAudio(int16_t *buf, int len) {
	_mix.mix(buf, len);
}

void Game::updateLvlObjectList(LvlObject *list) {
	for (LvlObject *ptr = list; ptr; ptr = ptr->nextPtr) {
		if (ptr->callbackFuncPtr) {
			(this->*(ptr->callbackFuncPtr))(ptr);
		}
		if (ptr->bitmapBits) {
			addToSpriteList(ptr);
		}
	}
}

void Game::updateLvlObjectLists() {
	updateLvlObjectList(_lvlObjectsList0);
	updateLvlObjectList(_lvlObjectsList1);
	updateLvlObjectList(_lvlObjectsList2);
	updateLvlObjectList(_lvlObjectsList3);
}

LvlObject *Game::updateAnimatedLvlObjectType0(LvlObject *ptr) {
	AnimBackgroundData *_esi = (AnimBackgroundData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAnimBackgroundData);
	uint8_t *_edi = _esi->currentSpriteData + 2;
	if (_res->_currentScreenResourceNum == ptr->screenNum) {
		if (ptr->soundToPlay != 0xFFFF) {
			playSound(ptr->soundToPlay, ptr, 0, 3);
			ptr->soundToPlay = 0xFFFF;
		}
		uint8_t *_edx = _edi + 2;
		Sprite *spr = _gameSpriteListHead;
		if (spr && READ_LE_UINT16(_edx) > 8) {
			spr->xPos = (int8_t)_edi[0];
			spr->yPos = (int8_t)_edi[1];
			spr->bitmapBits = _edx;
			spr->num = ptr->flags2;
			const int index = spr->num & 0x1F;
			_gameSpriteListHead = spr->nextPtr;
			spr->nextPtr = _gameSpriteListPtrTable[index];
			_gameSpriteListPtrTable[index] = spr;
		}
	}
	int16_t soundNum = -1;
	uint8_t *_eax = READ_LE_UINT16(_edi + 2) + _edi + 2; // nextSpriteData
	switch (ptr->stateValue - 1) {
	case 6:
		_esi->currentSpriteData = _esi->firstSpriteData;
		if (_esi->currentFrame == 0) {
			_esi->currentFrame = 1;
			soundNum = READ_LE_UINT16(_esi->firstSpriteData);
		}
		ptr->stateValue = 4;
		break;
	case 5:
		_esi->currentFrame = 0;
		_esi->currentSpriteData = _esi->otherSpriteData;
		ptr->stateValue = 1;
		break;
	case 3:
		++_esi->currentFrame;
		if (_esi->currentFrame < _esi->framesCount) {
			_esi->currentSpriteData = _eax;
			soundNum = READ_LE_UINT16(_eax);
		} else {
			_esi->currentFrame = 0;
			_esi->currentSpriteData = _esi->otherSpriteData;
			ptr->stateValue = 1;
			soundNum = READ_LE_UINT16(_esi->currentSpriteData);
		}
		break;
	case 4:
		++_esi->currentFrame;
		if (_esi->currentFrame <= _esi->framesCount) {
			_esi->currentSpriteData = _eax;
			soundNum = READ_LE_UINT16(_eax);
		} else {
			_esi->currentFrame = 0;
			_esi->currentSpriteData = _esi->otherSpriteData;
			ptr->stateValue = 1;
			soundNum = READ_LE_UINT16(_esi->currentSpriteData);
		}
		break;
	case 2:
		warning("updateAnimatedLvlObjectType0 - TODO case 2");
		break;
	case 1:
		++_esi->currentFrame;
		if (_esi->currentFrame < _esi->framesCount - 1) {
			_esi->currentSpriteData = _eax;
			soundNum = READ_LE_UINT16(_esi->currentSpriteData);
		} else {
			if (_esi->currentFrame > _esi->framesCount) {
				_esi->currentFrame = _esi->framesCount;
			}
			ptr->stateValue = 1;
			return ptr->nextPtr;
		}
		break;
	case 0:
		return ptr->nextPtr;
	default:
		soundNum = READ_LE_UINT16(_esi->currentSpriteData);
		if (ptr->stateCounter == 0) {
			++_esi->currentFrame;
			if (_esi->currentFrame >= _esi->framesCount) {
				_esi->currentSpriteData = _esi->firstSpriteData;
				_esi->currentFrame = 1;
			} else {
				_esi->currentSpriteData = _eax;
			}
		} else {
			--ptr->stateCounter;
		}
		break;
	}
	if (soundNum != -1) {
		playSound(soundNum, ptr, 0, 3);
	}
	return ptr->nextPtr;
}

LvlObject *Game::updateAnimatedLvlObjectType1(LvlObject *ptr) {
	if (ptr->screenNum == _res->_currentScreenResourceNum) {
		if (_res->_screensState[_res->_currentScreenResourceNum].s0 == ptr->screenState || ptr->screenState == 0xFF) {
			if (ptr->soundToPlay != 0xFFFF) {
				playSound(ptr->soundToPlay, 0, 0, 3);
				ptr->soundToPlay = 0xFFFF;
			}
			uint8_t *data = (uint8_t *)getLvlObjectDataPtr(ptr, kObjectDataTypeLvlBackgroundSound);
			Sprite *spr = _gameSpriteListHead;
			if (spr && READ_LE_UINT16(data + 2) > 8) {
				spr->bitmapBits = data + 2;
				spr->xPos = data[0];
				spr->yPos = data[1];
				_gameSpriteListHead = spr->nextPtr;
				spr->num = ptr->flags2;
				const int index = spr->num & 0x1F;
				spr->nextPtr = _gameSpriteListPtrTable[index];
				_gameSpriteListPtrTable[index] = spr;
			}
		}
	}
	return ptr->nextPtr;
}

LvlObject *Game::updateAnimatedLvlObjectType2(LvlObject *ptr) {
	LvlObject *next, *o;

	uint8_t _al = ptr->data0x2988;
	o = next = ptr->nextPtr;
	if ((_al > 15 && ptr->dataPtr == 0) || ptr->levelData0x2988 == 0) {
		if (ptr->linkObjPtr) {
			o = ptr->nextPtr;
		}
		return o;
	}
	_al = ptr->screenNum;
	if (_currentScreen != _al && _currentRightScreen != _al && _currentLeftScreen != _al) {
		return o;
	}
	if (!ptr->callbackFuncPtr) {
		warning("updateAnimatedLvlObjectType2: no callback ptr");
	} else {
		if ((this->*(ptr->callbackFuncPtr))(ptr) == 0) {
			return o;
		}
	}
	if ((ptr->flags1 & 6) == 2) {
		const int index = (15 < ptr->data0x2988) ? 5 : 7;
		ptr->yPos += calcScreenMaskDy(ptr->xPos + ptr->posTable[index].x, ptr->yPos + ptr->posTable[index].y, ptr->screenNum);
	}
	if (ptr->bitmapBits == 0) {
		return o;
	}
	if (_currentScreen == ptr->screenNum) {
		const uint8_t *_edi = ptr->bitmapBits;

		LvlObjectData *dat = ptr->levelData0x2988;
		LvlAnimHeader *ah = (LvlAnimHeader *)(dat->animsInfoData + kLvlAnimHdrOffset) + ptr->anim;
		LvlAnimSeqHeader *ash = (LvlAnimSeqHeader *)(dat->animsInfoData + ah->seqOffset) + ptr->frame;

		int _edx = (ptr->flags1 >> 4) & 0xFF;
		int _ecx = (ash->flags1 >> 4) & 0xFF;
		_ecx = (((_ecx ^ _edx) & 3) << 14) | ptr->flags2;
		Sprite *spr = _gameSpriteListHead;
		if (spr && READ_LE_UINT16(_edi) > 8) {
			spr->yPos = ptr->yPos;
			spr->xPos = ptr->xPos;
			spr->bitmapBits = _edi;
			int index = spr->num = _ecx;
			index &= 0x1F;
			_gameSpriteListHead = spr->nextPtr;
			spr->nextPtr = _gameSpriteListPtrTable[index];
			_gameSpriteListPtrTable[index] = spr;
		}
	}
	if (ptr->data0x2988 <= 15 || ptr->dataPtr == 0) {
		if (ptr->soundToPlay != 0xFFFF) {
			playSound(ptr->soundToPlay, ptr, 0, 3);
		}
		return o;
	}
	int a, c;
	if (ptr->dataPtr >= &_mstUnkDataTable[0] && ptr->dataPtr <= &_mstUnkDataTable[32]) {
		MstUnkData *m = (MstUnkData *)ptr->dataPtr;
		if (m->flagsA6 & 2) {
			ptr->actionKeyMask = _mstCurrentActionKeyMask;
			ptr->directionKeyMask = _andyObject->directionKeyMask;
		}
		a = m->soundType;
		c = 1;
	} else {
		MstUnkData *m = ((MstObject *)ptr->dataPtr)->unk8;
		if (m) {
			a = m->soundType;
			c = 2;
		} else {
			a = 4;
			c = 0;
		}
	}
	if (ptr->soundToPlay != 0xFFFF) {
		playSound(ptr->soundToPlay, ptr, c, a);
	}
	return o;
}

LvlObject *Game::updateAnimatedLvlObjectTypeDefault(LvlObject *ptr) {
	return ptr->nextPtr;
}

LvlObject *Game::updateAnimatedLvlObject(LvlObject *o) {
	switch (o->type) {
	case 0:
		o = updateAnimatedLvlObjectType0(o);
		break;
	case 1:
		o = updateAnimatedLvlObjectType1(o);
		break;
	case 2:
		o = updateAnimatedLvlObjectType2(o);
		break;
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		o = updateAnimatedLvlObjectTypeDefault(o);
		break;
	default:
		error("updateAnimatedLvlObject unhandled type %d", o->type);
		break;
	}
	return o;
}

void Game::updateAnimatedLvlObjectsLeftRightCurrentScreens() {
	LvlObject *ptr = _screenLvlObjectsList[_res->_currentScreenResourceNum];
	while (ptr) {
		if (ptr->screenState == 0xFF || ptr->screenState == _res->_screensState[_res->_currentScreenResourceNum].s0) {
			ptr = updateAnimatedLvlObject(ptr);
		} else {
			ptr = ptr->nextPtr;
		}
	}
	int index = _res->_screensGrid[_res->_currentScreenResourceNum * 4 + kPosRightScreen];
	if (index != 0xFF && _res->_screensState[index].s2 != 0) {
		ptr = _screenLvlObjectsList[index];
		while (ptr) {
			if (ptr->screenState == 0xFF || ptr->screenState == _res->_screensState[index].s0) {
				ptr = updateAnimatedLvlObject(ptr);
			} else {
				ptr = ptr->nextPtr;
			}
		}
	}
	index = _res->_screensGrid[_res->_currentScreenResourceNum * 4 + kPosLeftScreen];
	if (index != 0xFF && _res->_screensState[index].s2 != 0) {
		ptr = _screenLvlObjectsList[index];
		while (ptr) {
			if (ptr->screenState == 0xFF || ptr->screenState == _res->_screensState[index].s0) {
				ptr = updateAnimatedLvlObject(ptr);
			} else {
				ptr = ptr->nextPtr;
			}
		}
	}
}

void Game::updatePlasmaCannonExplosionLvlObject(LvlObject *ptr) {
	ptr->actionKeyMask = 0;
	ptr->directionKeyMask = 0;
	if (_plasmaCannonDirection != 0 && _plasmaCannonLastIndex1 != 0) {
		if (_plasmaCannonObject) {
			const int _al = (_plasmaCannonObject->xPos <= _andyObject->xPos) ? 0 : 0xFF;
			ptr->directionKeyMask = (_al & ~5) | 8;
			if (_plasmaCannonObject->yPos > _andyObject->yPos) {
				ptr->directionKeyMask |= 4;
			} else {
				ptr->directionKeyMask |= 1;
			}
		} else {
			ptr->directionKeyMask = 2;
			if (_plasmaCannonPointsMask != 0) {
				if ((_plasmaCannonPointsMask & 1) != 0 || (_plasmaCannonDirection & 4) != 0) {
					ptr->directionKeyMask = 4;
				} else if ((_plasmaCannonDirection & 8) != 0) {
					ptr->directionKeyMask = 8;
				}
			}
		}
		if ((_andyObject->flags0 & 0x1F) == 4 && (_andyObject->flags0 & 0xE0) == 0xC0) {
			ptr->directionKeyMask = 1;
		}
		if (_plasmaCannonExplodeFlag == 1) {
			ptr->actionKeyMask = 4;
			if ((_rnd._rndSeed & 1) != 0 && addLvlObjectToList3(1)) {
				_lvlObjectsList3->flags0 = _andyObject->flags0;
				_lvlObjectsList3->flags1 = _andyObject->flags1;
				_lvlObjectsList3->screenNum = _andyObject->screenNum;
				_lvlObjectsList3->flags2 = _andyObject->flags2 & ~0x2000;
				if ((ptr->directionKeyMask & 1) == 0) {
					_lvlObjectsList3->anim = 12;
					_lvlObjectsList3->flags1 ^= 0x20;
				} else {
					_lvlObjectsList3->anim = 11;
				}
				_lvlObjectsList3->frame = 0;
				setLvlObjectPosRelativeToPoint(_lvlObjectsList3, 0, _plasmaCannonXPointsTable1[_plasmaCannonLastIndex1], _plasmaCannonYPointsTable1[_plasmaCannonLastIndex1]);
			}
		}
		setLvlObjectPosRelativeToPoint(ptr, 0, _plasmaCannonXPointsTable1[_plasmaCannonLastIndex1], _plasmaCannonYPointsTable1[_plasmaCannonLastIndex1]);
	}
	updateAndyObject(ptr);
	ptr->screenNum = _andyObject->screenNum;
	ptr->flags2 = bitmask_set(ptr->flags2, _andyObject->flags2, 0x18);
	ptr->flags2 = bitmask_set(ptr->flags2, _andyObject->flags2 + 1, 7);
	addToSpriteList(ptr);
}

void Game::resetPlasmaCannonState() {
	_plasmaCannonDirection = 0;
	_plasmaCannonPrevDirection = 0;
	_plasmaCannonPointsSetupCounter = 0;
	_plasmaCannonLastIndex1 = 0;
	_plasmaCannonExplodeFlag = 0;
	_plasmaCannonPointsMask = 0;
	_plasmaCannonFirstIndex = 16;
	_plasmaCannonLastIndex2 = 16;
}

void Game::updateAndyMonsterObjects() {
	uint8_t _dl = 3;
	LvlObject *ptr = _andyObject;
	switch (_actionDirectionKeyMaskIndex >> 4) {
	case 6:
		_hideAndyObjectSprite = false;
		if (_actionDirectionKeyMaskIndex == 0x61) {
			if (!_currentMonsterObject) {
				warning("_currentMonsterObject is NULL, _actionDirectionKeyMaskIndex 0x%x", _actionDirectionKeyMaskIndex);
				return;
			}
			//assert(_currentMonsterObject);
			_mstOriginPosX += _currentMonsterObject->posTable[6].x + _currentMonsterObject->xPos;
			_mstOriginPosY += _currentMonsterObject->posTable[6].y + _currentMonsterObject->yPos;
		}
		ptr->linkObjPtr = 0;
		break;
	case 7:
		_hideAndyObjectSprite = true;
		if (_actionDirectionKeyMaskIndex == 0x71) {
			if (!_currentMonsterObject) {
				warning("_currentMonsterObject is NULL, _actionDirectionKeyMaskIndex 0x%x", _actionDirectionKeyMaskIndex);
				return;
			}
			//assert(_currentMonsterObject);
			_mstOriginPosX += _currentMonsterObject->posTable[6].x + _currentMonsterObject->xPos;
			_mstOriginPosY += _currentMonsterObject->posTable[6].y + _currentMonsterObject->yPos;
			ptr->linkObjPtr = _currentMonsterObject;
			ptr->screenNum = _currentMonsterObject->screenNum;
		} else {
			ptr->linkObjPtr = 0;
		}
		break;
	case 10:
		if (_actionDirectionKeyMaskIndex != 0xA3) {
			return;
		}
		ptr->actionKeyMask = _actionDirectionKeyMaskTable[0x146];
		ptr->directionKeyMask = _actionDirectionKeyMaskTable[_actionDirectionKeyMaskIndex * 2 + 1];
		updateAndyObject(ptr);
		_actionDirectionKeyMaskIndex = 0;
		_hideAndyObjectSprite = false;
		_mstFlags |= 0x80000000;
		_dl = 1;
		break;
	default:
		return;
	}
	if (_dl & 2) {
		_actionDirectionKeyMaskIndex = 0;
		ptr->anim = _mstCurrentAnim;
		ptr->frame = 0;
		ptr->flags1 = bitmask_set(ptr->flags1, _mstCurrentFlags1, 0x30);
		setupLvlObjectBitmap(ptr);
		setLvlObjectPosRelativeToPoint(ptr, 3, _mstOriginPosX, _mstOriginPosY);
	}
	_andyActionKeysFlags = 0;
	if (ptr->data0x2988 == 2) {
		removeLvlObject(ptr);
	}
}

void Game::updateInput() {
	const uint8_t inputMask = _system->inp.mask;
	if (inputMask & SYS_INP_RUN) {
		_actionKeyMask |= 1;
	}
	if (inputMask & SYS_INP_JUMP) {
		_actionKeyMask |= 2;
	}
	if (inputMask & SYS_INP_SHOOT) {
		_actionKeyMask |= 4;
	}
	if (inputMask & SYS_INP_UP) {
		_directionKeyMask |= 1;
	} else if (inputMask & SYS_INP_DOWN) {
		_directionKeyMask |= 4;
	}
	if (inputMask & SYS_INP_RIGHT) {
		_directionKeyMask |= 2;
	} else if (inputMask & SYS_INP_LEFT) {
		_directionKeyMask |= 8;
	}
}

// cutscene number when starting a level
static const uint8_t _cutscenes[] = { 0, 2, 4, 5, 6, 8, 10, 14, 19 };

void Game::levelMainLoop() {
	_andyCurrentLevelScreenNum = -1;
	initMstCode();
//	res_initIO();
	preloadLevelScreenData(_levelCheckpointData[_currentLevel][_levelCheckpoint * 12 + 8], 0xFF);
	memset(_screenCounterTable, 0, 40);
	clearDeclaredLvlObjectsList();
	initLvlObjects();
	resetPlasmaCannonState();
	for (int i = 0; i < _res->_lvlHdr.screensCount; ++i) {
		_res->_screensState[i].s2 = 0;
	}
	_res->_currentScreenResourceNum = _andyObject->screenNum;
	_currentRightScreen = _res->_screensGrid[_res->_currentScreenResourceNum * 4 + kPosRightScreen]; /* right */
	_currentLeftScreen = _res->_screensGrid[_res->_currentScreenResourceNum * 4 + kPosLeftScreen]; /* left */
	if (!_mstLogicDisabled) {
		startMstCode();
	}
//	snd_setupResampleFunc(_ecx = 1);
	if (!_paf->_skipCutscenes && _levelCheckpoint == 0) {
		const uint8_t num = _cutscenes[_currentLevel];
		_paf->preload(num);
		_paf->play(num);
		_paf->unload(num);
	}
	if (_res->_sssHdr.dataUnk1Count != 0) {
		resetSound();
	}
	_quit = false;
	clearObjectScreenDataList();
	callLevel_initialize();
	setupCurrentScreen();
	clearLvlObjectsList2();
	clearLvlObjectsList3();
	if (!_mstLogicDisabled) {
		resetMstCode();
		startMstCode();
	} else {
		_mstFlags = 0;
	}
	if (_res->_sssHdr.dataUnk1Count != 0) {
		resetSound();
	}
	const int num = _levelCheckpointData[_currentLevel][_levelCheckpoint * 12 + 8];
	preloadLevelScreenData(num, 0xFF);
	_andyObject->levelData0x2988 = _res->_resLevelData0x2988PtrTable[_andyObject->data0x2988];
	resetScreen();
	if (_andyObject->screenNum != num) {
		preloadLevelScreenData(_andyObject->screenNum, 0xFF);
	}
	updateScreen(_andyObject->screenNum);
	do {
		int frameTimeStamp = _system->getTimeStamp() + kFrameTimeStamp;
		memset(_gameSpriteListPtrTable, 0, sizeof(_gameSpriteListPtrTable));
		_gameSpriteListHead = &_gameSpriteListTable[0];
		for (int i = 0; i < 127; ++i) {
			_gameSpriteListTable[i].nextPtr = &_gameSpriteListTable[i + 1];
		}
		_gameSpriteListTable[127].nextPtr = 0;
		_directionKeyMask = 0;
		_actionKeyMask = 0;
		updateInput();
		_andyObject->directionKeyMask = _directionKeyMask;
		_andyObject->actionKeyMask = _actionKeyMask;
		_video->fillBackBuffer();
		if (_andyObject->screenNum != _res->_currentScreenResourceNum) {
			preloadLevelScreenData(_andyObject->screenNum, _res->_currentScreenResourceNum);
			updateScreen(_andyObject->screenNum);
		} else if (_fadePalette != 0 && _fadePaletteCounter == 0) {
			setupCurrentScreen();
			clearLvlObjectsList2();
			clearLvlObjectsList3();
			if (!_mstLogicDisabled) {
				resetMstCode();
				startMstCode();
			} else {
				_mstFlags = 0;
			}
			if (_res->_sssHdr.dataUnk1Count != 0) {
				resetSound();
			}
			const int num = _levelCheckpointData[_currentLevel][_levelCheckpoint * 12 + 8];
			preloadLevelScreenData(num, 0xFF);
			_andyObject->levelData0x2988 = _res->_resLevelData0x2988PtrTable[_andyObject->data0x2988];
			resetScreen();
			if (_andyObject->screenNum != num) {
				preloadLevelScreenData(_andyObject->screenNum, 0xFF);
			}
			updateScreen(_andyObject->screenNum);
		} else {
			callLevel_postScreenUpdate(_res->_currentScreenResourceNum);
			if (_currentLeftScreen != 0xFF) {
				callLevel_postScreenUpdate(_currentLeftScreen);
			}
			if (_currentRightScreen != 0xFF) {
				callLevel_postScreenUpdate(_currentRightScreen);
			}
		}
		_currentLevelCheckpoint = _levelCheckpoint;
		if (updateAndyLvlObject() != 0) {
			callLevel_tick();
//			_time_counter1 -= _time_counter2;
			continue;
		}
		executeMstCode();
		updateLvlObjectLists();
		callLevel_tick();
		updateAndyMonsterObjects();
		if (!_hideAndyObjectSprite) {
			addToSpriteList(_andyObject);
		}
		((AndyObjectScreenData *)_andyObject->dataPtr)->dxPos = 0;
		((AndyObjectScreenData *)_andyObject->dataPtr)->dyPos = 0;
		updateAnimatedLvlObjectsLeftRightCurrentScreens();
		if (_currentLevel == 0 || _currentLevel == 7 || _currentLevel == 9) {
			if (_andyObject->data0x2988 == 0 && _plasmaExplosionObject && _plasmaExplosionObject->nextPtr != 0) {
				updatePlasmaCannonExplosionLvlObject(_plasmaExplosionObject->nextPtr);
			}
		}
		if (_res->_sssHdr.dataUnk1Count != 0) {
#if 1
			/* original code had a dedicated thread for sound, that main thread/loop was signaling */
			mixSoundObjects17640(true);
#else
			if (_snd_numberOfBuffers != 0) {
				SetEvent(_snd_threadEvent1);
				while (_snd_numberOfBuffers != 0) {
					Sleep(1);
				}
			}
			EnterCriticalSection(_snd_mutex);
			snd_prepareDirectSoundBuffers();
			if (_snd_numberOfBuffersMixed == 0) {
				_snd_numberOfBuffers = 4;
				_snd_syncTimeOut = 55;
				LeaveCriticalSection(_snd_mutex);
				SetEvent(_snd_threadEvent1);
				game_unmuteSound();
			} else if (_snd_numberOfBuffersMixed - _snd_playbackDuration < 4) {
				_snd_numberOfBuffers = 1;
				LeaveCriticalSection(_snd_mutex);
				SetEvent(_snd_threadEvent1);
				game_unmuteSound();
			} else {
				LeaveCriticalSection(_snd_mutex);
				game_unmuteSound();
			}
#endif
		}
		if (_video->_paletteNeedRefresh) {
			_video->_paletteNeedRefresh = false;
			_video->refreshGamePalette(_video->_displayPaletteBuffer);
		}
		redrawObjects();
		if (_system->inp.screenshot) {
			_system->inp.screenshot = false;
			captureScreenshot();
		}
		if (_shakeScreenDuration != 0 || _fadePaletteCounter != 0 || _video->_displayShadowLayer) {
			shakeScreen();
			uint8_t *p = _video->_shadowLayer;
			if (!_video->_displayShadowLayer) {
				p = _video->_frontLayer;
			}
			_video->updateGameDisplay(p);
		} else {
			_video->updateGameDisplay(_video->_frontLayer);
		}
		_rnd.update();
		_system->processEvents();
		if (_system->inp.keyPressed(SYS_INP_ESC)) { // display exit confirmation screen
//			while (_res_ioStateIndex == 1) {
//				if (!(_sync_unkCounterVar2 < _sync_unkVar1))
//					break;
//				res_preload_(0, 1000);
//			}
			if (displayHintScreen(-1, 0)) {
				break;
			}
		} else {
			// displayHintScreen(1, 0);
			_video->updateScreen();
		}
		int diff = frameTimeStamp - _system->getTimeStamp();
		if (diff < 10) {
			diff = 10;
		}
		_system->sleep(diff);
	} while (!_system->inp.quit && !_quit);
	callLevel_terminate();
}

void Game::callLevel_postScreenUpdate(int num) {
	switch (_currentLevel) {
	case 0:
		callLevel_postScreenUpdate_rock(num);
		break;
	case 1:
		callLevel_postScreenUpdate_fort(num);
		break;
	case 2:
		callLevel_postScreenUpdate_pwr1(num);
		break;
	case 3:
		callLevel_postScreenUpdate_isld(num);
		break;
	case 4:
		callLevel_postScreenUpdate_lava(num);
		break;
	case 5:
		callLevel_postScreenUpdate_pwr2(num);
		break;
	case 6:
		callLevel_postScreenUpdate_lar1(num);
		break;
	case 7:
		callLevel_postScreenUpdate_lar2(num);
		break;
	case 8:
		callLevel_postScreenUpdate_dark(num);
		break;
	default:
		warning("callLevel_postScreenUpdate unimplemented for screen %d", num);
		break;
	}
}

void Game::callLevel_preScreenUpdate(int num) {
	switch (_currentLevel) {
	case 0:
		callLevel_preScreenUpdate_rock(num);
		break;
	case 1:
		callLevel_preScreenUpdate_fort(num);
		break;
	case 2:
		callLevel_preScreenUpdate_pwr1(num);
		break;
	case 3:
		callLevel_preScreenUpdate_isld(num);
		break;
	case 4:
		callLevel_preScreenUpdate_lava(num);
		break;
	case 5:
		callLevel_preScreenUpdate_pwr2(num);
		break;
	case 6:
		callLevel_preScreenUpdate_lar1(num);
		break;
	case 7:
		callLevel_preScreenUpdate_lar2(num);
		break;
	case 8:
		callLevel_preScreenUpdate_dark(num);
		break;
	default:
		warning("callLevel_preScreenUpdate unimplemented for screen %d", _currentLevel);
		break;
	}
}

void Game::callLevel_initialize() {
	switch (_currentLevel) {
	case 0:
		callLevel_initialize_rock();
		break;
	case 2:
		callLevel_initialize_pwr1();
		break;
	case 3:
		callLevel_initialize_isld();
		break;
	case 4:
		callLevel_initialize_lava();
		break;
	case 5:
		callLevel_initialize_pwr2();
		break;
	case 6:
		callLevel_initialize_lar1();
		break;
	}
}

void Game::callLevel_tick() {
	switch (_currentLevel) {
	case 0:
		callLevel_tick_rock();
		break;
	case 1:
		callLevel_tick_fort();
		break;
	case 2:
		callLevel_tick_pwr1();
		break;
	case 3:
		callLevel_tick_isld();
		break;
	case 4:
		callLevel_tick_lava();
		break;
	case 5:
		callLevel_tick_pwr2();
		break;
	case 6:
		callLevel_tick_lar1();
		break;
	case 7:
		callLevel_tick_lar2();
		break;
	}
}

void Game::callLevel_terminate() {
	switch (_currentLevel) {
	case 0:
		callLevel_terminate_rock();
		break;
	case 3:
		callLevel_terminate_isld();
		break;
	case 2:
	case 4:
	case 5:
	case 6:
		if (_video->_displayShadowLayer) {
			free(_transformShadowBuffer);
			_transformShadowBuffer = 0;
		}
		break;
	}
}

int Game::displayHintScreen(int num, int pause) {
	static const int kQuitYes = 0;
	static const int kQuitNo = 1;
	int quit = kQuitYes;
	bool confirmQuit = false;
	uint8_t *quitBuffers[] = {
		_video->_frontLayer,
		_video->_shadowLayer,
	};
	if (num == -1) {
		num = _res->_datHdr.yesNoQuitImage; // 'Yes'
		_res->loadHintImage(num + 1, _video->_shadowLayer, _video->_palette); // 'No'
		confirmQuit = true;
	}
	_res->loadHintImage(num, _video->_frontLayer, _video->_palette);
	_system->setPalette(_video->_palette, 256, 6);
	_system->copyRect(0, 0, Video::kScreenWidth, Video::kScreenHeight, _video->_frontLayer, 256);
	_system->updateScreen();
	do {
		_system->processEvents();
		if (confirmQuit) {
			const int currentQuit = quit;
			if (_system->inp.keyReleased(SYS_INP_LEFT)) {
				quit = kQuitNo;
			}
			if (_system->inp.keyReleased(SYS_INP_RIGHT)) {
				quit = kQuitYes;
			}
			if (currentQuit != quit) {
				_system->copyRect(0, 0, Video::kScreenWidth, Video::kScreenHeight, quitBuffers[quit], 256);
				_system->updateScreen();
			}
		}
		_system->sleep(30);
	} while (!_system->inp.quit && !_system->inp.keyReleased(SYS_INP_JUMP));
	_video->_paletteNeedRefresh = true;
	return confirmQuit && quit == kQuitYes;
}

void Game::prependLvlObjectToList(LvlObject **list, LvlObject *ptr) {
	ptr->nextPtr = *list;
	*list = ptr;
}

void Game::removeLvlObjectFromList(LvlObject **list, LvlObject *ptr) {
	LvlObject *current = *list;
	if (current && ptr) {
		if (current == ptr) {
			*list = ptr->nextPtr;
		} else {
			LvlObject *prev = 0;
			if (current != ptr) {
				do {
					prev = current;
					current = current->nextPtr;
				} while (current && current != ptr);
			}
			assert(prev);
			prev->nextPtr = current->nextPtr;
		}
	}
}

void *Game::getLvlObjectDataPtr(LvlObject *o, int type) {
	switch (type) {
	case kObjectDataTypeAndy:
		assert(o == _andyObject);
		assert(o->dataPtr == &_andyObjectScreenData);
		break;
	case kObjectDataTypeAnimBackgroundData:
		assert(o->dataPtr >= &_animBackgroundDataTable[0] && o->dataPtr < &_animBackgroundDataTable[64]);
		break;
	case kObjectDataTypeOther:
		assert(o->dataPtr >= &_otherObjectScreenDataTable[0] && o->dataPtr < &_otherObjectScreenDataTable[32]);
		break;
	case kObjectDataTypeLvlBackgroundSound:
		assert(o->type == 1);
		// dataPtr is _res->_resLvlScreenBackgroundDataTable[num].backgroundSoundTable + 2
		assert(o->dataPtr);
		break;
	}
	return o->dataPtr;
}

void Game::lvlObjectType0Init(LvlObject *ptr) {
	uint8_t num = ptr->data0x2988;
	if (_currentLevel == 0 && _levelCheckpoint >= 5) {
		num = 2;
	}
	_andyObject = declareLvlObject(ptr->type, num);
	assert(_andyObject);
	_andyObject->xPos = ptr->xPos;
	_andyObject->yPos = ptr->yPos;
	_andyObject->screenNum = ptr->screenNum;
	_andyObject->anim = ptr->anim;
	_andyObject->frame = ptr->frame;
	_andyObject->flags2 = ptr->flags2;
	_andyObject->dataPtr = &_andyObjectScreenData;
	memset(&_andyObjectScreenData, 0, sizeof(_andyObjectScreenData));
}

void Game::lvlObjectType1Init(LvlObject *ptr) {
	AndyObjectScreenData *dataPtr = (AndyObjectScreenData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
	if (dataPtr->nextPtr) {
		return;
	}
	LvlObject *o = declareLvlObject(8, 1);
	assert(o);
	o->xPos = ptr->xPos;
	o->yPos = ptr->yPos;
	o->anim = 13;
	o->frame = 0;
	o->screenNum = ptr->screenNum;
	o->flags1 = bitmask_set(o->flags1, ptr->flags1, 0x30); // _esi->flags1 ^= (_esi->flags1 ^ ptr->flags1) & 0x30;
	o->flags2 = ptr->flags2 & ~0x2000;
	setupLvlObjectBitmap(o);
	prependLvlObjectToList(&_plasmaExplosionObject, o);

	o = declareLvlObject(8, 1);
	assert(o);
	dataPtr->nextPtr = o;
	o->xPos = ptr->xPos;
	o->yPos = ptr->yPos;
	o->anim = 5;
	o->frame = 0;
	o->screenNum = ptr->screenNum;
	o->flags1 = bitmask_set(o->flags1, ptr->flags1, 0x30); // _esi->flags1 ^= (_esi->flags1 ^ ptr->flags1) & 0x30;
	o->flags2 = ptr->flags2 & ~0x2000;
	setupLvlObjectBitmap(o);
	prependLvlObjectToList(&_plasmaExplosionObject, o);
}

void Game::lvlObjectTypeInit(LvlObject *o) {
	switch (o->data0x2988) {
	case 0:
	case 2:
		lvlObjectType0Init(o);
		break;
	case 1:
		lvlObjectType1Init(o);
		break;
	default:
		error("lvlObjectTypeInit unhandled case %d", o->data0x2988);
		break;
	}
}

void Game::lvlObjectType0CallbackHelper1() {
	uint8_t _bl, _cl, _dl;

	_cl = _dl = _andyObject->flags0;
	_bl = _andyObject->actionKeyMask;

	_dl &= 0x1F;
	_cl >>= 5;
	_cl &= 7;

	if (_currentLevel == 8 && (_bl & 4) != 0) {
		_bl &= ~4;
		_bl |= 8;
	}
	if (_dl == 3) {
		if (_cl == _dl) {
			_andyActionKeysFlags |= 2;
		}
	} else if (_dl == 7) {
		if (_cl == 5) {
			_andyActionKeysFlags |= _bl & 4;
		} else {
			_andyActionKeysFlags &= ~4;
		}
	}
	if ((_andyActionKeysFlags & 2) != 0) {
		if (_bl & 2) {
			_bl &= ~2;
		} else {
			_andyActionKeysFlags &= ~2;
		}
	}
	if (_andyActionKeysFlags & 4) {
		_bl |= 4;
	}
	if (_andyObject->data0x2988 == 2 && (_bl & 5) == 5) {
		AndyObjectScreenData *data = (AndyObjectScreenData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		LvlObject *o = data->nextPtr;
		if (o) {
			OtherObjectScreenData *dataUnk1 = (OtherObjectScreenData *)getLvlObjectDataPtr(o, kObjectDataTypeOther);
			if (dataUnk1->unk0 < 4) {
				_bl |= 0xC0;
			}
		}
	}
	if (_plasmaCannonFlags & 2) {
		_bl &= ~4;
	}
	_andyObject->actionKeyMask = (_bl & _andyActionKeyMaskAnd) | _andyActionKeyMaskOr;
	_bl = _andyObject->directionKeyMask;
	_andyObject->directionKeyMask = (_bl & _andyDirectionKeyMaskAnd) | _andyDirectionKeyMaskOr;
}

int Game::calcScreenMaskDx(int x, int y, int num) {
	const int offset = ((y & ~7) << 6) + (x >> 3); // screenMaskPos (8x8)
	int ret = -(x & 7);
	if (num & 1) {
		ret += 8;
		if (_screenMaskBuffer[offset] & 2) {
			return ret;
		} else if (_screenMaskBuffer[offset - 1] & 2) {
			return ret - 8;
		}
	} else {
		--ret;
		if (_screenMaskBuffer[offset] & 2) {
			return ret;
		} else if (_screenMaskBuffer[offset + 1] & 2) {
			return ret + 8;
		} else if (_screenMaskBuffer[offset + 2] & 2) {
			return ret + 16;
		}
	}
	return 0;
}

void Game::lvlObjectType0CallbackHelper3(LvlObject *ptr) {

	AndyObjectScreenData *_edi = (AndyObjectScreenData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);

	int xPos = ptr->xPos + ptr->posTable[7].x;
	int yPos = ptr->yPos + ptr->posTable[7].y;

// 409657
	// TODO: clipping with transformed screen

	if (addLvlObjectToList3(4)) {
		_lvlObjectsList3->xPos = xPos;
		_lvlObjectsList3->yPos = yPos + 24;
		_lvlObjectsList3->screenNum = ptr->screenNum;
		_lvlObjectsList3->anim = 0;
		_lvlObjectsList3->frame = 0;
		_lvlObjectsList3->flags2 += 1;
		_lvlObjectsList3->flags0 = (_lvlObjectsList3->flags0 & 0xFFE6) | 6;
		_lvlObjectsList3->flags1 &= ~0x20;
	}

	uint8_t _dl = _edi->unk3;
	static const int16_t word_43E53C[] = { 625, 937, 1094, 1250 };
// 40973C
	int _al = 0;
	while (_al < 4 && _edi->unk6 >= word_43E53C[_al]) {
		++_al;
	}
	_edi->unk3 = _al;
	static const uint8_t byte_43E534[] = { 22, 20, 18, 16, 14 };
	_edi->unk2 = byte_43E534[_al];
	if (_dl == 1) {
		if (_al == 3) {
			if (_actionDirectionKeyMaskIndex < 1) {
				_actionDirectionKeyMaskIndex = 1;
				_plasmaCannonKeyMaskCounter = 0;
			}
		}
	} else if (_dl == 3) {
// 4097A2
		if (_al == 4) {
			if (_actionDirectionKeyMaskIndex < 2) {
				_actionDirectionKeyMaskIndex = 2;
				_plasmaCannonKeyMaskCounter = 0;
			}
		}
	} else if (_dl == 4 && _edi->unk6 >= 1250) {
// 4097BD
		if (_actionDirectionKeyMaskIndex < 160) {
			_actionDirectionKeyMaskIndex = 160;
			_plasmaCannonKeyMaskCounter = 0;
		}
	}
// 4097E1
	if (!_lvlObjectsList3) {
		// warning("_lvlObjectsList3 is 0");
	} else {
		switch (_al) {
		case 0:
			_lvlObjectsList3->actionKeyMask = 1;
			break;
		case 1:
			_lvlObjectsList3->actionKeyMask = 2;
			break;
		case 2:
			_lvlObjectsList3->actionKeyMask = 4;
			break;
		case 3:
			_lvlObjectsList3->actionKeyMask = 8;
			break;
		default:
			_lvlObjectsList3->actionKeyMask = 16;
			break;
		}
	}

// 409809
	if (_edi->unk2 != 0) {
		--_edi->unk2;
	}
}

void Game::setupSpecialPowers(LvlObject *ptr) {
	AndyObjectScreenData *_edi = (AndyObjectScreenData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
	LvlObject *_esi = _edi->nextPtr;
	const uint8_t pos  = ptr->flags0 & 0x1F;
	uint8_t var1 = (ptr->flags0 >> 5) & 7;
	if (pos == 4) {
// 40DB4C
		OtherObjectScreenData *_eax = (OtherObjectScreenData *)getLvlObjectDataPtr(_esi, kObjectDataTypeOther);
		// _esi->callbackFuncPtr = &Game::lvlObjectCallbackSpecialPowers;
		uint8_t _cl = (ptr->flags1 >> 4) & 3;
		if (_eax->unk0 == 4) {
			_eax->unk2 = 0x21;
			switch (var1) {
			case 0:
				_eax->unk1 = ((_cl & 1) != 0) ? 5 : 0;
				break;
			case 1:
				_eax->unk1 = ((_cl & 1) != 0) ? 3 : 1;
				break;
			case 2:
				_eax->unk1 = ((_cl & 1) != 0) ? 4 : 2;
				break;
			case 3:
				_eax->unk1 = ((_cl & 1) != 0) ? 1 : 3;
				break;
			case 4:
				_eax->unk1 = ((_cl & 1) != 0) ? 2 : 4;
				break;
			case 5:
				_eax->unk1 = ((_cl & 1) != 0) ? 0 : 5;
				break;
			case 6:
				_eax->unk1 = 6;
				break;
			case 7:
				_eax->unk1 = 7;
				break;
			}
// 40DBCE
			// TODO
			_esi->anim = 10;
		} else {
// 40DBF4
			_eax->unk2 = 0x11;
			switch (var1) {
			case 0:
				_esi->anim = 13;
				_eax->unk1 = ((_cl & 1) != 0) ? 5 : 0;
				break;
			case 1:
				_esi->anim = 12;
				_eax->unk1 = ((_cl & 1) != 0) ? 3 : 1;
				_cl ^= 1;
				break;
			case 2:
				_esi->anim = 12;
				_eax->unk1 = ((_cl & 1) != 0) ? 4 : 2;
				_cl ^= 3;
				break;
			case 3:
				_esi->anim = 12;
				_eax->unk1 = ((_cl & 1) != 0) ? 1 : 3;
				break;
			case 4:
				_esi->anim = 12;
				_eax->unk1 = ((_cl & 1) != 0) ? 2 : 4;
				_cl ^= 2;
				break;
			case 5:
				_esi->anim = 13;
				_eax->unk1 = ((_cl & 1) != 0) ? 0 : 5;
				_cl ^= 1;
				break;
			case 6:
				_esi->anim = 11;
				_eax->unk1 = 6;
				break;
			case 7:
				_esi->anim = 11;
				_eax->unk1 = 7;
				_cl ^= 2;
				break;
			}
// 40DCCE
			// TODO
		}
// 40DCE9
		_esi->frame = 0;
		_esi->flags1 = (_esi->flags1 & ~0x30) | ((_cl & 3) << 4);
		setupLvlObjectBitmap(_esi);
		_esi->screenNum = ptr->screenNum;
		setLvlObjectPosRelativeToObject(_esi, 7, ptr, 6);
		if (_currentLevel == kLvl_isld) {
			_esi->xPos += _eax->boundingBox.x1;
		}
		_edi->nextPtr = 0;
	} else if (pos == 7) {
// 40DD4B
		switch (var1) {
		case 0:
			if (!_esi) {
				LvlObject *o = _edi->nextPtr;
				if (!o) {
					LvlObject *_edx = declareLvlObject(8, 3);
					_edi->nextPtr = _edx;
					_edx->dataPtr = _otherObjectScreenDataList;
					if (_otherObjectScreenDataList) {
						_otherObjectScreenDataList = _otherObjectScreenDataList->nextPtr;
						 memset(_edx->dataPtr, 0, sizeof(OtherObjectScreenData));
					}
					_edx->xPos = ptr->xPos;
					_edx->yPos = ptr->yPos;
					_edx->flags &= ~0x30;
					_edx->screenNum = ptr->screenNum;
					_edx->anim = 7;
					_edx->frame = 0;
					_edx->bitmapBits = 0;
					_edx->flags2 = (ptr->flags2 & 0xDFFF) - 1;
					prependLvlObjectToList(&_lvlObjectsList0, _edx);
				}
// 40DDEE
				if (_esi) {
					OtherObjectScreenData *_edx = (OtherObjectScreenData *)getLvlObjectDataPtr(_esi, kObjectDataTypeOther);
					_edx->unk0 = 0;
				}
				break;
			} else {
// 40DE08
				OtherObjectScreenData *_eax = (OtherObjectScreenData *)getLvlObjectDataPtr(_esi, kObjectDataTypeOther);
				_esi->anim = (_eax->unk0 == 0) ? 14 : 15;
				updateAndyObject(_esi);
				setLvlObjectPosRelativeToObject(_esi, 0, ptr, 6);
				if (_currentLevel == kLvl_isld) {
					_esi->xPos += _eax->boundingBox.x1;
				}
			}
			break;
		case 2: {
				LvlObject *_eax = _edi->nextPtr;
				if (_eax) {
					LvlObject *_edx = declareLvlObject(8, 3);
					_edi->nextPtr = _edx;
					_edx->dataPtr = _otherObjectScreenDataList;
					if (_otherObjectScreenDataList) {
						_otherObjectScreenDataList = _otherObjectScreenDataList->nextPtr;
						 memset(_edx->dataPtr, 0, sizeof(OtherObjectScreenData));
					}
					_edx->xPos = ptr->xPos;
					_edx->yPos = ptr->yPos;
					_edx->flags &= 0xFFCF;
					_edx->screenNum = ptr->screenNum;
					_edx->anim = 7;
					_edx->frame = 0;
					_edx->bitmapBits = 0;
					_edx->flags2 = (ptr->flags2 & 0xDFFF) - 1;
					prependLvlObjectToList(&_lvlObjectsList0, _edx);
				}
// 40DEEC
				if (_esi) {
					OtherObjectScreenData *_edx = (OtherObjectScreenData *)getLvlObjectDataPtr(_esi, kObjectDataTypeOther);
					_edx->unk0 = 0;
				}
			}
			break;
		case 1:
			if (_esi) {
				updateAndyObject(_esi);
				_esi->bitmapBits = 0;
			}
			break;
		case 3: {
				LvlObject *_eax = _edi->nextPtr;
				if (_eax) {
					LvlObject *_edx = declareLvlObject(8, 3);
					_edi->nextPtr = _edx;
					_edx->dataPtr = _otherObjectScreenDataList;
					if (_otherObjectScreenDataList) {
						_otherObjectScreenDataList = _otherObjectScreenDataList->nextPtr;
						memset(_edx->dataPtr, 0, sizeof(OtherObjectScreenData));
					}
					_edx->xPos = ptr->xPos;
					_edx->yPos = ptr->yPos;
					_edx->flags &= 0xFFCF;
					_edx->screenNum = ptr->screenNum;
					_edx->anim = 7;
					_edx->frame = 0;
					_edx->bitmapBits = 0;
					_edx->flags2 = (ptr->flags2 & 0xDFFF) - 1;
					prependLvlObjectToList(&_lvlObjectsList0, _edx);
				}
// 40DF82
				if (_esi) {
					OtherObjectScreenData *_edx = (OtherObjectScreenData *)getLvlObjectDataPtr(_esi, kObjectDataTypeOther);
					_edx->unk0 = 0;
				}
			}
			break;
		case 4:
			if (_esi) {
				_edi->nextPtr = 0;
				removeLvlObjectFromList(&_lvlObjectsList0, _esi);
				destroyLvlObject(_esi);
			}
			break;
		}
	}
}

int Game::lvlObjectType0Callback(LvlObject *ptr) {
	AndyObjectScreenData *_edi = 0;
	if (!_hideAndyObjectSprite) {
		_edi = (AndyObjectScreenData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
		_edi->unk4 = ptr->flags0 & 0x1F;
		_edi->unk5 = (ptr->flags0 >> 5) & 7;
		lvlObjectType0CallbackHelper1();
		updateAndyObject(ptr);
		if (_edi->unk4 == (ptr->flags0 & 0x1F) && _edi->unk4 == 5) {
			_fallingAndyFlag = true;
		} else {
			_fallingAndyFlag = false;
			_fallingAndyCounter = 0;
		}
		_edi->boundingBox.x1 = ptr->xPos;
		_edi->boundingBox.x2 = ptr->xPos + ptr->width - 1;
		_edi->boundingBox.y1 = ptr->yPos;
		_edi->boundingBox.y2 = ptr->yPos + ptr->height - 1;
		if ((ptr->flags0 & 0x300) == 0x100) {
			const int y = _res->_screensBasePos[_res->_currentScreenResourceNum].v + ptr->posTable[3].y + ptr->yPos;
			const int x = _res->_screensBasePos[_res->_currentScreenResourceNum].u + ptr->posTable[3].x + ptr->xPos;
			ptr->xPos += calcScreenMaskDx(x, y, (ptr->flags1 >> 4) & 3);
		} else if ((ptr->flags1 & 6) == 2) {
			ptr->yPos += calcScreenMaskDy(ptr->posTable[7].x + ptr->xPos, ptr->posTable[7].y + ptr->yPos, ptr->screenNum);
		}
	} else if (ptr->linkObjPtr) {
		assert(_currentMonsterObject);
		if (_currentMonsterObject->screenNum != ptr->screenNum) {
			setLvlObjectPosRelativeToObject(ptr, 3, _currentMonsterObject, 6);
		}
	}
	switch (_currentLevel) {
	case 0:
	case 7:
		if (ptr->data0x2988 == 0) {
			setupPlasmaCannonPoints(ptr);
		}
		break;
	case 9:
		if (ptr->data0x2988 != 0) {
			setupSpecialPowers(ptr);
		} else {
			setupPlasmaCannonPoints(ptr);
		}
		break;
	case 2:
		if (!_hideAndyObjectSprite && _edi->unk4 == 6) {
			lvlObjectType0CallbackHelper3(ptr);
		}
		// fall through
	case 3:
	case 4:
	case 5:
	case 6:
		setupSpecialPowers(ptr);
		break;
	}
	if (!_hideAndyObjectSprite) {
		assert(_edi);
		_edi->unk0 = ptr->actionKeyMask;
	}
	return _hideAndyObjectSprite;
}

int Game::lvlObjectType1Callback(LvlObject *ptr) {
	if (ptr) {
		ptr->actionKeyMask = 0;
		switch (_plasmaCannonDirection - 1) {
		case 0:
			ptr->directionKeyMask = 1;
			break;
		case 2:
			ptr->directionKeyMask = 3;
			break;
		case 1:
			ptr->directionKeyMask = 2;
			break;
		case 5:
			ptr->directionKeyMask = 6;
			break;
		case 3:
			ptr->directionKeyMask = 4;
			break;
		case 11:
			ptr->directionKeyMask = 12;
			break;
		case 7:
			ptr->directionKeyMask = 8;
			break;
		case 8:
			ptr->directionKeyMask = 9;
			break;
		default:
			ptr->directionKeyMask = 0;
			break;
		}
		setLvlObjectPosRelativeToPoint(ptr, 0, _gameXPosTable[_plasmaCannonFirstIndex], _gameYPosTable[_plasmaCannonFirstIndex]);
		updateAndyObject(ptr);
		ptr->flags2 = bitmask_set(ptr->flags2, _andyObject->flags2, 0x18);
		ptr->flags2 = bitmask_set(ptr->flags2, _andyObject->flags2 + 1, 7);
	}
	return 0;
}

int Game::lvlObjectType8Callback(LvlObject *ptr) {
	if (_mstLogicDisabled) {
		ptr->actionKeyMask = _andyObject->actionKeyMask;
		ptr->directionKeyMask = _andyObject->directionKeyMask;
		if (_andyObject->data0x2988 == 2 && _lvlObjectsList0) {
			// game_unk24
		}
		updateAndyObject(ptr);
		setLvlObjectPosInScreenGrid(ptr, 7);
		if ((ptr->flags1 & 6) == 2) {
			ptr->yPos += calcScreenMaskDy(ptr->xPos + ptr->posTable[5].x, ptr->yPos + ptr->posTable[5].y, ptr->screenNum);
		}
	} else {
		const void *dataPtr = ptr->dataPtr;
		if (dataPtr) {
			if (dataPtr >= &_mstUnkDataTable[0] && dataPtr <= &_mstUnkDataTable[32]) {
				MstUnkData *m = (MstUnkData *)ptr->dataPtr;
				if (m->flagsA6 & 2) {
					m->o16->actionKeyMask = _mstCurrentActionKeyMask;
					m->o16->directionKeyMask = _andyObject->directionKeyMask;
				}
				if (m->flagsA6 & 8) {
					ptr->bitmapBits = 0;
				}
			} else {
				// MstUnkData *m = ((MstObject *)dataPtr)->unk8;
// 402B9F
			}
		} else {
			ptr->bitmapBits = 0;
		}
	}
	return 0;
}

int Game::lvlObjectList3Callback(LvlObject *o) {
	if ((o->data0x2988 <= 7 && (o->flags0 & 0x1F) == 0xB) || o->flags0 == 0x1F) {
		if (_lvlObjectsList3 && o) {
			if (o != _lvlObjectsList3) {
				LvlObject *prev = 0;
				LvlObject *ptr = _lvlObjectsList3;
				do {
					prev = ptr;
					ptr = ptr->nextPtr;
				} while (ptr && ptr != o);
				assert(ptr);
				prev->nextPtr = ptr->nextPtr;
			} else {
				_lvlObjectsList3 = o->nextPtr;
			}
		}
		if (o->type == 8) {
			_res->decLevelData0x2988RefCounter(o);
			o->nextPtr = _declaredLvlObjectsListHead;
			--_declaredLvlObjectsListCount;
			_declaredLvlObjectsListHead = o;
		} else {
			switch (o->data0x2988) {
			case 0:
			case 2:
				o->dataPtr = 0;
				break;
			case 3:
			case 7:
				if (o->dataPtr) {
					prependObjectScreenDataList(o);
				}
				break;
			}
		}
		if (o->sssObj) {
			removeSound(o);
		}
		o->sssObj = 0;
		o->bitmapBits = 0;
	} else {
		updateAndyObject(o);
		o->actionKeyMask = 0;
		o->directionKeyMask = 0;
		if (o->soundToPlay != 0xFFFF) {
			playSound(o->soundToPlay, o, 0, 3);
		}
		if (o->bitmapBits) {
			addToSpriteList(o);
		}
	}
	return 0;
}

void Game::lvlObjectTypeCallback(LvlObject *o) {
	switch (o->data0x2988) {
	case 0:
	case 2:
		o->callbackFuncPtr = &Game::lvlObjectType0Callback;
		break;
	case 1:
		o->callbackFuncPtr = &Game::lvlObjectType1Callback;
		break;
	case 3:
	case 4:
	case 5:
	case 6:
	case 30:
	case 31:
	case 32:
	case 33:
	case 34:
		// nop
		break;
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29: // objects driven by .mst code
		o->callbackFuncPtr = &Game::lvlObjectType8Callback;
		break;
	default:
		warning("lvlObjectTypeCallback unhandled case %d", o->data0x2988);
		break;
	}
}

LvlObject *Game::addLvlObject(int type, int y, int x, int screen, int num, int o_anim, int o_flags1, int o_flags2, int actionKeyMask, int directionKeyMask) {
	LvlObject *ptr = 0;
	switch (type) {
	case 0:
		addLvlObjectToList1(8, num);
		ptr = _lvlObjectsList1;
		break;
	case 1:
		if (screen != _currentScreen && screen != _currentLeftScreen && screen != _currentRightScreen) {
			return 0;
		}
		ptr = findLvlObjectType2(num, screen);
		break;
	case 2:
		addLvlObjectToList3(num);
		ptr = _lvlObjectsList3;
		break;
	}
	if (!ptr) {
		return 0;
	}
	ptr->anim = o_anim;
	ptr->flags2 = o_flags2;
	ptr->frame = 0;
	ptr->flags1 = ((o_flags1 & 3) << 4) | (ptr->flags1 & ~0x30);
	ptr->actionKeyMask = actionKeyMask;
	ptr->directionKeyMask = directionKeyMask;
	setupLvlObjectBitmap(ptr);
	ptr->xPos = x - ptr->posTable[7].x;
	ptr->yPos = y - ptr->posTable[7].y;
	ptr->screenNum = screen;
	return ptr;
}

int Game::setLvlObjectPosInScreenGrid(LvlObject *o, int num) {
	int ret = 0;
	if (o->screenNum < _res->_lvlHdr.screensCount) {
		int xPrev = o->xPos;
		int x = o->xPos + o->posTable[num].x;
		int yPrev = o->yPos;
		int y = o->yPos + o->posTable[num].y;
		int numPrev = o->screenNum;
		if (x < 0) {
			o->screenNum = num = _res->_screensGrid[num * 4 + kPosLeftScreen];
			x += 256;
		} else if (x >= 256) {
			o->screenNum = num = _res->_screensGrid[num * 4 + kPosRightScreen];
			x -= 256;
		}
		o->xPos = x;
		if (y < 0 && num != 0xFF) {
			o->screenNum = _res->_screensGrid[num * 4 + kPosTopScreen];
			y += 192;
		} else if (y >= 192) {
			assert(num != 0xFF);
			o->screenNum = _res->_screensGrid[num * 4 + kPosBottomScreen];
			y -= 192;
		}
		o->yPos = y;
		if (o->screenNum == 0xFF) {
			o->xPos = xPrev;
			o->yPos = yPrev;
			o->screenNum = numPrev;
			ret = -1;
		} else if (o->screenNum != num) {
			ret = 1;
		}
	}
	return ret;
}

LvlObject *Game::declareLvlObject(uint8_t type, uint8_t num) {
	if (type != 8 || _res->_resLevelData0x2988PtrTable[num] != 0) {
		if (_declaredLvlObjectsListCount < 160) {
			assert(_declaredLvlObjectsListHead);
			LvlObject *ptr = _declaredLvlObjectsListHead;
			_declaredLvlObjectsListHead = _declaredLvlObjectsListHead->nextPtr;
			assert(ptr);
			++_declaredLvlObjectsListCount;
			ptr->data0x2988 = num;
			ptr->type = type;
			if (type == 8) {
				_res->incLevelData0x2988RefCounter(ptr);
				lvlObjectTypeCallback(ptr);
			}
			ptr->unk22 = 0;
			ptr->sssObj = 0;
			ptr->nextPtr = 0;
			ptr->bitmapBits = 0;
			return ptr;
		}
	}
	return 0;
}

void Game::clearDeclaredLvlObjectsList() {
	memset(_declaredLvlObjectsList, 0, sizeof(_declaredLvlObjectsList));
	for (int i = 0; i < 159; ++i) {
		_declaredLvlObjectsList[i].nextPtr = &_declaredLvlObjectsList[i + 1];
	}
	_declaredLvlObjectsList[159].nextPtr = 0;
	_declaredLvlObjectsListHead = &_declaredLvlObjectsList[0];
	_declaredLvlObjectsListCount = 0;
}

void Game::initLvlObjects() {
	for (int i = 0; i < _res->_lvlHdr.screensCount; ++i) {
		_screenLvlObjectsList[i] = 0;
	}
	LvlObject *prevLvlObj = 0;
	for (int i = 0; i < _res->_lvlHdr.staticLvlObjectsCount; ++i) {
		LvlObject *ptr = &_res->_resLvlScreenObjectDataTable[i];
		int index = ptr->screenNum;
		ptr->nextPtr = _screenLvlObjectsList[index];
		_screenLvlObjectsList[index] = ptr;
		switch (ptr->type) {
		case 0:
			assert(_animBackgroundDataCount < 64);
			ptr->dataPtr = &_animBackgroundDataTable[_animBackgroundDataCount++];
			memset(ptr->dataPtr, 0, sizeof(AnimBackgroundData));
			break;
		case 1:
			debug(kDebug_GAME, "Trying to free _resLvlScreenBackgroundDataTable.backgroundSoundTable ; ignored (i=%d index=%d)", i, index);
#if 0
			if (ptr->dataPtr) {
				free(ptr->dataPtr);
				ptr->dataPtr = 0;
			}
#endif
			ptr->xPos = 0;
			ptr->yPos = 0;
			break;
		case 2:
			if (prevLvlObj == &_res->_dummyObject) {
				prevLvlObj = 0;
				ptr->linkObjPtr = ptr->nextPtr;
			} else {
				prevLvlObj = ptr->linkObjPtr;
			}
			break;
		}
	}
	for (int i = _res->_lvlHdr.staticLvlObjectsCount; i < _res->_lvlHdr.staticLvlObjectsCount + _res->_lvlHdr.otherLvlObjectsCount; ++i) {
		LvlObject *ptr = &_res->_resLvlScreenObjectDataTable[i];
		lvlObjectTypeInit(ptr);
	}
}

void Game::setLvlObjectType8Resource(LvlObject *ptr, uint8_t type, uint8_t num) {
	if (ptr->type == 8) {
		_res->decLevelData0x2988RefCounter(ptr);
		ptr->data0x2988 = num;
		ptr->type = type;
		_res->incLevelData0x2988RefCounter(ptr);
	}
}

LvlObject *Game::findLvlObject(uint8_t type, uint8_t num, int index) {
	LvlObject *ptr = _screenLvlObjectsList[index];
	while (ptr) {
		if (ptr->type == type && ptr->data0x2988 == num) {
			break;
		}
		ptr = ptr->nextPtr;
	}
	return ptr;
}

LvlObject *Game::findLvlObject2(uint8_t type, uint8_t flags, int index) {
	LvlObject *ptr = _screenLvlObjectsList[index];
	while (ptr) {
		if (ptr->type == type && ptr->flags == flags) {
			break;
		}
		ptr = ptr->nextPtr;
	}
	return ptr;
}

LvlObject *Game::findLvlObjectType2(int num, int index) {
	LvlObject *ptr = _screenLvlObjectsList[index];
	while (ptr) {
		if (ptr->type == 2 && ptr->data0x2988 == num && !ptr->dataPtr) {
			break;
		}
		ptr = ptr->nextPtr;
	}
	return ptr;
}

void Game::resetLevelTickHelperData() {
	// TODO
}

void Game::updateLevelTickHelper() {
	// TODO
}

void Game::captureScreenshot() {
	static int screenshot = 1;
	char name[64];

	snprintf(name, sizeof(name), "screenshot-%03d-front.bmp", screenshot);
	saveBMP(name, _video->_frontLayer, _video->_palette, Video::kScreenWidth, Video::kScreenHeight);

	snprintf(name, sizeof(name), "screenshot-%03d-background.bmp", screenshot);
	saveBMP(name, _video->_backgroundLayer, _video->_palette, Video::kScreenWidth, Video::kScreenHeight);

	snprintf(name, sizeof(name), "screenshot-%03d-shadow.bmp", screenshot);
	saveBMP(name, _video->_shadowLayer, _video->_palette, Video::kScreenWidth, Video::kScreenHeight);

	static const int kPaletteRectSize = 8;
	uint8_t paletteBuffer[8 * 256 * 8];
	for (int x = 0; x < 256; ++x) {
		const int xOffset = x * kPaletteRectSize;
		for (int y = 0; y < kPaletteRectSize; ++y) {
			memset(paletteBuffer + xOffset + y * 256 * kPaletteRectSize, x, kPaletteRectSize);
		}
	}

	snprintf(name, sizeof(name), "screenshot-%03d-palette.bmp", screenshot);
	saveBMP(name, paletteBuffer, _video->_palette, 256 * kPaletteRectSize, kPaletteRectSize);

	++screenshot;
}
