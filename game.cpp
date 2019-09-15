/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include "game.h"
#include "fileio.h"
#include "level.h"
#include "lzw.h"
#include "paf.h"
#include "screenshot.h"
#include "systemstub.h"
#include "util.h"
#include "video.h"

// cutscene number when starting a level
static const uint8_t _cutscenes[] = { 0, 2, 4, 5, 6, 8, 10, 14, 19 };

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

Game::Game(SystemStub *system, const char *dataPath) {

	_level = 0;
	_res = new Resource(dataPath);
	_paf = new PafPlayer(system, &_res->_fs);
	_rnd.setSeed();
	_video = new Video(system);
	_system = system;
	_cheats = 0;

	_difficulty = 1;

	memset(_screenLvlObjectsList, 0, sizeof(_screenLvlObjectsList));
	_andyObject = 0;
	_plasmaExplosionObject = 0;
	_plasmaCannonObject = 0;
	memset(_spritesTable, 0, sizeof(_spritesTable));
	memset(_spriteListPtrTable, 0, sizeof(_spriteListPtrTable));

	_shadowScreenMaskBuffer = (uint8_t *)malloc(99328);
	_transformShadowBuffer = 0;
	_transformShadowLayerDelta = 0;

	_directionKeyMask = 0;
	_actionKeyMask = 0;

	_currentScreen = 0;

	_lvlObjectsList0 = 0;
	_lvlObjectsList1 = 0;
	_lvlObjectsList2 = 0;
	_lvlObjectsList3 = 0;
	memset(_screenMaskBuffer, 0, sizeof(_screenMaskBuffer));
	memset(_shootLvlObjectDataTable, 0, sizeof(_shootLvlObjectDataTable));
	_mstAndyCurrentScreenNum = -1;
	_plasmaCannonDirection = 0;
	_andyActionKeyMaskAnd = 0xFF;
	_andyActionKeyMaskOr = 0;
	_andyDirectionKeyMaskAnd = 0xFF;
	_andyDirectionKeyMaskOr = 0;

	_specialAnimMask = 0; // original only clears ~0x30
	_mstCurrentAnim = 0;
	_mstOriginPosX = Video::W / 2;
	_mstOriginPosY = Video::H / 2;
	memset(_declaredLvlObjectsList, 0, sizeof(_declaredLvlObjectsList));
	_declaredLvlObjectsListHead = 0;
	_declaredLvlObjectsListCount = 0;

	memset(_animBackgroundDataTable, 0, sizeof(_animBackgroundDataTable));
	_animBackgroundDataCount = 0;

	memset(_monsterObjects1Table, 0, sizeof(_monsterObjects1Table));
	memset(_monsterObjects2Table, 0, sizeof(_monsterObjects2Table));

	_sssObjectsCount = 0;
	_sssObjectsList1 = 0;
	_sssObjectsList2 = 0;
	_playingSssObjectsMax = 16; // 10 if (lowMemory || slowCPU)
	_snd_masterPanning = 64;
	_snd_masterVolume = 128;
	_snd_muted = false;
}

Game::~Game() {
	delete _paf;
	delete _res;
	delete _video;
	free(_shadowScreenMaskBuffer);
}

void Game::resetShootLvlObjectDataTable() {
	_shootLvlObjectDataList = &_shootLvlObjectDataTable[0];
	for (int i = 0; i < kMaxShootLvlObjectData - 1; ++i) {
		_shootLvlObjectDataTable[i].nextPtr = &_shootLvlObjectDataTable[i + 1];
	}
	_shootLvlObjectDataTable[kMaxShootLvlObjectData - 1].nextPtr = 0;
}

void Game::clearShootLvlObjectData(LvlObject *ptr) {
	ShootLvlObjectData *dat = (ShootLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeShoot);
	dat->nextPtr = _shootLvlObjectDataList;
	_shootLvlObjectDataList = dat;
	ptr->dataPtr = 0;
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
	if (!_fadePalette) {
		assert(_levelRestartCounter != 0);
		for (int i = 0; i < 256 * 3; ++i) {
			_fadePaletteBuffer[i] = _video->_displayPaletteBuffer[i] / _levelRestartCounter;
		}
		_fadePalette = true;
	} else {
		if (_levelRestartCounter != 0) {
			_snd_masterVolume -= _snd_masterVolume / _levelRestartCounter;
			if (_snd_masterVolume < 0) {
				_snd_masterVolume = 0;
			}
		}
		--_levelRestartCounter;
	}
	for (int i = 0; i < 256 * 3; ++i) {
		int color = _video->_displayPaletteBuffer[i] - _fadePaletteBuffer[i];
		if (color < 0) {
			 color = 0;
		}
		_video->_displayPaletteBuffer[i] = color;
	}
	_video->_paletteNeedRefresh = true;
}

void Game::shakeScreen() {
	if (_video->_displayShadowLayer) {
		const int num = (_currentLevel == kLvl_lava || _currentLevel == kLvl_lar1) ? 1 : 4;
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
	if (_levelRestartCounter != 0) {
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
	for (int y = 0; y < Video::H; ++y) {
		if (0) {
			for (int x = 0; x < Video::W - 6; ++x) {
				const int offset = x + *src++;
				*dst++ = _video->_frontLayer[y * Video::W + offset];
			}
			memset(dst, 0xC4, 6);
			dst += 6;
			src += 6;
		} else {
			for (int x = 0; x < Video::W; ++x) {
				const int offset = MIN(255, x + *src++);
				*dst++ = _video->_frontLayer[y * Video::W + offset];
			}
		}
	}
	uint8_t r = 0;
	if (_currentLevel == kLvl_pwr1) {
		r = _pwr1_screenTransformLut[_res->_currentScreenResourceNum * 2 + 1];
	} else if (_currentLevel == kLvl_pwr2) {
		r = _pwr2_screenTransformLut[_res->_currentScreenResourceNum * 2 + 1];
	}
	if (r != 0) {
		assert(r < ARRAYSIZE(_screenTransformRects));
		const BoundingBox *b = &_screenTransformRects[r];
		const int offset = b->y1 * Video::W + b->x1;
		src = _video->_frontLayer + offset;
		dst = _video->_shadowLayer + offset;
		for (int y = b->y1; y < b->y2; ++y) {
			for (int x = b->x1; x < b->x2; ++x) {
				dst[x] = src[x];
			}
			dst += Video::W;
			src += Video::W;
		}
	}
}

void Game::decodeShadowScreenMask(LvlBackgroundData *lvl) {
	uint8_t *dst = _shadowScreenMaskBuffer;
	for (int i = lvl->currentShadowId; i < lvl->shadowCount; ++i) {
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

// a: type/source (0, 1, 2) b: num/index (3, monster1Index, monster2.monster1Index)
void Game::playSound(int num, LvlObject *ptr, int a, int b) {
	MixerLock ml(&_mix);
	if (num < _res->_sssHdr.infosDataCount) {
		debug(kDebug_GAME, "playSound num %d/%d a=%d b=%d", num, _res->_sssHdr.infosDataCount, a, b);
		_currentSoundLvlObject = ptr;
		playSoundObject(&_res->_sssInfosData[num], a, b);
		_currentSoundLvlObject = 0;
	}
}

void Game::removeSound(LvlObject *ptr) {
	MixerLock ml(&_mix);
	for (int i = 0; i < _sssObjectsCount; ++i) {
		if (_sssObjectsTable[i].lvlObject == ptr) {
			_sssObjectsTable[i].lvlObject = 0;
		}
	}
	ptr->sssObject = 0;
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
	if (lvl->shadowCount != 0) {
		decodeShadowScreenMask(lvl);
	}
	for (int i = 0; i < 256 * 3; ++i) {
		_video->_displayPaletteBuffer[i] = pal[i] << 8;
	}
	_video->_paletteNeedRefresh = true;
}

void Game::addToSpriteList(LvlObject *ptr) {
	Sprite *spr = _spritesListNextPtr;
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
			spr->yPos -= Video::H;
		} else if (index == bottomScreenId) {
			spr->yPos += Video::H;
		} else if (index == rightScreenId) {
			spr->xPos += Video::W;
		} else if (index == leftScreenId) {
			spr->xPos -= Video::W;
		} else if (index != _res->_currentScreenResourceNum) {
			return;
		}
		if (spr->xPos >= Video::W || spr->xPos + ptr->width < 0) {
			return;
		}
		if (spr->yPos >= Video::H || spr->yPos + ptr->height < 0) {
			return;
		}
		if (_currentLevel == kLvl_isld && ptr->spriteNum == 2) {
			AndyLvlObjectData *dataPtr = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
			spr->xPos += dataPtr->dxPos;
		}
		if (READ_LE_UINT16(ptr->bitmapBits) > 8) {
			spr->bitmapBits = ptr->bitmapBits;
			_spritesListNextPtr = spr->nextPtr;
			index = (ptr->flags2 & 31);
			spr->nextPtr = _spriteListPtrTable[index];
			_spriteListPtrTable[index] = spr;
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
	int _esi = screenMaskOffset(_res->_screensBasePos[num].u + xPos, _res->_screensBasePos[num].v + yPos);
	int _edi = screenGridOffset(xPos, yPos);
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
			const uint8_t *p = _res->getLvlScreenPosDataPtr(src[i] * 4 + index);
			if (p) {
				Video::decodeRLE(p, _screenPosTable[i], 768);
				continue;
			}
		}
		memset(_screenPosTable[i], 0, 768);
	}
	int index = _res->_resLvlScreenBackgroundDataTable[num].currentMaskId;
	const uint8_t *p = _res->getLvlScreenPosDataPtr(num * 4 + index);
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
		uint8_t *p = _res->getLvlScreenMaskDataPtr(num * 4 + mask);
		if (p) {
			Video::decodeRLE(p, _screenTempMaskBuffer, 768);
		} else {
			memset(_screenTempMaskBuffer, 0, 768);
		}
		p = _screenMaskBuffer + screenMaskOffset(_res->_screensBasePos[num].u, _res->_screensBasePos[num].v);
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

void Game::setScreenMaskRectHelper(int x1, int y1, int x2, int y2, int screenNum) {
	if (screenNum != 0xFF) {

		int index = _res->_resLvlScreenBackgroundDataTable[screenNum].currentMaskId;
		const uint8_t *p = _res->getLvlScreenPosDataPtr(screenNum * 4 + index);
		Video::decodeRLE(p, _screenTempMaskBuffer, 768);

		int h = (y2 - y1 + 7) >> 3;
		int w = (x2 - x1 + 7) >> 3;

		const int x = x1 - _res->_screensBasePos[screenNum].u;
		const int y = y1 - _res->_screensBasePos[screenNum].v;

		const int u = _res->_screensBasePos[screenNum].u + 256;
		if (x2 < u) {
			++w;
		}

		const uint8_t *src = _screenTempMaskBuffer + screenGridOffset(x, y);
		uint8_t *dst = _screenMaskBuffer + screenMaskOffset(y1, x1);
		for (int i = 0; i < h; ++i) {
			memcpy(dst, src, w);
			src += 32;
			dst += 512;
		}
	}
}

void Game::setScreenMaskRect(int x1, int y1, int x2, int y2, int screenNum) {
	const int u = _res->_screensBasePos[screenNum].u;
	const int v = _res->_screensBasePos[screenNum].v;
	const int topScreen = _res->_screensGrid[screenNum * 4 + kPosTopScreen];
	if (x1 < u || y1 < v || y2 >= v + 192) {
		if (topScreen != 255) {
			const int u2 = _res->_screensBasePos[topScreen].u;
			const int v2 = _res->_screensBasePos[topScreen].v;
			if (x1 >= u2 && y1 >= v2 && y2 < v + 192) {
				setScreenMaskRectHelper(u2, y1, x2, v2 + 192, topScreen);
			}
		}
	}
	setScreenMaskRectHelper(u, v, x2, y2, screenNum);
}

void Game::updateScreenMaskBuffer(int x, int y, int type) {
	uint8_t *p = _screenMaskBuffer + screenMaskOffset(x, y);
	switch (type) {
	case 0:
		p[0] &= ~8;
		break;
	case 1:
		p[0] |= 8;
		break;
	case 2:
		p[0] |= 8;
		p[-1] = 0;
		p[-2] = 0;
		p[-3] = 0;
		break;
	case 3:
		p[0] |= 8;
		p[1] = 0;
		p[2] = 0;
		p[3] = 0;
		break;
	}
}

void Game::setupLvlObjectBitmap(LvlObject *ptr) {
	LvlObjectData *dat = ptr->levelData0x2988;
	if (!dat) {
		return;
	}
	LvlAnimHeader *ah = (LvlAnimHeader *)(dat->animsInfoData + kLvlAnimHdrOffset) + ptr->anim;
	LvlAnimSeqHeader *ash = (LvlAnimSeqHeader *)(dat->animsInfoData + ah->seqOffset) + ptr->frame;

	ptr->currentSound = ash->sound;
	ptr->flags0 = merge_bits(ptr->flags0, ash->flags0, 0x3FF);
	ptr->flags1 = merge_bits(ptr->flags1, ash->flags1, 6);
	ptr->flags1 = merge_bits(ptr->flags1, ash->flags1, 8);
	ptr->currentSprite = ash->firstFrame;

	ptr->bitmapBits = _res->getLvlSpriteFramePtr(dat, ash->firstFrame);

	ptr->width = READ_LE_UINT16(ptr->bitmapBits + 2);
	ptr->height = READ_LE_UINT16(ptr->bitmapBits + 4);

	const int w = ptr->width - 1;
	const int h = ptr->height - 1;

	if (ptr->type == 8 && (ptr->spriteNum == 2 || ptr->spriteNum == 0)) {
		AndyLvlObjectData *dataPtr = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
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
		while ((xPos = _plasmaCannonXPointsTable1[var1]) >= 0) {
			xPos += _edi;
			_al = _screenMaskBuffer[screenMaskOffset(xPos, yPos)];
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
			xPos += _edi;
			_al = _screenMaskBuffer[screenMaskOffset(xPos, yPos)];
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
			const int x1 = _plasmaCannonPosX[_pointSrcIndex1Table[index]];
			const int x2 = _plasmaCannonPosX[_pointSrcIndex2Table[index]];
			_plasmaCannonPosX[index] = (x1 + x2 + (xR >> _pointRandomizeShiftTable[i])) / 2;
			xR *= 2;
		}
		int yR = _rnd.update();
		for (int i = 0; i < 64; ++i) {
			const int index = _pointDstIndexTable[i];
			const int y1 = _plasmaCannonPosY[_pointSrcIndex1Table[index]];
			const int y2 = _plasmaCannonPosY[_pointSrcIndex2Table[index]];
			_plasmaCannonPosY[index] = (y1 + y2 + (yR >> _pointRandomizeShiftTable[i])) / 2;
			yR *= 2;
		}
		if (_andyObject->anim == 84) {
			for (int i = 0; i <= 496; i += 8) {
				const int index = i / 4;
				_plasmaCannonXPointsTable2[2 + index] = (_plasmaCannonPosX[4 + index] - _plasmaCannonXPointsTable1[4 + index]) / 2;
				_plasmaCannonYPointsTable2[2 + index] = (_plasmaCannonPosY[4 + index] - _plasmaCannonYPointsTable1[4 + index]) / 8;
			}
		} else {
			for (int i = 0; i <= 496; i += 8) {
				const int index = i / 4;
				_plasmaCannonXPointsTable2[2 + index] = (_plasmaCannonPosX[4 + index] - _plasmaCannonXPointsTable1[4 + index]) / 2;
				_plasmaCannonYPointsTable2[2 + index] = (_plasmaCannonPosY[4 + index] - _plasmaCannonYPointsTable1[4 + index]) / 2;
			}
		}
		for (int i = 0; i <= 504; i += 8) {
			const int index = i / 4;
			_plasmaCannonXPointsTable1[2 + index] = _plasmaCannonPosX[2 + index];
			_plasmaCannonYPointsTable1[2 + index] = _plasmaCannonPosY[2 + index];
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
	AndyLvlObjectData *l = (AndyLvlObjectData *)getLvlObjectDataPtr(o, kObjectDataTypeAndy);
	if (l->shootLvlObject) {
		l->shootLvlObject = 0;

		assert(_plasmaExplosionObject);

		LvlObject *ptr = _plasmaExplosionObject->nextPtr;
		removeLvlObjectFromList(&_plasmaExplosionObject, ptr);
		destroyLvlObject(ptr);

		ptr = _plasmaExplosionObject;
		removeLvlObjectFromList(&_plasmaExplosionObject, ptr);
		destroyLvlObject(ptr);
	}
}

void Game::shuffleArray(uint8_t *p, int count) {
	for (int i = 0; i < count * 2; ++i) {
		const int index1 = _rnd.update() % count;
		const int index2 = _rnd.update() % count;
		SWAP(p[index1], p[index2]);
	}
}

void Game::destroyLvlObject(LvlObject *o) {
	assert(o);
	if (o->type == 8) {
		_res->decLvlSpriteDataRefCounter(o);
		o->nextPtr = _declaredLvlObjectsListHead;
		--_declaredLvlObjectsListCount;
		_declaredLvlObjectsListHead = o;
		switch (o->spriteNum) {
		case 0:
		case 2:
			o->dataPtr = 0;
			break;
		case 3:
		case 7:
			if (o->dataPtr) {
				clearShootLvlObjectData(o);
			}
			break;
		}
	}
	if (o->sssObject) {
		removeSound(o);
	}
	o->sssObject = 0;
	o->bitmapBits = 0;
}

void Game::setupPlasmaCannonPoints(LvlObject *ptr) {
	_plasmaCannonDirection = 0;
	if ((ptr->flags0 & 0x1F) == 4) {
		if ((ptr->actionKeyMask & 4) == 0) { // not using plasma cannon
			destroyLvlObjectPlasmaExplosion(ptr);
		} else {
			_plasmaCannonPosX[0] = _plasmaCannonPosX[128] = ptr->xPos + ptr->posTable[6].x;
			_plasmaCannonPosY[0] = _plasmaCannonPosY[128] = ptr->yPos + ptr->posTable[6].y;
			const int num = ((ptr->flags0 >> 5) & 7) - 3;
			switch (num) {
			case 0:
				_plasmaCannonPosY[128] -= 176; // 192 - 16
				_plasmaCannonDirection = 3;
				break;
			case 1:
				_plasmaCannonPosY[128] += 176;
				_plasmaCannonDirection = 6;
				break;
			case 3:
				_plasmaCannonPosY[128] -= 176;
				_plasmaCannonDirection = 1;
				break;
			case 4:
				_plasmaCannonPosY[128] += 176;
				_plasmaCannonDirection = 4;
				break;
			default:
				_plasmaCannonDirection = 2;
				break;
			}
			if (ptr->flags1 & 0x10) {
				if (_plasmaCannonDirection != 1) {
					_plasmaCannonDirection = (_plasmaCannonDirection & ~2) | 8;
					_plasmaCannonPosX[128] -= 264; // 256 + 8
				}
			} else {
				if (_plasmaCannonDirection != 1) {
					_plasmaCannonPosX[128] += 264;
				}
			}
			if (_plasmaCannonPrevDirection != _plasmaCannonDirection) {
				_plasmaCannonXPointsTable1[0] = _plasmaCannonPosX[0];
				_plasmaCannonXPointsTable1[128] = _plasmaCannonPosX[128];
				randomizeInterpolatePoints(_plasmaCannonXPointsTable1, 64);
				_plasmaCannonYPointsTable1[0] = _plasmaCannonPosY[0];
				_plasmaCannonYPointsTable1[128] = _plasmaCannonPosY[128];
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
	int _esi = _plasmaCannonPosX[index1];
	int _ebp = _plasmaCannonPosY[index1];
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
	MixerLock ml(&_mix);
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

void Game::clearLvlObjectsList0() {
	LvlObject *ptr = _lvlObjectsList0;
	while (ptr) {
		LvlObject *next = ptr->nextPtr;
		if (ptr->type == 8) {
			_res->decLvlSpriteDataRefCounter(ptr);
			ptr->nextPtr = _declaredLvlObjectsListHead;
			--_declaredLvlObjectsListCount;
			_declaredLvlObjectsListHead = ptr;
			switch (ptr->spriteNum) {
			case 0:
			case 2:
				ptr->dataPtr = 0;
				break;
			case 3:
			case 7:
				if (ptr->dataPtr) {
					clearShootLvlObjectData(ptr);
				}
				break;
			}
			if (ptr->sssObject) {
				removeSound(ptr);
			}
			ptr->sssObject = 0;
			ptr->bitmapBits = 0;
		}
		ptr = next;
	}
	_lvlObjectsList0 = 0;
}

void Game::clearLvlObjectsList1() {
	if (!_lvlObjectsList1) {
		return;
	}
	for (int i = 0; i < kMaxMonsterObjects1; ++i) {
		resetMonsterObject1(&_monsterObjects1Table[i]);
	}
	for (int i = 0; i < kMaxMonsterObjects2; ++i) {
		resetMonsterObject2(&_monsterObjects2Table[i]);
	}
	LvlObject *ptr = _lvlObjectsList1;
	while (ptr) {
		LvlObject *next = ptr->nextPtr;
		if (ptr->type == 8) {
			_res->decLvlSpriteDataRefCounter(ptr);
			ptr->nextPtr = _declaredLvlObjectsListHead;
			--_declaredLvlObjectsListCount;
			_declaredLvlObjectsListHead = ptr;
			switch (ptr->spriteNum) {
			case 0:
			case 2:
				ptr->dataPtr = 0;
				break;
			case 3:
			case 7:
				if (ptr->dataPtr) {
					clearShootLvlObjectData(ptr);
				}
				break;
			}
			if (ptr->sssObject) {
				removeSound(ptr);
			}
			ptr->sssObject = 0;
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
			_res->decLvlSpriteDataRefCounter(ptr);
			ptr->nextPtr = _declaredLvlObjectsListHead;
			--_declaredLvlObjectsListCount;
			_declaredLvlObjectsListHead = ptr;
			switch (ptr->spriteNum) {
			case 0:
			case 2:
				ptr->dataPtr = 0;
				break;
			case 3:
			case 7:
				if (ptr->dataPtr) {
					clearShootLvlObjectData(ptr);
				}
				break;
			}
			if (ptr->sssObject) {
				removeSound(ptr);
			}
			ptr->sssObject = 0;
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
			_res->decLvlSpriteDataRefCounter(ptr);
			ptr->nextPtr = _declaredLvlObjectsListHead;
			--_declaredLvlObjectsListCount;
			_declaredLvlObjectsListHead = ptr;
			switch (ptr->spriteNum) {
			case 0:
			case 2:
				ptr->dataPtr = 0;
				break;
			case 3:
			case 7:
				if (ptr->dataPtr) {
					clearShootLvlObjectData(ptr);
				}
				break;
			}
			if (ptr->sssObject) {
				removeSound(ptr);
			}
			ptr->sssObject = 0;
			ptr->bitmapBits = 0;
		}
		ptr = next;
	}
	_lvlObjectsList3 = 0;
}

LvlObject *Game::addLvlObjectToList0(int num) {
	if (_res->_resLevelData0x2988PtrTable[num] != 0 && _declaredLvlObjectsListCount < kMaxLvlObjects) {
		assert(_declaredLvlObjectsListHead);
		LvlObject *ptr = _declaredLvlObjectsListHead;
		_declaredLvlObjectsListHead = _declaredLvlObjectsListHead->nextPtr;
		++_declaredLvlObjectsListCount;
		ptr->spriteNum = num;
		ptr->type = 8;
		_res->incLvlSpriteDataRefCounter(ptr);
		lvlObjectTypeCallback(ptr);
		ptr->currentSprite = 0;
		ptr->sssObject = 0;
		ptr->nextPtr = 0;
		ptr->bitmapBits = 0;
		ptr->nextPtr = _lvlObjectsList0;
		_lvlObjectsList0 = ptr;
		return ptr;
	}
	return 0;
}

LvlObject *Game::addLvlObjectToList1(int type, int num) {
	if ((type != 8 || _res->_resLevelData0x2988PtrTable[num] != 0) && _declaredLvlObjectsListCount < kMaxLvlObjects) {
		assert(_declaredLvlObjectsListHead);
		LvlObject *ptr = _declaredLvlObjectsListHead;
		_declaredLvlObjectsListHead = _declaredLvlObjectsListHead->nextPtr;
		++_declaredLvlObjectsListCount;
		ptr->spriteNum = num;
		ptr->type = type;
		if (type == 8) {
			_res->incLvlSpriteDataRefCounter(ptr);
			lvlObjectTypeCallback(ptr);
		}
		ptr->currentSprite = 0;
		ptr->sssObject = 0;
		ptr->nextPtr = 0;
		ptr->bitmapBits = 0;
		ptr->nextPtr = _lvlObjectsList1;
		_lvlObjectsList1 = ptr;
		return ptr;
	}
	return 0;
}

LvlObject *Game::addLvlObjectToList2(int num) {
	if (_res->_resLevelData0x2988PtrTable[num] != 0 && _declaredLvlObjectsListCount < kMaxLvlObjects) {
		assert(_declaredLvlObjectsListHead);
		LvlObject *ptr = _declaredLvlObjectsListHead;
		_declaredLvlObjectsListHead = _declaredLvlObjectsListHead->nextPtr;
		++_declaredLvlObjectsListCount;
		ptr->spriteNum = num;
		ptr->type = 8;
		_res->incLvlSpriteDataRefCounter(ptr);
		lvlObjectTypeCallback(ptr);
		ptr->currentSprite = 0;
		ptr->sssObject = 0;
		ptr->nextPtr = 0;
		ptr->bitmapBits = 0;
		ptr->nextPtr = _lvlObjectsList2;
		_lvlObjectsList2 = ptr;
		return ptr;
	}
	return 0;
}

LvlObject *Game::addLvlObjectToList3(int num) {
	if (_res->_resLevelData0x2988PtrTable[num] != 0 && _declaredLvlObjectsListCount < kMaxLvlObjects) {
		assert(_declaredLvlObjectsListHead);
		LvlObject *ptr = _declaredLvlObjectsListHead;
		_declaredLvlObjectsListHead = _declaredLvlObjectsListHead->nextPtr;
		++_declaredLvlObjectsListCount;
		ptr->spriteNum = num;
		ptr->type = 8;
		_res->incLvlSpriteDataRefCounter(ptr);
		lvlObjectTypeCallback(ptr);
		ptr->currentSprite = 0;
		ptr->sssObject = 0;
		ptr->nextPtr = 0;
		ptr->bitmapBits = 0;
		ptr->nextPtr = _lvlObjectsList3;
		_lvlObjectsList3 = ptr;
		ptr->callbackFuncPtr = &Game::lvlObjectList3Callback;
		return ptr;
	}
	return 0;
}

void Game::removeLvlObject(LvlObject *ptr) {
	AndyLvlObjectData *dataPtr = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
	LvlObject *o = dataPtr->shootLvlObject;
	if (o) {
		dataPtr->shootLvlObject = 0;
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
		_res->decLvlSpriteDataRefCounter(o);
		o->nextPtr = _declaredLvlObjectsListHead;
		_declaredLvlObjectsListHead = o;
		--_declaredLvlObjectsListCount;
	} else {
		switch (o->spriteNum) {
		case 0:
		case 2:
			o->dataPtr = 0;
			break;
		case 3:
		case 7:
			if (o->dataPtr) {
				clearShootLvlObjectData(o);
			}
			break;
		}
	}
	if (o->sssObject) {
		removeSound(o);
		o->sssObject = 0;
	}
	o->bitmapBits = 0;
}

void Game::setAndySprite(int num) {
	switch (num) {
	case 0: //  Andy with plasma cannon and helmet
		removeLvlObject(_andyObject);
		setLvlObjectSprite(_andyObject, 8, 0);
		_andyObject->anim = 48;
		break;
	case 2: // Andy
		destroyLvlObjectPlasmaExplosion(_andyObject);
		_plasmaCannonDirection = 0;
		_plasmaCannonLastIndex1 = 0;
		_plasmaCannonExplodeFlag = false;
		_plasmaCannonPointsMask = 0;
		_plasmaCannonObject = 0;
		setLvlObjectSprite(_andyObject, 8, 2);
		_andyObject->anim = 232;
		break;
	}
	_andyObject->frame = 0;
}

void Game::setupAndyLvlObject() {
	LvlObject *ptr = _andyObject;
	_fallingAndyFlag = false;
	_andyActionKeysFlags = 0;
	_hideAndyObjectSprite = false;
	const CheckpointData *dat = &_levelCheckpointData[_currentLevel][_levelCheckpoint];
	_plasmaCannonFlags = 0;
	_actionDirectionKeyMaskIndex = 0;
	_mstAndyCurrentScreenNum = ptr->screenNum;
	if (dat->spriteNum != ptr->spriteNum) {
		setAndySprite(dat->spriteNum);
	}
	ptr->childPtr = 0;
	ptr->xPos = dat->xPos;
	ptr->yPos = dat->yPos;
	ptr->flags2 = dat->flags2;
	ptr->anim = dat->anim;
	ptr->flags1 = ((ptr->flags2 >> 10) & 0x30) | (ptr->flags1 & ~0x30);
	ptr->screenNum = dat->screenNum;
	ptr->directionKeyMask = 0;
	ptr->actionKeyMask = 0;
	_currentScreen = dat->screenNum;
	_currentLeftScreen = _res->_screensGrid[_currentScreen * 4 + kPosLeftScreen];
	_currentRightScreen = _res->_screensGrid[_currentScreen * 4 + kPosRightScreen];
	ptr->frame = 0;
	setupLvlObjectBitmap(ptr);
	AndyLvlObjectData *dataPtr = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
	dataPtr->unk6 = 0;
	if (ptr->spriteNum == 2) {
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
				uint8_t *data = _res->_resLvlScreenBackgroundDataTable[num].backgroundAnimationTable[ptr->dataNum];
				assert(data);
				p->framesCount = READ_LE_UINT16(data); data += 2;
				ptr->currentSound = READ_LE_UINT16(data); data += 2;
				p->currentSpriteData = p->otherSpriteData = data;
				p->currentFrame = 0;
				p->firstSpriteData = READ_LE_UINT16(data + 4) + data + 4;
			}
			break;
		case 1: {
				uint8_t *data =  _res->_resLvlScreenBackgroundDataTable[num].backgroundSoundTable[ptr->dataNum];
				ptr->currentSound = READ_LE_UINT16(data); data += 2;
				ptr->dataPtr = data;
			}
			break;
		case 2:
			ptr->levelData0x2988 = _res->_resLvlScreenBackgroundDataTable[num].dataUnk5Table[ptr->dataNum];
			if (_currentLevel == kLvl_rock) {
				switch (ptr->objectUpdateType) {
				case 0:
				case 5:
					ptr->callbackFuncPtr = &Game::objectUpdate_rock_case0;
					break;
				case 1: // shadow_screen2
					ptr->callbackFuncPtr = &Game::objectUpdate_rock_case1;
					break;
				case 2: // shadow_screen2
					ptr->callbackFuncPtr = &Game::objectUpdate_rock_case2;
					break;
				case 3:
					ptr->callbackFuncPtr = &Game::objectUpdate_rock_case3;
					break;
				case 4:
					ptr->callbackFuncPtr = &Game::objectUpdate_rock_case4;
					break;
				default:
					warning("updateScreenHelper unimplemented for level %d, state %d", _currentLevel, ptr->objectUpdateType);
					break;
				}
			} else {
				// other levels use two callbacks
				switch (ptr->objectUpdateType) {
				case 0:
					ptr->callbackFuncPtr = &Game::objectUpdate_rock_case0;
					break;
				case 1:
					ptr->callbackFuncPtr = &Game::objectUpdate_rock_case3;
					break;
				default:
					warning("updateScreenHelper unimplemented for level %d, state %d", _currentLevel, ptr->objectUpdateType);
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
	_levelRestartCounter = 0;
	_fadePalette = false;
	memset(_fadePaletteBuffer, 0, sizeof(_fadePaletteBuffer));
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
	const int n = _levelCheckpointData[_currentLevel][_levelCheckpoint].screenNum;
	for (int i = 0; i < n; ++i) {
		_res->_screensState[i].s0 = *dat2++;
		_screenCounterTable[i] = *dat2++;
	}
	resetScreenMask();
	for (int i = n; i < _res->_lvlHdr.screensCount; ++i) {
		_level->setupLvlObjects(i);
	}
	resetCrackSprites();
}

void Game::restartLevel() {
	setupAndyLvlObject();
	clearLvlObjectsList2();
	clearLvlObjectsList3();
	if (!_mstLogicDisabled) {
		resetMstCode();
		startMstCode();
	} else {
		_mstFlags = 0;
	}
	if (_res->_sssHdr.infosDataCount != 0) {
		resetSound();
	}
	const int num = _levelCheckpointData[_currentLevel][_levelCheckpoint].screenNum;
	preloadLevelScreenData(num, 0xFF);
	_andyObject->levelData0x2988 = _res->_resLevelData0x2988PtrTable[_andyObject->spriteNum];
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
			if (_andyObject->spriteNum == 0) {
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
			ptr->xPos = xPosPrev + 256;
		} else if (xPos > 256) {
			ptr->screenNum = _res->_screensGrid[num * 4 + kPosRightScreen];
			ptr->xPos = xPosPrev - 256;
		}
		if (yPos < 0 && ptr->screenNum != 0xFF) {
			ptr->screenNum = _res->_screensGrid[ptr->screenNum * 4 + kPosTopScreen];
			ptr->yPos = yPosPrev + 192;
		} else if (yPos > 192) {
			assert(ptr->screenNum != 0xFF);
			ptr->screenNum = _res->_screensGrid[ptr->screenNum * 4 + kPosBottomScreen];
			ptr->yPos = yPosPrev - 192;
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
			AndyLvlObjectData *data = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
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

void Game::setAndyAnimationForArea(BoundingBox *box, int dx) {
	static uint8_t _prevAndyFlags0 = 0;
	BoundingBox objBox;
	objBox.x1 = _andyObject->xPos;
	objBox.x2 = _andyObject->xPos + _andyObject->posTable[3].x;
	objBox.y1 = _andyObject->yPos;
	objBox.y2 = _andyObject->yPos + _andyObject->posTable[3].y;
	const uint8_t _bl = _andyObject->flags0 & 0x1F;
	if (clipBoundingBox(box, &objBox)) {
		if ((_andyObject->actionKeyMask & 1) == 0) {
			_andyObject->actionKeyMask |= 8;
		}
		if (objBox.x2 >= box->x1 + dx && objBox.x2 <= box->x2 - dx) {
			uint8_t _al = 0;
			if (_currentLevel != kLvl_rock) {
				if (_bl == 1) {
					if (((_andyObject->flags0 >> 5) & 7) == 3) {
						_al = 0x80;
					}
				}
			} else {
				_andyObject->actionKeyMask &= ~1;
				_andyObject->directionKeyMask &= ~4;
				_andyObject->actionKeyMask |= 8;
			}
			if (_bl != 2) {
				if (_prevAndyFlags0 == 2) {
					_al = 0x80;
				}
				if (_al != 0 && _al > _actionDirectionKeyMaskIndex) {
					_actionDirectionKeyMaskIndex = _al;
					_actionDirectionKeyMaskCounter = 0;
				}
			}
		}
	}
	_prevAndyFlags0 = _bl;
}

void Game::setAndyLvlObjectPlasmaCannonKeyMask() {
	if (_actionDirectionKeyMaskCounter == 0) {
		switch (_actionDirectionKeyMaskIndex >> 4) {
		case 0:
			_actionDirectionKeyMaskCounter = 6;
			break;
		case 1:
		case 8:
		case 10:
			_actionDirectionKeyMaskCounter = 2;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			_actionDirectionKeyMaskCounter = 1;
			break;
		case 6:
		case 7:
		case 9:
			break;
		}
	}
	if (_actionDirectionKeyMaskIndex != 0) {
		if (_actionDirectionKeyMaskIndex == 0xA4 && !_fadePalette) { // game over
			_levelRestartCounter = 10;
			_plasmaCannonFlags |= 1;
		} else {
			if (_andyObject->spriteNum == 2 && _actionDirectionKeyMaskIndex >= 16) {
				removeLvlObject(_andyObject);
			}
			_andyActionKeysFlags = 0;
		}
		_andyObject->actionKeyMask = _actionDirectionKeyMaskTable[_actionDirectionKeyMaskIndex * 2];
		_andyObject->directionKeyMask = _actionDirectionKeyMaskTable[_actionDirectionKeyMaskIndex * 2 + 1];
	}
	--_actionDirectionKeyMaskCounter;
	if (_actionDirectionKeyMaskCounter == 0) {
		_actionDirectionKeyMaskIndex = 0;
	}
}

int Game::setAndySpecialAnimation(uint8_t mask) {
	if (mask > _actionDirectionKeyMaskIndex) {
		_actionDirectionKeyMaskIndex = mask;
		_actionDirectionKeyMaskCounter = 0;
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
		for (; count-- != 0; coords += 4) {
			if (_ecx->x1 > _ebp->x2 - coords[0] || _ecx->x2 < _ebp->x2 - coords[2]) {
				continue;
			}
			if (_ecx->y1 > _ebp->y1 + coords[3] || _ecx->y2 < _ebp->y1 + coords[1]) {
				continue;
			}
			break;
		}
		break;
	case 2:
		for (; count-- != 0; coords += 4) {
			if (_ecx->x1 > coords[2] + _ebp->x1 || _ecx->x2 < coords[0] + _ebp->x1) {
				continue;
			}
			if (_ecx->y1 > _ebp->y2 - coords[1] || _ecx->y2 < _ebp->y2 - coords[3]) {
				continue;
			}
			break;
		}
		break;
	case 3:
		for (; count-- != 0; coords += 4) {
			if (_ecx->x1 > _ebp->x2 - coords[0] || _ecx->x2 < _ebp->x2 - coords[2]) {
				continue;
			}
			if (_ecx->y1 > _ebp->y2 - coords[1] || _ecx->y2 < _ebp->y2 - coords[3]) {
				continue;
			}
			break;
		}
		break;
	default:
		for (; count-- != 0; coords += 4) {
			if (_ecx->x1 > coords[2] + _ebp->x1 || _ecx->x2 < coords[0] + _ebp->x1) {
				continue;
			}
			if (_ecx->y1 > coords[3] + _ebp->y1 || _ecx->y2 < coords[1] + _ebp->y1) {
				continue;
			}
			break;
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

int Game::clipLvlObjectsBoundingBoxHelper(LvlObject *o1, BoundingBox *box1, LvlObject *o2, BoundingBox *box2) {
	int ret = 0;
	const uint8_t *coords1 = _res->getLvlSpriteCoordPtr(o1->levelData0x2988, o1->currentSprite);
	const uint8_t *coords2 = _res->getLvlSpriteCoordPtr(o2->levelData0x2988, o2->currentSprite);
	if (clipBoundingBox(box1, box2)) {
		int count = *coords1++;
		if (count != 0) {
			const int direction = (o1->flags1 >> 4) & 3;
			switch (direction) {
			case 1:
				for (; count-- != 0 && !ret; coords1 += 4) {
					BoundingBox tmp;
					tmp.x1 = box1->x2 - coords1[2];
					tmp.x2 = box1->x2 - coords1[0];
					tmp.y1 = box1->y1 + coords1[1];
					tmp.y2 = box1->y2 + coords1[3];
					ret = updateBoundingBoxClippingOffset(&tmp, box2, coords2, (o2->flags1 >> 4) & 3);
				}
				break;
			case 2:
				for (; count-- != 0 && !ret; coords1 += 4) {
					BoundingBox tmp;
					tmp.x1 = box1->x1 + coords1[0];
					tmp.x2 = box1->x1 + coords1[2];
					tmp.y1 = box1->y2 - coords1[3];
					tmp.y2 = box1->y2 - coords1[1];
					ret = updateBoundingBoxClippingOffset(&tmp, box2, coords2, (o2->flags1 >> 4) & 3);
				}
				break;
			case 3:
				for (; count-- != 0 && !ret; coords1 += 4) {
					BoundingBox tmp;
					tmp.x1 = box1->x2 - coords1[2];
					tmp.x2 = box1->x2 - coords1[0];
					tmp.y1 = box1->y2 - coords1[3];
					tmp.y2 = box1->y2 - coords1[1];
					ret = updateBoundingBoxClippingOffset(&tmp, box2, coords2, (o2->flags1 >> 4) & 3);
				}
				break;
			default:
				for (; count-- != 0 && !ret; coords1 += 4) {
					BoundingBox tmp;
					tmp.x1 = box1->x1 + coords1[0];
					tmp.x2 = box1->x1 + coords1[2];
					tmp.y1 = box1->y1 + coords1[1];
					tmp.y2 = box1->y1 + coords1[3];
					ret = updateBoundingBoxClippingOffset(&tmp, box2, coords2, (o2->flags1 >> 4) & 3);
				}
				break;
			}
		}
	}
	return ret;
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
		obj2.x2 = obj2.x1 + ptr->width - 1;
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
			updateBoundingBoxClippingOffset(&obj1, &obj2, _res->getLvlSpriteCoordPtr(ptr->levelData0x2988, ptr->currentSprite), (ptr->flags1 >> 4) & 3);
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
		obj2.y2 += ptr->height - 1;
		return clipBoundingBox(&obj1, &obj2);
	case 48:
		obj1.x2 += o->width - 1;
		obj1.y2 += o->height - 1;
		obj2.x2 += ptr->width - 1;
		obj2.y2 += ptr->height - 1;
		if (clipBoundingBox(&obj1, &obj2)) {
			return updateBoundingBoxClippingOffset(&obj1, &obj2, _res->getLvlSpriteCoordPtr(ptr->levelData0x2988, ptr->currentSprite), (ptr->flags1 >> 4) & 3);
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
			return updateBoundingBoxClippingOffset(&obj2, &obj1, _res->getLvlSpriteCoordPtr(o->levelData0x2988, o->currentSprite), (o->flags1 >> 4) & 3);
		}
		break;
	case 3:
		obj1.x2 = obj1.x1 + o->width - 1;
		obj1.y2 += o->height - 1;
		obj2.x2 = obj2.x1 + ptr->width - 1;
		obj2.y2 += ptr->height - 1;
		if (clipBoundingBox(&obj2, &obj1)) {
			return updateBoundingBoxClippingOffset(&obj2, &obj1, _res->getLvlSpriteCoordPtr(o->levelData0x2988, o->currentSprite), (o->flags1 >> 4) & 3);
		}
		break;
	case 51:
		obj1.x2 = obj1.x1 + o->width - 1;
		obj1.y2 += o->height - 1;
		obj2.x2 = obj2.x1 + ptr->width - 1;
		obj2.y2 += ptr->height - 1;
		return clipLvlObjectsBoundingBoxHelper(o, &obj1, ptr, &obj2);
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
			return updateBoundingBoxClippingOffset(&obj2, &obj1, _res->getLvlSpriteCoordPtr(o->levelData0x2988, o->currentSprite), (o->flags1 >> 4) & 3);
		}
		break;
	default:
		warning("Unhandled clipLvlObjectsBoundingBox type %d (%d)", type, type - 17);
		break;
	}
	return 0;
}

int Game::clipLvlObjectsSmall(LvlObject *o1, LvlObject *o2, int type) {
	if (o1->width > 3 || o1->height > 3) {
		return clipLvlObjectsBoundingBox(o1, o2, type);
	}
	LvlObject tmpObject;
	memcpy(&tmpObject, o1, sizeof(LvlObject));
	tmpObject.type = 2;
	if (o1->frame == 0) {
		updateAndyObject(&tmpObject);
	}
	updateAndyObject(&tmpObject);
	return clipLvlObjectsBoundingBox(&tmpObject, o2, type);
}

int Game::restoreAndyCollidesLava() {
	int ret = 0;
	if (_lvlObjectsList1 && !_hideAndyObjectSprite && (_mstFlags & 0x80000000) == 0) {
		AndyLvlObjectData *data = (AndyLvlObjectData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		for (LvlObject *o = _lvlObjectsList1; o; o = o->nextPtr) {
			if (o->spriteNum != 21 || o->screenNum != _res->_currentScreenResourceNum) {
				continue;
			}
			BoundingBox b;
			b.x1 = o->xPos + o->posTable[0].x;
			b.x2 = o->xPos + o->posTable[1].x;
			b.y1 = o->yPos + o->posTable[0].y;
			b.y2 = o->yPos + o->posTable[1].y;
			if (!clipBoundingBox(&data->boundingBox, &b)) {
				ret = clipLvlObjectsBoundingBox(_andyObject, o, 68);
				if (ret) {
					setAndySpecialAnimation(0xA3);
				}
			}
		}
	}
	return ret;
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
		AndyLvlObjectData *data = (AndyLvlObjectData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		_andyObject->xPos += data->dxPos;
		_andyObject->yPos += data->dyPos;
	}
	const uint8_t flags = _andyObject->flags0 & 255;
	if ((flags & 0x1F) == 0xB) {
		if (_andyObject->spriteNum == 2) {
			removeLvlObject(_andyObject);
		}
		if ((flags & 0xE0) == 0x40) {
			setAndySpecialAnimation(0xA4);
		}
	}
	const int ret = updateLvlObjectScreen(_andyObject);
	if (ret > 0) {
		// changed screen
		return 1;
	} else if (ret == 0) {
		if (_currentLevel != kLvl_rock && _currentLevel != kLvl_lar2 && _currentLevel != kLvl_test) {
			return 0;
		}
		if (_plasmaExplosionObject) {
			_plasmaExplosionObject->screenNum = _andyObject->screenNum;
			lvlObjectType1Callback(_plasmaExplosionObject);
			if (_andyObject->actionKeyMask & 4) {
				addToSpriteList(_plasmaExplosionObject);
			}
		} else if (_andyObject->spriteNum == 0) {
			lvlObjectType1Init(_andyObject);
		}
		return 0;
	}
	// moved to invalid screen (-1), restart
	if ((_andyObject->flags0 & 0x1F) != 0xB) {
		playAndyFallingCutscene(0);
	}
	restartLevel();
	return 1;
}

void Game::drawPlasmaCannon() {
	int index = _plasmaCannonFirstIndex;
	int lastIndex = _plasmaCannonLastIndex1;
	if (lastIndex == 0) {
		lastIndex = _plasmaCannonLastIndex2;
	}
	int x1 = _plasmaCannonPosX[index];
	int y1 = _plasmaCannonPosY[index];
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
	_plasmaCannonExplodeFlag = false;
	_plasmaCannonObject = 0;
}

void Game::drawScreen() {
	memcpy(_video->_frontLayer, _video->_backgroundLayer, Video::W * Video::H);

	// redraw background animation sprites
	LvlBackgroundData *dat = &_res->_resLvlScreenBackgroundDataTable[_res->_currentScreenResourceNum];
	for (Sprite *spr = _spriteListPtrTable[0]; spr; spr = spr->nextPtr) {
		if ((spr->num & 0x1F) == 0) {
			_video->decodeSPR(spr->bitmapBits, _video->_backgroundLayer, spr->xPos, spr->yPos, 0);
		}
	}
	memset(_video->_shadowLayer, 0, Video::W * Video::H);
	for (int i = 1; i < 8; ++i) {
		for (Sprite *spr = _spriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x2000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_shadowLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
	for (int i = 1; i < 4; ++i) {
		for (Sprite *spr = _spriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x1000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_frontLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
	if (_andyObject->spriteNum == 0 && (_andyObject->flags2 & 0x1F) == 4) {
		if (_plasmaCannonFirstIndex < _plasmaCannonLastIndex2) {
			drawPlasmaCannon();
		}
	}
	for (int i = 4; i < 8; ++i) {
		for (Sprite *spr = _spriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x1000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_frontLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
	for (int i = 0; i < 24; ++i) {
		for (Sprite *spr = _spriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x2000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_shadowLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
	for (int i = 0; i < dat->shadowCount; ++i) {
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
		for (Sprite *spr = _spriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x1000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_frontLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
	if (_andyObject->spriteNum == 0 && (_andyObject->flags2 & 0x1F) == 0xC) {
		if (_plasmaCannonFirstIndex < _plasmaCannonLastIndex2) {
			drawPlasmaCannon();
		}
	}
	for (int i = 12; i <= 24; ++i) {
		for (Sprite *spr = _spriteListPtrTable[i]; spr; spr = spr->nextPtr) {
			if ((spr->num & 0x1000) != 0) {
				_video->decodeSPR(spr->bitmapBits, _video->_frontLayer, spr->xPos, spr->yPos, (spr->num >> 0xE) & 3);
			}
		}
	}
}

void Game::mainLoop(int level, int checkpoint, bool levelChanged) {
	_video->_font = _res->_fontBuffer;
	assert(level < kLvl_test);
	_currentLevel = level;
	_levelCheckpoint = checkpoint;
	_mix._lock(1);
	_res->loadLevelData(_levels[_currentLevel]);
	_mix._lock(0);
	_mstAndyCurrentScreenNum = -1;
	_rnd.initTable();
	initMstCode();
//	res_initIO();
	preloadLevelScreenData(_levelCheckpointData[_currentLevel][_levelCheckpoint].screenNum, 0xFF);
	memset(_screenCounterTable, 0, sizeof(_screenCounterTable));
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
	if (!_paf->_skipCutscenes && _levelCheckpoint == 0 && !levelChanged) {
		const uint8_t num = _cutscenes[_currentLevel];
		_paf->preload(num);
		_paf->play(num);
		_paf->unload(num);
	}
	if (_res->_sssHdr.infosDataCount != 0) {
		resetSound();
	}
	_quit = false;
	resetShootLvlObjectDataTable();
	callLevel_initialize();
	setupAndyLvlObject();
	clearLvlObjectsList2();
	clearLvlObjectsList3();
	if (!_mstLogicDisabled) {
		resetMstCode();
		startMstCode();
	} else {
		_mstFlags = 0;
	}
	if (_res->_sssHdr.infosDataCount != 0) {
		resetSound();
	}
	const int num = _levelCheckpointData[_currentLevel][_levelCheckpoint].screenNum;
	preloadLevelScreenData(num, 0xFF);
	_andyObject->levelData0x2988 = _res->_resLevelData0x2988PtrTable[_andyObject->spriteNum];
	resetScreen();
	if (_andyObject->screenNum != num) {
		preloadLevelScreenData(_andyObject->screenNum, 0xFF);
	}
	updateScreen(_andyObject->screenNum);
	do {
		int frameTimeStamp = _system->getTimeStamp() + kFrameTimeStamp;
		levelMainLoop();
		int diff = frameTimeStamp - _system->getTimeStamp();
		if (diff < 10) {
			diff = 10;
		}
		_system->sleep(diff);
	} while (!_system->inp.quit && !_quit);
	_animBackgroundDataCount = 0;
	callLevel_terminate();
}

void Game::mixAudio(int16_t *buf, int len) {

	if (_snd_muted) {
		return;
	}

	static const int kStereoSamples = 3528; // stereo

	static int count = 0;

	static int16_t buffer[kStereoSamples];
	static int bufferOffset = 0;
	static int bufferSize = 0;

	// flush samples from previous run
	if (bufferSize > 0) {
		const int count = len < bufferSize ? len : bufferSize;
		memcpy(buf, buffer + bufferOffset, count * sizeof(int16_t));
		buf += count;
		len -= count;
		bufferOffset += count;
		bufferSize -= count;
	}

	while (len > 0) {
		// this enqueues 1764*2 bytes for mono samples and 3528*2 bytes for stereo
		_mix._mixingQueueSize = 0;
		// 17640 + 17640 * 25 / 100 == 22050 (1.25x)
		if (count == 4) {
			mixSoundObjects17640(true);
			count = 0;
		} else {
			mixSoundObjects17640(false);
			++count;
		}

		if (len >= kStereoSamples) {
			_mix.mix(buf, kStereoSamples);
			buf += kStereoSamples;
			len -= kStereoSamples;
		} else {
			memset(buffer, 0, sizeof(buffer));
			_mix.mix(buffer, kStereoSamples);
			memcpy(buf, buffer, len * sizeof(int16_t));
			bufferOffset = len;
			bufferSize = kStereoSamples - len;
			break;
		}
	}
}

void Game::updateLvlObjectList(LvlObject **list) {
	LvlObject *ptr = *list;
	while (ptr) {
		LvlObject *next = ptr->nextPtr; // get 'next' as callback can modify linked list (eg. remove)
		if (ptr->callbackFuncPtr == &Game::lvlObjectList3Callback && list != &_lvlObjectsList3) {
			warning("lvlObject %p has callbackType3 and not in _lvlObjectsList3", ptr);
			ptr = next;
			continue;
		}
		if (ptr->callbackFuncPtr) {
			(this->*(ptr->callbackFuncPtr))(ptr);
		}
		if (ptr->bitmapBits && list != &_lvlObjectsList3) {
			addToSpriteList(ptr);
		}
		ptr = next;
	}
}

void Game::updateLvlObjectLists() {
	updateLvlObjectList(&_lvlObjectsList0);
	updateLvlObjectList(&_lvlObjectsList1);
	updateLvlObjectList(&_lvlObjectsList2);
	updateLvlObjectList(&_lvlObjectsList3);
}

LvlObject *Game::updateAnimatedLvlObjectType0(LvlObject *ptr) {
	AnimBackgroundData *_esi = (AnimBackgroundData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAnimBackgroundData);
	const uint8_t *_edi = _esi->currentSpriteData + 2;
	if (_res->_currentScreenResourceNum == ptr->screenNum) {
		if (ptr->currentSound != 0xFFFF) {
			playSound(ptr->currentSound, ptr, 0, 3);
			ptr->currentSound = 0xFFFF;
		}
		Sprite *spr = _spritesListNextPtr;
		if (spr && READ_LE_UINT16(_edi + 2) > 8) {
			spr->xPos = _edi[0];
			spr->yPos = _edi[1];
			spr->bitmapBits = _edi + 2;
			spr->num = ptr->flags2;
			const int index = spr->num & 0x1F;
			_spritesListNextPtr = spr->nextPtr;
			spr->nextPtr = _spriteListPtrTable[index];
			_spriteListPtrTable[index] = spr;
		}
	}
	int16_t soundNum = -1;
	const uint8_t *_eax = READ_LE_UINT16(_edi + 2) + _edi + 2; // nextSpriteData
	switch (ptr->objectUpdateType - 1) {
	case 6:
		_esi->currentSpriteData = _esi->firstSpriteData;
		if (_esi->currentFrame == 0) {
			_esi->currentFrame = 1;
			soundNum = READ_LE_UINT16(_esi->firstSpriteData);
		}
		ptr->objectUpdateType = 4;
		break;
	case 5:
		_esi->currentFrame = 0;
		_esi->currentSpriteData = _esi->otherSpriteData;
		ptr->objectUpdateType = 1;
		break;
	case 3:
		++_esi->currentFrame;
		if (_esi->currentFrame < _esi->framesCount) {
			_esi->currentSpriteData = _eax;
			soundNum = READ_LE_UINT16(_eax);
		} else {
			_esi->currentFrame = 0;
			_esi->currentSpriteData = _esi->otherSpriteData;
			ptr->objectUpdateType = 1;
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
			ptr->objectUpdateType = 1;
			soundNum = READ_LE_UINT16(_esi->currentSpriteData);
		}
		break;
	case 2:
		while (_esi->currentFrame < _esi->framesCount - 2) {
			++_esi->currentFrame;
			_esi->currentSpriteData = _eax;
			_eax += 2;
			const uint16_t _di = READ_LE_UINT16(_eax + 2);
			_eax += _di + 2;
		}
		_eax = _esi->currentSpriteData + 2; // _esi
		if (_res->_currentScreenResourceNum == ptr->screenNum) {
			Sprite *spr = _spritesListNextPtr;
			if (spr && READ_LE_UINT16(_eax + 2) > 8) {
				spr->bitmapBits = _eax + 2;
				spr->xPos = _eax[0];
				spr->yPos = _eax[1];
				_spritesListNextPtr = spr->nextPtr;
				spr->num = ptr->flags2;
				const int index = spr->num & 0x1F;
				spr->nextPtr = _spriteListPtrTable[index];
				_spriteListPtrTable[index] = spr;
			}
		}
		ptr->objectUpdateType = 1;
		return ptr->nextPtr;
	case 1:
		++_esi->currentFrame;
		if (_esi->currentFrame < _esi->framesCount - 1) {
			_esi->currentSpriteData = _eax;
			soundNum = READ_LE_UINT16(_esi->currentSpriteData);
		} else {
			if (_esi->currentFrame > _esi->framesCount) {
				_esi->currentFrame = _esi->framesCount;
			}
			ptr->objectUpdateType = 1;
			return ptr->nextPtr;
		}
		break;
	case 0:
		return ptr->nextPtr;
	default:
		soundNum = READ_LE_UINT16(_esi->currentSpriteData);
		if (ptr->hitCount == 0) {
			++_esi->currentFrame;
			if (_esi->currentFrame >= _esi->framesCount) {
				_esi->currentSpriteData = _esi->firstSpriteData;
				_esi->currentFrame = 1;
			} else {
				_esi->currentSpriteData = _eax;
			}
		} else {
			--ptr->hitCount;
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
			if (ptr->currentSound != 0xFFFF) {
				playSound(ptr->currentSound, 0, 0, 3);
				ptr->currentSound = 0xFFFF;
			}
			uint8_t *data = (uint8_t *)getLvlObjectDataPtr(ptr, kObjectDataTypeLvlBackgroundSound);
			Sprite *spr = _spritesListNextPtr;
			if (spr && READ_LE_UINT16(data + 2) > 8) {
				spr->bitmapBits = data + 2;
				spr->xPos = data[0];
				spr->yPos = data[1];
				_spritesListNextPtr = spr->nextPtr;
				spr->num = ptr->flags2;
				const int index = spr->num & 0x1F;
				spr->nextPtr = _spriteListPtrTable[index];
				_spriteListPtrTable[index] = spr;
			}
		}
	}
	return ptr->nextPtr;
}

LvlObject *Game::updateAnimatedLvlObjectType2(LvlObject *ptr) {
	LvlObject *next, *o;

	o = next = ptr->nextPtr;
	if ((ptr->spriteNum > 15 && ptr->dataPtr == 0) || ptr->levelData0x2988 == 0) {
		if (ptr->childPtr) {
			o = ptr->nextPtr;
		}
		return o;
	}
	const int num = ptr->screenNum;
	if (_currentScreen != num && _currentRightScreen != num && _currentLeftScreen != num) {
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
		const int index = (15 < ptr->spriteNum) ? 5 : 7;
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
		Sprite *spr = _spritesListNextPtr;
		if (spr && READ_LE_UINT16(_edi) > 8) {
			spr->yPos = ptr->yPos;
			spr->xPos = ptr->xPos;
			spr->bitmapBits = _edi;
			int index = spr->num = _ecx;
			index &= 0x1F;
			_spritesListNextPtr = spr->nextPtr;
			spr->nextPtr = _spriteListPtrTable[index];
			_spriteListPtrTable[index] = spr;
		}
	}
	if (ptr->spriteNum <= 15 || ptr->dataPtr == 0) {
		if (ptr->currentSound != 0xFFFF) {
			playSound(ptr->currentSound, ptr, 0, 3);
		}
		return o;
	}
	int a, c;
	if (ptr->dataPtr >= &_monsterObjects1Table[0] && ptr->dataPtr < &_monsterObjects1Table[kMaxMonsterObjects1]) {
		MonsterObject1 *m = (MonsterObject1 *)ptr->dataPtr;
		if (m->flagsA6 & 2) {
			assert(ptr == m->o16);
			ptr->actionKeyMask = _mstCurrentActionKeyMask;
			ptr->directionKeyMask = _andyObject->directionKeyMask;
		}
		a = m->monster1Index;
		c = 1;
	} else {
		assert(ptr->dataPtr >= &_monsterObjects2Table[0] && ptr->dataPtr < &_monsterObjects2Table[kMaxMonsterObjects2]);
		MonsterObject1 *m = ((MonsterObject2 *)ptr->dataPtr)->monster1;
		if (m) {
			a = m->monster1Index;
			c = 2;
		} else {
			a = 4;
			c = 0;
		}
	}
	if (ptr->currentSound != 0xFFFF) {
		playSound(ptr->currentSound, ptr, c, a);
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
		if (_plasmaCannonExplodeFlag) {
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
	ptr->flags2 = merge_bits(ptr->flags2, _andyObject->flags2, 0x18);
	ptr->flags2 = merge_bits(ptr->flags2, _andyObject->flags2 + 1, 7);
	addToSpriteList(ptr);
}

void Game::resetPlasmaCannonState() {
	_plasmaCannonDirection = 0;
	_plasmaCannonPrevDirection = 0;
	_plasmaCannonPointsSetupCounter = 0;
	_plasmaCannonLastIndex1 = 0;
	_plasmaCannonExplodeFlag = false;
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
			assert(_specialAnimLvlObject);
			_mstOriginPosX += _specialAnimLvlObject->posTable[6].x + _specialAnimLvlObject->xPos;
			_mstOriginPosY += _specialAnimLvlObject->posTable[6].y + _specialAnimLvlObject->yPos;
		}
		ptr->childPtr = 0;
		break;
	case 7: // replace Andy sprite with a custom animation
		_hideAndyObjectSprite = true;
		if (_actionDirectionKeyMaskIndex == 0x71) {
			assert(_specialAnimLvlObject);
			_mstOriginPosX += _specialAnimLvlObject->posTable[6].x + _specialAnimLvlObject->xPos;
			_mstOriginPosY += _specialAnimLvlObject->posTable[6].y + _specialAnimLvlObject->yPos;
			ptr->childPtr = _specialAnimLvlObject;
			ptr->screenNum = _specialAnimLvlObject->screenNum;
		} else {
			ptr->childPtr = 0;
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
		ptr->flags1 = merge_bits(ptr->flags1, _specialAnimMask, 0x30);
		setupLvlObjectBitmap(ptr);
		setLvlObjectPosRelativeToPoint(ptr, 3, _mstOriginPosX, _mstOriginPosY);
	}
	_andyActionKeysFlags = 0;
	if (ptr->spriteNum == 2) {
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

void Game::levelMainLoop() {
	memset(_spriteListPtrTable, 0, sizeof(_spriteListPtrTable));
	_spritesListNextPtr = &_spritesTable[0];
	for (int i = 0; i < kMaxSprites - 1; ++i) {
		_spritesTable[i].nextPtr = &_spritesTable[i + 1];
	}
	_spritesTable[kMaxSprites - 1].nextPtr = 0;
	_directionKeyMask = 0;
	_actionKeyMask = 0;
	updateInput();
	_andyObject->directionKeyMask = _directionKeyMask;
	_andyObject->actionKeyMask = _actionKeyMask;
	_video->fillBackBuffer();
	if (_andyObject->screenNum != _res->_currentScreenResourceNum) {
		preloadLevelScreenData(_andyObject->screenNum, _res->_currentScreenResourceNum);
		updateScreen(_andyObject->screenNum);
	} else if (_fadePalette && _levelRestartCounter == 0) {
		restartLevel();
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
//		_time_counter1 -= _time_counter2;
		return;
	}
	executeMstCode();
	updateLvlObjectLists();
	callLevel_tick();
	updateAndyMonsterObjects();
	if (!_hideAndyObjectSprite) {
		addToSpriteList(_andyObject);
	}
	((AndyLvlObjectData *)_andyObject->dataPtr)->dxPos = 0;
	((AndyLvlObjectData *)_andyObject->dataPtr)->dyPos = 0;
	updateAnimatedLvlObjectsLeftRightCurrentScreens();
	if (_currentLevel == kLvl_rock || _currentLevel == kLvl_lar2 || _currentLevel == kLvl_test) {
		if (_andyObject->spriteNum == 0 && _plasmaExplosionObject && _plasmaExplosionObject->nextPtr != 0) {
			updatePlasmaCannonExplosionLvlObject(_plasmaExplosionObject->nextPtr);
		}
	}
	if (_res->_sssHdr.infosDataCount != 0) {
		/* original code appears to have a dedicated thread for sound, that main thread/loop signals */
	}
	if (_video->_paletteNeedRefresh) {
		_video->_paletteNeedRefresh = false;
		_video->refreshGamePalette(_video->_displayPaletteBuffer);
		_system->copyRectWidescreen(Video::W, Video::H, _video->_backgroundLayer, _video->_palette);
	}
	drawScreen();
	if (_system->inp.screenshot) {
		_system->inp.screenshot = false;
		captureScreenshot();
	}
	if (0) {
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "P%d S%02d %d R%d", _currentLevel, _andyObject->screenNum, _res->_screensState[_andyObject->screenNum].s0, _levelCheckpoint);
		_video->drawString(buffer, (Video::W - strlen(buffer) * 8) / 2, 8, _video->findWhiteColor(), _video->_frontLayer);
	}
	if (_shakeScreenDuration != 0 || _levelRestartCounter != 0 || _video->_displayShadowLayer) {
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
			_system->inp.quit = true;
			return;
		}
	} else {
		// displayHintScreen(1, 0);
		_video->updateScreen();
	}
}

void Game::callLevel_postScreenUpdate(int num) {
	_level->postScreenUpdate(num);
}

void Game::callLevel_preScreenUpdate(int num) {
	_level->preScreenUpdate(num);
}

void Game::callLevel_initialize() {
	switch (_currentLevel) {
	case 0:
		_level = Level_rock_create();
		break;
	case 1:
		_level = Level_fort_create();
		break;
	case 2:
		_level = Level_pwr1_create();
		break;
	case 3:
		_level = Level_isld_create();
		break;
	case 4:
		_level = Level_lava_create();
		break;
	case 5:
		_level = Level_pwr2_create();
		break;
	case 6:
		_level = Level_lar1_create();
		break;
	case 7:
		_level = Level_lar2_create();
		break;
	case 8:
		_level = Level_dark_create();
		break;
	}
	_level->setPointers(this, _andyObject, _paf, _res, _video);
	_level->initialize();
}

void Game::callLevel_tick() {
	_level->tick();
}

void Game::callLevel_terminate() {
	_level->terminate();
	delete _level;
	_level = 0;
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
	muteSound();
	if (num == -1) {
		num = _res->_datHdr.yesNoQuitImage; // 'Yes'
		_res->loadDatHintImage(num + 1, _video->_shadowLayer, _video->_palette); // 'No'
		confirmQuit = true;
	}
	_res->loadDatHintImage(num, _video->_frontLayer, _video->_palette);
	_system->setPalette(_video->_palette, 256, 6);
	_system->copyRect(0, 0, Video::W, Video::H, _video->_frontLayer, 256);
	_system->updateScreen(false);
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
				_system->copyRect(0, 0, Video::W, Video::H, quitBuffers[quit], 256);
				_system->updateScreen(false);
			}
		}
		_system->sleep(30);
	} while (!_system->inp.quit && !_system->inp.keyReleased(SYS_INP_JUMP));
	unmuteSound();
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
			do {
				prev = current;
				current = current->nextPtr;
			} while (current && current != ptr);
			assert(prev);
			prev->nextPtr = current->nextPtr;
		}
	}
}

void *Game::getLvlObjectDataPtr(LvlObject *o, int type) const {
	switch (type) {
	case kObjectDataTypeAndy:
		assert(o == _andyObject);
		assert(o->dataPtr == &_andyObjectScreenData);
		break;
	case kObjectDataTypeAnimBackgroundData:
		assert(o->dataPtr >= &_animBackgroundDataTable[0] && o->dataPtr < &_animBackgroundDataTable[kMaxBackgroundAnims]);
		break;
	case kObjectDataTypeShoot:
		assert(o->dataPtr >= &_shootLvlObjectDataTable[0] && o->dataPtr < &_shootLvlObjectDataTable[kMaxShootLvlObjectData]);
		break;
	case kObjectDataTypeLvlBackgroundSound:
		assert(o->type == 1);
		// dataPtr is _res->_resLvlScreenBackgroundDataTable[num].backgroundSoundTable + 2
		assert(o->dataPtr);
		break;
	case kObjectDataTypeMonster1:
		assert(o->dataPtr >= &_monsterObjects1Table[0] && o->dataPtr < &_monsterObjects1Table[kMaxMonsterObjects1]);
		break;
	case kObjectDataTypeMonster2:
		assert(o->dataPtr >= &_monsterObjects2Table[0] && o->dataPtr < &_monsterObjects2Table[kMaxMonsterObjects2]);
		break;
	}
	return o->dataPtr;
}

// Andy
void Game::lvlObjectType0Init(LvlObject *ptr) {
	uint8_t num = ptr->spriteNum;
	if (_currentLevel == kLvl_rock && _levelCheckpoint >= 5) {
		num = 2; // sprite without 'plasma cannon'
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

// Plasma cannon explosion
void Game::lvlObjectType1Init(LvlObject *ptr) {
	AndyLvlObjectData *dataPtr = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
	if (dataPtr->shootLvlObject) {
		return;
	}
	LvlObject *o = declareLvlObject(8, 1);
	assert(o);
	o->xPos = ptr->xPos;
	o->yPos = ptr->yPos;
	o->anim = 13;
	o->frame = 0;
	o->screenNum = ptr->screenNum;
	o->flags1 = merge_bits(o->flags1, ptr->flags1, 0x30); // _esi->flags1 ^= (_esi->flags1 ^ ptr->flags1) & 0x30;
	o->flags2 = ptr->flags2 & ~0x2000;
	setupLvlObjectBitmap(o);
	prependLvlObjectToList(&_plasmaExplosionObject, o);

	o = declareLvlObject(8, 1);
	assert(o);
	dataPtr->shootLvlObject = o;
	o->xPos = ptr->xPos;
	o->yPos = ptr->yPos;
	o->anim = 5;
	o->frame = 0;
	o->screenNum = ptr->screenNum;
	o->flags1 = merge_bits(o->flags1, ptr->flags1, 0x30); // _esi->flags1 ^= (_esi->flags1 ^ ptr->flags1) & 0x30;
	o->flags2 = ptr->flags2 & ~0x2000;
	setupLvlObjectBitmap(o);
	prependLvlObjectToList(&_plasmaExplosionObject, o);
}

void Game::lvlObjectTypeInit(LvlObject *o) {
	switch (o->spriteNum) {
	case 0: // Andy with plasma cannon and helmet
	case 2: // Andy
		lvlObjectType0Init(o);
		break;
	case 1: // plasma cannon explosion
		lvlObjectType1Init(o);
		break;
	default:
		error("lvlObjectTypeInit unhandled case %d", o->spriteNum);
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

	if (_currentLevel == kLvl_dark && (_bl & 4) != 0) {
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
	if (_andyObject->spriteNum == 2 && (_bl & 5) == 5) {
		AndyLvlObjectData *data = (AndyLvlObjectData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
		LvlObject *o = data->shootLvlObject;
		if (o) {
			ShootLvlObjectData *dataUnk1 = (ShootLvlObjectData *)getLvlObjectDataPtr(o, kObjectDataTypeShoot);
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
	const uint32_t offset = screenMaskOffset(x, y);
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

void Game::lvlObjectType0CallbackBreathBubbles(LvlObject *ptr) {

	AndyLvlObjectData *_edi = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);

	BoundingBox b;
	b.x1 = b.x2 = ptr->xPos + ptr->posTable[7].x;
	b.y1 = b.y2 = ptr->yPos + ptr->posTable[7].y;

	const int num = _pwr1_screenTransformLut[_res->_currentScreenResourceNum * 2 + 1];
	if (!clipBoundingBox(&_screenTransformRects[num], &b)) {
		++_edi->unk6; // apnea counter/time
	} else {
		_edi->unk6 = 0;
	}
	b.y1 -= 24;
	if (_edi->unk2 == 0 && !clipBoundingBox(&_screenTransformRects[num], &b)) {

		if (addLvlObjectToList3(4)) {
			_lvlObjectsList3->xPos = b.x1;
			_lvlObjectsList3->yPos = b.y1 + 24;
			_lvlObjectsList3->screenNum = ptr->screenNum;
			_lvlObjectsList3->anim = 0;
			_lvlObjectsList3->frame = 0;
			_lvlObjectsList3->flags2 = ptr->flags2 + 1;
			_lvlObjectsList3->flags0 = (_lvlObjectsList3->flags0 & ~0x19) | 6;
			_lvlObjectsList3->flags1 &= ~0x20;
		}

		int currentApneaLevel = _edi->unk3;
		static const int16_t _pwr1_apneaDuration[] = { 625, 937, 1094, 1250 };
		int newApneaLevel = 0;
		while (newApneaLevel < 4 && _edi->unk6 >= _pwr1_apneaDuration[newApneaLevel]) {
			++newApneaLevel;
		}
		_edi->unk3 = newApneaLevel;
		static const uint8_t _pwr1_apneaBubble[] = { 22, 20, 18, 16, 14 };
		_edi->unk2 = _pwr1_apneaBubble[newApneaLevel];
		// play Andy animation when apnea level changes
		switch (currentApneaLevel) {
		case 2:
			if (newApneaLevel == 3) {
				if (_actionDirectionKeyMaskIndex < 1) {
					_actionDirectionKeyMaskIndex = 1;
					_actionDirectionKeyMaskCounter = 0;
				}
			}
			break;
		case 3:
			if (newApneaLevel == 4) {
				if (_actionDirectionKeyMaskIndex < 2) {
					_actionDirectionKeyMaskIndex = 2;
					_actionDirectionKeyMaskCounter = 0;
				}
			}
			break;
		case 4:
			if (_edi->unk6 >= 1250) {
				if (_actionDirectionKeyMaskIndex < 160) {
					_actionDirectionKeyMaskIndex = 160;
					_actionDirectionKeyMaskCounter = 0;
				}
			}
			break;
		}
// 4097E1
		assert(_lvlObjectsList3);
		switch (newApneaLevel) {
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

static const uint8_t byte_43E680[] = {
	0xFF, 0x00, 0x07, 0x00, 0x0F, 0x00, 0x17, 0x00, 0x08, 0x08, 0x00, 0x00, 0xF8, 0xF8, 0xF0, 0xF0,
	0x08, 0xF8, 0x00, 0xFF, 0xF8, 0x07, 0xF0, 0x0F, 0xFF, 0x08, 0x07, 0x00, 0x0F, 0xF8, 0x17, 0xF0,
	0xFF, 0xF8, 0x07, 0xFF, 0x0F, 0x07, 0x17, 0x0F, 0x08, 0x00, 0x00, 0x00, 0xF8, 0x00, 0xF0, 0x00,
	0x00, 0x08, 0x00, 0x00, 0x00, 0xF8, 0x00, 0xF0, 0x00, 0xF8, 0x00, 0xFF, 0x00, 0x07, 0x00, 0x0F
};

static const uint8_t byte_43E6C0[] = {
	0x00, 0x00, 0x08, 0x00, 0x00, 0x08, 0x08, 0x08, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x08, 0xF8, 0x08,
	0x00, 0x00, 0x08, 0x00, 0x00, 0xF8, 0x08, 0xF8, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xF8, 0xF8, 0xF8
};

// anim
static const uint8_t byte_43E700[] = {
	0x04, 0x14, 0x03, 0x16, 0x03, 0x16, 0x03, 0x16, 0x03, 0x16, 0x04, 0x14, 0x02, 0x18, 0x02, 0x18
};

// anim
static const uint8_t byte_43E710[] = {
	0x04, 0x08, 0x03, 0x08, 0x03, 0x08, 0x03, 0x08, 0x03, 0x08, 0x04, 0x08, 0x02, 0x08, 0x02, 0x08
};

// anim sprite 7
static const uint8_t _byte_43E720[16] = {
	0x06, 0x0B, 0x05, 0x09, 0x05, 0x09, 0x05, 0x09, 0x05, 0x09, 0x06, 0x0B, 0x04, 0x07, 0x04, 0x07
};

// anim sprite 7
static const uint8_t _byte_43E750[16] = {
	0x06, 0x0E, 0x05, 0x0E, 0x05, 0x0E, 0x05, 0x0E, 0x05, 0x0E, 0x06, 0x0E, 0x04, 0x0E, 0x04, 0x0E
};

// sp mask offsets1
static const int dword_43E770[] = {
	0, 1, 1, 1, 0, -513, -513, -513, 0, 511, 511, 511, 0, -511, -511, -511, 0, 513, 513, 513, 0, -1, -1, -1, 0, -512, -512, -512, 0, 512, 512, 512
};

// sp mask offsets2
static const int dword_43E7F0[] = {
	0, 1, 512, -1, 0, -1, 512, 1, 0, 1, -512, -1, 0, -1, -512, 1
};

void Game::setupSpecialPowers(LvlObject *ptr) {
	assert(ptr == _andyObject);
	AndyLvlObjectData *_edi = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
	LvlObject *_esi = _edi->shootLvlObject;
	const uint8_t pos = ptr->flags0 & 0x1F;
	uint8_t var1 = (ptr->flags0 >> 5) & 7;
	if (pos == 4) { // release
// 40DB4C
		assert(_esi->dataPtr);
		ShootLvlObjectData *_eax = (ShootLvlObjectData *)getLvlObjectDataPtr(_esi, kObjectDataTypeShoot);
		_esi->callbackFuncPtr = &Game::lvlObjectSpecialPowersCallback;
		uint8_t _cl = (ptr->flags1 >> 4) & 3;
		if (_eax->unk0 == 4) {
			_eax->counter = 33;
			switch (var1) {
			case 0:
				_eax->state = ((_cl & 1) != 0) ? 5 : 0;
				break;
			case 1:
				_eax->state = ((_cl & 1) != 0) ? 3 : 1;
				break;
			case 2:
				_eax->state = ((_cl & 1) != 0) ? 4 : 2;
				break;
			case 3:
				_eax->state = ((_cl & 1) != 0) ? 1 : 3;
				break;
			case 4:
				_eax->state = ((_cl & 1) != 0) ? 2 : 4;
				break;
			case 5:
				_eax->state = ((_cl & 1) != 0) ? 0 : 5;
				break;
			case 6:
				_eax->state = 6;
				break;
			case 7:
				_eax->state = 7;
				break;
			}
// 40DBCE
			_eax->dxPos = (int8_t)_byte_43E670[_eax->state * 2];
			_eax->dyPos = (int8_t)_byte_43E670[_eax->state * 2 + 1];
			_esi->anim = 10;
		} else {
// 40DBF4
			_eax->counter = 17;
			switch (var1) {
			case 0:
				_esi->anim = 13;
				_eax->state = ((_cl & 1) != 0) ? 5 : 0;
				break;
			case 1:
				_esi->anim = 12;
				_eax->state = ((_cl & 1) != 0) ? 3 : 1;
				_cl ^= 1;
				break;
			case 2:
				_esi->anim = 12;
				_eax->state = ((_cl & 1) != 0) ? 4 : 2;
				_cl ^= 3;
				break;
			case 3:
				_esi->anim = 12;
				_eax->state = ((_cl & 1) != 0) ? 1 : 3;
				break;
			case 4:
				_esi->anim = 12;
				_eax->state = ((_cl & 1) != 0) ? 2 : 4;
				_cl ^= 2;
				break;
			case 5:
				_esi->anim = 13;
				_eax->state = ((_cl & 1) != 0) ? 0 : 5;
				_cl ^= 1;
				break;
			case 6:
				_esi->anim = 11;
				_eax->state = 6;
				break;
			case 7:
				_esi->anim = 11;
				_eax->state = 7;
				_cl ^= 2;
				break;
			}
// 40DCCE
			_eax->dxPos = (int8_t)_byte_43E660[_eax->state * 2];
			_eax->dyPos = (int8_t)_byte_43E660[_eax->state * 2 + 1];
		}
// 40DCE9
		_esi->frame = 0;
		_esi->flags1 = (_esi->flags1 & ~0x30) | ((_cl & 3) << 4);
		setupLvlObjectBitmap(_esi);
		_esi->screenNum = ptr->screenNum;
		setLvlObjectPosRelativeToObject(_esi, 7, ptr, 6);
		if (_currentLevel == kLvl_isld) {
			AndyLvlObjectData *_ecx = (AndyLvlObjectData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			_esi->xPos += _ecx->dxPos;
		}
		_edi->shootLvlObject = 0;
	} else if (pos == 7) { // hold
// 40DD4B
		switch (var1) {
		case 0:
			if (!_esi) {
				if (!_edi->shootLvlObject) {
					LvlObject *_edx = declareLvlObject(8, 3);
					_edi->shootLvlObject = _edx;
					_edx->dataPtr = _shootLvlObjectDataList;
					if (_shootLvlObjectDataList) {
						_shootLvlObjectDataList = _shootLvlObjectDataList->nextPtr;
						memset(_edx->dataPtr, 0, sizeof(ShootLvlObjectData));
					} else {
						warning("Nothing free in _shootLvlObjectDataList");
					}
					_edx->xPos = ptr->xPos;
					_edx->yPos = ptr->yPos;
					_edx->flags1 &= ~0x30;
					_edx->screenNum = ptr->screenNum;
					_edx->anim = 7;
					_edx->frame = 0;
					_edx->bitmapBits = 0;
					_edx->flags2 = (ptr->flags2 & ~0x2000) - 1;
					prependLvlObjectToList(&_lvlObjectsList0, _edx);
				}
// 40DDEE
				AndyLvlObjectData *_ecx = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
				LvlObject *_eax = _ecx->shootLvlObject;
				if (_eax) {
					if (!_eax->dataPtr) {
						warning("lvlObject %p with NULL dataPtr", _eax);
						break;
					}
					ShootLvlObjectData *_edx = (ShootLvlObjectData *)getLvlObjectDataPtr(_eax, kObjectDataTypeShoot);
					_edx->unk0 = 0;
				}
			} else {
// 40DE08
				if (!_esi->dataPtr) {
					warning("lvlObject %p with NULL dataPtr", _esi);
					break;
				}
				ShootLvlObjectData *_eax = (ShootLvlObjectData *)getLvlObjectDataPtr(_esi, kObjectDataTypeShoot);
				_esi->anim = (_eax->unk0 == 4) ? 14 : 15;
				updateAndyObject(_esi);
				setLvlObjectPosRelativeToObject(_esi, 0, ptr, 6);
				if (_currentLevel == kLvl_isld) {
					AndyLvlObjectData *_edx = (AndyLvlObjectData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
					assert(_edx == _edi);
					_esi->xPos += _edx->dxPos;
				}
			}
			break;
		case 2: {
				if (!_edi->shootLvlObject) {
					LvlObject *_edx = declareLvlObject(8, 3);
					_edi->shootLvlObject = _edx;
					_edx->dataPtr = _shootLvlObjectDataList;
					if (_shootLvlObjectDataList) {
						_shootLvlObjectDataList = _shootLvlObjectDataList->nextPtr;
						memset(_edx->dataPtr, 0, sizeof(ShootLvlObjectData));
					} else {
						warning("Nothing free in _shootLvlObjectDataList");
					}
					_edx->xPos = ptr->xPos;
					_edx->yPos = ptr->yPos;
					_edx->flags1 &= ~0x30;
					_edx->screenNum = ptr->screenNum;
					_edx->anim = 7;
					_edx->frame = 0;
					_edx->bitmapBits = 0;
					_edx->flags2 = (ptr->flags2 & ~0x2000) - 1;
					prependLvlObjectToList(&_lvlObjectsList0, _edx);
				}
// 40DEEC
				AndyLvlObjectData *_ecx = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
				assert(_ecx == _edi);
				LvlObject *_eax = _ecx->shootLvlObject;
				if (_eax) {
					if (!_eax->dataPtr) {
						warning("lvlObject %p with NULL dataPtr", _eax);
						break;
					}
					ShootLvlObjectData *_edx = (ShootLvlObjectData *)getLvlObjectDataPtr(_eax, kObjectDataTypeShoot);
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
				if (!_edi->shootLvlObject) {
					LvlObject *_edx = declareLvlObject(8, 3);
					_edi->shootLvlObject = _edx;
					_edx->dataPtr = _shootLvlObjectDataList;
					if (_shootLvlObjectDataList) {
						_shootLvlObjectDataList = _shootLvlObjectDataList->nextPtr;
						memset(_edx->dataPtr, 0, sizeof(ShootLvlObjectData));
					} else {
						warning("Nothing free in _shootLvlObjectDataList");
					}
					_edx->xPos = ptr->xPos;
					_edx->yPos = ptr->yPos;
					_edx->flags1 &= ~0x30;
					_edx->screenNum = ptr->screenNum;
					_edx->anim = 7;
					_edx->frame = 0;
					_edx->bitmapBits = 0;
					_edx->flags2 = (ptr->flags2 & ~0x2000) - 1;
					prependLvlObjectToList(&_lvlObjectsList0, _edx);
				}
// 40DF82
				AndyLvlObjectData *_ecx = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
				LvlObject *_eax = _ecx->shootLvlObject;
				if (_eax) {
					if (!_eax->dataPtr) {
						warning("lvlObject %p with NULL dataPtr", _eax);
						break;
					}
					ShootLvlObjectData *_edx = (ShootLvlObjectData *)getLvlObjectDataPtr(_eax, kObjectDataTypeShoot);
					_edx->unk0 = 4; // large power
				}
			}
			break;
		case 4:
			if (_esi) {
				_edi->shootLvlObject = 0;
				removeLvlObjectFromList(&_lvlObjectsList0, _esi);
				destroyLvlObject(_esi);
			}
			break;
		}
	}
}

int Game::lvlObjectType0Callback(LvlObject *ptr) {
	AndyLvlObjectData *_edi = 0;
	if (!_hideAndyObjectSprite) {
		_edi = (AndyLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeAndy);
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
	} else if (ptr->childPtr) {
		assert(_specialAnimLvlObject);
		if (_specialAnimLvlObject->screenNum != ptr->screenNum) {
			setLvlObjectPosRelativeToObject(ptr, 3, _specialAnimLvlObject, 6);
		}
	}
	switch (_currentLevel) {
	case 0:
	case 7:
		if (ptr->spriteNum == 0) {
			setupPlasmaCannonPoints(ptr);
		}
		break;
	case 9: // test_hod
		if (ptr->spriteNum == 0) {
			setupPlasmaCannonPoints(ptr);
		} else {
			setupSpecialPowers(ptr);
		}
		break;
	case 2: // pwr1_hod
		if (!_hideAndyObjectSprite && _edi->unk4 == 6) {
			lvlObjectType0CallbackBreathBubbles(ptr);
		}
		// fall through
	case 3:
	case 4:
	case 5:
	case 6:
		setupSpecialPowers(ptr);
		break;
	case 1:
	case 8:
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
		setLvlObjectPosRelativeToPoint(ptr, 0, _plasmaCannonPosX[_plasmaCannonFirstIndex], _plasmaCannonPosY[_plasmaCannonFirstIndex]);
		updateAndyObject(ptr);
		ptr->flags2 = merge_bits(ptr->flags2, _andyObject->flags2, 0x18);
		ptr->flags2 = merge_bits(ptr->flags2, _andyObject->flags2 + 1, 7);
	}
	return 0;
}

int Game::lvlObjectType7Callback(LvlObject *ptr) {
	ShootLvlObjectData *dat = (ShootLvlObjectData *)getLvlObjectDataPtr(ptr, kObjectDataTypeShoot);
	if (!dat) {
		return 0;
	}
	if ((ptr->flags0 & 0x1F) == 1) {
		dat->xPosObject = ptr->posTable[7].x + ptr->xPos;
		dat->yPosObject = ptr->posTable[7].y + ptr->yPos;
		ptr->xPos += dat->dxPos;
		ptr->yPos += dat->dyPos;
		if (!_hideAndyObjectSprite && ptr->screenNum == _andyObject->screenNum && (_andyObject->flags0 & 0x1F) != 0xB && clipLvlObjectsBoundingBox(_andyObject, ptr, 68) && (_mstFlags & 0x80000000) == 0 && (_cheats & kCheatSpectreFireballNoHit) == 0) {
			dat->unk3 = 0x80;
			dat->xPosShoot = _clipBoxOffsetX;
			dat->yPosShoot = _clipBoxOffsetY;
			setAndySpecialAnimation(0xA3);
		}
// 40D6E0
		if (dat->unk3 != 0x80 && dat->counter != 0) {
			if (dat->unk0 == 7) {
				ptr->yPos += calcScreenMaskDy(ptr->xPos + ptr->posTable[7].x, ptr->yPos + ptr->posTable[7].y + 4, ptr->screenNum);

			}
// 40D711
			const uint8_t ret = lvlObjectSpecialPowersCallbackScreen(ptr);
			if (ret != 0 && (dat->unk0 != 7 || (ret & 1) == 0)) {
				dat->unk3 = 0x80;
			}
		}
// 40D729
		assert(dat->state < 8);
		const uint8_t *anim = (dat->unk0 >= 7) ? &_byte_43E750[dat->state * 2] : &_byte_43E720[dat->state * 2];
		if (dat->counter != 0 && dat->unk3 != 0x80) {
			if (addLvlObjectToList3(ptr->spriteNum)) {
				LvlObject *o = _lvlObjectsList3;
				o->flags0 = ptr->flags0;
				o->flags1 = ptr->flags1;
				o->screenNum = ptr->screenNum;
				o->flags2 = ptr->flags2;
				o->anim = anim[1];
				if (_rnd._rndSeed & 1) {
					++o->anim;
				}
				if (o->anim > 12) {
					warning("Invalid frame number %d for fireball type %d", o->anim, dat->unk0);
					o->anim = 12;
				}
				o->frame = 0;
				if (dat->xPosShoot >= 256) {
					dat->xPosShoot -= _res->_screensBasePos[ptr->screenNum].u;
				}
				if (dat->yPosShoot >= 192) {
					dat->yPosShoot -= _res->_screensBasePos[ptr->screenNum].v;
				}
				setupLvlObjectBitmap(o);
				setLvlObjectPosRelativeToPoint(o, 0, dat->xPosObject, dat->yPosObject);
			}
		} else {
// 40D7F5
			dat->unk0 = (dat->unk0 < 7) ? 3 : 9;
			if (dat->counter != 0 && _actionDirectionKeyMaskIndex != 0xA3) {
				ptr->anim = anim[0];
			} else {
				ptr->anim = 3;
			}
// 40D81F
			ptr->frame = 0;
			if (dat->xPosShoot >= 256) {
				dat->xPosShoot -= _res->_screensBasePos[ptr->screenNum].u;
			}
			if (dat->yPosShoot >= 192) {
				dat->yPosShoot -= _res->_screensBasePos[ptr->screenNum].v;
			}
			setupLvlObjectBitmap(ptr);
			setLvlObjectPosRelativeToPoint(ptr, 0, dat->xPosShoot, dat->yPosShoot);
			return 0;
		}
	} else if ((ptr->flags0 & 0x1F) == 11) {
// 40D873
		dat->counter = ((ptr->flags0 & 0xE0) == 0x40) ? 0 : 1;
	}
// 40D88B
	if (dat->counter == 0) {
		if (ptr->spriteNum == 3) {
			removeLvlObjectFromList(&_lvlObjectsList0, ptr);
		} else {
			removeLvlObjectFromList(&_lvlObjectsList2, ptr);
		}
		destroyLvlObject(ptr);
	} else {
		--dat->counter;
		updateAndyObject(ptr);
	}
	if (setLvlObjectPosInScreenGrid(ptr, 3) < 0) {
		dat->counter = 0;
	}
	return 0;
}

int Game::lvlObjectType8Callback(LvlObject *ptr) {
	if (_mstLogicDisabled) {
		ptr->actionKeyMask = _andyObject->actionKeyMask;
		ptr->directionKeyMask = _andyObject->directionKeyMask;
		if (_andyObject->spriteNum == 2 && _lvlObjectsList0) {
			warning("lvlObjectType8CallbackHelper unimplemented");
			// lvlObjectType8CallbackHelper(ptr);
		}
		updateAndyObject(ptr);
		setLvlObjectPosInScreenGrid(ptr, 7);
	} else {
		const void *dataPtr = ptr->dataPtr;
		if (!dataPtr) {
			ptr->bitmapBits = 0;
			return 0;
		}
		int _ebx, var4;
		MonsterObject1 *m = 0; // _ebp
		if (dataPtr >= &_monsterObjects1Table[0] && dataPtr < &_monsterObjects1Table[kMaxMonsterObjects1]) {
			m = (MonsterObject1 *)ptr->dataPtr;
			_ebx = 1;
			var4 = m->monster1Index;
			if (m->flagsA6 & 2) {
				assert(ptr == m->o16);
				m->o16->actionKeyMask = _mstCurrentActionKeyMask;
				m->o16->directionKeyMask = _andyObject->directionKeyMask;
			}
			if (m->flagsA6 & 8) {
				ptr->bitmapBits = 0;
				return 0;
			}
		} else {
			assert(dataPtr >= &_monsterObjects2Table[0] && dataPtr < &_monsterObjects2Table[kMaxMonsterObjects2]);
			MonsterObject2 *mo = (MonsterObject2 *)dataPtr;
			m = mo->monster1;
			if (m) {
				_ebx = 2;
				var4 = m->monster1Index;
			} else {
				_ebx = 0;
				var4 = 4;
			}
			m = 0; // _ebp = 0
			if (mo->flags24 & 8) {
				ptr->bitmapBits = 0;
				return 0;
			}
		}
// 402BC2
		LvlObject *o = 0; // _edi
		updateAndyObject(ptr);
		if (m && m->o20) {
			o = m->o20;
			o->actionKeyMask = ptr->actionKeyMask;
			o->directionKeyMask = ptr->directionKeyMask;
			updateAndyObject(o);
			setLvlObjectPosRelativeToObject(ptr, 6, o, 6);
			addToSpriteList(o);
			setLvlObjectPosInScreenGrid(o, 7);
		}
// 402C03
		setLvlObjectPosInScreenGrid(ptr, 7);
		if (ptr->screenNum == _currentScreen || ptr->screenNum == _currentLeftScreen || ptr->screenNum == _currentRightScreen || o || (_currentLevel == kLvl_lar2 && ptr->spriteNum == 27) || (_currentLevel == kLvl_isld && ptr->spriteNum == 26)) {
// 402C57
			if (ptr->currentSound != 0xFFFF) {
				playSound(ptr->currentSound, ptr, _ebx, var4);
			}
			if (o && o->currentSound != 0xFFFF) {
				playSound(o->currentSound, o, _ebx, var4);
			}
		}
	}
// 402C8B
	if ((ptr->flags1 & 6) == 2) {
		ptr->yPos += calcScreenMaskDy(ptr->xPos + ptr->posTable[5].x, ptr->yPos + ptr->posTable[5].y, ptr->screenNum);
	}
	return 0;
}

int Game::lvlObjectList3Callback(LvlObject *o) {
	const uint8_t flags = o->flags0 & 0xFF;
	if ((o->spriteNum <= 7 && (flags & 0x1F) == 0xB) || (o->spriteNum > 7 && flags == 0x1F)) {
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
			_res->decLvlSpriteDataRefCounter(o);
			o->nextPtr = _declaredLvlObjectsListHead;
			--_declaredLvlObjectsListCount;
			_declaredLvlObjectsListHead = o;
			switch (o->spriteNum) {
			case 0:
			case 2:
				o->dataPtr = 0;
				break;
			case 3:
			case 7:
				if (o->dataPtr) {
					clearShootLvlObjectData(o);
				}
				break;
			}
		}
		if (o->sssObject) {
			removeSound(o);
		}
		o->sssObject = 0;
		o->bitmapBits = 0;
	} else {
		updateAndyObject(o);
		o->actionKeyMask = 0;
		o->directionKeyMask = 0;
		if (o->currentSound != 0xFFFF) {
			playSound(o->currentSound, o, 0, 3);
		}
		if (o->bitmapBits) {
			addToSpriteList(o);
		}
	}
	return 0;
}

void Game::lvlObjectSpecialPowersCallbackHelper1(LvlObject *o) {
	warning("lvlObjectSpecialPowersCallbackHelper1 unimplemented");
	// TODO
}

uint8_t Game::lvlObjectSpecialPowersCallbackScreen(LvlObject *o) {
	uint8_t var2F = 0;
	uint8_t screenNum = o->screenNum;
	uint8_t var30 = 0;

	int yPos = o->yPos; // _eax
	if ((o->flags0 & 0xE0) != 0x20) {
		yPos += o->posTable[6].y;
	} else {
		yPos += o->posTable[3].y;
	}
	int xPos = o->xPos + o->posTable[3].x; // _ecx

	int var1C;
	int var20;
	int var24 = xPos;
	if (xPos < 0) {
		xPos += 256;
		var20 = -256;
		var24 = xPos;
		screenNum = _res->_screensGrid[screenNum * 4 + 3];
	} else if (xPos >= 256) {
		xPos -= 256;
		var20 = 256;
		var24 = xPos;
		screenNum = _res->_screensGrid[screenNum * 4 + 1];
	} else {
		var20 = 0;
	}
	if (screenNum != 0xFF && yPos < 0) {
		yPos += 192;
		var1C = -192;
		screenNum = _res->_screensGrid[screenNum * 4 + 0];
	} else if (yPos >= 192) {
		assert(screenNum != 0xFF);
		yPos -= 192;
		var1C = 192;
		screenNum = _res->_screensGrid[screenNum * 4 + 2];
	} else {
		var1C = 0;
	}
// 40D0CA
	if (screenNum == 0xFF) {
		return 0;
	}
	uint8_t var2C, _bl;
	ShootLvlObjectData *dat = (ShootLvlObjectData *)getLvlObjectDataPtr(o, kObjectDataTypeShoot);
	uint8_t _cl = dat->unk0;
	const uint8_t *var10;
	const int *_esi;
	if (_cl == 4) {
		_bl = _cl;
		var2C = (o->flags1 >> 4) & 3;
		var10 = byte_43E6C0 + var2C * 8;
		_esi = dword_43E7F0 + var2C * 16 / sizeof(uint32_t);
	} else {
// 40D115
		var2C = dat->state;
		var10 = byte_43E680 + var2C * 8;
		_esi = dword_43E770 + var2C * 16 / sizeof(uint32_t);
		_bl = (_cl != 2) ? 4 : 2;
	}
// 40D147
	int var2E = _bl;
	int _edx = _res->_screensBasePos[screenNum].v + yPos;
	int _edi = _res->_screensBasePos[screenNum].u + var24; // _edi
	_edx = screenMaskOffset(_edi, _edx);
	int var8 = yPos & ~7;
	int var4 = screenGridOffset(var24, yPos);
	if (_cl >= 4) {
		var2C = 0;
		_edx += *_esi;
		int _ebp = _edx;
		uint8_t _cl = _screenMaskBuffer[_edx];
		uint8_t _al = 0;
		while ((_cl & 6) == 0) {
			++var2C;
			if (var2C >= var2E) {
				break; // goto 40D1D7;
			}
			_edx += _esi[var2C];
			_cl = _screenMaskBuffer[_edx];
		}
		if ((_cl & 6) != 0) {
			var2F = _al = 1;
		} else {
// 40D1D7
			_al = var2F;
		}
// 40D1DB
		_bl = dat->state; // var18
		if (_bl != 6 && _bl != 1 && _bl != 3) {
			_edx = _ebp;
			++_esi;
			var2C = 0;
			var30 = _screenMaskBuffer[_edx];
			while ((var30 & 1) == 0) {
				++var2C;
				if (var2C >= var2E) {
					break; // goto 40D236
				}
				_edx += *_esi++;
				var30 = _screenMaskBuffer[_edx];
			}
			if ((var30 & 1) != 0) {
				_al = 1;
			} else {
// 40D236
				_al = var2F;
			}
		}
// 40D23E
		if (_cl & 6) {
			var30 = _cl;
		}
		if (_al == 0) {
			return 0;
		}

	} else {
// 40D2F1
		_bl = dat->state;
		const uint8_t var2D = (_bl == 6 || _bl == 1 || _bl == 3) ? 6 : 7;
		var2C = 0;
		_edx += *_esi++;
		var30 = _screenMaskBuffer[_edx];
		while ((var30 & var2D) == 0) {
			++var2C;
			if (var2C >= var2E) {
				return var2F;
			}
			_edx += *_esi++;
			var30 = _screenMaskBuffer[_edx];
		}
	}
// 40D253
	dat->xPosShoot = var10[var2C * 2] + var20 + (var24 & ~7);
	dat->yPosShoot = var10[var2C * 2 + 1] + var1C + var8;
	_bl = dat->state;
	if (_bl != 2 && _bl != 4 && _bl != 7) {
		return var30;
	}
	var2C = 0;
	while (_res->_screensGrid[_res->_currentScreenResourceNum * 4 + var2C] != screenNum) {
		++var2C;
		if (var2C >= 4) {
// 40D35D
			if (o->screenNum != _res->_currentScreenResourceNum) {
				dat->yPosShoot += 4;
				return var30;
			}
			break;
		}
	}
// 40D384
	const int _ecx = (o->posTable[3].x + o->xPos) & 7;
	const uint8_t *p = _res->_resLevelData0x470CTablePtrData + _ecx;
	dat->yPosShoot += (int8_t)p[_screenPosTable[var2C][var4] * 8];
	return var30;
}

int Game::lvlObjectSpecialPowersCallback(LvlObject *o) {
	if (!o->dataPtr) {
		return 0;
	}
	ShootLvlObjectData *dat = (ShootLvlObjectData *)getLvlObjectDataPtr(o, kObjectDataTypeShoot);
	const uint16_t fl = o->flags0 & 0x1F;
	if (fl == 1) {
		if (dat->unk3 != 0x80 && dat->counter != 0) {
			uint8_t _al = lvlObjectSpecialPowersCallbackScreen(o);
			if (_al != 0) {
				if (dat->unk0 == 4 && (_al & 1) != 0 && (dat->state == 4 || dat->state == 2)) {
					dat->unk0 = 5;
					_al -= 4;
					dat->state = (_al != 0) ? 5 : 0;
				} else {
					dat->unk3 = 0x80;
				}
			}
		}
// 40D947
		if (dat->unk0 == 5) {
			dat->dyPos = 0;
			if (dat->unk3 != 0x80) {
				lvlObjectSpecialPowersCallbackHelper1(o);
			}
		}
		const uint8_t *p = (dat->unk0 >= 4) ? &byte_43E710[dat->state * 2] : &byte_43E700[dat->state * 2];
// 40D97C
		if (dat->unk3 != 0x80 && dat->counter != 0) {
			if (addLvlObjectToList3(o->spriteNum)) {
				LvlObject *ptr = _lvlObjectsList3;
				ptr->flags0 = o->flags0;
				ptr->flags1 = o->flags1;
				ptr->flags2 = o->flags2;
				ptr->screenNum = o->screenNum;
				ptr->anim = p[1];
				if (_rnd._rndSeed & 1) {
					++ptr->anim;
				}
				ptr->frame = 0;
				setupLvlObjectBitmap(ptr);
				setLvlObjectPosRelativeToObject(ptr, 0, o, 7);
				o->xPos += dat->dxPos;
				o->yPos += dat->dyPos;
			}
		} else {
// 40D9FC
			o->anim = p[0];
			if (dat->xPosShoot >= 256) {
				dat->xPosShoot -= _res->_screensBasePos[o->screenNum].u;
			}
			if (dat->yPosShoot >= 192) {
				dat->yPosShoot -= _res->_screensBasePos[o->screenNum].v;
			}
// 40DA36
			if (dat->o && (dat->o->actionKeyMask & 7) == 7) {
				const uint8_t num = dat->o->spriteNum;
				if ((num >= 8 && num <= 16) || num == 28) {
					o->anim = 16;
				}
			} else {
// 40DA5E
				if (dat->counter == 0) {
					o->anim = 16;
				}
			}
// 40DA6B
			if (dat->unk0 >= 4) {
				dat->unk0 = 6;
				if (dat->dxPos <= 0) {
					dat->xPosShoot += 8;
				}
				if (dat->dyPos <= 0) {
					dat->yPosShoot += 8;
				}
			} else {
				dat->unk0 = 1;
			}
			dat->dxPos = 0;
			dat->dyPos = 0;
			o->frame = 0;
			setupLvlObjectBitmap(o);
			setLvlObjectPosRelativeToPoint(o, 0, dat->xPosShoot, dat->yPosShoot);
			return 0;
		}
	} else if (fl == 11) {
// 40DAB1
		if ((o->flags0 & 0xE0) == 0x40) {
			dat->counter = 0;
		} else {
			dat->counter = 1;
		}
	}
// 40DAC9
	if (dat->counter == 0) {
		if (o->spriteNum == 3) {
			removeLvlObjectFromList(&_lvlObjectsList0, o);
		} else {
			removeLvlObjectFromList(&_lvlObjectsList2, o);
		}
		destroyLvlObject(o);
	} else {
		--dat->counter;
		updateAndyObject(o);
	}
// 40DB00
	if (setLvlObjectPosInScreenGrid(o, 3) < 0) {
		dat->counter = 0;
	}
	return 0;
}

void Game::lvlObjectTypeCallback(LvlObject *o) {
	switch (o->spriteNum) {
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
		o->callbackFuncPtr = 0;
		break;
	case 7: // flying spectre fireball
		o->callbackFuncPtr = &Game::lvlObjectType7Callback;
		break;
	case 8: // lizard
	case 9: // spider
	case 10: // flying spectre
	case 11: // dog
	case 12: // spectre
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21: // lava
	case 22:
	case 23:
	case 24: // rope
	case 25: // swamp serpent
	case 26: // plant
	case 27: // green fire-fly
	case 28:
	case 29:
		o->callbackFuncPtr = &Game::lvlObjectType8Callback;
		break;
	default:
		warning("lvlObjectTypeCallback unhandled case %d", o->spriteNum);
		break;
	}
}

LvlObject *Game::addLvlObject(int type, int x, int y, int screen, int num, int o_anim, int o_flags1, int o_flags2, int actionKeyMask, int directionKeyMask) {
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

int Game::setLvlObjectPosInScreenGrid(LvlObject *o, int pos) {
	int ret = 0;
	if (o->screenNum < _res->_lvlHdr.screensCount) {
		int xPrev = o->xPos;
		int x = o->xPos + o->posTable[pos].x;
		int yPrev = o->yPos;
		int y = o->yPos + o->posTable[pos].y;
		int numPrev = o->screenNum;
		int screenNum = o->screenNum;
		if (x < 0) {
			o->screenNum = _res->_screensGrid[screenNum * 4 + kPosLeftScreen];
			o->xPos = xPrev + 256;
		} else if (x >= 256) {
			o->screenNum = _res->_screensGrid[screenNum * 4 + kPosRightScreen];
			o->xPos = xPrev - 256;
		}
		screenNum = o->screenNum;
		if (y < 0 && screenNum != 0xFF) {
			o->screenNum = _res->_screensGrid[screenNum * 4 + kPosTopScreen];
			o->yPos = yPrev + 192;
		} else if (y >= 192) {
			assert(screenNum != 0xFF);
			o->screenNum = _res->_screensGrid[screenNum * 4 + kPosBottomScreen];
			o->yPos = yPrev - 192;
		}
		screenNum = o->screenNum;
		if (screenNum == 0xFF) {
			o->xPos = xPrev;
			o->yPos = yPrev;
			o->screenNum = numPrev;
			ret = -1;
		} else if (screenNum != numPrev) {
			ret = 1;
		}
	}
	return ret;
}

LvlObject *Game::declareLvlObject(uint8_t type, uint8_t num) {
	if (type != 8 || _res->_resLevelData0x2988PtrTable[num] != 0) {
		if (_declaredLvlObjectsListCount < kMaxLvlObjects) {
			assert(_declaredLvlObjectsListHead);
			LvlObject *ptr = _declaredLvlObjectsListHead;
			_declaredLvlObjectsListHead = _declaredLvlObjectsListHead->nextPtr;
			assert(ptr);
			++_declaredLvlObjectsListCount;
			ptr->spriteNum = num;
			ptr->type = type;
			if (type == 8) {
				_res->incLvlSpriteDataRefCounter(ptr);
				lvlObjectTypeCallback(ptr);
			}
			ptr->currentSprite = 0;
			ptr->sssObject = 0;
			ptr->nextPtr = 0;
			ptr->bitmapBits = 0;
			return ptr;
		}
	}
	return 0;
}

void Game::clearDeclaredLvlObjectsList() {
	memset(_declaredLvlObjectsList, 0, sizeof(_declaredLvlObjectsList));
	for (int i = 0; i < kMaxLvlObjects - 1; ++i) {
		_declaredLvlObjectsList[i].nextPtr = &_declaredLvlObjectsList[i + 1];
	}
	_declaredLvlObjectsList[kMaxLvlObjects - 1].nextPtr = 0;
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
			assert(_animBackgroundDataCount < kMaxBackgroundAnims);
			ptr->dataPtr = &_animBackgroundDataTable[_animBackgroundDataCount++];
			memset(ptr->dataPtr, 0, sizeof(AnimBackgroundData));
			break;
		case 1:
			if (ptr->dataPtr) {
				warning("Trying to free _resLvlScreenBackgroundDataTable.backgroundSoundTable (i=%d index=%d)", i, index);
			}
			ptr->xPos = 0;
			ptr->yPos = 0;
			break;
		case 2:
			if (prevLvlObj == &_res->_dummyObject) {
				prevLvlObj = 0;
				ptr->childPtr = ptr->nextPtr;
			} else {
				prevLvlObj = ptr->childPtr;
			}
			break;
		}
	}
	for (int i = _res->_lvlHdr.staticLvlObjectsCount; i < _res->_lvlHdr.staticLvlObjectsCount + _res->_lvlHdr.otherLvlObjectsCount; ++i) {
		LvlObject *ptr = &_res->_resLvlScreenObjectDataTable[i];
		lvlObjectTypeInit(ptr);
	}
}

void Game::setLvlObjectSprite(LvlObject *ptr, uint8_t type, uint8_t num) {
	if (ptr->type == 8) {
		_res->decLvlSpriteDataRefCounter(ptr);
		ptr->spriteNum = num;
		ptr->type = type;
		_res->incLvlSpriteDataRefCounter(ptr);
	}
}

LvlObject *Game::findLvlObject(uint8_t type, uint8_t spriteNum, int screenNum) {
	LvlObject *ptr = _screenLvlObjectsList[screenNum];
	while (ptr) {
		if (ptr->type == type && ptr->spriteNum == spriteNum) {
			break;
		}
		ptr = ptr->nextPtr;
	}
	return ptr;
}

LvlObject *Game::findLvlObject2(uint8_t type, uint8_t dataNum, int screenNum) {
	LvlObject *ptr = _screenLvlObjectsList[screenNum];
	while (ptr) {
		if (ptr->type == type && ptr->dataNum == dataNum) {
			break;
		}
		ptr = ptr->nextPtr;
	}
	return ptr;
}

LvlObject *Game::findLvlObjectType2(int spriteNum, int screenNum) {
	LvlObject *ptr = _screenLvlObjectsList[screenNum];
	while (ptr) {
		if (ptr->type == 2 && ptr->spriteNum == spriteNum && !ptr->dataPtr) {
			break;
		}
		ptr = ptr->nextPtr;
	}
	return ptr;
}

LvlObject *Game::findLvlObjectBoundingBox(BoundingBox *box) {
	LvlObject *ptr = _lvlObjectsList0;
	while (ptr) {
		if ((ptr->flags0 & 0x1F) != 0xB || (ptr->flags0 & 0xE0) != 0x40) {
			BoundingBox b;
			b.x1 = ptr->xPos;
			b.x2 = ptr->xPos + ptr->width - 1;
			b.y1 = ptr->yPos;
			b.y2 = ptr->yPos + ptr->height - 1;
			const uint8_t *coords = _res->getLvlSpriteCoordPtr(ptr->levelData0x2988, ptr->currentSprite);
			if (updateBoundingBoxClippingOffset(box, &b, coords, (ptr->flags1 >> 4) & 3)) {
				return ptr;
			}
		}
		ptr = ptr->nextPtr;
	}
	return 0;
}

void Game::postScreenUpdate_lava_helper(int yPos) {
	const uint8_t flags = (_andyObject->flags0) & 0x1F;
	if (!_hideAndyObjectSprite) {
		if ((_mstFlags & 0x80000000) == 0) {
			uint8_t mask = 0;
			const int y = _andyObject->yPos;
			if (_andyObject->posTable[5].y + y >= yPos || _andyObject->posTable[4].y + y >= yPos) {
				mask = 0xA3;
			}
			if (flags == 2 || _andyObject->posTable[7].y + y >= yPos) {
				mask = 0xA3;
			}
			if (mask != 0 && _actionDirectionKeyMaskIndex > 0) {
				_actionDirectionKeyMaskIndex = mask;
				_actionDirectionKeyMaskCounter = 0;
			}
		} else if (flags == 0xB) {
			_mstFlags &= 0x7FFFFFFF;
		}
	}
}

void Game::updateLevelTick_lar(int count, uint8_t *p1, BoundingBox *r) {
	for (int i = 0; i < count; ++i) {
		p1[i * 4 + 1] &= ~0x40;
	}
	for (int i = 0; i < count; ++i) {
		if (_andyObject->screenNum == p1[i * 4]) {
			if ((p1[i * 4 + 1] & 0x10) == 0x10) {
				updateLevelTick_lar_helper1(i, &p1[i * 4], &r[i]);
			}
			AndyLvlObjectData *data = (AndyLvlObjectData *)getLvlObjectDataPtr(_andyObject, kObjectDataTypeAndy);
			updateLevelTick_lar_helper2(i, &p1[i * 4], &data->boundingBox, &r[i]);
		}
	}
	for (int i = 0; i < count; ++i) {
		uint8_t _al = p1[i * 4 + 1];
		uint8_t _dl = _al & 0x40;
		if (_dl == 0 && (_al & 0x80) == 0) {
			p1[i * 4 + 1] |= 0x80;
		} else if (_dl != 0 && (_al & 4) != 0) {
			p1[i * 4 + 1] &= ~1;
		}
	}
}

void Game::updateLevelTick_lar_helper1(int num, uint8_t *p, BoundingBox *r) {
	bool found = false;
	for (LvlObject *o = _lvlObjectsList1; o && !found; o = o->nextPtr) {
		if (o->screenNum != p[0]) {
			continue;
		}
		if (!((o->spriteNum >= 11 && o->spriteNum <= 13) || o->spriteNum == 16)) {
			continue;
		}
		BoundingBox b;
		b.x1 = o->xPos;
		b.y1 = o->yPos;
		b.x2 = b.x1 + o->width - 1;
		b.y2 = b.y1 + o->height - 1;
		uint8_t *_edi;
		if ((p[1] & 0x40) == 0 && clipBoundingBox(r, &b)) {
			found = true;
// 406B66
			if ((p[2] & 0x80) == 0 && !updateLevelTick_lar_helper3(true, p[2], p[0], num, (p[1] >> 5) & 1)) {
				continue;
			}
			p[1] |= 0x40;
			if ((p[1] & 0x8) != 0) {
				continue;
			}
			if (_currentLevel == kLvl_lar2) {
				_edi = &Game::_lar2_unkData0[p[3] * 4];
			} else {
				_edi = &Game::_lar1_unkData0[p[3] * 4];
			}
			uint8_t _al = (p[1] >> 1) & 1;
			uint8_t _bl = (_edi[0] >> 4);
			if (_bl == _al) {
				continue;
			}
			_bl = (_al << 4) | (_edi[0] & 15);
			_edi[0] = _bl;
			uint8_t _cl = (p[1] >> 5) & 1;
			if (_cl != 1 || _al != _cl) {
				continue;
			}
			_edi[3] = 4;
		} else {
// 406C3A
			if ((p[1] & 0xC) == 0 && (p[1] & 0x80) != 0) {
				if (_currentLevel == kLvl_lar2) {
					_edi = &Game::_lar2_unkData0[p[3] * 4];
				} else {
					_edi = &Game::_lar1_unkData0[p[3] * 4];
				}
				uint8_t _al = ((~p[1]) >> 1) & 1;
				uint8_t _bl = (_edi[0] >> 4);
				if (_bl == _al) {
					continue;
				}
				_bl = (_al << 4) | (_edi[0] & 15);
				_edi[0] = _bl;
				uint8_t _cl = (p[1] >> 5) & 1;
				if (_cl != 1 || _al != _cl) {
					continue;
				}
				_edi[3] = 4;
			}
		}
	}
}

int Game::updateLevelTick_lar_helper2(int num, uint8_t *p, BoundingBox *b1, BoundingBox *b2) {
	int ret = 0;
	//const uint8_t flags = _andyObject->flags0 & 0x1F;
	if ((p[1] & 0x40) == 0) {
		ret = clipBoundingBox(b1, b2);
		if (ret) {
			if ((p[1] & 1) == 0) {
				return ret;
			}
// 4068A4
			const int flag = (p[1] >> 5) & 1;
			uint8_t _al = updateLevelTick_lar_helper5(b2, flag);
			_al = updateLevelTick_lar_helper3(_al, p[2], p[0], num, flag);
			p[1] = ((_al & 1) << 6) | (p[1] & ~0x40);
			_al = p[1];
			if ((_al & 0x40) == 0) {
				return ret;
			}
			ret = 1;
			if ((_al & 8) != 0) {
				if (_al & 2) {
					_al = p[3];
				} else {
					_al = -p[3];
				}
				int _bl, i;
				if (_al < 0) {
					i = (-_al) * 6;
					updateLevelTick_lar_helper4(&_lar1_unkData1[i], 0);
					_bl = 5;
				} else {
					i = _al * 6;
					updateLevelTick_lar_helper4(&_lar1_unkData1[i], 1);
					_bl = 2;
				}
				LvlObject *o = findLvlObject2(0, _lar1_unkData1[i + 5], _lar1_unkData1[i + 4]);
				if (o) {
					o->objectUpdateType = _bl;
				}
				return ret;
			}
// 40699A
			uint8_t _cl = (_al >> 5) & 1;
			_al = (_al >> 1) & 1;
			uint8_t *_esi;
			if (_currentLevel == kLvl_lar2) {
				_esi = &Game::_lar2_unkData0[p[3] * 4];
			} else {
				_esi = &Game::_lar1_unkData0[p[3] * 4];
			}
			uint8_t _dl = _esi[0];
			uint8_t _bl = _dl >> 4;
			if (_bl == _al) {
				return ret;
			}
			_bl = (_al << 4) | (_dl & 15);
			*_esi=  _bl;
			if (_cl != 1 || _al != _cl) {
				return ret;
			}
			_esi[3] = 4;
			return ret;
		}
	}
// 406A0D
	if ((p[1] & 0xC) == 0 && (p[1] & 0x80) != 0) {
		if (_currentLevel == kLvl_lar2) {
			p = &Game::_lar2_unkData0[p[3] * 4];
		} else {
			p = &Game::_lar1_unkData0[p[3] * 4];
		}
		uint8_t _al = ((~p[1]) >> 1) & 1;
		uint8_t _cl = (p[1] >> 5) & 1;
		uint8_t _bl = p[0] >> 4;
		if (_bl == _al) {
			return ret;
		}
		_bl = (_al << 4) | (p[0] & 0xF);
		p[0] = _bl;
		if (_cl != 1 || _al != _cl) {
			return ret;
		}
		p[3] = 4;
	}
	return ret;
}

int Game::updateLevelTick_lar_helper3(bool flag, int dataNum, int screenNum, int boxNum, int anim) {
	uint8_t _al = (_andyObject->flags0 >> 5) & 7;
	uint8_t _cl = (_andyObject->flags0 & 0x1F);
	uint8_t _bl = 0;
	if (dataNum >= 0) {
		BoundingBox *box;
		if (_currentLevel == kLvl_lar2) {
			box = &Game::_lar2_unkData2[boxNum];
		} else {
			box = &Game::_lar1_unkData2[boxNum];
		}
		int dy = box->y2 - box->y1;
		int dx = box->x2 - box->x1;
		if (dx >= dy) {
			_bl = 1;
		} else {
			uint8_t _dl = ((_andyObject->flags1) >> 4) & 3;
			if (anim != _dl) {
				return 0;
			}
			if (_cl == 3 || _cl == 5 || _cl == 2 || _al == 2 || _cl == 0 || _al == 7) {
				_bl = 1;
			} else {
				setAndySpecialAnimation(anim);
			}
		}
	} else {
// 406736
		_bl = 1;
	}
// 406738
	if (_bl) {
		if (!flag) {
			_bl = _andyObject->anim == 224;
		}
		if (_bl) {
			LvlObject *o = findLvlObject(0, dataNum, screenNum);
			if (o) {
				o->objectUpdateType = 7;
			}
		}
	}
	return _bl;
}

// updateScreenMaskLar
void Game::updateLevelTick_lar_helper4(uint8_t *p, int flag) {
	if (p[1] != flag) {
		p[1] = flag;
		const uint8_t screenNum = p[4];
		uint32_t offset = screenMaskOffset(_res->_screensBasePos[screenNum].u + p[2], _res->_screensBasePos[screenNum].v + p[3]);
		uint8_t *dst = _screenMaskBuffer + offset;
		const int16_t *src = off_452580[p[0]];
		const int count = *src++;
		if (flag == 0) {
			for (int i = 0; i < count; ++i) {
				offset = *src++;
				dst += offset;
				*dst &= ~8;
			}
		} else {
			for (int i = 0; i < count; ++i) {
				offset = *src++;
				dst += offset;
				*dst |= 8;
			}
		}
	}
}

// clipAndyLvlObjectLar
int Game::updateLevelTick_lar_helper5(BoundingBox *b, bool flag) {
	int ret = 0;
	const uint8_t flags = _andyObject->flags0 & 0x1F;
	if (flags != 3 && flags != 5 && !flag) {
		BoundingBox b1;
		b1.x1 = _andyObject->xPos + _andyObject->posTable[5].x - 3;
		b1.x2 = b1.x1 + 6;
		b1.y1 = _andyObject->yPos + _andyObject->posTable[5].y - 3;
		b1.y2 = b1.y1 + 6;
		ret = clipBoundingBox(&b1, b);
		if (!ret) {
			BoundingBox b2;
			b2.x1 = _andyObject->xPos + _andyObject->posTable[4].x - 3;
			b2.x2 = b2.x1 + 6;
			b2.y1 = _andyObject->yPos + _andyObject->posTable[4].y - 3;
			b2.y2 = b2.y1 + 6;
			ret = clipBoundingBox(&b2, b);
		}
	}
	return ret;
}

void Game::resetCrackSprites() {
	memset(_crackSpritesTable, 0, sizeof(_crackSpritesTable));
	_crackSpritesCount = 0;
}

void Game::updateCrackSprites() {
	const uint8_t screenNum = _res->_currentScreenResourceNum;
	CrackSprite *spr = 0;
	for (int i = 0; i < _crackSpritesCount; ++i) {
		if (_crackSpritesTable[i].screenNum == screenNum) {
			spr = &_crackSpritesTable[i];
			break;
		}
	}
	if (!spr) {
		return;
	}
	LvlObject tmp;
	memset(&tmp, 0, sizeof(tmp));
	tmp.screenNum = screenNum;
	tmp.flags2 = 0x1001;
	tmp.type = 8;
	tmp.spriteNum = 20;
	_res->incLvlSpriteDataRefCounter(&tmp);
	uint32_t var68 = 0;
	for (int i = 0; i < 6; ++i) {
		uint32_t *flags = &spr->flags[i];
		uint32_t _eax = 0;
		if (*flags != _eax) {
			int var70 = 0;
			int var6C = 0;
			for (int var75 = 0; var75 < 11; ++var75) {
				uint8_t _al = (*flags >> (var70 * 2)) & 3;
				if (_al != 0 && _spritesListNextPtr != 0) {
					const int _edi = spr->initData2 + var6C + 12; // xPos
					const int _ebp = spr->initData3 + var68 + 16; // yPos
					if (_edi >= spr->initData8 && _edi <= spr->initDataA && _ebp >= spr->initData9 && _ebp <= spr->initDataB) {
						_al += 3;
					} else if (_edi >= spr->initDataC && _edi <= spr->initDataE && _ebp >= spr->initDataD && _ebp <= spr->initDataF) {
						_al += 6;
					}
					tmp.anim = _al;
					setupLvlObjectBitmap(&tmp);
					setLvlObjectPosRelativeToPoint(&tmp, 7, _edi, _ebp);
					if (var75 & 1) {
						tmp.yPos -= 16;
					}
					if (READ_LE_UINT16(tmp.bitmapBits) != 8) {
						Sprite *spr = _spritesListNextPtr;
						spr->xPos = _edi;
						spr->yPos = _ebp;
						spr->bitmapBits = tmp.bitmapBits;
						spr->num = tmp.flags2 & 0x3FFF;
						const int index = spr->num & 0x1F;
						_spritesListNextPtr = spr->nextPtr;
						spr->nextPtr = _spriteListPtrTable[index];
						_spriteListPtrTable[index] = spr;
					}
				}
				++var70;
				var6C += 24;
			}
		}
		var68 += 32;
	}
	_res->decLvlSpriteDataRefCounter(&tmp);
}

void Game::captureScreenshot() {
	static int screenshot = 1;
	char name[64];

	snprintf(name, sizeof(name), "screenshot-%03d-front.bmp", screenshot);
	saveBMP(name, _video->_frontLayer, _video->_palette, Video::W, Video::H);

	snprintf(name, sizeof(name), "screenshot-%03d-background.bmp", screenshot);
	saveBMP(name, _video->_backgroundLayer, _video->_palette, Video::W, Video::H);

	snprintf(name, sizeof(name), "screenshot-%03d-shadow.bmp", screenshot);
	saveBMP(name, _video->_shadowLayer, _video->_palette, Video::W, Video::H);

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
