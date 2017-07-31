/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef RESOURCE_H__
#define RESOURCE_H__

#include "intern.h"

struct DatHdr {
	uint32_t sssOffset; // 0xC
	int _res_setupDatHeader0x40; // yesNoQuitImageOffset;
	uint32_t _setupImageOffsetTable[0x2E]; // hintsImageOffsetTable
	uint32_t _setupImageSizeTable[0x2E]; // hintsImageSizeTable
};

struct LvlHdr {
	uint8_t screensCount;
	uint8_t staticLvlObjectsCount;
	uint8_t extraLvlObjectsCount; // otherLvlObjectsCount
	uint8_t spritesCount;
};

struct MstHdr {
};

struct SssHdr {
	int unk0;
	int unk4;
	int unk8;
	int unkC;
	int unk10; // sssDataUnk1Count
	int unk14; // sssDataUnk2Count
	int unk18; // sssDataUnk3Count
	int unk1C; // sssCodeOffsetsCount
	int unk20; // sssCodeSize
	int unk24; // sssPreloadData1Count
	int unk28; // sssPreloadData2Count
	int unk2C; // sssPreloadData3Count
	int dpcmCount; // 30
};

struct SssUnk1 {
	uint16_t unk0; // 0 index to _sssDataUnk3
	uint8_t unk2; // 2
	uint8_t unk3;
	uint8_t unk4; // 4
	uint16_t unk5;
	uint8_t unk7;
} PACKED;

#define SIZEOF_SssUnk1 8

struct SssUnk3 {
	uint8_t unk0; // 0
	uint8_t unk1; // 1
	uint16_t sssUnk4; // 2 index to _sssDataUnk4
	uint32_t unk4; // 4 offset to init data
} PACKED;

#define SIZEOF_SssUnk3 8

struct SssUnk4 {
	uint8_t data[52];
} PACKED;

#define SIZEOF_SssUnk4 52

struct SssUnk5 {
	uint8_t *ptr;    // 0 PCM data
	uint32_t offset; // 4 offset in .sss
	uint32_t size;   // 8 size in .sss (256 int16_t words + followed with indexes)
	uint32_t unkC;
	uint32_t unk10;
} PACKED;

#define SIZEOF_SssUnk5 20

struct SssUnk6 {
	uint32_t unk0;
	uint32_t unk4;
	uint32_t unk8;
	uint32_t unkC;
	uint32_t unk10;
} PACKED;

#define SIZEOF_SssUnk6

struct Resource {

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

	SssUnk1 *_sssDataUnk1;
	SssUnk3 *_sssDataUnk3;
	SssUnk4 *_sssDataUnk4;
	SssUnk5 *_sssDpcmTable;

	uint32_t _sssCodeSize;
	uint8_t *_sssCodeData;

	bool _isDemoData;

	Resource();

	bool detectGameData();

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
	void checkSssCode(const uint8_t *buf, int size);
};

#endif // RESOURCE_H__

