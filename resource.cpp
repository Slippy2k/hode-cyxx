/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#include "fileio.h"
#include "game.h"
#include "lzw.h"
#include "resource.h"
#include "util.h"

Resource::Resource() {
	memset(_screensGrid, 0, sizeof(_screensGrid));
	memset(_screensBasePos, 0, sizeof(_screensBasePos));
	memset(_screensState, 0, sizeof(_screensState));
	_resLevelData0x470CTable = 0;
	_resLevelData0x470CTablePtrHdr = 0;
	_resLevelData0x470CTablePtrData = 0;
	memset(_resLvlScreenSpriteDataPtrTable, 0, sizeof(_resLvlScreenSpriteDataPtrTable));
	memset(_resLevelData0x2B88SizeTable, 0, sizeof(_resLevelData0x2B88SizeTable));
	memset(_resLevelData0x2988SizeTable, 0, sizeof(_resLevelData0x2988SizeTable));
	memset(_resLvlData0x288PtrTable, 0, sizeof(_resLvlData0x288PtrTable));
	memset(_resLevelData0x2988Table, 0, sizeof(_resLevelData0x2988Table));
	memset(_resLevelData0x2988PtrTable, 0, sizeof(_resLevelData0x2988PtrTable));
	memset(_resLvlScreenBackgroundDataTable, 0, sizeof(_resLvlScreenBackgroundDataTable));
	memset(_resLvlScreenBackgroundDataPtrTable, 0, sizeof(_resLvlScreenBackgroundDataPtrTable));
	memset(_resLvlScreenObjectDataTable, 0, sizeof(_resLvlScreenObjectDataTable));
	memset(&_lvlLinkObject, 0, sizeof(_lvlLinkObject));
	_isDemoData = detectGameData();
	if (_isDemoData) {
		_datFile = new SectorFile;
		_lvlFile = new SectorFile;
		_mstFile = 0;
		_sssFile = new SectorFile;
	} else {
		_datFile = new File;
		_lvlFile = new File;
		_mstFile = 0;
		_sssFile = 0;
	}
	_loadingImageBuffer = 0;
}

bool Resource::detectGameData() {
	File f;
	if (!f.open("SETUP.DAT")) {
		error("Unable to open 'SETUP.DAT'");
		return false;
	}
	return f.getSize() == 5804032;
}

void Resource::loadSetupDat() {
	uint8_t hdr[512];
	_datFile->open("SETUP.DAT");
	_datFile->read(hdr, sizeof(hdr));
	const int version = READ_LE_UINT32(hdr);
	_datHdr.sssOffset = READ_LE_UINT32(hdr + 0xC);
	_datHdr.yesNoQuitImage = READ_LE_UINT32(hdr + 0x40);
	_datHdr.loadingImageSize = READ_LE_UINT32(hdr + 0x48);
	const int hintsCount = _isDemoData ? 46 : 20;
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
	dat->data0x2988 = _lvlFile->readByte();
	dat->flags0 = _lvlFile->readUint16();
	dat->flags1 = _lvlFile->readUint16();
	dat->flags2 = _lvlFile->readUint16();
	dat->stateValue = _lvlFile->readByte();
	dat->stateCounter = _lvlFile->readByte();
	const uint32_t objRef = _lvlFile->readUint32();
	if (objRef) {
		dat->linkObjPtr = &_lvlLinkObject;
		debug(kDebug_RESOURCE, "loadLvlObj num %d linkObjRef %d", num, (int)objRef);
	}
	dat->width = _lvlFile->readUint16();
	dat->height = _lvlFile->readUint16();
	dat->directionKeyMask = _lvlFile->readByte();
	dat->actionKeyMask = _lvlFile->readByte();
	dat->unk22 = _lvlFile->readUint16();
	dat->soundToPlay = _lvlFile->readUint16();
	dat->unk26 = _lvlFile->readByte();
	dat->unk27 = _lvlFile->readByte();
	dat->bitmapBits = 0; _lvlFile->readUint32();
	dat->callbackFuncPtr = 0; _lvlFile->readUint32();
	dat->dataPtr = 0; _lvlFile->readUint32();
	dat->unk34 = _lvlFile->readUint32();
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
	for (int i = 0; i < dat->framesCount; ++i) {
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

static const uint32_t lvlHdrTag = 0x484F4400;

void Resource::loadLvlData(const char *levelName) {
	_lvlFile->close();
	char filename[32];
	snprintf(filename, sizeof(filename), "%s.LVL", levelName);
	if (!_lvlFile->open(filename)) {
		error("Unable to open '%s'", filename);
		return;
	}

	const uint32_t tag = _lvlFile->readUint32();
	assert(tag == lvlHdrTag);

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

//	loadLevelDataMst();
}

static void resFixPointersLevelData0x2B88(const uint8_t *src, uint8_t *ptr, LvlBackgroundData *dat) {
	const uint8_t *src_ = src;

	dat->backgroundCount = *src++;
	dat->currentBackgroundId = *src++;
	dat->dataUnk0Count = *src++;
	dat->unk3 = *src++;
	dat->dataUnk1Count = *src++;
	dat->currentDataUnk1Id = *src++;
	dat->dataUnk2Count = *src++; // soundsCount
	dat->unk7 = *src++; // currentSoundId
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
	LvlObjectData *dat = _resLevelData0x2988PtrTable[ptr->data0x2988];
	assert(dat);
	++dat->refCount;
	ptr->levelData0x2988 = dat;
}

void Resource::decLevelData0x2988RefCounter(LvlObject *ptr) {
	LvlObjectData *dat = _resLevelData0x2988PtrTable[ptr->data0x2988];
	if (dat) {
		--dat->refCount;
	}
}

LvlObject *Resource::findLvlObject(uint8_t type, uint8_t num, int index) {
	LvlObject *ptr = _resLvlData0x288PtrTable[index];
	while (ptr) {
		if (ptr->type == type && ptr->data0x2988 == num) {
			break;
		}
		ptr = ptr->nextPtr;
	}
	return ptr;
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

uint8_t *Resource::getLvlSpriteFramePtr(LvlObjectData *dat, int frame) {
	assert(frame < dat->framesCount);
	uint8_t *p = dat->framesData;
	for (int i = 0; i < frame; ++i) {
		const int size = READ_LE_UINT16(p);
		p += size;
	}
	return p;
}

uint8_t *Resource::getLvlSpriteCoordPtr(LvlObjectData *dat, int num) {
	assert(num < dat->animsCount);
	uint8_t *p = dat->coordsData;
	for (int i = 0; i < num; ++i) {
		const int count = p[0];
		p += count * 4 + 1;
	}
	return p;
}

static void dumpPcm(File *fp, const SssPcm *dpcmTable, int pcmCount) {
	for (int i = 0; i < pcmCount; ++i) {
		if (dpcmTable[i].totalSize != 0) {
			char name[32];
			snprintf(name, sizeof(name), "/tmp/%03d.raw", i);
			FILE *out = fopen(name, "wb");
			if (out) {
				fp->seek(dpcmTable[i].offset, SEEK_SET);
				for (uint32_t offset = 0; offset < dpcmTable[i].totalSize; offset += dpcmTable[i].strideSize) {
					int16_t lut[256];
					for (int j = 0; j < 256; ++j) {
						lut[j] = fp->readUint16();
					}
					for (uint32_t j = 256 * sizeof(int16_t); j < dpcmTable[i].strideSize; ++j) {
						int16_t sample = lut[fp->readByte()];
						fwrite(&sample, 1, sizeof(int16_t), out);
					}
				}
				fclose(out);
			}
		}
	}
}

void Resource::loadSssData(const char *levelName) {
	if (!_sssFile) {
		memset(&_sssHdr, 0, sizeof(_sssHdr));
		return;
	}
	char filename[32];
	snprintf(filename, sizeof(filename), "%s.SSS", levelName);
	if (!_sssFile->open(filename)) {
		error("Unable to open '%s'", filename);
		return;
	}
	// if (_sssBuffer1) {
		int count = _sssHdr.pcmCount;
		if (count > _sssHdr.unk8) {
			count = _sssHdr.unk8;
		}
		for (int i = 0; i < count; ++i) {
			free(_sssPcmTable[i].ptr);
		}
		// free(_sssBuffer1);
		// _sssBuffer1 = 0;
		_sssHdr.dataUnk1Count = 0;
	// }
	_sssHdr.unk0 = _sssFile->readUint32();
	assert(_sssHdr.unk0 == 10);
	_sssHdr.unk4 = _sssFile->readUint32(); // _edi
	_sssHdr.unk8 = _sssFile->readUint32();
	_sssHdr.unkC = _sssFile->readUint32();
	debug(kDebug_RESOURCE, "_sssHdr.unk4 %d _sssHdr.unk8 %d _sssHdr.unkC %d", _sssHdr.unk4, _sssHdr.unk8, _sssHdr.unkC);
	_sssHdr.dataUnk1Count = _sssFile->readUint32(); // _dataUnk1Count
	_sssHdr.dataUnk2Count = _sssFile->readUint32(); // _eax _dataUnk2Count
	_sssHdr.dataUnk3Count = _sssFile->readUint32(); // _ecx _dataUnk3Count
	debug(kDebug_RESOURCE, "_sssHdr.dataUnk1Count %d _sssHdr.dataUnk2Count %d _sssHdr.dataUnk3Count %d", _sssHdr.dataUnk1Count, _sssHdr.dataUnk2Count, _sssHdr.dataUnk3Count);
	_sssHdr.codeOffsetsCount = _sssFile->readUint32(); // _sssCodeOffsetsCount
	_sssHdr.codeSize = _sssFile->readUint32();
	_sssHdr.preloadData1Count = _sssFile->readUint32() & 255;
	debug(kDebug_RESOURCE, "_sssHdr.codeOffsetsCount %d _sssHdr.codeSize %d", _sssHdr.codeOffsetsCount, _sssHdr.codeSize);
	_sssHdr.preloadData2Count = _sssFile->readUint32() & 255;
	_sssHdr.preloadData3Count = _sssFile->readUint32() & 255;
	_sssHdr.pcmCount = _sssFile->readUint32(); // _edx

	const int bufferSize = _sssHdr.unk4 + _sssHdr.dataUnk2Count * 52 + _sssHdr.dataUnk3Count * 56;
	debug(kDebug_RESOURCE, "bufferSize %d", bufferSize);

	_sssFile->flush();

	// _sssBuffer1
	int bytesRead = 0;

	// _sssDataUnk1
	_sssDataUnk1 = (SssUnk1 *)malloc(_sssHdr.dataUnk1Count * sizeof(SssUnk1));
	for (int i = 0; i < _sssHdr.dataUnk1Count; ++i) {
		_sssDataUnk1[i].sssUnk3 = _sssFile->readUint16(); // index _sssDataUnk3
		_sssDataUnk1[i].unk2 = _sssFile->readByte();
		_sssDataUnk1[i].unk3 = _sssFile->readByte();
		_sssDataUnk1[i].unk4 = _sssFile->readByte();
		_sssDataUnk1[i].unk5 = _sssFile->readByte();
		_sssDataUnk1[i].unk6 = _sssFile->readByte();
		_sssDataUnk1[i].unk7 = _sssFile->readByte();
		// debug(kDebug_RESOURCE, "SssDataUnk1 #%d 0x%x 0x%x 0x%x", i, unk1, unk2, unk3);
		bytesRead += 8;
	}
	// _sssDataUnk2, indexes to _sssFilters
	_sssDataUnk2 = (SssUnk2 *)malloc(_sssHdr.dataUnk2Count * sizeof(SssUnk2));
	for (int i = 0; i < _sssHdr.dataUnk2Count; ++i) {
		_sssDataUnk2[i].unk0 = _sssFile->readByte();
		_sssDataUnk2[i].unk1 = (int8_t)_sssFile->readByte();
		_sssDataUnk2[i].unk2 = (int8_t)_sssFile->readByte();
		_sssFile->readByte(); // padding
		// debug(kDebug_RESOURCE, "SssDataUnk2 #%d %d %d %d", i, unk0, unk1, unk2);
		bytesRead += 4;
	}
	// _sssDataUnk3
	_sssDataUnk3 = (SssUnk3 *)malloc(_sssHdr.dataUnk3Count * sizeof(SssUnk3));
	for (int i = 0; i < _sssHdr.dataUnk3Count; ++i) {
		_sssDataUnk3[i].unk0 = _sssFile->readByte();
		_sssDataUnk3[i].unk1 = _sssFile->readByte();
		_sssDataUnk3[i].sssFilter = _sssFile->readUint16();
		_sssDataUnk3[i].firstCodeOffset = _sssFile->readUint32();
		debug(kDebug_RESOURCE, "SssDataUnk3 #%d unk1 %d codeOffset 0x%x", i, _sssDataUnk3[i].unk1, _sssDataUnk3[i].firstCodeOffset);
		bytesRead += 8;
	}
	// _sssCodeOffsets
	_sssCodeOffsets = (SssCodeOffset *)malloc(_sssHdr.codeOffsetsCount * sizeof(SssCodeOffset));
	for (int i = 0; i < _sssHdr.codeOffsetsCount; ++i) {
		_sssCodeOffsets[i].unk0 = _sssFile->readUint16(); // 0x0
		_sssCodeOffsets[i].unk2 = _sssFile->readUint16(); // 0x0
		_sssCodeOffsets[i].unk4 = _sssFile->readUint16(); // 0x4
		_sssCodeOffsets[i].unk6 = _sssFile->readByte(); // 0x6
		_sssCodeOffsets[i].unk7 = _sssFile->readByte(); // 0x7
		_sssCodeOffsets[i].unk8 = _sssFile->readUint32(); // 0x8 offset to sssCodeData
		_sssCodeOffsets[i].unkC = _sssFile->readUint32(); // 0xC offset to sssCodeData
		_sssCodeOffsets[i].unk10 = _sssFile->readUint32(); // 0x10 offset to sssCodeData
		_sssCodeOffsets[i].unk14 = _sssFile->readUint32(); // 0x14 offset to sssCodeData
		debug(kDebug_RESOURCE, "SssCodeOffset #%d unk0 %d unk2 %d", i, _sssCodeOffsets[i].unk0, _sssCodeOffsets[i].unk2);
		bytesRead += 24;
	}
	_sssCodeData = (uint8_t *)malloc(_sssHdr.codeSize);
	_sssFile->read(_sssCodeData, _sssHdr.codeSize);
	bytesRead += _sssHdr.codeSize;

	// _sssPreloadData1
	for (int i = 0; i < _sssHdr.preloadData1Count; ++i) {
		int addr = _sssFile->readUint32();
		debug(kDebug_RESOURCE, "sssPreloadData1 #%d 0x%x", i, addr);
		bytesRead += 4;
	}
	// _sssPreloadData2
	for (int i = 0; i < _sssHdr.preloadData2Count; ++i) {
		int addr = _sssFile->readUint32();
		debug(kDebug_RESOURCE, "sssPreloadData2 #%d 0x%x", i, addr);
		bytesRead += 4;
	}
	// _sssPreloadData3
	for (int i = 0; i < _sssHdr.preloadData3Count; ++i) {
		int addr = _sssFile->readUint32();
		debug(kDebug_RESOURCE, "sssPreloadData3 #%d 0x%x", i, addr);
		bytesRead += 4;
	}
	_sssPreloadData1 = (SssPreloadData *)malloc(_sssHdr.preloadData1Count * sizeof(SssPreloadData));
	for (int i = 0; i < _sssHdr.preloadData1Count; ++i) {
		// _sssPreloadData1[i] = data
		const int count = _sssFile->readByte();
		_sssPreloadData1[i].count = count;
		_sssPreloadData1[i].ptr = (uint8_t *)malloc(count);
		debug(kDebug_RESOURCE, "sssPreloadData1 #%d count %d", i, count);
		_sssFile->read(_sssPreloadData1[i].ptr, count);
		bytesRead += count + 1;
	}
	_sssPreloadData2 = (SssPreloadData *)malloc(_sssHdr.preloadData1Count * sizeof(SssPreloadData));
	for (int i = 0; i < _sssHdr.preloadData2Count; ++i) {
		// _sssPreloadData2[i] = data
		const int count = _sssFile->readByte();
		_sssPreloadData2[i].count = count;
		_sssPreloadData2[i].ptr = (uint8_t *)malloc(count);
		debug(kDebug_RESOURCE, "sssPreloadData2 #%d count %d", i, count);
		_sssFile->read(_sssPreloadData2[i].ptr, count);
		bytesRead += count + 1;
	}
	_sssPreloadData3 = (SssPreloadData *)malloc(_sssHdr.preloadData1Count * sizeof(SssPreloadData));
	for (int i = 0; i < _sssHdr.preloadData3Count; ++i) {
		// _sssPreloadData3[i] = data
		const int count = _sssFile->readByte();
		_sssPreloadData3[i].count = count;
		_sssPreloadData3[i].ptr = (uint8_t *)malloc(count);
		debug(kDebug_RESOURCE, "sssPreloadData3 #%d count %d", i, count);
		_sssFile->read(_sssPreloadData3[i].ptr, count);
		bytesRead += count + 1;
	}
// loc_429A06:
	{
		const int count = _sssFile->readByte();
		uint8_t buf[256];
		_sssFile->read(buf, count);
		bytesRead += count + 1;
		// _sssPreloadData4 = data;
	}
// 00429A20:
	// data += _sssHdr.unkC * 8;
	int dataUnk6Bytes = 0;
	for (int i = 0; i < _sssHdr.unkC; ++i) {
		int32_t count = _sssFile->readUint32();
		int32_t offset = _sssFile->readUint32();
		debug(kDebug_RESOURCE, "DataUnk6 #%d/%d count %d offset 0x%x", i, _sssHdr.unkC, count, offset);
		bytesRead += 8;
		dataUnk6Bytes += count * 32;
	}
	for (int i = 0; i < _sssHdr.unkC; ++i) {
// 00429A25:
		// _sssPreloadData4[i * 8 + 4] = data;
/*
		_esi = _sssPreloadData4;
		_ebp = 0;
		_ecx = *(uint32_t *)data;
		data += _ecx * 32;
		if (_ecx > 0) {
			_edx = 0;
			_ecx = _esi + i * 8 + 4;
			do {
				*(uint32_t *)(_edx + _ecx + 24) = _eax;
				++_ebp;
				_edx += 32;
				_ecx = *(uint32_t *)(_esi + _edi * 8 + 4);
				_ebx = *(uint32_t *)(_edx + _ecx - 4);
				_eax += _ebx * 4;
				_ebx = *(uint32_t *)(_esi + _edi * 8);
			} while (_ebp < _ebx);
		}
		_ebx = 0;
*/
	}
// loc_429A78:
	// TEMP:
	debug(kDebug_RESOURCE, "DataUnk6Bytes %d", dataUnk6Bytes);
	for (int i = 0; i < dataUnk6Bytes; ++i) {
		_sssFile->readByte();
		++bytesRead;
	}

	// _res_sssPcmTable = data; // size : sssHdr.unk30 * 20
	_sssPcmTable = (SssPcm *)malloc(_sssHdr.pcmCount * sizeof(SssPcm));
// loc_429AB8:
	for (int i = 0; i < _sssHdr.pcmCount; ++i) {
		int a = _sssFile->readUint32(); // ptr to PCM data
		int b = _sssFile->readUint32(); // offset in .sss
		int c = _sssFile->readUint32(); // size in .sss
		int d = _sssFile->readUint32();
		int e = _sssFile->readUint16();
		int f = _sssFile->readUint16();
		_sssPcmTable[i].ptr = 0;
		_sssPcmTable[i].offset = b;
		_sssPcmTable[i].totalSize = c;
		_sssPcmTable[i].strideSize = d;
		_sssPcmTable[i].strideCount = e;
		_sssPcmTable[i].flag = f;
		debug(kDebug_RESOURCE, "sssDpcmTable #%d/%d 0x%x offset 0x%x size %d stride %d %d flag %d", i, _sssHdr.pcmCount, a, b, c, d, e, f);
		if (c != 0) {
			assert((c % d) == 0); // total size must be multiple of stride
			assert(c == d * e); // total size is stride size * count
		}
		bytesRead += 20;
	}
	// _res_sssFilters = data; // size : sssHdr.unk14 * 52
	static const int kSizeOfDataUnk4 = 52;
	_sssFilters = (SssFilter *)malloc(_sssHdr.dataUnk2Count * sizeof(SssFilter));
	for (int i = 0; i < _sssHdr.dataUnk2Count; ++i) {
		uint8_t buf[kSizeOfDataUnk4];
		_sssFile->read(buf, kSizeOfDataUnk4);
		_sssFilters[i].unk4 = READ_LE_UINT32(buf + 4);
		_sssFilters[i].unk8 = READ_LE_UINT32(buf + 8);
		_sssFilters[i].unkC = READ_LE_UINT32(buf + 0xC);
		_sssFilters[i].unk14 = READ_LE_UINT32(buf + 0x14);
		_sssFilters[i].unk18 = READ_LE_UINT32(buf + 0x18);
		_sssFilters[i].unk1C = READ_LE_UINT32(buf + 0x1C);
		_sssFilters[i].unk24 = READ_LE_UINT32(buf + 0x24);
		_sssFilters[i].unk30 = READ_LE_UINT32(buf + 0x30);
		bytesRead += kSizeOfDataUnk4;
		// debug(kDebug_RESOURCE, "sssFilter #%d/%d ", i, _sssHdr.dataUnk2Count);
	}

	// _res_sssDataUnk6 = data; // size : sssHdr.unk18 * 20
	for (int i = 0; i < _sssHdr.dataUnk3Count; ++i) {
		uint8_t buf[20];
		_sssFile->read(buf, sizeof(buf));
		bytesRead += sizeof(buf);
		// debug(kDebug_RESOURCE, "sssUnk12 #%d/%d 0x%x", i, _sssHdr.dataUnk3Count, READ_LE_UINT32(buf));
	}

// loc_429AB8:
	const int lutSize = _sssHdr.dataUnk3Count * sizeof(uint32_t);
	for (int i = 0; i < 3; ++i) {
		_sssLookupTable1[i] = (uint32_t *)malloc(lutSize);
		for (int j = 0; j < _sssHdr.dataUnk3Count; ++j) {
			_sssLookupTable1[i][j] = _sssFile->readUint32();
		}
		debug(kDebug_RESOURCE, "sssLookupTable1[%d] = 0x%x", i, _sssLookupTable1[i][0]);
		_sssLookupTable2[i] = (uint32_t *)malloc(lutSize);
		for (int j = 0; j < _sssHdr.dataUnk3Count; ++j) {
			_sssLookupTable2[i][j] = _sssFile->readUint32();
		}
		debug(kDebug_RESOURCE, "sssLookupTable2[%d] = 0x%x", i, _sssLookupTable2[i][0]);
		_sssLookupTable3[i] = (uint32_t *)malloc(lutSize);
		for (int j = 0; j < _sssHdr.dataUnk3Count; ++j) {
			_sssLookupTable3[i][j] = _sssFile->readUint32();
		}
		debug(kDebug_RESOURCE, "sssLookupTable3[%d] = 0x%x", i, _sssLookupTable3[i][0]);
		bytesRead += lutSize * 3;
	}

// loc_429B9F:
	checkSssCode(_sssCodeData, _sssHdr.codeSize);
	if (0) {
		FILE *fp = fopen("code.sss", "wb");
		if (fp) {
			fwrite(_sssCodeData, 1, _sssHdr.codeSize, fp);
			fclose(fp);
		}
	}
	for (int i = 0; i < _sssHdr.dataUnk3Count; ++i) {
		if (_sssDataUnk3[i].unk1 != 0) {
			const int num = _sssDataUnk3[i].firstCodeOffset;
			// _sssDataUnk3[i].codeOffset = &_sssCodeOffsets[num];
			debug(kDebug_RESOURCE, "sssDataUnk3 %d num %d", i, num);
		} else {
			_sssDataUnk3[i].firstCodeOffset = 0;
		}
	}
// loc_429C00:
	for (int i = 0; i < _sssHdr.codeOffsetsCount; ++i) {
//		if (_sssCodeOffset[] != 0xFFFFFFFF) {
//		}
	}
	debug(kDebug_RESOURCE, "bufferSize %d bytesRead %d", bufferSize, bytesRead);
	assert(bufferSize == bytesRead);
// loc_429C96:
	if (_sssHdr.dataUnk2Count != 0) {
		// TODO:
		_sssFile->flush();
		uint8_t buf[256];
		assert(_sssHdr.dataUnk2Count <= (int)sizeof(buf));
		_sssFile->read(buf, _sssHdr.dataUnk2Count);
		for (int i = 0; i < _sssHdr.dataUnk2Count; i += 4) {
			uint32_t j = READ_LE_UINT32(buf + i);
			debug(kDebug_RESOURCE, "unk14 offset 0x%x data 0x%x", i, j);
		}
	}
// loc_429D32:
	// TODO:

// loc_429E64:
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
// loc_429EFA:

// loc_429F38:
	// clearSssData();
	if (0) {
		dumpPcm(_sssFile, _sssPcmTable, _sssHdr.pcmCount);
	}
}

void Resource::checkSssCode(const uint8_t *buf, int size) {
	const uint8_t *end = buf + size;
	while (buf < end) {
		switch (*buf) {
		case 0:
		case 2:
		case 4:
		case 9:
		case 10:
		case 11:
		case 12:
		case 16:
		case 19:
		case 20:
		case 21:
		case 23:
		case 25:
		case 29:
			buf += 4;
			break;
		case 6:
		case 13:
		case 17:
		case 18:
		case 22:
		case 24:
		case 26:
		case 28:
			buf += 8;
			break;
		case 5:
		case 14:
			buf += 12;
			break;
		case 8:
		case 27:
			buf += 16;
			break;
		default:
			error("Invalid .sss opcode %d", *buf);
			break;
		}
	}
}

uint32_t Resource::getSssDpcmSize(int num) const {
	if (num >= 0 && num < _sssHdr.pcmCount) {
		return (_sssPcmTable[num].strideSize - 256 * sizeof(int16_t)) * _sssPcmTable[num].strideCount * sizeof(int16_t);
	}
	return 0;
}

void Resource::loadSssDpcm(int num) {
	uint32_t decompressedSize = getSssDpcmSize(num);
	if (decompressedSize != 0 && !_sssPcmTable[num].ptr) {
		debug(kDebug_SOUND, "Loading PCM %d decompressedSize %d", num, decompressedSize);
		int16_t *p = (int16_t *)malloc(decompressedSize);
		if (p) {
			_sssPcmTable[num].ptr = p;
			_sssFile->seek(_sssPcmTable[num].offset, SEEK_SET);
			for (int i = 0; i < _sssPcmTable[num].strideCount; ++i) {
				int16_t lut[256];
				for (int j = 0; j < 256; ++j) {
					lut[j] = _sssFile->readUint16();
				}
				for (uint32_t j = 256 * sizeof(int16_t); j < _sssPcmTable[num].strideSize; ++j) {
					*p++ = lut[_sssFile->readByte()];
				}
			}
			assert((p - _sssPcmTable[num].ptr) * sizeof(int16_t) == decompressedSize);
		}
	}
}

void Resource::loadMstData(const char *levelName) {
	if (!_mstFile) {
		return;
	}
	char filename[32];
	snprintf(filename, sizeof(filename), "%s.MST", levelName);
	if (!_mstFile->open(filename)) {
		error("Unable to open '%s'", filename);
		return;
	}
	// TODO:
}
