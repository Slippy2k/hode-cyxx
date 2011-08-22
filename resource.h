/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009 Gregory Montoir
 */

#ifndef RESOURCE_H__
#define RESOURCE_H__

#include "intern.h"

struct DatHdr {
	int _res_setupDatHeader0x40;
	uint32_t _setupImageOffsetTable[0x2E];
	uint32_t _setupImageSizeTable[0x2E];
};

struct LvlHdr {
	uint8_t screensCount;
	uint8_t staticLvlObjectsCount;
	uint8_t extraLvlObjectsCount;
	uint8_t spritesCount;
};

struct MstHdr {
};

struct SssHdr {
	int unk0;
	int unk4;
	int unk8;
	int unkC;
	int unk10;
	int unk14;
	int unk18;
	int unk1C;
	int unk20;
	int unk24;
	int unk28;
	int unk2C;
	int unk30;
};

struct Resource {

	enum {
		kMaxPtr32Size = 64
	};

	DatHdr _datHdr;
	File *_datFile;
	LvlHdr _lvlHdr;
	File *_lvlFile;
	MstHdr _mstHdr;
	SssHdr _sssHdr;
	File *_sssFile;

	uint8_t _currentScreenResourceNum;

	uint8_t _screensGrid[40 * 4];
	LvlScreenVector _screensBasePos[40];
	LvlScreenState _screensState[40];
	uint8_t *_resLevelData0x470CTable;
	uint8_t *_resLevelData0x470CTablePtrHdr;
	uint8_t *_resLevelData0x470CTablePtrData;
	uint8_t *_resLvlScreenSpriteDataPtrTable[32];
	uint32_t _resLevelData0x2B88SizeTable[40];
	uint32_t _resLevelData0x2988SizeTable[32];
	LvlObject *_resLvlData0x288PtrTable[40];
	LvlObjectData _resLevelData0x2988Table[40];
	LvlObjectData *_resLevelData0x2988PtrTable[32];
	LvlBackgroundData _resLvlScreenBackgroundDataTable[40];
	uint8_t *_resLvlScreenBackgroundDataPtrTable[40];

	LvlObject _resLvlScreenObjectDataTable[104];
	LvlObject _lvlLinkObject;

	void *_ptr32Pool[kMaxPtr32Size];
	int _ptr32PoolSize;

	bool _isDemoData;

	Resource();

	bool detectGameData();

	int addPtr(void *p) {
		assert(_ptr32PoolSize < kMaxPtr32Size);
		const int i = _ptr32PoolSize++;
		_ptr32Pool[i] = p;
		return i;
	}
	void *derefPtr(int index) {
		assert(index >= 0 && index < kMaxPtr32Size);
		return _ptr32Pool[_ptr32PoolSize];
	}

	void loadSetupDat();
	void loadLvlScreenMoveData(int num);
	void loadLvlScreenVectorData(int num);
	void loadLvlScreenStateData(int num);
	void loadLvlScreenObjectData(int num);
	void loadLvlData(const char *levelName);
	void loadLvlSpriteData(int num);

	uint8_t *getLevelData0x470CPtr0(int num);
	uint8_t *getLevelData0x470CPtr4(int num);

	void loadLevelData0x470C();

	void loadLvlScreenBackgroundData(int num);
	void unloadLvlScreenBackgroundData(int num);

	bool isLevelData0x2988Loaded(int num);
	bool isLevelData0x2B88Loaded(int num);

	void incLevelData0x2988RefCounter(LvlObject *ptr);
	void decLevelData0x2988RefCounter(LvlObject *ptr);

	LvlObject *findLvlObject(uint8_t type, uint8_t num, int index);

	void loadSetupImage(int num, uint8_t *dst, uint8_t *pal);

	uint8_t *getLvlSpriteFramePtr(LvlObjectData *dat, int frame);
	uint8_t *getLvlSpriteCoordPtr(LvlObjectData *dat, int num);

	void loadSssData(const char *levelName);
	void checkSoundSize(const uint8_t *buf, int size);
};

#endif // RESOURCE_H__

