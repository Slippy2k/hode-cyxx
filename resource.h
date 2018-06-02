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

struct LvlScreenVector {
	int32_t u;
	int32_t v;
};

struct LvlScreenState {
	uint8_t s0;
	uint8_t s1;
	uint8_t s2;
	uint8_t s3; // maskData
};

struct LvlBackgroundData {
	uint8_t backgroundCount;
	uint8_t currentBackgroundId;
	uint8_t dataUnk0Count;
	uint8_t unk3; /* movingMask */
	uint8_t dataUnk1Count; /* castedShadowCount */
	uint8_t currentDataUnk1Id; /* currentCastedShadowId */
	uint8_t dataUnk2Count;
	uint8_t unk7;
	uint8_t dataUnk3Count;
	uint8_t unk9;
	uint8_t dataUnk45Count;
	uint8_t unkB;
	uint16_t backgroundPaletteId;
	uint16_t backgroundBitmapId;
	uint8_t *backgroundPaletteTable[4];
	uint8_t *backgroundBitmapTable[4];
	uint8_t *dataUnk0Table[4];
	uint8_t *backgroundMaskTable[4];
	uint8_t *dataUnk2Table[4];
	uint8_t *backgroundAnimationTable[4];
	uint8_t *dataUnk4Table[4]; /* unused ? */
	LvlObjectData *dataUnk5Table[4];
	uint8_t *dataUnk6Table[4]; /* unused ? */
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
	uint16_t sssUnk3; // 0 index to _sssDataUnk3
	uint8_t unk2; // 2
	uint8_t unk3;
	uint8_t unk4; // 4
	uint8_t unk5;
	uint8_t unk6;
	uint8_t unk7;
};

struct SssUnk2 {
	uint8_t unk0;
	int8_t unk1;
	int8_t unk2;
	uint8_t pad;
};

struct SssUnk3 {
	uint8_t unk0; // 0
	uint8_t unk1; // 1
	uint16_t sssUnk4; // 2 index to _sssDataUnk4
	uint32_t firstCodeOffset; // 4 offset to _sssCodeOffsets
};

struct SssCodeOffset {
	uint16_t unk0; // index to _sssDpcmTable
	uint16_t unk2;
	uint16_t unk4;
	uint8_t unk6;
	uint8_t unk7;
	uint32_t unk8; // offset to _sssCodeData
	uint32_t unkC; // offset to _sssCodeData
	uint32_t unk10; // offset to _sssCodeData
	uint32_t unk14; // offset to _sssCodeData
};

struct SssUnk4 {
	int32_t unk4;
	int32_t unk8;
	int32_t unkC;
	int32_t unk14;
	int32_t unk18;
	int32_t unk1C;
	int32_t unk24;
	int32_t unk30;
};

struct SssUnk5 {
	uint8_t *ptr;    // 0 PCM data
	uint32_t offset; // 4 offset in .sss
	uint32_t totalSize;   // 8 size in .sss (256 int16_t words + followed with indexes)
	uint32_t strideSize;
	uint16_t strideCount;
	uint16_t flag;
};

struct SssUnk6 {
	uint32_t unk0;
	uint32_t unk4;
	uint32_t unk8;
	uint32_t unkC;
	uint32_t unk10;
};

struct SssPreloadData {
	uint8_t count;
	uint8_t *ptr;
};

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
	SssUnk2 *_sssDataUnk2;
	SssUnk3 *_sssDataUnk3;
	SssCodeOffset *_sssCodeOffsets;
	SssUnk4 *_sssDataUnk4;
	SssUnk5 *_sssDpcmTable;
	SssPreloadData *_sssPreloadData1;
	SssPreloadData *_sssPreloadData2;
	SssPreloadData *_sssPreloadData3;
	uint32_t *_sssLookupTable1[3];
	uint32_t *_sssLookupTable2[3];
	uint32_t *_sssLookupTable3[3];

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

	uint8_t *getLevelData0x470CPtr0(int num); // getLvlScreenMaskDataPtr
	uint8_t *getLevelData0x470CPtr4(int num); // getLvlScreenPosDataPtr
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

	uint32_t *getSssLutPtr(int lut, uint32_t flags) {
		const uint32_t a = (flags >> 20) & 0xF;
		const uint32_t b = flags & 0xFFF;
		switch (lut) {
		case 1:
			return _sssLookupTable1[a] + b;
		case 2:
			return _sssLookupTable2[a] + b;
		case 3:
			return _sssLookupTable3[a] + b;
		}
		return 0;
	}
};

#endif // RESOURCE_H__

