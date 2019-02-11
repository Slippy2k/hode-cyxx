/*
 * Heart Of Darkness engine rewrite
 * Copyright (C) 2009-2011 Gregory Montoir
 */

#ifndef RESOURCE_H__
#define RESOURCE_H__

#include "fs.h"
#include "intern.h"

struct DatHdr {
	uint32_t sssOffset; // 0xC
	int yesNoQuitImage; // 0x40
	int loadingImageSize; // 0x48
	uint32_t hintsImageOffsetTable[46];
	uint32_t hintsImageSizeTable[46];
};

struct LvlHdr {
	uint8_t screensCount;
	uint8_t staticLvlObjectsCount;
	uint8_t otherLvlObjectsCount;
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
	uint8_t *backgroundSoundTable[4];
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
	int dataUnk1Count;
	int dataUnk2Count;
	int dataUnk3Count;
	int codeOffsetsCount;
	int codeSize;
	int preloadData1Count; // 24
	int preloadData2Count; // 28
	int preloadData3Count; // 2C
	int pcmCount; // 30
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

struct SssUnk2 { // SssProperty
	uint8_t unk0; // defaultPriority
	int8_t unk1; // defaultVolume
	int8_t unk2;
};

struct SssUnk3 {
	uint8_t flags; // 0 flags0
	int8_t count; // 1
	uint16_t sssFilter; // 2 index to _sssFilters
	uint32_t firstCodeOffset; // 4 offset to _sssCodeOffsets
};

struct SssCodeOffset {
	uint16_t pcm; // index to _sssPcmTable
	uint16_t unk2;
	uint8_t unk4;
	uint8_t unk5;
	int8_t unk6;
	uint8_t unk7;
	uint32_t codeOffset1; // 0x8 offset to _sssCodeData
	uint32_t codeOffset2; // 0xC offset to _sssCodeData
	uint32_t codeOffset3; // 0x10 offset to _sssCodeData
	uint32_t codeOffset4; // 0x14 offset to _sssCodeData
};

struct SssFilter {
	int32_t unk0;
	int32_t unk4; // priority (0,7)
	int32_t unk8; // priorityDelta
	int32_t unkC; // priorityCounter
	int32_t unk10;
	int32_t unk14; // volume (0,127)
	int32_t unk18; // volumeDelta
	int32_t unk1C; // volumeCounter
	int32_t unk20;
	int32_t unk24;
	int32_t unk30; // flag
};

#define SIZEOF_SssFilter 52

struct SssPcm {
	int16_t *ptr;    // 0 PCM data
	uint32_t offset; // 4 offset in .sss
	uint32_t totalSize;   // 8 size in .sss (256 int16_t words + followed by indexes)
	uint32_t strideSize;
	uint16_t strideCount;
	uint16_t flag;
};

struct SssUnk6 {
	uint32_t unk0[4]; // 0
	uint32_t unk10; // 10
};

struct SssPreloadData {
	uint8_t count;
	uint8_t *ptr;
};

struct Resource {

	FileSystem _fs;

	DatHdr _datHdr;
	File *_datFile;
	LvlHdr _lvlHdr;
	File *_lvlFile;
	MstHdr _mstHdr;
	File *_mstFile;
	SssHdr _sssHdr;
	File *_sssFile;

	uint8_t *_loadingImageBuffer;

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
	SssFilter *_sssFilters;
	SssPcm *_sssPcmTable;
	SssUnk6 *_sssDataUnk6;
	SssPreloadData *_sssPreloadData1;
	SssPreloadData *_sssPreloadData2;
	SssPreloadData *_sssPreloadData3;
	uint32_t *_sssLookupTable1[3];
	uint32_t *_sssLookupTable2[3];
	uint32_t *_sssLookupTable3[3];
	uint8_t *_sssCodeData;

	bool _isDemoData;

	Resource(const char *dataPath);

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
	LvlObject *findLvlObject2(uint8_t type, uint8_t flags, int index);

	void loadHintImage(int num, uint8_t *dst, uint8_t *pal);
	void loadLoadingImage(uint8_t *dst, uint8_t *pal);

	uint8_t *getLvlSpriteFramePtr(LvlObjectData *dat, int frame);
	uint8_t *getLvlSpriteCoordPtr(LvlObjectData *dat, int num);

	void loadSssData(const char *levelName);
	void checkSssCode(const uint8_t *buf, int size);
	void loadSssPcm(int num);
	uint32_t getSssPcmSize(SssPcm *pcm) const;

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
		assert(0);
		return 0;
	}

	void loadMstData(const char *levelName);
};

#endif // RESOURCE_H__

