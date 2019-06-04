/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include "fileio.h"
#include "fs.h"
#include "game.h"
#include "lzw.h"
#include "resource.h"
#include "util.h"

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
	memset(_resLvlScreenSpriteDataPtrTable, 0, sizeof(_resLvlScreenSpriteDataPtrTable));
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
	_loadingImageBuffer = 0;
}

Resource::~Resource() {
}

bool Resource::sectorAlignedGameData() {
	File f;
	if (!_fs.openFile("SETUP.DAT", &f)) {
		error("Unable to open 'SETUP.DAT'");
		return false;
	}
	uint8_t buf[2048];
	if (f.read(buf, sizeof(buf)) == sizeof(buf)) {
		if (fioUpdateCRC(0, buf, sizeof(buf)) == 0) {
			return true;
		}
	}
	return false;
}

void Resource::loadSetupDat() {
	uint8_t hdr[512];
	_fs.openFile("SETUP.DAT", _datFile);
	_datFile->read(hdr, sizeof(hdr));
	const int version = READ_LE_UINT32(hdr);
	_datHdr.sssOffset = READ_LE_UINT32(hdr + 0xC);
	_datHdr.yesNoQuitImage = READ_LE_UINT32(hdr + 0x40);
	_datHdr.loadingImageSize = READ_LE_UINT32(hdr + 0x48);
	const int hintsCount = (version == 11) ? 46 : 20;
	for (int i = 0; i < hintsCount; ++i) {
		_datHdr.hintsImageOffsetTable[i] = READ_LE_UINT32(hdr + 0x4C + i * 4);
		_datHdr.hintsImageSizeTable[i] = READ_LE_UINT32(hdr + 0x4C + (hintsCount + i) * 4);
	}
	if (version == 11) {
		_datFile->seek(2048, SEEK_SET); // fioAlignSizeTo2048(76)
		_loadingImageBuffer = (uint8_t *)malloc(_datHdr.loadingImageSize);
		if (_loadingImageBuffer) {
			_datFile->read(_loadingImageBuffer, _datHdr.loadingImageSize);
		}
	}
}

void Resource::loadLevelData(const char *levelName) {
	char filename[32];

	if (_lvlFile) {
		_fs.closeFile(_lvlFile);
		snprintf(filename, sizeof(filename), "%s.LVL", levelName);
		if (_fs.openFile(filename, _lvlFile)) {
			loadLvlData(_lvlFile);
		} else {
			error("Unable to open '%s'", filename);
		}
	}
	if (_mstFile) {
		_fs.closeFile(_mstFile);
		snprintf(filename, sizeof(filename), "%s.MST", levelName);
		if (_fs.openFile(filename, _mstFile)) {
			loadMstData(_mstFile);
		} else {
			warning("Unable to open '%s'", filename);
			memset(&_mstHdr, 0, sizeof(_mstHdr));
		}
	}
	if (_sssFile) {
		_fs.closeFile(_sssFile);
		snprintf(filename, sizeof(filename), "%s.SSS", levelName);
		if (_fs.openFile(filename, _sssFile)) {
			loadSssData(_sssFile);
		} else {
			warning("Unable to open '%s'", filename);
			memset(&_sssHdr, 0, sizeof(_sssHdr));
		}
	}
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
	dat->flags = _lvlFile->readByte();
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
	dat->sssObj = 0; _lvlFile->readUint32();
	dat->levelData0x2988 = 0; _lvlFile->readUint32();
	for (int i = 0; i < 8; ++i) {
		dat->posTable[i].x = _lvlFile->readUint16();
		dat->posTable[i].y = _lvlFile->readUint16();
	}
	dat->nextPtr = 0; _lvlFile->readUint32();
}

static void resFixPointersLevelData0x2988(uint8_t *src, uint8_t *ptr, LvlObjectData *dat) {
	uint8_t *base = src;

	dat->unk0 = *src++;
	dat->index = *src++;
	dat->framesCount = READ_LE_UINT16(src); src += 2;
	dat->hotspotsCount = READ_LE_UINT16(src); src += 2;
	dat->movesCount = READ_LE_UINT16(src); src += 2;
	dat->animsCount = READ_LE_UINT16(src); src += 2;
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
	uint32_t animsDataOffset = READ_LE_UINT32(src); src += 4; // 0x24
	uint32_t hotspotsDataOffset = READ_LE_UINT32(src); src += 4; // 0x28

	if (dat->refCount != 0) {
		return;
	}

	assert(src == base + kLvlAnimHdrOffset);
	dat->animsInfoData = base;
	for (int i = 0; i < dat->hotspotsCount; ++i) {
		LvlAnimHeader *ah = ((LvlAnimHeader *)(base + kLvlAnimHdrOffset)) + i;
		ah->unk0 = FROM_LE16(ah->unk0);
		ah->seqOffset = FROM_LE32(ah->seqOffset);
		if (ah->seqOffset != 0) {
			for (int seq = 0; seq < ah->seqCount; ++seq) {
				LvlAnimSeqHeader *ash = ((LvlAnimSeqHeader *)(base + ah->seqOffset)) + seq;
				ash->firstFrame = FROM_LE16(ash->firstFrame);
				ash->unk2 = FROM_LE16(ash->unk2);
				ash->sound = FROM_LE16(ash->sound);
				ash->flags0 = FROM_LE16(ash->flags0);
				ash->flags1 = FROM_LE16(ash->flags1);
				ash->unkE = FROM_LE16(ash->unkE);
				ash->offset = FROM_LE32(ash->offset);
				if (ash->offset != 0) {
					LvlAnimSeqFrameHeader *asfh = (LvlAnimSeqFrameHeader *)(base + ash->offset);
					asfh->move = FROM_LE16(asfh->move);
					asfh->anim = FROM_LE16(asfh->anim);
				}
			}
		}
	}
	dat->refCount = 0xFF;
	dat->framesData = (framesDataOffset == 0) ? 0 : base + framesDataOffset;
	dat->framesDataOffset = framesDataOffset;
	dat->hotspotsData = (hotspotsDataOffset == 0) ? 0 : base + hotspotsDataOffset;
	dat->movesData = (movesDataOffset == 0) ? 0 : base + movesDataOffset;
	dat->animsData = (animsDataOffset == 0) ? 0 : base + animsDataOffset;
	if (dat->animsData != 0) {
		dat->coordsData = dat->animsData + dat->framesCount * 4;
	}

#if 0 /* ResGetLvlSpriteFramePtr - ResGetLvlSpriteAnimPtr */
	/* original preallocated the structure */
	dat->framesOffsetTable = ptr;
	if (framesDataOffset != 0) {
		assert(dat->framesCount < MAX_SPRITE_FRAMES);
		p = dat->framesData = _ecx + framesDataOffset;
		for (i = 0; i < dat->framesCount; ++i) {
			WRITE_LE_UINT32(dat->framesOffsetsTable + i * 4, p);
			size = READ_LE_UINT16(p);
			p += size;
		}
	}
	if (animsDataOffset != 0) {
		assert(dat->animsData < MAX_SPRITE_ANIMS);
		for (i = 0; i < dat->animsCount; ++i) {
			WRITE_LE_UINT32(dat->coordsData + i * 4, p);
			size = p[0];
			p += size * 4 + 1;
		}
	}
#endif
}

void Resource::loadLvlSpriteData(int num) {
	_lvlFile->seekAlign(0x2988 + num * 16);
	uint32_t offs = _lvlFile->readUint32();
	uint32_t size = _lvlFile->readUint32();
	uint32_t readSize = _lvlFile->readUint32();
	uint8_t *ptr = (uint8_t *)calloc(size, 1);
	_lvlFile->seek(offs, SEEK_SET);
	_lvlFile->read(ptr, readSize);

	LvlObjectData *dat = &_resLevelData0x2988Table[num];
	resFixPointersLevelData0x2988(ptr, ptr + readSize, dat);
	_resLevelData0x2988PtrTable[dat->index] = dat;
	_resLvlScreenSpriteDataPtrTable[num] = ptr;
	_resLevelData0x2988SizeTable[num] = size;
}

uint8_t *Resource::getLevelData0x470CPtr0(int num) {
	assert(num >= 0 && num < 160);
	const uint32_t offset = READ_LE_UINT32(_resLevelData0x470CTablePtrHdr + num * 8 + 0);
	return (offset != 0) ? _resLevelData0x470CTable + offset : 0;
}

uint8_t *Resource::getLevelData0x470CPtr4(int num) {
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

void Resource::loadLvlData(File *fp) {

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

static void resFixPointersLevelData0x2B88(const uint8_t *src, uint8_t *ptr, LvlBackgroundData *dat) {
	const uint8_t *src_ = src;

	dat->backgroundCount = *src++;
	dat->currentBackgroundId = *src++;
	dat->maskCount = *src++;
	dat->currentMaskId = *src++;
	dat->dataUnk1Count = *src++;
	dat->currentDataUnk1Id = *src++;
	dat->dataUnk2Count = *src++;
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
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		if (offs != 0) {
			dat->dataUnk5Table[i] = (LvlObjectData *)malloc(sizeof(LvlObjectData));
			resFixPointersLevelData0x2988(ptr + offs, 0, dat->dataUnk5Table[i]);
		} else {
			dat->dataUnk5Table[i] = 0;
		}
	}
	for (int i = 0; i < 4; ++i) {
		const uint32_t offs = READ_LE_UINT32(src); src += 4;
		dat->dataUnk6Table[i] = (offs != 0) ? ptr + offs : 0;
	}
	assert((src - src_) == 160);
}

void Resource::loadLvlScreenBackgroundData(int num) {
	assert(num >= 0 && num < 40);

	_lvlFile->seekAlign(0x2B88 + num * 16);
	const uint32_t offs = _lvlFile->readUint32();
	const int size = _lvlFile->readUint32();
	const int readSize = _lvlFile->readUint32();
	uint8_t *ptr = (uint8_t *)calloc(size, 1);
	_lvlFile->seek(offs, SEEK_SET);
	_lvlFile->read(ptr, readSize);

	_lvlFile->seekAlign(0x2E08 + num * 160);
	uint8_t buf[160];
	_lvlFile->read(buf, 160);
	LvlBackgroundData *dat = &_resLvlScreenBackgroundDataTable[num];
	resFixPointersLevelData0x2B88(buf, ptr, dat);

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

bool Resource::isLevelData0x2988Loaded(int num) {
	return _resLevelData0x2988SizeTable[num] != 0;
}

bool Resource::isLevelData0x2B88Loaded(int num) {
	return _resLevelData0x2B88SizeTable[num] != 0;
}

void Resource::incLevelData0x2988RefCounter(LvlObject *ptr) {
	LvlObjectData *dat = _resLevelData0x2988PtrTable[ptr->spriteNum];
	assert(dat);
	++dat->refCount;
	ptr->levelData0x2988 = dat;
}

void Resource::decLevelData0x2988RefCounter(LvlObject *ptr) {
	LvlObjectData *dat = _resLevelData0x2988PtrTable[ptr->spriteNum];
	if (dat) {
		--dat->refCount;
	}
}

void Resource::loadHintImage(int num, uint8_t *dst, uint8_t *pal) {
	const int offset = _datHdr.hintsImageOffsetTable[num];
	const int size = _datHdr.hintsImageSizeTable[num];
	assert(size == 256 * 192);
	_datFile->seek(offset, SEEK_SET);
	_datFile->read(dst, size);
	_datFile->flush();
	_datFile->read(pal, 768);
}

void Resource::loadLoadingImage(uint8_t *dst, uint8_t *pal) {
	if (_loadingImageBuffer) {
		const uint32_t bufferSize = READ_LE_UINT32(_loadingImageBuffer);
		const int size = decodeLZW(_loadingImageBuffer + 8, dst);
		assert(size == 256 * 192);
		// palette follows compressed bitmap (and uses 8 bits per color)
		memcpy(pal, _loadingImageBuffer + 8 + bufferSize, 256 * 3);
	}
}

const uint8_t *Resource::getLvlSpriteFramePtr(LvlObjectData *dat, int frame) {
	assert(frame < dat->framesCount);
	uint8_t *p = dat->framesData;
	for (int i = 0; i < frame; ++i) {
		const int size = READ_LE_UINT16(p);
		p += size;
	}
	return p;
}

const uint8_t *Resource::getLvlSpriteCoordPtr(LvlObjectData *dat, int num) {
	assert(num < dat->animsCount);
	uint8_t *p = dat->coordsData;
	for (int i = 0; i < num; ++i) {
		const int count = p[0];
		p += count * 4 + 1;
	}
	return p;
}

static int skipBytesAlign(File *f, int len) {
	const int size = (len + 3) & ~3;
	f->seek(size, SEEK_CUR);
	return size;
}

void Resource::loadSssData(File *fp) {

	assert(fp == _sssFile); // TODO: or _datFile

	if (0 /* _sssBuffer1 */ ) {
		int count = _sssHdr.pcmCount;
		if (count > _sssHdr.preloadPcmCount) {
			count = _sssHdr.preloadPcmCount;
		}
		for (int i = 0; i < count; ++i) {
			free(_sssPcmTable[i].ptr);
		}
		// free(_sssBuffer1);
		// _sssBuffer1 = 0;
		_sssHdr.dataUnk1Count = 0;
	}
	_sssHdr.version = fp->readUint32();
	if (_sssHdr.version != 6 && _sssHdr.version != 10) {
		warning("Unhandled .sss version %d", _sssHdr.version);
		_fs.closeFile(_sssFile);
		return;
	}
	_sssHdr.unk4 = fp->readUint32();
	_sssHdr.preloadPcmCount = fp->readUint32();
	_sssHdr.preloadInfoCount = fp->readUint32();
	debug(kDebug_RESOURCE, "_sssHdr.unk4 %d _sssHdr.preloadPcmCount %d _sssHdr.preloadInfoCount %d", _sssHdr.unk4, _sssHdr.preloadPcmCount, _sssHdr.preloadInfoCount);
	_sssHdr.dataUnk1Count = fp->readUint32();
	_sssHdr.dataUnk2Count = fp->readUint32();
	_sssHdr.dataUnk3Count = fp->readUint32();
	debug(kDebug_RESOURCE, "_sssHdr.dataUnk1Count %d _sssHdr.dataUnk2Count %d _sssHdr.dataUnk3Count %d", _sssHdr.dataUnk1Count, _sssHdr.dataUnk2Count, _sssHdr.dataUnk3Count);
	_sssHdr.codeOffsetsCount = fp->readUint32();
	_sssHdr.codeSize = fp->readUint32();
	debug(kDebug_RESOURCE, "_sssHdr.codeOffsetsCount %d _sssHdr.codeSize %d", _sssHdr.codeOffsetsCount, _sssHdr.codeSize);
	if (_sssHdr.version == 10 || _sssHdr.version == 12) {
		_sssHdr.preloadData1Count = fp->readUint32() & 255; // pcm
		_sssHdr.preloadData2Count = fp->readUint32() & 255; // sprites
		_sssHdr.preloadData3Count = fp->readUint32() & 255; // mst
	}
	_sssHdr.pcmCount = fp->readUint32();

	const int bufferSize = _sssHdr.unk4 + _sssHdr.dataUnk2Count * 52 + _sssHdr.dataUnk3Count * 56;
	debug(kDebug_RESOURCE, "bufferSize %d", bufferSize);

	// fp->flush();
	fp->seek(2048, SEEK_SET); // fioAlignSizeTo2048(52) _sssHdr.unk4

	// _sssBuffer1
	int bytesRead = 0;

	// _sssDataUnk1
	_sssDataUnk1 = (SssUnk1 *)malloc(_sssHdr.dataUnk1Count * sizeof(SssUnk1));
	for (int i = 0; i < _sssHdr.dataUnk1Count; ++i) {
		_sssDataUnk1[i].sssUnk3 = fp->readUint16(); // index _sssDataUnk3
		_sssDataUnk1[i].unk2 = fp->readByte();
		_sssDataUnk1[i].unk3 = fp->readByte();
		_sssDataUnk1[i].unk4 = fp->readByte();
		_sssDataUnk1[i].unk5 = fp->readByte();
		_sssDataUnk1[i].unk6 = fp->readByte();
		_sssDataUnk1[i].unk7 = fp->readByte();
		// debug(kDebug_RESOURCE, "SssDataUnk1 #%d 0x%x 0x%x 0x%x", i, unk1, unk2, unk3);
		bytesRead += 8;
	}
	// _sssDataUnk2, indexes to _sssFilters
	_sssDataUnk2 = (SssUnk2 *)malloc(_sssHdr.dataUnk2Count * sizeof(SssUnk2));
	for (int i = 0; i < _sssHdr.dataUnk2Count; ++i) {
		_sssDataUnk2[i].unk0 = fp->readByte();
		_sssDataUnk2[i].unk1 = (int8_t)fp->readByte();
		_sssDataUnk2[i].unk2 = (int8_t)fp->readByte();
		fp->readByte(); // padding
		// debug(kDebug_RESOURCE, "SssDataUnk2 #%d %d %d %d", i, unk0, unk1, unk2);
		bytesRead += 4;
	}
	// _sssDataUnk3
	_sssDataUnk3 = (SssUnk3 *)malloc(_sssHdr.dataUnk3Count * sizeof(SssUnk3));
	for (int i = 0; i < _sssHdr.dataUnk3Count; ++i) {
		_sssDataUnk3[i].flags = fp->readByte();
		_sssDataUnk3[i].count = fp->readByte();
		_sssDataUnk3[i].sssFilter = fp->readUint16();
		_sssDataUnk3[i].firstCodeOffset = fp->readUint32();
		debug(kDebug_RESOURCE, "SssDataUnk3 #%d count %d codeOffset 0x%x", i, _sssDataUnk3[i].count, _sssDataUnk3[i].firstCodeOffset);
		bytesRead += 8;
	}
	// _sssCodeOffsets
	_sssCodeOffsets = (SssCodeOffset *)malloc(_sssHdr.codeOffsetsCount * sizeof(SssCodeOffset));
	for (int i = 0; i < _sssHdr.codeOffsetsCount; ++i) {
		_sssCodeOffsets[i].pcm = fp->readUint16(); // 0x0
		_sssCodeOffsets[i].unk2 = fp->readUint16(); // 0x0
		_sssCodeOffsets[i].unk4 = fp->readByte(); // 0x4
		_sssCodeOffsets[i].unk5 = fp->readByte(); // 0x5
		_sssCodeOffsets[i].unk6 = fp->readByte(); // 0x6
		_sssCodeOffsets[i].unk7 = fp->readByte(); // 0x7
		_sssCodeOffsets[i].codeOffset1 = fp->readUint32(); // 0x8 offset to sssCodeData
		_sssCodeOffsets[i].codeOffset2 = fp->readUint32(); // 0xC offset to sssCodeData
		_sssCodeOffsets[i].codeOffset3 = fp->readUint32(); // 0x10 offset to sssCodeData
		_sssCodeOffsets[i].codeOffset4 = fp->readUint32(); // 0x14 offset to sssCodeData
		debug(kDebug_RESOURCE, "SssCodeOffset #%d unk0 %d unk2 %d", i, _sssCodeOffsets[i].pcm, _sssCodeOffsets[i].unk2);
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
			bytesRead += count + 1;
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
		// _sssDataUnk4 = data;
	} else {
		_sssPreloadData1 = 0;
		_sssPreloadData2 = 0;
		_sssPreloadData3 = 0;
	}
// 429A20
	// data += _sssHdr.preloadInfoCount * 8;
	_sssDataUnk4 = (SssUnk4 *)malloc(_sssHdr.preloadInfoCount * sizeof(SssUnk4));
	for (int i = 0; i < _sssHdr.preloadInfoCount; ++i) {
		int32_t count = fp->readUint32();
		int32_t offset = fp->readUint32();
		_sssDataUnk4[i].count = count;
		debug(kDebug_RESOURCE, "DataUnk6 #%d/%d count %d offset 0x%x", i, _sssHdr.preloadInfoCount, count, offset);
		bytesRead += 8;
	}
	if (_sssHdr.version == 10 || _sssHdr.version == 12) {
		static const int kSizeOfUnk4Data_V11 = 32;

		for (int i = 0; i < _sssHdr.preloadInfoCount; ++i) {
			const int size = _sssDataUnk4[i].count * kSizeOfUnk4Data_V11;
			fp->seek(size, SEEK_CUR);
			bytesRead += size;
// 429A25
			// _sssDataUnk4[i * 8 + 4] = data;
/*
			_esi = _sssDataUnk4;
			_ebp = 0;
			_ecx = *(uint32_t *)data; // count
			data += _ecx * 32;
			if (_ecx > 0) {
				_edx = 0;
				_ecx = _esi + i * 8 + 4; // offset
				do {
					*(uint32_t *)(_edx + _ecx + 24) = _eax; //
					++_ebp;
					_edx += 32; // next entry

					_ecx = *(uint32_t *)(_esi + _edi * 8 + 4); // offset
					_ebx = *(uint32_t *)(_edx + _ecx - 4); //
					_eax += _ebx * 4;
					_ebx = *(uint32_t *)(_esi + _edi * 8); // count
				} while (_ebp < _ebx);
			}
			_ebx = 0;
*/
		}
// 429A78

	} else if (_sssHdr.version == 6) {
// 42E8DF
		static const int kSizeOfUnk4Data_V6 = 68;
		for (int i = 0; i < _sssHdr.preloadInfoCount; ++i) {
			const int count = _sssDataUnk4[i].count;
			uint8_t *p = (uint8_t *)malloc(kSizeOfUnk4Data_V6 * count);
			assert(p);
			fp->read(p, kSizeOfUnk4Data_V6 * count);
			_sssDataUnk4[i].data = p;

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

	// _res_sssPcmTable = data; // size : sssHdr.unk30 * 20
	_sssPcmTable = (SssPcm *)malloc(_sssHdr.pcmCount * sizeof(SssPcm));
// 429AB8:
	for (int i = 0; i < _sssHdr.pcmCount; ++i) {
		_sssPcmTable[i].ptr = 0; fp->readUint32();
		_sssPcmTable[i].offset = fp->readUint32();
		_sssPcmTable[i].totalSize = fp->readUint32();
		_sssPcmTable[i].strideSize = fp->readUint32();
		_sssPcmTable[i].strideCount = fp->readUint16();
		_sssPcmTable[i].flag = fp->readUint16();
		debug(kDebug_RESOURCE, "sssPcmTable #%d/%d offset 0x%x size %d", i, _sssHdr.pcmCount, _sssPcmTable[i].offset, _sssPcmTable[i].totalSize);
		if (_sssPcmTable[i].totalSize != 0) {
			assert((_sssPcmTable[i].totalSize % _sssPcmTable[i].strideSize) == 0);
			assert(_sssPcmTable[i].totalSize == _sssPcmTable[i].strideSize * _sssPcmTable[i].strideCount);
		}
		bytesRead += 20;
	}
	// _res_sssFilters = data; // size : sssHdr.unk14 * 52
	static const int kSizeOfSssFilter = 52;
	_sssFilters = (SssFilter *)malloc(_sssHdr.dataUnk2Count * sizeof(SssFilter));
	for (int i = 0; i < _sssHdr.dataUnk2Count; ++i) {
		uint8_t buf[kSizeOfSssFilter];
		fp->read(buf, kSizeOfSssFilter);
		_sssFilters[i].unk4 = READ_LE_UINT32(buf + 4);
		_sssFilters[i].unk8 = READ_LE_UINT32(buf + 8);
		_sssFilters[i].unkC = READ_LE_UINT32(buf + 0xC);
		_sssFilters[i].unk14 = READ_LE_UINT32(buf + 0x14);
		_sssFilters[i].unk18 = READ_LE_UINT32(buf + 0x18);
		_sssFilters[i].unk1C = READ_LE_UINT32(buf + 0x1C);
		_sssFilters[i].unk24 = READ_LE_UINT32(buf + 0x24);
		_sssFilters[i].unk30 = READ_LE_UINT32(buf + 0x30);
		bytesRead += kSizeOfSssFilter;
		// debug(kDebug_RESOURCE, "sssFilter #%d/%d ", i, _sssHdr.dataUnk2Count);
	}

	// _res_sssDataUnk6 = data; // size : sssHdr.unk18 * 20
	_sssDataUnk6 = (SssUnk6 *)malloc(_sssHdr.dataUnk3Count * sizeof(SssUnk6));
	for (int i = 0; i < _sssHdr.dataUnk3Count; ++i) {
		_sssDataUnk6[i].unk0[0] = fp->readUint32();
		_sssDataUnk6[i].unk0[1] = fp->readUint32();
		_sssDataUnk6[i].unk0[2] = fp->readUint32();
		_sssDataUnk6[i].unk0[3] = fp->readUint32();
		_sssDataUnk6[i].unk10 = fp->readUint32();
		bytesRead += 20;
		// debug(kDebug_RESOURCE, "sssUnk12 #%d/%d", i, _sssHdr.dataUnk3Count);
	}

// 429AB8:
	const int lutSize = _sssHdr.dataUnk3Count * sizeof(uint32_t);
	for (int i = 0; i < 3; ++i) {
		_sssLookupTable1[i] = (uint32_t *)malloc(lutSize);
		for (int j = 0; j < _sssHdr.dataUnk3Count; ++j) {
			_sssLookupTable1[i][j] = fp->readUint32();
		}
		debug(kDebug_RESOURCE, "sssLookupTable1[%d] = 0x%x", i, _sssLookupTable1[i][0]);
		_sssLookupTable2[i] = (uint32_t *)malloc(lutSize);
		for (int j = 0; j < _sssHdr.dataUnk3Count; ++j) {
			_sssLookupTable2[i][j] = fp->readUint32();
		}
		debug(kDebug_RESOURCE, "sssLookupTable2[%d] = 0x%x", i, _sssLookupTable2[i][0]);
		_sssLookupTable3[i] = (uint32_t *)malloc(lutSize);
		for (int j = 0; j < _sssHdr.dataUnk3Count; ++j) {
			_sssLookupTable3[i][j] = fp->readUint32();
		}
		debug(kDebug_RESOURCE, "sssLookupTable3[%d] = 0x%x", i, _sssLookupTable3[i][0]);
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
	for (int i = 0; i < _sssHdr.dataUnk3Count; ++i) {
		if (_sssDataUnk3[i].count != 0) {
			const int num = _sssDataUnk3[i].firstCodeOffset;
			// _sssDataUnk3[i].codeOffset = &_sssCodeOffsets[num];
			debug(kDebug_RESOURCE, "sssDataUnk3 %d num %d", i, num);
		} else {
			_sssDataUnk3[i].firstCodeOffset = kNone;
		}
	}
// 429C00
	for (int i = 0; i < _sssHdr.codeOffsetsCount; ++i) {
		if (_sssCodeOffsets[i].codeOffset1 != kNone) {
		}
		if (_sssCodeOffsets[i].codeOffset2 != kNone) {
		}
		if (_sssCodeOffsets[i].codeOffset3 != kNone) {
		}
		if (_sssCodeOffsets[i].codeOffset4 != kNone) {
		}
	}
	debug(kDebug_RESOURCE, "bufferSize %d bytesRead %d", bufferSize, bytesRead);
	if (bufferSize != bytesRead) {
		error("Unexpected number of bytes read %d (%d)", bytesRead, bufferSize);
	}

// 429C96:
	if (_sssHdr.dataUnk2Count != 0) {
		// TODO:
		fp->flush();
		uint8_t buf[256];
		assert(_sssHdr.dataUnk2Count <= (int)sizeof(buf));
		fp->read(buf, _sssHdr.dataUnk2Count);
		for (int i = 0; i < _sssHdr.dataUnk2Count; i += 4) {
			uint32_t j = READ_LE_UINT32(buf + i);
			debug(kDebug_RESOURCE, "unk14 offset 0x%x data 0x%x", i, j);
		}
	}
// 429D32
	for (int i = 0; i < _sssHdr.dataUnk3Count; ++i) {
		uint32_t mask = 1;
		_sssDataUnk6[i].unk10 = 0;
		const SssCodeOffset *codeOffset = &_sssCodeOffsets[_sssDataUnk3[i].firstCodeOffset];
		uint32_t *ptr = _sssDataUnk6[i].unk0;
		for (int j = 0; j < _sssDataUnk3[i].count; ++j) {
			for (int k = 0; k < codeOffset->unk5; ++k) {
				if (mask != 0) {
					*ptr |= mask;
					mask <<= 1;
				}
			}
			++codeOffset;
			_sssDataUnk6[i].unk10 |= *ptr;
			++ptr;
		}
	}

// 429E09
	memset(_sssFilters, 0, _sssHdr.dataUnk2Count * sizeof(SssFilter));
// 429E64
	for (int i = 0; i < _sssHdr.dataUnk2Count; ++i) {
		const uint8_t a = _sssDataUnk2[i].unk0;
		_sssFilters[i].unk4 = a << 16;
		_sssFilters[i].unk0 = a;
		const int8_t b = (int8_t)_sssDataUnk2[i].unk2;
		_sssFilters[i].unk14 = b << 16;
		_sssFilters[i].unk10 = b;
		const int8_t c = (int8_t)_sssDataUnk2[i].unk1;
		_sssFilters[i].unk24 = c;
		_sssFilters[i].unk20 = c;
	}
// 429EFA:
	// same as clearSoundObjects()

// 429F38:
	clearSssLookupTable3();
}

void Resource::checkSssCode(const uint8_t *buf, int size) {
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

uint32_t Resource::getSssPcmSize(SssPcm *pcm) const {
	return (pcm->strideSize - 256 * sizeof(int16_t)) * pcm->strideCount * sizeof(int16_t);
}

void Resource::loadSssPcm(File *fp, int num) {
	uint32_t decompressedSize = getSssPcmSize(&_sssPcmTable[num]);
	if (decompressedSize != 0 && !_sssPcmTable[num].ptr) {
		debug(kDebug_SOUND, "Loading PCM %d decompressedSize %d", num, decompressedSize);
		int16_t *p = (int16_t *)malloc(decompressedSize);
		if (p) {
			_sssPcmTable[num].ptr = p;
			fp->seek(_sssPcmTable[num].offset, SEEK_SET);
			for (int i = 0; i < _sssPcmTable[num].strideCount; ++i) {
				int16_t lut[256];
				for (int j = 0; j < 256; ++j) {
					lut[j] = fp->readUint16();
				}
				for (uint32_t j = 256 * sizeof(int16_t); j < _sssPcmTable[num].strideSize; ++j) {
					*p++ = lut[fp->readByte()];
				}
			}
			assert((p - _sssPcmTable[num].ptr) * sizeof(int16_t) == decompressedSize);
		}
	}
}

void Resource::clearSssLookupTable3() {
	for (int i = 0; i < 3; ++i) {
		memset(_sssLookupTable3[i], 0, _sssHdr.dataUnk3Count * sizeof(uint32_t));
	}
}

void Resource::loadMstData(File *fp) {
	assert(fp == _mstFile);
	_mstHdr.version  = fp->readUint32();
	_mstHdr.dataSize = fp->readUint32();
	_mstHdr.unk0x08  = fp->readUint32();
	_mstHdr.unk0x0C  = fp->readUint32();
	_mstHdr.unk0x10  = fp->readUint32();
	_mstHdr.unk0x14  = fp->readUint32();
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
	_mstHdr.unk0x78  = fp->readUint32();
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
		_mstUnk34[i].x1 = fp->readUint32();
		_mstUnk34[i].x2 = fp->readUint32();
		_mstUnk34[i].y1 = fp->readUint32();
		_mstUnk34[i].y2 = fp->readUint32();
		_mstUnk34[i].unk10 = fp->readUint32();
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
		fp->read(_mstUnk35[i].data2, _mstUnk35[i].count2);
		if ((_mstUnk35[i].count2 & 3) != 0) {
			fp->seek(4 - (_mstUnk35[i].count2 & 3), SEEK_CUR);
		}
		bytesRead += (_mstUnk35[i].count2 + 3) & ~3;
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

	_mstScreenInitCodeData = (uint32_t *)malloc(_mstHdr.unk0x14 * sizeof(uint32_t));
	for (int i = 0; i < _mstHdr.unk0x14; ++i) {
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
		msac->next = fp->readUint32();
		msac->prev = fp->readUint32();
		msac->unk0x18 = fp->readUint32();
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
		fp->read(_mstUnk42[i].data2, _mstUnk42[i].count2);
		if ((_mstUnk42[i].count2 & 3) != 0) {
			fp->seek(4 - (_mstUnk42[i].count2 & 3), SEEK_CUR);
		}
		bytesRead += (_mstUnk42[i].count2 + 3) & ~3;
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
		fp->read(_mstUnk43[i].data2, _mstUnk43[i].count2);
		if ((_mstUnk43[i].count2 & 3) != 0) {
			fp->seek(4 - (_mstUnk43[i].count2 & 3), SEEK_CUR);
		}
		bytesRead += (_mstUnk43[i].count2 + 3) & ~3;
	}

	_mstUnk44 = (MstUnk44 *)malloc(_mstHdr.unk0x28 * sizeof(MstUnk44));
	for (int i = 0; i < _mstHdr.unk0x28; ++i) {
		fp->readUint32();
		fp->readUint32();
		_mstUnk44[i].unk8  = fp->readUint32();
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
			_mstUnk44[i].data[j].indexUnk35_24 = READ_LE_UINT32(data + 24); // sizeof == 16
			_mstUnk44[i].data[j].indexUnk36_28 = READ_LE_UINT32(data + 28); // sizeof == 12
			_mstUnk44[i].data[j].indexUnk36_32 = READ_LE_UINT32(data + 32); // sizeof == 12
			_mstUnk44[i].data[j].indexUnk35_0x24[0] = READ_LE_UINT32(data + 36); // sizeof == 16
			_mstUnk44[i].data[j].indexUnk35_0x24[1] = READ_LE_UINT32(data + 40); // sizeof == 16
			_mstUnk44[i].data[j].unk2C[0] = READ_LE_UINT32(data + 0x2C);
			_mstUnk44[i].data[j].unk2C[1] = READ_LE_UINT32(data + 0x30);
			_mstUnk44[i].data[j].unk34[0] = READ_LE_UINT32(data + 0x34);
			_mstUnk44[i].data[j].unk34[1] = READ_LE_UINT32(data + 0x38);
			_mstUnk44[i].data[j].unk3C[0] = READ_LE_UINT32(data + 0x3C);
			_mstUnk44[i].data[j].unk3C[1] = READ_LE_UINT32(data + 0x40);
			_mstUnk44[i].data[j].unk44[0] = READ_LE_UINT32(data + 0x44);
			_mstUnk44[i].data[j].unk44[1] = READ_LE_UINT32(data + 0x48);
			_mstUnk44[i].data[j].indexUnk44_76 = READ_LE_UINT32(data + 76); // sizeof == 104
			_mstUnk44[i].data[j].indexUnk44_80 = READ_LE_UINT32(data + 80); // sizeof == 104
			_mstUnk44[i].data[j].indexUnk44_84 = READ_LE_UINT32(data + 84); // sizeof == 104
			_mstUnk44[i].data[j].indexUnk44_88 = READ_LE_UINT32(data + 88); // sizeof == 104
			_mstUnk44[i].data[j].indexUnk44_92 = READ_LE_UINT32(data + 92); // sizeof == 104
		}
		_mstUnk44[i].indexUnk44Unk1 = (uint32_t *)malloc(_mstHdr.pointsCount * sizeof(uint32_t));
		for (int j = 0; j < _mstHdr.pointsCount; ++j) {
			_mstUnk44[i].indexUnk44Unk1[j] = fp->readUint32();
			bytesRead += 4;
		}
		for (int j = 0; j < count; ++j) {
			for (int k = 0; k < 2; ++k) {
				bytesRead += skipBytesAlign(fp, count);
			}
		}
	}

	_mstUnk45 = (MstUnk45 *)malloc(_mstHdr.unk0x2C * sizeof(MstUnk45));
	for (int i = 0; i < _mstHdr.unk0x2C; ++i) {
		_mstUnk45[i].unk0 = fp->readByte();
		_mstUnk45[i].unk1 = fp->readByte();
		_mstUnk45[i].unk2 = fp->readUint16();
		_mstUnk45[i].unk4 = fp->readUint32();
		_mstUnk45[i].unk8 = fp->readUint32();
		bytesRead += 12;
	}

	_mstUnk46 = (MstUnk46 *)malloc(_mstHdr.unk0x30 * sizeof(MstUnk46));
	for (int i = 0; i < _mstHdr.unk0x30; ++i) {
		fp->readUint32();
		_mstUnk46[i].count = fp->readUint32();
		_mstUnk46[i].data  = (MstUnk46Unk1 *)malloc(_mstUnk46[i].count * sizeof(MstUnk46Unk1));
		bytesRead += 8;
	}
	for (int i = 0; i < _mstHdr.unk0x30; ++i) {
		for (uint32_t j = 0; j < _mstUnk46[i].count; ++j) {
			uint8_t data[44];
			fp->read(data, sizeof(data));
			bytesRead += 44;
			_mstUnk46[i].data[j].indexHeight = READ_LE_UINT32(data);
			_mstUnk46[i].data[j].anim        = READ_LE_UINT16(data + 0x04);
			_mstUnk46[i].data[j].unk6        = READ_LE_UINT16(data + 0x06);
			_mstUnk46[i].data[j].unkC        = READ_LE_UINT32(data + 0x0C);
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
				m12[j].data[k].unk8 = READ_LE_UINT32(data + 0x8);
				m12[j].data[k].unkC = READ_LE_UINT32(data + 0xC);
				m12[j].data[k].codeData = READ_LE_UINT32(data + 0x10);
				m12[j].data[k].unk18 = data[0x18];
				m12[j].data[k].unk19 = data[0x19];
				m12[j].data[k].unk1A = data[0x1A];
				m12[j].data[k].unk1B = data[0x1B];
				bytesRead += 28;
			}
		}
		m->unk12 = m12;
	}

	const int mapDataSize = _mstHdr.unk0x3C * 948;
	_mstHeightMapData = (uint8_t *)malloc(mapDataSize);
	fp->read(_mstHeightMapData, mapDataSize);
	bytesRead += mapDataSize;

	_mstUnk49 = (MstUnk49 *)malloc(_mstHdr.unk0x40 * sizeof(MstUnk49));
	for (int i = 0; i < _mstHdr.unk0x40; ++i) {
		_mstUnk49[i].unk0    = fp->readUint32();
		fp->readUint32();
		_mstUnk49[i].count1  = fp->readUint32();
		fp->readUint32();
		_mstUnk49[i].count2  = fp->readUint32();
		_mstUnk49[i].unk0x14 = fp->readUint32();
		bytesRead += 24;
	}
	for (int i = 0; i < _mstHdr.unk0x40; ++i) {
		_mstUnk49[i].data1 = (uint8_t *)malloc(_mstUnk49[i].count1 * 16);
		fp->read(_mstUnk49[i].data1, _mstUnk49[i].count1 * 16);
		bytesRead += _mstUnk49[i].count1 * 16;
		_mstUnk49[i].data2 = (uint8_t *)malloc(_mstUnk49[i].count2);
		fp->read(_mstUnk49[i].data2, _mstUnk49[i].count2);
		bytesRead += _mstUnk49[i].count2;
	}

	_mstUnk50 = (MstUnk50 *)malloc(_mstHdr.unk0x44 * sizeof(MstUnk50));
	for (int i = 0; i < _mstHdr.unk0x44; ++i) {
		_mstUnk50[i].offset = fp->readUint32();
		_mstUnk50[i].count  = fp->readUint32();
		bytesRead += 8;
	}
	for (int i = 0; i < _mstHdr.unk0x44; ++i) {
		fp->seek(_mstUnk50[i].count * 40, SEEK_CUR);
		bytesRead += _mstUnk50[i].count * 40;
	}

	_mstUnk51 = (MstUnk51 *)malloc(_mstHdr.unk0x48 * sizeof(MstUnk51));
	for (int i = 0; i < _mstHdr.unk0x48; ++i) {
		_mstUnk51[i].unk0   = fp->readUint32();
		_mstUnk51[i].offset = fp->readUint32();
		_mstUnk51[i].count  = fp->readUint32();
		bytesRead += 12;
	}
	for (int i = 0; i < _mstHdr.unk0x48; ++i) {
		fp->seek(_mstUnk51[i].count * 36, SEEK_CUR);
		bytesRead += _mstUnk51[i].count * 36;
	}
	_mstUnk52 = (uint8_t *)malloc(_mstHdr.unk0x4C * 4);
	fp->read(_mstUnk52, _mstHdr.unk0x4C * 4);
	bytesRead += _mstHdr.unk0x4C * 4;

	_mstUnk53 = (MstUnk53 *)malloc(_mstHdr.unk0x50 * sizeof(MstUnk53));
	for (int i = 0; i < _mstHdr.unk0x50; ++i) {
		_mstUnk53[i].indexVar1 = fp->readUint16();
		_mstUnk53[i].indexVar2 = fp->readUint16();
		_mstUnk53[i].indexVar3 = fp->readUint16();
		_mstUnk53[i].indexVar4 = fp->readUint16();
		_mstUnk53[i].unk8      = fp->readByte();
		_mstUnk53[i].unk9      = fp->readByte();
		_mstUnk53[i].indexVar5 = fp->readByte();
		_mstUnk53[i].unkB      = fp->readByte();
		_mstUnk53[i].unkC      = fp->readUint16();
		_mstUnk53[i].unkE      = fp->readUint16();
		_mstUnk53[i].maskVars  = fp->readUint32();
		bytesRead += 20;
	}

	_mstUnk63 = (MstUnk63 *)malloc(_mstHdr.unk0x54 * sizeof(MstUnk63));
	for (int i = 0; i < _mstHdr.unk0x54; ++i) {
		_mstUnk63[i].unk0 = fp->readByte();
		_mstUnk63[i].unk1 = fp->readByte();
		_mstUnk63[i].unk2 = fp->readByte();
		_mstUnk63[i].unk3 = fp->readByte();
		_mstUnk63[i].unk4 = fp->readByte();
		_mstUnk63[i].unk5 = fp->readByte();
		_mstUnk63[i].unk6 = fp->readByte();
		_mstUnk63[i].unk7 = fp->readByte();
		bytesRead += 8;
	}

	_mstUnk54 = (MstUnk54 *)malloc(_mstHdr.unk0x58 * sizeof(MstUnk54));
	for (int i = 0; i < _mstHdr.unk0x58; ++i) {
		_mstUnk54[i].indexVar1 = fp->readUint16();
		_mstUnk54[i].indexVar2 = fp->readUint16();
		_mstUnk54[i].compare   = fp->readByte();
		_mstUnk54[i].maskVars  = fp->readByte();
		_mstUnk54[i].codeData  = fp->readUint16();
		bytesRead += 8;
	}

	_mstUnk55 = (MstUnk55 *)malloc(_mstHdr.unk0x5C * sizeof(MstUnk55));
	for (int i = 0; i < _mstHdr.unk0x5C; ++i) {
		_mstUnk55[i].indexVar1 = fp->readUint16();
		_mstUnk55[i].indexVar2 = fp->readUint16();
		_mstUnk55[i].compare   = fp->readByte();
		_mstUnk55[i].maskVars  = fp->readByte();
		_mstUnk55[i].codeData  = fp->readUint16();
		bytesRead += 8;
	}

	_mstUnk56 = (MstUnk56 *)malloc(_mstHdr.unk0x60 * sizeof(MstUnk56));
	for (int i = 0; i < _mstHdr.unk0x60; ++i) {
		_mstUnk56[i].indexVar1 = fp->readUint32();
		_mstUnk56[i].indexVar2 = fp->readUint32();
		_mstUnk56[i].maskVars  = fp->readByte();
		_mstUnk56[i].unk9      = fp->readByte();
		_mstUnk56[i].unkA      = fp->readByte();
		_mstUnk56[i].unkB      = fp->readByte();
		bytesRead += 12;
	}

	_mstOp49Data = (MstOp49Data *)malloc(_mstHdr.unk0x64 * sizeof(MstOp49Data));
	for (int i = 0; i < _mstHdr.unk0x64; ++i) {
		_mstOp49Data[i].unk0 = fp->readUint16();
		_mstOp49Data[i].unk2 = fp->readUint16();
		_mstOp49Data[i].unk4 = fp->readUint16();
		_mstOp49Data[i].unk6 = fp->readUint16();
		_mstOp49Data[i].unk8 = fp->readUint32();
		_mstOp49Data[i].unkC = fp->readUint16();
		_mstOp49Data[i].unkE = fp->readByte();
		_mstOp49Data[i].unkF = fp->readByte();
		bytesRead += 16;
	}

	_mstOp58Data = (MstOp58Data *)malloc(_mstHdr.unk0x68 * sizeof(MstOp58Data));
	for (int i = 0; i < _mstHdr.unk0x68; ++i) {
		_mstOp58Data[i].indexVar1 = fp->readUint16();
		_mstOp58Data[i].indexVar2 = fp->readUint16();
		_mstOp58Data[i].unk4      = fp->readUint16();
		_mstOp58Data[i].unk6      = fp->readUint16();
		_mstOp58Data[i].unk8      = fp->readByte();
		_mstOp58Data[i].unk9      = fp->readByte();
		_mstOp58Data[i].unkA      = fp->readByte();
		_mstOp58Data[i].unkB      = fp->readByte();
		_mstOp58Data[i].unkC      = fp->readByte();
		_mstOp58Data[i].unkD      = fp->readByte();
		_mstOp58Data[i].unkE      = fp->readUint16();
		bytesRead += 16;
	}

	_mstUnk59 = (MstUnk59 *)malloc(_mstHdr.unk0x6C * sizeof(MstUnk59));
	for (int i = 0; i < _mstHdr.unk0x6C; ++i) {
		_mstUnk59[i].taskId   = fp->readUint32();
		_mstUnk59[i].codeData = fp->readUint32();
		bytesRead += 8;
	}

	_mstUnk60 = (uint32_t *)malloc(_mstHdr.unk0x70 * sizeof(uint32_t));
	for (int i = 0; i < _mstHdr.unk0x70; ++i) {
		_mstUnk60[i] = fp->readUint32();
		bytesRead += 4;
	}

	fp->seek(_mstHdr.unk0x74 * 4, SEEK_CUR); // _mstUnk61
	bytesRead += _mstHdr.unk0x74 * 4;

	_mstOp56Data = (MstOp56Data *)malloc(_mstHdr.unk0x78 * sizeof(MstOp56Data));
	for (int i = 0; i < _mstHdr.unk0x78; ++i) {
		_mstOp56Data[i].unk0 = fp->readUint32();
		_mstOp56Data[i].unk4 = fp->readUint32();
		_mstOp56Data[i].unk8 = fp->readUint32();
		_mstOp56Data[i].unkC = fp->readUint32();
		bytesRead += 16;
	}

	_mstCodeData = (uint8_t *)malloc(_mstHdr.codeSize * 4);
	fp->read(_mstCodeData, _mstHdr.codeSize * 4);
	bytesRead += _mstHdr.codeSize * 4;

	if (bytesRead != _mstHdr.dataSize) {
		warning("Unexpected .mst bytesRead %d dataSize %d", bytesRead, _mstHdr.dataSize);
	}

	// TODO:

	if (0) {
		for (int i = 0; i < _lvlHdr.screensCount; ++i) {
			uint32_t j = _mstUnk40[i];
			while (j != kNone) {
				MstScreenAreaCode *msac = &_mstScreenAreaCodes[j];
				fprintf(stdout, ".mst screen %d pos %d,%d,%d,%d\n", i, msac->x1, msac->y1, msac->x2, msac->y2);
				j = msac->next;
			}
		}
	}

	if (0) {
		int histOp[256];
		memset(histOp, 0, sizeof(histOp));
		for (int i = 0; i < _mstHdr.codeSize; ++i) {
			++histOp[_mstCodeData[i * 4]];
		}
		for (int i = 0; i < 256; ++i) {
			if (histOp[i]) {
				fprintf(stdout, ".mst opcode %d count %d\n", i, histOp[i]);
			}
		}
	}
}

MstScreenAreaCode *Resource::findMstCodeForPos(int num, int xPos, int yPos) {
	uint32_t i = _mstUnk40[num];
	while (i != kNone) {
		MstScreenAreaCode *msac = &_mstScreenAreaCodes[i];
		if (msac->x1 <= xPos && msac->x2 >= xPos && msac->unk0x1D != 0 && msac->y1 <= yPos && msac->y2 >= yPos) {
			return msac;
		}
		i = msac->next;
	}
	return 0;
}

void Resource::flagMstCodeForPos(int num, uint8_t value) {
	uint32_t i = _mstUnk39[num];
	while (i != kNone) {
		MstScreenAreaCode *msac = &_mstScreenAreaCodes[i];
		msac->unk0x1D = value;
		i = msac->unk0x18;
	}
}
