/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir (cyx@users.sourceforge.net)
 */

#include "fileio.h"
#include "fs.h"
#include "game.h"
#include "lzw.h"
#include "resource.h"
#include "util.h"

static const char *kSetupDat = "SETUP.DAT";

static bool openDat(FileSystem &fs, const char *name, File *f) {
	FILE *fp = fs.openFile(name);
	if (fp) {
		f->setFp(fp);
		return true;
	}
	return false;
}

static void closeDat(FileSystem &fs, File *f) {
	if (f->_fp) {
		fs.closeFile(f->_fp);
		f->setFp(0);
	}
}

Resource::Resource(const char *dataPath)
	: _fs(dataPath) {

	memset(_screensGrid, 0, sizeof(_screensGrid));
	memset(_screensBasePos, 0, sizeof(_screensBasePos));
	memset(_screensState, 0, sizeof(_screensState));

	// masks
	_resLevelData0x470CTable = 0;
	_resLevelData0x470CTablePtrHdr = 0;
	_resLevelData0x470CTablePtrData = 0;

	// sprites
	memset(_resLevelData0x2988SizeTable, 0, sizeof(_resLevelData0x2988SizeTable));
	memset(_resLevelData0x2988Table, 0, sizeof(_resLevelData0x2988Table));
	memset(_resLevelData0x2988PtrTable, 0, sizeof(_resLevelData0x2988PtrTable));

	// backgrounds
	memset(_resLvlScreenBackgroundDataTable, 0, sizeof(_resLvlScreenBackgroundDataTable));
	memset(_resLvlScreenBackgroundDataPtrTable, 0, sizeof(_resLvlScreenBackgroundDataPtrTable));
	memset(_resLevelData0x2B88SizeTable, 0, sizeof(_resLevelData0x2B88SizeTable));

	memset(_resLvlScreenObjectDataTable, 0, sizeof(_resLvlScreenObjectDataTable));
	memset(&_dummyObject, 0, sizeof(_dummyObject));

	if (sectorAlignedGameData()) {
		_datFile = new SectorFile;
		_lvlFile = new SectorFile;
		_mstFile = new SectorFile;
		_sssFile = new SectorFile;
	} else {
		_datFile = new File;
		_lvlFile = new File;
		_mstFile = new File;
		_sssFile = new File;
	}
	memset(&_datHdr, 0, sizeof(_datHdr));
	memset(&_lvlHdr, 0, sizeof(_lvlHdr));
	memset(&_mstHdr, 0, sizeof(_mstHdr));
	memset(&_sssHdr, 0, sizeof(_sssHdr));

	_loadingImageBuffer = 0;
	_fontBuffer = 0;
	_menuBuffer0 = 0;
	_menuBuffer1 = 0;
}

Resource::~Resource() {
	delete _datFile;
	delete _lvlFile;
	delete _mstFile;
	delete _sssFile;
}

bool Resource::sectorAlignedGameData() {
	FILE *fp = _fs.openFile(kSetupDat);
	if (!fp) {
		error("Unable to open '%s'", kSetupDat);
		return false;
	}
	bool ret = false;
	uint8_t buf[2048];
	if (fread(buf, 1, sizeof(buf), fp) == sizeof(buf)) {
		ret = fioUpdateCRC(0, buf, sizeof(buf)) == 0;
	}
	fclose(fp);
	return ret;
}

void Resource::loadSetupDat() {
	uint8_t hdr[512];
	openDat(_fs, kSetupDat, _datFile);
	_datFile->read(hdr, sizeof(hdr));
	_datHdr.version = READ_LE_UINT32(hdr);
	_datHdr.bufferSize0 = READ_LE_UINT32(hdr + 0x4);
	_datHdr.bufferSize1 = READ_LE_UINT32(hdr + 0x8);
	_datHdr.sssOffset = READ_LE_UINT32(hdr + 0xC);
	_datHdr.iconsCount = READ_LE_UINT32(hdr + 0x10);
	_datHdr.menusCount = READ_LE_UINT32(hdr + 0x14);
	_datHdr.cutscenesCount = READ_LE_UINT32(hdr + 0x18);
	_datHdr.levelsCount = READ_LE_UINT32(hdr + 0x1C);
	_datHdr.checkpointsLevel1Count = READ_LE_UINT32(hdr + 0x20);
	_datHdr.checkpointsLevel2Count = READ_LE_UINT32(hdr + 0x24);
	_datHdr.checkpointsLevel3Count = READ_LE_UINT32(hdr + 0x28);
	_datHdr.checkpointsLevel4Count = READ_LE_UINT32(hdr + 0x2C);
	_datHdr.checkpointsLevel5Count = READ_LE_UINT32(hdr + 0x30);
	_datHdr.checkpointsLevel6Count = READ_LE_UINT32(hdr + 0x34);
	_datHdr.checkpointsLevel7Count = READ_LE_UINT32(hdr + 0x38);
	_datHdr.checkpointsLevel8Count = READ_LE_UINT32(hdr + 0x3C);
	_datHdr.yesNoQuitImage = READ_LE_UINT32(hdr + 0x40);
	_datHdr.loadingImageSize = READ_LE_UINT32(hdr + 0x48);
	const int hintsCount = (_datHdr.version == 11) ? 46 : 20;
	for (int i = 0; i < hintsCount; ++i) {
		_datHdr.hintsImageOffsetTable[i] = READ_LE_UINT32(hdr + 0x4C + i * 4);
		_datHdr.hintsImageSizeTable[i] = READ_LE_UINT32(hdr + 0x4C + (hintsCount + i) * 4);
	}
	_datFile->seek(2048, SEEK_SET); // fioAlignSizeTo2048(76)
	_loadingImageBuffer = (uint8_t *)malloc(_datHdr.loadingImageSize);
	if (_loadingImageBuffer) {
		_datFile->read(_loadingImageBuffer, _datHdr.loadingImageSize);

		uint32_t offset = 0;

		// loading image
		uint32_t size = READ_LE_UINT32(_loadingImageBuffer + offset); offset += 8;
		offset += size + 768;

		// loading animation
		size = READ_LE_UINT32(_loadingImageBuffer + offset + 8); offset += 16;
		offset += size;

		// font
		static const int kFontSize = 16 * 16 * 64;
		_fontBuffer = (uint8_t *)malloc(kFontSize);
		if (_fontBuffer) {
			size = READ_LE_UINT32(_loadingImageBuffer + offset); offset += 4;
			if (_datHdr.version == 11) {
				const uint32_t uncompressedSize = decodeLZW(_loadingImageBuffer + offset, _fontBuffer);
				assert(uncompressedSize == kFontSize);
			} else {
				memcpy(_fontBuffer, _loadingImageBuffer + offset, kFontSize);
			}
		}
	}
	_menuBuffersOffset = READ_LE_UINT32(hdr + 0x4C + (2 + _datHdr.yesNoQuitImage) * 4);
}

void Resource::loadLevelData(const char *levelName) {
	char filename[32];

	closeDat(_fs, _lvlFile);
	snprintf(filename, sizeof(filename), "%s.LVL", levelName);
	if (openDat(_fs, filename, _lvlFile)) {
		loadLvlData(_lvlFile, filename);
	} else {
		error("Unable to open '%s'", filename);
	}

	closeDat(_fs, _mstFile);
	snprintf(filename, sizeof(filename), "%s.MST", levelName);
	if (openDat(_fs, filename, _mstFile)) {
		loadMstData(_mstFile, filename);
	} else {
		warning("Unable to open '%s'", filename);
		memset(&_mstHdr, 0, sizeof(_mstHdr));
	}

	closeDat(_fs, _sssFile);
	snprintf(filename, sizeof(filename), "%s.SSS", levelName);
	if (openDat(_fs, filename, _sssFile)) {
		loadSssData(_sssFile, filename);
	} else {
		warning("Unable to open '%s'", filename);
		memset(&_sssHdr, 0, sizeof(_sssHdr));
	}
}

void Resource::unloadLevelData() {
}

void Resource::loadLvlScreenMoveData(int num) { // GridData
	_lvlFile->seekAlign(0x8 + num * 4);
	_lvlFile->read(&_screensGrid[num * 4], 4);
}

void Resource::loadLvlScreenVectorData(int num) {
	_lvlFile->seekAlign(0xA8 + num * 8);
	LvlScreenVector *dat = &_screensBasePos[num];
	dat->u = _lvlFile->readUint32();
	dat->v = _lvlFile->readUint32();
}

void Resource::loadLvlScreenStateData(int num) {
	_lvlFile->seekAlign(0x1E8 + num * 4);
	LvlScreenState *dat = &_screensState[num];
	dat->s0 = _lvlFile->readByte();
	dat->s1 = _lvlFile->readByte();
	dat->s2 = _lvlFile->readByte();
	dat->s3 = _lvlFile->readByte();
}

void Resource::loadLvlScreenObjectData(int num) {
	_lvlFile->seekAlign(0x288 + num * 96);
	LvlObject *dat = &_resLvlScreenObjectDataTable[num];

	dat->xPos = _lvlFile->readUint32();
	dat->yPos = _lvlFile->readUint32();
	dat->screenNum = _lvlFile->readByte();
	dat->screenState = _lvlFile->readByte();
	dat->dataNum = _lvlFile->readByte();
	dat->frame = _lvlFile->readByte();
	dat->anim = _lvlFile->readUint16();
	dat->type = _lvlFile->readByte();
	dat->spriteNum = _lvlFile->readByte();
	dat->flags0 = _lvlFile->readUint16();
	dat->flags1 = _lvlFile->readUint16();
	dat->flags2 = _lvlFile->readUint16();
	dat->objectUpdateType = _lvlFile->readByte();
	dat->hitCount = _lvlFile->readByte();
	const uint32_t objRef = _lvlFile->readUint32();
	if (objRef) {
		dat->childPtr = &_dummyObject;
		debug(kDebug_RESOURCE, "loadLvlObj num %d linkObjRef 0x%x", num, objRef);
	}
	dat->width = _lvlFile->readUint16();
	dat->height = _lvlFile->readUint16();
	dat->directionKeyMask = _lvlFile->readByte();
	dat->actionKeyMask = _lvlFile->readByte();
	dat->currentSprite = _lvlFile->readUint16();
	dat->currentSound = _lvlFile->readUint16();
	dat->unk26 = _lvlFile->readByte();
	dat->unk27 = _lvlFile->readByte();
	dat->bitmapBits = 0; _lvlFile->readUint32();
	dat->callbackFuncPtr = 0; _lvlFile->readUint32();
	dat->dataPtr = 0; _lvlFile->readUint32();
	dat->sssObject = 0; _lvlFile->readUint32();
	dat->levelData0x2988 = 0; _lvlFile->readUint32();
	for (int i = 0; i < 8; ++i) {
		dat->posTable[i].x = _lvlFile->readUint16();
		dat->posTable[i].y = _lvlFile->readUint16();
	}
	dat->nextPtr = 0; _lvlFile->readUint32();
}

static uint32_t resFixPointersLevelData0x2988(uint8_t *src, uint8_t *ptr, LvlObjectData *dat) {
	uint8_t *base = src;

	dat->unk0 = *src++;
	dat->spriteNum = *src++;
	dat->framesCount = READ_LE_UINT16(src); src += 2;
	dat->hotspotsCount = READ_LE_UINT16(src); src += 2;
	dat->movesCount = READ_LE_UINT16(src); src += 2;
	dat->coordsCount = READ_LE_UINT16(src); src += 2;
	dat->refCount = *src++;
	dat->frame = *src++;
	dat->anim = READ_LE_UINT16(src); src += 2;
	dat->unkE = *src++;
	dat->unkF = *src++;
	src += 4; // 0x10
	uint32_t movesDataOffset = READ_LE_UINT32(src); src += 4; // 0x14
	src += 4; // 0x18
	uint32_t framesDataOffset = READ_LE_UINT32(src); src += 4; // 0x1C
	src += 4; // 0x20
	uint32_t coordsDataOffset = READ_LE_UINT32(src); src += 4; // 0x24
	uint32_t hotspotsDataOffset = READ_LE_UINT32(src); src += 4; // 0x28

	if (dat->refCount != 0) {
		return 0;
	}

	assert(src == base + kLvlAnimHdrOffset);
	dat->animsInfoData = base;
	if (__BYTE_ORDER == __BIG_ENDIAN) {
		for (int i = 0; i < dat->hotspotsCount; ++i) {
			LvlAnimHeader *ah = ((LvlAnimHeader *)(base + kLvlAnimHdrOffset)) + i;
			ah->unk0 = le16toh(ah->unk0);
			ah->seqOffset = le32toh(ah->seqOffset);
			if (ah->seqOffset != 0) {
				for (int seq = 0; seq < ah->seqCount; ++seq) {
					LvlAnimSeqHeader *ash = ((LvlAnimSeqHeader *)(base + ah->seqOffset)) + seq;
					ash->firstFrame = le16toh(ash->firstFrame);
					ash->unk2 = le16toh(ash->unk2);
					ash->sound = le16toh(ash->sound);
					ash->flags0 = le16toh(ash->flags0);
					ash->flags1 = le16toh(ash->flags1);
					ash->unkE = le16toh(ash->unkE);
					ash->offset = le32toh(ash->offset);
					if (ash->offset != 0) {
						LvlAnimSeqFrameHeader *asfh = (LvlAnimSeqFrameHeader *)(base + ash->offset);
						asfh->move = le16toh(asfh->move);
						asfh->anim = le16toh(asfh->anim);
					}
				}
			}
		}
	}
	dat->refCount = 0xFF;
	dat->framesData = (framesDataOffset == 0) ? 0 : base + framesDataOffset;
	dat->hotspotsData = (hotspotsDataOffset == 0) ? 0 : base + hotspotsDataOffset;
	dat->movesData = (movesDataOffset == 0) ? 0 : base + movesDataOffset;
	dat->coordsData = (coordsDataOffset == 0) ? 0 : base + coordsDataOffset;

	dat->framesOffsetsTable = ptr;
	if (dat->coordsData) {
		dat->coordsOffsetsTable = ptr + dat->framesCount * 4;
	} else {
		dat->coordsOffsetsTable = 0;
	}

	uint32_t framesOffset = 0;
	for (int i = 0; i < dat->framesCount; ++i) {
		const int size = READ_LE_UINT16(dat->framesData + framesOffset);
		WRITE_LE_UINT32(dat->framesOffsetsTable + i * sizeof(uint32_t), framesOffset);
		framesOffset += size;
	}
	uint32_t coordsOffset = 0;
	for (int i = 0; i < dat->coordsCount; ++i) {
		const int count = dat->coordsData[coordsOffset];
		WRITE_LE_UINT32(dat->coordsOffsetsTable + i * sizeof(uint32_t), coordsOffset);
		coordsOffset += count * 4 + 1;
	}

	return (dat->framesCount + dat->coordsCount) * sizeof(uint32_t);
}

void Resource::loadLvlSpriteData(int num) {
	_lvlFile->seekAlign(0x2988 + num * 16);
	const uint32_t offs = _lvlFile->readUint32();
	const uint32_t size = _lvlFile->readUint32();
	const uint32_t readSize = _lvlFile->readUint32();
	uint8_t *ptr = (uint8_t *)calloc(size, 1);
	_lvlFile->seek(offs, SEEK_SET);
	_lvlFile->read(ptr, readSize);

	LvlObjectData *dat = &_resLevelData0x2988Table[num];
	const uint32_t readOffsetsSize = resFixPointersLevelData0x2988(ptr, ptr + readSize, dat);
	assert(readSize <= size);
	const uint32_t allocatedOffsetsSize = size - readSize;
	assert(allocatedOffsetsSize == readOffsetsSize);

	_resLevelData0x2988PtrTable[dat->spriteNum] = dat;
	// _resLvlScreenSpriteDataPtrTable[num] = ptr;
	_resLevelData0x2988SizeTable[num] = size;
}

uint8_t *Resource::getLvlScreenMaskDataPtr(int num) {
	assert(num >= 0 && num < 160);
	const uint32_t offset = READ_LE_UINT32(_resLevelData0x470CTablePtrHdr + num * 8 + 0);
	return (offset != 0) ? _resLevelData0x470CTable + offset : 0;
}

uint8_t *Resource::getLvlScreenPosDataPtr(int num) {
	assert(num >= 0 && num < 160);
	const uint32_t offset = READ_LE_UINT32(_resLevelData0x470CTablePtrHdr + num * 8 + 4);
	return (offset != 0) ? _resLevelData0x470CTable + offset : 0;
}

void Resource::loadLevelData0x470C() {
	_lvlFile->seekAlign(0x4708);
	uint32_t offs = _lvlFile->readUint32();
	uint32_t size = _lvlFile->readUint32();
	_resLevelData0x470CTable = (uint8_t *)calloc(size, 1);
	_lvlFile->seek(offs, SEEK_SET);
	_lvlFile->read(_resLevelData0x470CTable, size);
	_resLevelData0x470CTablePtrHdr = _resLevelData0x470CTable;
	_resLevelData0x470CTablePtrData = _resLevelData0x470CTable + 1280;
}

static const uint32_t kLvlHdrTag = 0x484F4400; // 'HOD\x00'

void Resource::loadLvlData(File *fp, const char *name) {

	assert(fp == _lvlFile);

	const uint32_t tag = _lvlFile->readUint32();
	assert(tag == kLvlHdrTag);

	_lvlHdr.screensCount = _lvlFile->readByte();
	_lvlHdr.staticLvlObjectsCount = _lvlFile->readByte();
	_lvlHdr.otherLvlObjectsCount = _lvlFile->readByte();
	_lvlHdr.spritesCount = _lvlFile->readByte();
	debug(kDebug_RESOURCE, "Resource::loadLvlData() %d %d %d %d", _lvlHdr.screensCount, _lvlHdr.staticLvlObjectsCount, _lvlHdr.otherLvlObjectsCount, _lvlHdr.spritesCount);

	for (int i = 0; i < _lvlHdr.screensCount; ++i) {
		loadLvlScreenMoveData(i);
	}
	for (int i = 0; i < _lvlHdr.screensCount; ++i) {
		loadLvlScreenVectorData(i);
	}
	for (int i = 0; i < _lvlHdr.screensCount; ++i) {
		loadLvlScreenStateData(i);
	}
	for (int i = 0; i < (0x2988 - 0x288) / 96; ++i) {
		loadLvlScreenObjectData(i);
	}

	loadLevelData0x470C();

	memset(_resLevelData0x2B88SizeTable, 0, sizeof(_resLevelData0x2B88SizeTable));
	memset(_resLevelData0x2988SizeTable, 0, sizeof(_resLevelData0x2988SizeTable));
	memset(_resLevelData0x2988PtrTable, 0, sizeof(_resLevelData0x2988PtrTable));

	for (int i = 0; i < _lvlHdr.spritesCount; ++i) {
		loadLvlSpriteData(i);
	}
}

static uint32_t resFixPointersLevelData0x2B88(const uint8_t *src, uint8_t *ptr, uint8_t *offsetsPtr, LvlBackgroundData *dat) {
	const uint8_t *src_ = src;

	dat->backgroundCount = *src++;
	dat->currentBackgroundId = *src++;
	dat->maskCount = *src++;
	dat->currentMaskId = *src++;
	dat->shadowCount = *src++;
	dat->currentShadowId = *src++;
	dat->soundCount = *src++;
	dat->currentSoundId = *src++;
	dat->dataUnk3Count = *src++;
	dat->unk9 = *src++;
	dat->dataUnk45Count = *src++;
	dat->unkB = *src++;
	dat->backgroundPaletteId = READ_LE_UINT16(src); src += 2;
	dat->backgroundBitmapId  = READ_LE_UINT16(src); src += 2;
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		dat->backgroundPaletteTable[i] = (offs != 0) ? ptr + offs : 0;
	}
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		dat->backgroundBitmapTable[i] = (offs != 0) ? ptr + offs : 0;
	}
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		dat->dataUnk0Table[i] = (offs != 0) ? ptr + offs : 0;
	}
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		dat->backgroundMaskTable[i] = (offs != 0) ? ptr + offs : 0;
	}
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		dat->backgroundSoundTable[i] = (offs != 0) ? ptr + offs : 0;
	}
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		dat->backgroundAnimationTable[i] = (offs != 0) ? ptr + offs : 0;
	}
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		dat->dataUnk4Table[i] = (offs != 0) ? ptr + offs : 0;
	}
	uint32_t offsetsSize = 0;
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		if (offs != 0) {
			dat->dataUnk5Table[i] = (LvlObjectData *)malloc(sizeof(LvlObjectData));
			offsetsSize += resFixPointersLevelData0x2988(ptr + offs, offsetsPtr + offsetsSize, dat->dataUnk5Table[i]);
		} else {
			dat->dataUnk5Table[i] = 0;
		}
	}
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		dat->dataUnk6Table[i] = (offs != 0) ? ptr + offs : 0;
	}
	assert((src - src_) == 160);
	return offsetsSize;
}

void Resource::loadLvlScreenBackgroundData(int num) {
	assert(num >= 0 && num < 40);

	_lvlFile->seekAlign(0x2B88 + num * 16);
	const uint32_t offs = _lvlFile->readUint32();
	const uint32_t size = _lvlFile->readUint32();
	const uint32_t readSize = _lvlFile->readUint32();
	uint8_t *ptr = (uint8_t *)calloc(size, 1);
	_lvlFile->seek(offs, SEEK_SET);
	_lvlFile->read(ptr, readSize);

	_lvlFile->seekAlign(0x2E08 + num * 160);
	uint8_t buf[160];
	_lvlFile->read(buf, 160);
	LvlBackgroundData *dat = &_resLvlScreenBackgroundDataTable[num];
	const uint32_t readOffsetsSize = resFixPointersLevelData0x2B88(buf, ptr, ptr + readSize, dat);

	assert(size >= readSize);
	const uint32_t allocatedOffsetsSize = size - readSize;
	assert(allocatedOffsetsSize == readOffsetsSize);

	_resLvlScreenBackgroundDataPtrTable[num] = ptr;
	_resLevelData0x2B88SizeTable[num] = size;
}

void Resource::unloadLvlScreenBackgroundData(int num) {
	if (_resLevelData0x2B88SizeTable[num] != 0) {
		free(_resLvlScreenBackgroundDataPtrTable[num]);
		_resLvlScreenBackgroundDataPtrTable[num] = 0;
		_resLevelData0x2B88SizeTable[num] = 0;
	}
}

bool Resource::isLvlSpriteDataLoaded(int num) {
	return _resLevelData0x2988SizeTable[num] != 0;
}

bool Resource::isLvlBackgroundDataLoaded(int num) {
	return _resLevelData0x2B88SizeTable[num] != 0;
}

void Resource::incLvlSpriteDataRefCounter(LvlObject *ptr) {
	LvlObjectData *dat = _resLevelData0x2988PtrTable[ptr->spriteNum];
	assert(dat);
	++dat->refCount;
	ptr->levelData0x2988 = dat;
}

void Resource::decLvlSpriteDataRefCounter(LvlObject *ptr) {
	LvlObjectData *dat = _resLevelData0x2988PtrTable[ptr->spriteNum];
	if (dat) {
		--dat->refCount;
	}
}

void Resource::loadDatHintImage(int num, uint8_t *dst, uint8_t *pal) {
	const int offset = _datHdr.hintsImageOffsetTable[num];
	const int size = _datHdr.hintsImageSizeTable[num];
	assert(size == 256 * 192);
	_datFile->seek(offset, SEEK_SET);
	_datFile->read(dst, size);
	_datFile->flush();
	_datFile->read(pal, 768);
}

void Resource::loadDatLoadingImage(uint8_t *dst, uint8_t *pal) {
	if (_loadingImageBuffer) {
		const uint32_t bufferSize = READ_LE_UINT32(_loadingImageBuffer);
		const int size = decodeLZW(_loadingImageBuffer + 8, dst);
		assert(size == 256 * 192);
		// palette follows compressed bitmap (and uses 8 bits per color)
		memcpy(pal, _loadingImageBuffer + 8 + bufferSize, 256 * 3);
	}
}

void Resource::loadDatMenuBuffers() {
	uint32_t baseOffset = _menuBuffersOffset;
	_datFile->seek(baseOffset, SEEK_SET);
	_menuBuffer1 = (uint8_t *)malloc(_datHdr.bufferSize1);
	if (_menuBuffer1) {
		_datFile->read(_menuBuffer1, _datHdr.bufferSize1);
	}

	baseOffset += fioAlignSizeTo2048(_datHdr.bufferSize1); // align to the next sector
	_datFile->seek(baseOffset, SEEK_SET);
	_menuBuffer0 = (uint8_t *)malloc(_datHdr.bufferSize0);
	if (_menuBuffer0) {
		_datFile->read(_menuBuffer0, _datHdr.bufferSize0);
	}
}

const uint8_t *Resource::getLvlSpriteFramePtr(LvlObjectData *dat, int frame) const {
	assert(frame < dat->framesCount);
	return dat->framesData + READ_LE_UINT32(dat->framesOffsetsTable + frame * sizeof(uint32_t));
}

const uint8_t *Resource::getLvlSpriteCoordPtr(LvlObjectData *dat, int num) const {
	assert(num < dat->coordsCount);
	return dat->coordsData + READ_LE_UINT32(dat->coordsOffsetsTable + num * sizeof(uint32_t));
}

static int skipBytesAlign(File *f, int len) {
	const int size = (len + 3) & ~3;
	f->seek(size, SEEK_CUR);
	return size;
}

static int readBytesAlign(File *f, uint8_t *buf, int len) {
	f->read(buf, len);
	if ((len & 3) != 0) {
		f->seek(4 - (len & 3), SEEK_CUR);
	}
	return (len + 3) & ~3;
}

void Resource::loadSssData(File *fp, const char *name) {

	assert(fp == _sssFile || fp == _datFile);

	if (_sssHdr.bufferSize != 0) {
		const int count = MIN(_sssHdr.pcmCount, _sssHdr.preloadPcmCount);
		for (int i = 0; i < count; ++i) {
			free(_sssPcmTable[i].ptr);
		}
		unloadSssData();
		_sssHdr.bufferSize = 0;
		_sssHdr.infosDataCount = 0;
	}
	_sssHdr.version = fp->readUint32();
	if (_sssHdr.version != 6 && _sssHdr.version != 10 && _sssHdr.version != 12) {
		warning("Unhandled .sss version %d", _sssHdr.version);
		closeDat(_fs, _sssFile);
		return;
	}
	_sssHdr.bufferSize = fp->readUint32();
	_sssHdr.preloadPcmCount = fp->readUint32();
	_sssHdr.preloadInfoCount = fp->readUint32();
	debug(kDebug_RESOURCE, "_sssHdr.bufferSize %d _sssHdr.preloadPcmCount %d _sssHdr.preloadInfoCount %d", _sssHdr.bufferSize, _sssHdr.preloadPcmCount, _sssHdr.preloadInfoCount);
	_sssHdr.infosDataCount = fp->readUint32();
	_sssHdr.filtersDataCount = fp->readUint32();
	_sssHdr.banksDataCount = fp->readUint32();
	debug(kDebug_RESOURCE, "_sssHdr.infosDataCount %d _sssHdr.filtersDataCount %d _sssHdr.banksDataCount %d", _sssHdr.infosDataCount, _sssHdr.filtersDataCount, _sssHdr.banksDataCount);
	_sssHdr.samplesDataCount = fp->readUint32();
	_sssHdr.codeSize = fp->readUint32();
	debug(kDebug_RESOURCE, "_sssHdr.samplesDataCount %d _sssHdr.codeSize %d", _sssHdr.samplesDataCount, _sssHdr.codeSize);
	if (_sssHdr.version == 10 || _sssHdr.version == 12) {
		_sssHdr.preloadData1Count = fp->readUint32() & 255; // pcm
		_sssHdr.preloadData2Count = fp->readUint32() & 255; // sprites
		_sssHdr.preloadData3Count = fp->readUint32() & 255; // mst
	}
	_sssHdr.pcmCount = fp->readUint32();

	const int bufferSize = _sssHdr.bufferSize + _sssHdr.filtersDataCount * 52 + _sssHdr.banksDataCount * 56;
	debug(kDebug_RESOURCE, "bufferSize %d", bufferSize);

	// fp->flush();
	fp->seek(2048, SEEK_SET); // fioAlignSizeTo2048(52) _sssHdr.bufferSize

	// _sssBuffer1
	int bytesRead = 0;

	_sssInfosData = (SssInfo *)malloc(_sssHdr.infosDataCount * sizeof(SssInfo));
	for (int i = 0; i < _sssHdr.infosDataCount; ++i) {
		_sssInfosData[i].sssBankIndex = fp->readUint16(); // index _sssBanksData
		_sssInfosData[i].sampleIndex = fp->readByte();
		_sssInfosData[i].targetVolume = fp->readByte();
		_sssInfosData[i].targetPriority = fp->readByte();
		_sssInfosData[i].targetPanning = fp->readByte();
		_sssInfosData[i].concurrencyMask = fp->readByte();
		fp->readByte(); // padding to 8 bytes
		bytesRead += 8;
	}
	_sssDefaultsData = (SssDefaults *)malloc(_sssHdr.filtersDataCount * sizeof(SssDefaults));
	for (int i = 0; i < _sssHdr.filtersDataCount; ++i) {
		_sssDefaultsData[i].defaultVolume   = fp->readByte();
		_sssDefaultsData[i].defaultPriority = fp->readByte();
		_sssDefaultsData[i].defaultPanning  = fp->readByte();
		fp->readByte(); // padding to 4 bytes
		bytesRead += 4;
	}
	_sssBanksData = (SssBank *)malloc(_sssHdr.banksDataCount * sizeof(SssBank));
	for (int i = 0; i < _sssHdr.banksDataCount; ++i) {
		_sssBanksData[i].flags = fp->readByte();
		_sssBanksData[i].count = fp->readByte();
		assert(_sssBanksData[i].count <= 4); // matches sizeof(_sssDataUnk6.unk0)
		_sssBanksData[i].sssFilter = fp->readUint16();
		_sssBanksData[i].firstSampleIndex = fp->readUint32();
		debug(kDebug_RESOURCE, "SssBank #%d count %d codeOffset 0x%x", i, _sssBanksData[i].count, _sssBanksData[i].firstSampleIndex);
		bytesRead += 8;
	}
	_sssSamplesData = (SssSample *)malloc(_sssHdr.samplesDataCount * sizeof(SssSample));
	for (int i = 0; i < _sssHdr.samplesDataCount; ++i) {
		_sssSamplesData[i].pcm = fp->readUint16(); // 0x0
		_sssSamplesData[i].framesCount = fp->readUint16();
		_sssSamplesData[i].initVolume = fp->readByte();
		_sssSamplesData[i].unk5 = fp->readByte(); // 0x5
		_sssSamplesData[i].initPriority = fp->readByte();
		_sssSamplesData[i].initPanning = fp->readByte();
		_sssSamplesData[i].codeOffset1 = fp->readUint32();
		_sssSamplesData[i].codeOffset2 = fp->readUint32();
		_sssSamplesData[i].codeOffset3 = fp->readUint32();
		_sssSamplesData[i].codeOffset4 = fp->readUint32();
		debug(kDebug_RESOURCE, "SssSample #%d pcm %d frames %d", i, _sssSamplesData[i].pcm, _sssSamplesData[i].framesCount);
		bytesRead += 24;
	}
	_sssCodeData = (uint8_t *)malloc(_sssHdr.codeSize);
	fp->read(_sssCodeData, _sssHdr.codeSize);
	bytesRead += _sssHdr.codeSize;
	if (_sssHdr.version == 10 || _sssHdr.version == 12) {
		// _sssPreloadData1
		for (int i = 0; i < _sssHdr.preloadData1Count; ++i) {
			int addr = fp->readUint32();
			debug(kDebug_RESOURCE, "sssPreloadData1 #%d 0x%x", i, addr);
			bytesRead += 4;
		}
		// _sssPreloadData2
		for (int i = 0; i < _sssHdr.preloadData2Count; ++i) {
			int addr = fp->readUint32();
			debug(kDebug_RESOURCE, "sssPreloadData2 #%d 0x%x", i, addr);
			bytesRead += 4;
		}
		// _sssPreloadData3
		for (int i = 0; i < _sssHdr.preloadData3Count; ++i) {
			int addr = fp->readUint32();
			debug(kDebug_RESOURCE, "sssPreloadData3 #%d 0x%x", i, addr);
			bytesRead += 4;
		}
		_sssPreloadData1 = (SssPreloadData *)malloc(_sssHdr.preloadData1Count * sizeof(SssPreloadData));
		for (int i = 0; i < _sssHdr.preloadData1Count; ++i) {
			// _sssPreloadData1[i] = data
			const int count = (_sssHdr.version == 12) ? fp->readUint16() * 2 : fp->readByte();
			_sssPreloadData1[i].count = count;
			_sssPreloadData1[i].ptr = (uint8_t *)malloc(count);
			debug(kDebug_RESOURCE, "sssPreloadData1 #%d count %d", i, count);
			fp->read(_sssPreloadData1[i].ptr, count);
			bytesRead += count + ((_sssHdr.version == 12) ? 2 : 1);
		}
		_sssPreloadData2 = (SssPreloadData *)malloc(_sssHdr.preloadData2Count * sizeof(SssPreloadData));
		for (int i = 0; i < _sssHdr.preloadData2Count; ++i) {
			// _sssPreloadData2[i] = data
			const int count = fp->readByte();
			_sssPreloadData2[i].count = count;
			_sssPreloadData2[i].ptr = (uint8_t *)malloc(count);
			debug(kDebug_RESOURCE, "sssPreloadData2 #%d count %d", i, count);
			fp->read(_sssPreloadData2[i].ptr, count);
			bytesRead += count + 1;
		}
		_sssPreloadData3 = (SssPreloadData *)malloc(_sssHdr.preloadData3Count * sizeof(SssPreloadData));
		for (int i = 0; i < _sssHdr.preloadData3Count; ++i) {
			// _sssPreloadData3[i] = data
			const int count = fp->readByte();
			_sssPreloadData3[i].count = count;
			_sssPreloadData3[i].ptr = (uint8_t *)malloc(count);
			debug(kDebug_RESOURCE, "sssPreloadData3 #%d count %d", i, count);
			fp->read(_sssPreloadData3[i].ptr, count);
			bytesRead += count + 1;
		}
// 429A06
		{
			const int count = fp->readByte();
			fp->seek(count, SEEK_CUR);
			bytesRead += count + 1;
		}
		// _sssPreloadInfosData = data;
	} else {
		_sssPreloadData1 = 0;
		_sssPreloadData2 = 0;
		_sssPreloadData3 = 0;
	}
// 429A20
	// data += _sssHdr.preloadInfoCount * 8;
	_sssPreloadInfosData = (SssPreloadInfo *)malloc(_sssHdr.preloadInfoCount * sizeof(SssPreloadInfo));
	for (int i = 0; i < _sssHdr.preloadInfoCount; ++i) {
		int32_t count = fp->readUint32();
		int32_t offset = fp->readUint32();
		_sssPreloadInfosData[i].count = count;
		debug(kDebug_RESOURCE, "_sssPreloadInfosData #%d/%d count %d offset 0x%x", i, _sssHdr.preloadInfoCount, count, offset);
		bytesRead += 8;
	}
	if (_sssHdr.version == 10 || _sssHdr.version == 12) {
		static const int kSizeOfUnk4Data_V11 = 32;

		for (int i = 0; i < _sssHdr.preloadInfoCount; ++i) {
			const int size = _sssPreloadInfosData[i].count * kSizeOfUnk4Data_V11;
			fp->seek(size, SEEK_CUR);
			bytesRead += size;
// 429A25
		}
	} else if (_sssHdr.version == 6) {
// 42E8DF
		static const int kSizeOfUnk4Data_V6 = 68;
		for (int i = 0; i < _sssHdr.preloadInfoCount; ++i) {
			const int count = _sssPreloadInfosData[i].count;
			uint8_t *p = (uint8_t *)malloc(kSizeOfUnk4Data_V6 * count);
			fp->read(p, kSizeOfUnk4Data_V6 * count);
			_sssPreloadInfosData[i].data = p;

			bytesRead += kSizeOfUnk4Data_V6 * count;

			for (int j = 0; j < count; ++j) {

				const uint32_t unk0x2C = READ_LE_UINT32(p + j * kSizeOfUnk4Data_V6 + 0x2C) * 2;
				const uint32_t unk0x30 = READ_LE_UINT32(p + j * kSizeOfUnk4Data_V6 + 0x30);
				const uint32_t unk0x34 = READ_LE_UINT32(p + j * kSizeOfUnk4Data_V6 + 0x34);
				const uint32_t unk0x04 = READ_LE_UINT32(p + j * kSizeOfUnk4Data_V6 + 0x04) * 2;
				const uint32_t unk0x08 = READ_LE_UINT32(p + j * kSizeOfUnk4Data_V6 + 0x08) * 2;
				const uint32_t unk0x0C = READ_LE_UINT32(p + j * kSizeOfUnk4Data_V6 + 0x0C);
				const uint32_t unk0x10 = READ_LE_UINT32(p + j * kSizeOfUnk4Data_V6 + 0x10);
				const uint32_t unk0x14 = READ_LE_UINT32(p + j * kSizeOfUnk4Data_V6 + 0x14);

				bytesRead += skipBytesAlign(_sssFile, unk0x2C);
				bytesRead += skipBytesAlign(_sssFile, unk0x30);
				bytesRead += skipBytesAlign(_sssFile, unk0x34);
				bytesRead += skipBytesAlign(_sssFile, unk0x04);
				bytesRead += skipBytesAlign(_sssFile, unk0x08);
				bytesRead += skipBytesAlign(_sssFile, unk0x0C);
				bytesRead += skipBytesAlign(_sssFile, unk0x10);
				bytesRead += skipBytesAlign(_sssFile, unk0x14);
			}
		}
	}

	_sssPcmTable = (SssPcm *)malloc(_sssHdr.pcmCount * sizeof(SssPcm));
// 429AB8
	for (int i = 0; i < _sssHdr.pcmCount; ++i) {
		_sssPcmTable[i].ptr = 0; fp->readUint32();
		_sssPcmTable[i].offset = fp->readUint32();
		_sssPcmTable[i].totalSize = fp->readUint32();
		_sssPcmTable[i].strideSize = fp->readUint32();
		_sssPcmTable[i].strideCount = fp->readUint16();
		_sssPcmTable[i].flags = fp->readUint16();
		debug(kDebug_RESOURCE, "sssPcmTable #%d/%d offset 0x%x size %d", i, _sssHdr.pcmCount, _sssPcmTable[i].offset, _sssPcmTable[i].totalSize);
		if (_sssPcmTable[i].totalSize != 0) {
			assert((_sssPcmTable[i].totalSize % _sssPcmTable[i].strideSize) == 0);
			assert(_sssPcmTable[i].totalSize == _sssPcmTable[i].strideSize * _sssPcmTable[i].strideCount);
		}
		bytesRead += 20;
	}

	// allocate structure but skip read as table is cleared and initialized in clearSoundObjects()
	static const int kSizeOfSssFilter = 52;
	fp->seek(_sssHdr.filtersDataCount * kSizeOfSssFilter, SEEK_CUR);
	_sssFilters = (SssFilter *)malloc(_sssHdr.filtersDataCount * sizeof(SssFilter));
	bytesRead += _sssHdr.filtersDataCount * kSizeOfSssFilter;

	_sssDataUnk6 = (SssUnk6 *)malloc(_sssHdr.banksDataCount * sizeof(SssUnk6));
	for (int i = 0; i < _sssHdr.banksDataCount; ++i) {
		_sssDataUnk6[i].unk0[0] = fp->readUint32();
		_sssDataUnk6[i].unk0[1] = fp->readUint32();
		_sssDataUnk6[i].unk0[2] = fp->readUint32();
		_sssDataUnk6[i].unk0[3] = fp->readUint32();
		_sssDataUnk6[i].mask    = fp->readUint32();
		bytesRead += 20;
		debug(kDebug_RESOURCE, "sssDataUnk6 #%d/%d unk10 0x%x", i, _sssHdr.banksDataCount);
	}

// 429AB8
	const int lutSize = _sssHdr.banksDataCount * sizeof(uint32_t);
	for (int i = 0; i < 3; ++i) {
		// allocate structures but skip read as tables are initialized in clearSoundObjects()
		_sssLookupTable1[i] = (uint32_t *)malloc(lutSize);
		_sssLookupTable2[i] = (uint32_t *)malloc(lutSize);
		_sssLookupTable3[i] = (uint32_t *)malloc(lutSize);
		fp->seek(lutSize * 3, SEEK_CUR);
		bytesRead += lutSize * 3;
	}
	_sssPreloadedPcmTotalSize = 0;
// 429B4B
	for (int i = 0; i < _sssHdr.pcmCount; ++i) {
		// _sssPcmTable[i].offset += baseOffset;
		// _sssPcmTable[i].ptr = 0;
		if (i >= _sssHdr.preloadPcmCount && _sssPcmTable[i].strideCount != 0) {
			// *var103++ = i;
			_sssPreloadedPcmTotalSize += _sssPcmTable[i].totalSize;
		}
		// if (_sssPcmTable[i].totalSize == 0) {
		//	_sssPcmTable[i].strideCount = 1;
		// }
	}

// 429B9F
	checkSssCode(_sssCodeData, _sssHdr.codeSize);
	for (int i = 0; i < _sssHdr.banksDataCount; ++i) {
		if (_sssBanksData[i].count != 0) {
			// const int num = _sssBanksData[i].firstSampleIndex;
			// _sssBanksData[i].codeOffset = &_sssSamplesData[num];
		} else {
			_sssBanksData[i].firstSampleIndex = kNone;
		}
	}
// 429C00
	for (int i = 0; i < _sssHdr.samplesDataCount; ++i) {
		if (_sssSamplesData[i].codeOffset1 != kNone) {
		}
		if (_sssSamplesData[i].codeOffset2 != kNone) {
		}
		if (_sssSamplesData[i].codeOffset3 != kNone) {
		}
		if (_sssSamplesData[i].codeOffset4 != kNone) {
		}
	}
	debug(kDebug_RESOURCE, "bufferSize %d bytesRead %d", bufferSize, bytesRead);
	if (bufferSize != bytesRead) {
		error("Unexpected number of bytes read %d (%d)", bytesRead, bufferSize);
	}

// 429C96
	if (0 && _sssHdr.filtersDataCount != 0) {
		fp->flush();
		uint8_t buf[256];
		assert(_sssHdr.filtersDataCount <= (int)sizeof(buf));
		fp->read(buf, _sssHdr.filtersDataCount);
		for (int i = 0; i < _sssHdr.filtersDataCount; i += 4) {
			uint32_t j = READ_LE_UINT32(buf + i);
			debug(kDebug_RESOURCE, "unk14 offset 0x%x data 0x%x", i, j);
		}
	}
// 429D32
	for (int i = 0; i < _sssHdr.banksDataCount; ++i) {
		uint32_t mask = 1;
		_sssDataUnk6[i].mask = 0;
		const SssSample *codeOffset = &_sssSamplesData[_sssBanksData[i].firstSampleIndex];
		uint32_t *ptr = _sssDataUnk6[i].unk0;
		for (int j = 0; j < _sssBanksData[i].count; ++j) {
			for (int k = 0; k < codeOffset->unk5; ++k) {
				if (mask != 0) {
					*ptr |= mask;
					mask <<= 1;
				}
			}
			++codeOffset;
			_sssDataUnk6[i].mask |= *ptr;
			++ptr;
		}
	}

// 429E09
	memset(_sssFilters, 0, _sssHdr.filtersDataCount * sizeof(SssFilter));
// 429E64
	for (int i = 0; i < _sssHdr.filtersDataCount; ++i) {
		const int a = _sssDefaultsData[i].defaultVolume;
		_sssFilters[i].volumeCurrent = a << 16;
		_sssFilters[i].volume = a;
		const int b = _sssDefaultsData[i].defaultPanning;
		_sssFilters[i].panningCurrent = b << 16;
		_sssFilters[i].panning = b;
		const int c = _sssDefaultsData[i].defaultPriority;
		_sssFilters[i].priorityCurrent = c;
		_sssFilters[i].priority = c;
	}
// 429EFA
	// same as clearSoundObjects()

// 429F38
	clearSssLookupTable3();
}

void Resource::unloadSssData() {
	free(_sssInfosData);
	_sssInfosData = 0;
	free(_sssDefaultsData);
	_sssDefaultsData = 0;
	free(_sssBanksData);
	_sssBanksData = 0;
	free(_sssSamplesData);
	_sssSamplesData = 0;
	free(_sssPreloadInfosData);
	_sssPreloadInfosData = 0;
	free(_sssFilters);
	_sssFilters = 0;
	for (int i = 0; i < _sssHdr.pcmCount; ++i) {
		free(_sssPcmTable[i].ptr);
	}
	free(_sssPcmTable);
	_sssPcmTable = 0;
        free(_sssDataUnk6);
	_sssDataUnk6 = 0;
	free(_sssPreloadData1);
	_sssPreloadData1 = 0;
	free(_sssPreloadData2);
	_sssPreloadData2 = 0;
	free(_sssPreloadData3);
	_sssPreloadData3 = 0;
	for (int i = 0; i < 3; ++i) {
		free(_sssLookupTable1[i]);
		_sssLookupTable1[i] = 0;
		free(_sssLookupTable2[i]);
		_sssLookupTable2[i] = 0;
		free(_sssLookupTable3[i]);
		_sssLookupTable3[i] = 0;
	}
	free(_sssCodeData);
	_sssCodeData = 0;
}

void Resource::checkSssCode(const uint8_t *buf, int size) const {
	static const uint8_t _opcodesLength[] = {
		4, 0, 4, 0, 4, 12, 8, 0, 16, 4, 4, 4, 4, 8, 12, 0, 4, 8, 8, 4, 4, 4, 8, 4, 8, 4, 8, 16, 8, 4
	};
	int offset = 0;
	while (offset < size) {
		const int len = _opcodesLength[buf[offset]];
		if (len == 0) {
			warning("Invalid .sss opcode %d", buf[offset]);
			break;
		}
		offset += len;
	}
	assert(offset == size);
}

uint32_t Resource::getSssPcmSize(const SssPcm *pcm) const {
	return (pcm->strideSize - 256 * sizeof(int16_t)) * pcm->strideCount * sizeof(int16_t);
}

void Resource::loadSssPcm(File *fp, int num) {
	if (_sssPcmTable[num].ptr) {
		return;
	}
	SssPcm *pcm = &_sssPcmTable[num];
	const uint32_t decompressedSize = getSssPcmSize(pcm);
	if (decompressedSize != 0) {
		debug(kDebug_SOUND, "Loading PCM %d decompressedSize %d", num, decompressedSize);
		int16_t *p = (int16_t *)malloc(decompressedSize);
		if (!p) {
			warning("Failed to allocate %d bytes for PCM %d", decompressedSize, num);
			return;
		}
		pcm->ptr = p;
		fp->seek(pcm->offset, SEEK_SET);
		for (int i = 0; i < pcm->strideCount; ++i) {
			int16_t lut[256];
			for (int j = 0; j < 256; ++j) {
				lut[j] = fp->readUint16();
			}
			for (uint32_t j = 256 * sizeof(int16_t); j < pcm->strideSize; ++j) {
				*p++ = lut[fp->readByte()];
			}
		}
		assert((p - pcm->ptr) * sizeof(int16_t) == decompressedSize);
	}
}

void Resource::clearSssLookupTable3() {
	if (_sssHdr.banksDataCount != 0) {
		for (int i = 0; i < 3; ++i) {
			memset(_sssLookupTable3[i], 0, _sssHdr.banksDataCount * sizeof(uint32_t));
		}
	}
}

void Resource::loadMstData(File *fp, const char *name) {
	assert(fp == _mstFile);
	_mstHdr.version  = fp->readUint32();
	_mstHdr.dataSize = fp->readUint32();
	_mstHdr.unk0x08  = fp->readUint32();
	_mstHdr.unk0x0C  = fp->readUint32();
	_mstHdr.unk0x10  = fp->readUint32();
	_mstHdr.screenInitDataCount  = fp->readUint32();
	_mstHdr.screenAreaCodesCount = fp->readUint32();
	_mstHdr.unk0x1C  = fp->readUint32();
	_mstHdr.unk0x20  = fp->readUint32();
	_mstHdr.unk0x24  = fp->readUint32();
	_mstHdr.unk0x28  = fp->readUint32();
	_mstHdr.unk0x2C  = fp->readUint32();
	_mstHdr.unk0x30  = fp->readUint32();
	_mstHdr.unk0x34  = fp->readUint32();
	_mstHdr.unk0x38  = fp->readUint32();
	_mstHdr.unk0x3C  = fp->readUint32();
	_mstHdr.unk0x40  = fp->readUint32();
	_mstHdr.unk0x44  = fp->readUint32();
	_mstHdr.unk0x48  = fp->readUint32();
	_mstHdr.unk0x4C  = fp->readUint32();
	_mstHdr.unk0x50  = fp->readUint32();
	_mstHdr.unk0x54  = fp->readUint32();
	_mstHdr.unk0x58  = fp->readUint32();
	_mstHdr.unk0x5C  = fp->readUint32();
	_mstHdr.unk0x60  = fp->readUint32();
	_mstHdr.unk0x64  = fp->readUint32();
	_mstHdr.unk0x68  = fp->readUint32();
	_mstHdr.unk0x6C  = fp->readUint32();
	_mstHdr.unk0x70  = fp->readUint32();
	_mstHdr.unk0x74  = fp->readUint32();
	_mstHdr.mstOp204DataCount    = fp->readUint32();
	_mstHdr.codeSize    = fp->readUint32();
	_mstHdr.pointsCount = fp->readUint32();
	debug(kDebug_RESOURCE, "_mstHdr.version %d _mstHdr.codeSize %d", _mstHdr.version, _mstHdr.codeSize);

	fp->seek(2048, SEEK_SET);

	int bytesRead = 0;

	_mstPointOffsets = (MstPointOffset *)malloc(_mstHdr.pointsCount * sizeof(MstPointOffset));
	for (int i = 0; i < _mstHdr.pointsCount; ++i) {
		_mstPointOffsets[i].xOffset = fp->readUint32();
		_mstPointOffsets[i].yOffset = fp->readUint32();
		bytesRead += 8;
	}

	_mstUnk34 = (MstUnk34 *)malloc(_mstHdr.unk0x08 * sizeof(MstUnk34));
	for (int i = 0; i < _mstHdr.unk0x08; ++i) {
		_mstUnk34[i].right  = fp->readUint32();
		_mstUnk34[i].left   = fp->readUint32();
		_mstUnk34[i].top    = fp->readUint32();
		_mstUnk34[i].bottom = fp->readUint32();
		_mstUnk34[i].flags[0] = fp->readByte();
		_mstUnk34[i].flags[1] = fp->readByte();
		_mstUnk34[i].flags[2] = fp->readByte();
		_mstUnk34[i].flags[3] = fp->readByte();
		bytesRead += 20;
	}

	_mstUnk35 = (MstUnk35 *)malloc(_mstHdr.unk0x0C * sizeof(MstUnk35));
	for (int i = 0; i < _mstHdr.unk0x0C; ++i) {
		fp->readUint32();
		_mstUnk35[i].count1 = fp->readUint32();
		_mstUnk35[i].indexCodeData = (uint32_t *)malloc(_mstUnk35[i].count1 * sizeof(uint32_t));
		fp->readUint32();
		_mstUnk35[i].count2 = fp->readUint32();
		_mstUnk35[i].data2 = (uint8_t *)malloc(_mstUnk35[i].count2);
		bytesRead += 16;
	}
	for (int i = 0; i < _mstHdr.unk0x0C; ++i) {
		for (uint32_t j = 0; j < _mstUnk35[i].count1; ++j) {
			_mstUnk35[i].indexCodeData[j] = fp->readUint32();
			bytesRead += 4;
		}
		bytesRead += readBytesAlign(fp, _mstUnk35[i].data2, _mstUnk35[i].count2);
	}

	_mstUnk36 = (MstUnk36 *)malloc(_mstHdr.unk0x10 * sizeof(MstUnk36));
	if (_mstUnk36) {
		for (int i = 0; i < _mstHdr.unk0x10; ++i) {
			_mstUnk36[i].indexUnk49 = fp->readUint32();
			_mstUnk36[i].unk4 = fp->readUint32();
			_mstUnk36[i].unk8 = fp->readUint32();
			bytesRead += 12;
		}
	}

	_mstTickDelay    = fp->readUint32();
	_mstTickCodeData = fp->readUint32();
	bytesRead += 8;

	_mstScreenInitCodeData = (uint32_t *)malloc(_mstHdr.screenInitDataCount * sizeof(uint32_t));
	for (int i = 0; i < _mstHdr.screenInitDataCount; ++i) {
		_mstScreenInitCodeData[i] = fp->readUint32();
		bytesRead += 4;
	}

	_mstScreenAreaCodes = (MstScreenAreaCode *)malloc(_mstHdr.screenAreaCodesCount * sizeof(MstScreenAreaCode)); // _mstUnk38
	for (int i = 0; i < _mstHdr.screenAreaCodesCount; ++i) {
		MstScreenAreaCode *msac = &_mstScreenAreaCodes[i];
		msac->x1 = fp->readUint32();
		msac->x2 = fp->readUint32();
		msac->y1 = fp->readUint32();
		msac->y2 = fp->readUint32();
		msac->nextByPos = fp->readUint32();
		msac->prev = fp->readUint32();
		msac->nextByValue = fp->readUint32();
		msac->unk0x1C = fp->readByte();
		msac->unk0x1D = fp->readByte();
		msac->unk0x1E = fp->readUint16();
		msac->codeData = fp->readUint32();
		bytesRead += 36;
	}

	_mstUnk39 = (uint32_t *)malloc(_mstHdr.unk0x1C * sizeof(uint32_t));
	for (int i = 0; i < _mstHdr.unk0x1C; ++i) {
		_mstUnk39[i] = fp->readUint32();
		bytesRead += 4;
	}

	_mstUnk40 = (uint32_t *)malloc(_mstHdr.pointsCount * sizeof(uint32_t));
	for (int i = 0; i < _mstHdr.pointsCount; ++i) {
		_mstUnk40[i] = fp->readUint32();
		bytesRead += 4;
	}

	_mstUnk41 = (uint32_t *)malloc(_mstHdr.pointsCount * sizeof(uint32_t));
	for (int i = 0; i < _mstHdr.pointsCount; ++i) {
		_mstUnk41[i] = fp->readUint32();
		bytesRead += 4;
	}

	_mstUnk42 = (MstUnk42 *)malloc(_mstHdr.unk0x20 * sizeof(MstUnk42));
	for (int i = 0; i < _mstHdr.unk0x20; ++i) {
		fp->readUint32();
		_mstUnk42[i].count1 = fp->readUint32();
		_mstUnk42[i].indexUnk46 = (uint32_t *)malloc(_mstUnk42[i].count1 * sizeof(uint32_t));
		fp->readUint32();
		_mstUnk42[i].count2 = fp->readUint32();
		_mstUnk42[i].data2  = (uint8_t *)malloc(_mstUnk42[i].count2);
		bytesRead += 16;
	}
	for (int i = 0; i < _mstHdr.unk0x20; ++i) {
		for (uint32_t j = 0; j < _mstUnk42[i].count1; ++j) {
			_mstUnk42[i].indexUnk46[j] = fp->readUint32();
			bytesRead += 4;
		}
		bytesRead += readBytesAlign(fp, _mstUnk42[i].data2, _mstUnk42[i].count2);
	}

	_mstUnk43 = (MstUnk43 *)malloc(_mstHdr.unk0x24 * sizeof(MstUnk43));
	for (int i = 0; i < _mstHdr.unk0x24; ++i) {
		fp->readUint32();
		_mstUnk43[i].count1 = fp->readUint32();
		_mstUnk43[i].indexUnk48 = (uint32_t *)malloc(_mstUnk43[i].count1 * sizeof(uint32_t));
		fp->readUint32();
		_mstUnk43[i].count2 = fp->readUint32();
		_mstUnk43[i].data2  = (uint8_t *)malloc(_mstUnk43[i].count2);
		bytesRead += 16;
	}
	for (int i = 0; i < _mstHdr.unk0x24; ++i) {
		for (uint32_t j = 0; j < _mstUnk43[i].count1; ++j) {
			_mstUnk43[i].indexUnk48[j] = fp->readUint32();
			bytesRead += 4;
		}
		bytesRead += readBytesAlign(fp, _mstUnk43[i].data2, _mstUnk43[i].count2);
	}

	_mstUnk44 = (MstUnk44 *)malloc(_mstHdr.unk0x28 * sizeof(MstUnk44));
	for (int i = 0; i < _mstHdr.unk0x28; ++i) {
		fp->readUint32();
		fp->readUint32();
		_mstUnk44[i].mask  = fp->readUint32();
		_mstUnk44[i].count = fp->readUint32();
		bytesRead += 16;
	}
	for (int i = 0; i < _mstHdr.unk0x28; ++i) {
		const int count = _mstUnk44[i].count;
		_mstUnk44[i].data = (MstUnk44Unk1 *)malloc(sizeof(MstUnk44Unk1) * count);
		for (int j = 0; j < count; ++j) {
			uint8_t data[104];
			fp->read(data, sizeof(data));
			bytesRead += 104;
			_mstUnk44[i].data[j].x1 = READ_LE_UINT32(data);
			_mstUnk44[i].data[j].x2 = READ_LE_UINT32(data + 4);
			_mstUnk44[i].data[j].y1 = READ_LE_UINT32(data + 8);
			_mstUnk44[i].data[j].y2 = READ_LE_UINT32(data + 12);
			_mstUnk44[i].data[j].indexUnk34_16 = READ_LE_UINT32(data + 16); // sizeof == 20
			_mstUnk44[i].data[j].indexUnk35_20 = READ_LE_UINT32(data + 20); // sizeof == 16
			_mstUnk44[i].data[j].indexUnk35_0x18 = READ_LE_UINT32(data + 24); // sizeof == 16
			_mstUnk44[i].data[j].indexUnk36_28 = READ_LE_UINT32(data + 28); // sizeof == 12
			_mstUnk44[i].data[j].indexUnk36_32 = READ_LE_UINT32(data + 32); // sizeof == 12
			_mstUnk44[i].data[j].indexUnk35_0x24[0] = READ_LE_UINT32(data + 36); // sizeof == 16
			_mstUnk44[i].data[j].indexUnk35_0x24[1] = READ_LE_UINT32(data + 40); // sizeof == 16
			for (int k = 0; k < 4; ++k) {
				_mstUnk44[i].data[j].unk2C[k][0] = READ_LE_UINT32(data + 0x2C + k * 8);
				_mstUnk44[i].data[j].unk2C[k][1] = READ_LE_UINT32(data + 0x30 + k * 8);
			}
			_mstUnk44[i].data[j].indexUnk44Unk1_76[0] = READ_LE_UINT32(data + 76); // sizeof == 104
			_mstUnk44[i].data[j].indexUnk44Unk1_76[1] = READ_LE_UINT32(data + 80); // sizeof == 104
			_mstUnk44[i].data[j].indexUnk44Unk1_76[2] = READ_LE_UINT32(data + 84); // sizeof == 104
			_mstUnk44[i].data[j].indexUnk44Unk1_76[3] = READ_LE_UINT32(data + 88); // sizeof == 104
			_mstUnk44[i].data[j].indexUnk44Unk1_92 = READ_LE_UINT32(data + 92); // sizeof == 104
			_mstUnk44[i].data[j].unk60[0] = (uint8_t *)malloc(count);
			_mstUnk44[i].data[j].unk60[1] = (uint8_t *)malloc(count);
		}
		_mstUnk44[i].indexUnk44Unk1 = (uint32_t *)malloc(_mstHdr.pointsCount * sizeof(uint32_t));
		for (int j = 0; j < _mstHdr.pointsCount; ++j) {
			_mstUnk44[i].indexUnk44Unk1[j] = fp->readUint32();
			bytesRead += 4;
		}
		for (int j = 0; j < count; ++j) {
			for (int k = 0; k < 2; ++k) {
				bytesRead += readBytesAlign(fp, _mstUnk44[i].data[j].unk60[k], count);
			}
		}
	}

	_mstUnk45 = (MstInfoMonster2 *)malloc(_mstHdr.unk0x2C * sizeof(MstInfoMonster2));
	for (int i = 0; i < _mstHdr.unk0x2C; ++i) {
		_mstUnk45[i].type = fp->readByte();
		_mstUnk45[i].shootMask = fp->readByte();
		_mstUnk45[i].anim = fp->readUint16();
		_mstUnk45[i].codeData = fp->readUint32();
		_mstUnk45[i].unk8 = fp->readUint32();
		bytesRead += 12;
	}

	_mstUnk46 = (MstUnk46 *)malloc(_mstHdr.unk0x30 * sizeof(MstUnk46));
	for (int i = 0; i < _mstHdr.unk0x30; ++i) {
		fp->readUint32();
		_mstUnk46[i].count = fp->readUint32();
		bytesRead += 8;
	}
	for (int i = 0; i < _mstHdr.unk0x30; ++i) {
		_mstUnk46[i].data  = (MstUnk46Unk1 *)malloc(_mstUnk46[i].count * sizeof(MstUnk46Unk1));
		for (uint32_t j = 0; j < _mstUnk46[i].count; ++j) {
			uint8_t data[44];
			fp->read(data, sizeof(data));
			bytesRead += 44;
			_mstUnk46[i].data[j].indexMonsterInfo = READ_LE_UINT32(data);
			_mstUnk46[i].data[j].anim        = READ_LE_UINT16(data + 0x04);
			_mstUnk46[i].data[j].unk6        = READ_LE_UINT16(data + 0x06);
			_mstUnk46[i].data[j].unk8        = READ_LE_UINT32(data + 0x08);
			_mstUnk46[i].data[j].energy      = READ_LE_UINT32(data + 0x0C);
			_mstUnk46[i].data[j].unk10       = READ_LE_UINT32(data + 0x10);
			_mstUnk46[i].data[j].count       = READ_LE_UINT32(data + 0x14);
			_mstUnk46[i].data[j].unk18       = READ_LE_UINT32(data + 0x18);
			_mstUnk46[i].data[j].indexUnk51  = READ_LE_UINT32(data + 0x1C);
			_mstUnk46[i].data[j].indexUnk44  = READ_LE_UINT32(data + 0x20);
			_mstUnk46[i].data[j].indexUnk47  = READ_LE_UINT32(data + 0x24);
			_mstUnk46[i].data[j].codeData    = READ_LE_UINT32(data + 0x28);
		}
	}

	_mstUnk47 = (MstUnk47 *)malloc(_mstHdr.unk0x34 * sizeof(MstUnk47));
	for (int i = 0; i < _mstHdr.unk0x34; ++i) {
		fp->readUint32();
		_mstUnk47[i].count  = fp->readUint32();
		bytesRead += 8;
	}
	for (int i = 0; i < _mstHdr.unk0x34; ++i) {
		_mstUnk47[i].data = (uint8_t *)malloc(_mstUnk47[i].count * 20);
		fp->read(_mstUnk47[i].data, _mstUnk47[i].count * 20);
		bytesRead += _mstUnk47[i].count * 20;
	}

	_mstUnk48 = (MstUnk48 *)malloc(_mstHdr.unk0x38 * sizeof(MstUnk48));
	for (int i = 0; i < _mstHdr.unk0x38; ++i) {
		MstUnk48 *m = &_mstUnk48[i];
		m->unk0 = fp->readUint16();
		m->unk2 = fp->readUint16();
		m->unk4 = fp->readByte();
		m->unk5 = fp->readByte();
		m->unk6 = fp->readByte();
		m->unk7 = fp->readByte();
		m->codeData = fp->readUint32();
		m->unk12 = 0; fp->readUint32();
		m->countUnk12 = fp->readUint32();
		fp->readUint32();
		fp->readUint32();
		fp->readUint32();
		fp->readUint32();
		m->count[0] = fp->readUint32();
		m->count[1] = fp->readUint32();
		bytesRead += 44;
	}
	for (int i = 0; i < _mstHdr.unk0x38; ++i) {
		MstUnk48 *m = &_mstUnk48[i];
		for (int j = 0; j < 2; ++j) {
			const int count = m->count[j];
			if (count != 0) {
				m->data1[j] = (uint32_t *)malloc(count * sizeof(uint32_t));
				for (int k = 0; k < count; ++k) {
					m->data1[j][k] = fp->readUint32();
				}
				bytesRead += count * 4;
				m->data2[j] = (uint32_t *)malloc(count * sizeof(uint32_t));
				for (int k = 0; k < count; ++k) {
					m->data2[j][k] = fp->readUint32();
				}
				bytesRead += count * 4;
			}
		}
		MstUnk48Unk12 *m12 = (MstUnk48Unk12 *)malloc(m->countUnk12 * sizeof(MstUnk48Unk12));
		for (int j = 0; j < m->countUnk12; ++j) {
			m12[j].unk0  = fp->readByte();
			fp->readByte();
			fp->readByte();
			fp->readByte();
			m12[j].data  = 0; fp->readUint32();
			m12[j].count = fp->readUint32();
			bytesRead += 12;
		}
		for (int j = 0; j < m->countUnk12; ++j) {
			m12[j].data = (MstUnk48Unk12Unk4 *)malloc(m12[j].count * sizeof(MstUnk48Unk12Unk4));
			for (uint32_t k = 0; k < m12[j].count; ++k) {
				uint8_t data[28];
				fp->read(data, sizeof(data));
				m12[j].data[k].unk0 = READ_LE_UINT32(data);
				m12[j].data[k].indexUnk51 = READ_LE_UINT32(data + 0x4);
				m12[j].data[k].unk8 = READ_LE_UINT32(data + 0x8);
				m12[j].data[k].unkC = READ_LE_UINT32(data + 0xC);
				m12[j].data[k].codeData = READ_LE_UINT32(data + 0x10);
				m12[j].data[k].codeData2 = READ_LE_UINT32(data + 0x14);
				m12[j].data[k].unk18 = data[0x18];
				m12[j].data[k].unk19 = data[0x19];
				m12[j].data[k].screenNum = data[0x1A];
				m12[j].data[k].monster1Index = data[0x1B];
				bytesRead += 28;
			}
		}
		m->unk12 = m12;
	}

	const int mapDataSize = _mstHdr.unk0x3C * kMonsterInfoDataSize;
	_mstMonsterInfos = (uint8_t *)malloc(mapDataSize);
	fp->read(_mstMonsterInfos, mapDataSize);
	bytesRead += mapDataSize;

	_mstUnk49 = (MstUnk49 *)malloc(_mstHdr.unk0x40 * sizeof(MstUnk49));
	for (int i = 0; i < _mstHdr.unk0x40; ++i) {
		_mstUnk49[i].indexMonsterInfo = fp->readUint32();
		fp->readUint32();
		_mstUnk49[i].count1  = fp->readUint32();
		fp->readUint32();
		_mstUnk49[i].count2  = fp->readUint32();
		_mstUnk49[i].unk14   = fp->readByte();
		_mstUnk49[i].unk15   = fp->readByte();
		_mstUnk49[i].unk16   = fp->readByte();
		_mstUnk49[i].unk17   = fp->readByte();
		bytesRead += 24;
	}
	for (int i = 0; i < _mstHdr.unk0x40; ++i) {
		_mstUnk49[i].data1 = (MstUnk49Unk1 *)malloc(_mstUnk49[i].count1 * sizeof(MstUnk49Unk1));
		const int start = _mstUnk49[i].indexMonsterInfo;
		assert(start < _mstHdr.unk0x3C);
		for (uint32_t j = 0; j < _mstUnk49[i].count1; ++j) {
			fp->readUint32();
			_mstUnk49[i].data1[j].unk4 = fp->readUint32();
			_mstUnk49[i].data1[j].unk8 = fp->readByte();
			_mstUnk49[i].data1[j].unk9 = fp->readByte();
			_mstUnk49[i].data1[j].unkA = fp->readByte();
			_mstUnk49[i].data1[j].unkB = fp->readByte();
			_mstUnk49[i].data1[j].unkC = fp->readByte();
			_mstUnk49[i].data1[j].unkD = fp->readByte();
			_mstUnk49[i].data1[j].unkE = fp->readByte();
			_mstUnk49[i].data1[j].unkF = fp->readByte();
			const uint32_t num = _mstUnk49[i].data1[j].unk4;
			assert(num < 32);
			_mstUnk49[i].data1[j].offsetMonsterInfo = start * kMonsterInfoDataSize + num * 28;
			bytesRead += 16;
		}
		_mstUnk49[i].data2 = (uint8_t *)malloc(_mstUnk49[i].count2);
		fp->read(_mstUnk49[i].data2, _mstUnk49[i].count2);
		bytesRead += _mstUnk49[i].count2;
	}

	_mstUnk50 = (MstUnk50 *)malloc(_mstHdr.unk0x44 * sizeof(MstUnk50));
	for (int i = 0; i < _mstHdr.unk0x44; ++i) {
		_mstUnk50[i].data  = 0; fp->readUint32();
		_mstUnk50[i].count = fp->readUint32();
		bytesRead += 8;
	}
	for (int i = 0; i < _mstHdr.unk0x44; ++i) {
		_mstUnk50[i].data = (MstUnk50Unk1 *)malloc(_mstUnk50[i].count * sizeof(MstUnk50Unk1));
		for (uint32_t j = 0; j < _mstUnk50[i].count; ++j) {
			_mstUnk50[i].data[j].codeData = fp->readUint32();
			_mstUnk50[i].data[j].unk4 = fp->readUint32();
			_mstUnk50[i].data[j].unk8 = fp->readUint32();
			_mstUnk50[i].data[j].unkC = fp->readUint32();
			_mstUnk50[i].data[j].unk10 = fp->readUint32();
			_mstUnk50[i].data[j].unk14 = fp->readUint32();
			_mstUnk50[i].data[j].unk18 = fp->readUint32();
			_mstUnk50[i].data[j].unk1C = fp->readUint32();
			_mstUnk50[i].data[j].unk20 = fp->readUint32();
			_mstUnk50[i].data[j].unk24 = fp->readUint32();
			bytesRead += 40;
		}
	}

	_mstUnk51 = (MstUnk51 *)malloc(_mstHdr.unk0x48 * sizeof(MstUnk51));
	for (int i = 0; i < _mstHdr.unk0x48; ++i) {
		_mstUnk51[i].indexUnk50 = fp->readUint32();
		assert(_mstUnk51[i].indexUnk50 < (uint32_t)_mstHdr.unk0x44);
		_mstUnk51[i].indexUnk50Unk1 = 0; fp->readUint32();
		_mstUnk51[i].count = fp->readUint32();
		bytesRead += 12;
	}
	for (int i = 0; i < _mstHdr.unk0x48; ++i) {
		_mstUnk51[i].indexUnk50Unk1 = (uint32_t *)malloc(_mstUnk51[i].count * 9 * sizeof(uint32_t));
		for (uint32_t j = 0; j < _mstUnk51[i].count * 9; ++j) {
			_mstUnk51[i].indexUnk50Unk1[j] = fp->readUint32();
			assert(_mstUnk51[i].indexUnk50Unk1[j] < _mstUnk50[_mstUnk51[i].indexUnk50].count);
			bytesRead += 4;
		}
	}
	_mstUnk52 = (MstUnk52 *)malloc(_mstHdr.unk0x4C * sizeof(MstUnk52));
	for (int i = 0; i < _mstHdr.unk0x4C; ++i) {
		_mstUnk52[i].unk0 = fp->readByte();
		_mstUnk52[i].unk1 = fp->readByte();
		_mstUnk52[i].unk2 = fp->readByte();
		_mstUnk52[i].unk3 = fp->readByte();
		bytesRead += 4;
	}

	_mstOp223Data = (MstOp223Data *)malloc(_mstHdr.unk0x50 * sizeof(MstOp223Data));
	for (int i = 0; i < _mstHdr.unk0x50; ++i) {
		_mstOp223Data[i].indexVar1 = fp->readUint16();
		_mstOp223Data[i].indexVar2 = fp->readUint16();
		_mstOp223Data[i].indexVar3 = fp->readUint16();
		_mstOp223Data[i].indexVar4 = fp->readUint16();
		_mstOp223Data[i].unk8      = fp->readByte();
		_mstOp223Data[i].unk9      = fp->readByte();
		_mstOp223Data[i].indexVar5 = fp->readByte();
		_mstOp223Data[i].unkB      = fp->readByte();
		_mstOp223Data[i].unkC      = fp->readUint16();
		_mstOp223Data[i].unkE      = fp->readUint16();
		_mstOp223Data[i].maskVars  = fp->readUint32();
		bytesRead += 20;
	}

	_mstOp226Data = (MstOp226Data *)malloc(_mstHdr.unk0x54 * sizeof(MstOp226Data));
	for (int i = 0; i < _mstHdr.unk0x54; ++i) {
		_mstOp226Data[i].unk0 = fp->readByte();
		_mstOp226Data[i].unk1 = fp->readByte();
		_mstOp226Data[i].unk2 = fp->readByte();
		_mstOp226Data[i].unk3 = fp->readByte();
		_mstOp226Data[i].unk4 = fp->readByte();
		_mstOp226Data[i].unk5 = fp->readByte();
		_mstOp226Data[i].unk6 = fp->readByte();
		_mstOp226Data[i].unk7 = fp->readByte();
		bytesRead += 8;
	}

	_mstOp227Data = (MstOp227Data *)malloc(_mstHdr.unk0x58 * sizeof(MstOp227Data));
	for (int i = 0; i < _mstHdr.unk0x58; ++i) {
		_mstOp227Data[i].indexVar1 = fp->readUint16();
		_mstOp227Data[i].indexVar2 = fp->readUint16();
		_mstOp227Data[i].compare   = fp->readByte();
		_mstOp227Data[i].maskVars  = fp->readByte();
		_mstOp227Data[i].codeData  = fp->readUint16();
		bytesRead += 8;
	}

	_mstOp234Data = (MstOp234Data *)malloc(_mstHdr.unk0x5C * sizeof(MstOp234Data));
	for (int i = 0; i < _mstHdr.unk0x5C; ++i) {
		_mstOp234Data[i].indexVar1 = fp->readUint16();
		_mstOp234Data[i].indexVar2 = fp->readUint16();
		_mstOp234Data[i].compare   = fp->readByte();
		_mstOp234Data[i].maskVars  = fp->readByte();
		_mstOp234Data[i].codeData  = fp->readUint16();
		bytesRead += 8;
	}

	_mstOp2Data = (MstOp2Data *)malloc(_mstHdr.unk0x60 * sizeof(MstOp2Data));
	for (int i = 0; i < _mstHdr.unk0x60; ++i) {
		_mstOp2Data[i].indexVar1 = fp->readUint32();
		_mstOp2Data[i].indexVar2 = fp->readUint32();
		_mstOp2Data[i].maskVars  = fp->readByte();
		_mstOp2Data[i].unk9      = fp->readByte();
		_mstOp2Data[i].unkA      = fp->readByte();
		_mstOp2Data[i].unkB      = fp->readByte();
		bytesRead += 12;
	}

	_mstOp197Data = (MstOp197Data *)malloc(_mstHdr.unk0x64 * sizeof(MstOp197Data));
	for (int i = 0; i < _mstHdr.unk0x64; ++i) {
		_mstOp197Data[i].unk0 = fp->readUint16();
		_mstOp197Data[i].unk2 = fp->readUint16();
		_mstOp197Data[i].unk4 = fp->readUint16();
		_mstOp197Data[i].unk6 = fp->readUint16();
		_mstOp197Data[i].maskVars = fp->readUint32();
		_mstOp197Data[i].indexUnk49 = fp->readUint16();
		_mstOp197Data[i].unkE = fp->readByte();
		_mstOp197Data[i].unkF = fp->readByte();
		bytesRead += 16;
	}

	_mstOp211Data = (MstOp211Data *)malloc(_mstHdr.unk0x68 * sizeof(MstOp211Data));
	for (int i = 0; i < _mstHdr.unk0x68; ++i) {
		_mstOp211Data[i].indexVar1 = fp->readUint16();
		_mstOp211Data[i].indexVar2 = fp->readUint16();
		_mstOp211Data[i].unk4      = fp->readUint16();
		_mstOp211Data[i].unk6      = fp->readUint16();
		_mstOp211Data[i].unk8      = fp->readByte();
		_mstOp211Data[i].unk9      = fp->readByte();
		_mstOp211Data[i].unkA      = fp->readByte();
		_mstOp211Data[i].unkB      = fp->readByte();
		_mstOp211Data[i].unkC      = fp->readByte();
		_mstOp211Data[i].unkD      = fp->readByte();
		_mstOp211Data[i].unkE      = fp->readUint16();
		bytesRead += 16;
	}

	_mstOp240Data = (MstOp240Data *)malloc(_mstHdr.unk0x6C * sizeof(MstOp240Data));
	for (int i = 0; i < _mstHdr.unk0x6C; ++i) {
		_mstOp240Data[i].flags    = fp->readUint32();
		_mstOp240Data[i].codeData = fp->readUint32();
		bytesRead += 8;
	}

	_mstUnk60 = (uint32_t *)malloc(_mstHdr.unk0x70 * sizeof(uint32_t));
	for (int i = 0; i < _mstHdr.unk0x70; ++i) {
		_mstUnk60[i] = fp->readUint32();
		bytesRead += 4;
	}

	fp->seek(_mstHdr.unk0x74 * 4, SEEK_CUR); // _mstUnk61
	bytesRead += _mstHdr.unk0x74 * 4;

	_mstOp204Data = (MstOp204Data *)malloc(_mstHdr.mstOp204DataCount * sizeof(MstOp204Data));
	for (int i = 0; i < _mstHdr.mstOp204DataCount; ++i) {
		_mstOp204Data[i].arg0 = fp->readUint32();
		_mstOp204Data[i].arg1 = fp->readUint32();
		_mstOp204Data[i].arg2 = fp->readUint32();
		_mstOp204Data[i].arg3 = fp->readUint32();
		bytesRead += 16;
	}

	_mstCodeData = (uint8_t *)malloc(_mstHdr.codeSize * 4);
	fp->read(_mstCodeData, _mstHdr.codeSize * 4);
	bytesRead += _mstHdr.codeSize * 4;

	if (bytesRead != _mstHdr.dataSize) {
		warning("Unexpected .mst bytesRead %d dataSize %d", bytesRead, _mstHdr.dataSize);
	}
}

MstScreenAreaCode *Resource::findMstCodeForPos(int num, int xPos, int yPos) {
	uint32_t i = _mstUnk40[num];
	while (i != kNone) {
		MstScreenAreaCode *msac = &_mstScreenAreaCodes[i];
		if (msac->x1 <= xPos && msac->x2 >= xPos && msac->unk0x1D != 0 && msac->y1 <= yPos && msac->y2 >= yPos) {
			return msac;
		}
		i = msac->nextByPos;
	}
	return 0;
}

void Resource::flagMstCodeForPos(int num, uint8_t value) {
	uint32_t i = _mstUnk39[num];
	while (i != kNone) {
		MstScreenAreaCode *msac = &_mstScreenAreaCodes[i];
		msac->unk0x1D = value;
		i = msac->nextByValue;
	}
}
